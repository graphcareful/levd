#!/bin/bash
set -e

cur=$(pwd)
git_root=$(git rev-parse --show-toplevel)
cd $git_root
for f in $(git ls-files -- '*.h' '*.cpp'); do
    clang-format -i ${f};
done
cd $cur
