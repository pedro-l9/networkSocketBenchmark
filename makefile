CC=gcc
CFLAGS=-I.

.PHONY: all setup clean test

all: setup bin/clienteFTP bin/servidorFTP

setup:
	@mkdir -p download files bin

bin/clienteFTP: src/clienteFTP.c
	@$(CC) src/clienteFTP.c -o bin/clienteFTP

bin/servidorFTP: src/servidorFTP.c
	@$(CC) src/servidorFTP.c -o bin/servidorFTP

clean: 
	@rm -rf download bin clientData.txt

test: all
	@rm -rf clientData.txt
