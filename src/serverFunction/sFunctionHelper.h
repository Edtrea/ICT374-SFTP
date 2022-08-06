/**
 * @file sFunctionHelper.h
 * @brief Holds functions related to server's SFTP functions such as put and get files
 * @version 1.0
 * @date 2022-07-25
 * 
 * 
 */

#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include "../stream/stream.h"

#define BUFSIZE 512

/* opcodes */
#define PUT      'A'
#define GET      'G'
#define RESPOND  'B'
#define PWD      'E'
#define DIR_OPC  'F'
#define CD       'D'

/**
 * @brief Respond to client's put request by receiving client's file and storing it
 * 
 * @param sd Socket descriptor
 */
void put_func(int sd);

/**
 * @brief Respond to client's get request by retriving file and sending it to client
 * 
 * @param sd Socket descriptor
 */
void get_func(int sd);

/**
 * @brief Respond to client's pwd request by sending client the server's current directory path
 * 
 * @param sd Socket descriptor
 */
void pwd_func(int sd);

/**
 * @brief Respond to client's sd request by sending client the server's list of file in current directory
 * 
 * @param sd Socket descriptor
 */
void dir_func(int sd);

/**
 * @brief Respond to client's cd request by changing the current directory to what client defined
 * 
 * @param sd Socket descriptor
 */
void cd_func(int sd);