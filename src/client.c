/**
 * @file client.c
 * @brief The main program of SFTP client. Responsible for connecting to server and serving the user by calling other functions
 * @version 1.0
 * @date 2022-07-25
 *
 * @version 1.1
 * @date 2022-07-27
 * Added checks for cd, put & get for path
 * Token[] will be initialized as NULL
 * Token[1] will be reset to NULL at end of each while
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <dirent.h>
#include <fcntl.h>
#include <netdb.h>

#include "stream/stream.h"
#include "clientFunction/token.h"
#include "clientFunction/cFunctionHelper.h"

int main(int argc, char *argv[])
{
     int sd, nr, n;
     char buf[128], host[60];
     unsigned short port;
     struct sockaddr_in ser_addr;
     struct hostent *hp;
     char *token[2] = {NULL};

     /* get server host name and port number */
     if (argc == 1)
     {
          /* assume server running on the local host and on default port */
          strcpy(host, "localhost");
          port = SERV_TCP_PORT;
     }
     else if (argc == 2)
     { /* use the given host name */
          strcpy(host, argv[1]);
          port = SERV_TCP_PORT;
     }
     else if (argc == 3)
     { /* use given host and port for server */
          strcpy(host, argv[1]);
          n = atoi(argv[2]);

          if (n >= 1024 && n < 65536)
          {
               port = n;
          }
          else
          {
               printf("Error: server port number must be between 1024 and 65535\n");
               exit(1);
          }
     }
     else
     {
          printf("Usage: %s [ <server host name> [ <server listening port> ] ]\n", argv[0]);
          exit(1);
     }

     /* get host address, & build a server socket address */
     bzero((char *)&ser_addr, sizeof(ser_addr));
     ser_addr.sin_family = AF_INET;
     ser_addr.sin_port = htons(port);
     if ((hp = gethostbyname(host)) == NULL)
     {
          printf("host %s not found\n", host);
          exit(1);
     }
     ser_addr.sin_addr.s_addr = *(u_long *)hp->h_addr;

     /* create TCP socket & connect socket to server address */
     sd = socket(PF_INET, SOCK_STREAM, 0);
     if (connect(sd, (struct sockaddr *)&ser_addr, sizeof(ser_addr)) < 0)
     {
          perror("client connect");
          exit(1);
     }

     while (1)
     {
          printf("> "); /* display prompt */

          /* read user input and tokenise */
          fgets(buf, sizeof(buf), stdin);
          nr = strlen(buf);
          // replace last char
          if (buf[nr - 1] == '\n')
          {
               buf[nr - 1] = '\0';
               nr--;
          }

          // check if quit
          if (strcmp(buf, "quit") == 0)
          {
               printf("Bye from the client\n");
               exit(0);
          }

          // split command and filename
          tokenise(buf, token);

          printf("command: %s\n", token[0]);         

          // execute corresponding function
          if (strcmp(token[0], "put") == 0)
          {
               if(token[1] == NULL)
               {
                    printf("Error no path given\n");
               }
               else
               {
                    put(sd, token[1]);
               }
          }
          else if (strcmp(token[0], "get") == 0)
          {
               if(token[1] == NULL)
               {
                    printf("Error no path given\n");
               }
               else
               {
                    get(sd, token[1]);
               }
          }
          else if (strcmp(token[0], "pwd") == 0)
          {
               pwd(sd);
          }
          else if (strcmp(token[0], "lpwd") == 0)
          {
               lpwd(sd);
          }
          else if (strcmp(token[0], "ldir") == 0)
          {
               ldir(token[1]);
          }
          else if (strcmp(token[0], "dir") == 0)
          {
               dir(sd);
          }
          else if (strcmp(token[0], "cd") == 0)
          {
               if(token[1] == NULL)
               {
                    printf("Error no path given\n");
               }
               else
               {
                    cd(sd, token[1]);
               }
          }
          else if (strcmp(token[0], "lcd") == 0)
          {
               lcd(token[1]);
          }
          else if (strcmp(buf, "help") == 0)
          {
               DisplayMenu();
          }
          else
          {
               printf("Invalid command\n");
          }

          token[1] = NULL;
     }
}