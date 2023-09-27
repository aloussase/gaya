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
    if [ "$expected_output" = "$actual_output" -a $? -eq 0 ]; then
        echo 'ok'
    else
        echo "$actual_output != $expected_output"
    fi
}

test_error()
{
    example="$1"
    result=`${GAYA_EXE} "$example"`
    [ $? -ne 0 ] && echo "ok"
}

test_assertions()
{
    example="$1"
    result=`${GAYA_EXE} "$example"`
    [ $? -eq 0 ] && echo "ok"
}

for test in tests/*.gaya; do
    is_output_test=`cat "$test" | grep 'Output:'`
    is_error_test=`cat "$test" | grep 'Expect error'`

    echo "Testing $test"

    if [ ! -z "$is_output_test" ]; then
        test_result=`test_output "$test"`
    elif [ ! -z "$is_error_test" ]; then
        test_result=`test_error "$test"`
    fi

    if [ "$test_result" = "ok" ]; then
        successes=$((successes + 1))
        echo -e "✅ \x1b[1m\x1b[32m$test : All good\x1b[m"
    else
        failures=$((failures + 1))
        echo -e -n "❌ \x1b[1m\x1b[31m$test : "
        if [ ! -z "$is_output_test" ]; then
            echo -n "$test_result"
        elif [ ! -z "$is_error_test" ]; then
            echo -n "expected error"
        fi
        echo -e "\x1b[m"
    fi

    echo
done

echo -e "\nsummary: $successes success(es), $failures failure(s)"
