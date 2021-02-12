#!/bin/bash

# Usage: bash run.sh

gcc -o s.out sender.c 
gcc -o r.out receiver.c 
./r.out & ./s.out 8000 8000 1 1 # Order is important