# Makefile for example dbx plugin

CC      = c99
#CFLAGS  = -Wc,xplink $(LIBS)
#LDFLAGS = -Wl,xplink
CFLAGS  = -c -Wc,xplink $(LIBS)
LIBS    =

OBJS = main request requestparser resource response rstclient rstutil service tcpserver wsconint zoshttpcommon http_parser

all: $(OBJS) bind

main: "./src/main.c"  
	$(CC) $(CFLAGS) "./src/main.c" 

request: "./src/request.c" "./src/request.h"
	$(CC) $(CFLAGS) "./src/request.c"

requestparser: "./src/requestparser.c" "./src/requestparser.h"
	$(CC) $(CFLAGS) "./src/requestparser.c"

resource: "./src/resource.c" "./src/resource.h"
	$(CC) $(CFLAGS) "./src/resource.c"

response: "./src/response.c" "./src/response.h"
	$(CC) $(CFLAGS) "./src/response.c"

rstclient: "./src/rstclient.c" "./src/rstclient.h"
	$(CC) $(CFLAGS) "./src/rstclient.c"

rstutil: "./src/rstutil.c" "./src/rstutil.h"
	$(CC) $(CFLAGS) "./src/rstutil.c" 

service: "./src/service.c" "./src/service.h"
	$(CC) $(CFLAGS) "./src/service.c"

tcpserver: "./src/tcpserver.c" "./src/tcpserver.h"
	$(CC) $(CFLAGS) "./src/tcpserver.c"

wsconint: "./src/wsconninterface.c" "./src/websocket.h"
	$(CC) $(CFLAGS) "./src/wsconninterface.c"

zoshttpcommon: "./src/zoshttpcommon.c" "./src/zoshttpcommon.h"
	$(CC) $(CFLAGS) "./src/zoshttpcommon.c"
	

http_parser: "./src/http_parser.c" "./src/http_parser.h"
	$(CC) $(CFLAGS) "./src/http_parser.c"
		
bind:
	$(CC) -o restcexe -Wl,xplink main.o request.o requestparser.o resource.o response.o rstclient.o rstutil.o service.o tcpserver.o wsconninterface.o zoshttpcommon.o http_parser.o
#"./src/Request.C" "./src/Resource.C" ./src/Response.C ./src/RestClient.C ./src/RestServerEvents.C ./src/Service.C ./src/StringUtil.C
	
#-o restexe

clean:
	rm -f *.dll
