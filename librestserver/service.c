#ifndef _XOPEN_SOURCE_EXTENDED
#define _XOPEN_SOURCE_EXTENDED 1
#endif 
#ifndef _OPEN_THREADS
#define _OPEN_THREADS
#endif 

#include "restserver.h"
#include "tcpserver.h"
#include "requestparser.h"
//#include "http_parser.h"

#include <assert.h>
#include <stdlib.h>


int RST_service_on_client_accepted(struct RST_SelectTcpServer* server, int client_socket)
{
    RST_log_console("RST_service_on_client_accepted");
    RST_Service* service = server->owner;
    RST_Client* client = RST_client_init(service, client_socket);
    RST_map_insert(service->clients, (void*)client_socket, (void*)client);
    return 0;
}

int RST_service_on_data_received(struct RST_SelectTcpServer* server, int client_socket, uint8_t* buf, int recved)
{
    RST_Service* service = server->owner;
    RST_Client* client = NULL;
    RST_log_console("RST_service_on_data_received");
    RST_map_find(service->clients, (void*)client_socket, (void**)&client);
    assert(client->parser);  
    if (buf[0] == (char)0x7C) {
        RST_log_console("Exit byte received\n");
        exit(0);
    }
#ifdef RST_USE_HTTP_PARSER
    http_parser_settings settings;
#else
    RST_RequestParserSettings settings;
#endif
    settings.on_url = RST_client_parser_on_url;
    settings.on_header_value = RST_client_parser_on_header_value;
    settings.on_header_field = RST_client_parser_on_header_field;
    settings.on_headers_complete = RST_client_parser_on_headers_complete;
    settings.on_message_begin = RST_client_parser_on_message_begin;
    settings.on_message_complete = RST_client_parser_on_message_complete;
    settings.on_body = RST_client_parser_on_body;
    printf("(recved: %d)\n", recved);
    RST_log_console("12 received bytes:\n");
    for (int i = 0; i < recved && i < 25; i++) { printf("%x ", buf[i]); } printf("\n");
    __a2e_l((char*)buf, recved);
    RST_log_console("25 converted bytes:\n");
    for (int i = 0; i < recved && i < 25; i++) { printf("%x ", buf[i]); } printf("\n");
    RST_log_console("60 converted bytes as str:\n");
    for (int i = 0; i < recved && i < 60; i++) { printf("%c", buf[i]); } printf("\n");
#ifdef RST_USE_HTTP_PARSER
    int nparsed = http_parser_execute(client->parser, &settings, (const char*)buf, recved);
#else
    int nparsed = RST_requestparser_execute(client->parser, &settings, (const char*)buf, recved);
#endif
    printf("RST_service_on_data_received::nparsed(%d)\n", nparsed);
    return 0;
}

int RST_service_on_data_sent(struct RST_SelectTcpServer* server, int client_socket)
{
    RST_Service* service = server->owner;
    RST_Client* client = NULL;
    RST_log_console("RST_service_on_data_sent");
    assert(RST_map_find(service->clients, (void*)client_socket, &client));
    RST_string_builder_release(client->response_buffer);
    if (!client->upgrade_websocket)
    {
        RST_map_erase(service->clients, (void*)client_socket);
        return 1;
    }
    return 0;
}

RST_Service* RST_service_init(uint16_t port)
{
    RST_Service* service = (RST_Service*)RST_alloc(sizeof(RST_Service));
    service->clients = RST_map_init(RST_int_comparator, NULL, RST_client_release);
    service->port = port;
    service->resources = RST_vector_init(RST_resource_release);
    service->server = RST_tcpserver_init(port, service);
    service->server->on_client_accepted = RST_service_on_client_accepted;
    service->server->on_data_received = RST_service_on_data_received;
    service->server->on_data_sent = RST_service_on_data_sent;
    return service;
}

void RST_service_publish(RST_Service* service, RST_Resource* value)
{
    RST_vector_push_back(service->resources, (void*)value);
}

void RST_service_publish_generic_handler(RST_Service* service, RST_resource_callback cb)
{
    service->generic_handler = cb;
}

void RST_service_start(RST_Service* service)
{
    RST_tcpserver_start(service->server);
}

RST_Resource* RST_service_find_resource(RST_Service* service, RST_Request *request)
{
    for (RST_VectorIterator it = RST_vector_begin(service->resources); !RST_vector_end(it); RST_vector_iterator_next(&it))
    {
        RST_Resource* res = (RST_Resource*)RST_vector_iterator_value(it);
        if (res->method == request->method && strcmp(res->path, request->path) == 0)
        {
            return res;
        }
    }
    return NULL;
}

void RST_service_release(RST_Service* service)
{
    RST_map_release(service->clients);
    RST_vector_release(service->resources);
    RST_tcpserver_release(service->server);
}
