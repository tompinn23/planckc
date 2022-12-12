cmake_minimum_required(VERSION 3.20)
project(planck)

add_library(planck src/pk_vm.c)
target_include_directories(planck PUBLIC src/)

add_executable(planckc src/planckc.c)
target_link_libraries(planckc planck)


