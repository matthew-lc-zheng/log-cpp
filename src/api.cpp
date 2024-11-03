#include "impl.h"
#include <log.h>
#include <stdarg.h>

static int log_start(const char *log_path)
{
    return LogImpl::get()->start(log_path);
}
static void log_stop()
{
    LogImpl::get()->stop();
}
static void log_error(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    LogImpl::get()->error(fmt, args);
    va_end(args);
}
static void log_debug(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    LogImpl::get()->debug(fmt, args);
    va_end(args);
}
static void log_info(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    LogImpl::get()->info(fmt, args);
    va_end(args);
}
static void log_warn(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    LogImpl::get()->warn(fmt, args);
    va_end(args);
}
static void log_fatal(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    LogImpl::get()->fatal(fmt, args);
    va_end(args);
}

Log log_get()
{
    Log log;
    log.start = log_start;
    log.stop = log_stop;

    log.debug = log_debug;
    log.info = log_info;
    log.warn = log_warn;
    log.error = log_error;
    log.fatal = log_fatal;

    return log;
}
