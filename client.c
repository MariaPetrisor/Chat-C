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
#include<ctype.h>

#include "netio.h"

typedef struct client{
	char username[10];
	char password[10];
	int socket_fd;
	int valid;
} client_t;

client_t clients[100];

int main(int argc, char *argv[])
{
int socket_fd, server_port;
struct sockaddr_in remote_addr;
int nread;
char *server_address;
char username[10], password[10];
int i, k = 0, ok, count, len_pass, clients_no, n, n1, usernameLen;
char c;
char buff[1024], bufUsername[10], len[10], lenUsername[10], lengthUsername[10], lenPassword[10], len1[10], buf[15];

/* initilizarea bazei de date cu clienti */
strcpy(clients[0].username,"c1");
strcpy(clients[0].password,"1");
clients[0].valid=0;
strcpy(clients[1].username,"c2");
strcpy(clients[1].password,"2");
clients[1].valid=0;
strcpy(clients[2].username,"c3");
strcpy(clients[2].password,"3");
clients[2].valid=0;
clients_no=3;

if (argc != 3)
{
	printf("Usage: %s <ip> <server port>\n", argv[0]);
	exit(1);
}

server_address = argv[1];
server_port = atoi(argv[2]);

socket_fd = socket(PF_INET, SOCK_STREAM, 0);
if (socket_fd== -1)
{
	printf("Error: socket()\n");
	exit(1);
}

memset(&remote_addr, 0, sizeof(remote_addr));
set_addr(&remote_addr, server_address, 0, server_port);

inet_ntop(AF_INET, &(remote_addr.sin_addr), buf, sizeof(buf));
printf("Server address: %s, server port: %d\n",buf, ntohs(remote_addr.sin_port));	/* "10.0.2.15" = adresa IP a masinii virtuale, port 5678, la care asculta server-ul */

if (connect(socket_fd, (struct sockaddr *)&remote_addr, sizeof(remote_addr)) == -1)	/* clientul face cerere de conectare la server */
{
	printf("Error: connect()\n");
	exit(1);
}
printf("Client connecting to server\n");

printf("\nClient authentication\n");

ok = 0;
count = 3;
while (count != 0 && ok == 0)
{
	printf("Username(maximum 10 characters): ");
	fgets(username, sizeof(username), stdin);

	/* elimin \n din username-ul introdus de la tastatura */
	usernameLen = strlen(username);
	if (usernameLen > 1 && username[usernameLen-1] == '\n')
		username[usernameLen-1] = '\0';

	/* daca username-ul introdus de client se gaseste in baza de date,incrementez campul de valid (va avea valoarea 1) */
	for (i = 0; i < clients_no; i++)
	{
		if (strcmp(clients[i].username, username) == 0)	
		{
			k = i;
			clients[i].valid++;
			ok = 1;
		}
	}
	sprintf(lenUsername, "%d", usernameLen);
	if (ok != 1) 
		count--;
	
	/* trimit lungimea username-ului catre server */	
	else if (stream_write(socket_fd, (void *)lenUsername, 1) < 0)	
	{
		printf("Error sending username length through socket\n");
		exit(1);
	}

	/* trimit username-ul catre server */
	else if (stream_write(socket_fd, (void *)username, usernameLen) < 0)	
	{
		printf("Error sending username through socket\n");
		exit(1);
	}
}

if (ok == 0) 
{
	printf("Too many tries for entering username.\n");
	exit(1);
}
/* k = indicele din vectorul de clienti unde am gasit username-ul, pentru a verifica daca si parola de pe aceeasi pozitie coincide cu cea introdusa de client */
/* scriu in socket username-ul clientului care se va conecta; */

ok = 0;
count = 3;
while (count != 0 && ok == 0)
{
	printf("Password(maximum 10 characters): ");
	memset(password, 0, sizeof(password));
	fgets(password, sizeof(password), stdin);
	
	/* elimin \n din password */
	len_pass = strlen(password);
	if (len_pass > 1 && password[len_pass-1] == '\n')
	password[len_pass-1] = '\0';

	/* daca password-ul introdus de client se gaseste in baza de date,incrementez campul de valid (va avea valoarea 2) */
	if (strcmp(clients[k].password, password) == 0)
	{	
		clients[k].valid++;
		ok = 1;
	}
	sprintf(lenPassword, "%d", len_pass);	
	if (ok != 1) 
		count--;
	
	/* trimit lungimea password-ului catre server */
	else if(stream_write(socket_fd, (void *)lenPassword, 1) < 0)	
	{
		printf("Eroare la trimitere lungime password prin socket\n");
		exit(1);
	}

	/* trimit password-ul catre server */
	else if(stream_write(socket_fd, (void *)password, len_pass) < 0)	
	{
		printf("Eroare la trimitere password prin socket\n");
		exit(1);
	}
}

if (ok == 0) 
{
	printf("Ati incercat de prea multe ori sa introduceti parola. \n");
	exit(1);
}

/* daca la pozitia k(a username-ului introdus de client), parola coincide cu parola introdusa de noi, incrementam valid(va avea valoarea 2) */
/* valid=2 -> datele exista in baza de date si scriem in soclu mesajul introdus de client; daca datele nu coincid nu ma va lasa sa introduc niciun mesaj */

memset(buf, 0, sizeof(buf));
if (clients[k].valid == 2)
{
	while(1)
	{	
		printf("\nDoriti sa scrieti?D/N \n");
		scanf(" %c", &c);
		fflush(stdin);

		if (c == 'D')
		{
			ok = 0;
			while (!ok)
			{
				strcpy(buff, "");
				fgets(buff, sizeof(buff), stdin);
				n = strlen(buff);
				sprintf(len, "%d", n);
				
				if (strcmp(buff,"\n") == 0)
					continue;

				if (n <= 9)
				{
					len[1] = len[0];
					len[0] = '-';
				}
				
				sprintf(lengthUsername, "%d", usernameLen);
				if (stream_write(socket_fd, (void *)lengthUsername, 1) < 0)
				{
					printf("Error sending username length to socket\n");
					exit(1);
				}

				if (stream_write(socket_fd, (void *)username, usernameLen) < 0)
				{
					printf("Error sending username %s to socket\n", username);
					exit(1);
				}

				if (stream_write(socket_fd, (void *)len, 2) < 0)
				{
					printf("Error sending mesage length from %s to socket\n", username);
					exit(1);
				}
				
				if (stream_write(socket_fd, (void *)buff, n) < 0)
				{
					printf("Error sending mesage from %s to socket\n", username);
					exit(1);
				}

				if (strcmp(buff, ".\n") == 0)
				{
					c = 'N';
					ok = 1;
				}
			}
		}
		if (c == 'N')
		{	
			while(1)
			{
				strcpy(buff, "");
				strcpy(bufUsername, "");
				strcpy(lenUsername, "");

				/* citeste din socket lungimea username-ului clientului care a trimis mesajul */
				if ((nread = stream_read(socket_fd, (void *) lenUsername, 1)) > 0)
				{
					n1 = atoi(lenUsername);
				}

				/* citeste din socket username-ul clientului care a trimis mesajul */
				if ((nread = stream_read(socket_fd, (void *) bufUsername, n1)) < 0)
				{
					printf("Error receiving username from client.\n");
				}

				if ((nread = stream_read(socket_fd, (void *) len1, 2)) > 0)
				{
					if (len1[0] == '-')
						strcpy(len1, len1+1);				
					n = atoi(len1);	
				}

				if ((nread = stream_read(socket_fd, (void *) buff, n)) > 0)
				{	
					strncpy(buff, buff, n);
					buff[n] = '\0';
					if (strcmp(buff, ".\n") == 0)
						break;
					else
					{											
						printf("%s : %s", bufUsername, buff);
					}
				}
			}					
		}
		else if (c != 'D' && c != 'N')
		{
			printf("Command doesn't exist. Try again(D\N)\n");
			continue;
		}
	}
}
close(socket_fd);
return 0;
}


