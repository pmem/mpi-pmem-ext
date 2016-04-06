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
#include <string.h>
#include <common/logger.h>
#include <common/mpi_init_pmem.h>
#include <mpi_one_sided_extension/mpi_win_pmem.h>
#include <mpi_one_sided_extension/mpi_win_pmem_helper.h>
#include "helper.h"

int main(int argc, char *argv[]) {
   int thread_support;
   char root_path[MPI_PMEM_MAX_ROOT_PATH];
   char *window_name = "test_window";
   void *win_data;
   MPI_Aint win_size = 1024;
   MPI_Win_pmem win;
   MPI_Win_pmem_metadata windows[1];
   int result = 0;

   MPI_Init_thread_pmem(&argc, &argv, MPI_THREAD_MULTIPLE, &thread_support);
   sprintf(root_path, "%s/0", argv[1]);
   MPI_Win_pmem_set_root_path(root_path);

   // Create window and 3 checkpoints.
   allocate_window(&win, &win_data, window_name, win_size);
   create_checkpoint(win, false);
   create_checkpoint(win, false);
   create_checkpoint(win, false);
   MPI_Win_free_pmem(&win);

   // Prepare expected results.
   windows[0].flags = MPI_PMEM_FLAG_OBJECT_DELETED;
   windows[0].size = win_size;
   strcpy(windows[0].name, window_name);

   // Delete window and check result.
   MPI_Win_pmem_delete(window_name);
   result |= check_data_file(window_name, false, win_size, false, 0);
   result |= check_checkpoint_data(window_name, 0, false, win_size, 0);
   result |= check_checkpoint_data(window_name, 1, false, win_size, 0);
   result |= check_checkpoint_data(window_name, 2, false, win_size, 0);
   result |= check_versions_metadata_file(window_name, false, NULL, 0);
   result |= check_global_metadata_file(windows, 1);

   MPI_Finalize_pmem();

   return result;
}
