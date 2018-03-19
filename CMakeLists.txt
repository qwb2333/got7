cmake_minimum_required(VERSION 3.7)
project(got7)

set(CMAKE_CXX_STANDARD 11)
include_directories(src/)

set(SOURCE_FILES src/lib/common/base.h src/lib/common/config.cpp src/lib/common/config.h src/lib/common/exit.h
        src/lib/common/logging.cpp src/lib/common/logging.h src/lib/common/utils.cpp src/lib/common/utils.h
        src/lib/connect/connect_pool.cpp src/lib/connect/connect_pool.h src/lib/connect/epoll.cpp src/lib/connect/epoll.h
        src/lib/connect/task.h src/lib/connect/tcp.cpp src/lib/connect/tcp.h src/model/feed.pb.cc src/model/feed.pb.h)

set(LINK_FILES -lpthread -pthread -lprotobuf)

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

add_executable(tests-main_flow ${SOURCE_FILES} src/tests/main_flow.cpp)
target_link_libraries(tests-main_flow ${LINK_FILES})