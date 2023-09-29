add_subdirectory(repl)

add_executable(gaya main.cpp)
target_link_libraries(gaya fmt::fmt gaya_lib gaya_repl)

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
