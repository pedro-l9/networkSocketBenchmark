#!/bin/bash

########
#
# Script made to test socket download speeds based on buffer size
#
# Created by Pedro Lacerda<phenrique.lacerda1@gmail.com>
#
########

THIS=$(basename $0)

readonly REPETITIONS=10
readonly MIN_BUFFER_POWER=14
readonly MAX_BUFFER_POWER=27
SERVER_BUFFER=10000000
IS_LAN=false
DATA_FILE_NAME="remoteClientData-$REPETITIONS-$MIN_BUFFER_POWER~$MAX_BUFFER_POWER.txt"

function usage() {
    echo -e "$THIS 0.1\t-\tAuthor: Pedro Lacerda<phenrique.lacerda1@gmail.com>"
    echo "
    Usage:
    $THIS -f <arquivo> [-h <endereço_remoto>] [-b <server_buffer>] [-l]
    "
    exit -1
}


function startServer(){
    pkill server
    echo "Starting server"
    ./bin/server -b $SERVER_BUFFER -s&
    sleep 3
    echo "Server started..."
}

####-------CONFIGURATION
while getopts "f:h:b:l" OPT; do
    case $OPT in
        "f") FILENAME=$OPTARG;;
        "h") REMOTE_HOST=$OPTARG;;
        "b") SERVER_BUFFER=$OPTARG;;
        "l") IS_LAN=true;;
        "?") usage;;
    esac
done

####-------VALIDATION
if test -z "$FILENAME"
then 
    echo "SEM NOME DE ARQUIVO"
    usage
fi

####-------CLEANUP
make test

echo "File name: $FILENAME"
####-------SERVER STARTUP
if test -z "$REMOTE_HOST"
then
    echo "Server buffer size: $SERVER_BUFFER"
    startServer
    DATA_FILE_NAME="localClientData-$REPETITIONS-$MIN_BUFFER_POWER~$MAX_BUFFER_POWER.txt"
else
    echo "Remote server IP: $REMOTE_HOST"
    if $IS_LAN
    then
        DATA_FILE_NAME="lanClientData-$REPETITIONS-$MIN_BUFFER_POWER~$MAX_BUFFER_POWER.txt"
    fi
fi

for (( i=$MIN_BUFFER_POWER; i<=$MAX_BUFFER_POWER; i++));
do 
    echo "Starting 2^$i"
    for (( j=0; j<$REPETITIONS; j++));
    do
        if ! test -z "$REMOTE_HOST"
        then 
            ./bin/client -f $FILENAME -b $((2 ** i)) -h $REMOTE_HOST -l -s
        else    
            ./bin/client -f $FILENAME -b $((2 ** i)) -l -s
        fi
        if (( $? )); 
        then
            exit 1
        fi
    done
    echo "2^$i Done"
done

if test -z "$REMOTE_HOST"
then
    exec 2>/dev/null
    pkill server
    echo "Server stopped..."
fi

mv clientData.txt $DATA_FILE_NAME

exit 0