#include "stream.h"

int ReadN(int fd, char *buf, int nbytes)
{
    int n, nr;

    for (n = 0; n < nbytes; n += nr)
    {
        if ((nr = read(fd, buf + n, nbytes - n)) <= 0)
            return (nr); /* error in reading */
    }
    return (n);
}

int WriteN(int sd, char *buf, int bufsize)
{
    int n, nw;

    for (n = 0; n < bufsize; n += nw)
    {
        if ((nw = write(sd, buf + n, bufsize - n)) <= 0)
            return (nw); /* write error */
    }
    return n;
}

int WritePath(int sd, char opcode, int filepath_len, char *buf)
{
    int n, nw;

    // send opcode
    write(sd, (char *)&opcode, 1);

    // send 2-bytes int - length of filename
    short data_size = filepath_len; /* short must be two bytes long */
    data_size = htons(data_size);   /* convert to network byte order */
    write(sd, &data_size, 2);

    // send filename
    for (n = 0; n < filepath_len; n += nw)
    {
        if ((nw = write(sd, buf + n, filepath_len - n)) <= 0)
            return (nw); /* write error */
    }

    return n;
}