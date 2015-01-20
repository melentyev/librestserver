#include "tcpserver.h"
#include "rstutil.h"

int RST_selecttcpserver_server_loop(RST_SelectTcpServer* server);
void RST_selecttcpserver_release_client(RST_SelectTcpServer* server, int socket);
int RST_selecttcpserver_handle_client_read(RST_SelectTcpServer* server, int socket);
int RST_selecttcpserver_handle_client_write(RST_SelectTcpServer* server, int socket);

int RST_socket_set_non_block(int socket)
{
    SET_NONBLOCK(socket);
}

RST_SelectTcpServer* RST_selecttcpserver_init(uint16_t port, void* owner)
{
    RST_SelectTcpServer* server = (RST_SelectTcpServer*)RST_alloc(sizeof(RST_SelectTcpServer));
    server->writing_sockets = RST_set_init(RST_int_comparator, NULL);
    server->port = port;
    server->owner = owner;
    return server;
}

void RST_selecttcpserver_send(RST_SelectTcpServer* server, int socket, const char *buf, int size)
{
    RST_set_insert(server->writing_sockets, (void*)socket);
    send(socket, buf, size, 0);
}

int RST_selecttcpserver_start(RST_SelectTcpServer* server)
{
    if ((server->serv_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        RST_ERROR("Error: socket()");
    }
    server->max_connections = 5;
    server->server.sin_family = AF_INET;
    server->server.sin_port = htons(server->port);
    server->server.sin_addr.s_addr = INADDR_ANY;
    server->clients = RST_set_init(RST_int_comparator, NULL);
    server->finished_clients = RST_vector_init(NULL);
    int error = 0;
    if (bind(server->serv_sock, (struct sockaddr *)&(server->server), sizeof(server->server)) < 0)
    {   
#ifdef WIN32
        error = WSAGetLastError();
#endif
        fprintf(stderr, "Error: bind(), code=%d\n", error);
    }
    if (listen(server->serv_sock, server->max_connections) != 0)
    {
        RST_ERROR("Error: listen()");
    }
    return RST_selecttcpserver_server_loop(server);
}

int RST_selecttcpserver_server_loop(RST_SelectTcpServer* server)
{
    RST_socket_set_non_block(server->serv_sock);
    while (1)
    {
        fd_set readset;
        fd_set writeset;
        FD_ZERO(&readset);
        FD_ZERO(&writeset);
        FD_SET(server->serv_sock, &readset);

        for (RST_SetIterator it = RST_set_begin(server->clients); !RST_set_end(it); RST_set_iterator_next(&it))
        {
            int sock = (int)RST_set_iterator_value(it);
            FD_SET(sock, &readset);
            
        }
        
        for (RST_SetIterator it = RST_set_begin(server->writing_sockets); !RST_set_end(it); RST_set_iterator_next(&it))
        {
            int sock = (int)RST_set_iterator_value(it);
            FD_SET(sock, &writeset);
        }
        struct timeval timeout;
        timeout.tv_sec = 60;
        timeout.tv_usec = 0;

        int mx = RST_set_empty(server->clients) ? server->serv_sock : RST_max(server->serv_sock, (int)RST_set_max_element(server->clients));
        RST_log_console("select()");
        if (select(mx + 1, &readset, &writeset, NULL, &timeout) <= 0)
        {
            RST_ERROR("select");
        }

        if (FD_ISSET(server->serv_sock, &readset))
        {
            int client_sock;
            if ((client_sock = accept(server->serv_sock, NULL, NULL)) < 0)
            {
                RST_ERROR("accept");
            }
            RST_socket_set_non_block(client_sock);

            RST_set_insert(server->clients, (void*)client_sock);
            server->on_client_accepted(server, client_sock);
        }
        if (server->finished_clients->size > 0)
        {
            RST_vector_clear(server->finished_clients);
        }
        for (RST_SetIterator it = RST_set_begin(server->clients); !RST_set_end(it); RST_set_iterator_next(&it))
        {
            int sock = (int)RST_set_iterator_value(it);
            int release = 0;
            release |= (FD_ISSET(sock, &readset) && RST_selecttcpserver_handle_client_read(server, sock));
            if (FD_ISSET(sock, &writeset) && RST_set_contains(server->writing_sockets, (void*)sock) )
            {
                RST_set_erase(server->writing_sockets, (void*)sock);
                release |= RST_selecttcpserver_handle_client_write(server, sock);
            }
            if (release)
            {
                RST_selecttcpserver_release_client(server, sock);
            }
        }
        RST_selecttcpserver_release_clients(server);
    }
}

int RST_selecttcpserver_handle_client_read(RST_SelectTcpServer* server, int socket)
{
    int bytes_read = recv(socket, server->buf, RST_TCPSERVER_BUF_SIZE, 0);
    if (bytes_read <= 0)
    {
        return 1;
    }
    else
    {
        return server->on_data_received(server, socket, (uint8_t*)server->buf, bytes_read);
    }
}

int RST_selecttcpserver_handle_client_write(RST_SelectTcpServer* server, int socket)
{
    return server->on_data_sent(server, socket);
}

void RST_selecttcpserver_release_client(RST_SelectTcpServer* server, int socket)
{
    RST_vector_push_back(server->finished_clients, (void*)socket);
    
}

void RST_selecttcpserver_release_clients(RST_SelectTcpServer* server)
{
    for (int i = 0; i < server->finished_clients->size; i++)
    {
        int socket = (int)(server->finished_clients->data[i]);
        close(socket);
        RST_set_erase(server->clients, (void*)socket);
    }
}

void RST_selecttcpserver_release(RST_SelectTcpServer* server)
{
    RST_set_release(server->writing_sockets);
    RST_FREE(server, "RST_selecttcpserver_release::server");
}

/*
class TcpServer
{
protected:
    unsigned short _port;
    int _serv_sock, _max_connections;
    struct sockaddr_in _server;
    virtual void serverLoop() = 0;
public:
    TcpServer(unsigned short port) : _port(port)
    {
        if ((_serv_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        {
            throw("TcpServer()");
        }
        _max_connections = 5;
    }
    void Start()
    {
        
    }
    ~TcpServer()
    {
        if (_serv_sock >= 0)
        {
            close(_serv_sock);
        }
    }
};

class BlockingTcpServer : public TcpServer
{
public:
    BlockingTcpServer(unsigned short port) : TcpServer(port) {}
protected:
    socklen_t _namelen;
    struct sockaddr_in _client;
    virtual void serverLoop()
    {
        int client_sock;
        char buf[256];
        //while (true)
        //{
        _namelen = sizeof(_client);
        if ((client_sock = accept(server->serv_sock,
            (struct sockaddr *)&_client, &_namelen)) == -1)
        {
            throw("Accept()");
        }
        if (recv(client_sock, buf, sizeof(buf), 0) == -1)
        {
            throw("Recv()");
        }
        if (send(client_sock, buf, sizeof(buf), 0) < 0)
        {
            throw("Send()");
        }
        close(client_sock);
        //}
    }
};

class TcpServerEvents
{
public:
    virtual void client_accepted(int socket) = 0;
    virtual bool data_received(int socket, uint8_t data[], int recved) = 0;
    virtual bool data_sent(int socket) = 0;
    virtual void client_disconnected(int socket) = 0;
};

class NonBlockingTcpServer : public TcpServer
{
private:
    static const int BUF_SIZE = 1024;
    TcpServerEvents* m_callbacks;
    inline int set_non_block(int socket)
    {
        SET_NONBLOCK(socket);
    }
    void release_client(int socket)
    {
        close(socket);
        clients.erase(socket);
    }
public:
    NonBlockingTcpServer(unsigned short port, TcpServerEvents *callbacks)
        : TcpServer(port), m_callbacks(callbacks) {}
    void send(int socket, const char *buf, int size)
    {
        writing_sockets.insert(socket);
        ::send(socket, buf, size, 0);
    }
protected:
    socklen_t _namelen;
    struct sockaddr_in _client;
    std::set<int> clients, writing_sockets;
    char buf[BUF_SIZE];

    bool handle_client_read(int socket)
    {
        int bytes_read = recv(socket, buf, BUF_SIZE, 0);
        if (bytes_read <= 0)
        {
            return true;
        }
        else
        {
            try
            {
                return m_callbacks->data_received(socket, (uint8_t*)buf, bytes_read);
            }
            catch (StatusCode::Value code)
            {
                if (code == StatusCode::METHOD_NOT_ALLOWED)
                {
                    std::cerr << "METHOD_NOT_ALLOWED: " + __LINE__ << std::endl;
                }
                else
                {
                    std::cerr << "StatusCode error" + __LINE__ << std::endl;
                }
            }
            catch (...)
            {
                std::cerr << "Unknown exception: " + __LINE__ << std::endl;
            }
            return true;
        }
    }

    bool handle_client_write(int socket)
    {
        try
        {
            return m_callbacks->data_sent(socket);
        }
        catch (...)
        {
            std::cerr << "Unknown exception: " + __LINE__ << std::endl;
            return true;
        }
    }

    virtual void serverLoop()
    {
        
    }
};
#endif

*/