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


#ifndef __MPI_WIN_PMEM_HELPER_H__
#define __MPI_WIN_PMEM_HELPER_H__

#include <stdbool.h>
#include <sys/stat.h>
#include <mpi.h>
#include "mpi_win_pmem.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Open file in pmem and map it into memory. Save address of mapped memory into address. Use unmap_pmem_file to free memory region mapped by this function.
 *
 * @param comm       Communicator used for error handling.
 * @param file_name  File name to open.
 * @param size       Size of the file to open.
 * @param address    Output variable for memory address of mapped file.
 *
 * @returns Error code as described in MPI specification.
 */
int open_pmem_file(MPI_Comm comm, const char *file_name, MPI_Aint size, void **address);

/**
 * Force any changes to be stored durably in persistent memory (call either pmem_persist or pmem_msync depending whether specified memory area consists of persistent memory.
 *
 * @param comm    Communicator used for error handling.
 * @param address Starting address of memory area to persist.
 * @param size    Size of memory area to persist.
 *
 * @returns Error code as described in MPI specification.
 */
int persist_pmem_file(MPI_Comm comm, void *address, MPI_Aint size);

/**
 * Unmap memory mapped file.
 *
 * @param comm    Communicator used for error handling.
 * @param address Starting address of memory area belonging to memory mapped file.
 * @param size    Size of memory mapped area.
 *
 * @returns Error code as described in MPI specification.
 */
int unmap_pmem_file(MPI_Comm comm, void *address, MPI_Aint size);

/**
 * Check if file with specified file name exists.
 *
 * @param file_name Name of the file to check.
 *
 * @returns True if file exists and is readable, false otherwise.
 */
bool check_if_file_exist(const char *file_name);

/**
 * Get size of the file with specified file name.
 *
 * @param comm       Communicator used for error handling.
 * @param file_name  Name of the file.
 * @param size       Output variable for file size in bytes.
 *
 * @returns Error code as described in MPI specification.
 */
int get_file_size(MPI_Comm comm, const char *file_name, off_t *size);

/**
 * Open global windows metadata file and return its size.
 *
 * @param comm    Communicator used for error handling.
 * @param windows Output variable for address of memory mapped area of global windows metadata file.
 * @param size    Output variable for file size in bytes.
 *
 * @returns Error code as described in MPI specification.
 */
int open_windows_metadata_file(MPI_Comm comm, MPI_Win_pmem_metadata **windows, off_t *size);

/**
 * Open specified window's versions metadata file and return its size.
 *
 * @param comm       Communicator used for error handling.
 * @param versions   Output variable for address of memory mapped area of window's versions metadata file.
 * @param size       Output variable for file size in bytes.
 *
 * @returns Error code as described in MPI specification.
 */
int open_versions_metadata_file(MPI_Comm comm, const char *window_name, MPI_Win_pmem_version **versions, off_t *size);

/**
 * Delete all checkpoints (set flag in metadata file and remove data file) created previously for window with specified name and set first (at index 0) record in metadata file to terminating record
 * indicating end of versions metadata file.
 *
 * @param comm       Communicator used for error handling.
 * @param name       Name of window.
 * @param versions   Address to memory area corresponding to memory mapped window's versions metadata file.
 *
 * @returns Error code as described in MPI specification.
 */
int delete_old_checkpoints(MPI_Comm comm, const char *name, MPI_Win_pmem_version *versions);

/**
 * Set default values for metadata stored in MPI_Win_pmem structure.
 *
 * @param win  Window object to modify.
 * @param comm Communicator to be added to win object.
 *
 * @returns Error code as described in MPI specification.
 */
int set_default_window_metadata(MPI_Win_pmem *win, MPI_Comm comm);

/**
 * Parse MPI_Info parameter of type bool with specified key.
 *
 * @param info    MPI_Info object to parse.
 * @param key     Key of MPI_Info parameter.
 * @param result  Output variable for parsed value.
 *
 * @returns Error code as described in MPI specification.
 */
int parse_mpi_info_bool(MPI_Info info, const char *key, bool *result);

/**
 * Parse pmem_name parameter in specified MPI_Info object.
 *
 * @param comm    Communicator used for error handling.
 * @param info    MPI_Info object to parse.
 * @param result  Output variable for parsed value.
 *
 * @returns Error code as described in MPI specification.
 */
int parse_mpi_info_name(MPI_Comm comm, MPI_Info info, char *result);

/**
 * Parse pmem_mode parameter in specified MPI_Info object.
 *
 * @param comm    Communicator used for error handling.
 * @param info    MPI_Info object to parse.
 * @param result  Output variable for parsed value.
 *
 * @returns Error code as described in MPI specification.
 */
int parse_mpi_info_mode(MPI_Comm comm, MPI_Info info, int *result);

/**
 * Parse pmem_checkpoint_version parameter in specified MPI_Info object.
 *
 * @param comm    Communicator used for error handling.
 * @param info    MPI_Info object to parse.
 * @param result  Output variable for parsed value.
 *
 * @returns Error code as described in MPI specification.
 */
int parse_mpi_info_checkpoint_version(MPI_Comm comm, MPI_Info info, int *result);

/**
 * Check if specified window was created previously. If window mode is set to checkpoint also check it's size.
 *
 * @param win     Window to check.
 * @param size    Requested size of window.
 * @param exists  Output variable for information whether window exists or not.
 *
 * @returns Error code as described in MPI specification.
 */
int check_if_window_exists_and_its_size(MPI_Win_pmem *win, MPI_Aint size, bool *exists);

/**
 * Create window's versions metadata file and update global metadata file.
 *
 * @param comm          Communicator used for error handling.
 * @param file_name     Name of window's versions metadata file.
 * @param versions      Output variable for memory address of mapped window's versions metadata file.
 * @param window_name   Window's name.
 * @param window_size   Window's size.
 *
 * @returns Error code as described in MPI specification.
 */
int create_window_metadata_file(MPI_Comm comm, const char *file_name, MPI_Win_pmem_version **versions, const char *window_name, MPI_Aint window_size);

/**
 * Update window's size in global metadata file.
 *
 * @param win  Window to be updated in global metadata file.
 * @param size New size of the window.
 *
 * @returns Error code as described in MPI specification.
 */
int update_window_size_in_metadata_file(MPI_Win_pmem *win, MPI_Aint size);

/**
 * Set checkpoint versions (last, next, highest) in MPI_Win_pmem object depending on window parameters and information in window's versions metadata file.
 *
 * @param win        MPI_Win_pmem object to modify.
 * @param versions   Memory address of mapped window's versions metadata file.
 *
 * @returns Error code as described in MPI specification.
 */
int set_checkpoint_versions(MPI_Win_pmem *win, MPI_Win_pmem_version *versions);

/**
 * Copy data from previously created checkpoint (specified by last_checkpoint_version) into destination area.
 *
 * @param win           Window object containing metadata about checkpoint to use.
 * @param size          Size of checkpoint in bytes.
 * @param destination   Destination memory area to copy data into.
 */
int copy_data_from_checkpoint(MPI_Win_pmem win, MPI_Aint size, void *destination);

/**
 * Create new checkpoint version of provided window.
 *
 * @param win     Window object.
 * @param fence   Flag specifying whether function is called from MPI_Win_fence.
 *
 * @returns Error code as described in MPI specification.
 */
int create_checkpoint(MPI_Win_pmem win, bool fence);

#ifdef __cplusplus
}
#endif

#endif
