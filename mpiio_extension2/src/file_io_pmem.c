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


#include <stdlib.h>

#include "util.h"
#include "logger.h"

#include "file_io_pmem.h"
#include "pmem_datatypes.h"

#include "file_io_distributed_cache.h"
#include "file_io_pmem_aware.h"

int parse_info_to_int(MPI_Info info, char* key);

int PMEM_IO_ERROR;
int PMEM_IO_ERROR_MALLOC;
int PMEM_IO_ERROR_PTHREAD;
int PMEM_IO_ERROR_PMEM_MAP;
int PMEM_IO_ERROR_POSIX_UNKNOWN;
int PMEM_IO_ERROR_FILE_OPEN;
int PMEM_IO_ERROR_MKDIR;
int PMEM_IO_ERROR_RMDIR;
int PMEM_IO_ERROR_CACHE_MANAGER_THREAD;
int PMEM_IO_ERROR_UNSUPPORTED_OPERATION;
int PMEM_IO_ERROR_WRONG_MODE;

int init_error_codes() {
   int result, i;
   int error_codes_count = 10;
   int *error_codes[] = {
      &PMEM_IO_ERROR_MALLOC,
      &PMEM_IO_ERROR_PTHREAD,
      &PMEM_IO_ERROR_PMEM_MAP,
      &PMEM_IO_ERROR_POSIX_UNKNOWN,
      &PMEM_IO_ERROR_FILE_OPEN,
      &PMEM_IO_ERROR_MKDIR,
      &PMEM_IO_ERROR_RMDIR,
      &PMEM_IO_ERROR_CACHE_MANAGER_THREAD,
      &PMEM_IO_ERROR_UNSUPPORTED_OPERATION,
      &PMEM_IO_ERROR_WRONG_MODE};
   char *error_strings[] = {
      "System memory allocation failed. Please check amount of free RAM memory.",
      "Unexpected pthread environment error. Does your environment fully support pthreads?",
      "Execution of pmem_map failed.",
      "Unexpected POSIX error.",
      "Cannot open file. Please check provided paths and process permissions.",
      "Cannot create directory. Please check provided paths and process permissions.",
      "Cannot remove directory. Is it empty?.",
      "Unexpected error in cache manager thread occured.",
      "This operation is unsupported.",
      "Cannot remove directory. Is it empty?."};

   // Add global error class
   result = MPI_Add_error_class(&PMEM_IO_ERROR);
   CHECK_MPI_ERROR(result);
   result = MPI_Add_error_string(PMEM_IO_ERROR, "General MPI IO PMEM error.");
   CHECK_MPI_ERROR(result);

   // Add individual error codes to class.
   for (i = 0; i < error_codes_count; i++) {
      result = MPI_Add_error_code(PMEM_IO_ERROR, error_codes[i]);
      CHECK_MPI_ERROR(result);
      result = MPI_Add_error_string(*error_codes[i], error_strings[i]);
      CHECK_MPI_ERROR(result);
      log_info("Successfully added error code for [%s]: %d", error_strings[i], *error_codes[i]);
   }

   return MPI_SUCCESS;
}

int MPI_Init_thread_pmem(int *argc, char ***argv, int required, int *provided) {
   int result = MPI_Init_thread(argc, argv, required, provided);
   CHECK_MPI_ERROR(result);
   return init_error_codes();
}

int MPI_File_open_pmem(MPI_Comm communicator, char* filename, int amode, MPI_Info info, MPI_File* fh) {

   // create MPI_File_pmem handler
   MPI_File_pmem* fh_pmem = (MPI_File_pmem*) malloc(sizeof(MPI_File_pmem));
   CHECK_MALLOC_ERROR(fh_pmem);
   fh_pmem->file = *fh;
   *fh = (MPI_File) fh_pmem;

   int mode = parse_info_to_int(info, MPI_PMEM_INFO_KEY_MODE);

   struct MPI_File_methods methods;
   switch (mode) {
      case PMEM_IO_DISTRIBUTED_CACHE:
         methods.file_open = &MPI_File_open_pmem_distributed;
         methods.file_close = &MPI_File_close_pmem_distributed;
         methods.file_read_at = &MPI_File_read_at_pmem_distributed;
         methods.file_write_at = &MPI_File_write_at_pmem_distributed;
         methods.file_read_at_all = &MPI_File_read_at_pmem_distributed;
         methods.file_write_at_all = &MPI_File_write_at_pmem_distributed;
         methods.file_set_size = &MPI_File_set_size_pmem_distributed;
         methods.file_sync = &MPI_File_sync_pmem_distributed;
         break;
      case PMEM_IO_AWARE_FS:
         methods.file_open = &MPI_File_open_pmem_aware;
         methods.file_close = &MPI_File_close_pmem_aware;
         methods.file_read_at = &MPI_File_read_at_pmem_aware;
         methods.file_write_at = &MPI_File_write_at_pmem_aware;
         methods.file_read_at_all = &MPI_File_read_at_pmem_aware;
         methods.file_write_at_all = &MPI_File_write_at_pmem_aware;
         methods.file_set_size = &MPI_File_set_size_pmem_aware;
         methods.file_sync = &MPI_File_sync_pmem_aware;
         break;
      default:
         return PMEM_IO_ERROR_WRONG_MODE;
   }

   int error = (*methods.file_open)(communicator, filename, amode, info, fh_pmem);
   MPI_File_pmem* file = (MPI_File_pmem*) *fh;
   file->methods = methods;
   return error;
}

int MPI_File_close_pmem(MPI_File* fh) {
   MPI_File_pmem* fh_pmem = (MPI_File_pmem*) *fh;
   return (*fh_pmem->methods.file_close)(fh_pmem);
}

int MPI_File_read_at_pmem(MPI_File fh, MPI_Offset offset, void *buf, int count, MPI_Datatype datatype, MPI_Status *status) {
   MPI_File_pmem* fh_pmem = (MPI_File_pmem*) fh;
   return (*fh_pmem->methods.file_read_at)(fh_pmem, offset, buf, count, datatype, status);
}

int MPI_File_write_at_pmem(MPI_File fh, MPI_Offset offset, void *buf, int count, MPI_Datatype datatype, MPI_Status *status) {
   MPI_File_pmem* fh_pmem = (MPI_File_pmem*) fh;
   return (*fh_pmem->methods.file_write_at)(fh_pmem, offset, buf, count, datatype, status);
}

int MPI_File_read_at_all_pmem(MPI_File fh, MPI_Offset offset, void *buf, int count, MPI_Datatype datatype, MPI_Status *status) {
   MPI_File_pmem* fh_pmem = (MPI_File_pmem*) fh;
   return (*fh_pmem->methods.file_read_at)(fh_pmem, offset, buf, count, datatype, status);
}

int MPI_File_write_at_all_pmem(MPI_File fh, MPI_Offset offset, void *buf, int count, MPI_Datatype datatype, MPI_Status *status) {
   MPI_File_pmem* fh_pmem = (MPI_File_pmem*) fh;
   return (*fh_pmem->methods.file_write_at)(fh_pmem, offset, buf, count, datatype, status);
}

int MPI_File_get_size_pmem(MPI_File fh, MPI_Offset *size) {
   MPI_File_pmem* fh_pmem = (MPI_File_pmem*) fh;
   *size = fh_pmem->file_size;
   return MPI_SUCCESS;
}

int MPI_File_set_size_pmem(MPI_File fh, MPI_Offset offset) {
   MPI_File_pmem* fh_pmem = (MPI_File_pmem*) fh;
   return (*fh_pmem->methods.file_set_size)(fh_pmem, offset);
}

int MPI_File_sync_pmem(MPI_File fh) {
   MPI_File_pmem* fh_pmem = (MPI_File_pmem*) fh;
   return (*fh_pmem->methods.file_sync)(fh_pmem);
}

int parse_info_to_int(MPI_Info info, char* key) {

   int flag, length;

   MPI_Info_get_valuelen(info, key, &length, &flag);

   if (!flag) {
      return -1;
   }

   char* value = (char*) malloc(length + 1);
   MPI_Info_get(info, key, length + 1, value, &flag);
   int value_int = atoi(value);
   free(value);

   return value_int;
}
