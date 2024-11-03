#pragma once

typedef struct
{
    int (*start)(const char *log_path);
    void (*stop)();

    void (*debug)(const char *fmt, ...);
    void (*info)(const char *fmt, ...);
    void (*warn)(const char *fmt, ...);
    void (*error)(const char *fmt, ...);
    void (*fatal)(const char *fmt, ...);
} Log;

Log log_get();
