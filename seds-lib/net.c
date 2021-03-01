/* 
 * net.c
 */

#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include "inc/net.h"
/* net_get_socket: binds a socket to a port, starts listening 
 * @param: port a string to use as the port number 
 * @param: backlog int to use for the backlog of connections 
 * @returns: int for the fd the server is listening for connections on , -1 on err */
int net_get_socket(char *port, int backlog) {
    int fd, rc;
    int yes = 1;
    struct addrinfo hints, *res;
    fd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if ((rc = getaddrinfo(NULL, port, &hints, &res)) != 0) {
        printf("getaddrinfo failed: %s\n", gai_strerror(rc));
        return -1;
    }
    // Lose the pesky "address already in use" error message
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    if (bind(fd, res->ai_addr, res->ai_addrlen) < 0) {
        perror("bind");
        return -1;
    }
    if (listen(fd, backlog) < 0) {
        perror("listen");
        return -1;
    }
    if (res) 
        freeaddrinfo(res);
    return fd;
}

int net_accept_conn(int fd) {
    socklen_t addrlen;
    struct sockaddr_in client_sa;
    addrlen = sizeof(client_sa);
    int client_fd = accept(fd, (struct sockaddr *) &client_sa, &addrlen);
    return client_fd;

}

