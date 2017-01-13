#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>		
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include<pthread.h> 

#include "netio.h"

#define BACKLOG 5

typedef struct client{
	char username[10];
	char password[10];
	int socket_fd;
} client_t;

client_t clients[100];	/* clients connected */
int client_no;

struct sockaddr_in local_addr, remote_addr;
int socket_fd; 	/* descriptoul de fisier asociat socketului creat, pt a il putea apela din functia connection_handler */
          
/* functia thread, trimitere, receptionare de mesaje si setarea adresei clientului */
void *connection_handler(void * socket)
{
	int connection_fd;
	connection_fd = *(int *)socket;
	int nread, nwrite;
	int k;
	char buf[1024];
    	char username[10], password[10], usernameReceived[10];
	int n, n1, n2, usernameLength, passwordLength;
	char len[10], lenUsername[10], lengthUsername[10], lenPassword[10];

	clients[client_no].socket_fd = connection_fd;	
	strcpy(clients[client_no].username, "");	
	strcpy(clients[client_no].password, "");	
        
	/* setam adresa clientului si portul(random) clientului(vazute de server) */
	set_addr(&remote_addr, NULL, INADDR_ANY, 0);
	inet_ntop(AF_INET, &(remote_addr.sin_addr), buf, sizeof(buf)); 
	//printf("Client address: %s, client port: %d\n", buf, ntohs(remote_addr.sin_port));	/* "0.0.0.0" , port 0*/
		
	/* server-ul primeste de la client username-ul */
	memset(username, 0, sizeof(username));

	if ((nread = stream_read(connection_fd, (void *) lengthUsername, 1)) > 0)	
	{
		n2 = atoi(lengthUsername);	
	}

	if ((nread = stream_read(connection_fd, (void *) username, n2)) > 0)	
	{
		printf("\n%s connected to server...\n", username); 
		strcpy(clients[client_no].username, username);	
	}

	/* server-ul primeste de la client password-ul */
	memset(password, 0, sizeof(password));
	if ((nread = stream_read(connection_fd, (void *) lenPassword, 1)) > 0)	
	{
		passwordLength = atoi(lenPassword);			
	}

	if ((nread = stream_read(connection_fd, (void *) password, passwordLength)) > 0)	
	{
		printf("Password accepted:  %s\n", password); 
		strcpy(clients[client_no].password, password); 			
	}
	
 	client_no++;
	
	/* serverul primeste de la client mesajul */
	while(1)
	{
		/* lungimea username-ului mesajului, pe 1 octet */
		if ((nread = stream_read(connection_fd, (void *) lenUsername, 1)) > 0)
		{			
			n1 = atoi(lenUsername);		
		}

		/* username-ul */
		if ((nread = stream_read(connection_fd, (void *) usernameReceived, n1)) < 0)	
		{	
			printf("Error receiving username\n");			
		}

		/* lungimea mesajului, pe 2 octeti */
		if ((nread = stream_read(connection_fd, (void *) len, 2)) > 0)	
		{	
	 		if(len[0] == '-')
			    strcpy(len, len+1);	
			n=atoi(len);		
		}

		/* corpul mesajului, pe n octeti */
		if ((nread = stream_read(connection_fd, (void *) buf, n)) > 0)
		{
			buf[n]='\0';	
			if (strcmp(buf, "exit\n") == 0)
			{
				printf("Client %s left chat.\n", username);
				pthread_exit(NULL);
			}

			if (strcmp(buf, ".\n") != 0)			
				printf("%s: %s", usernameReceived, buf); 		
		}

		/* trimit inapoi tuturor clientilor conectati la server */
		if (n <= 9)
		{
			len[1]=len[0];
			len[0]='-';
		}

		for (k = 0; k < client_no; k++)
		{			
			usernameLength = strlen(usernameReceived);
			strcpy(lengthUsername, "");
			sprintf(lengthUsername, "%d", usernameLength);

			/* trimit lungimea username-ului clientului */		
			if ((nwrite = stream_write(clients[k].socket_fd, (void *) lengthUsername, 1)) < 0)
			{
				printf("Error writing username length %s: %s to socket\n", username, lengthUsername);
			}

			/* trimit username-ul clientului */					
			if ((nwrite = stream_write(clients[k].socket_fd, (void *) usernameReceived, usernameLength)) < 0)
			{
				printf("Error writing username %s to socket\n", usernameReceived);
			}

			/* trimit lungimea mesajului */
			if ((nwrite = stream_write(clients[k].socket_fd, (void *) len, 2)) < 0)
			{ 
				printf("Error writing message length %s to socket\n", len);
			}

			/* trimit corpul mesajului */		
			if ((nwrite = stream_write(clients[k].socket_fd, (void *) buf, n)) < 0)
			{
				printf("Error writing message %s to socket\n", buf);
			}
		}		
	}
	free(socket);
}

int main(int argc, char *argv[])
{
int  connection_fd, server_port;
socklen_t socket_len;
socklen_t remote_len;
char buf[1024]; 
pthread_t thread_id;

if (argc != 2)
{
	printf("Usage: %s <server port>\n", argv[0]);
	exit(2);
}

server_port = atoi(argv[1]);

socket_fd = socket(PF_INET, SOCK_STREAM, 0);
if (socket_fd == -1)
{
	printf("Eroare la crearea socket-ului\n");
	exit(1);
}

memset(&local_addr, 0, sizeof(local_addr));
inet_ntop(AF_INET, &(local_addr.sin_addr), buf, sizeof(buf)); 

set_addr(&local_addr, NULL, INADDR_ANY, server_port);	/* serverul se leaga la adresa locala 0.0.0.0(INADDR_ANY): poate receptiona pachete si conexiuni de retea sosite catre oricare din adresele statiei si foloseste portul 5678*/
strcpy(buf,"");
inet_ntop(AF_INET, &(local_addr.sin_addr), buf, sizeof(buf)); 
printf("Server address: %s, server port: %d\n", buf, ntohs(local_addr.sin_port));	/* "0.0.0.0" , port 5678*/

if (bind(socket_fd, (struct sockaddr *)&local_addr, sizeof(local_addr)) == -1)  /* leaga socket-ul de local_addr */
{
	printf("Eroare la bind\n");
	exit(1);
}

if (listen(socket_fd, BACKLOG) == -1)	/* serverul asteapta conexiuni de la clienti, nr maxim de conexiuni in coada de asteptare = 5 */
{
	printf("Eroare la listen\n");
	exit(1);
}
printf("Server listening for incoming connections requests...\n");

while(1)
{
	/* remote_addr = adresa clientului */
	remote_len = sizeof(remote_addr);	
	/* extrage primul request din coada de asteptare. Se creaza un soclu nou pt fiecare conexiune acceptata */

	while(connection_fd = accept(socket_fd, (struct sockaddr *)&remote_addr, &remote_len))
	{
		if (connection_fd == -1)
		{
			printf("Eroare la accept\n");
			exit(1);
		}

	   	 if( pthread_create( &thread_id , NULL ,  connection_handler , (void*) &connection_fd) < 0)
		{
		    perror("Could not create thread");
		    return 1;
		}	
	}
}
return 0;
}

