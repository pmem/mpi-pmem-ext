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


#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>

#include "logger.h"
#include "util.h"

#define LOG_START_ERROR "ERROR: "
#define LOG_START_INFO " INFO: "
#define LOG_START_DEBUG "DEBUG: "

bool is_error_log_enabled() {

#ifdef _LOG_ERROR
   return true;
#endif

#ifdef _LOG_INFO
   return true;
#endif

#ifdef _LOG_DEBUG
   return true;
#endif

   return false;
}

bool is_info_log_enabled() {

#ifdef _LOG_INFO
   return true;
#endif

#ifdef _LOG_DEBUG
   return true;
#endif

   return false;
}

bool is_debug_log_enabled() {

#ifdef _LOG_DEBUG
   return true;
#endif

   return false;
}

void log_error(const char *format, ...) {
   if (is_error_log_enabled()) {
      va_list args;
      va_start(args, format);
      printf(LOG_START_ERROR);
      vprintf(format, args);
      printf("\n");
      va_end(args);
   }
}

void log_debug(const char *format, ...) {
   if (is_debug_log_enabled()) {
      va_list args;
      va_start(args, format);
      printf(LOG_START_DEBUG);
      vprintf(format, args);
      printf("\n");
      va_end(args);
   }
}

void log_info(const char *format, ...) {
   if (is_info_log_enabled()) {
      va_list args;
      va_start(args, format);
      printf(LOG_START_INFO);
      vprintf(format, args);
      printf("\n");
      va_end(args);
   }
}
