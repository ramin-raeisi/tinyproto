#include "tiny_debug.h"
#include <stdio.h>
#include <inttypes.h>
#include <stdarg.h>


#if defined(TINY_DEBUG) && defined(TINY_FILE_LOGGING)

static FILE *log_file = NULL;

static FILE *get_log_file(uintptr_t id)
{
    if (log_file == NULL) {
        char filename[64];
        snprintf(filename, sizeof(filename), "tiny_fd_%08" PRIxPTR ".csv", id);
        log_file = fopen(filename, "w");
        if (log_file == NULL) {
            fprintf(stderr, "Failed to open log file\n");
            return NULL;
        }
        // Num# - is frame number, Exp# - is expected frame number from remote peer
        fprintf(log_file, " time ms, DIR, FR, Type, Num#, Exp#\n");
    }
    return log_file;
}

void tiny_file_log(uintptr_t id, const char *fmt, ...)
{
    FILE *file = get_log_file(id);
    va_list args;
    va_start(args, fmt);
    vfprintf(file, fmt, args);
    va_end(args);
    fflush(file);
}

#endif