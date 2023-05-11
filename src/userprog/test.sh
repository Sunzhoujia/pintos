#!/bin/bash

# make && pintos --filesys-size=2 -p tests/userprog/args-multiple -a args-multiple -- -q -f extract run 'args-multiple some arguments for you!'
file="args-none"
rm build/tests/userprog/${file}.output
make build/tests/userprog/${file}.result
cp build/tests/userprog/${file}.output ./