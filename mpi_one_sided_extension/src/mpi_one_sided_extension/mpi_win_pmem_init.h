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


#ifndef __MPI_WIN_PMEM_INIT_H__
#define __MPI_WIN_PMEM_INIT_H__

#ifdef __cplusplus
extern "C" {
#endif

int MPI_Win_create_pmem(void *base, MPI_Aint size, int disp_unit, MPI_Info info, MPI_Comm comm, MPI_Win_pmem *win);
int MPI_Win_allocate_pmem(MPI_Aint size, int disp_unit, MPI_Info info, MPI_Comm comm, void *baseptr, MPI_Win_pmem *win);
int MPI_Win_allocate_shared_pmem(MPI_Aint size, int disp_unit, MPI_Info info, MPI_Comm comm, void *baseptr, MPI_Win_pmem *win);
int MPI_Win_shared_query_pmem(MPI_Win_pmem win, int rank, MPI_Aint *size, int *disp_unit, void *baseptr);
int MPI_Win_create_dynamic_pmem(MPI_Info info, MPI_Comm comm, MPI_Win_pmem *win);
int MPI_Win_attach_pmem(MPI_Win_pmem win, void *base, MPI_Aint size);
int MPI_Win_detach_pmem(MPI_Win_pmem win, const void *base);
int MPI_Win_free_pmem(MPI_Win_pmem *win);
int MPI_Win_get_attr_pmem(MPI_Win_pmem win, int win_keyval, void *attribute_val, int *flag);
int MPI_Win_set_attr_pmem(MPI_Win_pmem win, int win_keyval, void *attribute_val);
int MPI_Win_get_group_pmem(MPI_Win_pmem win, MPI_Group *group);
int MPI_Win_set_info_pmem(MPI_Win_pmem win, MPI_Info info);
int MPI_Win_get_info_pmem(MPI_Win_pmem win, MPI_Info *info_used);

#ifdef __cplusplus
}
#endif

#endif
