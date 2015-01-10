#include "restserver.h"
#include <assert.h>

#include "http_parser.h"

RST_Client* RST_client_init(RST_Service *owner, int client_socket)
{
    
    RST_Client* client = RST_ALLOC_STRUCT(RST_Client);
    
    client->parser_last_was_value = 1;
    client->body_recved = 0;
    client->upgrade_websocket = 0;
    client->request = RST_request_init();
    client->parser_header_field = NULL;
    client->parser_header_value = NULL;
    client->service = owner;
    client->socket = client_socket;

    client->parser = RST_alloc_tag(sizeof(http_parser), "RST_client_init::client->parser");
    http_parser_init(client->parser, HTTP_REQUEST);
    client->parser->data = client;
    return client;
}


void RST_client_send_async(RST_Client* client, const char* buf, int len)
{
    RST_tcpserver_send(client->service->server, client->socket, buf, len);
}

int RST_client_parser_on_header_field(http_parser *parser, const char *at, size_t len)
{
    RST_Client* client = (RST_Client*)(parser->data);
    if (client->parser_last_was_value) 
    {
        RST_client_parser_save_header(client);
        client->parser_header_field = RST_string_builder_init();
    }
    RST_string_builder_append_n(client->parser_header_field, at, len);
    client->parser_last_was_value = 0;
    return 0;
}

int RST_client_parser_on_header_value(http_parser *parser, const char *at, size_t len)
{
    RST_Client* client = (RST_Client*)parser->data;
    if (!client->parser_last_was_value) 
    {
        client->parser_header_value = RST_string_builder_init();
    }
    RST_string_builder_append_n(client->parser_header_value, at, len);
    client->parser_last_was_value = 1;
    return 0;
}

void RST_client_parser_save_header(RST_Client *client)
{
    assert(client->parser_header_field != NULL && client->parser_header_value != NULL
        || client->parser_header_value == NULL && client->parser_header_value == NULL);
    if (client->parser_header_field != NULL && client->parser_header_value != NULL)
    {
        RST_map_insert(client->request->headers,
            (void*)RST_alloc_string_n(client->parser_header_field->buf, client->parser_header_field->len),
            (void*)RST_alloc_string_n(client->parser_header_value->buf, client->parser_header_value->len));
        RST_string_builder_release(client->parser_header_field);
        RST_string_builder_release(client->parser_header_value);
        client->parser_header_field = NULL;
        client->parser_header_value = NULL;
    }
}

int RST_client_parser_on_message_begin(http_parser *parser)
{
    RST_Client* client = (RST_Client*)parser->data;
    return 0;
}

int RST_request_parse_query_string(RST_Request *request, const char* qstr, int len)
{
    int current_is_value = 0, started_from = 0, cur_len = 0;
    const char *key = NULL, *value = NULL;
    for (int i = 0; i < len; i++)
    {
        switch (qstr[i])
        {
        case '=':
            if (current_is_value)
            {
                key ? RST_FREE(key, "RST_request_parse_query_string::key") : 0;
                value ? RST_FREE(key, "RST_request_parse_query_string::value") : 0;
                return 0;
            }
            else
            {
                current_is_value = 1;
                key = RST_alloc_string_n(qstr + started_from, cur_len);
                started_from = i + 1;
                cur_len = 0;
            }
            break;
        case '&':
            value = current_is_value ? RST_alloc_string_n(qstr + started_from, cur_len) : RST_alloc_string("");
            RST_map_insert(request->query_params, (void*)key, (void*)value);
            key = value = NULL;
            started_from = i + 1;
            cur_len = current_is_value = 0;
        default:
            cur_len++;
        }
        //request->
    }
    value = current_is_value ? RST_alloc_string_n(qstr + started_from, cur_len) : RST_alloc_string("");
    RST_map_insert(request->query_params, (void*)key, (void*)value);
    return 1;
}
int RST_client_parser_on_message_complete(http_parser *parser)
{
    RST_Client *client = (RST_Client*)parser->data;
    RST_Request *request = client->request;
    RST_Response* response = NULL;
    RST_client_parser_save_header(client);
    request->method = parser->method;
    request->http_major = parser->http_major;
    request->http_minor = parser->http_minor;
    struct http_parser_url url_data;
    http_parser_parse_url(request->url->buf, request->url->len, 0, &url_data);
    
    int plen = url_data.field_data[UF_PATH].len;
    int poff = url_data.field_data[UF_PATH].off;
    int qlen = url_data.field_data[UF_QUERY].len;
    int qoff = url_data.field_data[UF_QUERY].off;
    request->path = RST_alloc_string_n(request->url->buf + poff, plen);
    int has_query_str = url_data.field_set & (1 << UF_QUERY);
    if (has_query_str && !RST_request_parse_query_string(request,
        request->url->buf + qoff, qlen))
    {
        response = RST_response_init_bad_request(request, NULL);
    }
    else 
    {
        RST_Resource *resource = RST_service_find_resource(client->service, client->request);
        if (resource == NULL)
        {
            response = RST_response_init_not_found(request, NULL);
        }
        else
        {
            response = RST_resource_handle_request(resource, client->request);
        }
    }
    RST_String_builder *response_data = RST_response_to_string(response);
    RST_response_release(response);
    RST_client_send_async(client, response_data->buf, response_data->len);
    return 0;
}

int RST_client_parser_on_url(http_parser *parser, const char *at, size_t len)
{
    RST_Client* client = (RST_Client*)parser->data;
    RST_string_builder_append_n(client->request->url, at, len);
    return 0;
}

int RST_client_parser_on_headers_complete(http_parser *parser)
{
    RST_Client* client = (RST_Client*)parser->data;
    return 0;
}

int RST_client_parser_on_body(http_parser *parser, const char *at, size_t len)
{
    RST_Client* client = (RST_Client*)parser->data;
    RST_string_builder_append_n(client->request->body, at, len);
    return 0;
}

void RST_client_release(RST_Client* client)
{
    RST_request_release(client->request);
    
    RST_FREE(client->parser, "RST_client_release::client->parser");
    RST_FREE(client, "RST_client_release::client");
}