#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <string.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>




#define LONGDADES	100

/*
	Principal
*/

int main(int argc,char *argv[])
{
        struct hostent *ent;
	int 			sock,port,laddr_cli,a,nprimers,i=0;
	struct sockaddr_in	addr_server,addr_cli;
	char			dadcli[LONGDADES];
	time_t			temps;



	if(argc!=4)
	{
		printf("Usar: \n");
		printf("\t%s <destinacio> <port> <nprimers>\n",argv[0]);
		exit(0);
	}

	port=atoi(argv[2]);		/* Recollim el port */
	nprimers=atoi(argv[3]);		/* Recollim el nombre de primers*/
	
	/* Busquem el host */
        ent=gethostbyname(argv[1]);
        if(!ent)
        {
                printf("Error! No trobat: %s \n",argv[1]);
                exit(-1);
        }

	/* Obrim un socket UDP */
	sock=socket(AF_INET,SOCK_DGRAM,0);	
	if(sock<0)
	{
		fprintf(stderr,"No puc obrir socket!!!\n");
		perror(argv[0]);
		exit(-1);
	}

	/* Addreça del bind del client */
	memset(&addr_cli,0,sizeof (struct sockaddr_in));
	addr_cli.sin_family=AF_INET;
	addr_cli.sin_addr.s_addr=htonl(INADDR_ANY);
	addr_cli.sin_port=htons(0);
	
	if(bind(sock,(struct sockaddr *)&addr_cli,sizeof(struct sockaddr_in))<0)
	{
		fprintf(stderr,"No puc fer el binding del socket!!!\n");
                perror(argv[0]);
                exit(-2);
	}	


	/* Adreça del servidor */
	memset(&addr_server,0,sizeof (struct sockaddr_in));
	addr_server.sin_family=AF_INET;
	addr_server.sin_addr.s_addr=(((struct in_addr *)ent->h_addr)->s_addr);
	addr_server.sin_port=htons(port);
	
	/* Enviem el missatge PRIMERS <n> on <n> es el nombre de primers que
	   volem enviar */
	sprintf(dadcli,"PRIMERS %d\n",nprimers);
	a=sendto(sock,dadcli,strlen(dadcli)+1,0,(struct sockaddr*)&addr_server,sizeof(addr_server));
        if(a<0)
          {
              fprintf(stderr,"Error al sendto\n");
              perror(argv[0]);
              exit(-2);
          }


	 /* Rebem la confirmació per part del server, amb el nombre de primers que 
	    ens enviara realment (hi ha un maxim) */
         a=recvfrom(sock,dadcli,LONGDADES,0,(struct sockaddr *)&addr_cli,&laddr_cli);
         if(a<0)
         {
              fprintf(stderr,"Error al recvfrom\n");
              perror(argv[0]);
              exit(-2);
         }
         sscanf(dadcli,"200 PRIMERS:%d ",&nprimers);

	/* Anem rebent els primers */
	i=0;
	while(i<nprimers)
	{
		/* Rebem nombre */
		a=recvfrom(sock,dadcli,LONGDADES,0,(struct sockaddr *)0,(int *)0);
		if(a<0)
		{
			fprintf(stderr,"Error al recvfrom\n");
			perror(argv[0]);
			exit(-2);
		}
		dadcli[a]='\0';	
		/* Imprimim el nombre */
		printf("%s\n",dadcli);
		i++;
	}
}
