#include "rstpublic.h"

#pragma linkage(ISPLINK,OS);

extern int ISPLINK();

void get_zos_dslist(RST_Request* request, RST_Response *response)
{
    const char* dsquery = RST_request_get_query_param(request, "dsquery");

    char mes[512] = "";
    char conv_buf[256] = "";
    char cmd_buf[256]  = "";
    char xname[256] = "                                                                                           ";
    const char* name = RST_request_get_query_param(request, "name");
    if (name != NULL)
    {
        strcat(mes, "<h1>Hello, ");
        strncat(mes, name, 100);
    }
    else
    {
        strcpy(cmd_buf, "LEVEL(");
        strcat(cmd_buf, "MELEN.INT*");
        strcat(cmd_buf, ")");
        /*int RCODE = ISPLINK("LMDINIT ", "LISTID(IDV) ", cmd_buf);
        printf("RCODE: %d\n", RCODE);
        strcat(mes, "<h1>It Works!</h1><p>Please, your name?</p>");
        RCODE = ISPLINK("VDEFINE ", "DSVAR ", xname, "CHAR ", 50);
        int RCODE = ISPLINK("ISPEXEC LMDLIST LISTID("IDV") OPTION(LIST) DATASET(DSVAR)"*/
    }
    RST_response_set_body(response, mes);
    response->status_code = RST_STATUSCODE_OK;
    RST_response_set_status_message(response, "");
}

void get_zos_file(RST_Request* request, RST_Response *response) {

}

void post_zos_submit(RST_Request* request, RST_Response *response) {

}

void set_zos_http_common_resources(RST_Service *service)
{
    RST_Resource* dslist = RST_resource_init("/dslist", HTTP_GET, get_zos_dslist);
    RST_Resource* file = RST_resource_init("/file", HTTP_GET, get_zos_file);
    RST_Resource* submit = RST_resource_init("/submit", HTTP_POST, post_zos_submit);
    RST_service_publish(service, dslist);
}