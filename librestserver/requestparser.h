#ifndef _RST_REQUESTPARSER
#define _RST_REQUESTPARSER 1

#include "rstutil.h"

#include <stdint.h>

enum RST_ParserState {
    RST_ps_start,
    RST_ps_req_line,
    RST_ps_header,
    RST_ps_body,
    RST_ps_finished
};



typedef struct RST_RequestParser {
    enum RST_ParserState state;
    RST_String_builder *lex;
    size_t last_pos;
    RST_String_builder *buf;
    int error;
    enum http_method method;
    unsigned short http_major;
    unsigned short http_minor;
    uint64_t content_length;
    uint64_t body_read;
    void *data;
    struct RST_RequestParserSettings *settings;
} RST_RequestParser;

typedef int(*RST_RequestParserDataCb) (RST_RequestParser*, const char *at, size_t length);
typedef int(*RST_RequestParserCb) (RST_RequestParser*);

typedef struct RST_RequestParserSettings {
    RST_RequestParserCb      on_message_begin;
    RST_RequestParserDataCb  on_url;
    RST_RequestParserDataCb  on_status;
    RST_RequestParserDataCb  on_header_field;
    RST_RequestParserDataCb  on_header_value;
    RST_RequestParserCb      on_headers_complete;
    RST_RequestParserDataCb  on_body;
    RST_RequestParserCb      on_message_complete;
} RST_RequestParserSettings;

RST_RequestParser* RST_requestparser_init();
int RST_requestparser_execute(RST_RequestParser *parser, RST_RequestParserSettings *settings, const char *data, size_t len);

void RST_requestparser_parse_url(const char *at, size_t len, size_t *plen, size_t *qoff, size_t *qlen);

#endif