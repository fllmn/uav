#ifndef SYS_LOG_H
#define SYS_LOG_H

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
int LOG_I(const char *fmt, ...);
int LOG_W(const char *fmt, ...);
int LOG_E(const char *fmt, ...);
int LOG_C(const char *fmt, ...);

#endif //SYS_LOG_H
