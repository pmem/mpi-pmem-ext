/*
Copyright 2014-2016, Gdansk University of Technology

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the copyright holder nor the names of its contributors
      may be used to endorse or promote products derived from this software
      without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/



// based on https://blogs.fau.de/wittmann/2013/02/mpi-node-local-rank-determination/

#include "mpi_node_rank.h"

#include <stdlib.h>
#include <stdbool.h>
#include <sys/param.h>
#include <unistd.h>
#include <string.h>

#include <mpi.h>

#include "util.h"

int djb2_hash(char *data, unsigned len);

int get_mpi_node_rank(MPI_Comm communicator, int rank, int* node_rank) {

   int error, hostname_len;

   char* hostname = (char *) malloc(sizeof(char) * MAXHOSTNAMELEN);
   CHECK_MALLOC_ERROR(hostname);

   error = gethostname(hostname, MAXHOSTNAMELEN);
   CHECK_POSIX_ERROR(error);
   hostname[MAXHOSTNAMELEN - 1] = 0;
   hostname_len = strnlen(hostname, MAXHOSTNAMELEN) + 1;

   int hostname_hash = djb2_hash(hostname, hostname_len);

   MPI_Comm node_communicator;
   error = MPI_Comm_split(communicator, hostname_hash, rank, &node_communicator);
   CHECK_MPI_ERROR(error);

   error = MPI_Comm_rank(node_communicator, node_rank);
   CHECK_MPI_ERROR(error);

   return MPI_SUCCESS;
}

int djb2_hash(char* data, unsigned len) {
   int result = 5381;
   for (int i = 0; i < len; i++) {
      result = result * 33 + data[i];
   }
   return result;
}

