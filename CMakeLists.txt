cmake_minimum_required(VERSION 3.0.0)
project(c-ls VERSION 0.1.0)

include(CTest)
enable_testing()

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

include_directories(include)

add_library(ls STATIC src/ls/ls.c src/ls/ls_stat.c src/ls/ls_status.c)
add_library(lsos STATIC src/os/posix.c)

add_executable(ls.bin src/core/ls.c)
target_link_libraries(ls.bin ls lsos)
