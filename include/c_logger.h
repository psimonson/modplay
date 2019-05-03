/*
 * c_logger.h - My C logger header library.
 *
 * Author : Philip R. Simonson
 * Date   : 03/13/2019
 ******************************************************************************
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#ifndef MAX_PATH
#define MAX_PATH 260
#endif

#define MAX_LOGS 5

#define ERRMSG "[ERROR]: "
#define DBGMSG "[DEBUG]: "
#define WRNMSG "[DEBUG]: "

#pragma pack(push, 1)
struct CLOG {
	char name[MAX_PATH];
	FILE *log_file;
	int status;
	long read_pos;
	long write_pos;
};
#pragma pack(pop)

enum CLOG_ENUM {
	CLOG0,
	CLOG1,
	CLOG2,
	CLOG3,
	CLOG4
};

struct CLOG log_files[MAX_LOGS];
char init_var;

static void close_log(int);
static int get_status(int);

static void
_logger_exit_func ()
{
	int i;

	for(i=0; i<MAX_LOGS; i++)
		if (get_status(i))
			close_log(i);
}

static void
init_logger ()
{
	if (!init_var) {
		int i;

		init_var = 1;
		for(i=0; i<MAX_LOGS; i++)
			memset(&log_files[i], 0, sizeof(struct CLOG));
		atexit(_logger_exit_func);
	} else {
		printf("C_Logger is already initialized!\n");
	}
}

static void
open_log (int logNum, const char *name)
{
	if (init_var) {
		if (!get_status(logNum)) {
			strncpy(log_files[logNum].name, name, MAX_PATH);
			if ((log_files[logNum].log_file = fopen(log_files[logNum].name, "a+t")) == NULL) {
				fprintf(stderr, "Error: Cannot open %s for writing.\n",
					log_files[logNum].name);
				return;
			}
			log_files[logNum].status = 1;
			return;
		}
		return;
	}
	printf("Please use init_logger() first.\n");
}

static int
read_log (int logNum, char *buf, int size)
{
	if (init_var) {
		if (get_status(logNum)) {
			int c, pos;

			if (fseek(log_files[logNum].log_file, log_files[logNum].read_pos, SEEK_SET) < 0) {
				printf("Error: seeking through log file.\n");
				return -1;
			}

			pos = 0;
			memset(buf, 0, size);
			while(pos < size && ((c = fgetc(log_files[logNum].log_file)) != '\n'))
				buf[pos++] = c;
			log_files[logNum].read_pos = ftell(log_files[logNum].log_file);
			buf[pos] = '\0';
			return c;
		}
		printf("Warning: Could not read, log CLOG%d not open.\n", logNum);
		return -1;
	}
	printf("Please use init_logger() first.\n");
	return -1;
}

static void
write_log (int logNum, const char *data, ...)
{
	if (init_var) {
		if (get_status(logNum)) {
			va_list ap;

			fseek(log_files[logNum].log_file, log_files[logNum].write_pos, SEEK_SET);
			va_start(ap, data);
			vfprintf(log_files[logNum].log_file, data, ap);
			va_end(ap);
			log_files[logNum].write_pos = ftell(log_files[logNum].log_file);
			fflush(log_files[logNum].log_file);
			return;
		}
		printf("Warning: Not writing, log CLOG%d not open.\n", logNum);
		return;
	}
	printf("Please use init_logger() first.\n");
}

static void
close_log (int logNum)
{
	if (init_var) {
		if (get_status(logNum)) {
			fclose(log_files[logNum].log_file);
			strncpy(log_files[logNum].name, "", 1);
			log_files[logNum].status = 0;
			log_files[logNum].read_pos = 0;
			log_files[logNum].write_pos = 0;
			return;
		}
		return;
	}
	printf("Please use init_logger() first.\n");
}

static void
print_status (int logNum)
{
	if (init_var) {
		printf("CLOG%d: Log %s\n", logNum, (get_status(logNum) == 1) ? "open" : "closed");
		return;
	}
	printf("Please use init_logger() first.\n");
}

static int
get_status (int logNum)
{
	if (init_var)
		return log_files[logNum].status;
	printf("Please use init_logger() first.\n");
	return 0;
}

static char*
get_name (int logNum)
{
	if (init_var)
		return log_files[logNum].name;
	printf("Please use init_logger() first.\n");
	return 0;
}
