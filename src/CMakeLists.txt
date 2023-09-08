add_executable(gaya main.cpp
    lexer.cpp
    span.cpp
    parser.cpp
    ast.cpp
    diagnostic.cpp)

target_compile_options(gaya PRIVATE -Wall -Wextra
    $<$<CONFIG:Release>:-Os>
    $<$<CONFIG:Debug>:-g>)
