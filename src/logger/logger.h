/**
 * @file logger.h
 * @brief Helps manage logging program activity into a file
 * @version 1.0
 * @date 2022-07-25
 * 
 * 
 */

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <fcntl.h> 
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#define logfile "sftp_log.txt"


void InitializeLogger();

void Logger(char* e);