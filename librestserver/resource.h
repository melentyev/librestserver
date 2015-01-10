#ifndef _RST_RESOURCE
#define _RST_RESOURCE 1

#include "websocket.h"

typedef void(*RST_resource_callback)(struct RST_Request*, struct RST_Response *response);

typedef void(*RST_ws_open_callback)(struct RST_WebSocketConnectionInterface*);
typedef void(*RST_ws_message_callback)(struct RST_WebSocketConnectionInterface*, char* buf, int len);
typedef void(*RST_ws_close_callback)(struct RST_WebSocketConnectionInterface*);

typedef struct RST_Resource
{
    const char* path;
    enum http_method method;
    RST_resource_callback callback;
    struct RST_ResourceWebSocket* websocket;
} RST_Resource;

///////////// RST_Resource ///////////////
RST_Resource* RST_resource_init(const char* path, enum http_method method, RST_resource_callback callback);
RST_Resource* RST_resource_websocket_init(const char* path, RST_ws_open_callback on_open, RST_ws_message_callback on_message, RST_ws_close_callback on_close);
RST_Response* RST_resource_handle_request(RST_Resource *resource, RST_Request *request);
void RST_resource_release(RST_Resource* response);

#endif