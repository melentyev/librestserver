#ifndef _XOPEN_SOURCE_EXTENDED
#define _XOPEN_SOURCE_EXTENDED 1
#endif 
#ifndef _OPEN_THREADS
#define _OPEN_THREADS
#endif 


#include <stdio.h>

#ifdef WIN32
#include <windows.h>
#include <rstpublic.h>
#include <jsonparser.h>
#else
#include "rstpublic.h"
#include "jsonparser.h"
#endif

//#include "zoshttpcommon.h"

//#include "C-Thread-Pool-master\C-Thread-Pool-master\thpool.h"
//#include <pthread.h>

RST_Service* service;
char document_root[512];
char *zos_exec_lib;

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

void get_sleep(RST_Request* request, RST_Response *response)
{
    char mes[200] = "";
    strcat(mes, "<h1>It Works!</h1><p></p>");
    printf("Sleeeeping...\n");
    //Sleep(3000);
    RST_response_set_body(response, mes);
    response->status_code = RST_STATUSCODE_OK;
    RST_response_set_header(response, "Access-Control-Allow-Origin", "*");
    RST_response_set_status_message(response, "");
}

int cfileexists(const char * filename)
{
    /* try to open file to read */
    FILE *file;
    if (file = fopen(filename, "r"))
    {
        fclose(file);
        return 1;
    }
    return 0;
}

#define MAX_FILE_PATH 512

void genericHandler(RST_Request* request, RST_Response *response)
{
    char path[MAX_FILE_PATH + 1];
    if (strlen(document_root) + request->url->len <= MAX_FILE_PATH) 
    {
        strcpy(path, document_root);
        strcat(path, request->url->buf);
        char* clear_path = RST_str_replace(path, "..", "");
        if (cfileexists(clear_path))
        {
            FILE *fp = fopen(clear_path, "rb");
            RST_free(clear_path);
            fseek(fp, 0L, SEEK_END);
            int sz = ftell(fp);
            fseek(fp, 0L, SEEK_SET);
            char* cont = RST_ALLOC(sizeof(char) * sz, "FILE");
            int frres = fread(cont, 1, sz, fp);
            printf("FREAD RESULT (%d)", frres);
            fclose(fp);
            RST_response_set_body_n(response, cont, sz);
            response->http_major = 1;
            response->http_minor = 1;
            response->status_code = RST_STATUSCODE_OK;
            RST_response_set_status_message(response, "OK");
        }
        else 
        {
            RST_response_set_not_found(response, request, NULL);
        }
    }
    else 
    {
        RST_response_set_not_found(response, request, NULL);
    }
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
    RST_JsonValue *found = NULL, *data = RST_json_parse(request->body->buf, request->body->len);
    RST_String_builder *sb = RST_json_serialize(data);
    RST_Vector *rexx = RST_json_to_rexx_vars(data);
    FILE *f = fopen("rexx.txt", "w");
    //fwrite(rexx->buf, 1, rexx->len, f);
    fclose(f);
    //int res = RST_json_get_node(data, &found, RST_JS_KEY, "items", RST_JS_END);
    int c = 5;
}

void finish_handler(RST_Request *request, RST_Response *response)
{
    RST_service_release(service);
    exit(0);
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


void * sleepy(void *arg) {
Sleep((int)arg * 1000);
printf("Thread: %d\n", (int)arg);
}
*/

int main(int argc, char **argv)
{
    int port;
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
    port = 1117;
    strcpy(document_root, "public_html");
#else
    char buf[100];
    FILE *config = fopen("/u/ibmuser/restc/conf.txt", "r");
    fscanf(config, "%s %d\n", &buf, &port);
    zos_exec_lib = (char*)RST_alloc(sizeof(char) * 512);
    fscanf(config, "%s %s\n", &buf, zos_exec_lib);
    printf("zos_exec:(%s)\n", zos_exec_lib);
    strcpy(document_root, "/u/ibmuser/restc/data");
    fclose(config);
#endif
    

    RST_Resource* testGET = RST_resource_init("/testget", HTTP_GET, &get_method_handler);
    RST_Resource* testSleep = RST_resource_init("/testsleep", HTTP_GET, &get_sleep);
    RST_Resource* testGETfile = RST_resource_init("/testpic", HTTP_GET, &get_picture);
    RST_Resource* testPOST = RST_resource_init("/testpost", HTTP_POST, &post_method_handler);
    service = RST_service_init(port);

    RST_service_publish_generic_handler(service, genericHandler);
    RST_service_publish(service, testGET);
    RST_service_publish(service, testSleep);
    RST_service_publish(service, testGETfile);
    RST_service_publish(service, testPOST);
    RST_log_console("RST_service_start");
#ifndef WIN32
    set_zos_http_common_resources(service);
#endif
    RST_service_start(service);


    /*
    //RST_service_release(service);
    test(0, 1.2);
    thpool_t *tp = thpool_init(5);
    for (int i = 0; i < 7; i++) {
    thpool_add_work(tp, sleepy, (void*)(i + 1));
    }
    Sleep(10000);
    return 0;
    */
    system("pause");
    return 0;
}