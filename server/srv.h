/*
 * srv.h
 *
 *  Created on: May 31, 2018
 *      Author: joxie
 */

#ifndef SRV_H_
#define SRV_H_

#include	<net/if.h>
#include	<string.h>
# include	<strings.h>		/* for convenience */
#include	<netinet/in.h>	/* sockaddr_in{} and other Internet defns */
#include	<arpa/inet.h>	/* inet(3) functions */
# include	<pthread.h>
#include	<errno.h>
#include	<stdarg.h>		/* ANSI C header file */
#include	<syslog.h>		/* for syslog() */
#include    <stdio.h>
#include	<unistd.h>
#include	<sys/un.h>
#include	<stdlib.h>


#define	SERV_PORT		 9877			/* TCP and UDP */
#define	MAXLINE		4096	/* max text line length */
#define WNDSIZE	20		/*window size for client and server*/
#define	SA	struct sockaddr

int 	Socket(int family, int type, int protocol);
void 	Setsockopt(int fd, int level, int optname, const void *optval, socklen_t optlen);
void 	Bind(int fd, const struct sockaddr *sa, socklen_t salen);
char 	*Sock_ntop(const struct sockaddr *sa, socklen_t salen);
pid_t	Fork(void);
void 	Close(int fd);
void	Sendmsg(int, const struct msghdr *, int);
ssize_t	Recvmsg(int, struct msghdr *, int);

void	 err_dump(const char *, ...);
void	 err_msg(const char *, ...);
void	 err_quit(const char *, ...);
void	 err_ret(const char *, ...);
void	 err_sys(const char *, ...);

#endif /* SRV_H_ */
