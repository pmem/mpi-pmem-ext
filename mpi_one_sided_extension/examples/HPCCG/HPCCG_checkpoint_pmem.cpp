
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
#include <mpi_one_sided_extension/mpi_win_pmem.h>
using std::cout;
using std::cerr;
using std::endl;
#include <cmath>
#include "mytimer.hpp"
#include "HPCCG.hpp"

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
   int *k;

#ifdef USING_MPI
   double * p;
   double * x_local;
   double * r;
   MPI_Win_pmem win;
   MPI_Info info;

   MPI_Win_pmem_set_root_path(checkpoint_path);

   MPI_Info_create(&info);
   MPI_Info_set(info, "pmem_is_pmem", "true");
   MPI_Info_set(info, "pmem_allocate_in_ram", "true");
   if (restart) {
      MPI_Info_set(info, "pmem_mode", "checkpoint");
   } else {
      MPI_Info_set(info, "pmem_mode", "expand");
   }
   MPI_Info_set(info, "pmem_name", "HPCCG");
   MPI_Win_allocate_pmem((2 * nrow + ncol) * sizeof(double) + sizeof(int), sizeof(double), info, MPI_COMM_WORLD, &p, &win);
   MPI_Info_free(&info);
   x_local = p + ncol;
   r = p + ncol + nrow;
   k = (int*) (p + ncol + 2 * nrow);
#else
   double * p = new double[ncol]; // In parallel case, A is rectangular
   double * x_local = new double[nrow];
   double * r = new double[nrow];
   k = new int;
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
   // If application is not restarted make initial calculations.
   if (!restart) {
#endif
      *k = 1;
      memcpy(x_local, x, nrow * sizeof(double));
      // p is of length ncols, copy x to p for sparse MV operation
      TICK(); waxpby(nrow, 1.0, x_local, 0.0, x_local, p); TOCK(t2);
#ifdef USING_MPI
      TICK(); exchange_externals(A, p, win); TOCK(t5);
#endif
      TICK(); HPC_sparsemv(A, p, Ap); TOCK(t3);
      TICK(); waxpby(nrow, 1.0, b, -1.0, Ap, r); TOCK(t2);
#ifdef USING_MPI
   }
#endif

   TICK(); ddot(nrow, r, r, &rtrans, t4); TOCK(t1);
   normr = sqrt(rtrans);

   if (rank == 0) cout << "Initial Residual = " << normr << endl;

   MPI_Barrier(MPI_COMM_WORLD);
   if (rank == 0) {
      cout << "Start/Restart time is " << mytimer() - t_begin << " seconds." << endl;
   }

   for (; *k<max_iter && normr > tolerance; (*k)++) {
      if (*k == 1) {
         TICK(); waxpby(nrow, 1.0, r, 0.0, r, p); TOCK(t2);
      } else if (!restart) {
         oldrtrans = rtrans;
         TICK(); ddot(nrow, r, r, &rtrans, t4); TOCK(t1);// 2*nrow ops
         double beta = rtrans / oldrtrans;
         TICK(); waxpby(nrow, 1.0, r, beta, p, p);  TOCK(t2);// 2*nrow ops
      }
      normr = sqrt(rtrans);
      if (rank == 0 && (*k%print_freq == 0 || *k + 1 == max_iter))
         cout << "Iteration = " << *k << "   Residual = " << normr << endl;

      // Create checkpoint.
      if (restart) {
         restart = false;
      }
#ifdef USING_MPI
      else {
         MPI_Win_fence_pmem_persist(0, win);
      }

      TICK(); exchange_externals(A, p, win); TOCK(t5);
#endif
      TICK(); HPC_sparsemv(A, p, Ap); TOCK(t3); // 2*nnz ops
      double alpha = 0.0;
      TICK(); ddot(nrow, p, Ap, &alpha, t4); TOCK(t1); // 2*nrow ops
      alpha = rtrans / alpha;
      TICK(); waxpby(nrow, 1.0, x_local, alpha, p, x_local);// 2*nrow ops
      waxpby(nrow, 1.0, r, -alpha, Ap, r);  TOCK(t2);// 2*nrow ops
      niters = *k;

      if (rank == 0) {
         cout << "Progress [%]: " << *k * 100 / (max_iter - 1) << endl;
      }
   }

   memcpy(x, x_local, nrow * sizeof(double));

   // Store times
   times[1] = t1; // ddot time
   times[2] = t2; // waxpby time
   times[3] = t3; // sparsemv time
   times[4] = t4; // AllReduce time
#ifdef USING_MPI
   times[5] = t5; // exchange boundary time
#endif

#ifdef USING_MPI
   MPI_Win_free_pmem(&win);
#else
   delete[] p;
   delete[] r;
   delete[] x_local;
   delete k;
#endif
   delete[] Ap;
   MPI_Barrier(MPI_COMM_WORLD);
   times[0] = mytimer() - t_begin;  // Total time. All done...
   return(0);
}
