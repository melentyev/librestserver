#include "restserver.h"

RST_Resource* RST_resource_init(const char* path, enum http_method method, 
    RST_resource_callback callback)
{
    RST_Resource* resource = (RST_Resource*)RST_alloc(sizeof(RST_Resource));
    resource->path = path;
    resource->method = method;
    resource->callback = callback;
    return resource;
}

void _websocket_connection_callback(RST_Request *request, RST_Response* response)
{

}

RST_ResourceWebSocket* RST_resourcewebsocket_init()
{
    RST_ResourceWebSocket* ws = (RST_ResourceWebSocket*)RST_alloc(sizeof(RST_ResourceWebSocket));
    return ws;
}

RST_Resource* RST_resource_websocket_init(const char* path, 
    RST_ws_open_callback on_open, 
    RST_ws_message_callback on_message, 
    RST_ws_close_callback on_close)
{
    RST_Resource* resource = (RST_Resource*)RST_alloc(sizeof(RST_Resource));
    resource->path = path;
    resource->method = HTTP_GET;
    resource->callback = _websocket_connection_callback;
    resource->websocket = RST_resourcewebsocket_init();
    resource->websocket->on_open = on_open;
    resource->websocket->on_message = on_message;
    resource->websocket->on_close = on_close;
    return resource;
}

RST_Response* RST_resource_handle_request(RST_Resource *resource, 
    RST_Request *request)
{
    RST_Response *response = RST_response_init();
    resource->callback(request, response);
    return response;
}

void RST_resource_release(RST_Resource *resource)
{
    RST_FREE(resource, "RST_resource_release::resource");
}