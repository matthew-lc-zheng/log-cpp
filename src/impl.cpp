#include "impl.h"
#include <chrono>
#include <csignal>
#include <ctime>
#include <mutex>
#include <stdarg.h>
#include <stdio.h>

const uint32_t g_max_log_buffer_length = 131072; // 1024 * 128 (128 kb)
const uint16_t g_max_line_buffer_length = 550;
const uint16_t g_max_content_length = 512;

static LogImpl *singleton = nullptr;
static std::mutex singleton_mtx;

struct Timestamp
{
    uint16_t year;
    uint16_t month;
    uint16_t day;

    uint16_t hour;
    uint16_t minute;
    uint16_t second;
    uint16_t milisecond;
};

static Timestamp get_time()
{
    auto now = std::chrono::system_clock::now();
    std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm local_time;
    localtime_s(&local_time, &now_time_t);

    auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    uint16_t year = local_time.tm_year + 1900;
    uint16_t month = local_time.tm_mon + 1;
    uint16_t day = local_time.tm_mday;
    uint16_t hour = local_time.tm_hour;
    uint16_t minute = local_time.tm_min;
    uint16_t second = local_time.tm_sec;
    uint16_t ms = now_ms.count();

    Timestamp ts = {year, month, day, hour, minute, second, ms};
    return ts;
}

static void handle_signal(int sig)
{
    LogImpl::get()->stop();
    signal(sig, SIG_DFL);
    raise(sig);
}

static void setup_signal_handler()
{
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);
    signal(SIGSEGV, handle_signal);
    signal(SIGABRT, handle_signal);
}

LogImpl *LogImpl::get()
{
    if (!singleton)
    {
        std::lock_guard<std::mutex> lock(singleton_mtx);
        if (!singleton)
        {
            singleton = new LogImpl();
        }
    }
    return singleton;
}

int LogImpl::start(const char *path, uint16_t level)
{
    _level = level;
    setup_signal_handler();
    return _printer.start(path);
}

void LogImpl::log(const char *level_prefix, uint16_t level, const char *fmt, va_list args)
{
    if (level < _level)
    {
        return;
    }
    auto ts = get_time();

    char content[g_max_content_length] = {};
    vsprintf_s(content, fmt, args);

    char line_buffer[g_max_line_buffer_length] = {};
    sprintf_s(line_buffer,
              "%04d-%02d-%02d %02d:%02d:%02d.%03d [%s]: %s\n",
              ts.year,
              ts.month,
              ts.day,
              ts.hour,
              ts.minute,
              ts.second,
              ts.milisecond,
              level_prefix,
              content);
    _printer.add_task(line_buffer);
}

int Printer::start(const char *path)
{
    do
    {
        if (_file)
        {
            break;
        }
        auto ret = fopen_s(&_file, path, "a");
        if (ret)
        {
            printf_s("Opening %s failed, errno: %d\n", path, ret);
            return -1;
        }
    } while (false);
    _run_flag = true;
    _runner = std::thread(&Printer::run, this);
    return 0;
}

void Printer::stop()
{
    if (_run_flag) {
        _run_flag = false;
        flush();
    }
    if (_runner.joinable()) {
        _runner.join();
    }
}

void Printer::add_task(char line[])
{
    std::lock_guard<std::mutex> lock(_add_mtx);
    if (strlen(_buffer[_working_buffer_id]) + strlen(line) >= g_max_log_buffer_length) {
        flush();
    }
    strcat(_buffer[_working_buffer_id], line);
}

void Printer::run()
{
    create_buffer();
    while (_run_flag)
    {
        std::unique_lock<std::mutex> lock(_mtx);
        while (!_print_flag)
        {
            _cv.wait(lock);
        }
        _print_flag = false;
        auto printing_buffer_id = _working_buffer_id ^ 1;
        fwrite(_buffer[printing_buffer_id], 1, strlen(_buffer[printing_buffer_id]), _file);
        _buffer[printing_buffer_id][0] = '\0';
    }
    destroy_buffer();
}

void Printer::flush()
{
    std::unique_lock<std::mutex> lock(_mtx);
    _working_buffer_id ^= 1;
    _print_flag = true;
    _cv.notify_all();
}

int Printer::create_buffer()
{
    if (_buffer)
    {
        destroy_buffer();
    }
    _buffer = (char **)malloc(2 * sizeof(char *));
    if (!_buffer)
    {
        return -1;
    }
    for (int i = 0; i < 2; ++i)
    {
        _buffer[i] = (char *) malloc(g_max_log_buffer_length);
        if (!_buffer[i])
        {
            return -1;
        }
    }
    _buffer[0][0] = '\0';
    _buffer[1][0] = '\0';
    return 0;
}

void Printer::destroy_buffer()
{
    if (!_buffer) {
        return;
    }
    for (int i = 0; i < 2; ++i) {
        if (!_buffer[i]) {
            continue;
        }
        free(_buffer[i]);
        _buffer[i] = nullptr;
    }
    free(_buffer);
    _buffer = nullptr;
}
