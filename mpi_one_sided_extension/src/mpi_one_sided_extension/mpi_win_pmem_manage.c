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


#include "mpi_win_pmem.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "../common/logger.h"
#include "mpi_win_pmem_helper.h"

char mpi_pmem_root_path[MPI_PMEM_MAX_ROOT_PATH];

/**
 * Check if MPI_Win_pmem_windows structure contains windows array.
 *
 * @param windows MPI_Win_pmem_windows object to check.
 *
 * @returns Error code as described in MPI specification.
 */
int check_windows_structure(MPI_Win_pmem_windows windows) {
   if (windows.windows == NULL) {
      mpi_log_error("No windows data in windows structure.");
      MPI_Comm_call_errhandler(MPI_COMM_WORLD, MPI_ERR_PMEM_WINDOWS);
      return MPI_ERR_PMEM_WINDOWS;
   }
   return MPI_SUCCESS;
}

/**
 * Check specified index n belongs to array windows in MPI_Win_pmem_windows.
 *
 * @param windows MPI_Win_pmem_windows object to check.
 * @param n       Index in windows array to check.
 *
 * @returns Error code as described in MPI specification.
 */
int check_index_in_windows_structure(MPI_Win_pmem_windows windows, int n) {
   if (n < 0 || n >= windows.size) {
      mpi_log_error("Invalid index %d in windows. Number of windows is %d.", n, windows.size);
      MPI_Comm_call_errhandler(MPI_COMM_WORLD, MPI_ERR_PMEM_ARG);
      return MPI_ERR_PMEM_ARG;
   }
   return MPI_SUCCESS;
}

/**
 * Check if MPI_Win_pmem_versions structure contains versions array.
 *
 * @param versions MPI_Win_pmem_versions object to check.
 *
 * @returns Error code as described in MPI specification.
 */
int check_versions_structure(MPI_Win_pmem_versions versions) {
   if (versions.versions == NULL) {
      mpi_log_error("No versions data in versions structure.");
      MPI_Comm_call_errhandler(MPI_COMM_WORLD, MPI_ERR_PMEM_VERSIONS);
      return MPI_ERR_PMEM_VERSIONS;
   }
   return MPI_SUCCESS;
}

/**
 * Check specified index n belongs to array versions in MPI_Win_pmem_versions.
 *
 * @param versions   MPI_Win_pmem_versions object to check.
 * @param n          Index in versions array to check.
 *
 * @returns Error code as described in MPI specification.
 */
int check_index_in_versions_structure(MPI_Win_pmem_versions versions, int n) {
   if (n < 0 || n >= versions.size) {
      mpi_log_error("Invalid index %d in versions. Number of versions is %d.", n, versions.size);
      MPI_Comm_call_errhandler(MPI_COMM_WORLD, MPI_ERR_PMEM_ARG);
      return MPI_ERR_PMEM_ARG;
   }
   return MPI_SUCCESS;
}

int MPI_Win_pmem_set_root_path(const char *path) {
   int result;
   char metadata_file_name[MPI_PMEM_MAX_ROOT_PATH + 9]; // 9 == length of "/.windows"
   MPI_Win_pmem_metadata *windows;

   mpi_log_debug("Setting root path to: %s", path);

   // Check if path is not too long.
   if (strlen(path) + 1 > MPI_PMEM_MAX_ROOT_PATH) {
      mpi_log_error("Root path too long.");
      MPI_Comm_call_errhandler(MPI_COMM_WORLD, MPI_ERR_PMEM_ROOT_PATH);
      return MPI_ERR_PMEM_ROOT_PATH;
   }

   // Check if path is correct and points to directory.
   struct stat file_status;
   if (stat(path, &file_status) != 0 || !S_ISDIR(file_status.st_mode)) {
      mpi_log_error("Root path either doesn't exist or isn't a directory.");
      MPI_Comm_call_errhandler(MPI_COMM_WORLD, MPI_ERR_PMEM_ROOT_PATH);
      return MPI_ERR_PMEM_ROOT_PATH;
   }

   // Check if global metadata file exists and create if necessary.
   sprintf(metadata_file_name, "%s/.windows", path);
   if (!check_if_file_exist(metadata_file_name)) {
      mpi_log_debug("Creating metadata file '%s'.", metadata_file_name);
      // Create file.
      result = open_pmem_file(MPI_COMM_WORLD, metadata_file_name, sizeof(MPI_Win_pmem_metadata), (void**) &windows);
      CHECK_ERROR_CODE(result);
      // Set first record in file to terminating record.
      windows[0].size = 0;
      windows[0].flags = MPI_PMEM_FLAG_NO_OBJECT;
      result = persist_pmem_file(MPI_COMM_WORLD, windows, sizeof(MPI_Win_pmem_metadata));
      CHECK_ERROR_CODE(result);
      result = unmap_pmem_file(MPI_COMM_WORLD, windows, sizeof(MPI_Win_pmem_metadata));
      CHECK_ERROR_CODE(result);
      mpi_log_debug("Metadata file '%s' created.", metadata_file_name);
   }

   strcpy(mpi_pmem_root_path, path);

   mpi_log_debug("Root path set to: %s", mpi_pmem_root_path);

   return MPI_SUCCESS;
}

int MPI_Win_pmem_list(MPI_Win_pmem_windows *windows) {
   int result, i, j, window_count;
   MPI_Win_pmem_metadata *metadata;
   off_t metadata_file_size;

   result = open_windows_metadata_file(MPI_COMM_WORLD, &metadata, &metadata_file_size);
   CHECK_ERROR_CODE(result);

   // Count all existing windows.
   window_count = 0;
   for (i = 0; metadata[i].flags != MPI_PMEM_FLAG_NO_OBJECT; i++) {
      if (metadata[i].flags == MPI_PMEM_FLAG_OBJECT_EXISTS) {
         window_count++;
      }
   }

   // Allocate windows array and fill it with windows' metadata.
   windows->size = window_count;
   windows->windows = malloc(window_count * sizeof(MPI_Win_pmem_window));
   if (windows->windows == NULL) {
      mpi_log_error("Unable to allocate memory.");
      MPI_Comm_call_errhandler(MPI_COMM_WORLD, MPI_ERR_PMEM_NO_MEM);
      return MPI_ERR_PMEM_NO_MEM;
   }
   j = 0;
   for (i = 0; metadata[i].flags != MPI_PMEM_FLAG_NO_OBJECT; i++) {
      if (metadata[i].flags == MPI_PMEM_FLAG_OBJECT_EXISTS) {
         strcpy(windows->windows[j].name, metadata[i].name);
         windows->windows[j].size = metadata[i].size;
         j++;
      }
   }
   result = unmap_pmem_file(MPI_COMM_WORLD, metadata, metadata_file_size);
   CHECK_ERROR_CODE(result);

   return MPI_SUCCESS;
}

int MPI_Win_pmem_free_windows_list(MPI_Win_pmem_windows *windows) {
   int result;

   result = check_windows_structure(*windows);
   CHECK_ERROR_CODE(result);
   free(windows->windows);
   windows->windows = NULL;
   
   return MPI_SUCCESS;
}

int MPI_Win_pmem_get_nwindows(MPI_Win_pmem_windows windows, int *nwindows) {
   int result;

   result = check_windows_structure(windows);
   CHECK_ERROR_CODE(result);
   *nwindows = windows.size;

   return MPI_SUCCESS;
}

int MPI_Win_pmem_get_name(MPI_Win_pmem_windows windows, int n, char *name) {
   int result;

   result = check_windows_structure(windows);
   CHECK_ERROR_CODE(result);
   result = check_index_in_windows_structure(windows, n);
   CHECK_ERROR_CODE(result);
   strcpy(name, windows.windows[n].name);

   return MPI_SUCCESS;
}

int MPI_Win_pmem_get_size(MPI_Win_pmem_windows windows, int n, MPI_Aint *size) {
   int result;

   result = check_windows_structure(windows);
   CHECK_ERROR_CODE(result);
   result = check_index_in_windows_structure(windows, n);
   CHECK_ERROR_CODE(result);
   *size = windows.windows[n].size;

   return MPI_SUCCESS;
}

int MPI_Win_pmem_get_versions(MPI_Win_pmem_windows windows, int n, MPI_Win_pmem_versions *versions) {
   int result, i, j, versions_count;
   MPI_Win_pmem_version *metadata;
   off_t metadata_file_size;

   result = check_windows_structure(windows);
   CHECK_ERROR_CODE(result);
   result = check_index_in_windows_structure(windows, n);
   CHECK_ERROR_CODE(result);

   result = open_versions_metadata_file(MPI_COMM_WORLD, windows.windows[n].name, &metadata, &metadata_file_size);
   CHECK_ERROR_CODE(result);

   // Count all existing checkpoint versions for specified window.
   versions_count = 0;
   for (i = 0; metadata[i].flags != MPI_PMEM_FLAG_NO_OBJECT; i++) {
      if (metadata[i].flags == MPI_PMEM_FLAG_OBJECT_EXISTS) {
         versions_count++;
      }
   }

   // Allocate versions array and fill it with checkpoint versions' metadata.
   versions->size = versions_count;
   versions->versions = malloc(versions_count * sizeof(MPI_Win_pmem_window));
   if (versions->versions == NULL) {
      mpi_log_error("Unable to allocate memory.");
      MPI_Comm_call_errhandler(MPI_COMM_WORLD, MPI_ERR_PMEM_NO_MEM);
      return MPI_ERR_PMEM_NO_MEM;
   }
   j = 0;
   for (i = 0; metadata[i].flags != MPI_PMEM_FLAG_NO_OBJECT; i++) {
      if (metadata[i].flags == MPI_PMEM_FLAG_OBJECT_EXISTS) {
         versions->versions[j] = metadata[i];
         j++;
      }
   }
   result = unmap_pmem_file(MPI_COMM_WORLD, metadata, metadata_file_size);
   CHECK_ERROR_CODE(result);

   return MPI_SUCCESS;
}

int MPI_Win_pmem_free_versions_list(MPI_Win_pmem_versions *versions) {
   int result;

   result = check_versions_structure(*versions);
   CHECK_ERROR_CODE(result);
   free(versions->versions);
   versions->versions = NULL;

   return MPI_SUCCESS;
}

int MPI_Win_pmem_get_nversions(MPI_Win_pmem_versions versions, int *nversions) {
   int result;

   result = check_versions_structure(versions);
   CHECK_ERROR_CODE(result);
   *nversions = versions.size;

   return MPI_SUCCESS;
}

int MPI_Win_pmem_get_version(MPI_Win_pmem_versions versions, int n, int *version) {
   int result;

   result = check_versions_structure(versions);
   CHECK_ERROR_CODE(result);
   result = check_index_in_versions_structure(versions, n);
   CHECK_ERROR_CODE(result);
   *version = versions.versions[n].version;

   return MPI_SUCCESS;
}

int MPI_Win_pmem_get_version_timestamp(MPI_Win_pmem_versions versions, int n, time_t *timestamp) {
   int result;

   result = check_versions_structure(versions);
   CHECK_ERROR_CODE(result);
   result = check_index_in_versions_structure(versions, n);
   CHECK_ERROR_CODE(result);
   *timestamp = versions.versions[n].timestamp;

   return MPI_SUCCESS;
}

int MPI_Win_pmem_delete(const char *name) {
   int result, i;
   char *file_name;
   MPI_Win_pmem_metadata *windows;
   MPI_Win_pmem_version *versions;
   off_t metadata_file_size, file_size;

   result = open_windows_metadata_file(MPI_COMM_WORLD, &windows, &metadata_file_size);
   CHECK_ERROR_CODE(result);
   for (i = 0; windows[i].flags != MPI_PMEM_FLAG_NO_OBJECT; i++) {
      if (strcmp(name, windows[i].name) == 0) {
         // Check if window is already deleted.
         if (windows[i].flags == MPI_PMEM_FLAG_OBJECT_DELETED) {
            mpi_log_debug("Window '%s' already deleted.", name);
            result = unmap_pmem_file(MPI_COMM_WORLD, windows, metadata_file_size);
            CHECK_ERROR_CODE(result);
            return MPI_SUCCESS;
         }
         
         // Delete old versions.
         result = open_versions_metadata_file(MPI_COMM_WORLD, name, &versions, &file_size);
         CHECK_ERROR_CODE(result);
         result = delete_old_checkpoints(MPI_COMM_WORLD, name, versions);
         CHECK_ERROR_CODE(result);
         result = unmap_pmem_file(MPI_COMM_WORLD, versions, file_size);
         CHECK_ERROR_CODE(result);

         // Set window status to deleted in global metadata file.
         windows[i].flags = MPI_PMEM_FLAG_OBJECT_DELETED;
         result = persist_pmem_file(MPI_COMM_WORLD, &windows[i].flags, sizeof(char));
         CHECK_ERROR_CODE(result);
         
         // Remove metadata file.
         file_name = malloc((strlen(mpi_pmem_root_path) + strlen(name) + 3) * sizeof(char)); // Additional 3 characters for: "/." and terminating zero.
         if (file_name == NULL) {
            mpi_log_error("Unable to allocate memory.");
            MPI_Comm_call_errhandler(MPI_COMM_WORLD, MPI_ERR_PMEM_NO_MEM);
            return MPI_ERR_PMEM_NO_MEM;
         }
         sprintf(file_name, "%s/.%s", mpi_pmem_root_path, name);
         if (remove(file_name) != 0) {
            mpi_log_error("Unable to delete file '%s'.", file_name);
            MPI_Comm_call_errhandler(MPI_COMM_WORLD, MPI_ERR_PMEM);
            return MPI_ERR_PMEM;
         }

         // Remove data file.
         sprintf(file_name, "%s/%s", mpi_pmem_root_path, name);
         if (remove(file_name) != 0) {
            mpi_log_error("Unable to delete file '%s'.", file_name);
            MPI_Comm_call_errhandler(MPI_COMM_WORLD, MPI_ERR_PMEM);
            return MPI_ERR_PMEM;
         }
         free(file_name);

         result = unmap_pmem_file(MPI_COMM_WORLD, windows, metadata_file_size);
         CHECK_ERROR_CODE(result);
         mpi_log_debug("Window '%s' successfully deleted.", name);
         return MPI_SUCCESS;
      }
   }

   result = unmap_pmem_file(MPI_COMM_WORLD, windows, metadata_file_size);
   CHECK_ERROR_CODE(result);
   mpi_log_error("Window '%s' not found.", name);
   MPI_Comm_call_errhandler(MPI_COMM_WORLD, MPI_ERR_PMEM_NAME);
   return MPI_ERR_PMEM_NAME;
}

int MPI_Win_pmem_delete_version(const char *name, int version) {
   int result, i;
   MPI_Win_pmem_metadata *windows;
   MPI_Win_pmem_version *versions;
   off_t file_size;
   bool found;
   char *file_name;

   // Check in global metadata file if window exists.
   result = open_windows_metadata_file(MPI_COMM_WORLD, &windows, &file_size);
   CHECK_ERROR_CODE(result);
   found = false;
   for (i = 0; windows[i].flags != MPI_PMEM_FLAG_NO_OBJECT; i++) {
      if (strcmp(windows[i].name, name) == 0) {
         if (windows[i].flags == MPI_PMEM_FLAG_OBJECT_EXISTS) {
            found = true;
         }
         break;
      }
   }
   result = unmap_pmem_file(MPI_COMM_WORLD, windows, file_size);
   CHECK_ERROR_CODE(result);
   if (!found) {
      mpi_log_error("Window '%s' not found.", name);
      MPI_Comm_call_errhandler(MPI_COMM_WORLD, MPI_ERR_PMEM_NAME);
      return MPI_ERR_PMEM_NAME;
   }

   // Check if version exist and delete it.
   result = open_versions_metadata_file(MPI_COMM_WORLD, name, &versions, &file_size);
   CHECK_ERROR_CODE(result);
   for (i = 0; versions[i].flags != MPI_PMEM_FLAG_NO_OBJECT; i++) {
      if (i == version) {
         // Check if window versions is already deleted.
         if (versions[i].flags == MPI_PMEM_FLAG_OBJECT_DELETED) {
            mpi_log_debug("Version %d of window '%s' already deleted.", version, name);
            result = unmap_pmem_file(MPI_COMM_WORLD, versions, file_size);
            CHECK_ERROR_CODE(result);
            return MPI_SUCCESS;
         }

         // Set window's version flag to deleted.
         versions[i].flags = MPI_PMEM_FLAG_OBJECT_DELETED;
         result = persist_pmem_file(MPI_COMM_WORLD, &versions[i].flags, sizeof(char));
         CHECK_ERROR_CODE(result);
         result = unmap_pmem_file(MPI_COMM_WORLD, versions, file_size);
         CHECK_ERROR_CODE(result);

         // Delete checkpoint data file.
         // Additional 14 characters for: "/.", "-", 10 characters for checkpoint number (length of maximum 4 byte integer number written in decimal form is 10 characters) and terminating zero.
         file_name = malloc((strlen(mpi_pmem_root_path) + strlen(name) + 14) * sizeof(char));
         if (file_name == NULL) {
            mpi_log_error("Unable to allocate memory.");
            MPI_Comm_call_errhandler(MPI_COMM_WORLD, MPI_ERR_PMEM_NO_MEM);
            return MPI_ERR_PMEM_NO_MEM;
         }
         sprintf(file_name, "%s/.%s-%d", mpi_pmem_root_path, name, version);
         if (remove(file_name) != 0) {
            mpi_log_error("Unable to delete file '%s'.", file_name);
            MPI_Comm_call_errhandler(MPI_COMM_WORLD, MPI_ERR_PMEM);
            return MPI_ERR_PMEM;
         }
         free(file_name);
         mpi_log_debug("Version %d of window '%s' successfully deleted.", version, name);
         return MPI_SUCCESS;
      }
   }

   mpi_log_error("Version %d of window '%s' not found.", version, name);
   MPI_Comm_call_errhandler(MPI_COMM_WORLD, MPI_ERR_PMEM_CKPT_VER);
   return MPI_ERR_PMEM_CKPT_VER;
}
