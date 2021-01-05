#ifndef GLOBAL_H_
#define GLOBAL_H_

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <linux/sockios.h>

#define HOSTNAME_LEN 128
#define PATH_LEN 256
#define BACKLOG 5
#define STDIN 0
#define TRUE 1
#define CMD_SIZE 512
#define BUFFER_SIZE 1024
#define MSG_SIZE 256

int client_num;
fd_set master_list, watch_list;

struct client{
	int socket_id;
	int no;
	char hostname[128];
	char ip_addr[INET_ADDRSTRLEN];
	int port_no;
	char blockedList[10][INET_ADDRSTRLEN];
	int block_num;
    int login_status;
    int send_msg_num;
    int recv_msg_num;
    char msg_buffer[100][BUFFER_SIZE];
	int buffered_msg;
}clients[BACKLOG];

#endif
