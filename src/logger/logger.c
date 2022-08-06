#include "logger.h"

void InitializeLogger()
{
    FILE *fd;

    fd = fopen(logfile, "a");
    if (fd == NULL)
    {
        perror("Error writing to log file");
        exit(0);
    }
    chmod(logfile, 0766);

    fclose(fd);
    return;
}

void Logger(char *e)
{
    time_t rawtime;
    struct tm *timeinfo;
    FILE *fd;
    char *output;

    fd = fopen(logfile, "a");

    if (fd == NULL)
    {
        perror("Error writing to log file");
        exit(0);
    }

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    fprintf(fd, "%s  %s", asctime(timeinfo), e);

    fclose(fd);

    return;
}