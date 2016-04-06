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


#ifndef __MPI_WIN_PMEM_DATATYPES_H__
#define __MPI_WIN_PMEM_DATATYPES_H__

#include <stdbool.h>
#include <time.h>
#include <mpi.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MPI_PMEM_MAX_NAME 256

// Allocated windows modes.
#define MPI_PMEM_MODE_EXPAND 0
#define MPI_PMEM_MODE_CHECKPOINT 1

// Flags used in metadata files.
#define MPI_PMEM_FLAG_NO_OBJECT 0
#define MPI_PMEM_FLAG_OBJECT_EXISTS 1
#define MPI_PMEM_FLAG_OBJECT_DELETED 3

typedef struct MPI_Win_pmem_structure MPI_Win_pmem;
typedef struct MPI_Win_pmem_modifiable_structure MPI_Win_pmem_modifiable;
typedef struct MPI_Win_memory_areas_list_structure MPI_Win_memory_areas_list;
typedef struct MPI_Win_pmem_metadata_structure MPI_Win_pmem_metadata;
typedef struct MPI_Win_pmem_version_structure MPI_Win_pmem_version;
typedef struct MPI_Win_pmem_windows_structure MPI_Win_pmem_windows;
typedef struct MPI_Win_pmem_window_structure MPI_Win_pmem_window;
typedef struct MPI_Win_pmem_versions_structure MPI_Win_pmem_versions;

// Structure containing information about window.
struct MPI_Win_pmem_structure {
   MPI_Win win;
   MPI_Comm comm;
   bool allocate_in_ram;
   bool created_via_allocate;
   bool is_pmem;
   bool is_volatile;
   bool append_checkpoints;
   bool global_checkpoint;
   char name[MPI_PMEM_MAX_NAME];
   int mode;
   MPI_Win_pmem_modifiable *modifiable_values;
};

// Modifiable part of information about window.
struct MPI_Win_pmem_modifiable_structure {
   bool transactional;
   bool keep_all_checkpoints;
   int last_checkpoint_version;     // Checkpoint version saved last time.
   int next_checkpoint_version;     // Checkpoint version that should be created next time.
   int highest_checkpoint_version;  // Highest checkpoint version of this window that was found in window's versions metadata file. This equals to index of terminating record - 1.
   MPI_Win_memory_areas_list *memory_areas;
};

// List structure of memory areas attached to window.
struct MPI_Win_memory_areas_list_structure {
   void *base;
   MPI_Aint size;
   bool is_pmem; // Is this memory real pmem (result of pmem_is_pmem)
   MPI_Win_memory_areas_list *next;
};

// Metadata structure about window. Saved in global metadata file.
struct MPI_Win_pmem_metadata_structure {
   char name[MPI_PMEM_MAX_NAME];
   MPI_Aint size;
   char flags;
};

// Metadata structure about single window version. Saved in window's versions metadata file.
struct MPI_Win_pmem_version_structure {
   int version;
   time_t timestamp;
   char flags;
};

// Metadata structure filled by MPI_Win_pmem_list.
struct MPI_Win_pmem_windows_structure {
   int size;
   MPI_Win_pmem_window *windows;
};

// Metadata about one window in MPI_Win_pmem_windows_structure.
struct MPI_Win_pmem_window_structure {
   char name[MPI_PMEM_MAX_NAME];
   MPI_Aint size;
};

// Metadata structure filled by MPI_Win_pmem_get_versions.
struct MPI_Win_pmem_versions_structure {
   int size;
   MPI_Win_pmem_version *versions;
};

#ifdef __cplusplus
}
#endif

#endif
