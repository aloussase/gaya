add_library(gaya_lib
    lexer.cpp
    span.cpp
    parser.cpp
    ast.cpp
    diagnostic.cpp
    env.cpp
    eval.cpp
    object.cpp
    file_reader.cpp
    types.cpp
    resolver.cpp
    object/arity.cpp
    object/call.cpp
    object/cmp.cpp
    object/equals.cpp
    object/hash.cpp
    object/is_callable.cpp
    object/is_comparable.cpp
    object/is_sequence.cpp
    object/is_truthy.cpp
    object/to_sequence.cpp
    object/to_string.cpp
    object/typeof.cpp
    object/next.cpp
    builtins/io.cpp
    builtins/core.cpp
    builtins/string.cpp
    builtins/array.cpp
    builtins/sequence.cpp
    builtins/dict.cpp
    builtins/math.cpp
    builtins/aoc.cpp)

target_compile_options(gaya_lib PRIVATE -Wall -Wextra
    $<$<CONFIG:Release>:-O2 -Wno-return-type>
    $<$<CONFIG:Debug>:-g>)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(GAYA_RUNTIME "runtime")
else()
    set(GAYA_RUNTIME "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/gaya/runtime")
endif ()

target_compile_definitions(gaya_lib PRIVATE
    GAYA_RUNTIME="${GAYA_RUNTIME}"
    DEBUG=$<IF:$<CONFIG:Debug>,1,0>)

target_link_libraries(gaya_lib PRIVATE
    fmt::fmt
    ffi
    dl
    ssl
    crypto
    httplib)

