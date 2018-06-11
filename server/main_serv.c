/*
 * main_serv.c
 *
 *  Created on: May 31, 2018
 *      Author: joxie
 */

#include "/home/joxie/UNP/networkExperiment/server/srv.h"



void	mydg_echo(int, SA *, socklen_t, SA *, FILE *fp);

static struct hdr {
  uint32_t	seq;	/* sequence # */
  uint32_t	ts;		/* timestamp when sent */
  uint8_t	wnd;	/*for windows*/
  size_t	msglen;	/*for message length*/
}recvhdr;
static struct msghdr msgrecv;
struct iovec	iovrecv[2];

int
main(int argc, char **argv)
{
	char				doc[100];
	int					sockfd;
	pid_t				pid;
	struct sockaddr_in	cliaddr, servaddr;

	sockfd = Socket(AF_INET, SOCK_DGRAM, 0);
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(SERV_PORT);
	Bind(sockfd, (SA *) &servaddr, sizeof(servaddr));
	printf("bound %s\n", Sock_ntop((SA *) &servaddr, sizeof(servaddr)));

	printf("please input the document name:\n");
	scanf("%s",doc);
	FILE *fp = NULL;
	fp = fopen(doc, "wb+");

	if ( (pid = Fork()) == 0) {		/* child */
		mydg_echo(sockfd, (SA *) &cliaddr, sizeof(cliaddr), (SA *) &servaddr, fp);
		exit(0);		/* never executed */
	}
	exit(0);
}
void
mydg_echo(int sockfd, SA *pcliaddr, socklen_t clilen, SA *myaddr, FILE *fp)
{
	char		inbuff[MAXLINE+1];
	size_t		msglen = 0;
	uint32_t	seq=0;
	msgrecv.msg_name = pcliaddr;
	msgrecv.msg_namelen = clilen;
	msgrecv.msg_iov = iovrecv;
	msgrecv.msg_iovlen = 2;
	iovrecv[0].iov_base = &recvhdr;
	iovrecv[0].iov_len = sizeof(struct hdr);
	iovrecv[1].iov_base = inbuff;
	iovrecv[1].iov_len = MAXLINE;
	int i = 1;
	for ( ; ; ) {
		Recvmsg(sockfd, &msgrecv, 0);
		msglen += recvhdr.msglen;
		printf("\nchild %d, datagram from %s to ", getpid(), Sock_ntop(msgrecv.msg_name, msgrecv.msg_namelen));
		printf("%s : \n", Sock_ntop(myaddr, sizeof(myaddr)));//printf function use the order of right to left
		printf("\tsequence is : %d\n",recvhdr.seq);
		printf("\tpeer window is : %d\n",recvhdr.wnd);
		printf("\tlength is: %d\n",recvhdr.msglen);
		printf("\tsequence of the wnd : %d\n", i);
		if(recvhdr.seq > seq){
			fwrite(inbuff, 1, recvhdr.msglen, fp);
			fseek(fp, 0, SEEK_CUR);

			bzero(&inbuff, sizeof(inbuff));
			if(i >= recvhdr.wnd){
				i = 1;
				recvhdr.wnd = rand()%WNDSIZE+1;
				recvhdr.msglen = 0;
				Sendmsg(sockfd, &msgrecv, 0);
			}else{
				i++;
			}
		}
		else {
			i=1;
			fseek(fp, -(msglen-recvhdr.msglen), SEEK_END);
			fwrite(inbuff, 1, recvhdr.msglen, fp);
			fseek(fp, 0, SEEK_CUR);
			bzero(&inbuff, sizeof(inbuff));
			msglen = recvhdr.msglen;
		}
		seq = recvhdr.seq;
	}
}
