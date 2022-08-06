#include "sFunctionHelper.h"
#include "../logger/logger.h"

void pwd_func(int sd)
{
	// log file operation
	Logger("PWD start\n");

	char current_directory[BUFSIZE];
	getcwd(current_directory, sizeof(current_directory)); // get pathname

	// respond to client
	// sent opcode to client first
	char code = (char)PWD;
	if (write(sd, (char *)&code, 1) != 1)
	{
		Logger("Error in sending opcode - pwd\n");
		return;
	}

	// sent 2-bytes of pathname size
	short data_size = strlen(current_directory); /* convert to network byte order */
	data_size = htons(data_size);
	if (write(sd, &data_size, 2) != 2)
	{
		Logger("Error in sending pathname size - pwd\n");
		return;
	}

	// sent pathname
	if (WriteN(sd, current_directory, strlen(current_directory)) < 0)
	{
		Logger("Error in sending pathname - pwd\n");
		return;
	}

	Logger("PWD completed\n");
	return;
}

void dir_func(int sd)
{
	// log file operation
	Logger("DIR start\n");

	char dirname[256];
	DIR *dp;
	struct dirent *direntp;
	char filenames[BUFSIZE];
	int length;

	// obtain all filenames first

	strcpy(dirname, "."); // current directory

	if ((dp = opendir(dirname)) == NULL)
	{
		char *logstr;
		strcat(logstr, "Error in opening directory ");
		strcat(logstr, dirname);
		strcat(logstr, "\n");
		Logger(logstr);
		exit(1);
	}

	// store all filenames
	while ((direntp = readdir(dp)) != NULL)
	{
		strcat(filenames, direntp->d_name);
		strcat(filenames, "\n");
	}

	length = strlen(filenames);
	filenames[length - 1] = '\0';
	closedir(dp);

	// respond client
	// sent opcode to client
	char code = (char)DIR_OPC;
	if (write(sd, (char *)&code, 1) != 1)
	{
		Logger("Error in sending opcode - dir\n");
		return;
	}

	// sent 4-bytes length of filenames array
	int data = htonl(strlen(filenames));
	if (write(sd, (char *)&data, 4) != 4)
	{
		Logger("Error in sending length of array - dir\n");
		return;
	}

	// sent content - all filenames
	if (WriteN(sd, filenames, strlen(filenames)) < 0)
	{
		Logger("Error in sending content - dir\n");
		return;
	}

	Logger("DIR completed\n");
}

void cd_func(int sd)
{
	// log file operation
	Logger("CD start\n");

	int length, check;
	char acknowledgment_code;

	// read client's message
	// read length of new directory name
	short data_size;
	if (read(sd, (char *)&data_size, 2) != 2)
	{
		Logger("Error in reading length of directory name - cd\n");
		return;
	}
	length = (int)ntohs(data_size); /* convert to host byte order */

	char directory[length + 1];

	// read name of new directory
	if (ReadN(sd, directory, length) == -1)
	{
		Logger("Error in reading directory name - cd\n");
		return;
	}

	directory[length] = '\0';

	// change directory
	check = chdir(directory);
	// Create new log in new directory
	InitializeLogger();

	if (check == 0)
	{
		acknowledgment_code = '0';
	}
	else
	{
		acknowledgment_code = '1';
	}

	// respond to client
	// sent opcode to client
	char code = (char)CD;
	if (write(sd, (char *)&code, 1) != 1)
	{
		Logger("Error in sending opcode - cd\n");
		return;
	}

	// sent acknowledment code
	if (write(sd, (char *)&acknowledgment_code, 1) != 1)
	{
		Logger("Error in sending opacknowledgment code - cd\n");
		return;
	}

	Logger("CD completed\n");
}

void put_func(int sd)
{
	// log file operation
	Logger("PUT start\n");

	int filename_len, filesize;
	char acknowledgment_code;
	char opcode;
	int fd, nr, nw, n;

	// read length of filename
	short data_size;
	if (read(sd, (char *)&data_size, 2) != 2)
	{
		Logger("Error in reading the length of filename - put\n");
		return;
	}
	filename_len = (int)ntohs(data_size); // convert to host byte order

	char filename[filename_len + 1];

	// read filename
	if (ReadN(sd, filename, filename_len) == -1)
	{
		Logger("Error in reading filename - put\n");
		return;
	}
	filename[filename_len] = '\0';

	// create and open the a file to prepare for recieving file from client
	acknowledgment_code = '0';
	if ((fd = open(filename, O_RDONLY)) != -1)
	{
		if(errno == EINVAL)
		{
			acknowledgment_code = '2';
			Logger("Invalid file name (EINVAL)\n");
		}
		else if(errno == EACCES)
		{
			acknowledgment_code = '1';
			Logger("Access denied or file do not exist (EACCES)\n");
		}
		else
		{
			acknowledgment_code = '3';
			Logger("The server cannot accept the file due to other reasons\n");
		}
	}
	else if ((fd = open(filename, O_WRONLY | O_CREAT, 0766)) == -1)
	{
		if(errno == EACCES)
		{
			acknowledgment_code = '1';
			Logger("Server unable to create file due to insufficient permission (EACCES)\n");
		}
		else if(errno == EINVAL)
		{
			acknowledgment_code = '2';
			Logger("Server unable to create file due to file name being invalid (EINVAL)\n");
		}
		else
		{
			acknowledgment_code = '3';
			Logger("Server unable to create file due to other reasons\n");
		}
	}

	// section b
	// Respond client with require message
	// sent opcode
	char code = (char)PUT;
	if (write(sd, (char *)&code, 1) != 1)
	{
		Logger("Error in sending opcode to client - put\n");
		return;
	}

	// send acknowledgment code
	if (write(sd, (char *)&acknowledgment_code, 1) != 1)
	{
		Logger("Error in sending acknowledgment code - put\n");
		return;
	}

	if (acknowledgment_code != '0')
	{
		Logger("Error in performing command put\n");
		return;
	}

	// After section c
	// Read client's respond

	// read opcode
	if (read(sd, (char *)&opcode, 1) != 1)
	{
		Logger("Error reading opcode - put\n");
		return;
	}

	// check client respond - opcode
	if (opcode == RESPOND)
	{
		// read filesize
		if (read(sd, (char *)&filesize, 4) != 4)
		{
			Logger("Error in reading file size - put\n");
			return;
		}
		filesize = (int)ntohl(filesize); // convert to host byte order
	}

	// Get number of loops and the remainder bytes needed to get file from client
	int blocks = filesize / (int)BUFSIZE;
	int remainder = filesize % (int)BUFSIZE;

	// read and write file content
	char content[BUFSIZE];
	// Loops for block + 1 times for the file
	for (int i = 0; i <= blocks; i++)
	{
		// only enters on last loop to get remainding bytes that is less than BUFSIZE
		if (i == blocks)
		{
			if ((nr = ReadN(sd, content, remainder)) <= 0)
			{
				Logger("Error in reading filename - put\n");
				close(fd);
				return;
			}
			if ((nw = write(fd, content, nr)) < nr)
			{
				Logger("Error in writing the requested file - put\n");
				close(fd);
				return;
			}
			break;
		}

		// Reads from client and write to created file only runs when recieving BUFSIZE bytes
		if ((nr = ReadN(sd, content, BUFSIZE)) <= 0)
		{
			Logger("Error in reading filename - put\n");
			close(fd);
			return;
		}
		if ((nw = write(fd, content, nr)) < nr)
		{
			Logger("Error in writing the requested file - put\n");
			close(fd);
			return;
		}
	}

	close(fd);

	Logger("PUT completed\n");
	return;
}

void get_func(int sd)
{
	// log file operation
	Logger("GET start\n");

	int fd, nr, n, nw;
	struct stat fileInfo;
	int filesize;
	int filename_len;
	char acknowledgment_code, acknowledgment_code_from_client;
	char opcode;

	// After section a
	// read client message

	// read filename
	short data_size; // sizeof (short) must be 2
	if (read(sd, (char *)&data_size, 2) != 2)
	{
		Logger("Error in reading length of filename - get\n");
		return;
	}
	filename_len = (int)ntohs(data_size); // convert to host byte order

	char filename[filename_len + 1];

	// read filename
	if (ReadN(sd, filename, filename_len) == -1)
	{
		Logger("Error in reading filename - get\n");
		return;
	}
	filename[filename_len] = '\0';

	acknowledgment_code = '0';

	// check file
	// if error encountered
	if ((fd = open(filename, O_RDONLY)) == -1)
	{
		if(errno == EINVAL)
		{
			acknowledgment_code = '2';
			Logger("Invalid file name (EINVAL)\n");
		}
		else if(errno == EACCES)
		{
			acknowledgment_code = '1';
			Logger("Access denied or file do not exist (EACCES)\n");
		}
		else
		{
			acknowledgment_code = '3';
			Logger("The server cannot send file due to other reasons\n");
		}
	}

	// send error opcode and ackowledgment code
	opcode = (char)GET;
	if (write(sd, (char *)&opcode, 1) != 1)
	{
		Logger("Error in sending respond - get\n");
		return;
	}
	if (write(sd, (char *)&acknowledgment_code, 1) != 1)
	{
		Logger("Error in sending acknowledgment - get\n");
		return;
	}

	// read message from client
	if (read(sd, (char *)&opcode, 1) != 1)
	{
		Logger("Error reading opcode - get\n");
		return;
	}
	if (read(sd, (char *)&acknowledgment_code_from_client, 1) != 1)
	{
		Logger("Error reading acknowledgment code from client - get\n");
		return;
	}


	// Checks that opcode and ackcode indicates continue from client
	//Stop function if there is error
	if(opcode == GET)
	{
		if(acknowledgment_code_from_client == '5')
		{
			Logger("Client unable to receive file due to insufficient permission\n");
			return;
		}
		else if(acknowledgment_code_from_client == '6')
		{
			Logger("Client unable to receive file due to invalid file name\n");
			return;
		}
		else if(acknowledgment_code_from_client == '7')
		{
			Logger("Client unable to receive file due to it being a directory\n");
			return;
		}
		else if(acknowledgment_code_from_client == '8')
		{
			Logger("Client unable to receive file due to other reasons\n");
			return;
		}
	}
	else
	{
		Logger("Unexpected opcode received\n");
		return;
	}
	if(acknowledgment_code != '0' || acknowledgment_code_from_client != '0')
	{
		Logger("Get ended with error\n");
		return;
	}

	// if no error encountered
	// sent opcode
	char code = (char)RESPOND;
	if (write(sd, (char *)&code, 1) != 1)
	{
		Logger("Error in sending respond - get\n");
		return;
	}
	// get file size
	if (fstat(fd, &fileInfo) < 0)
	{
		Logger("Error in fstat - get\n");
		return;
	}
	filesize = (int)fileInfo.st_size; // size of the file

	// reset file pointer
	lseek(fd, 0, SEEK_SET);

	// sent file size
	filesize = htonl(filesize); /* convert to network byte order */
	if (write(sd, &filesize, 4) != 4)
	{
		Logger("Error in sending file size - get\n");
		return;
	}

	// read and write file content
	char buf[BUFSIZE];

	Logger("Sending file to client\n");
	// read content first
	while ((nr = read(fd, buf, BUFSIZE)) > 0)
	{
		// write to server however many bytes was read
		if (WriteN(sd, buf, nr) == -1)
		{
			Logger("Error in sending file content - get\n");
			return;
		}
	}

	Logger("GET completed\n");
	return;
}
