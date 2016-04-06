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


#include "mpi_win_pmem.h"
#include <stdio.h>
#include <stdlib.h>
#include <libpmem.h>
#include "../common/logger.h"
#include "mpi_win_pmem_helper.h"

/**
 * Force any changes made to the window data to be stored durably in persistent memory.
 *
 * @param win Window object.
 *
 * @returns Error code as described in MPI specification.
 */
int MPI_Win_pmem_persist(MPI_Win_pmem win) {
   MPI_Win_memory_areas_list *current_item;

   if (win.is_pmem && !win.is_volatile && !win.allocate_in_ram) {
      mpi_log_debug("Persisting window.");
      for (current_item = win.modifiable_values->memory_areas; current_item != NULL; current_item = current_item->next) {
         if (current_item->is_pmem) {
            mpi_log_debug("Persisting memory area with base: 0x%lx, size: %lu using pmem_persist.", (long int) current_item->base, current_item->size);
            //pmem_persist(current_item->base, current_item->size);
            pmem_msync(current_item->base, current_item->size);
            pmem_drain();
         } else {
            mpi_log_debug("Persisting memory area with base: 0x%lx, size: %lu using pmem_msync.", (long int) current_item->base, current_item->size);
            if (pmem_msync(current_item->base, current_item->size) != 0) {
               mpi_log_error("Unable to msync memory area base: 0x%lx, size: %lu.", (long int) current_item->base, current_item->size);
               MPI_Win_call_errhandler(win.win, MPI_ERR_PMEM);
               return MPI_ERR_PMEM;
            }
         }
      }
      mpi_log_debug("Window persisted.");
   }

   return MPI_SUCCESS;
}

int MPI_Win_fence_pmem(int assert, MPI_Win_pmem win) {
   int result;

   mpi_log_debug("Starting MPI_Win_fence.");

   result = MPI_Win_fence(assert, win.win);
   CHECK_ERROR_CODE(result);

   mpi_log_debug("MPI_Win_fence completed.");

   return MPI_SUCCESS;
}

int MPI_Win_fence_pmem_persist(int assert, MPI_Win_pmem win) {
   int result;

   mpi_log_debug("Starting MPI_Win_fence persist.");

   result = MPI_Win_fence(assert, win.win);
   CHECK_ERROR_CODE(result);
   result = MPI_Win_pmem_persist(win);
   CHECK_ERROR_CODE(result);
   result = create_checkpoint(win, true);
   CHECK_ERROR_CODE(result);

   mpi_log_debug("MPI_Win_fence persist completed.");

   return MPI_SUCCESS;
}

int MPI_Win_start_pmem(MPI_Group group, int assert, MPI_Win_pmem win) {
   int result;

   mpi_log_debug("Starting MPI_Win_start.");

   result = MPI_Win_start(group, assert, win.win);
   CHECK_ERROR_CODE(result);

   mpi_log_debug("MPI_Win_start completed.");

   return MPI_SUCCESS;
}

int MPI_Win_complete_pmem(MPI_Win_pmem win) {
   int result;

   mpi_log_debug("Starting MPI_Win_complete.");

   result = MPI_Win_complete(win.win);
   CHECK_ERROR_CODE(result);

   mpi_log_debug("MPI_Win_complete completed.");

   return MPI_SUCCESS;
}

int MPI_Win_post_pmem(MPI_Group group, int assert, MPI_Win_pmem win) {
   int result;

   mpi_log_debug("Starting MPI_Win_post.");

   result = MPI_Win_post(group, assert, win.win);
   CHECK_ERROR_CODE(result);

   mpi_log_debug("MPI_Win_post completed.");

   return MPI_SUCCESS;
}

int MPI_Win_post_pmem_persist(MPI_Group group, int assert, MPI_Win_pmem win) {
   int result;

   mpi_log_debug("Starting MPI_Win_post persist.");

   result = MPI_Win_pmem_persist(win);
   CHECK_ERROR_CODE(result);
   result = create_checkpoint(win, false);
   CHECK_ERROR_CODE(result);
   result = MPI_Win_post(group, assert, win.win);
   CHECK_ERROR_CODE(result);

   mpi_log_debug("MPI_Win_post persist completed.");

   return MPI_SUCCESS;
}

int MPI_Win_wait_pmem(MPI_Win_pmem win) {
   int result;

   mpi_log_debug("Starting MPI_Win_wait.");

   result = MPI_Win_wait(win.win);
   CHECK_ERROR_CODE(result);

   mpi_log_debug("MPI_Win_wait completed.");

   return MPI_SUCCESS;
}

int MPI_Win_wait_pmem_persist(MPI_Win_pmem win) {
   int result;

   mpi_log_debug("Starting MPI_Win_wait persist.");

   result = MPI_Win_wait(win.win);
   CHECK_ERROR_CODE(result);
   result = MPI_Win_pmem_persist(win);
   CHECK_ERROR_CODE(result);
   result = create_checkpoint(win, false);
   CHECK_ERROR_CODE(result);

   mpi_log_debug("MPI_Win_wait persist completed.");

   return MPI_SUCCESS;
}

int MPI_Win_test_pmem(MPI_Win_pmem win, int *flag) {
   int result;

   mpi_log_debug("Starting MPI_Win_test.");

   result = MPI_Win_test(win.win, flag);
   CHECK_ERROR_CODE(result);

   mpi_log_debug("MPI_Win_test completed.");

   return MPI_SUCCESS;
}

int MPI_Win_test_pmem_persist(MPI_Win_pmem win, int *flag) {
   int result;

   mpi_log_debug("Starting MPI_Win_test persist.");

   result = MPI_Win_test(win.win, flag);
   CHECK_ERROR_CODE(result);
   if (*flag) {
      result = MPI_Win_pmem_persist(win);
      CHECK_ERROR_CODE(result);
      result = create_checkpoint(win, false);
      CHECK_ERROR_CODE(result);
   }

   mpi_log_debug("MPI_Win_test persist completed.");

   return MPI_SUCCESS;
}

int MPI_Win_lock_pmem(int lock_type, int rank, int assert, MPI_Win_pmem win) {
   return MPI_Win_lock(lock_type, rank, assert, win.win);
}

int MPI_Win_lock_all_pmem(int assert, MPI_Win_pmem win) {
   return MPI_Win_lock_all(assert, win.win);
}

int MPI_Win_unlock_pmem(int rank, MPI_Win_pmem win) {
   return MPI_Win_unlock(rank, win.win);
}

int MPI_Win_unlock_all_pmem(MPI_Win_pmem win) {
   return MPI_Win_unlock_all(win.win);
}

int MPI_Win_flush_pmem(int rank, MPI_Win_pmem win) {
   return MPI_Win_flush(rank, win.win);
}

int MPI_Win_flush_all_pmem(MPI_Win_pmem win) {
   return MPI_Win_flush_all(win.win);
}

int MPI_Win_flush_local_pmem(int rank, MPI_Win_pmem win) {
   return MPI_Win_flush_local(rank, win.win);
}

int MPI_Win_flush_local_all_pmem(MPI_Win_pmem win) {
   return MPI_Win_flush_local_all(win.win);
}

int MPI_Win_sync_pmem(MPI_Win_pmem win) {
   return MPI_Win_sync(win.win);
}
