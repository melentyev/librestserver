#ifndef _RST_RESPONSE
#define _RST_RESPONSE 1

#include "rstutil.h"
#include "request.h"

typedef struct RST_Response
{
    unsigned short http_major;
    unsigned short http_minor;
    int status_code;
    char *status_message;
    RST_Map *headers;
    RST_String_builder *body;
} RST_Response;

///////////// RST_Response ///////////////
RST_Response* RST_response_init();
RST_Response* RST_response_init_not_found(RST_Request *request, const char* message);
RST_Response* RST_response_init_bad_request(RST_Request *request, const char* message);
void RST_response_set_not_found(RST_Response *response, RST_Request *request, const char* message);
void RST_response_set_bad_request(RST_Response *response, RST_Request *request, const char* message);
void RST_response_set_body(RST_Response* response, const char* body);
void RST_response_set_body_n(RST_Response* response, const char* buf, int len);
void RST_response_set_status_message(RST_Response* response, const char* body);
void RST_response_set_header(RST_Response* response, const char* name, const char* value);
RST_String_builder* RST_response_to_string(RST_Response* response);
void RST_response_release(RST_Response* response);

#endif