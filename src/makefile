#Server
server: server.o logger.o sFunctionHelper.o stream.o
	gcc server.o stream.o logger.o sFunctionHelper.o -o myftpdExe/myftpd

server.o: server.c
	gcc -c server.c

sFunctionHelper.o: serverFunction/sFunctionHelper.c stream/stream.h logger/logger.h
	gcc -c "serverFunction/sFunctionHelper.c"

logger.o: logger/logger.h logger/logger.c 
	gcc -c "logger/logger.c"

#Client
client: client.o stream.o token.o cFunctionHelper.o
	gcc client.o stream.o token.o cFunctionHelper.o -o myftpExe/myftp

client.o: client.c
	gcc -c  client.c

cFunctionHelper.o: clientFunction/cFunctionHelper.c stream/stream.h
	gcc -c "clientFunction/cFunctionHelper.c"

token.o: clientFunction/token.c clientFunction/token.h
	gcc -c "clientFunction/token.c" 

#Server & Client
stream.o: stream/stream.c stream/stream.h
	gcc -c "stream/stream.c"
clean:
	rm *.o