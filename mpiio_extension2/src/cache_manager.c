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


#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <limits.h>

#include <libpmem.h>

#include "logger.h"
#include "pmem_datatypes.h"
#include "messages.h"
#include "util.h"
#include "failure_recovery.h"

#define CACHE_FILE_NAME "cache"

// private functions
int cache_manager_thread_function(void *attributes);
void* cache_manager_thread_function_wrapper(void *attributes);
char* init_cache(unsigned long long cache_size, char* cache_path);
void deinit_cache(char* cache, char* cache_path, unsigned long long cache_size);
int synchronize_cache_with_fs(char* cache, MPI_File_pmem* fh, int cache_manager_id);

// private datatypes
struct init_thread_attr {
   int cache_manager_id;
   MPI_File_pmem* fh;
};

int init_cache_manager(int cache_manager_id, MPI_File_pmem* fh) {

   struct init_thread_attr* attr = (struct init_thread_attr*) malloc(sizeof(struct init_thread_attr));
   CHECK_MALLOC_ERROR(attr);

   attr->cache_manager_id = cache_manager_id;
   attr->fh = fh;

   fh->cache_manager_thread = (pthread_t*) malloc(sizeof(pthread_t));
   CHECK_MALLOC_ERROR(fh->cache_manager_thread);

   int error = pthread_create(fh->cache_manager_thread, NULL, cache_manager_thread_function_wrapper, (void*) attr);
   CHECK_PTHREAD_ERROR(error);
   return MPI_SUCCESS;
}

int deinit_cache_manager(MPI_File_pmem* fh) {
   int error = pthread_join(*fh->cache_manager_thread, NULL);
   CHECK_PTHREAD_ERROR(error);
   return MPI_SUCCESS;
}

void* cache_manager_thread_function_wrapper(void *attributes) {
   int error = cache_manager_thread_function(attributes);
   if (error != MPI_SUCCESS) {
      MPI_Abort(MPI_COMM_WORLD, PMEM_IO_ERROR_CACHE_MANAGER_THREAD);
   }
   return NULL;
}

int cache_manager_thread_function(void *attributes) {

   int error, count;
   struct init_thread_attr* attr = (struct init_thread_attr*) attributes;
   MPI_File_pmem* fh = attr->fh;
   MPI_File_pmem_message msg;
   MPI_Status status;
   int manager_id = attr->cache_manager_id;
   unsigned long long manager_offset = fh->cache_offsets[manager_id];

   char* cache = init_cache(fh->cache_size, fh->cache_path);
   bool is_pmem = pmem_is_pmem(cache, fh->cache_size);
   if (is_pmem) {
      log_info("Cache manager [%d]: pmem device found, optimized code will be executed.", manager_id);
   } else {
      log_info("Cache manager [%d]: pmem path is not persistent memory, cannot execute optimized code.", manager_id);
   }

   long long offset = 0;
   long long left_to_read = fh->cache_size;
   while (left_to_read > INT_MAX) {
      error = MPI_File_read_at(fh->file, manager_offset + offset, cache + offset, INT_MAX, MPI_BYTE, &status);
      CHECK_MPI_ERROR(error);
      left_to_read -= INT_MAX;
      offset += INT_MAX;
   }
   error = MPI_File_read_at(fh->file, manager_offset + offset, cache + offset, left_to_read, MPI_BYTE, &status);
   CHECK_MPI_ERROR(error);

   log_info("Cache manager [%d]: listening. Cache block from %d to %d", manager_id, manager_offset, manager_offset + fh->cache_size);

   while (1) {

      error = MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, fh->communicator, &status);
      CHECK_MPI_ERROR(error);
      log_debug("Cache manager [%d]: received request %d from %d", manager_id, status.MPI_TAG, status.MPI_SOURCE);

      switch (status.MPI_TAG) {

         case PMEM_TAG_READ_AT_REQUEST_TAG: {
            error = MPI_Recv(&msg, 1, PMEM_MSG_TYPE, MPI_ANY_SOURCE, PMEM_TAG_READ_AT_REQUEST_TAG, fh->communicator, &status);
            CHECK_MPI_ERROR(error);
            error = MPI_Send(cache + msg.offset - manager_offset, msg.size, MPI_BYTE, status.MPI_SOURCE, PMEM_TAG_READ_AT_RESPONSE_TAG,
                  fh->communicator);
            CHECK_MPI_ERROR(error);
            break;
         }

         case PMEM_TAG_WRITE_AT_REQUEST_TAG: {
            error = MPI_Get_count(&status, MPI_BYTE, &count);
            CHECK_MPI_ERROR(error);
            char* buf = (char*) malloc(count); // possible opt to consider: preallocating memory
            MPI_File_pmem_message* msg = (MPI_File_pmem_message*) buf;
            error = MPI_Recv(buf, count, MPI_BYTE, MPI_ANY_SOURCE, PMEM_TAG_WRITE_AT_REQUEST_TAG, fh->communicator, &status);
            CHECK_MPI_ERROR(error);

            if (is_pmem) {
               pmem_memcpy_persist(cache + msg->offset - manager_offset, buf + sizeof(MPI_File_pmem_message), msg->size);
            } else {
               memcpy(cache + msg->offset - manager_offset, buf + sizeof(MPI_File_pmem_message),
                     msg->size);
               pmem_msync(cache + msg->offset - manager_offset, msg->size);
            }
            free(buf);
            break;
         }

         case PMEM_TAG_SYNC: {
            error = synchronize_cache_with_fs(cache, fh, manager_id);
            CHECK_MPI_ERROR(error);
            error = MPI_Recv(cache, 0, MPI_BYTE, MPI_ANY_SOURCE, PMEM_TAG_SYNC, fh->communicator, &status);
            CHECK_MPI_ERROR(error);
            break;
         }

         case PMEM_TAG_SHUTDOWN: {
            error = synchronize_cache_with_fs(cache, fh, manager_id);
            CHECK_MPI_ERROR(error);
            error = MPI_Recv(cache, 0, MPI_BYTE, MPI_ANY_SOURCE, PMEM_TAG_SHUTDOWN, fh->communicator, &status);
            CHECK_MPI_ERROR(error);
            log_info("Closing cache manager %d", manager_id);
            free(attributes);
            deinit_cache(cache, fh->cache_path, fh->cache_size);
            return MPI_SUCCESS;
         }
      }
   }

   return MPI_SUCCESS;
}

char* init_cache(unsigned long long cache_size, char* cache_path) {

   char* final_cache_file_name = concat(cache_path, CACHE_FILE_NAME);

   int fd = open(final_cache_file_name, O_CREAT | O_RDWR, 0666);
   if (fd < 0) {
      return 0;
   }

   if (posix_fallocate(fd, 0, cache_size) != 0) {
      return 0;
   }

   char* cache = pmem_map(fd);
   close(fd);
   return cache;
}

void deinit_cache(char* cache, char* cache_path, unsigned long long cache_size) {
   pmem_unmap(cache, cache_size);
   remove(concat(cache_path, CACHE_FILE_NAME));
}

int synchronize_cache_with_fs(char* cache, MPI_File_pmem* fh, int manager_id) {

   long long offset = 0;
   long long manager_offset = fh->cache_offsets[manager_id];
   long long left_to_write = fh->cache_size;
   MPI_Status status;
   int error;

   // last cache manager cannot flush all of its cache
   if (manager_id + 1 == fh->number_of_nodes) {
      left_to_write = fh->file_size - manager_offset;
   }

   // it is safe to write maximum INT_MAX bytes at once
   while (left_to_write > INT_MAX) {
      error = MPI_File_write_at(fh->file, manager_offset + offset, cache + offset, INT_MAX, MPI_BYTE, &status);
      CHECK_MPI_ERROR(error);
      left_to_write -= INT_MAX;
      offset += INT_MAX;
   }

   return MPI_File_write_at(fh->file, manager_offset + offset, cache + offset, left_to_write, MPI_BYTE, &status);
}


