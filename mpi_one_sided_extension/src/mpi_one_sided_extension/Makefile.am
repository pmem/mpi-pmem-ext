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

lib_LTLIBRARIES = libmpi-pmem-one-sided.la

AM_CFLAGS +=
AM_CPPFLAGS +=
LDADD +=
AM_LDFLAGS +=

commonincludedir = $(includedir)/common
commoninclude_HEADERS = ../common/error_codes.h ../common/logger.h ../common/mpi_init_pmem.h
onesidedincludedir = $(includedir)/mpi_one_sided_extension
onesidedinclude_HEADERS = defines.h mpi_win_pmem.h mpi_win_pmem_communication.h mpi_win_pmem_datatypes.h mpi_win_pmem_init.h mpi_win_pmem_manage.h mpi_win_pmem_sync.h 

libmpi_pmem_one_sided_la_SOURCES =	defines.h mpi_win_pmem.h mpi_win_pmem_communication.c mpi_win_pmem_communication.h mpi_win_pmem_datatypes.h mpi_win_pmem_init.c mpi_win_pmem_init.h\
					mpi_win_pmem_manage.c mpi_win_pmem_manage.h mpi_win_pmem_sync.c mpi_win_pmem_sync.h mpi_win_pmem_helper.c mpi_win_pmem_helper.h\
					../common/error_codes.h ../common/logger.c ../common/logger.h ../common/util.c ../common/util.h ../common/mpi_init_pmem.c ../common/mpi_init_pmem.h
