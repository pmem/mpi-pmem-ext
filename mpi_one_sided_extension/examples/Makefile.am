#
# Copyright 2014-2016, Gdansk University of Technology
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
# 
#     * Redistributions of source code must retain the above copyright notice,
#       this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in the
#       documentation and/or other materials provided with the distribution.
#     * Neither the name of the copyright holder nor the names of its contributors
#       may be used to endorse or promote products derived from this software
#       without specific prior written permission.
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#


AUTOMAKE_OPTIONS = subdir-objects

bin_PROGRAMS = mpi_one_sided \
               HPCCG/HPCCG HPCCG/HPCCG_checkpoint_disk HPCCG/HPCCG_checkpoint_pmem HPCCG/HPCCG_checkpoint_double_buffered

AM_CFLAGS += -fopenmp
AM_CXXFLAGS += -DUSING_MPI -fopenmp
AM_CPPFLAGS += -I$(srcdir)/../src -DUSING_OMP
LDADD += ../src/mpi_one_sided_extension/libmpi-pmem-one-sided.la
AM_LDFLAGS +=

mpi_one_sided_SOURCES = main.c

HPCCG_HPCCG_SOURCES =   HPCCG/compute_residual.cpp HPCCG/compute_residual.hpp HPCCG/ddot.cpp HPCCG/ddot.hpp HPCCG/dump_matlab_matrix.cpp HPCCG/dump_matlab_matrix.hpp \
                  HPCCG/exchange_externals.cpp HPCCG/exchange_externals.hpp HPCCG/generate_matrix.cpp HPCCG/generate_matrix.hpp HPCCG/HPC_Sparse_Matrix.cpp HPCCG/HPC_Sparse_Matrix.hpp \
                  HPCCG/HPC_sparsemv.cpp HPCCG/HPC_sparsemv.hpp HPCCG/HPCCG.cpp HPCCG/HPCCG.hpp HPCCG/make_local_matrix.cpp HPCCG/make_local_matrix.hpp HPCCG/mytimer.cpp HPCCG/mytimer.hpp \
                  HPCCG/read_HPC_row.cpp HPCCG/read_HPC_row.hpp HPCCG/waxpby.cpp HPCCG/waxpby.hpp HPCCG/YAML_Doc.cpp HPCCG/YAML_Doc.hpp HPCCG/YAML_Element.cpp HPCCG/YAML_Element.hpp \
                  HPCCG/HPCCG_main.cpp

HPCCG_HPCCG_checkpoint_disk_SOURCES = HPCCG/compute_residual.cpp HPCCG/compute_residual.hpp HPCCG/ddot.cpp HPCCG/ddot.hpp HPCCG/dump_matlab_matrix.cpp HPCCG/dump_matlab_matrix.hpp \
                                HPCCG/exchange_externals.cpp HPCCG/exchange_externals.hpp HPCCG/generate_matrix.cpp HPCCG/generate_matrix.hpp HPCCG/HPC_Sparse_Matrix.cpp HPCCG/HPC_Sparse_Matrix.hpp \
                                HPCCG/HPC_sparsemv.cpp HPCCG/HPC_sparsemv.hpp HPCCG/HPCCG_checkpoint_disk.cpp HPCCG/HPCCG.hpp HPCCG/make_local_matrix.cpp HPCCG/make_local_matrix.hpp \
                                HPCCG/mytimer.cpp HPCCG/mytimer.hpp HPCCG/read_HPC_row.cpp HPCCG/read_HPC_row.hpp HPCCG/waxpby.cpp HPCCG/waxpby.hpp HPCCG/YAML_Doc.cpp HPCCG/YAML_Doc.hpp \
                                HPCCG/YAML_Element.cpp HPCCG/YAML_Element.hpp HPCCG/HPCCG_main.cpp
                                
HPCCG_HPCCG_checkpoint_pmem_SOURCES = HPCCG/compute_residual.cpp HPCCG/compute_residual.hpp HPCCG/ddot.cpp HPCCG/ddot.hpp HPCCG/dump_matlab_matrix.cpp HPCCG/dump_matlab_matrix.hpp \
                                HPCCG/exchange_externals.cpp HPCCG/exchange_externals.hpp HPCCG/generate_matrix.cpp HPCCG/generate_matrix.hpp HPCCG/HPC_Sparse_Matrix.cpp HPCCG/HPC_Sparse_Matrix.hpp \
                                HPCCG/HPC_sparsemv.cpp HPCCG/HPC_sparsemv.hpp HPCCG/HPCCG_checkpoint_pmem.cpp HPCCG/HPCCG.hpp HPCCG/make_local_matrix.cpp HPCCG/make_local_matrix.hpp \
                                HPCCG/mytimer.cpp HPCCG/mytimer.hpp HPCCG/read_HPC_row.cpp HPCCG/read_HPC_row.hpp HPCCG/waxpby.cpp HPCCG/waxpby.hpp HPCCG/YAML_Doc.cpp HPCCG/YAML_Doc.hpp \
                                HPCCG/YAML_Element.cpp HPCCG/YAML_Element.hpp HPCCG/HPCCG_main.cpp

HPCCG_HPCCG_checkpoint_double_buffered_SOURCES =  HPCCG/compute_residual.cpp HPCCG/compute_residual.hpp HPCCG/ddot.cpp HPCCG/ddot.hpp HPCCG/dump_matlab_matrix.cpp HPCCG/dump_matlab_matrix.hpp \
                                            HPCCG/exchange_externals.cpp HPCCG/exchange_externals.hpp HPCCG/generate_matrix.cpp HPCCG/generate_matrix.hpp \
                                            HPCCG/HPC_Sparse_Matrix.cpp HPCCG/HPC_Sparse_Matrix.hpp HPCCG/HPC_sparsemv.cpp HPCCG/HPC_sparsemv.hpp HPCCG/HPCCG_checkpoint_double_buffered.cpp \
                                            HPCCG/HPCCG.hpp HPCCG/make_local_matrix.cpp HPCCG/make_local_matrix.hpp HPCCG/mytimer.cpp HPCCG/mytimer.hpp HPCCG/read_HPC_row.cpp HPCCG/read_HPC_row.hpp \
                                            HPCCG/waxpby.cpp HPCCG/waxpby.hpp HPCCG/YAML_Doc.cpp HPCCG/YAML_Doc.hpp HPCCG/YAML_Element.cpp HPCCG/YAML_Element.hpp HPCCG/HPCCG_main.cpp
