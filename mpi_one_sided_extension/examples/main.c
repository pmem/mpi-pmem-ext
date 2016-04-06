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
#include <mpi.h>
#include <common/logger.h>
#include <common/mpi_init_pmem.h>
#include <mpi_one_sided_extension/mpi_win_pmem.h>

#include <mpi_one_sided_extension/defines.h>

int main(int argc, char *argv[]) {
   MPI_Win win;
   MPI_Info info;
   MPI_Win_pmem_windows windows;
   MPI_Win_pmem_versions versions;
   MPI_Win_memory_areas_list *memory_area;
   MPI_Aint address;
   int i, j;
   int provided_thread_support;
   int rank;
   int proc_count;
   char root_path[256];
   int *win_data;
   int win_size = 256 * sizeof(int);
   int data_to_copy[256];
   int info_nkeys, info_valuelen, info_flag;
   char info_key[MPI_MAX_INFO_KEY];
   char *info_value;
   int windows_count, versions_count;
   MPI_Aint window_size;
   char window_name[MPI_PMEM_MAX_NAME];
   int version;
   time_t version_timestamp;
   bool create = false;
   bool dynamic = false;
   bool checkpoint = false;
   char *checkpoint_version = NULL;
   bool delete_window = false;
   bool delete_version = false;
   int window_version_to_delete = 0;

   log_info("Starting main.");

   if (argc < 2) {
      log_error("Wrong arguments. Usage %s <root-path> [-create | -dynamic] [-checkpoint] [-version <checkpoint version>] [-delete-window | -delete-version <checkpoint-version>]", argv[0]);
      return 1;
   }

   for (i = 2; i < argc; i++) {
      if (strcmp(argv[i], "-create") == 0) {
         create = true;
      } else if (strcmp(argv[i], "-dynamic") == 0) {
         dynamic = true;
      } else if (strcmp(argv[i], "-checkpoint") == 0) {
         checkpoint = true;
      } else if (strcmp(argv[i], "-version") == 0) {
         checkpoint_version = argv[i + 1];
         i++;
      } else if (strcmp(argv[i], "-delete-window") == 0) {
         delete_window = true;
      } else if (strcmp(argv[i], "-delete-version") == 0) {
         delete_version = true;
         window_version_to_delete = atoi(argv[i + 1]);
         i++;
      }
   }
   if (create && dynamic) {
      log_error("-create and -dynamic cannot be used together.");
      return 1;
   }
   if (delete_window && delete_version) {
      log_error("-delete-window and -delete-version cannot be used together.");
      return 1;
   }

   // Prepare simple array with some data for communication.
   for (i = 0; i < 256; i++) {
      data_to_copy[i] = 255 - i;
   }

   // Initialize MPI.
   MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided_thread_support);
   if (provided_thread_support != MPI_THREAD_MULTIPLE) {
      log_error("MPI doesn't support MPI_THREAD_MULTIPLE");
      MPI_Abort(MPI_COMM_WORLD, 1);
      MPI_Finalize_pmem();
      return 1;
   }
   MPI_Comm_rank(MPI_COMM_WORLD, &rank);
   MPI_Comm_size(MPI_COMM_WORLD, &proc_count);

   // Create MPI_Info object.
   sprintf(root_path, "%s/%d", argv[1], rank);
   MPI_Win_pmem_set_root_path(root_path);
   MPI_Info_create(&info);
   MPI_Info_set(info, "pmem_is_pmem", "true");
   MPI_Info_set(info, "pmem_dont_use_transactions", "true");
   MPI_Info_set(info, "pmem_keep_all_checkpoints", "true");
   MPI_Info_set(info, "pmem_volatile", "false");
   MPI_Info_set(info, "pmem_name", "main_window");
   if (checkpoint) {
      MPI_Info_set(info, "pmem_mode", "checkpoint");
   } else {
      MPI_Info_set(info, "pmem_mode", "expand");
   }
   if (checkpoint_version != NULL) {
      MPI_Info_set(info, "pmem_checkpoint_version", checkpoint_version);
   }
   MPI_Info_set(info, "pmem_append_checkpoints", "true");
   MPI_Info_set(info, "pmem_global_checkpoint", "true");

   // Create/allocate window.
   if (create || dynamic) {
      win_data = malloc(win_size);
      if (create) {
         MPI_Win_create(win_data, win_size, sizeof(int), info, MPI_COMM_WORLD, &win);
      } else {
         MPI_Win_create_dynamic(info, MPI_COMM_WORLD, &win);
      }
   } else {
      MPI_Win_allocate(win_size, sizeof(int), info, MPI_COMM_WORLD, &win_data, &win);
   }

   // Check working of MPI_Win_set_info and MPI_Win_get_info.
   MPI_Info_set(info, "pmem_dont_use_transactions", "false");
   MPI_Info_set(info, "pmem_keep_all_checkpoints", "true");
   MPI_Win_set_info(win, info);
   MPI_Info_free(&info);
   MPI_Win_get_info(win, &info);

   // Parse MPI_Info object obtained using MPI_Win_get_info.
   MPI_Info_get_nkeys(info, &info_nkeys);
   mpi_log_info("Parsing MPI_Info.");
   for (i = 0; i < info_nkeys; i++) {
      MPI_Info_get_nthkey(info, i, info_key);
      MPI_Info_get_valuelen(info, info_key, &info_valuelen, &info_flag);
      info_value = malloc((info_valuelen + 1) * sizeof(char));
      MPI_Info_get(info, info_key, info_valuelen + 1, info_value, &info_flag);
      mpi_log_info("%s: %s", info_key, info_value);
      free(info_value);
   }
   mpi_log_info("MPI_Info parsed.");
   MPI_Info_free(&info);

   // Attach memory areas to window.
   if (dynamic) {
      MPI_Win_attach(win, win_data, 64 * sizeof(int));
      MPI_Win_attach(win, &win_data[64], 64 * sizeof(int));
      MPI_Win_attach(win, &win_data[128], 128 * sizeof(int));
   }

   // Manually parse information set during window creation/allocation.
   mpi_log_info("Parsing MPI_Win_pmem.");
   mpi_log_info("created_via_allocate: %s", win.created_via_allocate ? "true" : "false");
   mpi_log_info("is_pmem: %s", win.is_pmem ? "true" : "false");
   mpi_log_info("is_volatile: %s", win.is_volatile ? "true" : "false");
   mpi_log_info("append_checkpoints: %s", win.append_checkpoints ? "true" : "false");
   mpi_log_info("global_checkpoint: %s", win.global_checkpoint ? "true" : "false");
   mpi_log_info("name: %s", win.name);
   mpi_log_info("mode: %s", win.mode == MPI_PMEM_MODE_EXPAND ? "expand" : "checkpoint");
   mpi_log_info("transactional: %s", win.modifiable_values->transactional ? "true" : "false");
   mpi_log_info("keep_all_checkpoints: %s", win.modifiable_values->keep_all_checkpoints ? "true" : "false");
   mpi_log_info("last_checkpoint_version: %d", win.modifiable_values->last_checkpoint_version);
   mpi_log_info("next_checkpoint_version: %d", win.modifiable_values->next_checkpoint_version);
   mpi_log_info("highest_checkpoint_version: %d", win.modifiable_values->highest_checkpoint_version);
   memory_area = win.modifiable_values->memory_areas;
   while (memory_area != NULL) {
      mpi_log_info("base: 0x%lx, size: %lu, is_pmem: %s", (long int*) memory_area->base, memory_area->size, memory_area->is_pmem ? "true" : "false");
      memory_area = memory_area->next;
   }
   mpi_log_info("MPI_Win_pmem parsed.");

   // Put some data to your own window.
   MPI_Win_fence(0, win);
   for (i = 0; i < 256; i++) {
      if (dynamic) {
         MPI_Address(&win_data[i], &address);
         MPI_Put(&data_to_copy[i], 1, MPI_INT, rank, address, 1, MPI_INT, win);
      } else {
         MPI_Put(&data_to_copy[i], 1, MPI_INT, rank, i, 1, MPI_INT, win);
      }
   }
   MPI_Win_fence_pmem_persist(0, win);

   // Detach memory areas from window.
   if (dynamic) {
      MPI_Win_detach(win, win_data);
      MPI_Win_detach(win, &win_data[128]);
   }

   // Check if data were properly put into window.
   if (rank == 0) {
      for (i = 0; i < 16; i++) {
         mpi_log_info("%d ", win_data[i]);
      }
   }

   MPI_Win_free(&win);
   if (create || dynamic) {
      free(win_data);
   }

   // Check managing functions by parsing all information about all versions of all windows.
   if (delete_window) {
      MPI_Win_pmem_delete("main_window");
   }
   if (delete_version) {
      MPI_Win_pmem_delete_version("main_window", window_version_to_delete);
   }
   mpi_log_info("Getting info about available windows.");
   MPI_Win_pmem_list(&windows);
   MPI_Win_pmem_get_nwindows(windows, &windows_count);
   for (i = 0; i < windows_count; i++) {
      MPI_Win_pmem_get_name(windows, i, window_name);
      MPI_Win_pmem_get_size(windows, i, &window_size);
      mpi_log_info("%s: %lu", window_name, window_size);
      MPI_Win_pmem_get_versions(windows, i, &versions);
      MPI_Win_pmem_get_nversions(versions, &versions_count);
      for (j = 0; j < versions_count; j++) {
         MPI_Win_pmem_get_version(versions, j, &version);
         MPI_Win_pmem_get_version_timestamp(versions, j, &version_timestamp);
         mpi_log_info("%d: %s", version, ctime(&version_timestamp));
      }
      MPI_Win_pmem_free_versions_list(&versions);
   }
   MPI_Win_pmem_free_windows_list(&windows);
   mpi_log_info("End of getting info about available windows.");

   MPI_Finalize();

   log_info("Ending main.");

   return 0;
}
