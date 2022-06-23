
 /**************************************
  * Computer Networks Laboratory     
  * Lab              9
  *                  Socket programming
  * Question         1       
  * File             TCP_sender.c
 **************************************/

// includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// enums for the ERROR exit
enum EXIT_NUMBERS { SUCCSES, INPUT, SOCKET, SETSOCKOPT, BIND, LISTEN, ACCEPT, FILE_OPEN, SEND, FILE_CLOSE, CLOSE };

// Defines
#define BUFFER_SIZE         1024
#define NUMBER_OF_ARG_INPUT 5



int main (int argc, char* argv[])
{
/*
* The sender’s input is: {IP_address , port , num_parts , file_name }
* IP_address :  The server’s IP (i.e. the sender's local IP address).
* port :        The TCP port.
* num_parts :   The number of parts to send from the file.
* file_name :   The file to send.
*/

    // Initializes variables
	int         welcome_socketfd, socketfd;
	char        buffer[BUFFER_SIZE] = "";
	int         total_bytes = 0, parts_counter, bytes_counter = 0, cur_data = 0, reuse_addr = 1;
	struct      sockaddr_in         serv_addr;
	struct      sockaddr_storage    serv_stor;
	socklen_t   addr_size;
	FILE        *fptr;

    // Check arguments. Should be only 5.
	if (argc != NUMBER_OF_ARG_INPUT)
	{
		printf("The sender’s input is: {IP_address , port , num_parts , file_name }\n");
		perror("TCP_sender: input");

		return INPUT;
	}

    // Open file - the file we send
	if ((fptr = fopen(argv[4], "r")) == NULL)
	   {
	       perror("TCP_sender : fopen()");
	       return FILE_OPEN;
	   }
	
    // Get a TCP socket - welcome socket.
  	welcome_socketfd = socket(AF_INET, SOCK_STREAM, 0);
	if (welcome_socketfd < 0)
	{
		perror("TCP_sender: socket");
		return SOCKET;
	}
	
	// thats for we can re-bind to it without TIME_WAIT problems.
	if (setsockopt(welcome_socketfd, SOL_SOCKET, SO_REUSEADDR, &reuse_addr, sizeof(reuse_addr)) != SUCCSES)
	{
		perror("TCP_sender: setsockopt");
		return SETSOCKOPT;
	}


    // Init the counters
    parts_counter   = (short)atoi(argv[3]);
    total_bytes     = BUFFER_SIZE * parts_counter;

    // Init the fields by the given Arguments.
	serv_addr.sin_family      = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
	serv_addr.sin_port        = htons(atoi(argv[2]));
	memset(serv_addr.sin_zero, '\0', sizeof serv_addr.sin_zero);  

    // Bind the welcome socket to the server address.
	if ((bind(welcome_socketfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr))) < 0)
	{
		perror("TCP_sender: bind");
		if (close(welcome_socketfd) < 0)
		{
			perror("TCP_sender: close - welcome_socketfd");
			return CLOSE;
		}

		return BIND;
	}

    // Listening to the welcome socket
	if (listen(welcome_socketfd,SOMAXCONN) < 0)
	{
		perror("TCP_sender: listen");
		if (close(welcome_socketfd) < 0)
		{
			perror("TCP_sender: close");
			return CLOSE;
		}

		return LISTEN;
	}
	else printf("Listening\n");

    // Server accepts a connection request from a client.
    // Blocking until a connection is received.
    addr_size   = sizeof serv_stor;
   	socketfd    = accept(welcome_socketfd, (struct sockaddr *) &serv_stor, &addr_size);
	if (socketfd < 0)
	{
		perror("TCP_sender: accept\n");
		if (close(welcome_socketfd) < 0)
		{
			perror("TCP_sender: close - welcome_socketfd");
			return CLOSE;
		}

		return ACCEPT;
	}

	printf("Start packets \n");

    // Main loop
	while(1)
	{
        // Clear the buffer
		bzero(buffer,BUFFER_SIZE);

        // Read BUFFER_SIZE of data from the files
		fread(buffer, BUFFER_SIZE,1 , fptr);

        // Check if finish file before 
		if (feof(fptr))
		{
			if (fclose(fptr) == EOF)
			{
				perror("TCP_sender: close file");
				return FILE_CLOSE;
			}

			printf("End Of File reached");
		}

        // Send data in the buffer
     	cur_data = send(socketfd, buffer, BUFFER_SIZE, 0);
		if (cur_data < 0)
		{
			perror("TCP_sender: send() return negative value");

			if (close(welcome_socketfd) < 0)
			{
				perror("TCP_sender: close - welcome_socketfd");
				return CLOSE;
			}
			if (close(socketfd) < 0)
			{
				perror("TCP_sender: close - socketfd");
				return CLOSE;
			}
			if (fclose(fptr) == EOF)
			{
				perror("TCP_sender: close file");
				return FILE_CLOSE;
			}

			return SEND;
		}

        // Calculate and check if we finish get all the data.
		bytes_counter = bytes_counter + cur_data;
		if (bytes_counter >= total_bytes)
		{
			printf("Finish send all data !\n");
			break;
		}

        // If all parts sends quit
        if (--parts_counter == 0)
        {
            printf("Finish send all parts !\n");
			break;
        }
	}
	
    // Close the file and the sockets
	if (fclose(fptr) == EOF)
	{
		perror("TCP_sender: close file");
		return FILE_CLOSE;
	}

	if (close(welcome_socketfd) < 0)
	{
		perror("TCP_sender: close - welcome_socketfd");
		return CLOSE;
	}

	if (close(socketfd) < 0)
	{
		perror("TCP_sender: close - socketfd");
		return CLOSE;
	}


	return SUCCSES;
}
