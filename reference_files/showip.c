/*
Authors: Francisco Castro, Antonio Umali
CS 513 Project 1 - Chat Roulette
Last modified: 28 Sept 2015

This is ...
*/

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define MYPORT "3490" // the port users will be connecting to test
#define BACKLOG 10 

// pass hints and res as args?
void listen_to_connections(int myport)
{
	// wait for incoming connections and handle them
	struct sockaddr_storage their_addr;
	socklen_t addr_size;
	struct addrinfo hints, *res;
	int sockfd, new_fd;

	// load up structs with getaddrinfo();
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(NULL, myport, &hints, &res);

	// make socket, bind it, and listen on it
	sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	int b = bind(sockfd, res->ai_addr, res->ai_addrlen);
	int l = listen(sockfd, BACKLOG);

	// accept incoming connection
	addr_size = sizeof their_addr;
	new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &addr_size);

	// ready to communicate on socket descriptor new_fd
}

bool send_data(int sockfd, char *msg)
{
	int len, bytes_sent;
	len = strlen(msg);

	bytes_sent = send(sockfd, msg, len, 0);

	// add sendto() function later
	return (bytes_sent == len);
}

}
int recieve_data(int sockfd, void *buffer, int len)
{
	int flags = 0;

	// add recvfrm() function later
	return recieve(sockfd, buffer, len, flags);
}

int main(int argc, char *argv[])
{
	struct addrinfo hints, *res, *p;
	int status;
	char ipstr[INET6_ADDSTRLEN];

	if (argc != 2)
	{
		fprintf(stderr, "usage: showip hostname\n", );
		return 1;
	}

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC; // AF_INET or AF_INET6 to force version
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // bind to IP of host it's running on

	// getaddrinfo fills out linked list point to by res
	if ((status = getaddrinfo(argv[1], NULL, &hints, &res)) != 0) 
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
		return 2;
	}
	printf("IP addresses for %s:\n\n", argv[1]);

	for(p = res;p != NULL; p = p->ai_next) 
	{
		void *addr;
		char *ipver;
		// get the pointer to the address itself,
		// different fields in IPv4 and IPv6:
		if (p->ai_family == AF_INET) 
		{ // IPv4
			struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
			addr = &(ipv4->sin_addr);
			ipver = "IPv4";
		} 
		else 
		{ // IPv6
			struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
			addr = &(ipv6->sin6_addr);
			ipver = "IPv6";
		}
		// convert the IP to a string and print it:
		inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);
		printf(" %s: %s\n", ipver, ipstr);
	}
	freeaddrinfo(res); // free the linked list

	// returns a socket descriptor that can be used later (-1 on error)
	int s = socket(res->ai_family, res->ai_socktype, res->ai_protocol);


	// bind (what port am I on?) associate socket with port on local machine
	int b = bind(s, res->ai_addr, res->ai_addrlen);

	// code to reuse port in case port is hogged
	int yes = 1;
	if (setsockopt(listener,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int)) == -1) 
	{
		perror("setsockopt");
		exit(1);
	} 
	// calling connect() w/o caring for port = don't care about bind()


	// use connect()
	int c = connect(s, res->ai_addr, res->ai_addrlen);

	return 0;
}