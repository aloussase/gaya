#!/bin/bash

[ -d build ] || { echo 'Run this script from the project root' >&2 && exit 1; }

echo 'Building fresh executable'
cmake --build build >/dev/null

echo -e '✅ Done building fresh executable\n'

GAYA_EXE=./build/src/gaya
[ -x ${GAYA_EXE} ] || { echo "${GAYA_EXE} is not an executable" >&2 && exit 1; }

[ -d examples ] || { echo 'examples is not a directory' >&2 && exit 1; }

successes=0
failures=0

echo -e 'Running tests\n'

for example in examples/*; do
    expected_output=`cat "$example" | perl -n -e "/Output: '(.*)'/ && print \\$1"`
    expected_output=`echo -e $expected_output` # Interpret line breaks.
    actual_output=`${GAYA_EXE} "$example"`

    if [ "$expected_output" = "$actual_output" ]; then
        successes=$((successes + 1))
        echo -e "✅ \x1b[1m\x1b[32m$example : All good\x1b[m"
    else
        failures=$((failures + 1))
        echo -e -n "❌ \x1b[1m\x1b[31m$example : "
        echo -n "$expected_output != $actual_output"
        echo -e "\x1b[m"
    fi
done

echo -e "\nsummary: $successes success(es), $failures failure(s)"
