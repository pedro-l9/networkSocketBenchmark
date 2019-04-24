#!/bin/bash

readonly REPETITIONS=30
readonly MIN_BUFFER_POWER=5
readonly MAX_BUFFER_POWER=27

make clean
make
pkill server
./server -b 135000000 &

for (( i=$MIN_BUFFER_POWER; i<=$MAX_BUFFER_POWER; i++));
do 
    for (( j=0; j<$REPETITIONS; j++));
    do
        ./client -f kitten.png -b $((2 ** i)) -l -s
    done
done
pkill server