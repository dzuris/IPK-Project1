// Server side C program to demonstrate HTTP Server programming
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <netinet/in.h>
#include <string.h>

#include "server.h"

int server_socket;

int main(int argc, char const *argv[])
{
	// check if user passed port number
	if (argc != 2)
	{
		perror("ERROR in arguments!");
		exit(EXIT_FAILURE);
	}
    
    // CTRL+C listener
	signal(SIGINT, INThandler);

	// setting port
	const int PORT = atoi(argv[1]);

    // Creating socket file descriptor
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) <= 0)
    {
        perror("ERROR in socket");
        exit(EXIT_FAILURE);
    }

    int optval = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR,
    	(const void *)&optval, sizeof(int));

    struct sockaddr_in server_address;

    bzero((char *) &server_address, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons((unsigned short)PORT);
    
    if (bind(server_socket, (struct sockaddr *) &server_address, sizeof(server_address)) < 0)
    {
        perror("ERROR in bind");
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, 10) < 0)
    {
        perror("ERROR in listen");
        exit(EXIT_FAILURE);
    }

    int addrlen = sizeof(server_address);

    while(1)
    {
        printf("\n+++++++ Waiting for new connection ++++++++\n\n");

    	int client_socket;
        if ((client_socket = accept(server_socket, (struct sockaddr *)&server_address, (socklen_t*)&addrlen))<0)
        {
            perror("ERROR in accept");
            exit(EXIT_FAILURE);
        }
    
        char buffer[2048] = {0};
        read( client_socket , buffer, 2048);

        char message[256] = {0};
        if (!strncmp(buffer, "GET /hostname ", 14))
        {
            printf("System returning hostname to client\n");
            char hostname[256];
            get_hostname(hostname);
            create_http_message(message, hostname);
        }
        else if(!strncmp(buffer, "GET /cpu-name ", 14))
        {
            printf("System returning cpu-name to client\n");
            char cpu[256];
            get_cpu_name(cpu);
            create_http_message(message, cpu);
        }
        else if(!strncmp(buffer, "GET /load ", 10))
        {
            printf("System returning cpu-load to client\n");
            create_http_message(message, "vrat load");
        }
        else
        {
            printf("Invalid request by client\n");
            create_http_message(message, "404: Not Found");
        }

        write(client_socket , message , (int)strlen(message));

        printf("\n+++++++ Closing connection ++++++++\n\n");

        close(client_socket);
    }

    close(server_socket);

    return 0;
}

void create_http_message(char *final_message, char *message_to_include)
{
    char message[256] = "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: ";
    char length[5];
    sprintf(length, "%d", (int)strlen(message_to_include));
    strcat(message, length);
    strcat(message, "\n\n");
    strcat(message, message_to_include);
    strcat(message, "\r\n\r\n");
    strcpy(final_message, message);
}

void  INThandler(int sig)
{
     signal(sig, SIG_IGN);
     printf("\nServer is shutting down...\n");

     close(server_socket);

     exit(0);
}

void get_hostname(char *return_hostname)
{
    FILE *file = fopen("/proc/sys/kernel/hostname", "r");
    fgets(return_hostname, 100, file);
    fclose(file);
    return_hostname[strlen(return_hostname) - 1] = '\0';
}

void get_cpu_name(char *return_cpu_name)
{
    char *str = "cat /proc/cpuinfo | grep \"model name\" | head -n 1 | awk -F \": \" '{print $2}'";
    FILE *file = popen(str, "r");
    fgets(return_cpu_name, 100, file);
    return_cpu_name[strlen(return_cpu_name) - 1] = '\0';
    pclose(file);
}

int get_load()
{

}
