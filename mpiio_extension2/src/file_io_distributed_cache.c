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


#include "file_io_distributed_cache.h"

#include <mpi.h>

#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>

#include "util.h"
#include "logger.h"

#include "messages.h"
#include "pmem_datatypes.h"
#include "mpi_node_rank.h"
#include "cache_manager.h"
#include "file_io_pmem.h"
#include "failure_recovery.h"


// private function definitions
char* get_attribute_for_key(MPI_Info info, char* key);
char* generate_cache_folder_path(char* pmem_path);
bool get_boolean_value_from_info(MPI_Info info, char* key);

int MPI_File_open_pmem_distributed(MPI_Comm communicator, char* filename, int amode, MPI_Info info, MPI_File_pmem* fh_pmem) {

   UNUSED(info);

   // variables
   MPI_Comm cache_communicator;
   MPI_Comm internode_communicator;
   int error, rank, node_rank, cache_manager_id, number_of_nodes;
   int* cache_managers = NULL;

   // new commmunicator to exchange cache messages
   error = MPI_Comm_dup(communicator, &cache_communicator);
   CHECK_MPI_ERROR(error);
   error = MPI_Comm_rank(cache_communicator, &rank);
   CHECK_MPI_ERROR(error);

   srand(time(0) + rank);

   // file open
   error = MPI_File_open(cache_communicator, filename, amode, info, &fh_pmem->file);
   CHECK_MPI_ERROR(error);

   fh_pmem->communicator = cache_communicator;
   fh_pmem->cache_manager_thread = NULL;
   error = MPI_File_get_size(fh_pmem->file, &fh_pmem->file_size);
   CHECK_MPI_ERROR(error);
   fh_pmem->filename = (char*) malloc(strlen(filename));
   CHECK_MALLOC_ERROR(fh_pmem->filename);
   strcpy(fh_pmem->filename, filename);

   // commmunicator within a node to select only one process per node
   get_mpi_node_rank(cache_communicator, rank, &node_rank);
   error = MPI_Comm_split(cache_communicator, node_rank, rank, &internode_communicator);
   CHECK_MPI_ERROR(error);

   if (node_rank == 0) {

      // collecting rank of cache_communicator for each first process in node (cache managers)
      error = MPI_Comm_rank(internode_communicator, &cache_manager_id);
      CHECK_MPI_ERROR(error);

      error = MPI_Comm_size(internode_communicator, &number_of_nodes);
      CHECK_MPI_ERROR(error);

      cache_managers = (int*) malloc(number_of_nodes * sizeof(int));
      CHECK_MALLOC_ERROR(cache_managers);
      cache_managers[0] = rank;

      error = MPI_Gather(&rank, 1, MPI_INT, cache_managers, 1, MPI_INT, 0, internode_communicator);
      CHECK_MPI_ERROR(error);
   } else {
      // possible opt: close unused comms
   }

   // sharing list of cache managers
   error = MPI_Bcast(&number_of_nodes, 1, MPI_INT, 0, cache_communicator);
   CHECK_MPI_ERROR(error);
   if (node_rank != 0) {
      cache_managers = (int*) malloc(number_of_nodes * sizeof(int));
      CHECK_MALLOC_ERROR(cache_managers);
   }
   error = MPI_Bcast(cache_managers, number_of_nodes, MPI_INT, 0, cache_communicator);
   fh_pmem->number_of_nodes = number_of_nodes;
   fh_pmem->cache_managers = cache_managers;

   // cache managers offsets
   fh_pmem->cache_size = fh_pmem->file_size / fh_pmem->number_of_nodes;
   fh_pmem->cache_offsets = (unsigned long long*) malloc(number_of_nodes * sizeof(unsigned long long));
   CHECK_MALLOC_ERROR(fh_pmem->cache_offsets);
   for (int i = 0; i < number_of_nodes; i++) {
      fh_pmem->cache_offsets[i] = i * fh_pmem->cache_size;
   }

   // logging
   if (is_debug_log_enabled() && rank == 0) {
      log_debug("Number of nodes: %d.", number_of_nodes);
      for (int i = 0; i < number_of_nodes; i++) {
         log_debug("[cache managers list] Node %d: %d. ", i, cache_managers[i]);
      }
   }

   error = MPI_Barrier(fh_pmem->communicator);
   CHECK_MPI_ERROR(error);

   fh_pmem->is_failure_recovery_enabled = get_boolean_value_from_info(info, MPI_PMEM_INFO_KEY_FAILURE_RECOVERY);
   fh_pmem->do_recovery = get_boolean_value_from_info(info, MPI_PMEM_INFO_KEY_DO_FAILURE_RECOVERY);
   log_info("Failure recovery: recovery enabled: %s, do recovery: %s",
         fh_pmem->is_failure_recovery_enabled ? "true" : "false", fh_pmem->do_recovery ? "true" : "false");

   char* pmem_path = get_attribute_for_key(info, MPI_PMEM_INFO_KEY_PMEM_PATH);
   if (fh_pmem->do_recovery) {
      fh_pmem->cache_path = get_attribute_for_key(info, MPI_PMEM_INFO_KEY_DO_FAILURE_RECOVERY_PATH);
      fh_pmem->cache_path = concat(fh_pmem->cache_path, "/");
   } else {
      fh_pmem->cache_path = generate_cache_folder_path(pmem_path);
      error = MPI_Bcast(fh_pmem->cache_path, strlen(fh_pmem->cache_path) + 1, MPI_CHAR, 0, fh_pmem->communicator);
      CHECK_MPI_ERROR(error);
   }

   if (node_rank == 0) {
      // folder for cache storage
      if (fh_pmem->do_recovery) {
         if (!check_file_exists(concat(fh_pmem->cache_path, "/cache"))) {
            log_error("Cannot recover from provided cache path, file does not exists.");
            return PMEM_IO_ERROR_FILE_OPEN;
         }
      } else {
         error = mkdir(fh_pmem->cache_path, 0777);
         CHECK_MKDIR_ERROR(error);
      }
      init_cache_manager(cache_manager_id, fh_pmem);
   }

   error = MPI_Barrier(fh_pmem->communicator);
   CHECK_MPI_ERROR(error);

   if (fh_pmem->do_recovery && node_rank == 0) {
      perform_data_recovery(fh_pmem, fh_pmem->cache_path);
   }

   return error;
}

int MPI_File_close_pmem_distributed(MPI_File_pmem* fh_pmem) {

   MPI_File_pmem_message msg;
   int rank;

   int error = MPI_Barrier(fh_pmem->communicator);
   CHECK_MPI_ERROR(error);
   error = MPI_Comm_rank(fh_pmem->communicator, &rank);
   CHECK_MPI_ERROR(error);

   if (rank == 0) {
      for (int i = 0; i < fh_pmem->number_of_nodes; i++) {
         error = MPI_Send(&msg, 0, MPI_BYTE, fh_pmem->cache_managers[i], PMEM_TAG_SHUTDOWN, fh_pmem->communicator);
         CHECK_MPI_ERROR(error);
      }
   }

   if (fh_pmem->cache_manager_thread != NULL) {
      deinit_cache_manager(fh_pmem);
      error = rmdir(fh_pmem->cache_path);
      CHECK_RMDIR_ERROR(error);
   }

   error = MPI_Barrier(fh_pmem->communicator);
   CHECK_MPI_ERROR(error);

   error = MPI_File_close(&fh_pmem->file);
   CHECK_MPI_ERROR(error);

   free(fh_pmem);

   return error;
}

int MPI_File_read_at_pmem_distributed(MPI_File_pmem* fh_pmem, MPI_Offset offset, void *buf, int count, MPI_Datatype datatype, MPI_Status *status) {

   UNUSED(status);

   int type_size;
   int error = MPI_Type_size(datatype, &type_size);
   CHECK_MPI_ERROR(error);
   int left_to_read = type_size * count;

   MPI_File_pmem_message msg;
   msg.offset = offset;
   int receiver;
   char* buf_position = (char*) buf;

   /* three phases of reading:
    * 1. from first cache manager
    * 2. from n cache managers, each read with size of cache
    * 3. from last cache manager
    */

   // first phase
   log_debug("Read at, phase 1");
   receiver = msg.offset / fh_pmem->cache_size;
   long long left_in_cache = fh_pmem->cache_size - msg.offset % fh_pmem->cache_size;
   if (left_in_cache > left_to_read) {
      msg.size = left_to_read;
   } else {
      msg.size = left_in_cache;
   }
   error = MPI_Sendrecv(&msg, 1, PMEM_MSG_TYPE, fh_pmem->cache_managers[receiver], PMEM_TAG_READ_AT_REQUEST_TAG, buf_position, msg.size,
         MPI_BYTE, fh_pmem->cache_managers[receiver], PMEM_TAG_READ_AT_RESPONSE_TAG, fh_pmem->communicator, MPI_STATUS_IGNORE);

   CHECK_MPI_ERROR(error);
   buf_position += msg.size;
   left_to_read -= msg.size;
   msg.offset += msg.size;

   // second phase
   log_debug("Read at, phase 2");
   msg.size = fh_pmem->cache_size;
   while ((unsigned)left_to_read > fh_pmem->cache_size) {
      receiver++;
      error = MPI_Sendrecv(&msg, 1, PMEM_MSG_TYPE, fh_pmem->cache_managers[receiver], PMEM_TAG_READ_AT_REQUEST_TAG, buf_position, msg.size,
            MPI_BYTE, fh_pmem->cache_managers[receiver], PMEM_TAG_READ_AT_RESPONSE_TAG, fh_pmem->communicator, MPI_STATUS_IGNORE);
      CHECK_MPI_ERROR(error);
      msg.offset += fh_pmem->cache_size;
      left_to_read -= msg.size;
      buf_position += msg.size;
   }

   // third phase
   log_debug("Read at, phase 3");
   if (left_to_read > 0) {
      receiver++;
      msg.size = left_to_read;
      error = MPI_Sendrecv(&msg, 1, PMEM_MSG_TYPE, fh_pmem->cache_managers[receiver], PMEM_TAG_READ_AT_REQUEST_TAG, buf_position, msg.size,
            MPI_BYTE, fh_pmem->cache_managers[receiver], PMEM_TAG_READ_AT_RESPONSE_TAG, fh_pmem->communicator, MPI_STATUS_IGNORE);
      CHECK_MPI_ERROR(error);
   }

   return error;
}

int MPI_File_write_at_pmem_distributed(MPI_File_pmem* fh_pmem, MPI_Offset offset, void *buf, int count, MPI_Datatype datatype, MPI_Status *status) {

   UNUSED(status);

   int left_to_write;
   int error = MPI_Type_size(datatype, &left_to_write);
   left_to_write *= count;

   // possible opt: try to reduce the size
   char* msg_full = (char*) malloc(sizeof(MPI_File_pmem_message) + fh_pmem->cache_size);
   MPI_File_pmem_message* msg = (MPI_File_pmem_message*) msg_full;
   msg->offset = offset;
   int receiver;
   char* buf_position = (char*) buf;

   char* recovery_data_file = NULL;
   if (fh_pmem->is_failure_recovery_enabled) {
      recovery_data_file = recovery_data_create(buf, offset, left_to_write, fh_pmem->cache_path);
   }

   /* three phases of writing:
    * 1. into first cache manager
    * 2. into n cache managers, each write with size of cache
    * 3. into last cache manager
    */

   // first phase
   log_debug("Write at, phase 1");
   receiver = msg->offset / fh_pmem->cache_size;
   long long left_in_cache = fh_pmem->cache_size - msg->offset % fh_pmem->cache_size;
   if (left_in_cache > left_to_write) {
      msg->size = left_to_write;
   } else {
      msg->size = left_in_cache;
   }
   memcpy(msg_full + sizeof(MPI_File_pmem_message), buf_position, msg->size);
   error = MPI_Send(msg_full, msg->size + sizeof(MPI_File_pmem_message), MPI_BYTE, fh_pmem->cache_managers[receiver],
         PMEM_TAG_WRITE_AT_REQUEST_TAG, fh_pmem->communicator);
   CHECK_MPI_ERROR(error);
   buf_position += msg->size;
   left_to_write -= msg->size;
   msg->offset += msg->size;

   // second phase
   log_debug("Write at, phase 2");
   msg->size = fh_pmem->cache_size;
   while ((unsigned) left_to_write > fh_pmem->cache_size) {
      receiver++;
      memcpy(msg_full + sizeof(MPI_File_pmem_message), buf_position, msg->size);
      error = MPI_Send(msg_full, msg->size + sizeof(MPI_File_pmem_message), MPI_BYTE, fh_pmem->cache_managers[receiver],
            PMEM_TAG_WRITE_AT_REQUEST_TAG, fh_pmem->communicator);
      CHECK_MPI_ERROR(error);
      msg->offset += fh_pmem->cache_size;
      left_to_write -= msg->size;
      buf_position += msg->size;
   }

   // third phase
   log_debug("Write at, phase 3");
   if (left_to_write > 0) {
      receiver++;
      msg->size = left_to_write;
      memcpy(msg_full + sizeof(MPI_File_pmem_message), buf_position, msg->size);
      error = MPI_Send(msg_full, msg->size + sizeof(MPI_File_pmem_message), MPI_BYTE, fh_pmem->cache_managers[receiver],
            PMEM_TAG_WRITE_AT_REQUEST_TAG, fh_pmem->communicator);
      CHECK_MPI_ERROR(error);
   }

   free(msg_full);
   if (fh_pmem->is_failure_recovery_enabled) {
      recovery_data_remove(recovery_data_file);
   }
   return error;
}

int MPI_File_set_size_pmem_distributed(MPI_File_pmem* fh_pmem, MPI_Offset offset) {
   UNUSED(offset);
   UNUSED(fh_pmem);
   return PMEM_IO_ERROR_UNSUPPORTED_OPERATION;
}

int MPI_File_sync_pmem_distributed(MPI_File_pmem* fh_pmem) {

   MPI_File_pmem_message msg;
   int rank;

   int error = MPI_Barrier(fh_pmem->communicator);
   CHECK_MPI_ERROR(error);
   error = MPI_Comm_rank(fh_pmem->communicator, &rank);
   CHECK_MPI_ERROR(error);

   if (rank == 0) {
      for (int i = 0; i < fh_pmem->number_of_nodes; i++) {
         error = MPI_Send(&msg, 0, MPI_BYTE, fh_pmem->cache_managers[i], PMEM_TAG_SYNC, fh_pmem->communicator);
         CHECK_MPI_ERROR(error);
      }
   }

   error = MPI_Barrier(fh_pmem->communicator);
   CHECK_MPI_ERROR(error);

   error = MPI_File_sync(fh_pmem->file);
   CHECK_MPI_ERROR(error);

   return error;
}

bool get_boolean_value_from_info(MPI_Info info, char* key) {
   char* value_for_key = get_attribute_for_key(info, key);
   return value_for_key != NULL && !strcmp(value_for_key, "true");
}

char* get_attribute_for_key(MPI_Info info, char* key) {

   int length, flag;
   MPI_Info_get_valuelen(info, key, &length, &flag);

   if (!flag) {
      return 0;
   }

   char* value = (char*) malloc(length + 1);
   MPI_Info_get(info, key, length + 1, value, &flag);
   return value;
}

char* generate_cache_folder_path(char* pmem_path) {

   // some consts
   static const char* file_name_start = "/mpi_io_pmem_";
   static const int filename_length = 64;

   char file_name[64];

   // directory path
   strcpy(file_name, file_name_start);

   // timestamp part
   sprintf(file_name + strlen(file_name), "%s_", generate_timestamp());

   // random part
   sprintf(file_name + strlen(file_name), "%s", generate_random_string(filename_length - strlen(file_name) - 1));

   // complete path
   char* path = (char*) malloc(filename_length + strlen(pmem_path) + 1);
   strcpy(path, pmem_path);
   strcpy(path + strlen(pmem_path), file_name);
   path[strlen(path) - 2] = '/';
   path[strlen(path) - 1] = 0;
   return path;
}
