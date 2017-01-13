#ifndef _NETIO_H
#define _NETIO_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

extern int set_addr(struct sockaddr_in *, char *, uint32_t, short);
extern int stream_read(int, char *, int);
extern int stream_write(int , char *, int);

#endif
