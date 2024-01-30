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

git pull --recurse-submodules
pushd $git_root/1brc
  ./mvnw clean verify
  # 12 GB file created 2 mins+
  # Created file with 1,000,000,000 measurements in 261059 ms
  ./create_measurements.sh 1000000000
  ln -s measurements.txt $git_root/measurements.txt
popd