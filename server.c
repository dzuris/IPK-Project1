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
    
    // CTRL+C listener, server is closing in INThandler function
	signal(SIGINT, INThandler);

	// setting port
	const int PORT = atoi(argv[1]);

    // Creating socket file descriptor
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) <= 0)
    {
        perror("ERROR in socket");
        exit(EXIT_FAILURE);
    }

    // User is able to use port instantly after closing server
    int optval = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR,
    	(const void *)&optval, sizeof(int));

    // Server address options
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

    // Server run cycle
    while(1)
    {
        printf("\n+++++++ Waiting for new connection ++++++++\n\n");

        // Creating client socket
    	int client_socket;
        if ((client_socket = accept(server_socket, (struct sockaddr *)&server_address, (socklen_t*)&addrlen))<0)
        {
            perror("ERROR in accept");
            exit(EXIT_FAILURE);
        }
    
        // Get request from client
        char buffer[2048] = {0};
        read( client_socket , buffer, 2048);

        // Compare request
        char message[256] = {0};
        // Return hostname
        if (!strncmp(buffer, "GET /hostname ", 14))
        {
            printf("System returning hostname to client\n");
            char hostname[256];
            get_hostname(hostname);
            create_http_message(message, hostname);
        }
        // Return cpu name
        else if(!strncmp(buffer, "GET /cpu-name ", 14))
        {
            printf("System returning cpu-name to client\n");
            char cpu[256];
            get_cpu_name(cpu);
            create_http_message(message, cpu);
        }
        // Return cpu usage
        else if(!strncmp(buffer, "GET /load ", 10))
        {
            printf("System returning cpu-load to client\n");
            char percent[4];
            sprintf(percent, "%d", GetCPULoad());
            strcat(percent, "%");
            create_http_message(message, percent);
        }
        // Error: Bad request
        else
        {
            create_http_message(message, "400: Bad request");
        }

        // Write message to client
        write(client_socket , message , (int)strlen(message));

        printf("\n+++++++ Closing connection ++++++++\n\n");

        // Closing client socket, then waiting for another one
        close(client_socket);
    }

    return 0;
}

/*
 * Creating http message in text/plain form, returning by pointer final_message and printing information from message_to_include
 */
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

/*
 * Ctrl+C listener, closing socket when server is shutting down
 */
void  INThandler(int sig)
{
     signal(sig, SIG_IGN);
     printf("\nServer is shutting down...\n");

     close(server_socket);

     exit(0);
}

/*
 * Funtion returns hostname by pointer
 */
void get_hostname(char *return_hostname)
{
    FILE *file = fopen("/proc/sys/kernel/hostname", "r");
    fgets(return_hostname, 100, file);
    fclose(file);
    return_hostname[strlen(return_hostname) - 1] = '\0';
}

/*
 * Funtion returns cpu name by pointer
 */
void get_cpu_name(char *return_cpu_name)
{
    char *str = "cat /proc/cpuinfo | grep \"model name\" | head -n 1 | awk -F \": \" '{print $2}'";
    FILE *file = popen(str, "r");
    fgets(return_cpu_name, 100, file);
    return_cpu_name[strlen(return_cpu_name) - 1] = '\0';
    pclose(file);
}

/*
 * Function returns integer of on how many percent is cpu used
 */
int GetCPULoad()
{
    // List of data
    unsigned long long int first_data[10];
    unsigned long long int second_data[10];
    // Getting values with space of 0.5sec
    get_cpu_data(&first_data);
    usleep(500000);
    get_cpu_data(&second_data);

    // Calculations for cpu usage
    first_data[3] = first_data[3] + first_data[4];
    second_data[3] = second_data[3] + second_data[4];
    unsigned long long int prev_non_idle = first_data[0] + first_data[1] + first_data[2] + first_data[5] + first_data[6] + first_data[7];
    unsigned long long int non_idle = second_data[0] + second_data[1] + second_data[2] + second_data[5] + second_data[6] + second_data[7];
    unsigned long long int prev_total = first_data[3] + prev_non_idle;
    unsigned long long int total = second_data[3] + non_idle;
    unsigned long long int total_diff = total - prev_total;
    unsigned long long int idle_diff = second_data[3] - first_data[3];

    // Rounding cpu usage
    int load_int = ((total_diff - idle_diff)/(float)total_diff)*100;
    float load = ((total_diff - idle_diff)/(float)total_diff)*100;
    if ((load - load_int) >= 0.50)
        load_int++;

    return load_int;
}

/*
 * Function return data by list pointer, list is filled by cpu usage values
 */
void get_cpu_data(unsigned long long int *data)
{
    // get cpu stats
    char cpu_data_str[256];
    FILE *file = fopen("/proc/stat", "r");
    fgets(cpu_data_str, 256, file);
    fclose(file);

    // Split stats into list
    int init_size = strlen(cpu_data_str);
    char delim[] = " ";

    char *ptr = strtok(cpu_data_str, delim);

    char data_list[11][256];
    int i = 0;

    // copy splitted rows into list
    while (ptr != NULL)
    {
        strcpy(data_list[i++], ptr);
        ptr = strtok(NULL, delim);
    }

    // Return list
    data[0] = atoi(data_list[1]);
    data[1] = atoi(data_list[2]);
    data[2] = atoi(data_list[3]);
    data[3] = atoi(data_list[4]);
    data[4] = atoi(data_list[5]);
    data[5] = atoi(data_list[6]);
    data[6] = atoi(data_list[7]);
    data[7] = atoi(data_list[8]);
    data[8] = atoi(data_list[9]);
    data[9] = atoi(data_list[10]);
}
