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

int main(int argc, char *argv[]) {
   int thread_support;
   char root_path[MPI_PMEM_MAX_ROOT_PATH];
   FILE *file;
   int error_code;
   int result = 0;

   MPI_Init_thread_pmem(&argc, &argv, MPI_THREAD_MULTIPLE, &thread_support);

   sprintf(root_path, "%s/0/file", argv[1]);
   file = fopen(root_path, "w");
   if (file == NULL) {
      mpi_log_error("Unable to create file '%s'", root_path);
      MPI_Finalize_pmem();
      return 1;
   }
   fclose(file);
   MPI_Comm_set_errhandler(MPI_COMM_WORLD, MPI_ERRORS_RETURN);
   error_code = MPI_Win_pmem_set_root_path(root_path);
   MPI_Comm_set_errhandler(MPI_COMM_WORLD, MPI_ERRORS_ARE_FATAL);

   if (error_code != MPI_ERR_PMEM_ROOT_PATH) {
      mpi_log_error("Error code is %d, expected %d.", error_code, MPI_ERR_PMEM_ROOT_PATH);
      result = 1;
   }

   MPI_Finalize_pmem();

   return result;
}
