#!/bin/bash

# make && pintos --filesys-size=2 -p tests/userprog/args-multiple -a args-multiple -- -q -f extract run 'args-multiple some arguments for you!'

# "create-empty" "create-long" "create-normal" "create-exists" "open-missing" "open-normal" "open-twice" "close-normal"
# "wait-simple" "wait-twice" 
# "close-stdin" "close-stdout" "close-bad-fd" "close-twice"

# "create-null" "open-null" "open-empty"

# "read-normal" "read-zero"

# "write-normal" "write-zero"
# file=""
# for file in "no-vm/multi-oom"
# do
#   rm build/tests/userprog/${file}.output
#   make build/tests/userprog/${file}.result
#   cp build/tests/userprog/${file}.output ./
# done

file=""
for file in "no-vm/multi-oom"
do
  rm build/tests/userprog/${file}.output
  make build/tests/userprog/${file}.result
  cp build/tests/userprog/${file}.output ./
done