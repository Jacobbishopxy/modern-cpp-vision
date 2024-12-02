cmake_minimum_required(VERSION 3.7)
project(cpp_todo_server)


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# set(CMAKE_C_COMPILER "/usr/bin/gcc-12")
# set(CMAKE_CXX_COMPILER "/usr/bin/g++-12")


# ================================================================================================
# deps
# ================================================================================================

# fmt
find_package(fmt REQUIRED)

# ================================================================================================
# exec
# ================================================================================================

add_executable(simple_generator dev/SimpleGenerator.cpp)

add_executable(async_io dev/AsyncIO.cpp)

add_executable(co_vec_fut dev/CoVecFut.cpp)

add_executable(co_generator dev/CoGenerator.cpp)

add_executable(builder_tmp dev/BuilderTemplate.cpp)
target_link_libraries(builder_tmp fmt::fmt)
