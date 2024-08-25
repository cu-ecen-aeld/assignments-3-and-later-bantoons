#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>

int main(int argc, char *argv[])
{
    //Open log deamon
    openlog(NULL, 0, LOG_USER);
    //Log information when program start
    syslog (LOG_INFO, "Writer had start.");

    //Check if input arguments are correct
    if (argc != 3)
    {
        syslog(LOG_ERR, "Wrong number of arguments\n");
        closelog();
        exit(1);
    }

    //Open file for write.
    FILE *f;
    f = fopen(argv[1], "w");
    //checked if file already opened
    if (f == NULL)
    {
        syslog(LOG_ERR, "ERROR: Can not open the file\n");
        closelog();
        exit(1);
    }


    //write input message
    fputs(argv[2], f);
    //Report with syslog how its write.
    syslog(LOG_DEBUG, "Writing %s to %s\n", argv[2], argv[1]);
    closelog();
    fclose(f);

    return 0;
}
