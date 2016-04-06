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


#include "helper.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <common/logger.h>
#include <mpi_one_sided_extension/mpi_win_pmem.h>
#include <mpi_one_sided_extension/mpi_win_pmem_helper.h>
#include <libpmem.h>

int create_window(const char *window_name, MPI_Aint size) {
   char *file_name;
   MPI_Win_pmem_version *versions;
   FILE *file;

   file_name = malloc((strlen(mpi_pmem_root_path) + strlen(window_name) + 3) * sizeof(char)); // Additional 3 characters for: "/." and terminating zero.
   if (file_name == NULL) {
      mpi_log_error("Unable to allocate memory.");
      return 1;
   }

   sprintf(file_name, "%s/.%s", mpi_pmem_root_path, window_name);
   create_window_metadata_file(MPI_COMM_WORLD, file_name, &versions, window_name, size);
   unmap_pmem_file(MPI_COMM_WORLD, versions, size);
   sprintf(file_name, "%s/%s", mpi_pmem_root_path, window_name);
   file = fopen(file_name, "w");
   if (file == NULL) {
      mpi_log_error("Unable to create data file '%s'", file_name);
      free(file_name);
      return 1;
   }
   fclose(file);

   free(file_name);

   return 0;
}

int check_versions_metadata_file(const char *window_name, bool exists, const MPI_Win_pmem_version *expected_versions, int versions_length) {
   int i;
   bool file_exists;
   char *file_name;
   off_t file_size;
   MPI_Win_pmem_version *versions;
   int result = 0;

   file_name = malloc((strlen(mpi_pmem_root_path) + strlen(window_name) + 3) * sizeof(char)); // Additional 3 characters for: "/." and terminating zero.
   if (file_name == NULL) {
      mpi_log_error("Unable to allocate memory.");
      return 1;
   }

   sprintf(file_name, "%s/.%s", mpi_pmem_root_path, window_name);
   file_exists = check_if_file_exist(file_name);
   if (!exists) {
      if (file_exists) {
         mpi_log_error("Versions metadata file '%s' exist, while it shouldn't.", file_name);
         free(file_name);
         return 1;
      }
      free(file_name);
      return 0;
   }
   if (!file_exists) {
      mpi_log_error("Versions metadata file '%s' doesn't exist, while it should.", file_name);
      free(file_name);
      return 1;
   }
   free(file_name);
   open_versions_metadata_file(MPI_COMM_WORLD, window_name, &versions, &file_size);

   // Check intermediate records.
   for (i = 0; i < versions_length; i++) {
      if (versions[i].flags != expected_versions[i].flags) {
         mpi_log_error("Flag at index %d equals %d, expected %d.", i, versions[i].flags, expected_versions[i].flags);
         result = 1;
      }
      if (expected_versions[i].timestamp == 0 && versions[i].timestamp != 0) {
         mpi_log_error("Timestamp at index %d equals %d, expected 0.", i, versions[i].timestamp);
         result = 1;
      }
      if (expected_versions[i].timestamp != 0 && versions[i].timestamp == 0) {
         mpi_log_error("Timestamp at index %d equals 0, expected non-zero.", i);
         result = 1;
      }
      if (versions[i].version != expected_versions[i].version) {
         mpi_log_error("Version at index %d equals %d, expected %d.", i, versions[i].version, expected_versions[i].version);
         result = 1;
      }
   }

   // Check terminating record.
   if (versions[versions_length].flags != 0) {
      mpi_log_error("Flag at index %d equals %d, expected 0.", versions_length, versions[versions_length].flags);
      result = 1;
   }
   if (versions[versions_length].timestamp != 0) {
      mpi_log_error("Timestamp at index %d equals %d, expected 0.", versions_length, versions[versions_length].timestamp);
      result = 1;
   }
   if (versions[versions_length].version != 0) {
      mpi_log_error("Version at index %d equals %d, expected 0.", versions_length, versions[versions_length].version);
      result = 1;
   }

   unmap_pmem_file(MPI_COMM_WORLD, versions, file_size);

   return result;
}

int check_global_metadata_file(const MPI_Win_pmem_metadata *expected_windows, int windows_length) {
   int i;
   off_t file_size;
   MPI_Win_pmem_metadata *windows;
   int result = 0;

   open_windows_metadata_file(MPI_COMM_WORLD, &windows, &file_size);

   // Check intermediate records.
   for (i = 0; i < windows_length; i++) {
      if (windows[i].flags != expected_windows[i].flags) {
         mpi_log_error("Flag at index %d equals %d, expected %d.", i, windows[i].flags, expected_windows[i].flags);
         result = 1;
      }
      if (windows[i].size != expected_windows[i].size) {
         mpi_log_error("Size at index %d equals %lu, expected %lu.", i, windows[i].size, expected_windows[i].size);
         result = 1;
      }
      if (strcmp(windows[i].name, expected_windows[i].name) != 0) {
         mpi_log_error("Name at index %d equals '%s', expected '%s'.", i, windows[i].name, expected_windows[i].name);
         result = 1;
      }
   }

   // Check terminating record.
   if (windows[windows_length].flags != 0) {
      mpi_log_error("Flag at index %d equals %d, expected 0.", windows_length, windows[windows_length].flags);
      result = 1;
   }
   if (windows[windows_length].size != 0) {
      mpi_log_error("Size at index %d equals %lu, expected 0.", windows_length, windows[windows_length].size);
      result = 1;
   }

   unmap_pmem_file(MPI_COMM_WORLD, windows, file_size);

   return result;
}

int check_checkpoint_versions(const MPI_Win_pmem win, int next_checkpoint_version, int last_checkpoint_version, int highest_checkpoint_version) {
   int result = 0;

   if (win.modifiable_values->next_checkpoint_version != next_checkpoint_version) {
      mpi_log_error("next_checkpoint_version is %d, expected %d", win.modifiable_values->next_checkpoint_version, next_checkpoint_version);
      result = 1;
   }
   if (win.modifiable_values->last_checkpoint_version != last_checkpoint_version) {
      mpi_log_error("last_checkpoint_version is %d, expected %d", win.modifiable_values->last_checkpoint_version, last_checkpoint_version);
      result = 1;
   }
   if (win.modifiable_values->highest_checkpoint_version != highest_checkpoint_version) {
      mpi_log_error("highest_checkpoint_version is %d, expected %d", win.modifiable_values->highest_checkpoint_version, highest_checkpoint_version);
      result = 1;
   }

   return result;
}

void allocate_window(MPI_Win_pmem *win, void **window_data, const char *window_name, MPI_Aint size) {
   MPI_Info info;

   MPI_Info_create(&info);
   MPI_Info_set(info, "pmem_is_pmem", "true");
   MPI_Info_set(info, "pmem_name", window_name);
   MPI_Info_set(info, "pmem_mode", "expand");
   MPI_Info_set(info, "pmem_keep_all_checkpoints", "true");
   MPI_Win_allocate_pmem(size, 1, info, MPI_COMM_WORLD, window_data, win);
   MPI_Info_free(&info);
}

int check_data(const char *data, MPI_Aint size, char value) {
   MPI_Aint i;

   for (i = 0; i < size; i++) {
      if (data[i] != value) {
         mpi_log_error("Value at index %d equals %d, expected %d.", i, data[i], value);
         return 1;
      }
   }

   return 0;
}

int check_if_file_exists_size_and_contents(const char *file_name, bool exists, MPI_Aint size, bool check_contents, char value) {
   bool file_exists;
   off_t file_size;
   int result = 0;
   void *data;

   file_exists = check_if_file_exist(file_name);
   if (!exists) {
      if (file_exists) {
         mpi_log_error("File '%s' exist, while it shouldn't.", file_name);
         return 1;
      }
      return 0;
   }
   if (!file_exists) {
      mpi_log_error("File '%s' doesn't exist, while it should.", file_name);
      return 1;
   }

   get_file_size(MPI_COMM_WORLD, file_name, &file_size);
   if (file_size != size) {
      mpi_log_error("File '%s' size is %lu, expected %lu.", file_name, file_size, size);
      return 1;
   }
   if (check_contents) {
      open_pmem_file(MPI_COMM_WORLD, file_name, size, &data);
      result |= check_data(data, size, value);
      unmap_pmem_file(MPI_COMM_WORLD, data, size);
   }

   return result;
}

int check_data_file(const char *window_name, bool exists, MPI_Aint size, bool check_contents, char value) {
   char *file_name;
   int result = 0;

   file_name = malloc((strlen(mpi_pmem_root_path) + strlen(window_name) + 2) * sizeof(char)); // Additional 2 characters for: "/" and terminating zero.
   if (file_name == NULL) {
      mpi_log_error("Unable to allocate memory.");
      return 1;
   }
   sprintf(file_name, "%s/%s", mpi_pmem_root_path, window_name);
   result |= check_if_file_exists_size_and_contents(file_name, exists, size, check_contents, value);
   free(file_name);

   return result;
}

int check_checkpoint_data(const char *window_name, int checkpoint_version, bool exists, MPI_Aint size, char value) {
   char *file_name;
   int result = 0;

   // Additional 14 characters for: "/.", "-", 10 characters for checkpoint number (length of maximum 4 byte integer number written in decimal form is 10 characters) and terminating zero.
   file_name = malloc((strlen(mpi_pmem_root_path) + strlen(window_name) + 14) * sizeof(char));
   if (file_name == NULL) {
      mpi_log_error("Unable to allocate memory.");
      return 1;
   }
   sprintf(file_name, "%s/.%s-%d", mpi_pmem_root_path, window_name, checkpoint_version);
   result |= check_if_file_exists_size_and_contents(file_name, exists, size, true, value);
   free(file_name);

   return result;
}

int check_window_object(const MPI_Win_pmem win, const MPI_Win_pmem expected, bool name, bool memory_areas) {
   int result = 0;

   if (win.comm != expected.comm) {
      mpi_log_error("Wrong communicator in window.");
      result = 1;
   }
   if (win.created_via_allocate != expected.created_via_allocate) {
      mpi_log_error("created_via_allocate is %s, expected %s.", win.created_via_allocate ? "true" : "false", expected.created_via_allocate ? "true" : "false");
      result = 1;
   }
   if (win.is_pmem != expected.is_pmem) {
      mpi_log_error("is_pmem is %s, expected %s.", win.is_pmem ? "true" : "false", expected.is_pmem ? "true" : "false");
      result = 1;
   }
   if (win.is_volatile != expected.is_volatile) {
      mpi_log_error("is_volatile is %s, expected %s.", win.is_volatile ? "true" : "false", expected.is_volatile ? "true" : "false");
      result = 1;
   }
   if (win.append_checkpoints != expected.append_checkpoints) {
      mpi_log_error("append_checkpoints is %s, expected %s.", win.append_checkpoints ? "true" : "false", expected.append_checkpoints ? "true" : "false");
      result = 1;
   }
   if (win.global_checkpoint != expected.global_checkpoint) {
      mpi_log_error("global_checkpoint is %s, expected %s.", win.global_checkpoint ? "true" : "false", expected.global_checkpoint ? "true" : "false");
      result = 1;
   }
   if (name && strcmp(win.name, expected.name) != 0) {
      mpi_log_error("name is '%s', expected '%s'.", win.name, expected.name);
      result = 1;
   }
   if (win.mode != expected.mode) {
      mpi_log_error("mode is %s, expected %s.",
                    win.mode == MPI_PMEM_MODE_EXPAND ? "expand" : win.mode == MPI_PMEM_MODE_CHECKPOINT ? "checkpoint" : "unknown",
                    expected.mode == MPI_PMEM_MODE_EXPAND ? "expand" : expected.mode == MPI_PMEM_MODE_CHECKPOINT ? "checkpoint" : "unknown");
      result = 1;
   }
   if (win.modifiable_values == NULL) {
      mpi_log_error("No modifiable values in window.");
      result = 1;
   } else {
      if (win.modifiable_values->transactional != expected.modifiable_values->transactional) {
         mpi_log_error("transactional is %s, expected %s.", win.modifiable_values->transactional ? "true" : "false", expected.modifiable_values->transactional ? "true" : "false");
         result = 1;
      }
      if (win.modifiable_values->keep_all_checkpoints != expected.modifiable_values->keep_all_checkpoints) {
         mpi_log_error("keep_all_checkpoints is %s, expected %s.", win.modifiable_values->keep_all_checkpoints ? "true" : "false", expected.modifiable_values->keep_all_checkpoints ? "true" : "false");
         result = 1;
      }
      if (win.modifiable_values->last_checkpoint_version != expected.modifiable_values->last_checkpoint_version) {
         mpi_log_error("last_checkpoint_version is %d, expected %d.", win.modifiable_values->last_checkpoint_version, expected.modifiable_values->last_checkpoint_version);
         result = 1;
      }
      if (win.modifiable_values->next_checkpoint_version != expected.modifiable_values->next_checkpoint_version) {
         mpi_log_error("next_checkpoint_version is %d, expected %d.", win.modifiable_values->next_checkpoint_version, expected.modifiable_values->next_checkpoint_version);
         result = 1;
      }
      if (win.modifiable_values->highest_checkpoint_version != expected.modifiable_values->highest_checkpoint_version) {
         mpi_log_error("highest_checkpoint_version is %d, expected %d.", win.modifiable_values->highest_checkpoint_version, expected.modifiable_values->highest_checkpoint_version);
         result = 1;
      }
      if (memory_areas && win.modifiable_values->memory_areas == NULL) {
         mpi_log_error("Memory areas is empty, while it should not.");
         result = 1;
      }
      if (!memory_areas && win.modifiable_values->memory_areas != NULL) {
         mpi_log_error("Memory areas is not empty, while it should.");
         result = 1;
      }
   }

   return result;
}

int check_memory_areas_list_element(const MPI_Win_memory_areas_list *element, void *base, MPI_Aint size, bool win_is_pmem, bool next) {
   int result = 0;
   int expected_is_pmem;

   if (element->base != base) {
      mpi_log_error("Memory area base is 0x%lx, expected 0x%lx.", element->base, base);
      result = 1;
   }
   if (element->size != size) {
      mpi_log_error("Memory area at 0x%lx size is %lu, expected %lu.", element->base, element->size, size);
      result = 1;
   }
   if (win_is_pmem) {
      expected_is_pmem = pmem_is_pmem(element->base, element->size);
      if (element->is_pmem != expected_is_pmem) {
         mpi_log_error("Memory area at 0x%lx is_pmem is %d, expected %d.", element->is_pmem, expected_is_pmem);
         result = 1;
      }
   }
   if (next && element->next == NULL) {
      mpi_log_error("There is no next element, while there should be.");
      result = 1;
   }
   if (!next && element->next != NULL) {
      mpi_log_error("There is a next element, while there should not be.");
      result = 1;
   }

   return result;
}
