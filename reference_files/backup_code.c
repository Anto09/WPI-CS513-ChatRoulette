/*
Authors: Francisco Castro, Antonio Umali
CS 513 Project 1 - Chat Roulette
Last modified: 09 Oct 2015

Several pieces of code for reference.
*/

// [ Reap dead processes that appear as the fork()ed child processes exit ]
//=================================================================================

sa.sa_handler = sigchld_handler; // reap all dead processes
sigemptyset(&sa.sa_mask);
sa.sa_flags = SA_RESTART;
if (sigaction(SIGCHLD, &sa, NULL) == -1) 
{
	perror("sigaction");
	exit(1);
}

//=================================================================================


//=================================================================================

// Main accept() loop in server
while(1)
{
	// Accept an incoming connection on the listening socket descriptor, listener
	addrlen = sizeof remoteaddr;
	new_fd = accept(listener, (struct sockaddr *)&remoteaddr, &addrlen);	// new socket descriptor for new connection

	// If error occurs on accept(), it returns -1 and sets errno
	if (new_fd == -1) 
	{
		perror("accept");
		continue;
	}

	// Convert a struct in_addr to numbers-and-dots notation (IP address) for printing
	inet_ntop(remoteaddr.ss_family, get_in_addr((struct sockaddr *)&remoteaddr), remoteIP, sizeof remoteIP);
	printf("Server: got connection from %s\n", remoteIP);

	// This is the child process
	if (!fork())
	{
		close(listener); // child doesn't need the listener

		if (send(new_fd, "Hello, world!", 13, 0) == -1)
		{
			perror("send");
		}

		close(new_fd);
		exit(0);
	}

	close(new_fd); // parent doesn't need this
}

//=================================================================================


//=================================================================================

// create timeoutstruct
struct timeval tv;
tv.tv_sec = 2;
tv.tv_usec = 500000;

// set file descriptors
fd_set readfds;
FD_ZERO(&readfds);
FD_SET(STDIN, &readfds);
	
select(STDIN+1, &readfds, NULL, NULL, &tv);
if (FD_ISSET(STDIN, &readfds))
	printf("A key was pressed!\n");
else
	printf("Timed out.\n");

//=================================================================================


// [ Load up address structs with getaddrinfo() ]
//=================================================================================

// Setup values in hints
memset(&hints, 0, sizeof hints);	// make sure the struct is empty
hints.ai_family = AF_UNSPEC;		// don't care if IPv4 (AF_INET) or IPv6 (AF_INET6)
hints.ai_socktype = SOCK_STREAM;	// use TCP stream sockets
	
// Call getaddrinfo() to setup the structures in hints
// - error: getaddrinfo() returns non-zero
// - success: *servinfo will point to a linked list of struct addrinfo,
//			  each of which contains a struct sockaddr to be used later
if ((error_status = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0) 
{
	fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(error_status));
	return 1;
}

//=================================================================================


// [ Make a socket, connect() to destination ]
//=================================================================================

// Loop through all the results in *servinfo and bind to the first we can
for(p = servinfo; p != NULL; p = p->ai_next) 
{
	// Make a socket
	// - assign a socket descriptor to sockfd on success, -1 on error
	if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) 
	{
		perror("client: socket");
		continue;
	}

	// Connect to a remote host in the destination port and IP address
	// - returns -1 on error and sets errno to the error's value
	if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) 
	{
		close(sockfd);
		perror("client: connect");
		continue;
	}
		break;
}

// Free the linked list when all done with *servinfo
freeaddrinfo(servinfo);

// If *servinfo is empty, then fail to connect
if (p == NULL) 
{
	fprintf(stderr, "client: failed to connect\n");
	return 2;
}

// Convert a struct in_addr to numbers-and-dots notation (IP address) for printing
inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof s);
printf("Client: connecting to %s\n", s);

//=================================================================================


//=================================================================================

// Trigger error if client was incorrectly run
if (argc != 2) {
	fprintf(stderr,"usage: client hostname\n");
	exit(1);
}

//=================================================================================


// [ Send data (?) ]
//=================================================================================
	
printf("sending to %i\n", sockfd);
char msg[5] = "hello";
printf("msg is %s\n", msg);
int sent = send(sockfd, msg, strlen(msg) + 1, 0);

printf("data sent? %i\n", sent != -1);
	

// sendall(sockfd, msg, sizeof msg);

//=================================================================================