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
#include <pthread.h>
#include <unistd.h>

/*  Parameter constants*/
#define MAX_PARAMETERS_LENGTH 200
#define MAX_FILE_PATH_LENGTH 100

/*  Register package types*/
#define REGISTER_REQ 0X00
#define REGISTER_ACK 0X01
#define REGISTER_NACK 0X02
#define REGISTER_REJ 0X03
#define ERROR 0X09

/*  Alive package types*/
#define ALIVE_INF 0X10
#define ALIVE_ACK 0X11
#define ALIVE_NACK 0X12
#define ALIVE_REJ 0X13

/*  Register possible states*/
#define DISCONNECTED 0
#define WAIT_REG 1
#define REGISTERED 2
#define ALIVE 3

/*  Loop controller variables*/
#define n 3
#define m 4
#define p 8
#define s 5
#define q 3
#define t 2
int request_number = 0, packages_sent = 0;
int break_loop = 0;

/*  Alive control variables*/
#define r 3
#define u 3
int count_packages_lost = u;

/*  Debug variable*/
int debug = 0;

/*  An auxiliar package to store the actual data*/
char* package;

/*  Thread variables*/
pthread_t ALIVE_send;
int alive_rejected_package = 0;

/*  Client and server information*/
char name[MAX_PARAMETERS_LENGTH];
char MAC[MAX_PARAMETERS_LENGTH];
char server_ip[MAX_PARAMETERS_LENGTH];
char server_udp_port[MAX_PARAMETERS_LENGTH];
char server_tcp_port[MAX_PARAMETERS_LENGTH];
int state = DISCONNECTED;
char random_number[7];

/*  Scoket information*/
struct sockaddr_in	addr_server,addr_client;
int sock_udp,sock_tcp;
socklen_t laddr_server;

int i = 0, j = 0, k = 0; /* auxiliar counters 1*/
int x = 0, y = 0, z = 0; /* auxiliar counters 2*/

/*  Initializing the variables needed*/

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
        printf("Sendto error\n");
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

/*  Function to do a register try*/
void register_try()
{
    for(j = 0; j < 8 && (state == WAIT_REG || state == DISCONNECTED) && break_loop == 0; j++)
    {
        if(j < 2){
            send_udp_message(create_package(REGISTER_REQ,"0"));
            if(state == DISCONNECTED)
            {
                state = WAIT_REG;
                print_time();
                printf("State changed from DISCONNECTED to WAIT_REG.\n");
            }
            receive_udp_message((float)t);
            if(package[0] == REGISTER_ACK)
            {
                state = REGISTERED;
                print_time();
                printf("State changed from WAIT_REG to REGISTERED.\n");
                break_loop = 1;
            }
            else if(package[0] == REGISTER_NACK)
            {
                state = DISCONNECTED;
                print_time();
                printf("State changed from WAIT_REG to DISCONNECTED.\n");
                j=8;
            }
            else if(package[0] == REGISTER_REJ)
            {
                print_time();
                printf("Register failed: ");
                for(k = 28; package[k] != 0; k++)
                {
                    printf("%c", package[k]);
                }
                printf("\n");
                state = DISCONNECTED;
                break_loop = 1;
            }
        }else if(t*j < t*m){
            send_udp_message(create_package(REGISTER_REQ,"0"));
            receive_udp_message((float)t*j);
            if(package[0] == REGISTER_ACK)
            {
                state = REGISTERED;
                print_time();
                printf("State changed from WAIT_REG to REGISTERED.\n");
                break_loop = 1;
            }
            else if(package[0] == REGISTER_NACK)
            {
                state = DISCONNECTED;
                print_time();
                printf("State changed from WAIT_REG to DISCONNECTED.\n");
                j=8;
            }
            else if(package[0] == REGISTER_REJ)
            {
                print_time();
                printf("Register failed: ");
                for(k = 28; package[k] != 0; k++)
                {
                    printf("%c", package[k]);
                }
                printf("\n");
                state = DISCONNECTED;
                break_loop = 1;
            }
        }else{
            send_udp_message(create_package(REGISTER_REQ,"0"));
            receive_udp_message((float)t*m);
            if(package[0] == REGISTER_ACK)
            {
                state = REGISTERED;
                print_time();
                printf("State changed from WAIT_REG to REGISTERED.\n");
                break_loop = 1;
            }
            else if(package[0] == REGISTER_NACK)
            {
                print_time();
                printf("State changed from WAIT_REG to DISCONNECTED.\n");
                j=8;
            }
            else if(package[0] == REGISTER_REJ)
            {
                print_time();
                printf("Register failed: ");
                for(k = 28; package[k] != 0; k++)
                {
                    printf("%c", package[k]);
                }
                printf("\n");
                state = DISCONNECTED;
                break_loop = 1;
            }
        }

    }
}

void * send_alive()
{
    struct timeval spec;
    fd_set readfds;
    FD_SET(sock_udp, &readfds);


    while(count_packages_lost != 0)
    {
        send_udp_message(create_package(ALIVE_INF, ""));
        state = ALIVE;
        count_packages_lost = count_packages_lost - 1;
        spec.tv_sec = r;
        spec.tv_usec = 0.0;
        print_time();
        if(select(sock_udp+1, &readfds, NULL, NULL, &spec) > 0)
        {

            recvfrom(sock_udp,package,78,0,(struct sockaddr *)&addr_server,&laddr_server);
            if(package[0] == ALIVE_ACK)
            {
                count_packages_lost = u;
                sleep(spec.tv_sec);
                usleep(spec.tv_usec);

            }
            else if(package[0] == ALIVE_NACK)
            {
                sleep(spec.tv_sec);
                usleep(spec.tv_usec);

            }
            else if(package[0] == ALIVE_REJ)
            {
                state = DISCONNECTED;
                alive_rejected_package = 1;
                return NULL;
            }
            else{
                sleep(spec.tv_sec);
                usleep(spec.tv_usec);
            }
        }

    }
    return NULL;

}


int main(int argc, char const *argv[])
{
    char configuration_file[MAX_FILE_PATH_LENGTH];

    /*  Definition of the configuration file path. */
    strcpy(configuration_file, "client.cfg");
    if(argc > 4)
    {
        printf("Parameter error, possible parameters:\n\t-c:\tAllows you to specify the file where configuration is stored, followed by the route of the configuration file.\n\t-d:\tActivates the debug mode.\n");
        return 0;
    }
    for(i = 1; i < argc; i++)
    {
        if(strcmp(argv[i], "-c") == 0)
        {
            if(argc > i + 1)
            {
                i = i + 1;
                strcpy(configuration_file, argv[i]);

            }
            else
            {
                printf("Error, after \"-c\" parameter must be the configuration file.\n");
                return 0;
            }
        }
        else if(strcmp(argv[i], "-d") == 0)
        {
            debug = 1;
        }
        else
        {
            printf("Parameter error, possible parameters:\n\t-c:\tAllows you to specify the file where configuration is stored, followed by the route of the configuration file.\n\t-d:\tActivates the debug mode.\n");
            return 0;
        }
    }
    read_configuration(configuration_file);

    /*  Create a package that will be sent for the register request*/
    for(i = 0; i < 6; i++)
    {
        random_number[i] = '0';
    }
    random_number[i] = '\0';

    /*  Start the register loop, it will do three tries on the register before it close the client*/
    open_udp_socket();
    break_loop = 0;
    for(request_number = 0; request_number < 3 && break_loop == 0; request_number++)
    {
        register_try();
        if(break_loop == 0 && request_number < 2)
        {
            sleep(5);
        }
    }
    if(break_loop == 0)
    {
        print_time();
        printf("Three register tries done, exiting the client.\n");
        return 0;
    }
    else
    {
        for(i = 21; i < 27; i++)
        {
            random_number[i-21] = package[i];
        }
    }
    pthread_create(&ALIVE_send,NULL,send_alive,NULL);
    while(count_packages_lost != 0)
    {
        if(alive_rejected_package != 0)
        {
            alive_rejected_package = 0;
            pthread_cancel(ALIVE_send);
            count_packages_lost = 3;
            break_loop = 0;
            for(i = 0; i < 6; i++)
            {
                random_number[i] = '0';
            }
            random_number[i] = '\0';
            for(request_number = 0; request_number < 3 && break_loop == 0; request_number++)
            {
                register_try();
                if(break_loop == 0 && request_number < 2)
                {
                    sleep(5);
                }
            }
            if(break_loop == 0)
            {
                print_time();
                printf("Three register tries done, exiting the client.\n");
                return 0;
            }
            else
            {
                for(i = 21; i < 27; i++)
                {
                    random_number[i-21] = package[i];
                }
            }
            pthread_create(&ALIVE_send,NULL,send_alive,NULL);
        }

    }
    pthread_cancel(ALIVE_send);
    return 0;

}
