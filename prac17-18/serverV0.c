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

#define DISCONNECTED 0XA0
#define NOT_SUBSCRIBED 0XA1
#define WAIT_ACK_SUBS 0XA2
#define WAIT_INFO 0XA3
#define WAIT_ACK_INFO 0XA4
#define SUBSCRIBED 0XA5
#define SEND_HELLO 0XA6

#define SUBS_REQ 0X00
#define SUBS_ACK 0X01
#define SUBS_REJ 0X02
#define SUBS_INFO 0X03
#define INFO_ACK 0X04
#define SUBS_NACK 0X05

#define HELLO 0X10
#define HELLO_REJ 0X11

char nom[20];
char MAC[20];
char TCP[20];
char UDP[20];
char nom_con[20][20];
char situacio_con[20][20];

struct sockaddr_in	addr_server,addr_cli,addr_server_2;
int sock,sock_2;
unsigned int laddr_cli;
char new_port[80];

void obrirsocket(){
	int  a;
	char dades[103];
	int port;
	unsigned int tipus;

	port=atoi(UDP);

	sock=socket(AF_INET,SOCK_DGRAM,0);	
	if(sock<0)
	{
		fprintf(stderr,"No puc obrir socket!!!\n");
		exit(-1);
	}
	memset(&addr_server,0,sizeof (struct sockaddr_in));
	addr_server.sin_family=AF_INET;
	addr_server.sin_addr.s_addr=htonl(INADDR_ANY);
	addr_server.sin_port=htons(port);
	if(bind(sock,(struct sockaddr *)&addr_server,sizeof(struct sockaddr_in))<0)
	{
		fprintf(stderr,"No puc fer el binding del socket!!!\n");
                exit(-2);
	}
		while(true){
		a=recvfrom(sock,dades,103,0,(struct sockaddr *)&addr_cli,&laddr_cli);	
		if(a<0)
		{
			fprintf(stderr,"Error al recvfrom\n");
			exit(-2);
		}
		tipus = dades[0];
		if(tipus == 0){
			
			/*packOpening(dades)*/
		}
	}


	
}

/*void packOpening(char *dades){
	if(tipus==SUBS_REQ){
		while(data[i]!=','){
			nom[i]=data[i];
			i++;			
		}
		nom[i]='\0';
		i=0;	
		while(i<6){
			if(strcmp(situacio_con[i],Mac_client)==0){
				printf("correct1");
				MAC_bool=true;
			}
			i++;
		}
		i=0;
		while(i<6){
			if(strcmp(nom_con[i],nom)==0){
				printf("correct2");
				nom_bool=true;
			}
			i++;
		}
		if(strcmp(num_aleatori,"00000000")==0){
			printf("correct3");
			num_bool=true;
		}
		if(nom_bool==true&&MAC_bool==true&&num_bool==true){
			send_ack_subs();
		}
		if(nom_bool==false){
			printf("Error nom de controlador no valid");		
		}
		if(num_bool==false){
			printf("Error numero aleatori no valid");		
		}
		if(Mac_bool==false){
			printf("Error Mac de controlador no valid");		
		}
	}
	if(tipus==SUBS_INFO){
		
		i=0;	
		while(i<6){
			if(strcmp(situacio_con[i],Mac_client)==0){
				printf("correct1");
				MAC_bool=true;
			}
			i++;
		}
		i=0;
		if(strcmp(num_aleatori,"12345678")==0){
			printf("correct3");
			num_bool=true;
		}
		if(MAC_bool==true&&num_bool==true){
			send_ack_info();
		}
		if(num_bool==false){
			printf("Error numero aleatori no valid");		
		}
		if(Mac_bool==false){
			printf("Error Mac de controlador no valid");		
		}
	}
	if(tipus==HELLO){
		while(data[i]!=','){
			nom[i]=data[i];
			i++;			
		}
		nom[i]='\0';
		i=0;	
		while(i<6){
			if(strcmp(situacio_con[i],Mac_client)==0){
				printf("correct1");
				MAC_bool=true;
			}
			i++;
		}
		i=0;
		while(i<6){
			if(strcmp(nom_con[i],nom)==0){
				printf("correct2");
				nom_bool=true;
			}
			i++;
		}
		if(strcmp(num_aleatori,"12345678")==0){
			printf("correct3");
			num_bool=true;
		}
		
		if(nom_bool==true&&MAC_bool==true&&num_bool==true){
			printf("hey");			
			sendHellos(data);
		}
		if(nom_bool==false){
			printf("Error nom de controlador no valid");		
		}
		if(num_bool==false){
			printf("Error numero aleatori no valid");		
		}
		if(Mac_bool==false){
			printf("Error Mac de controlador no valid");		
		}
	}
	
}*/

void readConfiguration(char *c_file){
	char c;
	bool stat=false;
	FILE * fp;
	fp=fopen (c_file,"r");
	c=fgetc(fp);
	while(c!='\n'&&stat==false){
		if(c=='='){
			c=fgetc(fp);
			fgets(nom,30,fp);
			stat=true;
		}
	c=fgetc(fp);
	}
	stat=false;
	while(c!='\n'&&stat==false){
		if(c=='='){
			c=fgetc(fp);
			fgets(MAC,30,fp);
			stat=true;
		}
	c=fgetc(fp);
	}
	stat=false;
	while(c!='\n'&&stat==false){
		if(c=='='){
			c=fgetc(fp);
			fgets(UDP,30,fp);
			stat=true;
		}
	c=fgetc(fp);
	}
	stat=false;
	while(c!='\n'&&stat==false){
		if(c=='='){
			c=fgetc(fp);
			fgets(TCP,30,fp);
			stat=true;
		}
	c=fgetc(fp);
	}

}

void readControlers(char *au_file){
	char c='a';
	int i=0;
	int j=0;
	FILE * controladors;
	controladors=fopen (au_file,"r");
	while(j<6){	
		while(i<8){
			c=getc(controladors);
			if(c!=','){
				nom_con[j][i]=c;
			}
			i++;
		}
		nom_con[j][i]='\0';
		i=0;
		c=getc(controladors);
		while(i<13){
			c=getc(controladors);
			if(c!='\n'){
				situacio_con[j][i]=c;
			}
			
			i++;
		}
		situacio_con[j][i]='\0';
		i=0;
		j++;
	}
}
int main(int argc, char *argv[]){
	int i;
	char *c_file = "", *au_file = "";
	if(argc > 1){
		for(i = 0; i < argc-1; i++){
			if(strcmp(argv[i], "-c") == 0){
				c_file = argv[i+1];
			}
		}
	}
	if(strcmp(c_file,"") == 0){
		c_file = "server.cfg";
	}
	if(argc > 1){
		for(i = 0; i < argc-1; i++){
			if(strcmp(argv[i], "-u") == 0){
				au_file = argv[i+1];
			}
		}
	}
	if(strcmp(au_file,"") == 0){
		au_file = "controlers.dat";
	}
	readConfiguration(c_file);
	readControlers(au_file);
	obrirsocket();
	return 0;
}
