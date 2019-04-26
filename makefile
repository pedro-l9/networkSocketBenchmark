CC=gcc
CFLAGS=-I.

.PHONY: all setup clean test

all: setup bin/client bin/server

setup:
	@mkdir -p download files bin

bin/client: src/client.c
	@$(CC) src/client.c -o bin/client

bin/server: src/server.c
	@$(CC) src/server.c -o bin/server

clean: 
	@rm -rf download bin clientData.txt

test: all
	@rm -rf clientData.txt
