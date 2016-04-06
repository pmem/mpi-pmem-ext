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
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <mpi.h>
#include <pthread.h>

#include "logger.h"
#include "error_codes.h"
#include "util.h"

#define LOG_START_ERROR "ERROR: "
#define LOG_START_INFO " INFO: "
#define LOG_START_DEBUG "DEBUG: "

// Maximum length of log message in bytes that may be used in mpi_log_xxx() functions.
#define LOG_MAX_SIZE 512

// Tags for logging messages.
#define LOG_MSG_ERROR 0
#define LOG_MSG_DEBUG 1
#define LOG_MSG_INFO 2
#define LOG_MSG_SHUTDOWN 3

// Communicator for conflictless logging.
MPI_Comm mpi_log_comm;

// Thread on root process (rank == 0) waiting for messages to log.
pthread_t mpi_log_thread;

bool is_error_log_enabled() {

#ifdef _LOG_ERROR
   return true;
#endif

#ifdef _LOG_INFO
   return true;
#endif

#ifdef _LOG_DEBUG
   return true;
#endif

   return false;
}

bool is_info_log_enabled() {

#ifdef _LOG_INFO
   return true;
#endif

#ifdef _LOG_DEBUG
   return true;
#endif

   return false;
}

bool is_debug_log_enabled() {

#ifdef _LOG_DEBUG
   return true;
#endif

   return false;
}

void log_error(const char *format, ...) {
   if (is_error_log_enabled()) {
      va_list args;
      va_start(args, format);
      printf(LOG_START_ERROR);
      vprintf(format, args);
      printf("\n");
      va_end(args);
   }
}

void log_debug(const char *format, ...) {
   if (is_debug_log_enabled()) {
      va_list args;
      va_start(args, format);
      printf(LOG_START_DEBUG);
      vprintf(format, args);
      printf("\n");
      va_end(args);
   }
}

void log_info(const char *format, ...) {
   if (is_info_log_enabled()) {
      va_list args;
      va_start(args, format);
      printf(LOG_START_INFO);
      vprintf(format, args);
      printf("\n");
      va_end(args);
   }
}

/**
 * Thread function for root process. It loops waiting for messages from all processes (in duplicated MPI_COMM_WORLD communicator) and prints them to stdout.
 *
 * @param unused Unused param.
 *
 * @returns NULL
 */
void* mpi_log_thread_function(void* unused) {
   UNUSED(unused);

   bool quit = false;
   char log_message[LOG_MAX_SIZE];
   MPI_Status status;

   while (!quit) {
      MPI_Recv(log_message, LOG_MAX_SIZE, MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, mpi_log_comm, &status);
      if (status.MPI_TAG == LOG_MSG_SHUTDOWN) {
         quit = true;
      } else {
         switch (status.MPI_TAG) {
         case LOG_MSG_ERROR:
            printf(LOG_START_ERROR);
            break;
         case LOG_MSG_DEBUG:
            printf(LOG_START_DEBUG);
            break;
         case LOG_MSG_INFO:
            printf(LOG_START_INFO);
            break;
         default:
            break;
         }
         printf("Rank: %d: %s\n", status.MPI_SOURCE, log_message);
      }
   }

   return NULL;
}

int init_mpi_logging() {
   int result;
   int new_rank, old_rank;

   if (is_error_log_enabled() || is_debug_log_enabled() || is_info_log_enabled()) {
      log_debug("Initializing MPI logging.");

      // Duplicate MPI_COMM_WORLD communicator and make sanity check of ranks in both (MPI_COMM_WORLD and new duplicate) communicators.
      result = MPI_Comm_dup(MPI_COMM_WORLD, &mpi_log_comm);
      CHECK_ERROR_CODE(result);
      result = MPI_Comm_rank(mpi_log_comm, &new_rank);
      CHECK_ERROR_CODE(result);
      result = MPI_Comm_rank(MPI_COMM_WORLD, &old_rank);
      CHECK_ERROR_CODE(result);
      if (new_rank != old_rank) {
         log_error("Ranks don't match.");
         MPI_Comm_call_errhandler(MPI_COMM_WORLD, MPI_ERR_OTHER);
         return MPI_ERR_OTHER;
      }

      // Start logging thread on process with rank 0.
      if (new_rank == 0) {
         pthread_attr_t attr;
         pthread_attr_init(&attr);
         pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
         result = pthread_create(&mpi_log_thread, &attr, mpi_log_thread_function, NULL);
         if (result != 0) {
            log_error("Unable to start logging thread.");
            MPI_Comm_call_errhandler(MPI_COMM_WORLD, MPI_ERR_OTHER);
            return MPI_ERR_OTHER;
         }
         pthread_attr_destroy(&attr);
      }
      mpi_log_debug("MPI logging initialized.");
   }

   return MPI_SUCCESS;
}

void deinit_mpi_logging() {
   int rank;

   if (is_error_log_enabled() || is_debug_log_enabled() || is_info_log_enabled()) {
      mpi_log_debug("Deinitializing MPI logging.");
      MPI_Barrier(mpi_log_comm);
      MPI_Comm_rank(mpi_log_comm, &rank);
      if (rank == 0) {
         MPI_Send(NULL, 0, MPI_CHAR, 0, LOG_MSG_SHUTDOWN, mpi_log_comm);
         pthread_join(mpi_log_thread, NULL);
      }
      MPI_Comm_free(&mpi_log_comm);
      log_debug("Rank: %d: MPI logging deinitialized.", rank);
   }
}

void mpi_log_error(const char *format, ...) {
   char log_message[LOG_MAX_SIZE];

   if (is_error_log_enabled()) {
      va_list args;
      va_start(args, format);
      vsprintf(log_message, format, args);
      MPI_Send(log_message, strlen(log_message) + 1, MPI_CHAR, 0, LOG_MSG_ERROR, mpi_log_comm);
      va_end(args);
   }
}

void mpi_log_debug(const char *format, ...) {
   char log_message[LOG_MAX_SIZE];

   if (is_debug_log_enabled()) {
      va_list args;
      va_start(args, format);
      vsprintf(log_message, format, args);
      MPI_Send(log_message, strlen(log_message) + 1, MPI_CHAR, 0, LOG_MSG_DEBUG, mpi_log_comm);
      va_end(args);
   }
}

void mpi_log_info(const char *format, ...) {
   char log_message[LOG_MAX_SIZE];

   if (is_info_log_enabled()) {
      va_list args;
      va_start(args, format);
      vsprintf(log_message, format, args);
      MPI_Send(log_message, strlen(log_message) + 1, MPI_CHAR, 0, LOG_MSG_INFO, mpi_log_comm);
      va_end(args);
   }
}
