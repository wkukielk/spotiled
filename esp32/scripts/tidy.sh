#!/bin/sh

find `pwd`/components/ -type d \( -name '*_notidy' -o -name '*test' -o -name '*_ht' \) -prune -o -name '*.cpp' -print0 \
    | xargs -0 clang-tidy -p build/ --warnings-as-errors=* main/main.cpp
