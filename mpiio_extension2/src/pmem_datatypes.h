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


#ifndef __PMEM_DATATYPES__
#define __PMEM_DATATYPES__

#include <mpi.h>
#include <pthread.h>
#include <sys/time.h>
#include <stdbool.h>

typedef struct MPI_File_pmem_structure MPI_File_pmem;
typedef struct MPI_File_pmem_wrapper MPI_File_pmem_wrapper;
typedef struct MPI_File_pmem_message MPI_File_pmem_message;

struct MPI_File_methods {
   int (*file_open)(MPI_Comm, char*, int, MPI_Info, MPI_File_pmem*);
   int (*file_close)(MPI_File_pmem*);
   int (*file_read_at)(MPI_File_pmem*, MPI_Offset, void*, int, MPI_Datatype, MPI_Status*);
   int (*file_write_at)(MPI_File_pmem*, MPI_Offset, void*, int, MPI_Datatype, MPI_Status*);
   int (*file_read_at_all)(MPI_File_pmem*, MPI_Offset, void*, int, MPI_Datatype, MPI_Status*);
   int (*file_write_at_all)(MPI_File_pmem*, MPI_Offset, void*, int, MPI_Datatype, MPI_Status*);
   int (*file_set_size)(MPI_File_pmem*, MPI_Offset);
   int (*file_sync)(MPI_File_pmem*);
};

struct MPI_File_pmem_structure {

   // general
   char* filename;
   MPI_File file;
   MPI_Offset file_size;

   // synchronization
   pthread_t* cache_manager_thread;
   MPI_Comm communicator;

   // distributed cache
   int number_of_nodes;
   unsigned long long cache_size;
   int* cache_managers;
   unsigned long long* cache_offsets;
   char* cache_path;

   // pmem aware
   char* mapped_file;
   int is_pmem;

   // failure recovery
   bool is_failure_recovery_enabled;
   bool do_recovery;

   struct MPI_File_methods methods;
};

struct MPI_File_pmem_message {
   unsigned long long offset;
   int size;
};

#define PMEM_MSG_TYPE get_pmem_message_type()
MPI_Datatype get_pmem_message_type();

#endif
