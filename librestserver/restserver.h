#ifndef _REST_SERVER
#define _REST_SERVER 1

#include <stdint.h>

#include "statuscode.h"
#include "rstutil.h"
#include "request.h"
#include "response.h"
#include "websocket.h"

#ifdef WIN32 
    #define __e2a_l(buf, recved) {}
    #define __a2e_l(buf, recved) {}
#else
    #include <unistd.h>  
#endif

#define RST_PARSER_CHAR(c) (c)
#define RST_PARSER_STR(s) (s)

void RST_socket_set_non_block(int socket);
#endif