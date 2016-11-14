/*
Authors: Francisco Castro, Antonio Umali
CS 513 Project 1 - Chat Roulette
Last modified: 28 Sept 2015

This is a test file.
*/

#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>

struct in_addr
{
	uint32_t s_addr;
};

struct sockaddr
{
	unsigned short sa_family;
	char sa_data[14];	
};

struct sockaddr_in
{
	short int sin_family;
	unsigned short int sin_port;
	struct in_addr	sin_addr;
	unsigned char sin_zero[8];	//set to all zeros using memset
};

struct addrinfo
{
	int ai_flags;
	int ai_family;
	int ai_socketype;
	int ai_protocol;
	size_t ai_addrlen;
	struct sockaddr *ai_addr;
	char *ai_canonname;
	struct addrinfo *ai_next;
};


struct sockaddr_storage
{
	sa_familty_t ss_family;

	char __sspad1[_SS_PAD1SIZE];
	int64_t __ss_align;
	char __ss_pad2[_SS_PAD2SIZE];
};

int getaddrinfo(const char *node, const char *service,
				const struct addrinfo *hints, struct addrinfo **res);

struct sockaddr_in sa;

int main()
{
	char ip4[INET_ADDSTRLEN];
	struct sockaddr_in sa;

	inet_ntop(AF_INET, &(sa.sin_addr), ip4, INET_ADDSTRLEN);

	printf("The IPv4 address is: %s\n", ip4);

	//connect to port 3490
	int status;
	struct addrinfo hints;
	struct addrinfo *serverinfo; //will point to the results

	memset(&hints, 0, sizeof hints); //struct should be empty

	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	if ((status = getaddrinfo(NULL, "3490", &hints, &serivnfo)) != 0 )
	{
		fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
		exit(1);
	}
	//servinfo now points to a linked list of 1 or more struct addrinfos

	//connect to specific host 
	status = getaddrinfo("www.example.net", "3490", &hints, &servinfo)

	return 0;
}
