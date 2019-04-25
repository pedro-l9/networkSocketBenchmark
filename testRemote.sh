#!/bin/bash

readonly REPETITIONS=20
readonly MIN_BUFFER_POWER=0
readonly MAX_BUFFER_POWER=27

make test

for (( i=$MIN_BUFFER_POWER; i<=$MAX_BUFFER_POWER; i++));
do 
    for (( j=0; j<$REPETITIONS; j++));
    do
        ./client -f kitten.png -b $((2 ** i)) -h 54.200.107.56 -l -s
    done
done

mv clientData.txt "remoteClientData-$REPETITIONS-$MIN_BUFFER_POWER-$MAX_BUFFER_POWER.txt"