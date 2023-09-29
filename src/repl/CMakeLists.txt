add_library(repl_deps
    raw.cpp
    run_line.cpp)
target_link_libraries(repl_deps gaya_lib)

add_custom_target(build_repl
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/repl.o")

add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/repl.o"
    COMMAND gdc -c "${CMAKE_CURRENT_SOURCE_DIR}/repl.d"
    COMMENT "Compiles the REPL"
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/repl.d")

add_library(gaya_repl OBJECT IMPORTED GLOBAL)
target_link_libraries(gaya_repl INTERFACE repl_deps gphobos)

set_property(
    TARGET gaya_repl
    PROPERTY IMPORTED_OBJECTS
    "${CMAKE_CURRENT_BINARY_DIR}/repl.o")

add_dependencies(gaya_repl build_repl)
