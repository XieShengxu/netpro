/*
 * cli.h
 *
 *  Created on: Jun 1, 2018
 *      Author: joxie
 */

#ifndef CLI_H_
#define CLI_H_

#include	<setjmp.h>    /* siglongjmp */
#include	<signal.h>
#include	<netinet/in.h>	/* sockaddr_in{} and other Internet defns */
#include	<stdio.h>
#include	<string.h>
#include	<stdarg.h>		/* ANSI C header file */
#include	<syslog.h>		/* for syslog() */
#include	<stdlib.h>
#include	<errno.h>
#include	<sys/time.h>
#include	<arpa/inet.h>	/* inet(3) functions */
#include 	<pthread.h>
#include	<sys/un.h>



#define	SA	struct sockaddr/* Miscellaneous constants */
#define	MAXLINE		4096	/* max text line length */
#define WNDSIZE		20		/*window size for client and server*/
#define	SERV_PORT	9877			/* TCP and UDP */
#define	RTT_RXTMIN      2	/* min retransmit timeout value, in seconds */
#define	RTT_RXTMAX     	60	/* max retransmit timeout value, in seconds */
#define	RTT_MAXNREXMT 	3	/* max # times to retransmit */

typedef	void	Sigfunc(int);	/* for signal handlers */

static struct hdr {
  uint32_t	seq;	/* sequence # */
  uint32_t	ts;		/* timestamp when sent */
  uint8_t	wnd;	/*for windows*/
  size_t	msglen;	/*for message length*/
};

struct rtt_info {
  float		rtt_rtt;	/* most recent measured RTT, in seconds */
  float		rtt_srtt;	/* smoothed RTT estimator, in seconds */
  float		rtt_rttvar;	/* smoothed mean deviation, in seconds */
  float		rtt_rto;	/* current RTO to use, in seconds */
  int		rtt_nrexmt;	/* # times retransmitted: 0, 1, 2, ... */
  uint32_t	rtt_base;	/* # sec since 1/1/1970 at start */
};

void dg_cli(FILE *fp, int sockfd, const SA *pservaddr, socklen_t servlen);

ssize_t	  Send_recv(int, struct hdr *, const void *[], ssize_t *, struct hdr *, void *, size_t, const SA *, socklen_t, uint8_t);
char	  *Fgets(char *ptr, int n, FILE *stream);
void	  Fputs(const char *ptr, FILE *stream);
Sigfunc	  *Signal(int signo, Sigfunc *func);
void	  Sendmsg(int, const struct msghdr *, int);
ssize_t	  Recvmsg(int, struct msghdr *, int);
void	  Gettimeofday(struct timeval *, void *);
int		  Socket(int, int, int);
void	  Inet_pton(int, const char *, void *);

void	err_dump(const char *, ...);
void	err_msg(const char *, ...);
void	err_quit(const char *, ...);
void	err_ret(const char *, ...);
void	err_sys(const char *, ...);



/* function prototypes */
void	rtt_debug(struct rtt_info *);
void	rtt_init(struct rtt_info *);
void	rtt_newpack(struct rtt_info *);
int		rtt_start(struct rtt_info *);
void	rtt_stop(struct rtt_info *, uint32_t);
int		rtt_timeout(struct rtt_info *);
uint32_t rtt_ts(struct rtt_info *);
extern int	rtt_d_flag;	/* can be set to nonzero for addl info */

#endif /* CLI_H_ */
