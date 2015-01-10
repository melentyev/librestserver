#include <assert.h>

#include "restserver.h"
#include "tcpserver.h"

#include "http_parser.h"

int RST_service_on_client_accepted(struct RST_SelectTcpServer* server, int client_socket)
{
    RST_Service* service = server->owner;
    RST_Client* client = RST_client_init(service, client_socket);
    RST_map_insert(service->clients, (void*)client_socket, (void*)client);
    return 0;
}

int RST_service_on_data_received(struct RST_SelectTcpServer* server, int client_socket, char* buf, int recved)
{
    RST_Service* service = server->owner;
    RST_Client* client = NULL;
    RST_log_console("RST_service_on_data_received");
    RST_map_find(service->clients, (void*)client_socket, &client);
    if (client->parser == NULL)
    {
        assert(0);
    }
    else 
    {
        http_parser_settings settings;
        settings.on_url = RST_client_parser_on_url;
        settings.on_header_value = RST_client_parser_on_header_value;
        settings.on_header_field = RST_client_parser_on_header_field;
        settings.on_headers_complete = RST_client_parser_on_headers_complete;
        settings.on_message_begin = RST_client_parser_on_message_begin;
        settings.on_message_complete = RST_client_parser_on_message_complete;
        settings.on_body = RST_client_parser_on_body;
        int nparsed = http_parser_execute(client->parser, &settings, buf, recved);
    }
    return 0;
}

int RST_service_on_data_sent(struct RST_SelectTcpServer* server, int client_socket)
{
    RST_Service* service = server->owner;
    RST_Client* client = NULL;
    RST_log_console("RST_service_on_data_sent");
    assert(RST_map_find(service->clients, (void*)client_socket, &client));
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
