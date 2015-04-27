#define THREAD_FOR_HANDLER

#include "tcpserver.h"
#include "rstutil.h"

#include <assert.h>

#ifndef WIN32
extern int errno;
#endif

#define LOCKED_OPERATION(mut, op) { MLOCK(mut); (op); MUNLOCK(mut); }
#define MLOCK(mut) pthread_mutex_lock(&(mut));
#define MUNLOCK(mut) pthread_mutex_unlock(&(mut));
//
//#define SCHEDULE2(tp, func, p1, p2) {                           \
//    void **ps = (void**)RST_alloc(sizeof(void*) * 2);           \
//    ps[0] = (void*)(p1);                                        \    
//    ps[1] = (void*)(p2);                                        \
//    thpool_add_work(tp, func, (void*)ps);                       \
//}

#ifndef USE_THREAD_POOL
thpool_t* thpool_init(int n) { return NULL; }
void thpool_destroy(int *tp){}
#endif



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
    server->writers_data = RST_map_init(RST_int_comparator, NULL, RST_free);
    server->port = port;
    server->owner = owner;
    pthread_mutex_init(&server->mutex_clients, NULL);
    pthread_mutex_init(&server->mutex_writers, NULL);
    return server;
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

#ifdef THREAD_FOR_HANDLER

typedef struct WriterData {
    const char *buf;
    int len;
} WriterData;

void RST_selecttcpserver_send(RST_SelectTcpServer* server, int socket, const char *buf, int size)
{
    int sentnow;
    WriterData  *wd = RST_ALLOC_STRUCT(WriterData, "WriterData");
    
    MLOCK(server->mutex_writers);
    
    RST_set_insert(server->writing_sockets, (void*)socket); //TODO кажется может быть ошибка
    sentnow = send(socket, buf, size, 0);
    wd->buf = buf + sentnow;
    wd->len = size - sentnow;
    RST_map_insert(server->writers_data, (void*)socket, (void*)wd);

    MUNLOCK(server->mutex_writers);

    printf("DOING SEND (sentnow=%d)\n", sentnow);
}

int RST_selecttcpserver_handle_client_read(RST_SelectTcpServer* server, int socket)
{
    int bytes_read = recv(socket, server->buf, RST_TCPSERVER_BUF_SIZE, 0);
    assert(bytes_read >= 0);
    if (bytes_read == 0)
    {
        return 0;
    }
    else
    {
        printf("RST_selecttcpserver_handle_client_read::on_data_received func(%X)\n", server->on_data_received);
        return server->on_data_received(server, socket, (uint8_t*)server->buf, bytes_read);
    }
}

int RST_selecttcpserver_handle_client_write(RST_SelectTcpServer* server, int socket)
{
    
    WriterData *wd;
    RST_log_console("RST_selecttcpserver_handle_client_write\n");
    RST_map_find(server->writers_data, (void*)socket, (void**)&wd);
    if (wd->len > 0)
    {
        MLOCK(server->mutex_writers);
        
        int sentnow = send(socket, wd->buf, wd->len, 0);
        wd->buf = wd->buf + sentnow;
        wd->len = wd->len - sentnow;
        
        MUNLOCK(server->mutex_writers);

        printf("DOING SEND (sentnow=%d)\n", sentnow);
        return 0;
    }
    else
    {
        return server->on_data_sent(server, socket);
    }
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


int RST_selecttcpserver_server_loop(RST_SelectTcpServer* server)
{
    server->threadpool = thpool_init(5);
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

        MLOCK(server->mutex_writers);
            for (RST_SetIterator it = RST_set_begin(server->writing_sockets); !RST_set_end(it); RST_set_iterator_next(&it))
            {
                int sock = (int)RST_set_iterator_value(it);
                FD_SET(sock, &writeset);
            }
        MUNLOCK(server->mutex_writers);

        struct timeval timeout;
        timeout.tv_sec = 120;
        timeout.tv_usec = 0;

        int mx = RST_set_empty(server->clients) ? server->serv_sock : RST_max(server->serv_sock, (int)RST_set_max_element(server->clients));
        RST_log_console("select()");
        if (select(mx + 1, &readset, &writeset, NULL, &timeout) <= 0)
        {
            //fprintf(stderr, "Error: select(), code=%d\n", WSAGetLastError());
            printf("Error: select(), code=%d\n", errno);
            RST_ERROR("select");
            
        }
        RST_log_console("select() done");

        if (FD_ISSET(server->serv_sock, &readset))
        {
            int client_sock;
            if ((client_sock = accept(server->serv_sock, NULL, NULL)) < 0)
            {
                RST_ERROR("accept");
            }
            RST_socket_set_non_block(client_sock);

            //LOCKED_OPERATION(server->mutex_clients, (RST_set_insert(server->clients, (void*)client_sock)));
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
            assert(!(FD_ISSET(sock, &readset) && release));
            if (FD_ISSET(sock, &writeset) && RST_set_contains(server->writing_sockets, (void*)sock))
            {
                if (RST_selecttcpserver_handle_client_write(server, sock)) 
                {
                    MLOCK(server->mutex_writers);
                    RST_set_erase(server->writing_sockets, (void*)sock);
                    RST_map_erase(server->writers_data, (void*)sock);
                    MUNLOCK(server->mutex_writers);

                    release = 1;
                }
            }
            if (release)
            {
                RST_selecttcpserver_release_client(server, sock);
            }
        }
        RST_selecttcpserver_release_clients(server);
    }
    printf("SERVER_LOOP_END\n");
}


#else

void* scheduled_data_received(void **params) 
{
    RST_SelectTcpServer* server = (RST_SelectTcpServer*)params[0];
    int socket = (int)params[1];
    RST_free(params);
    return NULL;
}


void RST_selecttcpserver_send(RST_SelectTcpServer* server, int socket, const char *buf, int size)
{
    LOCKED_OPERATION(server->mutex_writers, RST_set_insert(server->writing_sockets, (void*)socket));
    send(socket, buf, size, 0);
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

            LOCKED_OPERATION(server->mutex_clients, (RST_set_insert(server->clients, (void*)client_sock)));
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
                LOCKED_OPERATION(server->mutex_writers, RST_set_erase(server->writing_sockets, (void*)sock));
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

#endif

void RST_selecttcpserver_release(RST_SelectTcpServer* server)
{
    pthread_mutex_destroy(&server->mutex_writers);
    pthread_mutex_destroy(&server->mutex_clients);
    RST_set_release(server->writing_sockets);
    thpool_destroy(server->threadpool);
    RST_FREE(server, "RST_selecttcpserver_release::server");
}
