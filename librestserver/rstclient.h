#ifndef _RST_CLIENT
#define _RST_CLIENT 1

#include "http_parser.h"
#include "service.h"

typedef struct RST_Client
{
    int socket;
    int body_recved;
    int parser_last_was_value;
    int upgrade_websocket;
    RST_String_builder *parser_header_value;
    RST_String_builder *parser_header_field;
    RST_Request *request;
    RST_Service *service;
    http_parser *parser;
} RST_Client;

///////////// RST_Client ///////////////
RST_Client* RST_client_init(RST_Service *owner, int client_socket);
int RST_client_parser_on_header_field(http_parser *parser, const char *at, size_t len);
int RST_client_parser_on_header_value(http_parser *parser, const char *at, size_t len);
int RST_client_parser_on_headers_complete(http_parser *parser);
int RST_client_parser_on_message_begin(http_parser *parser);
int RST_client_parser_on_message_complete(http_parser *parser);
int RST_client_parser_on_url(http_parser *parser, const char *at, size_t len);
int RST_client_parser_on_body(http_parser *parser, const char *at, size_t len);

void RST_client_send_async(RST_Client* client, const char* buf, int len);
void RST_client_parser_save_header(RST_Client *client);
void RST_client_release(RST_Client* client);


#endif