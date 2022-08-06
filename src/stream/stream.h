/**
 * @file stream.h
 * @brief Holds functions that help to send or recieve data between client and server
 * @version 1.0
 * @date 2022-07-25
 * 
 * 
 */

#include  <unistd.h>
#include  <sys/types.h>
#include  <netinet/in.h> /* struct sockaddr_in, htons(), htonl(), */

#define MAX_BLOCK_SIZE 512
//#define MAX_BLOCK_SIZE (1024*5)    /* maximum size of any piece of */
                                   /* data that can be sent by client */

/*
 * purpose:  read a stream of bytes from "fd" to "buf".
 * pre:      1) size of buf bufsize >= MAX_BLOCK_SIZE,
 * post:     1) buf contains the byte stream; 
 *           2) return value > 0   : number ofbytes read
 *                           = 0   : connection closed
 *                           = -1  : read error
 *                           = -2  : protocol error
 *                           = -3  : buffer too small
 */   
/**
 * @brief read a stream of bytes from "fd" to "buf".
 * 
 * @param fd socket descriptor
 * @param buf buffer to hold data
 * @param bufsize size of buffer
 * @return int The number of bytes read
 */
int ReadN(int fd, char *buf, int bufsize);



/*
 * purpose:  write "nbytes" bytes from "buf" to "fd".
 * pre:      1) nbytes <= MAX_BLOCK_SIZE,
 * post:     1) nbytes bytes from buf written to fd;
 *           2) return value = nbytes : number ofbytes written
 *                           = -3     : too many bytes to send 
 *                           otherwise: write error
 */   

/**
 * @brief write "nbytes" bytes from "buf" to "fd".
 * 
 * @param fd socket descriptor
 * @param buf buffer of data to send
 * @param nbytes number of bytes in buf
 * @return int number of bytes written
 */
int WriteN(int fd, char *buf, int nbytes);

/**
 * @brief Writes opcode, filename's length and filename to sd
 * 
 * @param sd socket descriptor
 * @param opcode the opcode
 * @param filename_len the length of the file name
 * @param buf the filename
 * @return int  bytes of filename written
 */
int WritePath(int sd, char opcode, int filename_len, char *buf);