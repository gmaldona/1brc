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

# retrieve.sh -
#
# Retrieves txt examples from 1BRC gitlab repository
# and places the examples in $git_root/samples. All txt files are git
# ignored.

git_root=$(git worktree list | cut -d' ' -f1)
cp -r $git_root/1brc/src/test/resources/samples/ $git_root/samples/ 2>/dev/null
echo "measurements*" > $git_root/samples/.gitignore