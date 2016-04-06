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


#ifndef __MPI_WIN_PMEM_MANAGE_H__
#define __MPI_WIN_PMEM_MANAGE_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Set root path containing application's memory windows.
 *
 * @param path Filesystem path to directory containing application's memory windows.
 *
 * @returns Error code as described in MPI specification.
 */
int MPI_Win_pmem_set_root_path(const char *path);

/**
 * Creates MPI_Win_pmem_windows opaque object containing list of available windows. Created windows object should be freed using MPI_Win_pmem_free_windows_list.
 *
 * @param windows Output variable for opaque object containing list of available windows.
 *
 * @returns Error code as described in MPI specification.
 */
int MPI_Win_pmem_list(MPI_Win_pmem_windows *windows);

/**
 * Frees MPI_Win_pmem_windows opaque object.
 *
 * @param windows MPI_Win_pmem_windows object to free.
 *
 * @returns Error code as described in MPI specification.
 */
int MPI_Win_pmem_free_windows_list(MPI_Win_pmem_windows *windows);

/**
 * Gets number of available windows from windows opaque object.
 *
 * @param windows    MPI_Win_pmem_windows opaque object containing metadata about windows.
 * @param nwindows   Output variable for number of available windows.
 *
 * @returns Error code as described in MPI specification.
 */
int MPI_Win_pmem_get_nwindows(MPI_Win_pmem_windows windows, int *nwindows);

/**
 * Gets name of nth window in windows opaque object.
 *
 * @param windows MPI_Win_pmem_windows opaque object containing metadata about windows.
 * @param n       Window number.
 * @param name    Output variable for name of nth window. Enough memory for name should be allocated before calling this function.
 *
 * @returns Error code as described in MPI specification.
 */
int MPI_Win_pmem_get_name(MPI_Win_pmem_windows windows, int n, char *name);

/**
 * Gets size of nth window in windows opaque object.
 *
 * @param windows MPI_Win_pmem_windows opaque object containing metadata about windows.
 * @param n       Window number.
 * @param name    Output variable for size of nth window.
 *
 * @returns Error code as described in MPI specification.
 */
int MPI_Win_pmem_get_size(MPI_Win_pmem_windows windows, int n, MPI_Aint *size);

/**
 * Gets data about available window's versions of nth window in windows opaque object. Created versions object should be freed using MPI_Win_pmem_free_versions_list.
 *
 * @param windows    MPI_Win_pmem_windows opaque object containing metadata about windows.
 * @param n          Window number.
 * @param versions   Output variable for versions opaque object of nth window.
 *
 * @returns Error code as described in MPI specification.
 */
int MPI_Win_pmem_get_versions(MPI_Win_pmem_windows windows, int n, MPI_Win_pmem_versions *versions);

/**
 * Frees MPI_Win_pmem_versions opaque object.
 *
 * @param versions MPI_Win_pmem_versions object to free.
 *
 * @returns Error code as described in MPI specification.
 */
int MPI_Win_pmem_free_versions_list(MPI_Win_pmem_versions *versions);

/**
 * Gets number of available versions from versions opaque object.
 *
 * @param versions   MPI_Win_pmem_versions opaque object containing metadata about window's versions.
 * @param nversions  Output variable for number of available versions.
 *
 * @returns Error code as described in MPI specification.
 */
int MPI_Win_pmem_get_nversions(MPI_Win_pmem_versions versions, int *nversions);

/**
 * Gets version number of nth window's version in window's versions opaque object.
 *
 * @param versions   MPI_Win_pmem_versions opaque object containing metadata about window's versions.
 * @param n          Index in versions opaque object.
 * @param version    Output variable for version number of nth window's version.
 *
 * @returns Error code as described in MPI specification.
 */
int MPI_Win_pmem_get_version(MPI_Win_pmem_versions versions, int n, int *version);

/**
 * Gets timestamp in miliseconds (standard time_t from time.h) of nth window's version in window's versions opaque object.
 *
 * @param versions   MPI_Win_pmem_versions opaque object containing metadata about window's versions.
 * @param n          Index in versions opaque object.
 * @param timestamp  Output variable for timestamp of nth window's version.
 *
 * @returns Error code as described in MPI specification.
 */
int MPI_Win_pmem_get_version_timestamp(MPI_Win_pmem_versions versions, int n, time_t *timestamp);

/**
 * Deletes window (metadata, data and old checkpoints) with specified name.
 *
 * @param name Name of the window to delete.
 *
 * @returns Error code as described in MPI specification.
 */
int MPI_Win_pmem_delete(const char *name);

/**
 * Deletes specified version of window with specified name.
 *
 * @param name    Name of the window.
 * @param version Version of window to delete.
 *
 * @returns Error code as described in MPI specification.
 */
int MPI_Win_pmem_delete_version(const char *name, int version);

#ifdef __cplusplus
}
#endif

#endif
