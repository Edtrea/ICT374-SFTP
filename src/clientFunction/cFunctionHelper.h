/**
 * @file cFunctionHelper.h
 * @brief Holds client's SFTP related functions such as get and put files.
 * @version 1.0
 * @date 2022-07-25
 * 
 * 
 */

#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>

#include "../stream/stream.h"


#define BUFSIZE 512 
#define SERV_TCP_PORT   40263  // given default port number

/* opcodes */
#define PUT			'A'
#define RESPOND	    'B'
#define GET      	'G'
#define PWD      	'E'
#define DIR_OPC  	'F'
#define CD       	'D'


/**
 * @brief prints out SFTP command menu in terminal
 * 
 */
void DisplayMenu();

/**
 * @brief Get and display the current directory of server
 * 
 * @param sd    socket descriptor
 */
void pwd(int sd);

/**
 * @brief Display the current local directory
 * 
 * @param sd    socket descriptor
 */
void lpwd(int sd);

/**
 * @brief Get and display directory listing of server
 * 
 * @param sd    socket descriptor
 */
void dir(int sd);

/**
 * @brief Display local directory listing
 * 
 * @param token directory path input
 */
void ldir(char* token);

/**
 * @brief Change current directory of server
 * 
 * @param sd    socket descriptor
 * @param token directory path input
 */
void cd(int sd, char* token);

/**
 * @brief Change current local directory
 * 
 * @param token directory path input
 */
void lcd(char* token);

/**
 * @brief Send a file to server by filename
 * 
 * @param sd    socket descriptor
 * @param filename  filename of file to send to server
 */
void put(int sd, char* filename);

/**
 * @brief Retrieve a file from server by filename
 * 
 * @param sd    socket descriptor
 * @param filename  filename of file to be retrieved from server
 */
void get(int sd, char* filename);