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


#include "failure_recovery.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dirent.h>
#include <libgen.h>
#include <fcntl.h>
#include <unistd.h>

#include "file_io_distributed_cache.h"
#include "util.h"
#include "logger.h"

static char RECOVERY_DATA_INACTIVE = 0;
static char RECOVERY_DATA_ACTIVE   = 1;

struct fail_recovery_meta {
   unsigned long long offset;
   unsigned long long size;
};

void sync_file(FILE* f) {
   fflush(f);
   int file_no = fileno(f);
   fsync(file_no);
}

char* recovery_data_create(char* data, unsigned long long offset, unsigned long long size, char* cache_path) {
   char* recovery_data_file_name = concat(cache_path, generate_random_string(32));
   FILE* f = fopen(recovery_data_file_name, "w");
   if (f == NULL) {
       return NULL;
   }

   struct fail_recovery_meta meta;
   meta.size = size;
   meta.offset = offset;

   fwrite(&RECOVERY_DATA_INACTIVE, 1, 1, f);
   fwrite(&meta, 1, sizeof(struct fail_recovery_meta), f);
   fwrite(data, 1, size, f);
   sync_file(f);

   fseek(f, 0, SEEK_SET);
   fwrite(&RECOVERY_DATA_ACTIVE, 1, 1, f);
   sync_file(f);

   fclose(f);

   // sync parent directory
   char* filename_copy = concat(recovery_data_file_name, "");
   dirname(filename_copy);
   int parent_descriptor = open(filename_copy, O_RDWR);
   fsync(parent_descriptor);
   close(parent_descriptor);

   return recovery_data_file_name;
}

void recovery_data_remove(char* recovery_data_file_name) {
   remove(recovery_data_file_name);
   free(recovery_data_file_name);
}

void perform_data_recovery(MPI_File_pmem* fh_pmem, char* cache_path) {
   DIR *dir;
   struct dirent* ent;
   dir = opendir(cache_path);
   if (dir != NULL) {
      ent = readdir(dir);
      while (ent != NULL) {
         if (strcmp(ent->d_name, ".") && strcmp(ent->d_name, "..") && strcmp(ent->d_name, "cache")) {
            FILE *f = fopen(concat(cache_path, ent->d_name), "r");
            char active_flag;
            fread(&active_flag, 1, 1, f);
            if (active_flag) {
               struct fail_recovery_meta meta;
               fread(&meta, sizeof(struct fail_recovery_meta), 1, f);
               char *buffer = (char*) malloc(meta.size);
               fread(buffer, meta.size, 1, f);
               MPI_File_write_at_pmem_distributed(fh_pmem, meta.offset, buffer, meta.size, MPI_BYTE, MPI_STATUSES_IGNORE);
               fclose(f);
               remove(concat(cache_path, ent->d_name));
            }
         }
         ent = readdir(dir);
      }
      closedir(dir);
   }
}
