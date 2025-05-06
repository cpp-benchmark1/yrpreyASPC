CC = gcc
CFLAGS = -Wall -Wextra
LDFLAGS = 

all: server_cwe415 test_client_cwe415

server_cwe415: server_cwe415.o buffer_manager_cwe415.o
	$(CC) $(LDFLAGS) -o server_cwe415 server_cwe415.o buffer_manager_cwe415.o

test_client_cwe415: test_client_cwe415.o
	$(CC) $(LDFLAGS) -o test_client_cwe415 test_client_cwe415.o

server_cwe415.o: server_cwe415.c buffer_manager_cwe415.h
	$(CC) $(CFLAGS) -c server_cwe415.c

buffer_manager_cwe415.o: buffer_manager_cwe415.c buffer_manager_cwe415.h
	$(CC) $(CFLAGS) -c buffer_manager_cwe415.c

test_client_cwe415.o: test_client_cwe415.c
	$(CC) $(CFLAGS) -c test_client_cwe415.c

clean:
	rm -f *.o server_cwe415 test_client_cwe415 