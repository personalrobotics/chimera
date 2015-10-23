cmake_minimum_required(VERSION 2.8.8)
project(dartpy)

find_package(Boost REQUIRED COMPONENTS python)
find_package(DART REQUIRED)
find_package(PythonLibs REQUIRED)

set(CMAKE_CXX_FLAGS "-std=c++11")

include_directories(SYSTEM
  ${Boost_INCLUDE_DIRS}
  ${DART_INCLUDE_DIRS}
  ${PYTHON_INCLUDE_DIRS}
  /opt/ros/indigo/include
)

file(GLOB dartpy_SOURCES *.cpp)

add_library(${PROJECT_NAME} SHARED ${dartpy_SOURCES})
target_link_libraries(${PROJECT_NAME}
  ${Boost_LIBRARIES}
  ${DART_LIBRARIES}
  ${PYTHON_LIBRARIES}
)
