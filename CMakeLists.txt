PROJECT(example)
cmake_minimum_required(VERSION 3.5)

SET(CMAKE_BUILD_TYPE "Debug")

SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb")

# directory of opencv library
#link_directories(${OpenCV_LIBRARY_DIRS})

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
set(lib ${OpenCV_LIBS} ${lib})

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
set(lib Threads::Threads ${lib})

find_package(CURL REQUIRED)
set(lib ${CURL_LIBRARIES} ${lib})

add_library(tuner SHARED "tuner.cpp")
set(lib tuner ${lib})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# name of executable file and path of source file
add_executable(${PROJECT_NAME} example.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE ${lib})