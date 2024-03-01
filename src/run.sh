#!/bin/bash
make clean
make
echo '#!#! Starting server... #!#!'
./server &
while true; do continue; done
