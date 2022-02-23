// Server side C program to demonstrate HTTP Server programming
#include <stdio.h>
#include <sys/socket.h>
//#include <sys/types.h>
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
            char percent[4];
            sprintf(percent, "%d", GetCPULoad());
            strcat(percent, "%");
            create_http_message(message, percent);
        }
        else
        {
            create_http_message(message, "400: Bad request");
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

int GetCPULoad()
{
    unsigned long long int first_data[10];
    unsigned long long int second_data[10];
    get_cpu_data(&first_data);
    usleep(500000);
    get_cpu_data(&second_data);

    first_data[3] = first_data[3] + first_data[4];
    second_data[3] = second_data[3] + second_data[4];
    unsigned long long int prev_non_idle = first_data[0] + first_data[1] + first_data[2] + first_data[5] + first_data[6] + first_data[7];
    unsigned long long int non_idle = second_data[0] + second_data[1] + second_data[2] + second_data[5] + second_data[6] + second_data[7];
    unsigned long long int prev_total = first_data[3] + prev_non_idle;
    unsigned long long int total = second_data[3] + non_idle;
    unsigned long long int total_diff = total - prev_total;
    unsigned long long int idle_diff = second_data[3] - first_data[3];

    int load_int = ((total_diff - idle_diff)/(float)total_diff)*100;
    float load = ((total_diff - idle_diff)/(float)total_diff)*100;
    if ((load - load_int) >= 0.50)
        load_int++;

    /*
    pdata = first
    cdata = second
    unsigned long long int user;        0
    unsigned long long int nice;        1
    unsigned long long int systemA;     2
    unsigned long long int idle;        3
    unsigned long long int iowait;      4
    unsigned long long int irq;         5
    unsigned long long int softirq;     6
    unsigned long long int steal;       7
    unsigned long long int guest;       8
    unsigned long long int guest_nice;  9
    */

    return load_int;
}

void get_cpu_data(unsigned long long int *data)
{
    char cpu_data_str[256];
    FILE *file = fopen("/proc/stat", "r");
    fgets(cpu_data_str, 256, file);
    fclose(file);

    int init_size = strlen(cpu_data_str);
    char delim[] = " ";

    char *ptr = strtok(cpu_data_str, delim);

    char data_list[11][256];
    int i = 0;

    while (ptr != NULL)
    {
        strcpy(data_list[i++], ptr);
        ptr = strtok(NULL, delim);
    }

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

    /*data->user = data_list[0];
    data->nice = data_list[1];
    data->systemA;
    data->idle;
    data->iowait;
    data->irq;
    data->softirq;
    data->steal;
    data->guest;
    data->guest_nice;*/
}
