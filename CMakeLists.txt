cmake_minimum_required(VERSION 3.12)

set(PROGRAM_NAME "pong")

project(${PROGRAM_NAME}
	VERSION 1.0
	LANGUAGES CXX)

# Headers
include_directories(birb2d/include/)

add_subdirectory(birb2d)

add_executable(${PROGRAM_NAME} src/pong.cpp)
target_link_libraries(${PROGRAM_NAME} birb2d)

# Copy the resources into the build directory
file(COPY ./res DESTINATION ./)
