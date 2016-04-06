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


#ifndef __COMMON_TEST_CASES_H__
#define __COMMON_TEST_CASES_H__

#include "test_util.h"

// single process
void file_read_at_reads_correct_bytes_from_beginning_of_file();
void file_read_at_reads_correct_bytes_from_middle_of_file();
void file_read_at_reads_correct_bytes_from_end_of_file();
void file_write_at_writes_correct_bytes_at_beginning_of_file();
void file_write_at_writes_correct_bytes_at_middle_of_file();
void file_write_at_writes_correct_bytes_at_end_of_file();

// multiple processes
void read_at_processes_read_correct_bytes_from_same_file_part();
void read_at_processes_read_correct_bytes_from_overlapping_parts();
void read_at_processes_read_correct_bytes_from_non_overlapping_parts();
void write_at_processes_wrote_correct_bytes_into_same_file_part();
void write_at_processes_wrote_correct_bytes_into_overlapping_parts();
void write_at_processes_wrote_correct_bytes_into_non_overlapping_parts();

// long sequence of operations
void long_sequence_of_read_at_operations();
void long_sequence_of_write_at_operations();
void long_sequence_of_read_at_write_at_operations();

// other
void file_sync_call();

#endif
