#!/bin/bash

gcc server.c -o server
gcc client.c -o client

./server 8080 1024