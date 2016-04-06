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


#ifndef __LOGGER_H__
#define __LOGGER_H__

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

   bool is_error_log_enabled();
   bool is_debug_log_enabled();
   bool is_info_log_enabled();

   void log_error(const char *format, ...);
   void log_debug(const char *format, ...);
   void log_info(const char *format, ...);

   /**
    * Initialize conflictless logging from all processes in MPI application. Initialization consists of creating new communicator (duplicate of MPI_COMM_WORLD) for communication and starting new thread
    * on process with rank 0.
    *
    * @returns Error code as described in MPI specification.
    */
   int init_mpi_logging();

   /**
    * Deinitialize conflictless logging from all processes in MPI application. Deinitialization is done by sending shutdown message to logging thread by process with rank 0.
    */
   void deinit_mpi_logging();

   void mpi_log_error(const char *format, ...);
   void mpi_log_debug(const char *format, ...);
   void mpi_log_info(const char *format, ...);

#ifdef __cplusplus
}
#endif

#endif
