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


#include "mpi_init_pmem.h"
#include "error_codes.h"
#include "logger.h"
#include <mpi.h>

int MPI_ERR_PMEM;
int MPI_ERR_PMEM_ROOT_PATH;
int MPI_ERR_PMEM_NAME;
int MPI_ERR_PMEM_CKPT_VER;
int MPI_ERR_PMEM_MODE;
int MPI_ERR_PMEM_WINDOWS;
int MPI_ERR_PMEM_VERSIONS;
int MPI_ERR_PMEM_ARG;
int MPI_ERR_PMEM_NO_MEM;

/**
 * Initialize error codes by adding information about them to MPI as described in MPI specification.
 *
 * @returns Error code as described in MPI specification.
 */
int init_error_codes() {
   int result, i;
   int error_codes_count = 8;
   int *error_codes[] = {
      &MPI_ERR_PMEM_ROOT_PATH,
      &MPI_ERR_PMEM_NAME,
      &MPI_ERR_PMEM_CKPT_VER,
      &MPI_ERR_PMEM_MODE,
      &MPI_ERR_PMEM_WINDOWS,
      &MPI_ERR_PMEM_VERSIONS,
      &MPI_ERR_PMEM_ARG,
      &MPI_ERR_PMEM_NO_MEM };
   char *error_strings[] = {
      "Invalid root path",
      "Invalid persistent memory area name",
      "Invalid checkpoint version",
      "Invalid mode",
      "Invalid windows argument",
      "Invalid versions argument",
      "Invalid argument of some other kind",
      "Unable to allocate memory" };

   mpi_log_debug("Initializing error codes.");

   // Add global error class
   result = MPI_Add_error_class(&MPI_ERR_PMEM);
   CHECK_ERROR_CODE(result);
   result = MPI_Add_error_string(MPI_ERR_PMEM, "General MPI PMEM error");
   CHECK_ERROR_CODE(result);

   // Add individual error codes to class.
   for (i = 0; i < error_codes_count; i++) {
      result = MPI_Add_error_code(MPI_ERR_PMEM, error_codes[i]);
      CHECK_ERROR_CODE(result);
      result = MPI_Add_error_string(*error_codes[i], error_strings[i]);
      CHECK_ERROR_CODE(result);
   }

   mpi_log_debug("Error codes initialized.");

   return MPI_SUCCESS;
}

int MPI_Init_pmem(int *argc, char ***argv) {
   int result;

   log_debug("Initializing MPI.");

   result = MPI_Init(argc, argv);
   CHECK_ERROR_CODE(result);

   log_debug("MPI initialized.");

   return init_error_codes();
}

int MPI_Init_thread_pmem(int *argc, char ***argv, int required, int *provided) {
   int result;

   log_debug("Initializing MPI with threads.");

   result = MPI_Init_thread(argc, argv, required, provided);
   CHECK_ERROR_CODE(result);
   result = init_mpi_logging();
   CHECK_ERROR_CODE(result);

   mpi_log_debug("MPI initialized with threads.");

   return init_error_codes();
}

int MPI_Finalize_pmem() {
   mpi_log_debug("Finalizing MPI.");

   deinit_mpi_logging();
   MPI_Finalize();

   log_debug("MPI finalized.");

   return MPI_SUCCESS;
}
