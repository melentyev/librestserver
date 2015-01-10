#include "restserver.h"

void RST_websocketconnectioninterface_send(RST_WebSocketConnectionInterface* conn, const char* str)
{

}

void RST_websocketconnectioninterface_send_n(RST_WebSocketConnectionInterface* conn, const char* buf, int len)
{
    RST_client_send_async(conn->client, buf, len);
}