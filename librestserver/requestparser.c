#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "http_parser.h"
#include "rstutil.h"
#include "requestparser.h"
#include "restserver.h"

#define PARSER_ASSERT(ex) { if (!(ex)) { return 0; } }

int ascii_to_ebcdic_table[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, '!', '"', '#', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

char RST_char_ebcdic_to_ascii(char c)
{
    return 0;
}


RST_RequestParser* RST_requestparser_init() {
    RST_RequestParser *parser = RST_ALLOC_STRUCT(RST_RequestParser, "RST_requestparser_init::parser");
    parser->last_pos = 0;
    parser->error = 0;
    parser->body_read = parser->content_length = (uint64_t)0;
    parser->buf = RST_string_builder_init();
    return parser;
}

int RST_requestparser_go_until_eq(RST_RequestParser *parser, const char *pat, size_t *pos) {
    size_t len = strlen(pat);
    int res = 0;
    size_t cur_pos = parser->last_pos;
    while (cur_pos + len - 1 < (size_t)parser->buf->len) {
        if (res = (strncmp(parser->buf->buf + cur_pos, pat, len) == 0)) {
            *pos = cur_pos;
            break;
        }
        cur_pos++;
    }
    return res;
}

/*const char *RST_methods[] = {
"GET",
"POST",
"PUT"
};*/


int RST_requestparser_request_line(RST_RequestParser *parser, const char* at, size_t len)
{
    const char *pos;
    const char *pos2;
    PARSER_ASSERT((pos = memchr(at, RST_PARSER_CHAR(' '), len)) != NULL);
    if (strncmp(at, "GET", pos - at) == 0) { parser->method = HTTP_GET; }
    else if (strncmp(at, "POST", pos - at) == 0) { parser->method = HTTP_POST; }
    else if (strncmp(at, "PUT", pos - at) == 0) { parser->method = HTTP_PUT; }
    else if (strncmp(at, "DELETE", pos - at) == 0) { parser->method = HTTP_DELETE; }
    else { return 0; }
    while (pos < at + len && *pos == RST_PARSER_CHAR(' ')) pos++;
    PARSER_ASSERT(pos < at + len);
    PARSER_ASSERT((pos2 = memchr(pos, RST_PARSER_CHAR(' '), len - (pos - at))) != NULL);
    parser->settings->on_url(parser, pos, pos2 - pos);
    pos = pos2;
    while (pos < at + len && *pos == RST_PARSER_CHAR(' ')) pos++;
    PARSER_ASSERT(pos < at + len);
    PARSER_ASSERT(strncmp(pos, RST_PARSER_STR("HTTP/"), 5) == 0);
    pos += 5;
    PARSER_ASSERT(isdigit(*pos) && *(pos + 1) == RST_PARSER_CHAR('.') && isdigit(*(pos + 2)) && pos + 3 >= at + len);
    parser->http_major = *pos - '0';
    parser->http_minor = *(pos + 2) - '0';
    return 1;
}

int RST_requestparser_header(RST_RequestParser *parser, const char* at, size_t len, int *last_header)
{
    *last_header = 0;
    if (len == 0) { return (*last_header = 1); }
    int field_finished = 0, start_from = -1;
    for (size_t i = 0; i < len; i++)
    {
        if (at[i] != RST_PARSER_CHAR(' ') && start_from == -1) start_from = i;
        if (at[i] == RST_PARSER_CHAR(':')) {
            PARSER_ASSERT(!field_finished);
            field_finished = 1;
            parser->settings->on_header_field(parser, at + start_from, i - start_from);
            start_from = -1;
        }
    }
    PARSER_ASSERT(start_from != -1);
    parser->settings->on_header_value(parser, at + start_from, len - start_from);
    return 1;
}

void RST_requestparser_parse_url(const char *at, size_t len, size_t *plen, size_t *qoff, size_t *qlen)
{
    const char *end = at + len;
    *qoff = *plen = 0;
    while (at < end && *at != RST_PARSER_CHAR('?')) { at++; len--; (*plen)++; (*qoff)++; }
    if (*at == RST_PARSER_CHAR('?')) { len--; (*qoff)++; }
    *qlen = len;
}

int RST_requestparser_execute(RST_RequestParser *parser, RST_RequestParserSettings *settings, const char *data, size_t len)
{
    RST_string_builder_append_n(parser->buf, data, len);
    int pos = 0, last_header = 0, loop_done = 0;
    parser->settings = settings;
    size_t nbytes = 0;
    while (!loop_done) {
        switch (parser->state)
        {
        case RST_ps_start: parser->state++;
        case RST_ps_req_line:
            if (RST_requestparser_go_until_eq(parser, RST_PARSER_STR("\r\n"), &pos))
            {
                parser->settings->on_message_begin(parser);
                PARSER_ASSERT(RST_requestparser_request_line(parser, parser->buf->buf, pos - parser->last_pos));
                RST_string_builder_slice_inplace(parser->buf, pos + 2, parser->buf->len - pos - 2);
                parser->last_pos = 0;
                parser->state++;
            }
            else { loop_done = 1; }
            break;
        case RST_ps_header:
            if (RST_requestparser_go_until_eq(parser, RST_PARSER_STR("\r\n"), &pos))
            {
                RST_requestparser_header(parser, parser->buf->buf, pos - parser->last_pos, &last_header);
                RST_string_builder_slice_inplace(parser->buf, pos + 2, parser->buf->len - pos - 2);
                parser->last_pos = 0;
                if (last_header)
                {
                    settings->on_headers_complete(parser);
                    parser->state++;
                }
            }
            else { loop_done = 1; }
            break;
        case RST_ps_body:
            nbytes = RST_min(parser->buf->len - parser->last_pos, (int)(parser->content_length - parser->body_read));
            if (nbytes > 0) 
            { 
                parser->settings->on_body(parser, parser->buf->buf + parser->last_pos, nbytes);
                parser->body_read += nbytes;
                parser->last_pos += nbytes;
            }
            if (nbytes == parser->content_length - parser->body_read) 
            {
                parser->settings->on_message_complete(parser);
                parser->state = RST_ps_finished;
                loop_done = 1;
            }
            break;
        }
    }
    return 0;
}
