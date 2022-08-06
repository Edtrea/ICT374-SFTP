/**
 * @file server.c
 * @brief The main program of SFTP server. Responsible for creating connection to client and creating daemon process to serve each connected client
 * @version 1.0
 * @date 2022-07-25
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <signal.h>
#include "logger/logger.h"
#include "serverFunction/sFunctionHelper.h"

//#define BUFSIZE 512
#define SERV_TCP_PORT 40263 // given default port number

// /* opcodes */
// #define PUT      'A'
// #define GET      'G'
// #define RESPOND  'B'
// #define PWD      'E'
// #define DIR_OPC  'F'
// #define CD       'D'

void serve_a_client(int sd)
{
	char opcode;

	// read opcode from client first
	while (read(sd, (char *)&opcode, 1) > 0)
	{
		if (opcode == PUT)
		{
			Logger("Opcode for put received\n");
			put_func(sd);
		}
		else if (opcode == GET)
		{
			Logger("Opcode for get received\n");
			get_func(sd);
		}
		else if (opcode == PWD)
		{
			Logger("Opcode for pwd received\n");
			pwd_func(sd);
		}
		else if (opcode == DIR_OPC)
		{
			Logger("Opcode for dir received\n");
			dir_func(sd);
		}
		else if (opcode == CD)
		{
			Logger("Opcode for cd received\n");
			cd_func(sd);
		}
		else
		{
			Logger("Invalid opcode received\n");
		}
	} // end while

	return;
}

void claim_children()
{
	pid_t pid = 1;
	while (pid > 0)
	{ /* claim as many zombies as we can */
		pid = waitpid(0, (int *)0, WNOHANG);
	}
}

void daemon_init(void)
{
	pid_t pid;
	struct sigaction act;

	if ((pid = fork()) < 0)
	{
		perror("fork");
		exit(1);
	}
	else if (pid > 0)
	{
		/* parent */
		printf("Daemon pid: %d\n", pid);
		exit(0);
	}
	else
	{
		/* child */
		setsid(); /* become session leader */
		umask(0); /* clear file mode creation mask */

		/* catch SIGCHLD to remove zombies from system */
		act.sa_handler = claim_children; /* use reliable signal */
		sigemptyset(&act.sa_mask);		 /* not to block other signals */
		act.sa_flags = SA_NOCLDSTOP;	 /* not catch stopped children */
		sigaction(SIGCHLD, (struct sigaction *)&act, (struct sigaction *)0);
	}
}

int main(int argc, char *argv[])
{
	int nsd, sd, n;
	pid_t pid;
	unsigned short port; /* server listening port */
	socklen_t cli_addrlen;
	struct sockaddr_in ser_addr;
	struct sockaddr_in cli_addr;

	// remove log file
	remove(logfile);

	// get port number
	if (argc == 1)
	{
		port = SERV_TCP_PORT;
	}
	else if (argc == 2)
	{
		// use user given port
		n = atoi(argv[1]);
		if (n >= 1024 && n < 65536)
		{
			port = n;
		}
		else
		{
			printf("Error: port number must be between 1024 and 65535\n");
			exit(1);
		}
	}
	else
	{
		printf("Usage: %s [ server listening port ]\n", argv[0]);
		exit(1);
	}

	// set to current directory
	char dir[256] = ".";
	if (chdir(dir) == -1)
	{
		printf("Failed to set initial directory to: %s\n", dir);
		exit(1);
	}

	/* make the server a daemon. */
	daemon_init();

	/* set up listening socket sd */
	if ((sd = socket(PF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("server: socket");
		exit(1);
	}

	/* build server Internet socket address */
	bzero((char *)&ser_addr, sizeof(ser_addr));
	ser_addr.sin_family = AF_INET;				  // address family
	ser_addr.sin_port = htons(port);			  // network ordered port number
	ser_addr.sin_addr.s_addr = htonl(INADDR_ANY); // any interface

	/* bind server address to socket sd */
	if (bind(sd, (struct sockaddr *)&ser_addr, sizeof(ser_addr)) < 0)
	{
		perror("server bind");
		exit(1);
	}

	/* become a listening socket */
	listen(sd, 5);

	while (1)
	{
		/* wait to accept a client request for connection */
		cli_addrlen = sizeof(cli_addr);
		nsd = accept(sd, (struct sockaddr *)&cli_addr, &cli_addrlen);
		if (nsd < 0)
		{
			if (errno == EINTR) /* if interrupted by SIGCHLD */
				continue;
			perror("server: accpet");
			exit(1);
		}

		/* create a child process to handle this client */
		if ((pid = fork()) < 0)
		{
			perror("fork");
			exit(1);
		}
		else if (pid > 0)
		{
			close(nsd);
			continue; /* parent to wait for next client */
		}
		else
		{
			/* now in child, serve the current client */
			close(sd);
			InitializeLogger();
			serve_a_client(nsd);
			exit(0);
		}
	}
}