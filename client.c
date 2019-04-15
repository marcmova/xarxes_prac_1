#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

/*  Parameter constants*/
#define MAX_PARAMETERS_LENGTH 100
#define MAX_FILE_PATH_LENGTH 100

/*  Register package types*/
#define REGISTER_REQ 0X00
#define REGISTER_ACK 0X01
#define REGISTER_NACK 0X02
#define REGISTER_REJ 0X03
#define ERROR 0X09

/*  Register possible states*/
#define DISCONNECTED 0
#define WAIT_REG 1
#define REGISTERED 2
#define ALIVE 3

/*  Loop controller variables*/
int request_number = 0, packages_sent = 0;
float t = 2.0;
int n = 3, m = 4, p = 8, s = 5, q = 3;
int break_loop = 0;

/*  An auxiliar package to store the actual data*/
char* package;

/*  Client and server information*/
char name[MAX_PARAMETERS_LENGTH];
char MAC[MAX_PARAMETERS_LENGTH];
char server_ip[MAX_PARAMETERS_LENGTH];
char server_udp_port[MAX_PARAMETERS_LENGTH];
char server_tcp_port[MAX_PARAMETERS_LENGTH];
int state = DISCONNECTED;
char *random_number;

/*  Scoket information*/
struct sockaddr_in	addr_server,addr_client;
int sock_udp,sock_tcp;
socklen_t laddr_server;

int i = 0, j = 0, k = 0; /* auxiliar counters 1*/
int x = 0, y = 0, z = 0; /* auxiliar counters 2*/

/*  Initializing the variables needed*/


/*  Function to read the configuration from the given file and store it*/
void read_configuration(char* file)
{
    FILE *fp;
    int copy;
    char ch;
    char result [4][MAX_PARAMETERS_LENGTH];
    fp = fopen(file, "r");
    if (fp == NULL)
    {
        perror("Error while opening the file.\n");
        exit(EXIT_FAILURE);
    }
    i=0;
    j=0;
    while((ch = fgetc(fp)) != EOF)
    {
        if(ch == ' ')
        {
            ch = fgetc(fp);
            copy = 1;
        }
        if(ch == '\n')
        {
            result[i][j] = '\0';
            i++;
            j=0;
            copy = 0;
        }
        if(copy == 1)
        {
            result[i][j] = ch;
            j++;
        }

    }
    fclose(fp);
    strcpy(name, result[0]);
    strcpy(MAC, result[1]);
    strcpy(server_ip, result[2]);
    strcpy(server_udp_port, result[3]);

}

/*  Function to create a char array that we will use to send the information.
    Takes the information as parameter and returns a char array filled with it.*/
char* create_package(char package_type, char* data)
{
    char* result;

    result = (char*) malloc(78);

    result[0] = package_type;
    for(i=0; i<7; i++)
    {
        result[i+1] = name[i];
    }
    for(i=0; i<13; i++)
    {
        result[i+8] = MAC[i];
    }
    for(i=0; i<7; i++)
    {
        result[i+21] = random_number[i];
    }
    for(i=0; i<50; i++)
    {
        result[i+28] = data[i];
    }
    return result;
}

/*  Function to send a message by udp, it receives the string package and return void*/
void send_udp_message(char* package)
{
    if(sendto(sock_udp, package, 78, 0, (struct sockaddr*) &addr_server, sizeof(addr_server)) < 0){
        printf("Error al sendto\n");
    }
}

/*  Function to receive a message by udp socket within determinated time*/
void receive_udp_message(float waiting_time)
{
    struct timeval tv;

    package = malloc(78);
    tv.tv_sec = waiting_time;
    tv.tv_usec = 0.0;
    setsockopt(sock_udp, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv));
    recvfrom(sock_udp,package,78,0,(struct sockaddr *)&addr_server,&laddr_server);

}

/*  Open the socket and bind it to send messages later*/
void open_udp_socket()
{

	sock_udp = socket(AF_INET, SOCK_DGRAM, 0);
	if(sock_udp < 0)
	{
		printf("No puc obrir socket!!!\n");
		exit(-1);
	}

	memset(&addr_client, 0, sizeof (struct sockaddr_in));
	addr_client.sin_family = AF_INET;
	addr_client.sin_addr.s_addr = htonl(INADDR_ANY);
	addr_client.sin_port = htons(0);

	/* Fem el binding */
	if(bind(sock_udp, (struct sockaddr *)&addr_client, sizeof(struct sockaddr_in))<0)
	{
        fprintf(stderr, "No puc fer el binding del socket!!!\n");
	       exit(-2);
	}

    memset(&addr_server, 0, sizeof(addr_server));

    addr_server.sin_family = AF_INET;
    addr_server.sin_port = htons(atoi(server_udp_port));
    addr_server.sin_addr.s_addr = atoi(server_ip);
}
/*  Function to print the actual time at the beginning of the line*/
void print_time()
{
    time_t timer;
    char buffer[26];
    struct tm* tm_info;

    time(&timer);
    tm_info = localtime(&timer);

    strftime(buffer, 26, "%H:%M:%S", tm_info);
    printf("%s: ", buffer);


}

/*  Function to do a register try*/
void register_try()
{
    for(j = 0; j < 8 && (state == WAIT_REG || state == DISCONNECTED); j++)
    {
        if(j < 2){
            send_udp_message(create_package(REGISTER_REQ,"0"));
            if(state == DISCONNECTED)
            {
                state = WAIT_REG;
            }
            receive_udp_message(t);
            if(package[0] == REGISTER_ACK)
            {
                print_time();
                printf("Registered\n");
                state = REGISTERED;
                break_loop = 1;
            }
            else if(package[0] == REGISTER_NACK)
            {
                state = DISCONNECTED;
                j=8;
            }
            else if(package[0] == REGISTER_REJ)
            {
                state = DISCONNECTED;
                break_loop = 1;
            }
        }else if(t*j < t*m){
            send_udp_message(create_package(REGISTER_REQ,"0"));
            receive_udp_message(t*j);
        }else{
            send_udp_message(create_package(REGISTER_REQ,"0"));
            receive_udp_message(t*m);
        }

    }
}


int main(int argc, char const *argv[])
{

    char configuration_file[MAX_FILE_PATH_LENGTH];
    char data[50];

    /*  Definition of the configuration file path. */
    strcpy(configuration_file, "client.cfg");
    for(i=1; i<argc; i++)
    {
        if(strcmp(argv[i], "-c") == 0)
        {
            if(argc > i+1)
            {
                strcpy(configuration_file, argv[i+1]);

            }
            else
            {
                printf("Error, after \"-c\" parametermust be the configuration file.\n");
            }
        }
    }
    read_configuration(configuration_file);

    /*  Create a package that will be sent for the register request*/
    random_number = "000000";
    for(i=0; i<50; i++)
    {
        data[i] = '\0';
    }

    /*  Start the register loop, it will do three tries on the register before it close the client*/
    open_udp_socket();
    break_loop = 0;
    for(request_number = 0; request_number < 3 && break_loop == 0; request_number++)
    {
        register_try();
        if(break_loop == 0 && request_number < 3)
        {
            sleep(5);
        }

    }
    if(break_loop == 0)
    {
        printf("Paquet enviat incorrectament\n");
    }
    else
    {
        for(i = 21; i < 27; i++)
        {
            printf("%c", package[i]);
        }
        printf("\n");
    }
    return 0;

}
