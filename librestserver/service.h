#ifndef _RST_SERVICE
#define _RST_SERVICE 1

#include "resource.h"
#include "tcpserver.h"

typedef struct RST_Service
{
    uint16_t port;
    RST_Vector *resources;
    RST_Map *clients;
    RST_TcpServer *server;
} RST_Service;

///////////// RST_Service ///////////////
RST_Service* RST_service_init(uint16_t port);
void RST_service_start(RST_Service* service);
void RST_service_publish(RST_Service* service, RST_Resource* value);
RST_Resource* RST_service_find_resource(RST_Service* service, RST_Request *request);
void RST_service_release(RST_Service* service);

int RST_service_on_client_accepted(struct RST_SelectTcpServer*, int client_socket);
int RST_service_on_data_received(struct RST_SelectTcpServer* server, int client_socket, uint8_t* buf, int recved);
int RST_service_on_data_sent(struct RST_SelectTcpServer*, int client_socket);


#endif