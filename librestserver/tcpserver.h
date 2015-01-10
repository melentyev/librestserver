#ifndef _TCP_SERVER
#define _TCP_SERVER 1

#define _XOPEN_SOURCE_EXTENDED 1
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <stdint.h>

#ifndef WIN32                      
#include <sys/types.h>             
#include <sys/socket.h>            
#include <sys/time.h>
#include <sys/ioctl.h>
#include <unistd.h>                
#include <arpa/inet.h>    
#include <netinet/in.h>
#include <net/rtrouteh.h>
#include <net/if.h>
#define SET_NONBLOCK(socket) { int dontblock = 1; return ioctl(socket, FIONBIO, (char *)&dontblock); }
#else               
#include <winsock.h>
#undef max
#define close closesocket            
#define SET_NONBLOCK(socket) { DWORD dw = 1; return ioctlsocket(socket, FIONBIO, &dw); }
#endif

#include "rstutil.h"

#define RST_TCPSERVER_BUF_SIZE 4096

typedef struct RST_SelectTcpServer 
{
    void *owner;
    unsigned short port;
    int serv_sock, max_connections;
    struct sockaddr_in server;
    char buf[RST_TCPSERVER_BUF_SIZE];
    RST_Set *clients;
    RST_Vector *finished_clients;
    RST_Set *writing_sockets;
    int(*on_client_accepted)(struct RST_SelectTcpServer*, int client_socket);
    int(*on_data_received)(struct RST_SelectTcpServer*, int client_socket, char* buf, int recved);
    int(*on_data_sent)(struct RST_SelectTcpServer*, int client_socket);
} RST_SelectTcpServer;

RST_SelectTcpServer* RST_selecttcpserver_init(uint16_t port, void* owner);
int RST_selecttcpserver_start(RST_SelectTcpServer* server);
void RST_selecttcpserver_send(RST_SelectTcpServer* server, int socket, const char *buf, int size);
void RST_selecttcpserver_release(RST_SelectTcpServer* server);

void RST_selecttcpserver_release_clients(RST_SelectTcpServer* server);

#define RST_TcpServer RST_SelectTcpServer

#define RST_tcpserver_init RST_selecttcpserver_init
#define RST_tcpserver_start RST_selecttcpserver_start
#define RST_tcpserver_send RST_selecttcpserver_send
#define RST_tcpserver_release RST_selecttcpserver_release


#endif