cmake_minimum_required(VERSION 3.0.0)
project(c-ls VERSION 0.1.0)

include(CTest)
enable_testing()

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

add_executable(ls src/core/ls.c)
