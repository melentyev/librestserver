# Makefile for example dbx plugin

CC      = c99
#CFLAGS  = -Wc,xplink $(LIBS)
#LDFLAGS = -Wl,xplink
CFLAGS  = -c -Wc,xplink $(LIBS)
LIBS    =

OBJS = resource rstclient rstutil service request tcpserver wsconint response main http_parser

all: $(OBJS) bind

main: "./src/main.c"  
	$(CC) $(CFLAGS) "./src/main.c" 

request: "./src/request.c" "./src/request.h"
	$(CC) $(CFLAGS) "./src/request.c"

response: "./src/response.c" "./src/response.h"
	$(CC) $(CFLAGS) "./src/response.c"

resource: "./src/resource.c" "./src/resource.h"
	$(CC) $(CFLAGS) "./src/resource.c"

rstclient: "./src/rstclient.c" "./src/rstclient.h"
	$(CC) $(CFLAGS) "./src/rstclient.c"

rstutil: "./src/rstutil.c" "./src/rstutil.h"
	$(CC) $(CFLAGS) "./src/rstutil.c" 

service: "./src/service.c" "./src/service.h"
	$(CC) $(CFLAGS) "./src/service.c"

tcpserver: "./src/tcpserver.c" "./src/tcpserver.h"
	$(CC) $(CFLAGS) "./src/tcpserver.c"

wsconint: "./src/wsconninterface.c"
	$(CC) $(CFLAGS) "./src/wsconninterface.c"

http_parser: "./src/http_parser.c" "./src/http_parser.h"
	$(CC) $(CFLAGS) "./src/http_parser.c"


		
bind:
	$(CC) -o restcexe -Wl,xplink request.o response.o resource.o rstclient.o rstutil.o main.o
#"./src/Request.C" "./src/Resource.C" ./src/Response.C ./src/RestClient.C ./src/RestServerEvents.C ./src/Service.C ./src/StringUtil.C
	
#-o restexe

clean:
	rm -f *.dll
