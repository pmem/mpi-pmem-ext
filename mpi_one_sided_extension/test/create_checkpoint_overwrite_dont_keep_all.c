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
#include <common/logger.h>
#include <common/mpi_init_pmem.h>
#include <mpi_one_sided_extension/mpi_win_pmem.h>
#include <mpi_one_sided_extension/mpi_win_pmem_helper.h>
#include "helper.h"

int main(int argc, char *argv[]) {
   int thread_support;
   char root_path[MPI_PMEM_MAX_ROOT_PATH];
   MPI_Info info;
   MPI_Win_pmem win, expected_win;
   char *window_name = "test_window";
   char *win_data;
   MPI_Aint win_size = 1024;
   MPI_Win_pmem_version versions[4];
   MPI_Win_pmem_metadata windows[1];
   int result = 0;

   MPI_Init_thread_pmem(&argc, &argv, MPI_THREAD_MULTIPLE, &thread_support);
   sprintf(root_path, "%s/0", argv[1]);
   MPI_Win_pmem_set_root_path(root_path);

   // Allocate window, create 3 checkpoints and free window.
   MPI_Info_create(&info);
   MPI_Info_set(info, "pmem_is_pmem", "true");
   MPI_Info_set(info, "pmem_name", window_name);
   MPI_Info_set(info, "pmem_mode", "expand");
   MPI_Info_set(info, "pmem_keep_all_checkpoints", "true");
   MPI_Win_allocate_pmem(win_size, 1, info, MPI_COMM_WORLD, &win_data, &win);
   memset(win_data, 0, win_size);
   create_checkpoint(win, false);
   memset(win_data, 1, win_size);
   create_checkpoint(win, false);
   memset(win_data, 2, win_size);
   create_checkpoint(win, false);
   MPI_Win_free_pmem(&win);

   // Reallocate window with loading from second checkpoint.
   MPI_Info_set(info, "pmem_mode", "checkpoint");
   MPI_Info_set(info, "pmem_checkpoint_version", "0");
   MPI_Info_set(info, "pmem_keep_all_checkpoints", "false");
   MPI_Win_allocate_pmem(win_size, 1, info, MPI_COMM_WORLD, &win_data, &win);
   MPI_Info_free(&info);

   // Prepare expected result.
   set_default_window_metadata(&expected_win, MPI_COMM_WORLD);
   expected_win.created_via_allocate = true;
   expected_win.is_pmem = true;
   strcpy(expected_win.name, window_name);
   expected_win.mode = MPI_PMEM_MODE_CHECKPOINT;
   expected_win.modifiable_values->transactional = true;
   expected_win.modifiable_values->keep_all_checkpoints = false;
   versions[0].flags = MPI_PMEM_FLAG_OBJECT_EXISTS;
   versions[0].timestamp = 1;
   versions[0].version = 0;
   versions[1].flags = MPI_PMEM_FLAG_OBJECT_EXISTS;
   versions[1].timestamp = 1;
   versions[1].version = 1;
   versions[2].flags = MPI_PMEM_FLAG_OBJECT_EXISTS;
   versions[2].timestamp = 1;
   versions[2].version = 2;
   versions[3].flags = MPI_PMEM_FLAG_OBJECT_EXISTS;
   versions[3].timestamp = 1;
   versions[3].version = 3;
   windows[0].flags = MPI_PMEM_FLAG_OBJECT_EXISTS;
   windows[0].size = win_size;
   strcpy(windows[0].name, window_name);

   // Create first checkpoint and check result.
   memset(win_data, 3, win_size);
   create_checkpoint(win, false);
   expected_win.modifiable_values->last_checkpoint_version = 1;
   expected_win.modifiable_values->next_checkpoint_version = 2;
   expected_win.modifiable_values->highest_checkpoint_version = 2;
   versions[0].flags = MPI_PMEM_FLAG_OBJECT_DELETED;
   result |= check_window_object(win, expected_win, true, true);
   result |= check_memory_areas_list_element(win.modifiable_values->memory_areas, win_data, win_size, true, false);
   result |= check_checkpoint_data(window_name, 0, false, win_size, 0);
   result |= check_checkpoint_data(window_name, 1, true, win_size, 3);
   result |= check_checkpoint_data(window_name, 2, true, win_size, 2);
   result |= check_versions_metadata_file(window_name, true, versions, 3);
   result |= check_global_metadata_file(windows, 1);
   if (result != 0) {
      free(expected_win.modifiable_values);
      MPI_Win_free_pmem(&win);
      MPI_Finalize_pmem();
      return result;
   }

   // Create second checkpoint and check result.
   memset(win_data, 4, win_size);
   create_checkpoint(win, false);
   expected_win.modifiable_values->last_checkpoint_version = 2;
   expected_win.modifiable_values->next_checkpoint_version = 3;
   expected_win.modifiable_values->highest_checkpoint_version = 2;
   versions[1].flags = MPI_PMEM_FLAG_OBJECT_DELETED;
   result |= check_window_object(win, expected_win, true, true);
   result |= check_memory_areas_list_element(win.modifiable_values->memory_areas, win_data, win_size, true, false);
   result |= check_checkpoint_data(window_name, 0, false, win_size, 0);
   result |= check_checkpoint_data(window_name, 1, false, win_size, 3);
   result |= check_checkpoint_data(window_name, 2, true, win_size, 4);
   result |= check_versions_metadata_file(window_name, true, versions, 3);
   result |= check_global_metadata_file(windows, 1);
   if (result != 0) {
      free(expected_win.modifiable_values);
      MPI_Win_free_pmem(&win);
      MPI_Finalize_pmem();
      return result;
   }

   // Create third checkpoint and check result.
   memset(win_data, 5, win_size);
   create_checkpoint(win, false);
   expected_win.modifiable_values->last_checkpoint_version = 3;
   expected_win.modifiable_values->next_checkpoint_version = 4;
   expected_win.modifiable_values->highest_checkpoint_version = 3;
   versions[2].flags = MPI_PMEM_FLAG_OBJECT_DELETED;
   result |= check_window_object(win, expected_win, true, true);
   result |= check_memory_areas_list_element(win.modifiable_values->memory_areas, win_data, win_size, true, false);
   result |= check_checkpoint_data(window_name, 0, false, win_size, 0);
   result |= check_checkpoint_data(window_name, 1, false, win_size, 3);
   result |= check_checkpoint_data(window_name, 2, false, win_size, 4);
   result |= check_checkpoint_data(window_name, 3, true, win_size, 5);
   result |= check_versions_metadata_file(window_name, true, versions, 4);
   result |= check_global_metadata_file(windows, 1);

   free(expected_win.modifiable_values);
   MPI_Win_free_pmem(&win);
   MPI_Finalize_pmem();

   return result;
}
