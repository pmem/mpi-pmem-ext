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

#define SINGLE_PACKAGE_SIZE 1024

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

   if (argc != 3) {
      printf("Usage: [pmem path] [file path]. The file will be created.\n");
      return 0;
   }

   char* pmem_path = argv[1];
   char* file_path = argv[2];
   char is_ping = 1;

   char ping[SINGLE_PACKAGE_SIZE];
   char pong[SINGLE_PACKAGE_SIZE];
   for (int i=0; i<SINGLE_PACKAGE_SIZE; i++) {
      ping[i] = '0';
      pong[i] = '1';
   }

   // prepare test file
   FILE *file = fopen(file_path, "w");
   if (file == NULL) {
      return -1;
   }
   fwrite(ping, 1, SINGLE_PACKAGE_SIZE, file);
   fclose(file);

   // open file using MPI
   MPI_Info info;
   MPI_Info_create(&info);
   MPI_Info_set(info, MPI_PMEM_INFO_KEY_PMEM_PATH, pmem_path);
   MPI_Info_set(info, MPI_PMEM_INFO_KEY_MODE, PMEM_IO_DISTRIBUTED_CACHE_STRING);
   MPI_Info_set(info, MPI_PMEM_INFO_KEY_FAILURE_RECOVERY, "true");

   MPI_File f;
   MPI_File_open_pmem(MPI_COMM_WORLD, file_path, MPI_MODE_RDWR, info, &f);

   printf("Now you can stop program at any moment!\n");

   while (true) {
      if (is_ping) {
         MPI_File_write_at_pmem(f, 0, ping, SINGLE_PACKAGE_SIZE, MPI_BYTE, MPI_STATUS_IGNORE);
         is_ping = 0;
      } else {
         MPI_File_write_at_pmem(f, 0, pong, SINGLE_PACKAGE_SIZE, MPI_BYTE, MPI_STATUS_IGNORE);
         is_ping = 1;
      }
      sleep(1);
   }

   return 0;
}
