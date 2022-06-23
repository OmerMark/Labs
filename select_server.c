 /**************************************
  * Computer Networks Laboratory     
  * Lab              9
  *                  Socket programming
  * Question         3       
  * File             select_server.c
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
enum EXIT_NUMBERS { SUCCSES, INPUT, SOCKET, BIND, SELECT, GETSOCKNAME, CLOSE };

// Defines
#define BUFFER_SIZE 		1024
#define NUMBER_OF_ARG_INPUT 10

int main (int argc, char* argv[])
{
/*
* The server's input is: { port_1 , port_2,..., port_n }
* port_i : a udp port which the server listen to. There are up n ports, you may assume that
* 1 ≤ 𝑛 ≤ 10
*/

	 // Initializes variables
    char 		buffer[BUFFER_SIZE] = "";
	int 		socket_list[10], i, listnum, inputfd;
    fd_set 		fdset, temp_socket;
	struct		sockaddr_in 		serv_addr[10], sin_helper;
    socklen_t	sin_helper_len;

	// Clear the set
    FD_ZERO(& fdset);
    FD_ZERO(& temp_socket);

	// Check arguments. Should from 1 to 10
	if (argc > NUMBER_OF_ARG_INPUT && argc > 1)
	{
		printf("The server's input is: { port_1 , port_2,..., port_n }\n");
		perror("Select_server: input");
		return INPUT;
	}
    printf("select_server is ready\n");

	// Open n udp sockets and set their properties
    for(i = 0; i <= (argc - 2); i++)
    {
		// Get a UDP socket.
        socket_list[i] = socket(AF_INET, SOCK_DGRAM, 0);
        if (socket_list[i] < 0)
	    {
		    perror("Select_server: socket");
            for(i=i-1; i >= 0; i--)
            {
                if (close(socket_list[i]) < 0)
                {
			      perror("Select_server: close - socket_list");
			     return CLOSE;
		        }
            }

		    return SOCKET;
        }

		/*
		setsockopt(socket_list[i], SOL_SOCKET, SO_REUSEADDR, &reuse_addr, sizeof(reuse_addr)) != SUCCSES)
		*/


		// Init the fields by the given Arguments 
        serv_addr[i].sin_family 	 = AF_INET;
		serv_addr[i].sin_addr.s_addr = htonl(INADDR_ANY);  // ? need to check about the htonl
      	serv_addr[i].sin_port 		 = htons(atoi(argv[i + 1]));
	    memset(serv_addr[i].sin_zero, '\0', sizeof serv_addr[i].sin_zero);
        
		 // Bind the socket to the server address.
        if ((bind(socket_list[i], (struct sockaddr *) &serv_addr[i], sizeof serv_addr[i])) < 0)
        {
		    perror("Select_server: bind");
            for(i=i-1; i >= 0; i--)
            {
                if (close(socket_list[i]) < 0)
                {
			        perror("Select_server: close - socket_list");
			        return CLOSE;
		        }
            }

		    return BIND;
        }
        
        FD_SET(socket_list[i], & fdset);
    }
    // Main loop
	while(1)
	{
		//temp socket
		temp_socket = fdset;

		// Select(): waits for sockets(or any FD) to change status.
		// Wait for user sockets to have data.
		printf("\nWaiting for clients!!!\n");
        inputfd = select(FD_SETSIZE, &temp_socket, NULL , NULL, NULL);
        if (inputfd < 0)
        {
            perror("Select_server: select");
            for(i=i-1; i >= 0; i--)
            {
                if (close(socket_list[i]) < 0)
                {
			        perror("Select_server: close - socket_list");
			        return CLOSE;
		        }
            }

	       	return SELECT;
        }

     	// Look of which socket have data
        for (listnum = 0; listnum < FD_SETSIZE; listnum++)
        {
             memset(sin_helper.sin_zero, 0, sizeof sin_helper.sin_zero);
            if (FD_ISSET(listnum, &temp_socket))
            {
                sin_helper_len = sizeof(sin_helper);

				// get the port of that socket
                if (getsockname(listnum, (struct sockaddr *) &sin_helper, &sin_helper_len) < 0)
                {
                     perror("Select_server: getsockname");
                     for(i=i-1; i >= 0; i--)
                    {
                        if (close(socket_list[i]) < 0)
                        {
			            perror("Select_server: close - socket_list");
			            return CLOSE;
                        }
                    }
                
                    return GETSOCKNAME;
                }

				// If socket with port i has data - print Client with port i has sent a message
                printf("Client with port %d has sent a message\n", ntohs(sin_helper.sin_port));
                recvfrom(listnum, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&sin_helper, &sin_helper_len);
			}
        }
		memset(sin_helper.sin_zero, 0, sizeof sin_helper.sin_zero);
		listnum = 0;
    }
	
	// Close the sockets
	for(i=i-1; i >= 0; i--)
	{
        if (close(socket_list[i]) < 0)
        {
			perror("Select_server: close - socket_list");
			return CLOSE;
        }
    }


	return SUCCSES;
}

