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


#include <stddef.h>
#include <mpi.h>
#include <pthread.h>

#include "pmem_datatypes.h"
#include "util.h"

static MPI_Datatype pmem_message_type = -1;

MPI_Datatype get_pmem_message_type() {
   if (pmem_message_type == -1) {
      int blocklengths[2] = { 1, 1 };
      MPI_Datatype types[2] = { MPI_UNSIGNED_LONG_LONG, MPI_UNSIGNED };
      MPI_Aint offsets[2];
      offsets[0] = offsetof(MPI_File_pmem_message, offset);
      offsets[1] = offsetof(MPI_File_pmem_message, size);

      int datatype;
      MPI_Type_create_struct(2, blocklengths, offsets, types, &datatype);
      int error = MPI_Type_commit(&datatype);
      CHECK_MPI_ERROR(error);
      pmem_message_type = datatype;
   }
   return pmem_message_type;
}

