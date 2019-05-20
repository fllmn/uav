#ifndef SYS_LOG_H
#define SYS_LOG_H

#define LOG_I(...) LOG((LOG_INFO), __VA_ARGS__)
#define LOG_W(...) LOG((LOG_WARN), __VA_ARGS__)
#define LOG_E(...) LOG((LOG_ERROR), __VA_ARGS__)
#define LOG_C(...) LOG((LOG_CRITICAL), __VA_ARGS__)

typedef enum LOG_LEVEL
{
    LOG_OFF = 0,
    LOG_CRITICAL = 1,
    LOG_ERROR = 2,
    LOG_WARN = 4,
    LOG_INFO = 8
} LOG_LEVEL;

int init_file_log(void);
int set_log_level(LOG_LEVEL level);

int LOG(LOG_LEVEL level,const char *fmt, ...);

#endif //SYS_LOG_H
