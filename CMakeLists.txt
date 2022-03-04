set(SOURCE main.cpp cmvec.h main.cpp cmvec.h)
set(PROJECT_NAME cmcheckers)

cmake_minimum_required(VERSION 3.19)
project(${PROJECT_NAME})
set(CMAKE_CXX_STANDARD 14)

SET(GCC_COVERAGE_COMPILE_FLAGS "-no-pie")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0")


#set(SFML_DIR "/home/cyanmargh/SFML-compiled")
#set(CMAKE_MODULE_PATH "/home/cyanmargh/SFML-compiled" ${CMAKE_MODULE_PATH})
#set(SFML_STATIC_LIBRARIES TRUE)

find_package(SFML 2.5 COMPONENTS graphics audio REQUIRED)
add_executable(${PROJECT_NAME} ${SOURCE})
target_link_libraries(${PROJECT_NAME} sfml-graphics)