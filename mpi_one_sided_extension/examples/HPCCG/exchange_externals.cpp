
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

#ifdef USING_MPI  // Compile this routine only if running in parallel
#include <iostream>
#include <mpi_one_sided_extension/mpi_win_pmem.h>
using std::cerr;
using std::endl;
#include "exchange_externals.hpp"
#undef DEBUG
void exchange_externals(HPC_Sparse_Matrix * A, const double *x, MPI_Win_pmem win) {
   int i;

   // Extract Matrix pieces

   int num_neighbors = A->num_send_neighbors;
   int * send_length = A->send_length;
   int * neighbors = A->neighbors;
   int * send_offset = A->send_offset;
   double * send_buffer = A->send_buffer;
   int total_to_be_sent = A->total_to_be_sent;
   int * elements_to_send = A->elements_to_send;

   int size, rank; // Number of MPI processes, My process ID
   MPI_Comm_size(MPI_COMM_WORLD, &size);
   MPI_Comm_rank(MPI_COMM_WORLD, &rank);

   //
   // Fill up send buffer
   //

   for (i = 0; i < total_to_be_sent; i++) send_buffer[i] = x[elements_to_send[i]];

   //
   // Send to each neighbor
   //

   MPI_Win_fence_pmem(0, win);

   for (i = 0; i < num_neighbors; i++) {
      int n_send = send_length[i];
      MPI_Put_pmem(send_buffer, n_send, MPI_DOUBLE, neighbors[i], send_offset[i], n_send, MPI_DOUBLE, win);
      send_buffer += n_send;
   }

   MPI_Win_fence_pmem(0, win);
}
#endif // USING_MPI
