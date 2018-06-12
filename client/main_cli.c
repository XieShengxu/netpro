/*
 * main_cli.c
 *
 *  Created on: Jun 1, 2018
 *      Author: joxie
 */

#include "./cli.h"

int
main(int argc, char **argv)
{
	int					sockfd;
	struct sockaddr_in	servaddr;
	char				doc[100];

	if (argc != 2)
		err_quit("usage: udpcli <IPaddress>");

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(SERV_PORT);//port is definded 8977
	Inet_pton(AF_INET, argv[1], &servaddr.sin_addr);
	sockfd = Socket(AF_INET, SOCK_DGRAM, 0);
	FILE *fp = NULL;
	printf("please input the document path and name:\n");
	scanf("%s",doc);

	fp = fopen(doc, "rb+");
	dg_cli(fp, sockfd, (SA *) &servaddr, sizeof(servaddr));
	exit(0);
}


void
dg_cli(FILE *fp, int sockfd, const SA *pservaddr, socklen_t servlen)
{
	char	wndmsg[WNDSIZE][MAXLINE], recvline[MAXLINE + 1];
	char	*msgp[WNDSIZE];
	ssize_t		msglen[WNDSIZE];
	static struct hdr  sendhdr, recvhdr;
	sendhdr.wnd = 1;
	recvhdr.wnd = 1;
	int i;
	int bool = 1;
	while (bool){
		for(i = 0; i<recvhdr.wnd; i++){
			if(msglen[i]=fread(wndmsg[i], 1, MAXLINE, fp)){
				msgp[i]=wndmsg[i];
			}
			else{
				bool = 0;
				break;
			}
		}
		sendhdr.wnd = i;
		Send_recv(sockfd, (struct hdr*) &sendhdr, msgp, msglen,
			(struct hdr*) &recvhdr, recvline, MAXLINE, pservaddr, servlen, i);
		printf("\tpeer window is : %d\n", recvhdr.wnd);
		bzero(&wndmsg, sizeof(wndmsg));
		bzero(&msglen, sizeof(msglen));
	}
	fclose(fp);
}
