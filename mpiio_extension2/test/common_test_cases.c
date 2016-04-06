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

#define TEXT_TO_WRITE "This is a random text to write into the file."

extern pmem_io_test_context* global_context;

void test_read_at(int location, int size);
void test_write_at(char* data_to_write, int location, int size);
char* pmem_read_at(int location, int size);
void pmem_write_at(char* buffer, int location, int size);
char* c_read_at(int location, int size);
unsigned get_file_size();
MPI_Info create_mpi_info();

void file_read_at_reads_correct_bytes_from_beginning_of_file() {
   test_read_at(0, 64);
}

void file_read_at_reads_correct_bytes_from_middle_of_file() {
   int chunk_size = 64;
   int location = global_context->file_size / 2 - chunk_size / 2;
   test_read_at(location, chunk_size);
}

void file_read_at_reads_correct_bytes_from_end_of_file() {
   int chunk_size = 64;
   int location = global_context->file_size - chunk_size;
   test_read_at(location, chunk_size);
}

void file_write_at_writes_correct_bytes_at_beginning_of_file() {
   int size = strlen(TEXT_TO_WRITE);
   test_write_at(TEXT_TO_WRITE, 0, size);
}

void file_write_at_writes_correct_bytes_at_middle_of_file() {
   int size = strlen(TEXT_TO_WRITE);
   int location = global_context->file_size / 2 - size / 2;
   test_write_at(TEXT_TO_WRITE, location, size);
}

void file_write_at_writes_correct_bytes_at_end_of_file() {
   int size = strlen(TEXT_TO_WRITE);
   int location = global_context->file_size - size;
   test_write_at(TEXT_TO_WRITE, location, size);
}

void read_at_processes_read_correct_bytes_from_same_file_part() {
   test_read_at(0, 100);
   share_message_errors();
}

void read_at_processes_read_correct_bytes_from_overlapping_parts() {
   int chunk_size = 100 / get_comm_size();
   test_read_at(get_rank(global_context->communicator) * chunk_size / 2, chunk_size);
   share_message_errors();
}

void read_at_processes_read_correct_bytes_from_non_overlapping_parts() {
   int chunk_size = 100 / get_comm_size();
   test_read_at(get_rank(global_context->communicator) * chunk_size, chunk_size);
   share_message_errors();
}

void write_at_processes_wrote_correct_bytes_into_same_file_part() {
   test_write_at(TEXT_TO_WRITE, 0, strlen(TEXT_TO_WRITE));
   share_message_errors();
}

void write_at_processes_wrote_correct_bytes_into_overlapping_parts() {

   int rank = get_rank(global_context->communicator);
   int comm_size = get_comm_size();

   MPI_File file;
   int result = MPI_File_open_pmem(global_context->communicator, global_context->file_path, MPI_MODE_RDWR, create_mpi_info(), &file);
   assert_true(result == MPI_SUCCESS, "MPI_File_open_pmem returned with error");

   int location_modifier = strlen(TEXT_TO_WRITE) / get_comm_size() / 2;
   for (int i=0; i<comm_size; i++) {
      if (rank == i) {
         result = MPI_File_write_at_pmem(file, rank * location_modifier, TEXT_TO_WRITE, strlen(TEXT_TO_WRITE), MPI_CHAR, MPI_STATUS_IGNORE);
         assert_true(result == MPI_SUCCESS, "MPI_File_write_at_pmem returned with error");
      }
      MPI_Barrier(global_context->communicator);
   }

   result = MPI_File_close_pmem(&file);
   assert_true(result == MPI_SUCCESS, "MPI_File_close_pmem returned with error");

   if (rank == 0) {
      assert_true(get_file_size() == global_context->file_size, "File changed its size after read/write operations");

      char* final_text = (char*) malloc(strlen(TEXT_TO_WRITE) * get_comm_size());
      for (int i=0; i<get_comm_size(); i++) {
         strcpy(final_text + i * location_modifier, TEXT_TO_WRITE);
      }
      char* c_read_result = c_read_at(0, strlen(final_text));
      assert_mem_equals(final_text, c_read_result, strlen(final_text), "Function wrote incorrect bytes");
   }

   share_message_errors();
}

void long_sequence_of_read_at_operations() {
   MPI_File file;
   int result = MPI_File_open_pmem(global_context->communicator, global_context->file_path, MPI_MODE_RDWR, create_mpi_info(), &file);
   assert_true(result == MPI_SUCCESS, "MPI_File_open_pmem returned with error");

   int size = 64;
   char* file_fragment = (char*) malloc(size * sizeof(char));

   for (int i=0; i<LONG_SEQUENCE_LENGTH; i++) {
      int location = rand() % (global_context->file_size - size);
      result = MPI_File_read_at_pmem(file, location, file_fragment, size, MPI_BYTE, MPI_STATUS_IGNORE);
      assert_true(result == MPI_SUCCESS, "MPI_File_write_at_pmem returned with error");
      char* c_read_result = c_read_at(location, size);
      assert_mem_equals(file_fragment, c_read_result, size, "Function read incorrect bytes");
   }

   result = MPI_File_close_pmem(&file);
   assert_true(result == MPI_SUCCESS, "MPI_File_close_pmem returned with error");

   if (get_rank(global_context->communicator) == 0) {
      assert_true(get_file_size() == global_context->file_size, "File changed its size after read/write operations");
   }
}

void long_sequence_of_write_at_operations() {
   MPI_File file;
   int result = MPI_File_open_pmem(global_context->communicator, global_context->file_path, MPI_MODE_RDWR, create_mpi_info(), &file);
   assert_true(result == MPI_SUCCESS, "MPI_File_open_pmem returned with error");

   for (int i=0; i<LONG_SEQUENCE_LENGTH; i++) {
      int location = rand() % (global_context->file_size - strlen(TEXT_TO_WRITE));
      result = MPI_File_write_at_pmem(file, location, TEXT_TO_WRITE, strlen(TEXT_TO_WRITE), MPI_CHAR, MPI_STATUS_IGNORE);
      assert_true(result == MPI_SUCCESS, "MPI_File_write_at_pmem returned with error");
   }

   result = MPI_File_close_pmem(&file);
   assert_true(result == MPI_SUCCESS, "MPI_File_close_pmem returned with error");

   if (get_rank(global_context->communicator) == 0) {
      assert_true(get_file_size() == global_context->file_size, "File changed its size after read/write operations");
   }
}

void long_sequence_of_read_at_write_at_operations() {
   MPI_File file;
   int result = MPI_File_open_pmem(global_context->communicator, global_context->file_path, MPI_MODE_RDWR, create_mpi_info(), &file);
   assert_true(result == MPI_SUCCESS, "MPI_File_open_pmem returned with error");

   int size = 64;
   char* file_fragment = (char*) malloc(size * sizeof(char));

   for (int i=0; i<LONG_SEQUENCE_LENGTH; i++) {
      if (rand() % 2) {
         int location = rand() % (global_context->file_size - strlen(TEXT_TO_WRITE));
         result = MPI_File_write_at_pmem(file, location, TEXT_TO_WRITE, strlen(TEXT_TO_WRITE), MPI_CHAR, MPI_STATUS_IGNORE);
         assert_true(result == MPI_SUCCESS, "MPI_File_write_at_pmem returned with error");
      } else {
         int location = rand() % (global_context->file_size - size);
         result = MPI_File_read_at_pmem(file, location, file_fragment, size, MPI_BYTE, MPI_STATUS_IGNORE);
         assert_true(result == MPI_SUCCESS, "MPI_File_write_at_pmem returned with error");
      }
   }

   result = MPI_File_close_pmem(&file);
   assert_true(result == MPI_SUCCESS, "MPI_File_close_pmem returned with error");

   if (get_rank(global_context->communicator) == 0) {
      assert_true(get_file_size() == global_context->file_size, "File changed its size after read/write operations");
   }
}

void file_sync_call() {
   MPI_File file;

   int result = MPI_File_open_pmem(global_context->communicator, global_context->file_path, MPI_MODE_RDWR, create_mpi_info(), &file);
   assert_true(result == MPI_SUCCESS, "MPI_File_open_pmem returned with error");

   result = MPI_File_write_at_pmem(file, 0, TEXT_TO_WRITE, strlen(TEXT_TO_WRITE), MPI_CHAR, MPI_STATUS_IGNORE);
   assert_true(result == MPI_SUCCESS, "MPI_File_write_at_pmem returned with error");

   result = MPI_File_sync_pmem(file);
   assert_true(result == MPI_SUCCESS, "MPI_File_sync returned with error");

   // check data on storage device before MPI_File_close is called
   char* c_read_result = c_read_at(0, strlen(TEXT_TO_WRITE));
   assert_mem_equals(TEXT_TO_WRITE, c_read_result, strlen(TEXT_TO_WRITE), "Function does not sync bytes with fs");

   // another write operation to prove that the file is still opened
   result = MPI_File_write_at_pmem(file, 1, TEXT_TO_WRITE, strlen(TEXT_TO_WRITE), MPI_CHAR, MPI_STATUS_IGNORE);
   assert_true(result == MPI_SUCCESS, "MPI_File_write returned an error after MPI_File_sync was called");

   result = MPI_File_close_pmem(&file);
   assert_true(result == MPI_SUCCESS, "MPI_File_close_pmem returned with error");
}

void write_at_processes_wrote_correct_bytes_into_non_overlapping_parts() {
   int single_part = strlen(TEXT_TO_WRITE) / get_comm_size();
   int rank = get_rank(global_context->communicator);
   test_write_at(TEXT_TO_WRITE + rank * single_part, rank * single_part, single_part);
   share_message_errors();
}

void test_read_at(int location, int size) {
   char* pmem_read_result = pmem_read_at(location, size);
   char* c_read_result = c_read_at(location, size);
   assert_mem_equals(pmem_read_result, c_read_result, size, "Function read incorrect bytes");
}

void test_write_at(char* data_to_write, int location, int size) {
   pmem_write_at(data_to_write, location, size);
   char* c_read_result = c_read_at(location, size);
   assert_mem_equals(data_to_write, c_read_result, size, "Function wrote incorrect bytes");
}

char* pmem_read_at(int location, int size) {

   MPI_File file;
   int result = MPI_File_open_pmem(global_context->communicator, global_context->file_path, MPI_MODE_RDWR, create_mpi_info(), &file);
   assert_true(result == MPI_SUCCESS, "MPI_File_open_pmem returned with error");

   char* file_fragment = (char*) malloc(size * sizeof(char));
   result = MPI_File_read_at_pmem(file, location, file_fragment, size, MPI_BYTE, MPI_STATUS_IGNORE);
   assert_true(result == MPI_SUCCESS, "MPI_File_read_at_pmem returned with error");

   result = MPI_File_close_pmem(&file);
   assert_true(result == MPI_SUCCESS, "MPI_File_close_pmem returned with error");

   assert_true(get_file_size() == global_context->file_size, "File changed its size after read/write operations");

   return file_fragment;
}

void pmem_write_at(char* buffer, int location, int size) {

   MPI_File file;
   int result = MPI_File_open_pmem(global_context->communicator, global_context->file_path, MPI_MODE_RDWR, create_mpi_info(), &file);
   assert_true(result == MPI_SUCCESS, "MPI_File_open_pmem returned with error");

   result = MPI_File_write_at_pmem(file, location, buffer, size, MPI_BYTE, MPI_STATUS_IGNORE);
   assert_true(result == MPI_SUCCESS, "MPI_File_write_at_pmem returned with error");

   result = MPI_File_close_pmem(&file);
   assert_true(result == MPI_SUCCESS, "MPI_File_close_pmem returned with error");

   assert_true(get_file_size() == global_context->file_size, "File changed its size after read/write operations");
}

char* c_read_at(int location, int size) {

   FILE *fp = fopen(global_context->file_path, "r");
   assert_true(fp != NULL, "Cannot open test file in provided location");

   fseek(fp, location, SEEK_SET);

   char* buffer = (char*) malloc(sizeof(char) * size);
   int read_result = fread(buffer, 1, size, fp);
   assert_true(read_result == size, "Error while reading the file");

   fclose(fp);
   return buffer;
}

unsigned get_file_size() {

   FILE *fp = fopen(global_context->file_path, "r");
   assert_true(fp != NULL, "Cannot open test file in provided location");

   fseek(fp, 0L, SEEK_END);
   int size = ftell(fp);
   fclose(fp);
   return size;
}

MPI_Info create_mpi_info() {
   MPI_Info info;
   MPI_Info_create(&info);
   switch (global_context->mode) {
      case PMEM_IO_AWARE_FS:
         MPI_Info_set(info, MPI_PMEM_INFO_KEY_MODE, PMEM_IO_AWARE_FS_STRING);
         break;
      case PMEM_IO_DISTRIBUTED_CACHE:
         MPI_Info_set(info, MPI_PMEM_INFO_KEY_MODE, PMEM_IO_DISTRIBUTED_CACHE_STRING);
         MPI_Info_set(info, MPI_PMEM_INFO_KEY_PMEM_PATH, global_context->pmem_path);
         break;
   }
   return info;
}
