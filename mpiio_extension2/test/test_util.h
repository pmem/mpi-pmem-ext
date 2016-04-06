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


#ifndef __TEST_UTIL_H__
#define __TEST_UTIL_H__

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>

#include <mpi.h>

#define MAX_ALLOWED_TEST_FUNCTION_NAME_LENGTH 60
#define MAX_ERROR_DESC_LENGTH 1024
#define TEST_FILE_NAME "mpi_pmem_io_emp_test_file"
#define LONG_SEQUENCE_LENGTH 32

// exceptions (single jump buffer - only single try-catch allowed)
extern jmp_buf exception_jump_buffer;

typedef struct pmem_io_test_context pmem_io_test_context;
struct pmem_io_test_context {
   char* pmem_path;
   char* shared_directory_path;
   char* file_path;
   int mode;
   unsigned long file_size;
   char case_name[MAX_ALLOWED_TEST_FUNCTION_NAME_LENGTH + 1];
   char error_desc[MAX_ERROR_DESC_LENGTH + 1];
   MPI_Comm communicator;
};

typedef struct pmem_io_test pmem_io_test;
struct pmem_io_test {
   void (*function)();
   char* name;
   int procs_limited;
   int mode;
};

void test_setup();
void test_teardown();

void assert_mem_equals(char* buffer1, char* buffer2, int size, char* error_message);
void assert_true(int condition, char* error_message);
void assert(char* error_message);

void share_message_errors();

int get_rank(MPI_Comm comm);
int get_comm_size();

#endif
