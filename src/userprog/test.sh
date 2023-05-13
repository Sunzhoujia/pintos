#!/bin/bash

# make && pintos --filesys-size=2 -p tests/userprog/args-multiple -a args-multiple -- -q -f extract run 'args-multiple some arguments for you!'

file=""
for file in "exec-once" "exec-multiple" "exec-arg" "exec-missing" "wait-killed"
do
  rm build/tests/userprog/${file}.output
  make build/tests/userprog/${file}.result
  cp build/tests/userprog/${file}.output ./
done