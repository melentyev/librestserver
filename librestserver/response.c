#include "restserver.h"


RST_Response* RST_response_init()
{
    RST_Response* response = (RST_Response*)RST_alloc(sizeof(RST_Response));
    response->body = RST_string_builder_init();
    response->status_message = NULL;
    response->headers = RST_map_init(RST_cstr_comparator, RST_cstr_dtor, RST_cstr_dtor);
    response->http_major = 0;
    response->http_minor = 0;
    return response;
}

void RST_response_version_from_request(RST_Response *response, RST_Request *request)
{
    response->http_major = request->http_major;
    response->http_minor = request->http_minor;
}

RST_Response* RST_response_init_not_found(RST_Request *request, const char* message)
{
    RST_Response* response = RST_response_init();
    response->status_code = RST_STATUSCODE_NOT_FOUND;
    RST_response_set_status_message(response, message == NULL ? "Not found" : message);
    RST_response_version_from_request(response, request);
    RST_response_set_body(response, "");
    return response;
}

RST_Response* RST_response_init_bad_request(RST_Request *request, const char* message)
{
    RST_Response* response = RST_response_init();
    response->status_code = RST_STATUSCODE_BAD_REQUEST;
    RST_response_set_status_message(response, message == NULL ? "Bad request" : message);
    RST_response_version_from_request(response, request);
    RST_response_set_body(response, "");
    return response;
}

void RST_response_set_body(RST_Response* response, const char* body)
{
    RST_response_set_body_n(response, body, strlen(body));   
}

void RST_response_set_body_n(RST_Response* response, const char* body, int len)
{
    char sprintf_buf[30];
    RST_string_builder_append_n(response->body, body, len);
    sprintf(sprintf_buf, "%d", response->body->len);
    RST_map_insert(response->headers, (void*)RST_alloc_string("Content-Length"), (void*)RST_alloc_string(sprintf_buf));
}

void RST_response_set_status_message(RST_Response* response, const char* msg)
{
    response->status_message = RST_alloc_string(msg);
}


RST_String_builder* RST_response_to_string(RST_Response* response)
{
    RST_String_builder *builder = RST_string_builder_init();   
    RST_string_builder_append(builder, "HTTP/");
    RST_string_builder_append_int(builder, response->http_major);
    RST_string_builder_append_char(builder, '.');
    RST_string_builder_append_int(builder, response->http_minor);
    RST_string_builder_append_char(builder, ' ');
    RST_string_builder_append_int(builder, response->status_code);
    RST_string_builder_append_char(builder, ' ');
    RST_string_builder_append(builder, response->status_message);
    RST_string_builder_append(builder, "\r\n");
    for (RST_MapIterator it = RST_map_begin(response->headers); !RST_map_end(it); RST_map_iterator_next(&it))
    {
        char *key, *value;
        RST_map_iterator_value(it, (void**)(&key), (void**)(&value));
        RST_string_builder_append(builder, key);
        RST_string_builder_append(builder, ": ");
        RST_string_builder_append(builder, value);
        RST_string_builder_append(builder, "\r\n");
    }
    RST_string_builder_append(builder, "\r\n");
    RST_string_builder_append_builder(builder, response->body);
    return builder;
}

void RST_response_release(RST_Response* response)
{
    RST_FREE(response->status_message, "RST_response_release::response->status_message");
    RST_map_release(response->headers);
    RST_string_builder_release(response->body);
    RST_FREE(response, "RST_response_release::response");
}