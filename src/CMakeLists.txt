add_executable(gaya main.cpp
    lexer.cpp
    span.cpp
    parser.cpp
    ast.cpp
    diagnostic.cpp
    env.cpp
    eval.cpp
    object.cpp
    repl.cpp
    file_reader.cpp
    object/arity.cpp
    object/call.cpp
    object/cmp.cpp
    object/equals.cpp
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
    builtins/sequence.cpp)

target_compile_options(gaya PRIVATE -Wall -Wextra
    $<$<CONFIG:Release>:-Os>
    $<$<CONFIG:Debug>:-g>)


set(GAYA_RUNTIME_STDLIB
    "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/gaya/runtime/stdlib.gaya")

target_compile_definitions(gaya PRIVATE
    GAYA_STDLIB_PATH=$<IF:$<CONFIG:Release>,"${GAYA_RUNTIME_STDLIB}","runtime/stdlib.gaya">
    DEBUG=$<IF:$<CONFIG:Debug>,1,0>)

target_link_libraries(gaya PRIVATE fmt::fmt readline)

install(
    TARGETS gaya
    DESTINATION ${CMAKE_INSTALL_BINDIR}
    CONFIGURATIONS Release)

install(
    FILES ${CMAKE_SOURCE_DIR}/runtime/stdlib.gaya
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/gaya/runtime
    CONFIGURATIONS Release)
