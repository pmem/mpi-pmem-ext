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


#include "file_io_pmem_aware.h"

#include <mpi.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <libpmem.h>

#include "util.h"
#include "logger.h"

#include "messages.h"
#include "pmem_datatypes.h"


int MPI_File_open_pmem_aware(MPI_Comm communicator, char* filename, int amode, MPI_Info info, MPI_File_pmem* fh_pmem) {

   UNUSED(communicator);
   UNUSED(amode);
   UNUSED(info);

   int fd = open(filename, O_CREAT|O_RDWR, 0666);
   CHECK_FILE_OPEN_ERROR(fd);

   fh_pmem->file_size = lseek(fd, 0L, SEEK_END);

   fh_pmem->mapped_file = pmem_map(fd);
   CHECK_PMEM_MAP_ERROR(fh_pmem->mapped_file);
   fh_pmem->is_pmem = pmem_is_pmem(fh_pmem->mapped_file, fh_pmem->file_size);

   close(fd);

   return 0;
}

int MPI_File_close_pmem_aware(MPI_File_pmem* fh_pmem) {
   pmem_unmap(fh_pmem->mapped_file, fh_pmem->file_size);
   return 0;
}

int MPI_File_read_at_pmem_aware(MPI_File_pmem* fh_pmem, MPI_Offset offset, void *buf, int count, MPI_Datatype datatype, MPI_Status *status) {
   UNUSED(status);
   int size_of_type;
   int error = MPI_Type_size(datatype, &size_of_type);
   CHECK_MPI_ERROR(error);
   memcpy(buf, fh_pmem->mapped_file + offset, size_of_type * count);
   return 0;
}

int MPI_File_write_at_pmem_aware(MPI_File_pmem* fh_pmem, MPI_Offset offset, void *buf, int count, MPI_Datatype datatype, MPI_Status *status) {
   UNUSED(status);
   int size_of_type;
   int error = MPI_Type_size(datatype, &size_of_type);
   CHECK_MPI_ERROR(error);
   if (fh_pmem->is_pmem) {
      pmem_memcpy_persist(fh_pmem->mapped_file + offset, buf, size_of_type * count);
   } else {
      memcpy(fh_pmem->mapped_file + offset, buf, size_of_type * count);
      pmem_msync(fh_pmem->mapped_file + offset, size_of_type * count);
   }
   return 0;
}

int MPI_File_set_size_pmem_aware(MPI_File_pmem* fh_pmem, MPI_Offset offset) {
   UNUSED(offset);
   UNUSED(fh_pmem);
   return PMEM_IO_ERROR_UNSUPPORTED_OPERATION;
}

int MPI_File_sync_pmem_aware(MPI_File_pmem* fh_pmem) {
   UNUSED(fh_pmem);
   // file is always synchronized because of pmem_memcpy_persist/pmem_msync calls after each update
   return 0;
}
