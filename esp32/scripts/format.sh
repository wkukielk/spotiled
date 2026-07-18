#!/bin/sh

find `pwd`/components \( -name managed_components -o -name build \) -prune -o -iname '*.h' -o -iname '*.cpp' -o -iname '*.hpp' -o -iname '*.tpp' -print0 \
    | xargs -0 clang-format --dry-run --Werror