#pragma once
#include <condition_variable>
#include <cstdint>
#include <thread>

class Printer
{
public:
    Printer()
        : _file(nullptr)
        , _run_flag(false)
        , _print_flag(false)
        , _working_buffer_id(0)
        , _buffer(nullptr)
    {
    }
    ~Printer()
    {
        destroy_buffer();
        if (_file)
        {
            fclose(_file);
            _file = nullptr;
        }
    }

    int start(const char *);
    void stop();
    void add_task(char[]);

private:
    void run();
    void flush();

    int create_buffer();
    void destroy_buffer();

    char **_buffer;
    uint8_t _working_buffer_id;
    FILE *_file;
    std::thread _runner;
    bool _run_flag;
    bool _print_flag;
    std::mutex _add_mtx;
    std::mutex _mtx;
    std::condition_variable _cv;
};

class LogImpl
{
public:
    static LogImpl *get();
    ~LogImpl() { stop(); }

    int start(const char *path, uint16_t level = 1);
    void stop() { _printer.stop(); }

    void debug(const char *fmt, va_list args) { log("debug", 0, fmt, args); }
    void info(const char *fmt, va_list args) { log("info", 1, fmt, args); }
    void warn(const char *fmt, va_list args) { log("warn", 2, fmt, args); }
    void error(const char *fmt, va_list args) { log("error", 3, fmt, args); }
    void fatal(const char *fmt, va_list args) { log("fatal", 4, fmt, args); }

private:
    LogImpl() = default;
    void log(const char *, uint16_t, const char *, va_list);

    Printer _printer;
    uint16_t _level = 0;
};
