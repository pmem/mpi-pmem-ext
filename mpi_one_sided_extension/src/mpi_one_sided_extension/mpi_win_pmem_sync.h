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


#ifndef __MPI_WIN_PMEM_SYNC_H__
#define __MPI_WIN_PMEM_SYNC_H__

#ifdef __cplusplus
extern "C" {
#endif

int MPI_Win_fence_pmem(int assert, MPI_Win_pmem win);
int MPI_Win_fence_pmem_persist(int assert, MPI_Win_pmem win);
int MPI_Win_start_pmem(MPI_Group group, int assert, MPI_Win_pmem win);
int MPI_Win_complete_pmem(MPI_Win_pmem win);
int MPI_Win_post_pmem(MPI_Group group, int assert, MPI_Win_pmem win);
int MPI_Win_post_pmem_persist(MPI_Group group, int assert, MPI_Win_pmem win);
int MPI_Win_wait_pmem(MPI_Win_pmem win);
int MPI_Win_wait_pmem_persist(MPI_Win_pmem win);
int MPI_Win_test_pmem(MPI_Win_pmem win, int *flag);
int MPI_Win_test_pmem_persist(MPI_Win_pmem win, int *flag);
int MPI_Win_lock_pmem(int lock_type, int rank, int assert, MPI_Win_pmem win);
int MPI_Win_lock_all_pmem(int assert, MPI_Win_pmem win);
int MPI_Win_unlock_pmem(int rank, MPI_Win_pmem win);
int MPI_Win_unlock_all_pmem(MPI_Win_pmem win);
int MPI_Win_flush_pmem(int rank, MPI_Win_pmem win);
int MPI_Win_flush_all_pmem(MPI_Win_pmem win);
int MPI_Win_flush_local_pmem(int rank, MPI_Win_pmem win);
int MPI_Win_flush_local_all_pmem(MPI_Win_pmem win);
int MPI_Win_sync_pmem(MPI_Win_pmem win);

#ifdef __cplusplus
}
#endif

#endif
