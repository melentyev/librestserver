#ifndef _RST_REQUEST
#define _RST_REQUEST 1

#include "rstutil.h"
#include "http_parser.h"

typedef struct RST_Request
{
    RST_Map *headers;
    enum http_method method;
    unsigned short http_major;
    unsigned short http_minor;
    int upgrade_websocket;
    RST_Map *query_params;
    RST_String_builder *url;
    RST_String_builder *body;
    char* path;
    int from_socket;
} RST_Request;

///////////// RST_Request ///////////////
RST_Request* RST_request_init();
const char* RST_request_get_header_value(RST_Request *request, char* key);
const char* RST_request_get_query_param(RST_Request *request, char* key);
void RST_request_release(RST_Request* request);




#endif