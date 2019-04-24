#!/bin/bash

readonly REPETITIONS=10
readonly MIN_BUFFER_POWER=0
readonly MAX_BUFFER_POWER=16

make clean
make
./server -s -b 4000000 &

for (( i=$MIN_BUFFER_POWER; i<=$MAX_BUFFER_POWER; i++))
do 
    for (( j=0; j<$REPETITIONS; j++))
    do
        ./client -f kitten.png -b $((2 ** i)) -l
    done
done

pkill server