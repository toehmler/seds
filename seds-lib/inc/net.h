#ifndef __NET_H
#define __NET_H

int net_accept_conn(int serv_fd);
int net_get_socket(char *port, int backlog);

#endif


