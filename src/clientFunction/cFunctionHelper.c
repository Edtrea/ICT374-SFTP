#include "cFunctionHelper.h"

void DisplayMenu()
{
	printf("---Command List---\n");

	printf("put <filename>    - Sends file to server\n");
	printf("get <filename>    - Retrieve file from server\n");

	printf("pwd     - Display current directory path on server\n");
	printf("lpwd    - Display current local directory path\n");

	printf("dir    - Display current directory listing on server\n");
	printf("ldir    - Display current local directory listing\n");

	printf("cd <path>    - Change current directory on server\n");
	printf("lcd <path>    - Change current local directory\n");

	printf("help    - Display functions menu\n");

	printf("quit    - quit program\n");

	printf("---End List---\n");
	return;
}

void pwd(int sd)
{
	int msg_len;
	char opcode;

	// sent opcode to server
	char code = (char)PWD;
	if (write(sd, (char *)&code, 1) != 1)
	{
		printf("Error in sending opcode - pwd\n");
		return;
	}

	// read server's respond
	if (read(sd, (char *)&opcode, 1) != 1)
	{
		printf("Error reading opcode - pwd\n");
		return;
	}

	if (opcode == PWD)
	{
		// read length of message
		short data_size;
		if (read(sd, (char *)&data_size, 2) != 2)
		{
			printf("Error in reading length of message - pwd\n");
			return;
		}
		msg_len = (int)ntohs(data_size); // convert to host byte order
	}

	char message[msg_len + 1];

	// read message content
	if (ReadN(sd, message, msg_len) < 0)
	{
		printf("Error in reading directory - pwd\n");
		return;
	}

	message[msg_len] = '\0';
	printf("%s\n", message);
	return;
}

void lpwd(int sd)
{
	char current_directory[BUFSIZE];
	if (getcwd(current_directory, sizeof(current_directory)) == NULL) // get pathname
	{
		perror("Failed to get current directory\n");
		return;
	}
	else
	{
		printf("%s\n", current_directory);
		return;
	}
}

void dir(int sd)
{
	char opcode;
	int msg_len;

	// send 1 byte opcode to server to inform the operation
	char code = (char)DIR_OPC;
	if (write(sd, (char *)&code, 1) != 1)
	{
		printf("Error in sending opcode - dir\n");
		return;
	}

	// read server's respond
	if (read(sd, (char *)&opcode, 1) != 1)
	{
		printf("Error reading opcode - dir\n");
		return;
	}

	if (opcode == DIR_OPC)
	{
		// read length of message
		int data;
		if (read(sd, (char *)&data, 4) != 4)
		{
			printf("Error in reading length of message - pwd\n");
			return;
		}
		msg_len = (int)ntohl(data); // convert to host byte order
	}

	char message[msg_len + 1];
	// read message content
	if (ReadN(sd, message, msg_len) < 0)
	{
		printf("Error in reading directory - dir\n");
		return;
	}

	message[msg_len] = '\0';
	printf("%s\n", message);
	return;
}

void ldir(char *token)
{
	DIR *dp;
	struct dirent *direntp;

	if (token == NULL)
	{
		token = "."; // current directory
	}

	// open the client's current dicectory
	if ((dp = opendir(token)) == NULL)
	{
		perror("Error in opening client's current directory\n");
		return;
	}

	// read and print file names
	while ((direntp = readdir(dp)) != NULL)
	{
		printf("%s\n", direntp->d_name);
	}
	closedir(dp);
	return;
}

void cd(int sd, char *token)
{
	char opcode;
	char acknowlegment_code;
	int n, nw;

	// sent required message to server
	if (nw = WritePath(sd, CD, strlen(token), token) <= 0)
	{
		perror("Error in sending message to server - cd\n");
		return;
	}

	// read server's respond
	if (read(sd, (char *)&opcode, 1) != 1)
	{
		perror("Error reading opcode - cd\n");
		return;
	}

	if (opcode == CD)
	{
		// read acklodgment code
		if (read(sd, (char *)&acknowlegment_code, 1) != 1)
		{
			perror("Error reading ackowledgment code - cd\n");
			return;
		}
	}

	// if get 1 - means error
	// else no issue
	if (acknowlegment_code == '1')
	{
		perror("Server failed to change the working directory\n");
		return;
	}
	else if (acknowlegment_code == '0')
	{
		printf("Server directory changed\n");
		return;
	}
	else
	{
		perror("Unknown acknowledgement code\n");
		return;
	}
}

void lcd(char *token)
{
	if (chdir(token) == -1)
	{
		perror("Failed to change directory\n");
		return;
	}
	return;
}

void put(int sd, char *filename)
{
	int fd, nw, n, nr;
	struct stat fileInfo; // obtain file size
	int filesize;
	int filename_len = strlen(filename);
	char opcode;
	char acknowledgment_code = '0';

	// open requested file
	if ((fd = open(filename, O_RDONLY)) == -1)
	{
		perror("Error in opening the file: \n");
		return;
	}

	/* reset file pointer */
	lseek(fd, 0, SEEK_SET);

	// section a
	// client sent message to server
	if (nw = WritePath(sd, PUT, filename_len, filename) <= 0)
	{
		perror("Error in sending message to server - put\n");
		return;
	}

	// now read server's respond to see if filename has any error
	// read opcode
	if (read(sd, (char *)&opcode, 1) != 1)
	{
		perror("Error reading opcode - put\n");
		return;
	}

	// check opcode
	if (opcode != PUT)
	{
		printf("Unexpected opcode received from server - put\n");
		exit(1);
	}

	// read acknowledgment code
	if (read(sd, (char *)&acknowledgment_code, 1) != 1)
	{
		perror("Error in reading acknowledgment code - put\n");
		return;
	}

	// check acknowledgment code received
	if (acknowledgment_code == '1')
	{
		printf("Server unable to receive file due to insufficient permission (EACCES)\n");
		return; // end function
	}
	else if (acknowledgment_code == '2')
	{
		printf("Server unable to receive file due to file name being invalid (EINVAL)\n");
		return; // end function
	}
	else if(acknowledgment_code == '3')
	{
		printf("Server unable to receive file due to other reasons\n");
		return; // end function
	}
	else if(acknowledgment_code != '0')
	{
		printf("Unknown ack code received\n");
		return;
	}

	// section c
	// if acknowledgement code = 0
	// sent respond

	// sent opcode
	char code = (char)RESPOND;
	if (write(sd, (char *)&code, 1) != 1)
	{
		perror("Error in sending opcode - put\n");
		return;
	}

	// get file size
	if (fstat(fd, &fileInfo) < 0)
	{
		perror("Error in fstat\n");
		return;
	}
	filesize = (int)fileInfo.st_size; // size of the file

	// send 4-bytes int - file size
	int data_size = htonl(filesize); // convert to network byte order
	if (write(sd, &data_size, 4) != 4)
	{
		perror("Error sending filesize - put\n");
		return;
	}

	// sent content of the file
	char buf[BUFSIZE];

	printf("Starting to send file to server\n");
	// write to server
	while ((nr = read(fd, buf, BUFSIZE)) > 0)
	{
		// write to server
		if (WriteN(sd, buf, nr) == -1)
		{
			perror("Error in sending file content - get\n");
			return;
		}
	}
	printf("Sent: %s\n", filename);
	return;
}

void get(int sd, char *filename)
{
	int filename_len = strlen(filename);
	char acknowledgment_code, acknowledgment_code_to_server = '0';
	char opcode;
	int fd, nw, nr, n;
	int filesize;

	// section a
	// client send message to server to request the file
	if (nw = WritePath(sd, GET, filename_len, filename) <= 0)
	{
		perror("Error in sending message to server - get\n");
		exit(1);
	}

	// check server respond
	// get opcode from server
	if (read(sd, (char *)&opcode, 1) != 1)
	{
		perror("Error reading opcode - get\n");
		exit(1);
	}

	if (opcode == GET)
	{
		// obtain acknowledgment code from server
		if (read(sd, (char *)&acknowledgment_code, 1) != 1)
		{
			printf("Error reading acknowledgment code - get\n");
			acknowledgment_code_to_server = '1';
		}

		// check opcode & acknowledgment code
		if (acknowledgment_code == '1')
		{
			printf("Access denied or file do not exist (EACCES)\n");
			acknowledgment_code_to_server = '1';
		}
		else if(acknowledgment_code == '2')
		{
			printf("Invalid file name (EINVAL)\n");
			acknowledgment_code_to_server = '1';
		}
		else if(acknowledgment_code == '3')
		{
			printf("The server cannot accept the file due to other reasons\n");
			acknowledgment_code_to_server = '1';
		}
		else if(acknowledgment_code != '0')
		{
			printf("Unknown ack code received from server\n");
			acknowledgment_code_to_server = '2';
		}
	}
	else
	{
		acknowledgment_code_to_server = '3';
		printf("Unknown opcode received from server\n");
	}

	// if acknowledgment code from server is 0 - means ok
	// check is there is same file exist
	if ((fd = open(filename, O_RDONLY)) != -1)
	{
		printf("Requested file already exist: %s\n", filename);
		acknowledgment_code_to_server = '4';
	}

	// if no same file, create a new file
	if ((fd = open(filename, O_WRONLY | O_CREAT, 0766)) == -1)
	{
		if(errno == EACCES)
		{
			acknowledgment_code_to_server = '5';
			printf("Permission denied for %s\n", filename);
		}
		else if(errno == EINVAL)
		{
			acknowledgment_code_to_server = '6';
			printf("File name invalid: %s\n", filename);
		}
		else
		{
			acknowledgment_code_to_server = '7';
			printf("Unable to create: %s\n", filename);
		}
	}

	// respond server
	char code = (char)GET;
	if (write(sd, (char *)&code, 1) != 1)
	{
		perror("Error in sending opcode - get\n");
		return;
	}
	// sent acknowlegment code
	if (write(sd, (char *)&acknowledgment_code_to_server, 1) != 1)
	{
		perror("Error in sending acknowledgment code - get\n");
		return;
	}

	// if no issue - acknowledgment code is 0
	//else stop function
	if(acknowledgment_code_to_server != '0')
	{
		printf("Ending Get due to Error\n");
		return;
	}

	// start reading receiving the file
	// read opcode from server
	if (read(sd, (char *)&opcode, 1) != 1)
	{
		perror("Error reading opcode - get\n");
		exit(1);
	}
	// read file size
	int data_size;
	if (opcode == RESPOND)
	{
		printf("Server response: OK\n");
		if (read(sd, (char *)&data_size, 4) != 4)
		{
			perror("Error in reading filesize - put\n");
			exit(1);
		}
		filesize = (int)ntohl(data_size); // convert to host byte order
		printf("Filesize: %d\n", filesize);
	}
	else
	{
		printf("Server response: Opcode error\n");
		return;
	}

	// Get number of loops and the remainder bytes needed to get file from server
	int blocks = filesize / (int)BUFSIZE;
	int remainder = filesize % (int)BUFSIZE;

	// read and write file content
	char content[BUFSIZE];

	for (int i = 0; i <= blocks; i++)
	{
		if (i == blocks)
		{
			if ((nr = ReadN(sd, content, remainder)) <= 0)
			{
				perror("Error in reading filename - put\n");
				close(fd);
				return;
			}
			if ((nw = write(fd, content, nr)) < nr)
			{
				perror("Error in writing the requested file - put\n");
				close(fd);
				return;
			}
			break;
		}

		if ((nr = ReadN(sd, content, BUFSIZE)) <= 0)
		{
			perror("Error in reading filename - put\n");
			close(fd);
			return;
		}
		if ((nw = write(fd, content, nr)) < nr)
		{
			perror("Error in writing the requested file - put\n");
			close(fd);
			return;
		}
	}

	// // read and write content
	// char content[BUFSIZE];
	// int blockSize = BUFSIZE;

	// while(filesize > 0){
	// 	if(blockSize > filesize){
	// 		blockSize = filesize;
	// 	}
	// 	if((nr = ReadN(sd,content,blockSize))<=0){
	// 		printf("Error in reading the requested file - put\n");
	// 		close(fd);
	// 		return;
	// 	}
	// 	if( (nw = write(fd, content, nr)) < nr ){
	// 		printf("Error in writing the requested file - put\n");
	// 		close(fd);
	// 		return;
	// 	}
	// 	filesize -= nw;
	// }

	close(fd);
	printf("Received: %s\n", filename);
}