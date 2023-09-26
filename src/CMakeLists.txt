add_executable(gaya main.cpp
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
    builtins/math.cpp)

target_compile_options(gaya PRIVATE -Wall -Wextra
    $<$<CONFIG:Release>:-O2 -Wno-return-type>
    $<$<CONFIG:Debug>:-g>)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(GAYA_RUNTIME "runtime")
else()
    set(GAYA_RUNTIME "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/gaya/runtime")
endif ()

target_compile_definitions(gaya PRIVATE
    GAYA_RUNTIME="${GAYA_RUNTIME}"
    DEBUG=$<IF:$<CONFIG:Debug>,1,0>)

target_link_libraries(gaya PRIVATE fmt::fmt ffi dl)

install(
    TARGETS gaya
    DESTINATION ${CMAKE_INSTALL_BINDIR}
    CONFIGURATIONS Release)

install(
    FILES 
    ${CMAKE_SOURCE_DIR}/runtime/base.gaya
    ${CMAKE_SOURCE_DIR}/runtime/arrays.gaya
    ${CMAKE_SOURCE_DIR}/runtime/dictionaries.gaya
    ${CMAKE_SOURCE_DIR}/runtime/functions.gaya
    ${CMAKE_SOURCE_DIR}/runtime/sequences.gaya
    ${CMAKE_SOURCE_DIR}/runtime/strings.gaya
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/gaya/runtime
    CONFIGURATIONS Release)
