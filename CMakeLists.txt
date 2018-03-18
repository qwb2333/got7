cmake_minimum_required(VERSION 3.7)
project(got7)

set(CMAKE_CXX_STANDARD 11)
include_directories(src/)

aux_source_directory(src/lib SOURCE_LIB)
aux_source_directory(src/model SOURCE_MODEL)
set(SOURCE_FILES ${SOURCE_LIB} ${SOURCE_MODEL})

set(LINK_FILES -pthread -lprotobuf)

add_executable(inner_service ${SOURCE_FILES} src/service/inner_service.cpp)
target_link_libraries(inner_service ${LINK_FILES})

add_executable(outer_service ${SOURCE_FILES} src/service/outer_service.cpp)
target_link_libraries(outer_service ${LINK_FILES})

add_executable(tests-connect_pool ${SOURCE_FILES} src/tests/connect_pool.cpp)
target_link_libraries(tests-connect_pool ${LINK_FILES})

add_executable(tests-tcp ${SOURCE_FILES} src/tests/tcp.cpp)
target_link_libraries(tests-tcp ${LINK_FILES})

add_executable(tests-protobuf ${SOURCE_FILES} src/tests/protobuf.cpp)
target_link_libraries(tests-protobuf ${LINK_FILES})