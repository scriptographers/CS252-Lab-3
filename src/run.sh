#!/bin/bash

# Usage: bash run.sh <Senders Port> <Receivers Port> <Number of packets> <Timeout> <Drop probability>
# Example: "bash run.sh 8080 8000 3 1 0.2"

N_ARGS=$#
if [ $N_ARGS -ne 5 ] 
then
    echo "Usage: bash $0 <Senders Port> <Receivers Port> <Number of packets> <Timeout> <Drop probability>"
    exit 1
fi 

SPORT=$1
RPORT=$2
N_PKTS=$3
TIMEOUT=$4
DROP_PROB=$5

# Compile:
gcc -o s.out sender.c
gcc -o r.out receiver.c

# Run:
./r.out $RPORT $SPORT $DROP_PROB & # Start the receiver
./s.out $SPORT $RPORT $TIMEOUT $N_PKTS & # Start the sender in parallel