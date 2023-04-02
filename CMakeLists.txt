cmake_minimum_required(VERSION 3.0)

project(DataItem)

set(CMAKE_CXX_STANDARD 11)

set(SOURCES
  src/cbor.cpp
)
include_directories(src)

add_library (cbor STATIC ${SOURCES})

set_property(TARGET cbor PROPERTY POSITION_INDEPENDENT_CODE 1)

add_executable(test tests/test.cpp)
# link_libraries(test PRIVATE DataItem)
target_link_libraries(test PRIVATE cbor)