#ifdef WIN32
#include <windows.h>
#include <rstutil.h>
#include <jsonparser.h>
#include <irxexec.h>
#else
#include "rstutil.h"
#include "jsonparser.h"
#include "irxexec.h"
#endif

#include <stdio.h>

int main(void) {
    char res_buf[1024];
    char res_buf_addr[100];
    sprintf(res_buf_addr, "%X", res_buf);
    int rc;
    printf("Content-type: text/plain\n\n");
    printf("In main routine\n");
    RST_Vector *stmts = RST_vector_init(RST_cstr_dtor);
    RST_vector_push_back(stmts, "/*REXX*/");
    RST_vector_push_back(stmts, "PARSE ARG ARGVAL");
    RST_vector_push_back(stmts, "SAY 'HERE IN REXX ROUTINE'");
    RST_vector_push_back(stmts, "SER='FROM REXX DATA'");
    RST_vector_push_back(stmts, "R=STORAGE(ARGVAL,LENGTH(SER),SER)");
    RST_vector_push_back(stmts, "EXIT 42");
    RST_String_builder *rlt = NULL;
    rc = run_irxexec((char**)stmts->data, stmts->size, res_buf_addr, &rlt);
    printf("run_rexx: (%d) (data: (%d) (%s))\n", rc, rlt->len, rlt->buf);
    printf("res_buf (%s)\n", res_buf);
    if (rlt)
    {
        RST_string_builder_release(rlt);
    }
    return 0;
}