#include <stdio.h>
#include <stdarg.h>
#include <rc/pthread.h>
#include "sys_log.h"

static volatile LOG_LEVEL log_level=31;
FILE *sys_log;
pthread_mutex_t lock;

int set_log_level(LOG_LEVEL level)
{
    log_level = level;

    return 0;
}

int init_file_log()
{
    // add flag for enable and disable logging?
    sys_log = fopen("sys.log","w");

    if(sys_log == NULL)
    {
        LOG_E("Could not open an SYS log file!\n");
        return -1;
    }

    return 0;
}

int LOG(LOG_LEVEL level, const char *fmt, ...)
{
    if (level & log_level)
    {
        va_list args;
        va_start(args, fmt);

        pthread_mutex_lock(&lock);
        printf(fmt, args);

        if (sys_log != NULL)
        {
            fprintf(sys_log, fmt, args);
        }
        pthread_mutex_unlock(&lock);

        va_end(args);
    }

    return 0;
}


