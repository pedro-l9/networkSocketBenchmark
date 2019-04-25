#!/bin/bash

readonly REPETITIONS=5
readonly MIN_BUFFER_POWER=5
readonly MAX_BUFFER_POWER=27

make test
pkill server
./server -b 143000000 -s &
echo "Server started..."

for (( i=$MIN_BUFFER_POWER; i<=$MAX_BUFFER_POWER; i++));
do 
    echo "Starting 2^$i"
    for (( j=0; j<$REPETITIONS; j++));
    do
        ./client -f 142MB.bin -b $((2 ** i)) -l -s
    done
    echo "2^$i Done"
done

pkill server

mv clientData.txt localClientData.txt