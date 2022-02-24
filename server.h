/* Function declarations */

void INThandler(int);
void create_http_message(char *final_message, char *message_to_include);
int valid_int(char *number);

void get_hostname(char *return_hostname);
void get_cpu_name(char *return_cpu_name);
int GetCPULoad();
void get_cpu_data(unsigned long long int (*data)[10]);
