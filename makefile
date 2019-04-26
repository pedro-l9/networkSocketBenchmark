CC=gcc
CFLAGS=-I.

.PHONY: all setup clean test

all: setup client server

setup:
	@mkdir -p download files bin

client: src/client.c
	@$(CC) src/client.c -o bin/client

server: src/server.c
	@$(CC) src/server.c -o bin/server

clean: 
	@rm -rf download bin clientData.txt

test: all
	@rm -rf clientData.txt
