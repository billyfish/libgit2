#include "../posix.h"

#include <utime.h>

#include <proto/dos.h>
#include <proto/socket.h>



struct hostent *gethostbyname(const char *name)
{
	return ISocket->gethostbyname ((STRPTR) name);
}

struct servent *getservbyname(const char *name, const char *proto)
{
	return ISocket->getservbyname ((STRPTR) name, (STRPTR) proto);
}


int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
	return ISocket->connect (sockfd, (struct sockaddr *) addr, addrlen);
}


ssize_t p_send(int sockfd, const void *buf, size_t len, int flags)
{
	return ISocket->send (sockfd, (APTR) buf, len, flags);
}


ssize_t p_recv(int sockfd, void *buf, size_t len, int flags)
{
	return ISocket->recv (sockfd, buf, len, flags);
}


int p_inet_pton(int af, const char *src, void *dst)
{
	return ISocket->inet_pton (af, (char *) src, dst);
}


char *strndup (const char *src, size_t n)
{
	size_t len = strlen (src);
	char *dst = NULL;

	if (len > n) {
	len = n;
	}

	dst = (char *) malloc (len + 1);

	if (dst != NULL) {
		memcpy (dst, src, len);
		* (dst + len) = '\0';
	}

	return dst;
}



int socket(int domain, int type, int protocol)
{
	return ISocket->socket (domain, type, protocol);
}




/*
 * Currently ignore the microseconds
 */
int p_utimes(const char *filename, const struct timeval times[])
{
	struct utimbuf t;

	t.actime = times [0].tv_sec;
	t.modtime = times [1].tv_sec;

	return utime (filename, &t);
}

