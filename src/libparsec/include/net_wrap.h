/*
 * PARSEC HEADER: net_wrap.h
 */

#ifndef _NET_UNP_H_
#define _NET_UNP_H_

#if defined( SYSTEM_TARGET_WINDOWS )
	#include <winsock2.h>
	#include <ws2tcpip.h>
	#include <io.h>
#else
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <sys/time.h>
	#include <time.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <errno.h>
	#include <fcntl.h>
	#include <netdb.h>
	#include <signal.h>
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	#include <sys/stat.h>
	#include <sys/uio.h>
	#include <unistd.h>
	#include <sys/wait.h>
	#include <net/if.h>
	#include <sys/un.h>
	#include <sys/ioctl.h>
#endif
/*
#elif defined( SYSTEM_MACOSX_UNUSED )
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <sys/time.h>
	#include <time.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <errno.h>
	#include <fcntl.h>
	#include <netdb.h>
	#include <signal.h>
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	#include <sys/stat.h>
	#include <sys/uio.h>
	#include <unistd.h>
	#include <sys/wait.h>
	#include <net/if.h>
	#include <sys/un.h>
	#include <sys/sockio.h>
	#include <sys/ioctl.h>

	#define socklen_t int
#endif
*/

// constants and macros used by the network code ------------------------------
//
#define	MAXLINE	4096

#define	SA	struct sockaddr


// windows uses closesocket, osx/linux use close
#ifdef SYSTEM_TARGET_WINDOWS
#define CLOSESOCKET closesocket
#else
#define CLOSESOCKET close
#endif


// define some macros to make error handling more readable --------------------
//
#ifdef SYSTEM_TARGET_WINDOWS
	#define ERRNO				wsaerr
	#define FETCH_ERRNO()		int wsaerr = WSAGetLastError()
	#define ERRNO_EWOULDBLOCK	( wsaerr == WSAEWOULDBLOCK  )
	#define ERRNO_ECONNREFUSED	( wsaerr == WSAECONNREFUSED )
	#define ERRNO_ENETDOWN		( wsaerr == WSAENETDOWN     )
	#define ERRNO_ECONNRESET	( wsaerr == WSAECONNRESET   )
	//#define ERRNO_WSAEMSGSIZE	( wsaerr == WSAEMSGSIZE     )

	// UNP legacy stuff
	#define ioctl( fd, request, arg )	ioctlsocket(fd, request, (unsigned long *) arg)
	#define	bzero(ptr,n)				memset(ptr, 0, n)

	int			inet_aton(const char *cp, struct in_addr *ap);
	const char*	hstrerror(int);
	const char* inet_ntop( int family, const void* addrptr, char* strptr, size_t len);

#else
	#define ERRNO				errno
	#define FETCH_ERRNO()		{}
	#define ERRNO_EWOULDBLOCK	( errno == EWOULDBLOCK  )
	#define ERRNO_ECONNREFUSED	( errno == ECONNREFUSED )
	#define ERRNO_ECONNRESET	( errno == ECONNRESET   )
	#define ERRNO_ENETDOWN		( FALSE )
#endif


#ifndef MAXHOSTNAMELEN
	#define MAXHOSTNAMELEN 1024
#endif


#endif // _NET_UNP_H_


