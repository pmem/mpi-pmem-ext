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


#ifndef __FILE_IO_PMEM_H__
#define __FILE_IO_PMEM_H__

#include <mpi.h>

#define MPI_PMEM_INFO_KEY_PMEM_PATH "pmem_path"
#define MPI_PMEM_INFO_KEY_MODE "pmem_io_mode"
#define MPI_PMEM_INFO_KEY_FAILURE_RECOVERY "failure_recovery"
#define MPI_PMEM_INFO_KEY_DO_FAILURE_RECOVERY "do_recovery"
#define MPI_PMEM_INFO_KEY_DO_FAILURE_RECOVERY_PATH "do_recovery_path"

#define PMEM_IO_DISTRIBUTED_CACHE 0
#define PMEM_IO_DISTRIBUTED_CACHE_STRING "0"
#define PMEM_IO_AWARE_FS 1
#define PMEM_IO_AWARE_FS_STRING "1"

int MPI_Init_thread_pmem(int *argc, char ***argv, int required, int *provided);
int MPI_File_open_pmem(MPI_Comm communicator, char* filename, int amode, MPI_Info info, MPI_File* fh);
int MPI_File_close_pmem(MPI_File* fh);
int MPI_File_read_at_pmem(MPI_File fh, MPI_Offset offset, void *buf, int count, MPI_Datatype datatype, MPI_Status *status);
int MPI_File_write_at_pmem(MPI_File fh, MPI_Offset offset, void *buf, int count, MPI_Datatype datatype, MPI_Status *status);
int MPI_File_read_at_all_pmem(MPI_File fh, MPI_Offset offset, void *buf, int count, MPI_Datatype datatype, MPI_Status *status);
int MPI_File_write_at_all_pmem(MPI_File fh, MPI_Offset offset, void *buf, int count, MPI_Datatype datatype, MPI_Status *status);
int MPI_File_get_size_pmem(MPI_File fh, MPI_Offset *size);
int MPI_File_set_size_pmem(MPI_File fh, MPI_Offset offset);
int MPI_File_sync_pmem(MPI_File fh);

#endif
