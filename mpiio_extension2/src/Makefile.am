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

lib_LTLIBRARIES = libmpi-pmem-io.la

AM_CFLAGS +=
AM_CPPFLAGS +=
LDADD +=
AM_LDFLAGS +=

pmemioincludedir = $(includedir)/
pmemioinclude_HEADERS = file_io_pmem.h file_io_pmem_wrappers.h

libmpi_pmem_io_la_SOURCES = cache_manager.h file_io_distributed_cache.h file_io_pmem_aware.h file_io_pmem.h logger.h messages.h mpi_node_rank.h pmem_datatypes.h util.h failure_recovery.h\
	cache_manager.c file_io_distributed_cache.c file_io_pmem_aware.c file_io_pmem.c logger.c mpi_node_rank.c pmem_datatypes.c util.c failure_recovery.c

