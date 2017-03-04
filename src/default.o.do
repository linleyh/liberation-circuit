#!/bin/sh
file=$1
gcc -O3 -fwrapv -o $3 -c ${file%.o}.c -MD -MF $2.deps
read DEPS <$2.deps
redo-ifchange ${DEPS#*:}
