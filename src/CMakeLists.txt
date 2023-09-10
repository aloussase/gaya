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
    builtins/io.cpp
    builtins/math.cpp)

target_compile_options(gaya PRIVATE -Wall -Wextra
    $<$<CONFIG:Release>:-Os>
    $<$<CONFIG:Debug>:-g>)

target_link_libraries(gaya PRIVATE
    fmt::fmt
    readline)
