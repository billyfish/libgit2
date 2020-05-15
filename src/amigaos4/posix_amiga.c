#include "../posix.h"

#include <proto/dos.h>
#include <proto/socket.h>





struct hostent *gethostbyname(const char *name)
{
	return ISocket->gethostbyname (name);
}



int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
	return ISocket->connect (sockfd, (struct sockaddr *) addr, addrlen);
}

