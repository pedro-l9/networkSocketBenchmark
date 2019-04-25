CC=gcc
CFLAGS=-I.

.PHONY: all setup clean

all: setup client server

setup:
	@mkdir -p download files

client: src/client.c
	@$(CC) src/client.c -o client

server: src/server.c
	@$(CC) src/server.c -o server

clean: 
	@rm -rf download client server clientData.txt
