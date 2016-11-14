/*
Authors: Francisco Castro, Antonio Umali
CS 513 Project 1 - Chat Roulette
Last modified: 28 Sept 2015

This is the server process file.
*/

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>

#define BACKLOG 10
#define PORT "3490" // the port client will be connecting to
#define STDIN 0

/*
TO DO:
- ACKN
- IN_SESSION
*/

int shutdown_socket(int sockfd, int flag, int free_socket)
{
	// for freeing socket descripter
	if (free_socket)
	{
		close(sockfd);
		return 1;
	}

	// change socket usability with shutdown
	int success = shutdown(sockfd, flag);
	if (success == 0)
		return 1;
	return 0;
}

void sigchld_handler(int s)
{
	// waitpid() might overwrite errno, save and restore it
	int saved_errno = errno;
	while (waitpid(-1, NULL, WNOHANG) > 0);

	errno = saved_errno;
}

void *get_in_addr(struct sockaddr *sa)
{
	// IPv4
	if (sa->sa_family == AF_INET)
	{
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main() 
{
	int sockfd, new_fd; // listen on sock_fd, new connection on new_fd
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size;
	struct sigaction sa;
	int yes=1;
	char s[INET6_ADDRSTRLEN];
	int rv;

	// create timeoutstruct
	struct timeval tv;
	tv.tv_sec = 2;
	tv.tv_usec = 500000;

	// set file descriptors
	fd_set readfds;
	FD_ZERO(&readfds);
	FD_SET(STDIN, &readfds);
	
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP
	
	if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) 
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}
	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) 
	{
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) 
		{
			perror("server: socket");
			continue;
		}
		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) 
		{
			perror("setsockopt");
			exit(1);
		}
		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) 
		{
			close(sockfd);
			perror("server: bind");
			continue;
		}
		break;
	}
	freeaddrinfo(servinfo); // all done with this structure

	if (p == NULL) 
	{
		fprintf(stderr, "server: failed to bind\n");
		exit(1);
	}
	if (listen(sockfd, BACKLOG) == -1) 
	{
		perror("listen");
		exit(1);
	}

	sa.sa_handler = sigchld_handler; // reap all dead processes
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) 
	{
		perror("sigaction");
		exit(1);
	}
	char hostname[256];
	int gothostname = gethostname(hostname, 256);

	printf("my hostname is %s\n", hostname);
	printf("server: waiting for connections...\n");

	// don't care about writefds and exceptfds;
	select(STDIN+1, &readfds, NULL, NULL, &tv);
	if (FD_ISSET(STDIN, &readfds))
		printf("A key was pressed!\n");
	else
		printf("Timed out.\n");
	
	while(1) // main accept() loop
	{
		// insert select() somewhere here

		sin_size = sizeof their_addr;
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		if (new_fd == -1) 
		{
			perror("accept");
			continue;
		}
		inet_ntop(their_addr.ss_family,
		get_in_addr((struct sockaddr *)&their_addr),
		s, sizeof s);
		printf("server: got connection from %s\n", s);

		if (!fork()) // this is the child process
		{
			close(sockfd); // child doesn't need the listener
			if (send(new_fd, "Hello, world!", 13, 0) == -1)
				perror("send");
			close(new_fd);
			exit(0);
		}
		close(new_fd); // parent doesn't need this

	}

	return 0;	
}

/*
	int select(int numfds, fd_set *readfds, fd_set *writefds, 
				fd_set *exceptfds, struct timeval *timeout);

	select() monitors sets of filedescriptors (fd's)
		-readfds, writefds, exceptfds
		-read from stdin and some sockfd -> fds 0 and sockfd to readfds
		-numfds = values of the highest fd + 1
		
	sets of type fd_set:
		FD_SET(int fd, fd_set, *set) -> Add fd to the set
		FD_CLR(int fd, fd_Set, *set) -> Remove fd from the set
		FD_ISSET(int fd, fd_set *set) -> Return true if fd is in the set
		FD_ZERO(fd_set *set) -> Clear all entries from the set

	timeval - specify timeout period
*/