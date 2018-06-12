/*
 * send_recv.c
 *
 *  Created on: Jun 3, 2018
 *      Author: joxie
 */

#include "./cli.h"


static struct rtt_info   rttinfo;
static int	rttinit = 0;
static struct msghdr	msgsend, msgrecv;	/* assumed init to 0 */
static void	sig_alrm(int signo);
static sigjmp_buf	jmpbuf;

ssize_t
send_recv(int fd, struct hdr *sendhdr, const void *outbuff[], ssize_t *outbytes,
		struct hdr *recvhdr, void *inbuff, size_t inbytes,
			 const SA *destaddr, socklen_t destlen, uint8_t m)
{
	ssize_t			n;
	uint32_t		recordseq;
	struct iovec	iovsend[2], iovrecv[2];


	if (rttinit == 0) {
		rtt_init(&rttinfo);		/* first time we're called */
		rttinit = 1;
		rtt_d_flag = 1;
	}

	msgsend.msg_name = destaddr;
	msgsend.msg_namelen = destlen;
	msgsend.msg_iov = iovsend;
	msgsend.msg_iovlen = 2;
	iovsend[0].iov_base = sendhdr;
	iovsend[0].iov_len = sizeof(struct hdr);

	msgrecv.msg_name = NULL;
	msgrecv.msg_namelen = 0;
	msgrecv.msg_iov = iovrecv;
	msgrecv.msg_iovlen = 2;
	iovrecv[0].iov_base = recvhdr;
	iovrecv[0].iov_len = sizeof(struct hdr);
	iovrecv[1].iov_base = inbuff;
	iovrecv[1].iov_len = inbytes;

	Signal(SIGALRM, sig_alrm);
	rtt_newpack(&rttinfo);		/* initialize for this packet */
	recordseq = sendhdr->seq;

sendagain:
	sendhdr->seq = recordseq;
	for (int i=0;i<m;i++){
		iovsend[1].iov_base = outbuff[i];
		iovsend[1].iov_len = outbytes[i];
		sendhdr->msglen = outbytes[i];
		sendhdr->seq++;
		if(i == m-1)
			sendhdr->ts = rtt_ts(&rttinfo);
		Sendmsg(fd, &msgsend, 0);
		printf("\tsend sequence is : %d\n", sendhdr->seq);
		printf("\tlength is: %d\n",iovsend[1].iov_len);
	}

	alarm(rtt_start(&rttinfo));	/* calc timeout value & start timer */

	rtt_debug(&rttinfo);

	if (sigsetjmp(jmpbuf, 1) != 0) {
		if (rtt_timeout(&rttinfo) < 0) {
			err_msg("\tsend_recv: no response from server, giving up");
			rttinit = 0;	/* reinit in case we're called again */
			errno = ETIMEDOUT;
			return(-1);
		}
		err_msg("\tsend_recv: timeout, retransmitting");
		goto sendagain;
	}

	do {
		n = Recvmsg(fd, &msgrecv, 0);
	} while (n < sizeof(struct hdr) || recvhdr->seq != sendhdr->seq);

	alarm(0);			/* stop SIGALRM timer */
		/* 4calculate & store new RTT estimator values */
	rtt_stop(&rttinfo, rtt_ts(&rttinfo) - recvhdr->ts);

	return(n - sizeof(struct hdr));	/* return size of received datagram */
}

static void
sig_alrm(int signo)
{
	siglongjmp(jmpbuf, 1);
}

ssize_t
Send_recv(int fd, struct hdr *sendhdr, const void *outbuff[], ssize_t *outbytes, struct hdr *recvhdr, void *inbuff, size_t inbytes, const SA *destaddr, socklen_t destlen, uint8_t m)
{
	ssize_t	n;

	n = send_recv(fd, sendhdr, outbuff, outbytes, recvhdr, inbuff, inbytes,
					 destaddr, destlen, m);
	if (n < 0)
		err_quit("dg_send_recv error");

	return(n);
}
