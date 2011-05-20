#include "socket.h"

#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <netdb.h>

#define MAXSIZE 512

static int connectUrl(const char *url);
static int sendMessage(sockfd, const char *content);

int httpDownload(const char *url, const char *target)
{
	int sockfd = connectUrl(url);
	if (sockfd == -1) return -1;
	
	sendMessage(sockfd, "");
	
}

static int connectUrl(const char *url)
{
	int sockfd;
	struct sockaddr_in addr;
	struct hostent *host;

	if ((host = gethostbyname(url)) == NULL) {
		printf("url does not exists\n");
		return -1;
	}

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		printf("socket error\n");
		return -1;
	}

	addr.sin_family = AF_INET;
	addr.sin_port = htons(8888);
	addr.sin_addr = *((struct in_addr*)host->h_addr);
	bzero(&(addr.sin_zero), 8);
	
	if (connect(sockfd, (struct sockaddr*)&addr, sizeof(struct sockaddr)) == -1) {
		printf("connect error\n");
		return -1;
	}

	return sockfd;
}

static int sendMessage(int sockfd, const char *content)
{
	if (send(sockfd, content, strlen(content), 0) == -1) {
		printf("send \"%s\" eror\n", content);
		return -1;
	}
	
	return 0;
}
