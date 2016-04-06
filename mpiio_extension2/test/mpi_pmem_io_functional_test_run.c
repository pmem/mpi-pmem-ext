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


#include "common_test_cases.h"
#include "test_util.h"
#include "file_io_pmem.h"

void print_help();

#define TEST_CASE_OUTPUT_FORMAT
#define TRUE 1
#define FALSE 0

int main(int argc, char* argv[]) {

   if (argc != 3) {
      print_help();
      return 0;
   }

   int provided_thread_support;
   MPI_Init_thread_pmem(&argc, &argv, MPI_THREAD_MULTIPLE, &provided_thread_support);
   if (provided_thread_support != MPI_THREAD_MULTIPLE) {
      printf("Pmem IO extension requires MPI_THREAD_MULTIPLE support.");
      return -1;
   }

   pmem_io_test_context context;
   context.pmem_path = argv[1];
   context.shared_directory_path = argv[2];

   pmem_io_test tests[] = {

         // pmem aware fs
         {.mode = 1, .procs_limited = TRUE,  .name = "Read at, beginning of file",         .function = file_read_at_reads_correct_bytes_from_beginning_of_file},
         {.mode = 1, .procs_limited = TRUE,  .name = "Read at, middle of file",            .function = file_read_at_reads_correct_bytes_from_middle_of_file},
         {.mode = 1, .procs_limited = TRUE,  .name = "Read at, end of file",               .function = file_read_at_reads_correct_bytes_from_end_of_file},
         {.mode = 1, .procs_limited = TRUE,  .name = "Write at, beginning of file",        .function = file_write_at_writes_correct_bytes_at_beginning_of_file},
         {.mode = 1, .procs_limited = TRUE,  .name = "Write at, middle of file",           .function = file_write_at_writes_correct_bytes_at_middle_of_file},
         {.mode = 1, .procs_limited = TRUE,  .name = "Write at, end of file",              .function = file_write_at_writes_correct_bytes_at_end_of_file},
         {.mode = 1, .procs_limited = TRUE,  .name = "Long sequence of read operations",   .function = long_sequence_of_read_at_operations},
         {.mode = 1, .procs_limited = TRUE,  .name = "Long sequence of write operations",  .function = long_sequence_of_write_at_operations},
         {.mode = 1, .procs_limited = TRUE,  .name = "Long seq. of read/write operations", .function = long_sequence_of_read_at_write_at_operations},
         {.mode = 1, .procs_limited = TRUE,  .name = "File sync call",                     .function = file_sync_call},

         // distributed cache mode
         {.mode = 0, .procs_limited = TRUE,  .name = "Read at, beginning of file",         .function = file_read_at_reads_correct_bytes_from_beginning_of_file},
         {.mode = 0, .procs_limited = TRUE,  .name = "Read at, middle of file",            .function = file_read_at_reads_correct_bytes_from_middle_of_file},
         {.mode = 0, .procs_limited = TRUE,  .name = "Read at, end of file",               .function = file_read_at_reads_correct_bytes_from_end_of_file},
         {.mode = 0, .procs_limited = TRUE,  .name = "Write at, beginning of file",        .function = file_write_at_writes_correct_bytes_at_beginning_of_file},
         {.mode = 0, .procs_limited = TRUE,  .name = "Write at, middle of file",           .function = file_write_at_writes_correct_bytes_at_middle_of_file},
         {.mode = 0, .procs_limited = TRUE,  .name = "Write at, end of file",              .function = file_write_at_writes_correct_bytes_at_end_of_file},
         {.mode = 0, .procs_limited = FALSE, .name = "Read at, same part",                 .function = read_at_processes_read_correct_bytes_from_same_file_part},
         {.mode = 0, .procs_limited = FALSE, .name = "Read at, overlapping parts",         .function = read_at_processes_read_correct_bytes_from_overlapping_parts},
         {.mode = 0, .procs_limited = FALSE, .name = "Read at, non-overlapping parts",     .function = read_at_processes_read_correct_bytes_from_non_overlapping_parts},
         {.mode = 0, .procs_limited = FALSE, .name = "Write at, same part",                .function = write_at_processes_wrote_correct_bytes_into_same_file_part},
         {.mode = 0, .procs_limited = FALSE, .name = "Write at, overlapping parts",        .function = write_at_processes_wrote_correct_bytes_into_overlapping_parts},
         {.mode = 0, .procs_limited = FALSE, .name = "Write at, non-overlapping parts",    .function = write_at_processes_wrote_correct_bytes_into_non_overlapping_parts},
         {.mode = 0, .procs_limited = FALSE, .name = "Long sequence of read operations",   .function = long_sequence_of_read_at_operations},
         {.mode = 0, .procs_limited = FALSE, .name = "Long sequence of write operations",  .function = long_sequence_of_write_at_operations},
         {.mode = 0, .procs_limited = FALSE, .name = "Long seq. of read/write operations", .function = long_sequence_of_read_at_write_at_operations},
         {.mode = 0, .procs_limited = FALSE, .name = "File sync call",                     .function = file_sync_call},
   };

   int file_sizes[] = {128, 1024*1024, 10*1024*1024, 100*1024*1024};
   for (unsigned j = 0; j < sizeof(file_sizes) / sizeof(file_sizes[0]); j++) {
      for (unsigned i = 0; i < sizeof(tests) / sizeof(tests[0]); i++) {

         context.mode = tests[i].mode;
         context.file_size = file_sizes[j];
         int exception = FALSE;

         test_setup(&context);
         MPI_Barrier(MPI_COMM_WORLD);

         if (tests[i].procs_limited) {
            context.communicator = MPI_COMM_SELF;
         } else {
            context.communicator = MPI_COMM_WORLD;
         }

         if (get_rank(MPI_COMM_WORLD) == 0) {
            printf("[%3dMB][%s][%-18s] %-35s ",
                  file_sizes[j] / (1024*1024),
                  tests[i].mode ? "aware" : "cache",
                  tests[i].procs_limited ? "single process" : "multiple processes",
                  tests[i].name);
         }
         MPI_Barrier(MPI_COMM_WORLD);

         if (!tests[i].procs_limited || get_rank(MPI_COMM_WORLD) == 0) {
            if (!setjmp(exception_jump_buffer)) {
               tests[i].function();
            } else {
               exception = TRUE;
            }
         }
         MPI_Barrier(MPI_COMM_WORLD);

         if (get_rank(MPI_COMM_WORLD) == 0) {
            printf("%s %s\n",
                  exception ? "Error:" : "OK",
                  context.error_desc);
         }

         MPI_Barrier(MPI_COMM_WORLD);
         test_teardown();

         MPI_Barrier(MPI_COMM_WORLD);
      }
   }

   return 0;
}

void print_help() {
   printf("Usage: mpi_pmem_io_func_test [path on pmem] [path to directory shared by all of the nodes].\n");
}
