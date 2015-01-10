#include "restserver.h"

RST_Request* RST_request_init()
{
    RST_Request* request = (RST_Request*)RST_alloc(sizeof(RST_Request));
    RST_LOG_FMT(stderr, "RST_request_init: addr(%08x)", request);
    request->body = RST_string_builder_init();
    request->headers = RST_map_init(RST_cstr_comparator, RST_cstr_dtor, RST_cstr_dtor);
    request->url = RST_string_builder_init();
    request->path = NULL;
    request->upgrade_websocket = 0;
    request->query_params = RST_map_init(RST_cstr_comparator, RST_cstr_dtor, RST_cstr_dtor);
    return request;
}

const char* RST_request_get_header_value(RST_Request *request, char* key)
{
    char *value;
    RST_map_find(request->headers, (void*)key, (void**)(&value));
    return value;
}

const char* RST_request_get_query_param(RST_Request *request, char* key)
{
    char *value;
    if (RST_map_find(request->query_params, (void*)key, (void**)(&value)))
    {
        return value;
    }
    return 0;
}

void RST_request_release(RST_Request* request)
{
    RST_map_release(request->headers);
    RST_map_release(request->query_params);
    RST_string_builder_release(request->body);
    RST_string_builder_release(request->url);
    if (request->path != NULL)
    {
        RST_FREE(request->path, "RST_request_release::request->path");
    }
    RST_FREE(request, "RST_request_release::request");
}