#include <stdio.h>

#ifdef WIN32
#include <windows.h>
#endif

#include "rstpublic.h"
#include "zoshttpcommon.h"

RST_Service* service;

void get_method_handler(RST_Request* request, RST_Response *response)
{
    char mes[200] = "";
    const char* name = RST_request_get_query_param(request, "name");
    if (name != NULL) 
    {
        strcat(mes, "<h1>Hello, ");
        strncat(mes, name, 100);
    }
    else
    {
        strcat(mes, "<h1>It Works!</h1><p>Please, your name?</p>");
    }
    RST_response_set_body(response, mes);
    response->status_code = RST_STATUSCODE_OK;
    RST_response_set_status_message(response, "");
}

void get_picture(RST_Request* request, RST_Response *response)
{
    char path[255] = "C:\\Users\\user\\Downloads\\";
    const char* name = RST_request_get_query_param(request, "name");
    strcat(path, name);
    FILE *fp = fopen(path, "rb");
    fseek(fp, 0L, SEEK_END);
    int sz = ftell(fp);
    fseek(fp, 0L, SEEK_SET);
    char* cont = RST_ALLOC(sizeof(char) * sz, "FILE");
    fread(cont, 1, sz, fp);
    RST_map_insert(response->headers, RST_alloc_string("Content-Type"), RST_alloc_string("image/png"));
    RST_response_set_body_n(response, cont, sz);
    //RST_response_set_body_file(response, fp);
    response->status_code = RST_STATUSCODE_OK;
    RST_response_set_status_message(response, "OK");
}

void post_method_handler(RST_Request *request, RST_Response *response)
{
    FILE *fout = fopen("file.png", "wb");
    fwrite(request->body->buf, request->body->len, 1, fout);
    RST_response_set_body(response, "hello from post");
}

void ws_open(RST_WebSocketConnectionInterface *conn)
{
    RST_websocketconnectioninterface_send(conn, "hello, world");
}

void finish_handler(RST_Request *request, RST_Response *response) 
{
    RST_service_release(service); 
    exit(0);
}

void ws_message(RST_WebSocketConnectionInterface *conn, char *buf, int len)
{

}

void ws_close(RST_WebSocketConnectionInterface *conn)
{

}

int glob = 0;

#ifdef WIN32
int control_handler(unsigned int fdwCtrlType)
{
    RST_service_release(service);
    switch (fdwCtrlType)
    {
    case CTRL_C_EVENT:
        printf("Ctrl-C event\n\n");
        return FALSE;

        // CTRL-CLOSE: confirm that the user wants to exit. 
    case CTRL_CLOSE_EVENT:
        printf("Ctrl-Close event\n\n");
        return FALSE;
    default:
        return FALSE;
    }
}
#endif
/*
void test(int _, ...)
{
    struct Str {
        int x;
    };
    va_list vl;
    va_start(vl, _);
    int *a = &(va_arg(vl, int));
    double *x = (double*)a;
    printf("%lf", *x);
}
*/
int main(int argc, char **argv)
{
    RST_debug_init();
#ifdef WIN32
    WSADATA ws;
    if (FAILED(WSAStartup(MAKEWORD(1, 1), &ws)))
    {
        int error = WSAGetLastError();
    }
    SetConsoleCtrlHandler((PHANDLER_ROUTINE)control_handler, TRUE);
    /*if (SetConsoleCtrlHandler((PHANDLER_ROUTINE)control_handler, TRUE))
    {
        printf("Control handler set");
    }*/
#endif
    RST_Resource* testGET = RST_resource_init("/testget", HTTP_GET, &get_method_handler);
    RST_Resource* testGETfile = RST_resource_init("/testpic", HTTP_GET, &get_picture);
    RST_Resource* testPOST = RST_resource_init("/testpost", HTTP_POST, &post_method_handler);
    RST_Resource* testwebsock = RST_resource_websocket_init("/wsecho", &ws_open, &ws_message, &ws_close);
    service = RST_service_init(34739);
    
    RST_service_publish(service, testGET);
    RST_service_publish(service, testGETfile);
    RST_service_publish(service, testPOST);
    RST_service_publish(service, testwebsock);
    RST_log_console("RST_service_start");
    set_zos_http_common_resources(service);
    RST_service_start(service);
    

    /*
    //RST_service_release(service);
    test(0, 1.2);
    system("pause");
    thpool_t* threadpool;
    threadpool = thpool_init(2);
    puts("Adding 20 tasks to threadpool");
    int a = 54;
    for (int i = 0; i<10; i++){
        thpool_add_work(threadpool, (void*)task1, NULL);
        thpool_add_work(threadpool, (void*)task2, (void*)a);
    };


    puts("Will kill threadpool");
    
    Sleep(10000);
    system("pause");
    
    thpool_destroy(threadpool);
    */
    return 0;
}