#ifndef _RST_WEBSOCKET
#define _RST_WEBSOCKET 1

#include "rstclient.h"

typedef struct RST_WebSocketConnectionInterface
{
    int connectionId;
    RST_Client *client;
} RST_WebSocketConnectionInterface;

typedef struct RST_ResourceWebSocket
{
    RST_ws_open_callback on_open;
    RST_ws_message_callback on_message;
    RST_ws_close_callback on_close;
} RST_ResourceWebSocket;

///////////// RST_WebSocketConnectionInterface ///////////////
void RST_websocketconnectioninterface_send(RST_WebSocketConnectionInterface* conn, const char* str);
void RST_websocketconnectioninterface_send_n(RST_WebSocketConnectionInterface* conn, const char* buf, int len);

///////////// RST_ResourceWebSocket ///////////////
RST_ResourceWebSocket* RST_resourcewebsocket_init();

#endif