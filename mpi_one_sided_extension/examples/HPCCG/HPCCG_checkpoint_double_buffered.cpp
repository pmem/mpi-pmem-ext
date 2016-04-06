
//@HEADER
// ************************************************************************
// 
//               HPCCG: Simple Conjugate Gradient Benchmark Code
//                 Copyright (2006) Sandia Corporation
// 
// Under terms of Contract DE-AC04-94AL85000, there is a non-exclusive
// license for use of this work by or on behalf of the U.S. Government.
// 
// This library is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as
// published by the Free Software Foundation; either version 2.1 of the
// License, or (at your option) any later version.
//  
// This library is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//  
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA
// Questions? Contact Michael A. Heroux (maherou@sandia.gov) 
//
// changes Copyright 2014-2016 Gdansk University of Technology
// Questions regarding changes? Contact Piotr Dorozynski (piotr.dorozynski@pg.gda.pl)
// 
// ************************************************************************
//@HEADER

/////////////////////////////////////////////////////////////////////////

// Routine to compute an approximate solution to Ax = b where:

// A - known matrix stored as an HPC_Sparse_Matrix struct

// b - known right hand side vector

// x - On entry is initial guess, on exit new approximate solution

// max_iter - Maximum number of iterations to perform, even if
//            tolerance is not met.

// tolerance - Stop and assert convergence if norm of residual is <=
//             to tolerance.

// niters - On output, the number of iterations actually performed.

/////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <cstring>
#ifndef USING_MPI
#include <common/util.h>
#endif
#include <common/logger.h>
#include <common/mpi_init_pmem.h>
#include <mpi_one_sided_extension/mpi_win_pmem.h>
using std::cout;
using std::cerr;
using std::endl;
#include <cmath>
#include "mytimer.hpp"
#include "HPCCG.hpp"

typedef struct HPCCG_checkpoint_data_structure {
   double *p;
   double *x;
   double *r;
   int *k;
   char *flag;
} HPCCG_checkpoint_data;

#define TICK()  t0 = mytimer() // Use TICK and TOCK to time a code section
#define TOCK(t) t += mytimer() - t0
int HPCCG(HPC_Sparse_Matrix * A,
          const double * const b, double * const x,
          const char * const checkpoint_path, bool restart,
          const int max_iter, const double tolerance, int &niters, double & normr, double * times) {
#ifndef USING_MPI
   UNUSED(checkpoint_path);
   UNUSED(restart);
#endif

   MPI_Barrier(MPI_COMM_WORLD);
   double t_begin = mytimer();  // Start timing right away

   double t0, t1 = 0.0, t2 = 0.0, t3 = 0.0, t4 = 0.0;
#ifdef USING_MPI
   double t5 = 0.0;
#endif
   int nrow = A->local_nrow;
   int ncol = A->local_ncol;
   HPCCG_checkpoint_data data1, data2;
   HPCCG_checkpoint_data src, dst;
   int k;

#ifdef USING_MPI
   MPI_Win_pmem *src_win, *dst_win;
   MPI_Win_pmem win1, win2;
   MPI_Info info;
   const char flag_invalid = 0;
   const char flag_valid = 1;
   int available_iteration_number, available_iteration_number_on_all_processes;
   int iteration_available, iteration_available_on_all_processes;

   MPI_Win_pmem_set_root_path(checkpoint_path);

   MPI_Info_create(&info);
   MPI_Info_set(info, "pmem_is_pmem", "true");
   MPI_Info_set(info, "pmem_mode", "expand");
   MPI_Info_set(info, "pmem_dont_use_transactions", "true");
   MPI_Info_set(info, "pmem_name", "HPCCG1");
   MPI_Win_allocate_pmem((2 * nrow + ncol) * sizeof(double) + sizeof(int) + sizeof(char), sizeof(double), info, MPI_COMM_WORLD, &data1.p, &win1);
   data1.x = data1.p + ncol;
   data1.r = data1.x + nrow;
   data1.k = (int*) (data1.r + nrow);
   data1.flag = (char*) (data1.k + 1);
   MPI_Info_set(info, "pmem_name", "HPCCG2");
   MPI_Win_allocate_pmem((2 * nrow + ncol) * sizeof(double) + sizeof(int) + sizeof(char), sizeof(double), info, MPI_COMM_WORLD, &data2.p, &win2);
   data2.x = data2.p + ncol;
   data2.r = data2.x + nrow;
   data2.k = (int*) (data2.r + nrow);
   data2.flag = (char*) (data2.k + 1);
   MPI_Info_free(&info);
#else
   data1.p = new double[ncol]; // In parallel case, A is rectangular
   data1.x = new double[nrow];
   data1.r = new double[nrow];
   data2.p = new double[ncol]; // In parallel case, A is rectangular
   data2.x = new double[nrow];
   data2.r = new double[nrow];
#endif
   double * Ap = new double[nrow];

   normr = 0.0;
   double rtrans = 0.0;
   double oldrtrans;

#ifdef USING_MPI
   int rank; // Number of MPI processes, My process ID
   MPI_Comm_rank(MPI_COMM_WORLD, &rank);
#else
   int rank = 0; // Serial case (not using MPI)
#endif

   int print_freq = max_iter / 10;
   if (print_freq > 50) print_freq = 50;
   if (print_freq < 1)  print_freq = 1;

#ifdef USING_MPI
   // If application is restarted set appropriately iteration number and data.
   if (restart) {
      // Check if any process has only one valid data array.
      available_iteration_number = *data1.flag == flag_invalid ? *data2.k : *data2.flag == flag_invalid ? *data1.k : -1;
      MPI_Allreduce(&available_iteration_number, &available_iteration_number_on_all_processes, 1, MPI_INT, MPI_MAX, MPI_COMM_WORLD);
      if (available_iteration_number_on_all_processes == -1) { // All processes have both valid data arrays.
         k = *data1.k > *data2.k ? *data1.k : *data2.k;
      } else { // At least one of the processes have only one valid array.
         k = available_iteration_number_on_all_processes;
      }

      // Check if all processes have the proposed iteration.
      iteration_available = k == *data1.k || k == *data2.k ? 1 : 0;
      MPI_Allreduce(&iteration_available, &iteration_available_on_all_processes, 1, MPI_INT, MPI_MIN, MPI_COMM_WORLD);
      if (iteration_available_on_all_processes == 0) {
         mpi_log_error("One of processes doesn't have required iteration data.");
         MPI_Win_free_pmem(&win1);
         MPI_Win_free_pmem(&win2);
         delete[] Ap;
         MPI_Abort(MPI_COMM_WORLD, 1);
         MPI_Finalize_pmem();
         return 1;
      }

      // Set src and dst arrays based on iteration number.
      if (*data1.k == k) {
         src = data2;
         dst = data1;
         dst_win = &win1;
      } else {
         src = data1;
         dst = data2;
         dst_win = &win2;
      }
   } else {
      // Set data in both files as invalid.
      *data1.flag = flag_invalid;
      MPI_Win_fence_pmem_persist(0, win1);
      *data2.flag = flag_invalid;
      MPI_Win_fence_pmem_persist(0, win2);
      dst_win = &win1;
#endif

      k = 1;
      src = data2;
      dst = data1;
      memcpy(dst.x, x, nrow * sizeof(double));
      // p is of length ncols, copy x to p for sparse MV operation
      TICK(); waxpby(nrow, 1.0, dst.x, 0.0, dst.x, dst.p); TOCK(t2);
#ifdef USING_MPI
      TICK(); exchange_externals(A, dst.p, *dst_win); TOCK(t5);
#endif
      TICK(); HPC_sparsemv(A, dst.p, Ap); TOCK(t3);
      TICK(); waxpby(nrow, 1.0, b, -1.0, Ap, dst.r); TOCK(t2);
#ifdef USING_MPI
   }
#endif

   TICK(); ddot(nrow, dst.r, dst.r, &rtrans, t4); TOCK(t1);
   normr = sqrt(rtrans);

   if (rank == 0) cout << "Initial Residual = " << normr << endl;

   MPI_Barrier(MPI_COMM_WORLD);
   if (rank == 0) {
      cout << "Start/Restart time is " << mytimer() - t_begin << " seconds." << endl;
   }

   for (; k<max_iter && normr > tolerance; k++) {
      if (k == 1) {
         TICK(); waxpby(nrow, 1.0, dst.r, 0.0, dst.r, dst.p); TOCK(t2);
      } else if (!restart) {
         oldrtrans = rtrans;
         TICK(); ddot(nrow, dst.r, dst.r, &rtrans, t4); TOCK(t1);// 2*nrow ops
         double beta = rtrans / oldrtrans;
         TICK(); waxpby(nrow, 1.0, dst.r, beta, src.p, dst.p);  TOCK(t2);// 2*nrow ops
      }
      normr = sqrt(rtrans);
      if (rank == 0 && (k%print_freq == 0 || k + 1 == max_iter))
         cout << "Iteration = " << k << "   Residual = " << normr << endl;

      // Create checkpoint.
      if (restart) {
         restart = false;
      }
#ifdef USING_MPI
      else {
         // Set iteration number.
         *dst.k = k;
         MPI_Win_fence_pmem_persist(0, *dst_win);

         // Set flag indicating that destination is consistent.
         *dst.flag = flag_valid;
         MPI_Win_fence_pmem_persist(0, *dst_win);
      }
#endif
      // Swap source and destination.
      if (k % 2 == 1) {
         src = data1;
         dst = data2;
#ifdef USING_MPI
         src_win = &win1;
         dst_win = &win2;
#endif
      } else {
         src = data2;
         dst = data1;
#ifdef USING_MPI
         src_win = &win2;
         dst_win = &win1;
#endif      
      }

#ifdef USING_MPI
      // Set flag indicating that destination is being modified.
      *dst.flag = flag_invalid;
      MPI_Win_fence_pmem_persist(0, *dst_win);

      TICK(); exchange_externals(A, src.p, *src_win); TOCK(t5);
#endif
      TICK(); HPC_sparsemv(A, src.p, Ap); TOCK(t3); // 2*nnz ops
      double alpha = 0.0;
      TICK(); ddot(nrow, src.p, Ap, &alpha, t4); TOCK(t1); // 2*nrow ops
      alpha = rtrans / alpha;
      TICK(); waxpby(nrow, 1.0, src.x, alpha, src.p, dst.x);// 2*nrow ops
      waxpby(nrow, 1.0, src.r, -alpha, Ap, dst.r);  TOCK(t2);// 2*nrow ops
      niters = k;

      if (rank == 0) {
         cout << "Progress [%]: " << k * 100 / (max_iter - 1) << endl;
      }
   }

   memcpy(x, dst.x, nrow * sizeof(double));

   // Store times
   times[1] = t1; // ddot time
   times[2] = t2; // waxpby time
   times[3] = t3; // sparsemv time
   times[4] = t4; // AllReduce time
#ifdef USING_MPI
   times[5] = t5; // exchange boundary time
#endif

#ifdef USING_MPI
   MPI_Win_free_pmem(&win1);
   MPI_Win_free_pmem(&win2);
#else
   delete[] data1.p;
   delete[] data1.x;
   delete[] data1.r;
   delete[] data2.p;
   delete[] data2.x;
   delete[] data2.r;
#endif
   delete[] Ap;
   MPI_Barrier(MPI_COMM_WORLD);
   times[0] = mytimer() - t_begin;  // Total time. All done...
   return(0);
}
