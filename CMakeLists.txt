cmake_minimum_required(VERSION 3.15)
project(VacSim C)

set(CMAKE_C_STANDARD 99)
set(SOURCE_FILES ${CMAKE_SOURCE_DIR}/vacc_sim.c)
add_executable(${PROJECT_NAME} ${SOURCE_FILES})