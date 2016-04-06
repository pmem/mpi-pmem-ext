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


#ifndef __DEFINES_H__
#define __DEFINES_H__

#define MPI_Init MPI_Init_pmem
#define MPI_Init_thread MPI_Init_thread_pmem
#define MPI_Finalize MPI_Finalize_pmem

#define MPI_Win MPI_Win_pmem
#define MPI_Win_create MPI_Win_create_pmem
#define MPI_Win_allocate MPI_Win_allocate_pmem
#define MPI_Win_allocate_shared MPI_Win_allocate_shared_pmem
#define MPI_Win_create_dynamic MPI_Win_create_dynamic_pmem
#define MPI_Win_attach MPI_Win_attach_pmem
#define MPI_Win_detach MPI_Win_detach_pmem
#define MPI_Win_free MPI_Win_free_pmem
#define MPI_Win_get_attr MPI_Win_get_attr_pmem
#define MPI_Win_set_attr MPI_Win_set_attr_pmem
#define MPI_Win_get_group MPI_Win_get_group_pmem
#define MPI_Win_set_info MPI_Win_set_info_pmem
#define MPI_Win_get_info MPI_Win_get_info_pmem

#define MPI_Put MPI_Put_pmem
#define MPI_Get MPI_Get_pmem
#define MPI_Accumulate MPI_Accumulate_pmem
#define MPI_Get_accumulate MPI_Get_accumulate_pmem
#define MPI_Fetch_and_op MPI_Fetch_and_op_pmem
#define MPI_Compare_and_swap MPI_Compare_and_swap_pmem
#define MPI_Rput MPI_Rput_pmem
#define MPI_Rget MPI_Rget_pmem
#define MPI_Raccumulate MPI_Raccumulate_pmem
#define MPI_Rget_accumulate MPI_Rget_accumulate_pmem

#define MPI_Win_fence MPI_Win_fence_pmem
#define MPI_Win_start MPI_Win_start_pmem
#define MPI_Win_complete MPI_Win_complete_pmem
#define MPI_Win_post MPI_Win_post_pmem
#define MPI_Win_wait MPI_Win_wait_pmem
#define MPI_Win_test MPI_Win_test_pmem
#define MPI_Win_lock MPI_Win_lock_pmem
#define MPI_Win_lock_all MPI_Win_lock_all_pmem
#define MPI_Win_unlock MPI_Win_unlock_pmem
#define MPI_Win_unlock_all MPI_Win_unlock_all_pmem
#define MPI_Win_flush MPI_Win_flush_pmem
#define MPI_Win_flush_all MPI_Win_flush_all_pmem
#define MPI_Win_flush_local MPI_Win_flush_local_pmem
#define MPI_Win_flush_local_all MPI_Win_flush_local_all_pmem
#define MPI_Win_sync MPI_Win_sync_pmem

#endif
