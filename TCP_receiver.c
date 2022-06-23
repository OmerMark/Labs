
 /**************************************
  * Computer Networks Laboratory     
  * Lab              9
  *                  Socket programming
  * Question         1       
  * File             TCP_receiver.c
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
enum EXIT_NUMBERS { SUCCSES, INPUT, SOCKET, CONNECT, RECV, CLOSE };

// Defines
#define BUFFER_SIZE         1024
#define NUMBER_OF_ARG_INPUT 4



int main (int argc, char* argv[])
{
/*
* The receiver’s input is: {IP_address , port, num_parts}
* IP_address :  The server’s IP.
* port :        The TCP port.
* num_parts :   The number of parts of data that need to be received
*/

    // Initializes variables
	int     socketfd;
	char    buffer[BUFFER_SIZE] = "";
	int     total_bytes = 0,parts_counter, bytes_counter = 0, cur_data = 0;
	struct  sockaddr_in serv_addr;


    // Check arguments. Should be only 4.
	if (argc != NUMBER_OF_ARG_INPUT)
	{
		printf("The receiver's input is : {IP_address , port , num_parts}\n");
		perror("TCP_receiver: input");

		return INPUT;
	}

    // Get a TCP socket & check.
    socketfd = socket(AF_INET, SOCK_STREAM, 0);
	if (socketfd < 0)
	{
		perror("TCP_receiver: socket");
		return SOCKET;
	}

    // Init the counters
    parts_counter   = (short)atoi(argv[3]);
    total_bytes     = BUFFER_SIZE * parts_counter;

    // Init the fields by the given Arguments 
	serv_addr.sin_family      = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port        = htons((short)atoi(argv[2]));
	memset(serv_addr.sin_zero, '\0', sizeof serv_addr.sin_zero);  

    // Requests a connect.
	if ((connect(socketfd, (struct sockaddr *) &serv_addr, sizeof serv_addr)) < 0)
	{
		if (close(socketfd) < 0)
		{
			perror("TCP_receiver: close - socketfd");
			return CLOSE;
		}

		    perror("TCP_receiver: connect");
		    return CONNECT;
	}

    // Main loop
	while(1)
	{
        // Clear the buffer
		bzero(buffer, BUFFER_SIZE);

        // Read from connection.
        cur_data = recv(socketfd, buffer, BUFFER_SIZE, 0);

        // Check if read properly
		if(cur_data < 0)
		{
            perror("TCP_receiver: recv() return negative value");
			if (close(socketfd) < 0)
			{
				perror("TCP_receiver: close - socketfd");
			}

            break;
		}
        
        // Check if connection closed by server.
        else if (cur_data == 0)
		{
			printf("connection has been closed ( recv() return 0 )\n");
			break;
		}
        
        // Print data         
		printf("\t%s\n", buffer);

        // Calculate and check if we finish get all the data.
        bytes_counter = bytes_counter + cur_data;
		if (bytes_counter >= total_bytes)
		{
			printf("All the data arrived !\n");
			break;
		}

        // If all parts recv quit
        if (--parts_counter == 0)
        {
            printf("all the parts arrived !\n");
			break;
        }
	}

    // Print number of bytes we needed to receive and actual amount received.
	printf("Supposed to receive %d bytes. Actual amount received is %d bytes.\n",total_bytes, bytes_counter);


    // Close the sockets
	if (close(socketfd) < 0)
	{
		perror("TCP_receiver: close - socketfd");
		return CLOSE;
	}
    

	return SUCCSES;
}
