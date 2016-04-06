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


#ifndef __HELPER_H__
#define __HELPER_H__

#include <mpi.h>
#include <mpi_one_sided_extension/mpi_win_pmem_datatypes.h>

/**
 * Create window metadata and data file (dummy empty file). Update metadata in global metadata file.
 *
 * @param window_name   Name of the window to create.
 * @param size          Size of the window to create.
 *
 * @returns 0 on success or non zero value on failure.
 */
int create_window(const char *window_name, MPI_Aint size);

/**
 * Check if versions metadata file exists for a given window and compare its contents with expected_versions. Provided versions_length specify the number of records in versions array.
 * Function also checks whether last record in a file (at index equal to versions_length) is a terminating record.
 *
 * @param window_name         Name of the window.
 * @param exists              Flag specifying whether file should exist.
 * @param expected_versions   Expected contents of versions metadata file.
 * @param versions_length     Expected number of records in versions metadata file. Record at index equal to versions_length is expected to be a terminating record.
 *
 * @returns 0 on success or non zero value on failure.
 */
int check_versions_metadata_file(const char *window_name, bool exists, const MPI_Win_pmem_version *expected_versions, int versions_length);

/**
 * Check if global metadata is the same as expected_windows. Provided windows_length specify the number of records in windows array.
 * Function also checks whether last record in a file (at index equal to windows_length) is a terminating record.
 *
 * @param expected_windows Expected contents of global metadata file.
 * @param windows_length   Expected number of records in global metadata file. Record at index equal to windows_length is expected to be a terminating record.
 *
 * @returns 0 on success or non zero value on failure.
 */
int check_global_metadata_file(const MPI_Win_pmem_metadata *expected_windows, int windows_length);

/**
 * Checks checkpoint versions in specified window object.
 *
 * @param win                          Window object to check.
 * @param next_checkpoint_version      Expected value of next_checkpoint_version.
 * @param last_checkpoint_version      Expected value of last_checkpoint_version.
 * @param highest_checkpoint_version   Expected value of highest_checkpoint_version.
 *
 * @returns 0 on success or non zero value on failure.
 */
int check_checkpoint_versions(const MPI_Win_pmem win, int next_checkpoint_version, int last_checkpoint_version, int highest_checkpoint_version);

/**
 * Allocate window in expand mode, transactional and with keeping all checkpoints in order to be able to synchronize to create checkpoints.
 *
 * @param win           Window object to allocate.
 * @param window_data   Destination where allocated memory address is saved.
 * @param window_name   Name of the window to allocate.
 * @param size          Size of the window to allocate.
 */
void allocate_window(MPI_Win_pmem *win, void **window_data, const char *window_name, MPI_Aint size);

/**
 * Checks whether all bytes in data array equals value.
 *
 * @param data    Memory area to check.
 * @param size    Size of data array.
 * @param value   Expected value to be repeated on each byte of data array.
 *
 * @returns 0 on success or non zero value on failure.
 */
int check_data(const char *data, MPI_Aint size, char value);

/**
 * Checks whether file with specified name exists, its size and its contents.
 *
 * @param file_name        Name of the file to check.
 * @param exists           Flag specifying whether file should exist.
 * @param size             Expected file size.
 * @param check_contents   Flag specifying whether file contents should be checked.
 * @param value            Expected value to be repeated on each byte in file.
 *
 * @returns 0 on success or non zero value on failure.
 */
int check_if_file_exists_size_and_contents(const char *file_name, bool exists, MPI_Aint size, bool check_contents, char value);

/**
 * Checks whether data file of specified window exists, its size and contents.
 *
 * @param window_name   Name of the window.
 * @param exists        Flag specifying whether file should exist.
 * @param size          Expected size of a data file.
 * @param value         Expected value to be repeated on each byte in data file.
 *
 * @returns 0 on success or non zero value on failure.
 */
int check_data_file(const char *window_name, bool exists, MPI_Aint size, bool check_contents, char value);

/**
 * Checks whether specified checkpoint version exists and compares it with expectations. If checkpoint exists and should exist, function checks if all bytes in checkpoint equals value.
 *
 * @param window_name         Name of the window.
 * @param checkpoint_version  Checkpoint version to check.
 * @param exists              Flag specifying whether checkpoint should exist.
 * @param size                Expected size of a checkpoint.
 * @param value               Expected value to be repeated on each byte of checkpoint.
 *
 * @returns 0 on success or non zero value on failure.
 */
int check_checkpoint_data(const char *window_name, int checkpoint_version, bool exists, MPI_Aint size, char value);

/**
 * Checks metadata associated with window object.
 *
 * @param win           Window object to check.
 * @param expected      Window object with expected metadata.
 * @param name          Flag specifying whether window name should be checked.
 * @param memory_areas  Flag specifying whether memory_areas list should be non empty (i.e. first pointer should be not NULL).
 *
 * @returns 0 on success or non zero value on failure.
 */
int check_window_object(const MPI_Win_pmem win, const MPI_Win_pmem expected, bool name, bool memory_areas);

/**
 * Checks single element of memory areas list in window object.
 *
 * @param element       List element to check.
 * @param base          Expected base address.
 * @param size          Expected size
 * @param win_is_pmem   Value of is_pmem flag of window object containing checked memory areas list.
 * @param next          Flag specifying whether next element in a list should exist.
 *
 * @returns 0 on success or non zero value on failure.
 */
int check_memory_areas_list_element(const MPI_Win_memory_areas_list *element, void *base, MPI_Aint size, bool win_is_pmem, bool next);

#endif
