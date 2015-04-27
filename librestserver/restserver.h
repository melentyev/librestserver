#ifndef _REST_SERVER
#define _REST_SERVER 1

#ifndef _XOPEN_SOURCE_EXTENDED
#define _XOPEN_SOURCE_EXTENDED 1
#endif 
#ifndef _OPEN_THREADS
#define _OPEN_THREADS
#endif 


#include "statuscode.h"
#include "rstutil.h"
#include "request.h"
#include "response.h"
#include "websocket.h"

#include <stdint.h>

#ifdef WIN32 
    #define __e2a_l(buf, recved) {}
    #define __a2e_l(buf, recved) {}
#else
    #include <unistd.h>  
#endif

void RST_socket_set_non_block(int socket);
#endif