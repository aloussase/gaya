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

test_output() 
{
    example="$1"
    expected_output=`cat "$example" | perl -n -e "/Output: '(.*)'/ && print \\$1"`
    expected_output=`echo -e $expected_output` # Interpret line breaks.
    actual_output=`${GAYA_EXE} "$example"`
    [ "$expected_output" = "$actual_output" ] && echo 'ok'
}

test_error()
{
    example="$1"
    result=`${GAYA_EXE} "$example"`
    [ $? -ne 0 ] && echo "ok"
}

for example in examples/*; do
    is_output_test=`cat "$example" | grep 'Output:'`
    is_error_test=`cat "$example" | grep 'Expect error'`

    if [ -n "$is_output_test" ]; then
        test_result=`test_output "$example"`
    elif [ -n "$is_error_test" ]; then
        test_result=`test_error "$example"`
    fi

    if [ "$test_result" = "ok" ]; then
        successes=$((successes + 1))
        echo -e "✅ \x1b[1m\x1b[32m$example : All good\x1b[m"
    else
        failures=$((failures + 1))
        echo -e -n "❌ \x1b[1m\x1b[31m$example : "
        if [ -n "$is_output_test" ]; then
            echo -n "$expected_output != $actual_output"
        else 
            echo -n "expected error"
        fi
        echo -e "\x1b[m"
    fi
done

echo -e "\nsummary: $successes success(es), $failures failure(s)"
