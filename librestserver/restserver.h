#ifndef _REST_SERVER
#define _REST_SERVER 1

#include <stdint.h>

#include "statuscode.h"
#include "rstutil.h"
#include "request.h"
#include "response.h"
#include "websocket.h"

void RST_socket_set_non_block(int socket);
#endif