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
   void *win_data;
   MPI_Win_pmem win;
   MPI_Win_pmem_windows windows;
   MPI_Win_pmem_versions versions;
   int result = 0;

   MPI_Init_thread_pmem(&argc, &argv, MPI_THREAD_MULTIPLE, &thread_support);
   sprintf(root_path, "%s/0", argv[1]);
   MPI_Win_pmem_set_root_path(root_path);

   // Create 3 windows, delete the middle one, create 3 checkpoints in remaining windows and delete one checkpoint version in each.
   allocate_window(&win, &win_data, "1", 512);
   create_checkpoint(win, false);
   create_checkpoint(win, false);
   create_checkpoint(win, false);
   MPI_Win_free_pmem(&win);
   allocate_window(&win, &win_data, "2", 1024);
   MPI_Win_free_pmem(&win);
   allocate_window(&win, &win_data, "3", 2048);
   create_checkpoint(win, false);
   create_checkpoint(win, false);
   create_checkpoint(win, false);
   MPI_Win_free_pmem(&win);
   MPI_Win_pmem_delete_version("1", 0);
   MPI_Win_pmem_delete("2");
   MPI_Win_pmem_delete_version("3", 1);

   // Retrieve and check metadata.
   MPI_Win_pmem_list(&windows);
   MPI_Win_pmem_get_versions(windows, 0, &versions);
   if (versions.size != 2) {
      mpi_log_error("Number of returned versions for window '1' is %d, expected 2.", versions.size);
      MPI_Finalize_pmem();
      return 1;
   }
   if (versions.versions[0].version != 1) {
      mpi_log_error("Version at index 0 of window '1' is %d, expected 1.", versions.versions[0].version);
      result = 1;
   }
   if (versions.versions[1].version != 2) {
      mpi_log_error("Version at index 1 of window '1' is %d, expected 2.", versions.versions[1].version);
      result = 1;
   }
   MPI_Win_pmem_get_versions(windows, 1, &versions);
   if (versions.size != 2) {
      mpi_log_error("Number of returned versions for window '3' is %d, expected 2.", versions.size);
      MPI_Finalize_pmem();
      return 1;
   }
   if (versions.versions[0].version != 0) {
      mpi_log_error("Version at index 0 of window '3' is %d, expected 0.", versions.versions[0].version);
      result = 1;
   }
   if (versions.versions[1].version != 2) {
      mpi_log_error("Version at index 1 of window '3' is %d, expected 2.", versions.versions[1].version);
      result = 1;
   }

   MPI_Finalize_pmem();

   return result;
}
