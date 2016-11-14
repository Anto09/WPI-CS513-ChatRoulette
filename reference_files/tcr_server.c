/*
Authors: Francisco Castro, Antonio Umali
CS 513 Project 1 - Chat Roulette
Last modified: 01 Oct 2015

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

#define BACKLOG 20	// how many pending connections queue will hold
#define PORT "3490" // the port users will be connecting to

/*
TO DO:
- ACKN
- IN_SESSION
*/

int shutdown_socket(int sockfd, int flag, int free_socket) 
{
	
	// for freeing socket descriptor
	if (free_socket)
	{
		close(sockfd);
		return 1;
	}

	// change socket usability with shutdown
	int success = shutdown(sockfd, flag);
	if (success == 0)
	{
		return 1;
	}

	return 0;
}

void read_bytes(int socket, unsigned int len, void* buffer)
{
    int bytesRead = 0;
    int result;

    while (bytesRead < len)
    {
    	printf("read %i bytes\n", bytesRead);
        result = read(socket, buffer + bytesRead, len - bytesRead);
       
        if (result < 1 )
        {
            perror("read_bytes");
        }

        bytesRead += result;
    }
}

void sigchld_handler(int s) {
	
	// waitpid() might overwrite errno, save and restore it
	int saved_errno = errno;
	while (waitpid(-1, NULL, WNOHANG) > 0);

	errno = saved_errno;
}

// Get sockaddr, IPv4 or IPv6
void *get_in_addr(struct sockaddr *sa) {
	
	// sockaddr is IPv4
	if (sa->sa_family == AF_INET)
	{
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	// else, sockaddr is IPv6
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main()  {

	// [ Variable setup ]
	//=================================================================================

	fd_set master;	// master file descriptor list, holds all socket descriptors currently
					// - connected and the socket descriptor listening for new connections
    fd_set read_fds;  // temp file descriptor list for select()
    int fdmax;        // maximum file descriptor number

    /*
		select() changes the set passed into it to reflect 
		which sockets are ready to read

		store connections somewhere -> copy master into the
		read_fds (read file descriptor) before calling select()
	*/

	// Socket file descriptors: 
	// - listener: listening socket descriptor
	// - new_fd: newly accept()ed socket descriptor/new connection
	int listener, new_fd;

	// For prepping sockaddrs later: 
	// - hints points to an addrinfo to be filled with information
	// - *servinfo points to a linked list of struct addrinfo
	// - *p serves as a temporary pointer to hold *servinfo's data later
	struct addrinfo hints, *servinfo, *p;

	// Client variables
	struct sockaddr_storage remoteaddr;	// Client socket address information
	socklen_t addrlen;	// Size of client's socket address
	char buf[256];		// buffer for client data
	int nbytes;

	// Will hold the error state when getaddrinfo() is called
	int error_status;

	// Used for setsockopt(), SO_REUSEADDR later
	int yes = 1;

	// Space to hold the IPv6 string
	char remoteIP[INET6_ADDRSTRLEN];

	//struct sigaction sa;

	//=================================================================================


	// Clear the master and temp fd sets
	FD_ZERO(&master);
    FD_ZERO(&read_fds);
	

	// [ Load up address structs with getaddrinfo() ]
	//=================================================================================

	// Setup values in hints
	memset(&hints, 0, sizeof hints);	// make sure the struct is empty
	hints.ai_family = AF_UNSPEC;		// don't care if IPv4 (AF_INET) or IPv6 (AF_INET6)
	hints.ai_socktype = SOCK_STREAM;	// use TCP stream sockets
	hints.ai_flags = AI_PASSIVE;		// fill in my IP for me
										// -- tells getaddrinfo() to assign the address of my local host to the socket structures
	
	// Call getaddrinfo() to setup the structures in hints
	// - error: getaddrinfo() returns non-zero
	// - success: *servinfo will point to a linked list of struct addrinfo,
	//			  each of which contains a struct sockaddr to be used later
	if ((error_status = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) 
	{
		fprintf(stderr, "selectserver [getaddrinfo()]: %s\n", gai_strerror(error_status));
		exit(1);
	}

	//=================================================================================


	// [ Make a socket, bind() it, listen() for incoming connections ]
	//=================================================================================

	// Loop through all the results in *servinfo and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) 
	{
		// Make a socket
		// - assign a socket descriptor to listener on success, -1 on error
		if ((listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) 
		{
			perror("server: socket");
			continue;
		}

		// Allows the reuse of a port to avoid "Address already in use" error message
		// - this happens when you rerun a server and bind() fails because the socket
		//	 is still in the kernel using the port.
		if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) 
		{
			perror("setsockopt");
			exit(1);
		}

		// If a socket was made above, bind it to the port passed in to getaddrinfo()
		// - returns -1 on error and sets errno to the error's value
		if (bind(listener, p->ai_addr, p->ai_addrlen) == -1) 
		{
			close(listener);
			perror("server: bind");
			continue;
		}

		break;
	}
	
	// Free the linked list when all done with *servinfo
	freeaddrinfo(servinfo);

	// If *servinfo is empty, then fail to bind a socket to a port
	if (p == NULL) 
	{
		fprintf(stderr, "selectserver: failed to bind\n");
		exit(2);
	}

	// Listen to incoming connections on the port, return -1 on error
	if (listen(listener, BACKLOG) == -1) 
	{
		perror("listen");
		exit(3);
	}

	// Add the listener to the master set
    FD_SET(listener, &master);

    // Keep track of the biggest file descriptor
    fdmax = listener; // so far, it's this one

	//=================================================================================
	

	// Returns the name of the computer this is running on
	char hostname[256];
	int gothostname = gethostname(hostname, 256);

	printf("My hostname: %s\n", hostname);
	printf("Server: waiting for connections...\n");
	

	// Main loop for multi-client chat server
	int i, j;	// Just iterator variables for this main loop
    for(;;) 
    {
    	// Copy master descriptor list to temporary descriptor list
        read_fds = master;

        if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1) {
            perror("select");
            exit(4);
        }

        // Run through the existing connections looking for data to read
        for(i = 0; i <= fdmax; i++) 
        {
        	// We got one - fd i is in the set read_fds
            if (FD_ISSET(i, &read_fds)) 
            {
                if (i == listener) 
                {
                    // Handle new connections:
                    // Accept an incoming connection on the listening socket descriptor, listener
                    addrlen = sizeof remoteaddr;
                    new_fd = accept(listener, (struct sockaddr *)&remoteaddr, &addrlen);

                    // If error occurs on accept(), it returns -1 and sets errno
                    if (new_fd == -1) 
                    {
                        perror("accept");
                    } 
                    else 
                    {
                        FD_SET(new_fd, &master); // add to master set

                        if (new_fd > fdmax) 
                        {   
                            fdmax = new_fd;	// keep track of the max
                        }

                        printf("selectserver: new connection from %s on socket %d\n",
								inet_ntop(remoteaddr.ss_family, 
										  get_in_addr((struct sockaddr*)&remoteaddr),
										  remoteIP, INET6_ADDRSTRLEN), new_fd);
                    }
                } 
                else 
                {
                    // Handle data from a client
                    if ((nbytes = recv(i, buf, sizeof buf, 0)) <= 0) 
                    {
                        // Got error or connection closed by client
                        if (nbytes == 0) 
                        {
                            // connection closed
                            printf("selectserver: socket %d hung up\n", i);
                        } 
                        else 
                        {
                            perror("recv");
                        }

                        close(i); // close() this descriptor
                        FD_CLR(i, &master); // Remove from master set
                    } 
                    else 
                    {
                        // We got some data from a client
                        printf("data received %s\n", buf);

                        for(j = 0; j <= fdmax; j++) 
                        {
                            // send to everyone!
                            if (FD_ISSET(j, &master)) 
                            {
                                // except the listener and ourselves
                                if (j != listener && j != i) 
                                {
                                    if (send(j, buf, nbytes, 0) == -1) 
                                    {
                                        perror("send");
                                    }
                                }
                            }
                        }
                    }
                } // END handle data from client
            } // END got new incoming connection
        } // END looping through file descriptors
    } // END for(;;)--and you thought it would never end!
	

	return 0;	
}

