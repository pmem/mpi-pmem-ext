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
#include <time.h>
#include <libpmem.h>
#include "../common/logger.h"
#include "mpi_win_pmem_helper.h"

int MPI_Win_create_pmem(void *base, MPI_Aint size, int disp_unit, MPI_Info info, MPI_Comm comm, MPI_Win_pmem *win) {
   int result;

   mpi_log_debug("Creating window with base: 0x%lx, size: %lu.", (long int) base, size);

   result = MPI_Win_create(base, size, disp_unit, info, comm, &win->win);
   CHECK_ERROR_CODE(result);

   result = set_default_window_metadata(win, comm);
   CHECK_ERROR_CODE(result);
   if (info == MPI_INFO_NULL) {
      mpi_log_debug("MPI_Info object is NULL.");
   } else {
      result = parse_mpi_info_bool(info, "pmem_is_pmem", &win->is_pmem);
      CHECK_ERROR_CODE(result);
   }
   win->modifiable_values->memory_areas = malloc(sizeof(MPI_Win_memory_areas_list));
   if (win->modifiable_values->memory_areas == NULL) {
      mpi_log_error("Unable to allocate memory.");
      MPI_Comm_call_errhandler(comm, MPI_ERR_PMEM_NO_MEM);
      return MPI_ERR_PMEM_NO_MEM;
   }
   win->modifiable_values->memory_areas->base = base;
   win->modifiable_values->memory_areas->size = size;
   win->modifiable_values->memory_areas->next = NULL;
   if (win->is_pmem == true) {
      win->modifiable_values->memory_areas->is_pmem = pmem_is_pmem(base, size);
   }
   
   mpi_log_debug("Window with base: 0x%lx, size: %lu created.", (long int) base, size);

   return MPI_SUCCESS;
}

int MPI_Win_allocate_pmem(MPI_Aint size, int disp_unit, MPI_Info info, MPI_Comm comm, void *baseptr, MPI_Win_pmem *win) {
   int result;
   char *file_name;
   MPI_Win_pmem_version *versions;
   off_t versions_file_size;
   void **pmem_ptr = baseptr;
   bool window_exists;

   mpi_log_debug("Allocating window of size: %lu.", size);

   // Parse MPI_Info.
   result = set_default_window_metadata(win, comm);
   CHECK_ERROR_CODE(result);
   if (info == MPI_INFO_NULL) {
      mpi_log_debug("MPI_Info object is NULL.");
   } else {
      result = parse_mpi_info_bool(info, "pmem_is_pmem", &win->is_pmem);
      CHECK_ERROR_CODE(result);
      if (win->is_pmem) {
         result = parse_mpi_info_bool(info, "pmem_allocate_in_ram", &win->allocate_in_ram);
         CHECK_ERROR_CODE(result);
         bool dont_use_transactions;
         win->created_via_allocate = true;
         result = parse_mpi_info_bool(info, "pmem_dont_use_transactions", &dont_use_transactions);
         CHECK_ERROR_CODE(result);
         win->modifiable_values->transactional = !dont_use_transactions;
         if (win->modifiable_values->transactional) {
            result = parse_mpi_info_bool(info, "pmem_keep_all_checkpoints", &win->modifiable_values->keep_all_checkpoints);
            CHECK_ERROR_CODE(result);
         }
         result = parse_mpi_info_name(comm, info, win->name);
         CHECK_ERROR_CODE(result);
         result = parse_mpi_info_mode(comm, info, &win->mode);
         CHECK_ERROR_CODE(result);
         if (win->mode == MPI_PMEM_MODE_CHECKPOINT) {
            result = parse_mpi_info_checkpoint_version(comm, info, &win->modifiable_values->last_checkpoint_version);
            CHECK_ERROR_CODE(result);
            result = parse_mpi_info_bool(info, "pmem_append_checkpoints", &win->append_checkpoints);
            CHECK_ERROR_CODE(result);
            result = parse_mpi_info_bool(info, "pmem_global_checkpoint", &win->global_checkpoint);
            CHECK_ERROR_CODE(result);
         }
         result = parse_mpi_info_bool(info, "pmem_volatile", &win->is_volatile);
         CHECK_ERROR_CODE(result);
      }
   }

   // Allocate memory and update metadata.
   if (win->is_pmem) {
      file_name = malloc((strlen(mpi_pmem_root_path) + strlen(win->name) + 3) * sizeof(char)); // Additional 3 characters for: "/." and terminating zero.
      if (file_name == NULL) {
         mpi_log_error("Unable to allocate memory.");
         MPI_Comm_call_errhandler(comm, MPI_ERR_PMEM_NO_MEM);
         return MPI_ERR_PMEM_NO_MEM;
      }

      // Check if window already exists in metadata and create it if not.
      if (!win->is_volatile) {
         result = check_if_window_exists_and_its_size(win, size, &window_exists);
         CHECK_ERROR_CODE(result);
         sprintf(file_name, "%s/.%s", mpi_pmem_root_path, win->name);
         if (window_exists) {
            result = get_file_size(comm, file_name, &versions_file_size);
            CHECK_ERROR_CODE(result);
            result = open_pmem_file(comm, file_name, versions_file_size, (void**) &versions);
            CHECK_ERROR_CODE(result);
            // Cleanup old window's versions when expanding window.
            if (win->mode == MPI_PMEM_MODE_EXPAND) {
               result = delete_old_checkpoints(win->comm, win->name, versions);
               CHECK_ERROR_CODE(result);
               result = update_window_size_in_metadata_file(win, size);
               CHECK_ERROR_CODE(result);
            }
         } else {
            if (win->mode == MPI_PMEM_MODE_CHECKPOINT) {
               mpi_log_error("Window with name '%s' doesn't exist.", win->name);
               MPI_Comm_call_errhandler(comm, MPI_ERR_PMEM_NAME);
               return MPI_ERR_PMEM_NAME;
            }
            result = create_window_metadata_file(comm, file_name, &versions, win->name, size);
            CHECK_ERROR_CODE(result);
            versions_file_size = sizeof(MPI_Win_pmem_version);
         }
         result = set_checkpoint_versions(win, versions);
         CHECK_ERROR_CODE(result);
         result = unmap_pmem_file(comm, versions, versions_file_size);
         CHECK_ERROR_CODE(result);
      }

      // Allocate memory.
      if (win->allocate_in_ram) {
         *pmem_ptr = malloc(size);
         if (*pmem_ptr == NULL) {
            mpi_log_error("Unable to allocate memory.");
            MPI_Comm_call_errhandler(comm, MPI_ERR_PMEM_NO_MEM);
            return MPI_ERR_PMEM_NO_MEM;
         }
      } else {
         sprintf(file_name, "%s/%s", mpi_pmem_root_path, win->name);
         result = open_pmem_file(comm, file_name, size, pmem_ptr);
         CHECK_ERROR_CODE(result);
      }

      if (!win->is_volatile && win->mode == MPI_PMEM_MODE_CHECKPOINT) {
         result = copy_data_from_checkpoint(*win, size, *pmem_ptr);
         CHECK_ERROR_CODE(result);
      }

      free(file_name);

      result = MPI_Win_create(*pmem_ptr, size, disp_unit, info, comm, &win->win);
      CHECK_ERROR_CODE(result);

      win->modifiable_values->memory_areas = malloc(sizeof(MPI_Win_memory_areas_list));
      if (win->modifiable_values->memory_areas == NULL) {
         mpi_log_error("Unable to allocate memory.");
         MPI_Comm_call_errhandler(comm, MPI_ERR_PMEM_NO_MEM);
         return MPI_ERR_PMEM_NO_MEM;
      }
      win->modifiable_values->memory_areas->base = *pmem_ptr;
      win->modifiable_values->memory_areas->size = size;
      win->modifiable_values->memory_areas->next = NULL;
      win->modifiable_values->memory_areas->is_pmem = pmem_is_pmem(*pmem_ptr, size);
   } else {
      result = MPI_Win_allocate(size, disp_unit, info, comm, baseptr, &win->win);
      CHECK_ERROR_CODE(result);
   }

   mpi_log_debug("Window of size: %lu allocated.", size);

   return MPI_SUCCESS;
}

int MPI_Win_allocate_shared_pmem(MPI_Aint size, int disp_unit, MPI_Info info, MPI_Comm comm, void *baseptr, MPI_Win_pmem *win) {
   int result;

   result = MPI_Win_allocate_shared(size, disp_unit, info, comm, baseptr, &win->win);
   CHECK_ERROR_CODE(result);
   result = set_default_window_metadata(win, comm);
   CHECK_ERROR_CODE(result);
   
   return MPI_SUCCESS;
}

int MPI_Win_shared_query_pmem(MPI_Win_pmem win, int rank, MPI_Aint *size, int *disp_unit, void *baseptr) {
   return MPI_Win_shared_query(win.win, rank, size, disp_unit, baseptr);
}

int MPI_Win_create_dynamic_pmem(MPI_Info info, MPI_Comm comm, MPI_Win_pmem *win) {
   int result;

   mpi_log_debug("Creating dynamic window.");

   result = MPI_Win_create_dynamic(info, comm, &win->win);
   CHECK_ERROR_CODE(result);
   result = set_default_window_metadata(win, comm);
   CHECK_ERROR_CODE(result);
   if (info == MPI_INFO_NULL) {
      mpi_log_debug("MPI_Info object is NULL.");
   } else {
      result = parse_mpi_info_bool(info, "pmem_is_pmem", &win->is_pmem);
      CHECK_ERROR_CODE(result);
   }
   
   mpi_log_debug("Dynamic window created.");

   return MPI_SUCCESS;
}

int MPI_Win_attach_pmem(MPI_Win_pmem win, void *base, MPI_Aint size) {
   int result;
   MPI_Win_memory_areas_list *list_item;

   mpi_log_debug("Attaching memory area with base: 0x%lx, size: %lu.", (long int) base, size);

   result = MPI_Win_attach(win.win, base, size);
   CHECK_ERROR_CODE(result);
   list_item = malloc(sizeof(MPI_Win_memory_areas_list));
   if (list_item == NULL) {
      mpi_log_error("Unable to allocate memory.");
      MPI_Win_call_errhandler(win.win, MPI_ERR_PMEM_NO_MEM);
      return MPI_ERR_PMEM_NO_MEM;
   }
   list_item->base = base;
   list_item->size = size;
   list_item->next = win.modifiable_values->memory_areas;
   win.modifiable_values->memory_areas = list_item;
   if (win.is_pmem == true) {
      list_item->is_pmem = pmem_is_pmem(base, size);
   }

   mpi_log_debug("Memory area with base: 0x%lx, size: %lu attached.", (long int) base, size);

   return MPI_SUCCESS;
}

int MPI_Win_detach_pmem(MPI_Win_pmem win, const void *base) {
   int result;
   MPI_Win_memory_areas_list *current_item, *previous_item;

   mpi_log_debug("Detaching memory area with base: 0x%lx.", (long int) base);

   result = MPI_Win_detach(win.win, base);
   CHECK_ERROR_CODE(result);
   previous_item = NULL;
   current_item = win.modifiable_values->memory_areas;
   while (current_item != NULL) {
      if (current_item->base == base) {
         if (previous_item == NULL) {
            win.modifiable_values->memory_areas = current_item->next;
         } else {
            previous_item->next = current_item->next;
         }
         free(current_item);
         mpi_log_debug("Memory area with base: 0x%lx detached.", (long int) base);
         return MPI_SUCCESS;
      } else {
         previous_item = current_item;
         current_item = current_item->next;
      }
   }
   mpi_log_error("Memory area with base: 0x%lx not found on list of attached memories.");
   MPI_Win_call_errhandler(win.win, MPI_ERR_ARG);
   return MPI_ERR_ARG;
}

int MPI_Win_free_pmem(MPI_Win_pmem *win) {
   int result;
   MPI_Win_memory_areas_list *current_item, *next_item;
   char file_name[MPI_PMEM_MAX_ROOT_PATH + MPI_PMEM_MAX_NAME];

   mpi_log_debug("Freeing window.");

   result = MPI_Win_free(&win->win);
   CHECK_ERROR_CODE(result);

   // If allocated via MPI_Win_allocate unmap memory and delete file if set as volatile.
   if (win->created_via_allocate) {
      if (win->allocate_in_ram) {
         mpi_log_debug("Freeing memory area base: 0x%lx, size: %lu.", (long int) win->modifiable_values->memory_areas->base, win->modifiable_values->memory_areas->size);
         free(win->modifiable_values->memory_areas->base);
      } else {
         mpi_log_debug("Unmapping memory area base: 0x%lx, size: %lu.", (long int) win->modifiable_values->memory_areas->base, win->modifiable_values->memory_areas->size);
         result = unmap_pmem_file(win->comm, win->modifiable_values->memory_areas->base, win->modifiable_values->memory_areas->size);
         CHECK_ERROR_CODE(result);
         if (win->is_volatile) {
            sprintf(file_name, "%s/%s", mpi_pmem_root_path, win->name);
            mpi_log_debug("Deleting file: %s", file_name);
            if (remove(file_name) != 0) {
               mpi_log_error("Unable to delete file '%s'.", file_name);
               MPI_Comm_call_errhandler(win->comm, MPI_ERR_PMEM);
               return MPI_ERR_PMEM;
            }
         }
      }
   }

   // Clear list of memory areas.
   current_item = win->modifiable_values->memory_areas;
   while (current_item != NULL) {
      next_item = current_item->next;
      mpi_log_debug("Freeing memory area metadata base: 0x%lx, size: %lu.", (long int) current_item->base, current_item->size);
      free(current_item);
      current_item = next_item;
   }
   free(win->modifiable_values);

   mpi_log_debug("Window freed.");

   return MPI_SUCCESS;
}

int MPI_Win_get_attr_pmem(MPI_Win_pmem win, int win_keyval, void *attribute_val, int *flag) {
   return MPI_Win_get_attr(win.win, win_keyval, attribute_val, flag);
}

int MPI_Win_set_attr_pmem(MPI_Win_pmem win, int win_keyval, void *attribute_val) {
   return MPI_Win_set_attr(win.win, win_keyval, attribute_val);
}

int MPI_Win_get_group_pmem(MPI_Win_pmem win, MPI_Group *group) {
   return MPI_Win_get_group(win.win, group);
}

int MPI_Win_set_info_pmem(MPI_Win_pmem win, MPI_Info info) {
   int result;
   bool dont_use_transactions;

   mpi_log_debug("Setting window info.");

   result = MPI_Win_set_info(win.win, info);
   CHECK_ERROR_CODE(result);

   if (win.created_via_allocate) {
      result = parse_mpi_info_bool(info, "pmem_dont_use_transactions", &dont_use_transactions);
      CHECK_ERROR_CODE(result);
      win.modifiable_values->transactional = !dont_use_transactions;
      if (win.modifiable_values->transactional) {
         result = parse_mpi_info_bool(info, "pmem_keep_all_checkpoints", &win.modifiable_values->keep_all_checkpoints);
         CHECK_ERROR_CODE(result);
      }
   }

   mpi_log_debug("Window info set.");

   return MPI_SUCCESS;
}

int MPI_Win_get_info_pmem(MPI_Win_pmem win, MPI_Info *info_used) {
   int result;
   char checkpoint_version[11]; // 10 characters for checkpoint number (length of maximum 4 byte integer number written in decimal form is 10 characters) and terminating zero.

   mpi_log_debug("Getting window info.");

   result = MPI_Win_get_info(win.win, info_used);
   CHECK_ERROR_CODE(result);

   if (win.is_pmem) {
      result = MPI_Info_set(*info_used, "pmem_is_pmem", "true");
      CHECK_ERROR_CODE(result);
      if (win.created_via_allocate) {
         result = MPI_Info_set(*info_used, "pmem_allocate_in_ram", win.allocate_in_ram ? "true" : "false");
         CHECK_ERROR_CODE(result);
         result = MPI_Info_set(*info_used, "pmem_dont_use_transactions", win.modifiable_values->transactional ? "false" : "true");
         CHECK_ERROR_CODE(result);
         if (win.modifiable_values->transactional) {
            result = MPI_Info_set(*info_used, "pmem_keep_all_checkpoints", win.modifiable_values->keep_all_checkpoints ? "true" : "false");
            CHECK_ERROR_CODE(result);
         }
         if (win.mode == MPI_PMEM_MODE_CHECKPOINT) {
            sprintf(checkpoint_version, "%d", win.modifiable_values->last_checkpoint_version);
            result = MPI_Info_set(*info_used, "pmem_checkpoint_version", checkpoint_version);
            CHECK_ERROR_CODE(result);
            result = MPI_Info_set(*info_used, "pmem_append_checkpoints", win.append_checkpoints ? "true" : "false");
            CHECK_ERROR_CODE(result);
            result = MPI_Info_set(*info_used, "pmem_global_checkpoint", win.global_checkpoint ? "true" : "false");
            CHECK_ERROR_CODE(result);
         }
         result = MPI_Info_set(*info_used, "pmem_name", win.name);
         CHECK_ERROR_CODE(result);
         switch (win.mode) {
         case MPI_PMEM_MODE_EXPAND:
            result = MPI_Info_set(*info_used, "pmem_mode", "expand");
            break;
         case MPI_PMEM_MODE_CHECKPOINT:
            result = MPI_Info_set(*info_used, "pmem_mode", "checkpoint");
            break;
         default:
            break;
         }
         CHECK_ERROR_CODE(result);
         result = MPI_Info_set(*info_used, "pmem_volatile", win.is_volatile ? "true" : "false");
      }
   } else {
      result = MPI_Info_set(*info_used, "pmem_is_pmem", "false");
      CHECK_ERROR_CODE(result);
   }

   mpi_log_debug("Window info returned.");

   return MPI_SUCCESS;
}
