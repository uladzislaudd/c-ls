cmake_minimum_required(VERSION 3.0.0)
project(c-ls VERSION 0.1.0 LANGUAGES C)

set(default_build_type "Debug")

include(CTest)
enable_testing()

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

add_compile_options(
    $<$<CONFIG:>:-Wall>
    $<$<CONFIG:>:-Wextra>
    $<$<CONFIG:DEBUG>:-g>
    $<$<CONFIG:RELEASE>:-O3>
    $<$<CONFIG:RELEASE>:-Werror>
)

add_link_options(
    $<$<CONFIG:RELEASE>:-Wl,--strip-all>
)

include_directories(include)

add_library(ls STATIC src/ls/ls.c src/ls/ls_stat.c src/ls/ls_status.c)
add_library(lsos STATIC src/os/ls_os.c src/os/ls_posix.c)

add_executable(ls.bin src/core/ls.c)
target_link_libraries(ls.bin ls lsos)
