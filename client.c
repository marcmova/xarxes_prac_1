#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdlib.h>

#define MAX_PARAMETERS_LENGTH 100
#define MAX_FILE_PATH_LENGTH 100

/* Client information*/
char name[MAX_PARAMETERS_LENGTH];
char MAC[MAX_PARAMETERS_LENGTH];
char server_ip[MAX_PARAMETERS_LENGTH];
char server_udp_port[MAX_PARAMETERS_LENGTH];
char server_tcp_port[MAX_PARAMETERS_LENGTH];

/* Scoket information*/
struct sockaddr_in	addr_server,addr_client;
int sock_udp,sock_tcp;

int i=0, j=0, k=0; /* auxiliar counters 1*/
int x=0, y=0, z=0; /* auxiliar counters 2*/


/*  Function to read the configuration from the given file and store it.*/
void read_configuration(char* file){
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
    while((ch = fgetc(fp)) != EOF){
        if(ch == ' '){
            ch = fgetc(fp);
            copy = 1;
        }
        if(ch == '\n'){
            result[i][j] = '\0';
            i++;
            j=0;
            copy = 0;
        }
        if(copy == 1){
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


int main(int argc, char const *argv[]) {

    /* Definition of the configuration file path. */
    char configuration_file[MAX_FILE_PATH_LENGTH];
    strcpy(configuration_file, "client.cfg");
    for(i=1; i<argc; i++){
        if(strcmp(argv[i], "-c") == 0){
            if(argc > i+1){
                strcpy(configuration_file, argv[i+1]);

            }else{
                printf("Error, after \"-c\" parameter must be the configuration file.\n");
            }
        }
    }
    read_configuration(configuration_file);


    return 0;
}
