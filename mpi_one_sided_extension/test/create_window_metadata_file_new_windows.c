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
   MPI_Win_pmem_metadata windows[3];
   int result = 0;

   MPI_Init_thread_pmem(&argc, &argv, MPI_THREAD_MULTIPLE, &thread_support);
   sprintf(root_path, "%s/0", argv[1]);
   MPI_Win_pmem_set_root_path(root_path);

   // Prepare expected global metadata.
   windows[0].flags = MPI_PMEM_FLAG_OBJECT_EXISTS;
   windows[0].size = 512;
   strcpy(windows[0].name, "1");
   windows[1].flags = MPI_PMEM_FLAG_OBJECT_EXISTS;
   windows[1].size = 1024;
   strcpy(windows[1].name, "2");
   windows[2].flags = MPI_PMEM_FLAG_OBJECT_EXISTS;
   windows[2].size = 2048;
   strcpy(windows[2].name, "3");

   // Create first window and check metadata.
   result |= create_window("1", 512);
   result |= check_versions_metadata_file("1", true, NULL, 0);
   result |= check_global_metadata_file(windows, 1);
   if (result != 0) {
      MPI_Finalize_pmem();
      return result;
   }
   
   // Create second window and check metadata.
   result |= create_window("2", 1024);
   result |= check_versions_metadata_file("1", true, NULL, 0);
   result |= check_versions_metadata_file("2", true, NULL, 0);
   result |= check_global_metadata_file(windows, 2);
   if (result != 0) {
      MPI_Finalize_pmem();
      return result;
   }

   // Create third window and check metadata.
   result |= create_window("3", 2048);
   result |= check_versions_metadata_file("1", true, NULL, 0);
   result |= check_versions_metadata_file("2", true, NULL, 0);
   result |= check_versions_metadata_file("3", true, NULL, 0);
   result |= check_global_metadata_file(windows, 3);

   MPI_Finalize_pmem();

   return result;
}
