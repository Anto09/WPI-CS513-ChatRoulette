/*
Authors: Francisco Castro, Antonio Umali
CS 513 Project 1 - Chat Roulette
Last modified: 28 Sept 2015

This is a reference file for the structures and system calls being used.
*/

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

/*
--------------------------------------------------------------------------
Structures
--------------------------------------------------------------------------
*/

/*
Used to prep the socket address structures for subsequent use.
Used in host name lookups and service name lookups.
*/
struct addrinfo 
{
	int ai_flags;				// AI_PASSIVE, AI_CANONNAME, etc.
	int ai_family;				// AF_INET, AF_INET6, AF_UNSPEC
	int ai_socketype;			// SOCK_STREAM, SOCK_DGRAM
	int ai_protocol;			// use 0 for "any"
	size_t ai_addrlen;			// size of ai_addr in bytes
	struct sockaddr *ai_addr;	// struct sockaddr_in or _in6
	char *ai_canonname;			// full canonical hostname

	struct addrinfo *ai_next;	// next addrinfo node in the linked list
};

/*
Holds socket address information for many types of sockets.

To deal with struct sockaddr, programmers created a parallel 
structure: struct sockaddr_in ("in" for "Internet") to be used with IPv4.
*/
struct sockaddr
{
	unsigned short sa_family;	// address family, AF_xxx; AF_INET (IPv4) or AF_INET6 (IPv6)
	char sa_data[14];			// 14 bytes of protocol address;
								//-- contains a destination address and port number for the socket
};

/*
(IPv4 only--see struct sockaddr_in6 for IPv6)

A pointer to a struct sockaddr_in can be cast to a pointer to a struct sockaddr 
and vice-versa. So even though connect() wants a struct sockaddr*, you can still 
use a struct sockaddr_in and cast it.
*/
struct sockaddr_in
{
	short int sin_family;			// Address family, AF_INET
	unsigned short int sin_port;	// Port number; mus tbe in Network Byte Order using htons()
	struct in_addr sin_addr;		// Internet address
	unsigned char sin_zero[8];		// Same size as struct sockaddr; 
									//-- should be set to all zeros with memset()
};

/*
(IPv4 only--see struct in6_addr for IPv6)
Internet address (a structure for historical reasons)
*/
struct in_addr
{
	uint32_t s_addr;			// that's a 32-bit int (4 bytes)
};

/*
(IPv6 only--see struct sockaddr_in and struct in_addr for IPv4)
*/
struct sockaddr_in6 
{
    u_int16_t       sin6_family;   // address family, AF_INET6
    u_int16_t       sin6_port;     // port number, Network Byte Order
    u_int32_t       sin6_flowinfo; // IPv6 flow information
    struct in6_addr sin6_addr;     // IPv6 address
    u_int32_t       sin6_scope_id; // Scope ID
};

struct in6_addr 
{
    unsigned char   s6_addr[16];   // IPv6 address
};

/*
Designed to be large enough to hold both IPv4 and IPv6 structures.
*/
struct sockaddr_storage
{
	sa_family_t  ss_family;     // address family

    // all this is padding, implementation specific, ignore it:
    char      __ss_pad1[_SS_PAD1SIZE];
    int64_t   __ss_align;
    char      __ss_pad2[_SS_PAD2SIZE];
};

/*
--------------------------------------------------------------------------
System Calls
--------------------------------------------------------------------------
*/

/*
Fills out the structs you need, returns a pointer to a linked-list, res, of results.
*/
int getaddrinfo(const char *node, 				// e.g. "www.example.com" or IP
				const char *service, 			// e.g. "http" or port number
				const struct addrinfo *hints,	// points to a struct addrinfo filled out with relevant information
				struct addrinfo **res);			// holds results of call to gettaddrinfo()

/*
Returns to a socket descriptor for later system calls, or -1 on error.
*/
int socket(int domain,		// PF_INET or PF_INET6
		   int type,		// SOCK_STREAM or SOCK_DGRAM
		   int protocol);	// can be set to 0 to choose the proper protocol for the given type

/*
Associate that socket with a port on your local machine.
Returns -1 on error and sets errno to the error's value.
*/
int bind(int sockfd,				// socket file descriptor returned by socket()
		 struct sockaddr *my_addr,	// pointer to a struct sockaddr that contains information about your address, namely, port and IP addres
		 int addrlen);				// length in bytes of that^ address

/*
Connect to a remote host.
Return -1 on error and set the variable errno.
*/
int connect(int sockfd,						// socket file descriptor returned by the socket()
			struct sockaddr *serv_addr,		// a struct sockaddr containing the destination port and IP address
			int addrlen);					// length in bytes of the server address structure^

/*
Wait for incoming connections and handle them.
Call bind() before listen() so that the server is running on a specific port.
*/
int listen(int sockfd,		// socket file descriptor from the socket()
		   int backlog);	// number of connections allowed on the incoming queue
							//-- incoming connections are going to wait in this queue until you accept() them

/*
Returns a brand new socket file descriptor to use for a single connection.
*/
int accept(int sockfd,				// the listen()ing socket descriptor
		   struct sockaddr *addr,	// pointer to a local struct sockaddr_storage; where the information about the incoming connection will go
		   socklen_t *addrlen);		// local integer variable that should be set to sizeof(struct sockaddr_storage) before its address is passed to accept()

/*
Returns the number of bytes actually sent out.
(this might be less than the number you told it to send)
-1 is returned on error, and errno is set to the error number.
*/
int send(int sockfd,		// socket descriptor you want to send data to
		 const void *msg,	// pointer to the data you want to send
		 int len,			// length of that data^ in bytes
		 int flags);		// just set to 0

/*
Returns the number of bytes actually read into the buffer, 
or -1 on error (with errno set, accordingly.)
*/
int recv(int sockfd,	// socket descriptor to read from
		 void *buf,		// buffer to read the information into
		 int len,		// maximum length of the buffer
		 int flags);	// just set to 0

