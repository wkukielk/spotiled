#!/usr/bin/env sh
set -e

BASENAME=`basename ${PWD}`
OWN_CC="${PWD}/build/compile_commands.json"

if [ ${BASENAME} = "esp32" ]; then
    OTHER_CC="${PWD}/host_test/build/compile_commands.json"
    MERGED_CC="${PWD}/build/compile_commands_merged.json"
else
    OTHER_CC="${PWD}/../build/compile_commands.json"
    MERGED_CC="${PWD}/../build/compile_commands_merged.json"
fi

idf.py build "$@"

if [ -f "$OTHER_CC" ]; then
    python3 -c "
import json
a = json.load(open('$OWN_CC'))
b = json.load(open('$OTHER_CC'))
json.dump(a + b, open('$MERGED_CC', 'w'), indent=2)
"
    echo "Merged compile_commands.json -> ${MERGED_CC}"
else
    echo "Note: ${OTHER_CC} not built yet"
fi
