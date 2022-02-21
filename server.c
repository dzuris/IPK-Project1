#include <stdio.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <sys/types.h>

#include <netinet/in.h>

int main(int argc, char *argv[])
{
	// check if user passed PORT number
	if(argc != 2)
	{
		fprintf(stderr, "Wrong arguments input!\n");

		return 1;
	}

	int port = atoi(argv[1]);

	char server_message[256] = "You have reached the server!";

	// create the server socket
	int server_socket;
	server_socket = socket(AF_INET, SOCK_STREAM, 0);

	// check if socket was successfully created
	if(server_socket < 0)
	{
		fprintf(stderr, "Cannot create socket\n");

		return 1;
	}

	// define the server address
	struct sockaddr_in server_address;
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(port);
	server_address.sin_addr.s_addr = htonl(INADDR_ANY);

	// bind the socket to our specified IP and port
	if(bind(server_socket, (struct sockaddr*) &server_address, sizeof(server_address)) < 0)
	{
		fprintf(stderr, "Bind failed\n");

		return 1;
	}

	if (listen(server_socket, 5) < 0)
	{
		fprintf(stderr, "Error in listen\n");

		return 1;
	}

	while (1)
	{
		printf("Waiting for new connection\n\n");
		int client_socket;
		client_socket = accept(server_socket, NULL, NULL);

		if (client_socket < 0)
		{
			fprintf(stderr, "Error in accept\n");

			return 1;
		}

		// send the message
		send(client_socket, server_message, sizeof(server_message), 0);

		// close the socket
		close(server_socket);
	}

	return 0;
}
