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
#include "helper.h"

int main(int argc, char *argv[]) {
   int thread_support;
   char root_path[MPI_PMEM_MAX_ROOT_PATH];
   MPI_Win_pmem_windows windows;
   int result = 0;

   MPI_Init_thread_pmem(&argc, &argv, MPI_THREAD_MULTIPLE, &thread_support);
   sprintf(root_path, "%s/0", argv[1]);
   MPI_Win_pmem_set_root_path(root_path);

   // Create 3 windows and delete the middle one.
   result |= create_window("1", 512);
   result |= create_window("2", 1024);
   result |= create_window("3", 2048);
   if (result != 0) {
      MPI_Finalize_pmem();
      return result;
   }
   MPI_Win_pmem_delete("2");

   MPI_Win_pmem_list(&windows);

   if (windows.size != 2) {
      mpi_log_error("Number of returned windows is %d, expected 2.", windows.size);
      MPI_Finalize_pmem();
      return 1;
   }
   if (strcmp(windows.windows[0].name, "1") != 0) {
      mpi_log_error("Window at index 0 name is '%s', expected '1'.", windows.windows[0].name);
      result = 1;
   }
   if (windows.windows[0].size != 512) {
      mpi_log_error("Window at index 0 size is %lu, expected 512.", windows.windows[0].size);
      result = 1;
   }
   if (strcmp(windows.windows[1].name, "3") != 0) {
      mpi_log_error("Window at index 1 name is '%s', expected '3'.", windows.windows[1].name);
      result = 1;
   }
   if (windows.windows[1].size != 2048) {
      mpi_log_error("Window at index 1 size is %lu, expected 2048.", windows.windows[1].size);
      result = 1;
   }

   MPI_Finalize_pmem();

   return result;
}
