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


#include "mpi_win_pmem_helper.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <libpmem.h>
#include "../common/error_codes.h"
#include "../common/logger.h"
#include "mpi_win_pmem.h"

int open_pmem_file(MPI_Comm comm, const char *file_name, MPI_Aint size, void **address) {
   int fd;

   // Open file.
   if ((fd = open(file_name, O_CREAT | O_RDWR, 0666)) < 0) {
      mpi_log_error("Unable to open file '%s'.", file_name);
      MPI_Comm_call_errhandler(comm, MPI_ERR_PMEM);
      return MPI_ERR_PMEM;
   }

   // Allocate memory for the file.
   if (posix_fallocate(fd, 0, size) != 0) {
      mpi_log_error("Unable to allocate disk space for file '%s'.", file_name);
      close(fd);
      MPI_Comm_call_errhandler(comm, MPI_ERR_NO_SPACE);
      return MPI_ERR_NO_SPACE;
   }

   // Memory map file.
   if ((*address = pmem_map(fd)) == NULL) {
      mpi_log_error("Unable to map file '%s' to memory.", file_name);
      close(fd);
      MPI_Comm_call_errhandler(comm, MPI_ERR_PMEM);
      return MPI_ERR_PMEM;
   }
   close(fd);

   return MPI_SUCCESS;
}

int persist_pmem_file(MPI_Comm comm, void *address, MPI_Aint size) {
   if (pmem_is_pmem(address, size)) {
      //pmem_persist(address, size);
      pmem_msync(address, size);
      pmem_drain();
   } else {
      if (pmem_msync(address, size) != 0) {
         mpi_log_error("Unable to msync memory area base: 0x%lx, size: %lu.", (long int) address, size);
         MPI_Comm_call_errhandler(comm, MPI_ERR_PMEM);
         return MPI_ERR_PMEM;
      }
   }

   return MPI_SUCCESS;
}

int unmap_pmem_file(MPI_Comm comm, void *address, MPI_Aint size) {
   int result;

   result = munmap(address, size);
   if (result != 0) {
      mpi_log_error("Unable to unmap memory area base: 0x%lx, size: %lu.", (long int) address, size);
      MPI_Comm_call_errhandler(comm, MPI_ERR_PMEM);
      return MPI_ERR_PMEM;
   }

   return MPI_SUCCESS;
}

bool check_if_file_exist(const char *file_name) {
   FILE *file;
   if ((file = fopen(file_name, "r")) != NULL) {
      fclose(file);
      return true;
   }
   return false;
}

int get_file_size(MPI_Comm comm, const char *file_name, off_t *size) {
   int result;
   struct stat file_status;

   result = stat(file_name, &file_status);
   if (result != 0) {
      mpi_log_error("Unable to check file size of file '%s'.", file_name);
      MPI_Comm_call_errhandler(comm, MPI_ERR_PMEM);
      return MPI_ERR_PMEM;
   }
   *size = file_status.st_size;

   return MPI_SUCCESS;
}

int open_windows_metadata_file(MPI_Comm comm, MPI_Win_pmem_metadata **windows, off_t *size) {
   int result;
   char metadata_file_name[MPI_PMEM_MAX_ROOT_PATH + 9]; // 9 == length of "/.windows"

   sprintf(metadata_file_name, "%s/.windows", mpi_pmem_root_path);
   result = get_file_size(comm, metadata_file_name, size);
   CHECK_ERROR_CODE(result);
   result = open_pmem_file(comm, metadata_file_name, *size, (void**) windows);
   CHECK_ERROR_CODE(result);

   return MPI_SUCCESS;
}

int open_versions_metadata_file(MPI_Comm comm, const char *window_name, MPI_Win_pmem_version **versions, off_t *size) {
   int result;
   char *file_name;

   file_name = malloc((strlen(mpi_pmem_root_path) + strlen(window_name) + 3) * sizeof(char)); // Additional 3 characters for: "/." and terminating zero.
   if (file_name == NULL) {
      mpi_log_error("Unable to allocate memory.");
      MPI_Comm_call_errhandler(comm, MPI_ERR_PMEM_NO_MEM);
      return MPI_ERR_PMEM_NO_MEM;
   }
   sprintf(file_name, "%s/.%s", mpi_pmem_root_path, window_name);
   result = get_file_size(comm, file_name, size);
   CHECK_ERROR_CODE(result);
   result = open_pmem_file(comm, file_name, *size, (void**) versions);
   CHECK_ERROR_CODE(result);
   free(file_name);

   return MPI_SUCCESS;
}

int delete_old_checkpoints(MPI_Comm comm, const char *name, MPI_Win_pmem_version *versions) {
   int result, i;
   char *file_name;

   // Additional 14 characters for: "/.", "-", 10 characters for checkpoint number (length of maximum 4 byte integer number written in decimal form is 10 characters) and terminating zero.
   file_name = malloc((strlen(mpi_pmem_root_path) + strlen(name) + 14) * sizeof(char));
   if (file_name == NULL) {
      mpi_log_error("Unable to allocate memory.");
      MPI_Comm_call_errhandler(comm, MPI_ERR_PMEM_NO_MEM);
      return MPI_ERR_PMEM_NO_MEM;
   }

   // Delete all previously created checkpoints.
   for (i = 0; versions[i].flags != MPI_PMEM_FLAG_NO_OBJECT; i++) {
      if (versions[i].flags == MPI_PMEM_FLAG_OBJECT_EXISTS) {
         versions[i].flags = MPI_PMEM_FLAG_OBJECT_DELETED;
         result = persist_pmem_file(comm, &versions[i].flags, sizeof(char));
         CHECK_ERROR_CODE(result);
         sprintf(file_name, "%s/.%s-%d", mpi_pmem_root_path, name, versions[i].version);
         remove(file_name);
      }
   }
   free(file_name);

   // Set 0th record to terminating record.
   versions[0].version = 0;
   versions[0].timestamp = 0;
   versions[0].flags = MPI_PMEM_FLAG_NO_OBJECT;
   result = persist_pmem_file(comm, versions, sizeof(MPI_Win_pmem_version));
   CHECK_ERROR_CODE(result);

   return MPI_SUCCESS;
}

int set_default_window_metadata(MPI_Win_pmem *win, MPI_Comm comm) {
   win->comm = comm;
   win->allocate_in_ram = false;
   win->created_via_allocate = false;
   win->is_pmem = false;
   win->is_volatile = false;
   win->append_checkpoints = false;
   win->global_checkpoint = false;
   win->mode = MPI_PMEM_MODE_EXPAND;
   win->modifiable_values = malloc(sizeof(MPI_Win_pmem_modifiable));
   if (win->modifiable_values == NULL) {
      mpi_log_error("Unable to allocate memory.");
      MPI_Comm_call_errhandler(comm, MPI_ERR_PMEM_NO_MEM);
      return MPI_ERR_PMEM_NO_MEM;
   }
   win->modifiable_values->transactional = false;
   win->modifiable_values->keep_all_checkpoints = false;
   win->modifiable_values->last_checkpoint_version = 0;
   win->modifiable_values->next_checkpoint_version = 0;
   win->modifiable_values->highest_checkpoint_version = 0;
   win->modifiable_values->memory_areas = NULL;

   return MPI_SUCCESS;
}

int parse_mpi_info_bool(MPI_Info info, const char *key, bool *result) {
   int error, flag;
   const int bool_value_length = 5; // Maximum length (without terminating zero) of boolean values "true" and "false" is 5.
   char bool_value[6];
   int value_length;

   error = MPI_Info_get_valuelen(info, key, &value_length, &flag);
   CHECK_ERROR_CODE(error);
   if (flag && value_length <= bool_value_length) {
      error = MPI_Info_get(info, key, bool_value_length, bool_value, &flag);
      CHECK_ERROR_CODE(error);
      if (flag && strcmp(bool_value, "true") == 0) {
         *result = true;
      } else {
         *result = false;
      }
   } else {
      *result = false;
   }

   mpi_log_debug("%s: %s", key, *result ? "true" : "false");

   return MPI_SUCCESS;
}

int parse_mpi_info_name(MPI_Comm comm, MPI_Info info, char *result) {
   int error, flag;
   int value_length;

   error = MPI_Info_get_valuelen(info, "pmem_name", &value_length, &flag);
   CHECK_ERROR_CODE(error);
   if (!flag || value_length > MPI_PMEM_MAX_NAME - 1) {
      mpi_log_error("pmem_name not defined or too long.");
      MPI_Comm_call_errhandler(comm, MPI_ERR_PMEM_NAME);
      return MPI_ERR_PMEM_NAME;
   }
   error = MPI_Info_get(info, "pmem_name", MPI_PMEM_MAX_NAME, result, &flag);
   CHECK_ERROR_CODE(error);

   mpi_log_debug("pmem_name: %s", result);

   return MPI_SUCCESS;
}

int parse_mpi_info_mode(MPI_Comm comm, MPI_Info info, int *result) {
   int error, flag;
   int value_length = 10; // Maximum length (without terminating zero) of proper pmem_mode values: "expand" and "checkpoint" is 10.
   char value[11];

   error = MPI_Info_get(info, "pmem_mode", value_length, value, &flag);
   CHECK_ERROR_CODE(error);
   if (!flag) {
      mpi_log_error("pmem_mode not defined.");
      MPI_Comm_call_errhandler(comm, MPI_ERR_PMEM_MODE);
      return MPI_ERR_PMEM_MODE;
   }
   if (strcmp(value, "expand") == 0) {
      *result = MPI_PMEM_MODE_EXPAND;
   } else if (strcmp(value, "checkpoint") == 0) {
      *result = MPI_PMEM_MODE_CHECKPOINT;
   } else {
      mpi_log_error("Undefined value '%s' for key pmem_mode.", value);
      MPI_Comm_call_errhandler(comm, MPI_ERR_PMEM_MODE);
      return MPI_ERR_PMEM_MODE;
   }

   mpi_log_debug("pmem_mode: %s", value);

   return MPI_SUCCESS;
}

int parse_mpi_info_checkpoint_version(MPI_Comm comm, MPI_Info info, int *result) {
   int error, flag;
   int value_length;
   char *value;

   error = MPI_Info_get_valuelen(info, "pmem_checkpoint_version", &value_length, &flag);
   CHECK_ERROR_CODE(error);
   if (!flag) {
      *result = -1;
      mpi_log_debug("pmem_checkpoint_version: %d", *result);
      return MPI_SUCCESS;
   }
   value = malloc((value_length + 1) * sizeof(char));
   if (value == NULL) {
      mpi_log_error("Unable to allocate memory.");
      MPI_Comm_call_errhandler(comm, MPI_ERR_PMEM_NO_MEM);
      return MPI_ERR_PMEM_NO_MEM;
   }
   error = MPI_Info_get(info, "pmem_checkpoint_version", value_length + 1, value, &flag);
   CHECK_ERROR_CODE(error);
   *result = atoi(value);
   free(value);

   mpi_log_debug("pmem_checkpoint_version: %d", *result);

   return MPI_SUCCESS;
}

int check_if_window_exists_and_its_size(MPI_Win_pmem *win, MPI_Aint size, bool *exists) {
   int result, i;
   char metadata_file_name[MPI_PMEM_MAX_ROOT_PATH + 9]; // 9 == length of "/.windows"
   off_t metadata_file_size;
   MPI_Win_pmem_metadata *windows;

   // Open global metadata file.
   sprintf(metadata_file_name, "%s/.windows", mpi_pmem_root_path);
   result = get_file_size(win->comm, metadata_file_name, &metadata_file_size);
   CHECK_ERROR_CODE(result);
   result = open_pmem_file(win->comm, metadata_file_name, metadata_file_size, (void**) &windows);
   CHECK_ERROR_CODE(result);
   *exists = false;

   // Find window and it's size.
   for (i = 0; windows[i].flags != MPI_PMEM_FLAG_NO_OBJECT; i++) {
      if (strcmp(windows[i].name, win->name) == 0) {
         if (windows[i].flags == MPI_PMEM_FLAG_OBJECT_EXISTS) {
            *exists = true;
            if (win->mode == MPI_PMEM_MODE_CHECKPOINT) {
               if (size != windows[i].size) {
                  mpi_log_error("Requested windows size %d is different than saved size %d.", size, windows[i].size);
                  MPI_Comm_call_errhandler(win->comm, MPI_ERR_SIZE);
                  return MPI_ERR_SIZE;
               }
            }
         }
         break;
      }
   }
   result = unmap_pmem_file(win->comm, windows, metadata_file_size);
   CHECK_ERROR_CODE(result);

   return MPI_SUCCESS;
}

int create_window_metadata_file(MPI_Comm comm, const char *file_name, MPI_Win_pmem_version **versions, const char *window_name, MPI_Aint window_size) {
   int result, i;
   bool found;
   char windows_file_name[MPI_PMEM_MAX_ROOT_PATH + 9]; // 9 == length of "/.windows"
   MPI_Win_pmem_metadata *windows;
   off_t file_size;

   mpi_log_debug("Creating metadata file '%s'.", file_name);

   // Create window's versions metadata file.
   result = open_pmem_file(comm, file_name, sizeof(MPI_Win_pmem_version), (void**) versions);
   CHECK_ERROR_CODE(result);
   (*versions)[0].version = 0;
   (*versions)[0].timestamp = 0;
   (*versions)[0].flags = MPI_PMEM_FLAG_NO_OBJECT;
   result = persist_pmem_file(comm, versions, sizeof(MPI_Win_pmem_version));
   CHECK_ERROR_CODE(result);

   // Open global metadata file.
   sprintf(windows_file_name, "%s/.windows", mpi_pmem_root_path);
   result = get_file_size(comm, windows_file_name, &file_size);
   CHECK_ERROR_CODE(result);
   result = open_pmem_file(comm, windows_file_name, file_size, (void**) &windows);
   CHECK_ERROR_CODE(result);
   found = false;

   // Update window's metadata if it existed previously.
   for (i = 0; windows[i].flags != MPI_PMEM_FLAG_NO_OBJECT; i++) {
      if (strcmp(window_name, windows[i].name) == 0) {
         // Update size
         windows[i].size = window_size;
         result = persist_pmem_file(comm, &windows[i].size, sizeof(MPI_Aint));
         CHECK_ERROR_CODE(result);
         // Set flag to indicate that window exists.
         windows[i].flags = MPI_PMEM_FLAG_OBJECT_EXISTS;
         result = persist_pmem_file(comm, &windows[i].flags, sizeof(char));
         CHECK_ERROR_CODE(result);
         found = true;
         break;
      }
   }
   result = unmap_pmem_file(comm, windows, file_size);
   CHECK_ERROR_CODE(result);

   // Add new record to global metadata file if window with this name is created for the first time.
   if (!found) {
      file_size = (i + 2) * sizeof(MPI_Win_pmem_metadata);
      result = open_pmem_file(comm, windows_file_name, file_size, (void**) &windows);
      CHECK_ERROR_CODE(result);
      // Create new terminating record.
      windows[i + 1].size = 0;
      windows[i + 1].flags = MPI_PMEM_FLAG_NO_OBJECT;
      result = persist_pmem_file(comm, &windows[i + 1], sizeof(MPI_Win_pmem_metadata));
      CHECK_ERROR_CODE(result);
      // Update window's metadata in previous terminating record.
      strcpy(windows[i].name, window_name);
      windows[i].size = window_size;
      result = persist_pmem_file(comm, &windows[i], sizeof(MPI_Win_pmem_metadata));
      CHECK_ERROR_CODE(result);
      // Set flag to indicate that window exists.
      windows[i].flags = MPI_PMEM_FLAG_OBJECT_EXISTS;
      result = persist_pmem_file(comm, &windows[i].flags, sizeof(bool));
      CHECK_ERROR_CODE(result);
      result = unmap_pmem_file(comm, windows, file_size);
      CHECK_ERROR_CODE(result);
   }

   mpi_log_debug("Metadata file '%s' created.", file_name);

   return MPI_SUCCESS;
}

int update_window_size_in_metadata_file(MPI_Win_pmem *win, MPI_Aint size) {
   int result, i;
   off_t metadata_file_size;
   MPI_Win_pmem_metadata *windows;

   result = open_windows_metadata_file(win->comm, &windows, &metadata_file_size);
   CHECK_ERROR_CODE(result);
   for (i = 0; windows[i].flags != MPI_PMEM_FLAG_NO_OBJECT; i++) {
      if (strcmp(windows[i].name, win->name) == 0) {
         if (windows[i].flags != MPI_PMEM_FLAG_OBJECT_EXISTS) {
            mpi_log_error("Window with name '%s' has been deleted.", win->name);
            MPI_Comm_call_errhandler(win->comm, MPI_ERR_PMEM_NAME);
            return MPI_ERR_PMEM_NAME;
         }
         // No need for flag modification as all previous checkpoints will already be deleted and if anything fails, on restart, window will have to be created again in expand mode.
         windows[i].size = size;
         result = persist_pmem_file(win->comm, &windows[i].size, sizeof(MPI_Aint));
         CHECK_ERROR_CODE(result);
         result = unmap_pmem_file(win->comm, windows, metadata_file_size);
         CHECK_ERROR_CODE(result);
         return MPI_SUCCESS;
      }
   }
   result = unmap_pmem_file(win->comm, windows, metadata_file_size);
   CHECK_ERROR_CODE(result);

   mpi_log_error("Window with name '%s' doesn't exist.", win->name);
   MPI_Comm_call_errhandler(win->comm, MPI_ERR_PMEM_NAME);
   return MPI_ERR_PMEM_NAME;
}

int set_checkpoint_versions(MPI_Win_pmem *win, MPI_Win_pmem_version *versions) {
   int i;
   int last_checkpoint_on_all_processes;
   char version_available, version_available_on_all_processes;

   if (win->mode == MPI_PMEM_MODE_CHECKPOINT) {
      if (win->modifiable_values->last_checkpoint_version == -1) { // Last checkpoint version is not set and should be set to highest.
         // Find highest existing checkpoint version.
         for (i = 0; versions[i].flags != MPI_PMEM_FLAG_NO_OBJECT; i++) {
            if (versions[i].flags == MPI_PMEM_FLAG_OBJECT_EXISTS) {
               win->modifiable_values->last_checkpoint_version = versions[i].version;
            }
         }

         // If window is globally consistent find latest consistent version in all processes.
         if (win->global_checkpoint) {
            MPI_Allreduce(&win->modifiable_values->last_checkpoint_version, &last_checkpoint_on_all_processes, 1, MPI_INT, MPI_MIN, win->comm);
            version_available = versions[last_checkpoint_on_all_processes].flags == MPI_PMEM_FLAG_OBJECT_EXISTS ? 1 : 0;
            MPI_Allreduce(&version_available, &version_available_on_all_processes, 1, MPI_CHAR, MPI_MIN, win->comm);
            if (version_available_on_all_processes == 0) {
               mpi_log_error("One of processes doesn't have checkpoint version %d.", last_checkpoint_on_all_processes);
               MPI_Comm_call_errhandler(win->comm, MPI_ERR_PMEM);
               return MPI_ERR_PMEM;
            }
            win->modifiable_values->last_checkpoint_version = last_checkpoint_on_all_processes;
         }
         win->modifiable_values->next_checkpoint_version = win->modifiable_values->last_checkpoint_version + 1;
      } else {
         // Find highest checkpoint version in window's versions metadata file.
         for (i = 0; versions[i].flags != MPI_PMEM_FLAG_NO_OBJECT; i++) {
         }

         // Check if requested checkpoint version exists.
         if (win->modifiable_values->last_checkpoint_version < 0 || win->modifiable_values->last_checkpoint_version >= i ||
             versions[win->modifiable_values->last_checkpoint_version].flags != MPI_PMEM_FLAG_OBJECT_EXISTS) {
            mpi_log_error("Version %d of window '%s' doesn't exist.", win->modifiable_values->last_checkpoint_version, win->name);
            MPI_Comm_call_errhandler(win->comm, MPI_ERR_PMEM_CKPT_VER);
            return MPI_ERR_PMEM_CKPT_VER;
         }
         win->modifiable_values->next_checkpoint_version = win->append_checkpoints ? i : win->modifiable_values->last_checkpoint_version + 1;
      }
      win->modifiable_values->highest_checkpoint_version = i - 1;
   } else {
      // Set default values for checkpoint version's variables.
      win->modifiable_values->next_checkpoint_version = 0;     // Next checkpoint is first in the metadata file.
      win->modifiable_values->last_checkpoint_version = -1;    // There is no last checkpoint.
      win->modifiable_values->highest_checkpoint_version = -1; // There are no old checkpoints.
   }

   return MPI_SUCCESS;
}

int copy_data_from_checkpoint(MPI_Win_pmem win, MPI_Aint size, void *destination) {
   int result;
   char *file_name;
   void *checkpoint_data;

   // Additional 14 characters for: "/.", "-", 10 characters for checkpoint number (length of maximum 4 byte integer number written in decimal form is 10 characters) and terminating zero.
   file_name = malloc((strlen(mpi_pmem_root_path) + strlen(win.name) + 14) * sizeof(char));
   if (file_name == NULL) {
      mpi_log_error("Unable to allocate memory.");
      MPI_Comm_call_errhandler(win.comm, MPI_ERR_PMEM_NO_MEM);
      return MPI_ERR_PMEM_NO_MEM;
   }
   sprintf(file_name, "%s/.%s-%d", mpi_pmem_root_path, win.name, win.modifiable_values->last_checkpoint_version);
   if (check_if_file_exist(file_name)) {
      result = open_pmem_file(win.comm, file_name, size, &checkpoint_data);
      CHECK_ERROR_CODE(result);
      memcpy(destination, checkpoint_data, size);
      result = unmap_pmem_file(win.comm, checkpoint_data, size);
      CHECK_ERROR_CODE(result);
   } else {
      mpi_log_error("Checkpoint file '%s' doesn't exist.", file_name);
      MPI_Comm_call_errhandler(win.comm, MPI_ERR_PMEM);
      return MPI_ERR_PMEM;
   }
   free(file_name);

   return MPI_SUCCESS;
}

int create_checkpoint(MPI_Win_pmem win, bool fence) {
   int result;
   char *file_name;
   FILE *checkpoint_file;
   int root_file_descriptor;
   MPI_Win_pmem_version *versions;
   off_t versions_file_size;
   int next_checkpoint_version, last_checkpoint_version, highest_checkpoint_version;
   bool creating_new_version = false;

   if (win.is_pmem && !win.is_volatile && win.modifiable_values->transactional) {
      // Update checkpoint version variables.
      next_checkpoint_version = win.modifiable_values->next_checkpoint_version++;
      if (next_checkpoint_version > win.modifiable_values->highest_checkpoint_version) {
         win.modifiable_values->highest_checkpoint_version = next_checkpoint_version;
         creating_new_version = true;
      }
      last_checkpoint_version = win.modifiable_values->last_checkpoint_version;
      win.modifiable_values->last_checkpoint_version = next_checkpoint_version;
      highest_checkpoint_version = win.modifiable_values->highest_checkpoint_version;

      // Open checkpoint and metadata files.
      // Additional 14 characters for: "/.", "-", 10 characters for checkpoint number (length of maximum 4 byte integer number written in decimal form is 10 characters) and terminating zero.
      file_name = malloc((strlen(mpi_pmem_root_path) + strlen(win.name) + 14) * sizeof(char));
      if (file_name == NULL) {
         mpi_log_error("Unable to allocate memory.");
         MPI_Win_call_errhandler(win.win, MPI_ERR_PMEM_NO_MEM);
         return MPI_ERR_PMEM_NO_MEM;
      }
      sprintf(file_name, "%s/.%s-%d", mpi_pmem_root_path, win.name, next_checkpoint_version);
      mpi_log_debug("Creating checkpoint in file '%s'.", file_name);
      checkpoint_file = fopen(file_name, "wb");
      if (checkpoint_file == NULL) {
         mpi_log_error("Unable to open checkpoint file '%s'.", file_name);
         MPI_Win_call_errhandler(win.win, MPI_ERR_PMEM);
         return MPI_ERR_PMEM;
      }
      sprintf(file_name, "%s/.%s", mpi_pmem_root_path, win.name);
      versions_file_size = (highest_checkpoint_version + 2) * sizeof(MPI_Win_pmem_version);
      result = open_pmem_file(win.comm, file_name, versions_file_size, (void**) &versions);
      CHECK_ERROR_CODE(result);

      // Set checkpoint version flag to deleted in window's versions file if new checkpoint is overwriting the old one.
      if (!creating_new_version) {
         versions[next_checkpoint_version].flags = MPI_PMEM_FLAG_OBJECT_DELETED;
         result = persist_pmem_file(win.comm, &versions[next_checkpoint_version].flags, sizeof(char));
         CHECK_ERROR_CODE(result);
      }

      // Copy data to checkpoint file.
      fwrite(win.modifiable_values->memory_areas->base, 1, win.modifiable_values->memory_areas->size, checkpoint_file);
      fflush(checkpoint_file);
      fsync(fileno(checkpoint_file));
      fclose(checkpoint_file);
      // Sync also directory containing checkpoints.
      root_file_descriptor = open(mpi_pmem_root_path, O_RDWR);
      fsync(root_file_descriptor);
      close(root_file_descriptor);

      // Update checkpoint version in window's versions metadata file.
      if (creating_new_version) {
         // Create new terminating record.
         versions[highest_checkpoint_version + 1].version = 0;
         versions[highest_checkpoint_version + 1].timestamp = 0;
         versions[highest_checkpoint_version + 1].flags = MPI_PMEM_FLAG_NO_OBJECT;
         result = persist_pmem_file(win.comm, &versions[highest_checkpoint_version], sizeof(MPI_Win_pmem_version));
         CHECK_ERROR_CODE(result);
      }
      // Update checkpoint version metadata in window's versions metadata file.
      versions[next_checkpoint_version].version = next_checkpoint_version;
      versions[next_checkpoint_version].timestamp = time(NULL);
      result = persist_pmem_file(win.comm, &versions[next_checkpoint_version], sizeof(MPI_Win_pmem_version));
      CHECK_ERROR_CODE(result);
      // Set flag indicating that new checkpoint version exists.
      versions[next_checkpoint_version].flags = MPI_PMEM_FLAG_OBJECT_EXISTS;
      result = persist_pmem_file(win.comm, &versions[next_checkpoint_version].flags, sizeof(char));
      CHECK_ERROR_CODE(result);

      // Delete last checkpoint if not specified not to do so.
      if (!win.modifiable_values->keep_all_checkpoints) {
         if (fence) {
            MPI_Barrier(win.comm);
         }
         if (last_checkpoint_version != -1) {
            // Set flag indicating that checkpoint version is deleted.
            versions[last_checkpoint_version].flags = MPI_PMEM_FLAG_OBJECT_DELETED;
            result = persist_pmem_file(win.comm, &versions[last_checkpoint_version].flags, sizeof(char));
            CHECK_ERROR_CODE(result);
            // Delete checkpoint data file.
            sprintf(file_name, "%s/.%s-%d", mpi_pmem_root_path, win.name, last_checkpoint_version);
            if (remove(file_name) != 0) {
               mpi_log_error("Unable to delete file '%s'.", file_name);
               MPI_Win_call_errhandler(win.win, MPI_ERR_PMEM);
               return MPI_ERR_PMEM;
            }
         }
      }
      result = unmap_pmem_file(win.comm, versions, versions_file_size);
      CHECK_ERROR_CODE(result);
      free(file_name);
   }

   return MPI_SUCCESS;
}
