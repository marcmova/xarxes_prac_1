#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <string.h>

#include<netinet/in.h>




/*	Taula de primers */
#define MAXPRIMERS	15
int primers[]={ 1,2,3,5,7,9,11,13,17,19,23,29,31,37,41,43};

#define LONGDADES	100

int main(int argc,char *argv[])
{

	int 			sock,port,laddr_cli,a;
	struct sockaddr_in	addr_server,addr_cli;
	char			dadcli[LONGDADES];
	time_t			temps;


	
	/* Nombre de parametres */
	if(argc!=2)
	{
		printf("Usar: \n");
		printf("\t%s <port>\n",argv[0]);
		exit(0);
	}

	/* Port a usar */
	port=atoi(argv[1]);
	
	if(port < 1024)
	{
		printf("Alerta: Nomes el root pot obrir per sota de 1024\n");
		printf("        S'intentara obrir de totes formes\n");
	}


	/* Obrim socket UDP */
	sock=socket(AF_INET,SOCK_DGRAM,0);	
	if(sock<0)
	{
		fprintf(stderr,"No puc obrir socket!!!\n");
		perror(argv[0]);
		exit(-1);
	}

	/*
	Bind del socket, procencia qualsevol IP
	*/
	memset(&addr_server,0,sizeof (struct sockaddr_in));
	addr_server.sin_family=AF_INET;
	addr_server.sin_addr.s_addr=htonl(INADDR_ANY);
	addr_server.sin_port=htons(port);
	if(bind(sock,(struct sockaddr *)&addr_server,sizeof(struct sockaddr_in))<0)
	{
		fprintf(stderr,"No puc fer el binding del socket!!!\n");
                perror(argv[0]);
                exit(-2);
	}	


	/* Deixem morir tranquilament els fills sense
	   que "zombiejin" per el sistema */
	signal(SIGCHLD,SIG_IGN);

	while(1)
	{
		laddr_cli=sizeof(struct sockaddr_in);
		/* Rebem la peticio de PRIMERS */
		a=recvfrom(sock,dadcli,LONGDADES,0,(struct sockaddr *)&addr_cli,&laddr_cli);
		if(a<0 && errno!=ECONNREFUSED)	/* ECONNREFUSED -> El client mor i els UDP reboten */
		{
			fprintf(stderr,"Error al recvfrom\n");
			perror(argv[0]);
			exit(-2);
		}
		if(fork()==0)
		{
			
			/* Proces fill, atendra la peticio */
			
			int nprimers,i=0;
			sscanf(dadcli,"PRIMERS %d ",&nprimers);
			if(nprimers>MAXPRIMERS) nprimers=MAXPRIMERS;
		
			/* Confirmem el nombre de primers que enviarem */
			sprintf(dadcli,"200 PRIMERS:%d\n",nprimers);
			a=sendto(sock,dadcli,strlen(dadcli)+1,0,(struct sockaddr*)&addr_cli,laddr_cli);
			if(a<0 && errno!=ECONNREFUSED)
			{
				fprintf(stderr,"Error al sendto\n");
				perror(argv[0]);
				exit(-2);
			}

			
			/* Anem enviant els primers */
			while(i<nprimers)
			{
				temps=time(NULL);
				sprintf(dadcli,"%d ",primers[i]);
				a=sendto(sock,dadcli,strlen(dadcli)+1,0,(struct sockaddr*)&addr_cli,laddr_cli);
				if(a<0 && errno!=ECONNREFUSED)
				{
					fprintf(stderr,"Error al sendto\n");
					perror(argv[0]);
					exit(-2);
				}
				sleep(1);
				i++;
			}
			exit(1);
			/* Mort del fill */
		}


	}
}
