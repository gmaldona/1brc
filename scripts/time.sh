#!/bin/bash

# 1 Billion Row Contest for Binghamton University CS 547.
# see: https://github.com/gunnarmorling/1brc
#
# author: Gregory Maldonado
# email : gmaldonado@cs.binghamton.edu
# date  : 2024-01-25
#
# Graduate student @ Thomas J. Watson College of Engineering and Applied
# Sciences, Binghamton University.

# time.sh -
#
# Compiles the src code using the grading flags and times the executable.
# eg.
#     real    0m0.085s
#     user    0m0.001s
#     sys     0m0.001s
#

git_root=$(git worktree list | cut -d' ' -f1)

pushd $git_root
   make
popd

time "$git_root/build/1brc" > /dev/null
