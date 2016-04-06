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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

#include <unistd.h>

#include <mpi.h>

#include "file_io_pmem.h"

void run_test_01(MPI_File, bool extension_mode_on, int iterations, unsigned long long file_size, int chunk_size);
void print_help();
unsigned long long random_64bit();
char* int_to_char_array(int value);


int main(int argc, char* argv[]) {

   int rank, provided_thread_support;

   MPI_Init_thread_pmem(&argc, &argv, MPI_THREAD_MULTIPLE, &provided_thread_support);
   MPI_Comm_rank(MPI_COMM_WORLD, &rank);

   if (provided_thread_support != MPI_THREAD_MULTIPLE) {
      if (rank == 0) {
         printf("Pmem IO extension requires MPI_THREAD_MULTIPLE support.");
      }
      return 1;
   }

   if (argc != 9) {
      if (rank == 0) {
         print_help();
      }
      return 0;
   }

   bool extension_mode_on = atoi(argv[1]);
   int pmem_io_aware = atoi(argv[2]);
   int test_case = atoi(argv[3]);
   char* pmem_path = argv[4];
   char* file_path = argv[5];
   unsigned long long file_size = (unsigned long long) atoi(argv[6]) * 1024 * 1024; //in bytes;
   int chunk_size = atoi(argv[7]);
   int iterations = atoi(argv[8]);

   MPI_Info info;
   MPI_Info_create(&info);
   MPI_Info_set(info, MPI_PMEM_INFO_KEY_PMEM_PATH, pmem_path);
   MPI_Info_set(info, MPI_PMEM_INFO_KEY_MODE, int_to_char_array(pmem_io_aware));

   MPI_File f;
   if (extension_mode_on) {
      MPI_File_open_pmem(MPI_COMM_WORLD, file_path, MPI_MODE_RDWR | MPI_MODE_CREATE, info, &f);
      MPI_File_set_size_pmem(f, file_size);
   } else {
      MPI_File_open(MPI_COMM_WORLD, file_path, MPI_MODE_RDWR | MPI_MODE_CREATE, info, &f);
      MPI_File_set_size(f, file_size);
   }

   switch (test_case) {
      case 1: {
         run_test_01(f, extension_mode_on, iterations, file_size, chunk_size);
         break;
      }
   }

   if (extension_mode_on) {
      MPI_File_close_pmem(&f);
   } else {
      MPI_File_close(&f);
   }

   MPI_Finalize();
}

void print_help() {
   printf("\n");
   printf("MPI File IO PMEM extension tests\n");
   printf("\n");
   printf(
         "Usage: [extension off=0,on=1] [pmem aware off=0,on=1] [test case] [pmem path] [file path] [file size (MB)] [chunk size (B)] [iterations] \n");
   printf("\n");
   printf("Test cases\n");
   printf("   1 - simple performance test\n");
   printf("\n");
}

void run_test_01(MPI_File f, bool extension_mode_on, int iterations, unsigned long long file_size, int chunk_size) {

   srand(time(NULL));

   char* data_to_write = (char*) malloc(chunk_size);
   char* data_to_read = (char*) malloc(chunk_size);
   for (int i = 0; i < chunk_size; i++) {
      *(data_to_write + i) = '0' + rand() % 10;
   }

   int rank, comm_size;
   MPI_Comm_rank(MPI_COMM_WORLD, &rank);
   MPI_Comm_size(MPI_COMM_WORLD, &comm_size);

   for (int i = 0; i < iterations; i++) {

      // read from the random slot
      unsigned long long offset = random_64bit() % (file_size - chunk_size);
      if (extension_mode_on) {
         MPI_File_read_at_pmem(f, offset, data_to_read, chunk_size, MPI_CHAR, MPI_STATUSES_IGNORE);
      } else {
         MPI_File_read_at(f, offset, data_to_read, chunk_size, MPI_CHAR, MPI_STATUSES_IGNORE);
      }

      // make some fake computations (collatz conjecture)
      int randomModificator = rand() % 1000000;
      int x = 123456;
      for (int j = 0; j < 1000000 + randomModificator; j++) {
         if (x % 2 == 0) {
            x = x / 2;
         } else {
            x = 3 * x + 1;

         }
      }
      printf("%d", x);

      // write into the random location
      offset = random_64bit() % (file_size - chunk_size);
      if (extension_mode_on) {
         MPI_File_write_at_pmem(f, offset, data_to_write, chunk_size, MPI_CHAR, MPI_STATUSES_IGNORE);
      } else {
         MPI_File_write_at(f, offset, data_to_write, chunk_size, MPI_CHAR, MPI_STATUSES_IGNORE);
      }

      MPI_Barrier(MPI_COMM_WORLD);
   }
}

unsigned long long random_64bit() {
   return (unsigned long long) rand()
         + ((unsigned long long)rand() << 15)
         + ((unsigned long long)rand() << 30)
         + ((unsigned long long)rand() << 45)
         + (((unsigned long long)rand() & 0xf) << 60);
}

char* int_to_char_array(int value) {
   char* str = (char*) malloc(3*sizeof(int) + 1); // approx log(INT_MAX) + null termination
   sprintf(str, "%d", value);
   return str;
}

