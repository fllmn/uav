#include <stdio.h>
#include <stdarg.h>
#include <rc/pthread.h>
#include "sys_log.h"

static volatile LOG_LEVEL log_level=31;
static FILE *sys_log_fp;
pthread_mutex_t lock;

static char *level_name[] = {"INFO","WARN", "ERROR", "CRITICAL"};

int set_log_level(LOG_LEVEL level)
{
    log_level = level;

    return 0;
}

int init_file_log()
{
    // add flag for enable and disable logging?
    sys_log_fp = fopen("sys.log","w");

    if(sys_log_fp == NULL)
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
	char *name;
        pthread_mutex_lock(&lock);
	switch (level)
	{
		case LOG_OFF:
		case LOG_INFO:
			name = level_name[0];
			break;
		case LOG_WARN:
			name = level_name[1];
			break;
		case LOG_ERROR:
			name = level_name[2];
			break;
		case LOG_CRITICAL:
			name = level_name[3];
			break;
	}
	printf("%s: ", name);
        vprintf(fmt, args);
	printf("\n");

        if (sys_log_fp != NULL)
        {
		fprintf(sys_log_fp, "%s: ", name);
            vfprintf(sys_log_fp, fmt, args);
		fprintf(sys_log_fp, "\n");
        }
        pthread_mutex_unlock(&lock);

        va_end(args);
    }

    return 0;
}


