#ifndef CORE_H
#define CORE_H

#include <intdef.h>
#include <stdio.h>

#define SDEWM_VERSION "0.1.0"

bool core__init_log_stream(void);
void core__close_log_stream(void);

extern FILE* logstream;

#ifdef DEBUG
    #define logf(fmt, ...) fprintf(logstream, "[%s:%d] " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__); fprintf(stderr, "[%s:%d] " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#else
    #define logf(fmt, ...)
#endif


#endif