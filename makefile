main: main.o g_config.o g_statistics.o g_requests.o dns_server.o
	gcc -o main -pthread -D_REENTRANT main.o g_config.o g_statistics.o g_requests.o dns_server.o  -Wall
main.o: main.c main.h
	gcc -c main.c -Wall
g_config.o: g_config.c main.h
	gcc -c g_config.c -Wall
g_requests.o: g_requests.c main.h
	gcc -c g_requests.c -Wall
g_statistics.o: g_statistics.c main.h
	gcc -c g_statistics.c -Wall
dns_server.o: dns_server.c main.h
	gcc -c dns_server.c -Wall
