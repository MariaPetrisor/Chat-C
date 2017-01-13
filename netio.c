#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>	
#include <stdint.h>	
#include <netinet/in.h> 	
#include <string.h>

#include "netio.h"	

int set_addr(struct sockaddr_in *s, char *name, uint32_t socket_address, short socket_port)
{
	struct hostent *h;

	memset((void *)s, 0, sizeof(*s));
	s->sin_family = AF_INET;
	if (name != NULL)
	{
		h = gethostbyname(name);
		if (h == NULL)
		{
			printf("Eroare la gethostbyname\n");
			exit(1);
		}
		s->sin_addr.s_addr = *(uint32_t *)h->h_addr_list[0];
	}
	else
		s->sin_addr.s_addr = htonl(socket_address);	/* setam adresa IP a socketului, convertind din host byte order in network 									byte order */
	s->sin_port = htons(socket_port);		/* setam portul socketului, convertind */
return 0;
}

/* daca numarul de octeti ce trb cititi/scrisi > limita lui read/write */
int stream_read(int socket_fd, char *buf, int len)
{
	int nread;
	int remaining = len;

	while (remaining > 0)
	{
		if ((nread = read(socket_fd, buf, remaining)) == -1)
			return -1;
		if (nread == 0)		/* nu mai am ce citi */
			break;
		remaining -= nread;
		buf += nread;	//????
	}
	return len - remaining;		/* returneaza len, pt ca remaining a ajuns la 0 */
}

int stream_write(int socket_fd, char *buf, int len)
{
	int nwrite;
	int remaining = len;

	while (remaining > 0)
	{
		if ((nwrite = write(socket_fd, buf, remaining)) == -1)
			return -1;
		remaining -= nwrite;
		buf += nwrite;	
	}
	return len - remaining;
}

