#!/bin/bash

# Usage: bash run.sh

gcc -o s.out sender.c
gcc -o r.out receiver.c
./r.out 8000 8080 0.5 & ./s.out 8080 8000 5 1 # Order is important