CC=gcc
CFLAGS=-I.

.PHONY: all setup clean

all: setup client server

setup:
	@mkdir -p download

client: client.c
	@$(CC) client.c -o client

server: server.c
	@$(CC) server.c -o server

clean: 
	@rm -rf download client server clientData.txt
