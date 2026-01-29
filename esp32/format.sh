#!/bin/sh

find components \( -name managed_components -o -name BMPxx -o -name build \) -prune -o -iname '*.h' -o -iname '*.cpp' -o -iname '*.hpp' -print0 \
    | xargs -0 clang-format --dry-run --Werror