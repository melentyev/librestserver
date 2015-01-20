#include "rstutil.h"
#include "restserver.h"

#define RST_JSON_ASSERT(ex) { if (!(ex)) { return NULL; } }

typedef enum RST_JsonToken {
    RST_js_null,
    RST_js_num,
    RST_js_string,
    RST_js_array,
    RST_js_object
} RST_JsonToken;

typedef struct RST_JsonValue 
{
    RST_JsonToken token;
    union {
        double jnum;
        RST_String_builder *jstring;
        RST_Map *jobject;
        RST_Vector *jarray;
    } val;
} RST_JsonValue;

#define RST_JSON_VALUE_INIT(t, f, v) {                                                          \
    RST_JsonValue *value = RST_ALLOC_STRUCT(RST_JsonValue, "RST_jsonvalue_init_str::value"); \
    value->token = (t); value->val.f = (v);                                             \
    return value;                                                                            \
} 

RST_JsonValue *RST_jsonvalue_init_str(RST_String_builder *sb) { RST_JSON_VALUE_INIT(RST_js_string, jstring, sb); }
RST_JsonValue *RST_jsonvalue_init_obj(RST_Map *obj) { RST_JSON_VALUE_INIT(RST_js_string, jobject, obj); }
RST_JsonValue *RST_jsonvalue_init_num(double num) { RST_JSON_VALUE_INIT(RST_js_null, jnum, num); }
RST_JsonValue *RST_jsonvalue_init_null() { RST_JSON_VALUE_INIT(RST_js_null, jnum, 0.0); }

void RST_jsonvalue_release(RST_JsonValue *val) 
{
    switch (val->token) 
    {
    case RST_js_null: case RST_js_num: break;
    case RST_js_string: RST_string_builder_release(val->val.jstring); break;
    case RST_js_array: RST_vector_release(val->val.jarray); break;
    case RST_js_object: RST_map_release(val->val.jobject); break;
    }
    RST_free_tag(val, "RST_jsonvalue_release::val");
}


void RST_json_dtor(void *x)
{
    RST_jsonvalue_release((RST_JsonValue*)x);

}

typedef struct RST_JsonParser 
{
    char *pos;
    int len;
} RST_JsonParser;

RST_JsonValue *RST_jsonparser_parse_string(RST_JsonParser *parser);
RST_JsonValue *RST_jsonparser_parse_object(RST_JsonParser *parser);
RST_JsonValue *RST_jsonparser_parse_array(RST_JsonParser *parser);
RST_JsonValue *RST_jsonparser_parse_num(RST_JsonParser *parser);
RST_JsonValue *RST_jsonparser_parse_null(RST_JsonParser *parser);

RST_JsonParser RST_jsonparser_init() {
    RST_JsonParser parser;
    parser.pos = 0;
    parser.len = 0;
    return parser;
}

#define RST_JSON_PARSER_CONSUME(p, n) { for (int i = 0; i < n; i++) {(p)->len--; (p)->pos++;} }



RST_String_builder *RST_jsonparser_pull_string(RST_JsonValue *val) {
    RST_JSON_ASSERT(val != NULL);
    RST_String_builder *sb = val->val.jstring;
    RST_free_tag(val, "RST_jsonparser_pull_string::val");
    return sb;
}

void RST_jsonparser_skip_spaces(RST_JsonParser *parser)
{
    while (parser->len > 0 && *(parser->pos) == RST_PARSER_CHAR(' ')) 
    {
        RST_JSON_PARSER_CONSUME(parser, 1);
    }
}

RST_JsonValue *RST_jsonparser_parse_abstract(RST_JsonParser *parser)
{
    RST_jsonparser_skip_spaces(parser);
    RST_JSON_ASSERT(parser->len > 0);
    if ((*parser->pos) == RST_PARSER_CHAR('{')) 
    {
        return RST_jsonparser_parse_object(parser);
    }
    else if ((*parser->pos) == RST_PARSER_CHAR('[')) 
    {
        return RST_jsonparser_parse_array(parser);
    }
    else if ((*parser->pos) == RST_PARSER_CHAR('"'))
    {
        return RST_jsonparser_parse_string(parser);
    }
    else if (isdigit((*parser->pos))) 
    {
        return RST_jsonparser_parse_string(parser);
    }
    return NULL;
}

#define RST_JSON_APPEND_ESCAPED(c) { RST_string_builder_append_char(sb, RST_PARSER_CHAR(c)); RST_JSON_PARSER_CONSUME(parser, 1); escaped = 0; }

RST_JsonValue *RST_jsonparser_parse_string(RST_JsonParser *parser)
{
    RST_JSON_ASSERT(*parser->pos == RST_PARSER_CHAR('"'));
    int loop_done = 0, escaped = 0;
    RST_String_builder *sb = RST_string_builder_init();
    while (parser->len > 0 && !loop_done)
    {
        if (*parser->pos == RST_PARSER_CHAR('\"') && !escaped) { RST_JSON_PARSER_CONSUME(parser, 1); return RST_jsonvalue_init_str(sb);  }
        else if (*parser->pos == RST_PARSER_CHAR('\\') && !escaped) { escaped = 1; RST_JSON_PARSER_CONSUME(parser, 1); }
        else if (*parser->pos == RST_PARSER_CHAR('\\') && escaped) { RST_JSON_APPEND_ESCAPED('\\'); }
        else if (*parser->pos == RST_PARSER_CHAR('n') && escaped) { RST_JSON_APPEND_ESCAPED('\n'); }
        else if (*parser->pos == RST_PARSER_CHAR('t') && escaped) { RST_JSON_APPEND_ESCAPED('\t'); }
        else if (*parser->pos == RST_PARSER_CHAR('r') && escaped) { RST_JSON_APPEND_ESCAPED('\r'); }
        else if (*parser->pos == RST_PARSER_CHAR('\"') && escaped) { RST_JSON_APPEND_ESCAPED('\"'); }
        else { RST_JSON_APPEND_ESCAPED(*parser->pos); }
        
    }
    return NULL;
}

RST_JsonValue *RST_jsonparser_parse_object(RST_JsonParser *parser) 
{
    RST_JSON_ASSERT(*parser->pos == RST_PARSER_CHAR('{'));
    RST_Map *m = RST_map_init(RST_cstr_comparator, RST_cstr_dtor, RST_json_dtor);
    RST_String_builder *sb = NULL;
    RST_JsonValue *parsed = NULL;
    int done = 0;
    while (!done) 
    {
        RST_jsonparser_skip_spaces(parser);
        if (!RST_jsonparser_parse_string(parser)) 
        {
            RST_map_release()
        }
    }
    return m;
}


RST_JsonValue *RST_jsonparser_parse_null(RST_JsonParser *parser) 
{
    RST_JSON_ASSERT(parser->len >= 4);
    strncmp(parser->pos, RST_PARSER_STR("null"), 4);
    RST_JSON_PARSER_CONSUME(parser, 4);
    return RST_jsonvalue_init_null();
}

RST_JsonValue *RST_json_parse(char *buf, int len)
{
    RST_JsonParser parser = RST_jsonparser_init();
    parser.pos = buf;
    parser.len = len;
    RST_JsonValue *value = RST_jsonparser_parse_abstract(&parser);
    RST_JSON_ASSERT(value != NULL);
    RST_jsonparser_skip_spaces(&parser);
    RST_JSON_ASSERT(parser.len == 0);
    return value;
}