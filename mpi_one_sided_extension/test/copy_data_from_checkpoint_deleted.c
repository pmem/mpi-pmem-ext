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
   off_t file_size;
   MPI_Win_pmem_version *versions;
   MPI_Win_pmem win;
   void *win_data;
   MPI_Aint win_size = 1024;
   char copied_data[1024];
   int error_code;
   int result = 0;

   MPI_Init_thread_pmem(&argc, &argv, MPI_THREAD_MULTIPLE, &thread_support);
   sprintf(root_path, "%s/0", argv[1]);
   MPI_Win_pmem_set_root_path(root_path);

   // Create window and 3 checkpoints.
   allocate_window(&win, &win_data, "test_window", win_size);
   memset(win_data, 0, win_size);
   create_checkpoint(win, false);
   memset(win_data, 1, win_size);
   create_checkpoint(win, false);
   memset(win_data, 2, win_size);
   create_checkpoint(win, false);
   MPI_Win_free_pmem(&win);
   MPI_Win_pmem_delete_version("test_window", 1);
   open_versions_metadata_file(MPI_COMM_WORLD, "test_window", &versions, &file_size);

   // Create window object and copy data from checkpoint.
   set_default_window_metadata(&win, MPI_COMM_WORLD);
   strcpy(win.name, "test_window");
   win.mode = MPI_PMEM_MODE_CHECKPOINT;
   win.modifiable_values->last_checkpoint_version = 1;
   MPI_Comm_set_errhandler(MPI_COMM_WORLD, MPI_ERRORS_RETURN);
   error_code = copy_data_from_checkpoint(win, win_size, copied_data);
   MPI_Comm_set_errhandler(MPI_COMM_WORLD, MPI_ERRORS_ARE_FATAL);
   unmap_pmem_file(MPI_COMM_WORLD, versions, file_size);
   free(win.modifiable_values);

   if (error_code != MPI_ERR_PMEM) {
      mpi_log_error("Error code is: %d, expected %d", error_code, MPI_ERR_PMEM);
      result = 1;
   }

   MPI_Finalize_pmem();

   return result;
}
