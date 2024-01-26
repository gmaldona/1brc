#!/bin/bash

git_root=$(git worktree list | cut -d' ' -f1)
cp -r $git_root/1brc/src/test/resources/samples/ $git_root/samples/ 2>/dev/null
echo "measurements*" > $git_root/samples/.gitignore