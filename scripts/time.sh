#!/bin/bash

# time.sh -
#
# Compiles the src code using the grading flags and times the executable.
# eg.
#     real    0m0.085s
#     user    0m0.001s
#     sys     0m0.001s
#

git_root=$(git worktree list | cut -d' ' -f1)
mkdir -p $git_root/build

FLAGS="-std=c++17 -Wall -Wextra -pedantic -o"
OUT="$git_root/build/1BRC"

g++ $FLAGS $OUT $git_root/1brc.cpp

time "$OUT" > /dev/null
