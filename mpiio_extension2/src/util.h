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


#ifndef __UTIL_H__
#define __UTIL_H__

#include <stdbool.h>

#define UNUSED(x) (void)(x)

extern int PMEM_IO_ERROR_MALLOC;
extern int PMEM_IO_ERROR_PTHREAD;
extern int PMEM_IO_ERROR_PMEM_MAP;
extern int PMEM_IO_ERROR_POSIX_UNKNOWN;
extern int PMEM_IO_ERROR_FILE_OPEN;
extern int PMEM_IO_ERROR_MKDIR;
extern int PMEM_IO_ERROR_RMDIR;
extern int PMEM_IO_ERROR_CACHE_MANAGER_THREAD;
extern int PMEM_IO_ERROR_UNSUPPORTED_OPERATION;
extern int PMEM_IO_ERROR_WRONG_MODE;

#define CHECK_MPI_ERROR(error_code) if (error_code != MPI_SUCCESS) return error_code;
#define CHECK_MALLOC_ERROR(pointer) if (pointer == NULL) return PMEM_IO_ERROR_MALLOC;
#define CHECK_PMEM_MAP_ERROR(pointer) if (pointer == NULL) return PMEM_IO_ERROR_PMEM_MAP;
#define CHECK_PTHREAD_ERROR(result) if (result != 0) return PMEM_IO_ERROR_PTHREAD;
#define CHECK_POSIX_ERROR(result) if (result != 0) return  PMEM_IO_ERROR_POSIX_UNKNOWN;
#define CHECK_FILE_OPEN_ERROR(result) if (result < 0) return PMEM_IO_ERROR_FILE_OPEN;
#define CHECK_MKDIR_ERROR(result) if (result != 0) return PMEM_IO_ERROR_MKDIR;
#define CHECK_RMDIR_ERROR(result) if (result != 0) return PMEM_IO_ERROR_RMDIR;

char* concat(char *s1, char *s2);
char* generate_timestamp();
char* generate_random_string(int length);
bool check_file_exists(char *filename);

#endif
