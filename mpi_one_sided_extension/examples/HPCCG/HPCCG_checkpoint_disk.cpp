
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
#include <cstdio>
#include <fcntl.h>
#include <unistd.h>
#include <common/logger.h>
#ifdef USING_MPI
#include <common/mpi_init_pmem.h>
#include <mpi_one_sided_extension/mpi_win_pmem.h>
#endif
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

#ifdef USING_MPI
   char file_name[MPI_PMEM_MAX_NAME];
#else
   char file_name[256];
#endif

   MPI_Barrier(MPI_COMM_WORLD);
   double t_begin = mytimer();  // Start timing right away

   double t0, t1 = 0.0, t2 = 0.0, t3 = 0.0, t4 = 0.0;
#ifdef USING_MPI
   double t5 = 0.0;
#endif
   int nrow = A->local_nrow;
   int ncol = A->local_ncol;
   int k;
   const char flag_invalid = 0;
   const char flag_valid = 1;
   FILE *file;
   size_t fread_result;
   int file_descriptor;
   char hpccg1_flag, hpccg2_flag;
   int hpccg1_iteration, hpccg2_iteration;
   int available_iteration_number;
#ifdef USING_MPI
   int available_iteration_number_on_all_processes;
   int iteration_available, iteration_available_on_all_processes;
#endif

   double * r = new double[nrow];
#ifdef USING_MPI
   double * p;
   MPI_Win_pmem p_win;
   MPI_Win_allocate_pmem(ncol * sizeof(double), sizeof(double), MPI_INFO_NULL, MPI_COMM_WORLD, &p, &p_win);
#else
   double * p = new double[ncol]; // In parallel case, A is rectangular
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

   // If application is restarted set appropriately iteration number and data.
   if (restart) {
      // Read flags and iterations' numbers in checkpoints.
      sprintf(file_name, "%s/HPCCG1", checkpoint_path);
      file = fopen(file_name, "rb");
      if (file == NULL) {
#ifdef USING_MPI
         mpi_log_error("Unable to open checkpoint file '%s'.", file_name);
         MPI_Win_free_pmem(&p_win);
#else
         log_error("Unable to open checkpoint file '%s'.", file_name);
         delete[] p;
#endif
         delete[] Ap;
         delete[] r;
#ifdef USING_MPI
         MPI_Abort(MPI_COMM_WORLD, 1);
         MPI_Finalize_pmem();
#endif
         return 1;
      }
      fread_result = fread(&hpccg1_flag, sizeof(char), 1, file);
      fread_result = fread(&hpccg1_iteration, sizeof(int), 1, file);
      fclose(file);
      sprintf(file_name, "%s/HPCCG2", checkpoint_path);
      file = fopen(file_name, "rb");
      if (file == NULL) {
#ifdef USING_MPI
         mpi_log_error("Unable to open checkpoint file '%s'.", file_name);
         MPI_Win_free_pmem(&p_win);
#else
         log_error("Unable to open checkpoint file '%s'.", file_name);
         delete[] p;
#endif
         delete[] Ap;
         delete[] r;
#ifdef USING_MPI
         MPI_Abort(MPI_COMM_WORLD, 1);
         MPI_Finalize_pmem();
#endif
         return 1;
      }
      fread_result = fread(&hpccg2_flag, sizeof(char), 1, file);
      fread_result = fread(&hpccg2_iteration, sizeof(int), 1, file);
      fclose(file);

#ifdef USING_MPI
      // Check if any process has only one valid data array.
      available_iteration_number = hpccg1_flag == 0 ? hpccg2_iteration : hpccg2_flag == 0 ? hpccg1_iteration : -1;
      MPI_Allreduce(&available_iteration_number, &available_iteration_number_on_all_processes, 1, MPI_INT, MPI_MAX, MPI_COMM_WORLD);
      if (available_iteration_number_on_all_processes == -1) { // All processes have both valid data arrays.
         k = hpccg1_iteration > hpccg2_iteration ? hpccg1_iteration : hpccg2_iteration;
      } else { // At least one of the processes have only one valid array.
         k = available_iteration_number_on_all_processes;
      }

      // Check if all processes have the proposed iteration.
      iteration_available = k == hpccg1_iteration || k == hpccg2_iteration ? 1 : 0;
      MPI_Allreduce(&iteration_available, &iteration_available_on_all_processes, 1, MPI_INT, MPI_MIN, MPI_COMM_WORLD);
      if (iteration_available_on_all_processes == 0) {
         mpi_log_error("One of processes doesn't have required iteration data.");
         MPI_Win_free_pmem(&p_win);
         delete[] Ap;
         delete[] r;
         MPI_Abort(MPI_COMM_WORLD, 1);
         MPI_Finalize_pmem();
         return 1;
      }
#else
      available_iteration_number = hpccg1_flag == 0 ? hpccg2_iteration : hpccg2_flag == 0 ? hpccg1_iteration : -1;
      if (available_iteration_number == -1) { // All processes have both valid data arrays.
         k = hpccg1_iteration > hpccg2_iteration ? hpccg1_iteration : hpccg2_iteration;
      } else { // At least one of the processes have only one valid array.
         k = available_iteration_number;
      }
#endif

      // Copy data from checkpoint.
      if (k == hpccg1_iteration) {
         sprintf(file_name, "%s/HPCCG1", checkpoint_path);
      } else {
         sprintf(file_name, "%s/HPCCG2", checkpoint_path);
      }
      file = fopen(file_name, "rb");
      if (file == NULL) {
#ifdef USING_MPI
         mpi_log_error("Unable to open checkpoint file '%s'.", file_name);
         MPI_Win_free_pmem(&p_win);
#else
         log_error("Unable to open checkpoint file '%s'.", file_name);
         delete[] p;
#endif
         delete[] Ap;
         delete[] r;
#ifdef USING_MPI
         MPI_Abort(MPI_COMM_WORLD, 1);
         MPI_Finalize_pmem();
#endif
         return 1;
      }
      fseek(file, sizeof(char) + sizeof(int), SEEK_SET);
      fread_result = fread(p, sizeof(double), nrow, file);
      fread_result = fread(x, sizeof(double), nrow, file);
      fread_result = fread(r, sizeof(double), nrow, file);
      fclose(file);
   } else {
      k = 1;
      // Create checkpoint files and set flag indicating that they are invalid.
      sprintf(file_name, "%s/HPCCG1", checkpoint_path);
      file = fopen(file_name, "wb");
      if (file == NULL) {
#ifdef USING_MPI
         mpi_log_error("Unable to open checkpoint file '%s'.", file_name);
         MPI_Win_free_pmem(&p_win);
#else
         log_error("Unable to open checkpoint file '%s'.", file_name);
         delete[] p;
#endif
         delete[] Ap;
         delete[] r;
#ifdef USING_MPI
         MPI_Abort(MPI_COMM_WORLD, 1);
         MPI_Finalize_pmem();
#endif
         return 1;
      }
      fwrite(&flag_invalid, sizeof(char), 1, file);
      fflush(file);
      fsync(fileno(file));
      fclose(file);
      sprintf(file_name, "%s/HPCCG2", checkpoint_path);
      file = fopen(file_name, "wb");
      if (file == NULL) {
#ifdef USING_MPI
         mpi_log_error("Unable to open checkpoint file '%s'.", file_name);
         MPI_Win_free_pmem(&p_win);
#else
         log_error("Unable to open checkpoint file '%s'.", file_name);
         delete[] p;
#endif
         delete[] Ap;
         delete[] r;
#ifdef USING_MPI
         MPI_Abort(MPI_COMM_WORLD, 1);
         MPI_Finalize_pmem();
#endif
         return 1;
      }
      fwrite(&flag_invalid, sizeof(char), 1, file);
      fflush(file);
      fsync(fileno(file));
      fclose(file);
      // Sync also directory containing checkpoints.
      file_descriptor = open(mpi_pmem_root_path, O_RDWR);
      fsync(file_descriptor);
      close(file_descriptor);

      // p is of length ncols, copy x to p for sparse MV operation
      TICK(); waxpby(nrow, 1.0, x, 0.0, x, p); TOCK(t2);
#ifdef USING_MPI
      TICK(); exchange_externals(A, p, p_win); TOCK(t5);
#endif
      TICK(); HPC_sparsemv(A, p, Ap); TOCK(t3);
      TICK(); waxpby(nrow, 1.0, b, -1.0, Ap, r); TOCK(t2);
   }

   TICK(); ddot(nrow, r, r, &rtrans, t4); TOCK(t1);
   normr = sqrt(rtrans);

   if (rank == 0) cout << "Initial Residual = " << normr << endl;

   MPI_Barrier(MPI_COMM_WORLD);
   if (rank == 0) {
      cout << "Start/Restart time is " << mytimer() - t_begin << " seconds." << endl;
   }

   for (; k<max_iter && normr > tolerance; k++) {
      if (k == 1) {
         TICK(); waxpby(nrow, 1.0, r, 0.0, r, p); TOCK(t2);
      } else if (!restart) {
         oldrtrans = rtrans;
         TICK(); ddot(nrow, r, r, &rtrans, t4); TOCK(t1);// 2*nrow ops
         double beta = rtrans / oldrtrans;
         TICK(); waxpby(nrow, 1.0, r, beta, p, p);  TOCK(t2);// 2*nrow ops
      }
      normr = sqrt(rtrans);
      if (rank == 0 && (k%print_freq == 0 || k + 1 == max_iter))
         cout << "Iteration = " << k << "   Residual = " << normr << endl;

      // Create checkpoint
      if (restart) {
         restart = false;
      } else {
         if (k % 2 == 1) {
            sprintf(file_name, "%s/HPCCG1", checkpoint_path);
         } else {
            sprintf(file_name, "%s/HPCCG2", checkpoint_path);
         }
         file = fopen(file_name, "rb+");
         if (file == NULL) {
            mpi_log_error("Unable to open checkpoint file '%s'.", file_name);
#ifdef USING_MPI
            MPI_Win_free_pmem(&p_win);
#else
            delete[] p;
#endif
            delete[] Ap;
            delete[] r;
#ifdef USING_MPI
            MPI_Abort(MPI_COMM_WORLD, 1);
            MPI_Finalize_pmem();
#endif
            return 1;
         }
         fwrite(&flag_invalid, sizeof(char), 1, file);
         fflush(file);
         fsync(fileno(file));
#ifdef USING_MPI
         MPI_Barrier(MPI_COMM_WORLD); // Barrier to make sure that all processes have invalidated new checkpoint before any one becomes valid.
#endif
         fwrite(&k, sizeof(int), 1, file);
         fwrite(p, sizeof(double), nrow, file);
         fwrite(x, sizeof(double), nrow, file);
         fwrite(r, sizeof(double), nrow, file);
         fflush(file);
         fsync(fileno(file));
         rewind(file);
         fwrite(&flag_valid, sizeof(char), 1, file);
         fflush(file);
         fsync(fileno(file));
         fclose(file);
      }

#ifdef USING_MPI
      TICK(); exchange_externals(A, p, p_win); TOCK(t5);
#endif
      TICK(); HPC_sparsemv(A, p, Ap); TOCK(t3); // 2*nnz ops
      double alpha = 0.0;
      TICK(); ddot(nrow, p, Ap, &alpha, t4); TOCK(t1); // 2*nrow ops
      alpha = rtrans / alpha;
      TICK(); waxpby(nrow, 1.0, x, alpha, p, x);// 2*nrow ops
      waxpby(nrow, 1.0, r, -alpha, Ap, r);  TOCK(t2);// 2*nrow ops
      niters = k;

      if (rank == 0) {
         cout << "Progress [%]: " << k * 100 / (max_iter - 1) << endl;
      }
   }

   // Store times
   times[1] = t1; // ddot time
   times[2] = t2; // waxpby time
   times[3] = t3; // sparsemv time
   times[4] = t4; // AllReduce time
#ifdef USING_MPI
   times[5] = t5; // exchange boundary time
#endif

#ifdef USING_MPI
   MPI_Win_free_pmem(&p_win);
#else
   delete[] p;
#endif
   delete[] Ap;
   delete[] r;
   MPI_Barrier(MPI_COMM_WORLD);
   times[0] = mytimer() - t_begin;  // Total time. All done...
   return(0);
}
