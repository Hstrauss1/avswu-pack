cmake_minimum_required(VERSION 3.18.4)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(sig)


#
# create libraries
#
include_directories(
    /usr/local/include/pbc
    /usr/local/lib
)

# make musig.a lib
add_library(musig STATIC musig.c)

# make libpbc-wrapper.a lib
add_library(pbc-wrapper STATIC pbc-wrapper.c)

#
# compile executables
#

# sig
set(_target sig)
message("_target=${_target}")
add_executable(${_target} ${_target}.c musig.c)
target_include_directories(${_target} PUBLIC /usr/local/include/pbc)
target_link_directories(${_target} PUBLIC /usr/local/lib)
target_link_libraries(${_target} PUBLIC gmp pbc)

# sig-orig
set(_target sig-orig)
message("_target=${_target}")
add_executable(${_target} ${_target}.c)
target_include_directories(${_target} PUBLIC /usr/local/include/pbc)
target_link_directories(${_target} PUBLIC /usr/local/lib)
target_link_libraries(${_target} PUBLIC gmp pbc)
