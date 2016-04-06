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


#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "test_util.h"
#include "file_io_pmem.h"

void create_test_file();
void remove_test_file();
char* prepare_test_file_path();
int get_comm_size();

#define SEED_FOR_RANDOM_NUMBER_GENERATOR 12345

pmem_io_test_context* global_context;
jmp_buf exception_jump_buffer;

void test_setup(pmem_io_test_context* context) {
   global_context = context;
   global_context->error_desc[0] = '\0';
   global_context->file_path = prepare_test_file_path();
   if (get_rank(MPI_COMM_WORLD) == 0) {
      create_test_file();
   }
   srand(SEED_FOR_RANDOM_NUMBER_GENERATOR);
}

void test_teardown() {
   if (get_rank(MPI_COMM_WORLD) == 0) {
      remove_test_file(global_context->pmem_path);
   }
}

void assert_mem_equals(char* buffer1, char* buffer2, int size, char* error_message) {
   if (memcmp(buffer1, buffer2, size)) {
      assert(error_message);
   }
}

void assert_true(int condition, char* error_message) {
   if (!condition) {
      assert(error_message);
   }
}

void assert(char* error_message) {
   // remember only single error desc
   if (global_context->error_desc[0] == '\0') {
      strcpy(global_context->error_desc, error_message);
   }

   // stop immediately only when single process operates;
   // multiple processes stops after whole routine
   if (global_context->communicator == MPI_COMM_SELF) {
      longjmp(exception_jump_buffer, 1); // throw exception
   }
}

void share_message_errors() {
   MPI_Barrier(global_context->communicator);
   int comm_size = get_comm_size();
   int global_error_count = 0;
   int local_error_count = global_context->error_desc[0] != '\0' ? 1 : 0;
   int* all_errors = (int*) malloc(comm_size * sizeof(int));
   MPI_Gather(&local_error_count, 1, MPI_INT, all_errors, 1, MPI_INT, 0, global_context->communicator);

   if (get_rank(global_context->communicator) == 0) {

      // count global errors
      for (int i=0; i<comm_size; i++) {
         global_error_count += all_errors[i];
      }

      // if errors occured
      if (local_error_count || global_error_count) {

         char message[MAX_ERROR_DESC_LENGTH * global_error_count];
         sprintf(message, "%d process(es) returned error", global_error_count);

         if (local_error_count) {
            sprintf(message + strlen(message), ". [0] %s", global_context->error_desc);
         }

         for (int i = 0; i < global_error_count - local_error_count; i++) {
            sprintf(message + strlen(message), ". [%d] ", i + local_error_count);
            MPI_Recv(message + strlen(message), MAX_ERROR_DESC_LENGTH, MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, global_context->communicator, MPI_STATUS_IGNORE);
         }

         strcpy(global_context->error_desc, message);
         longjmp(exception_jump_buffer, 1); // throw exception
      }
   } else if (local_error_count) {
      MPI_Send(global_context->error_desc, MAX_ERROR_DESC_LENGTH, MPI_CHAR, 0, 0, global_context->communicator);
   }
}

void create_test_file() {

   FILE *fp;
   fp = fopen(global_context->file_path, "w");
   if (fp == NULL) {
      printf("\nError. Cannot create test file in provided location: %s\n", global_context->file_path);
      exit(1);
   }

   for (unsigned i=0; i < global_context->file_size; i++) {
      fprintf(fp, "%d", i%10);
   }

   if (fclose(fp) != 0) {
      printf("Error. Cannot close test file in provided location: %s\n", global_context->file_path);
      exit(1);
   }
}

void remove_test_file() {
   remove(global_context->file_path);
}

char* prepare_test_file_path() {
   char* directory_path = NULL;
   if (global_context->mode == PMEM_IO_AWARE_FS) {
      directory_path = global_context->pmem_path;
   } else if (global_context->mode == PMEM_IO_DISTRIBUTED_CACHE) {
      directory_path = global_context->shared_directory_path;
   }
   char* file_path = (char*) malloc(strlen(directory_path) + strlen(TEST_FILE_NAME) + 2);
   sprintf(file_path, "%s/%s", directory_path, TEST_FILE_NAME);
   return file_path;
}

int get_rank(MPI_Comm comm) {
   int rank;
   int result = MPI_Comm_rank(comm, &rank);
   if (result != MPI_SUCCESS) {
      printf("Cannot call MPI_Comm_rank.");
      exit(1);
   }
   return rank;
}

int get_comm_size() {
   int size;
   int result = MPI_Comm_size(MPI_COMM_WORLD, &size);
   if (result != MPI_SUCCESS) {
      printf("Cannot call MPI_Comm_size.");
      exit(1);
   }
   return size;
}

