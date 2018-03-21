cmake_minimum_required(VERSION 2.8)
project(got7)

add_compile_options(-std=c++11 -O3)
include_directories(src/)

set(
        SOURCE_FILES src/lib/common/base.h src/lib/common/config.cpp src/lib/common/config.h src/lib/common/exit.h
        src/lib/common/logging.cpp src/lib/common/logging.h src/lib/common/utils.cpp src/lib/common/utils.h
        src/lib/connect/connect_pool.cpp src/lib/connect/connect_pool.h src/lib/connect/epoll.cpp src/lib/connect/epoll.h
        src/lib/connect/task.h src/lib/connect/tcp.cpp src/lib/connect/tcp.h src/got7/protobuf/feed.pb.cc src/got7/protobuf/feed.pb.h
        src/got7/common/const.h
        src/got7/inner/service.cpp src/got7/inner/service.h src/got7/common/context.h src/got7/common/feed_utils.cpp src/got7/common/feed_utils.h src/got7/inner/pipe_task.cpp src/got7/inner/pipe_task.h src/got7/inner/context.h src/got7/inner/request_task.cpp src/got7/inner/request_task.h "src/got7/outer/context.h" src/got7/outer/service.cpp src/got7/outer/service.h src/got7/outer/request_task.cpp src/got7/outer/request_task.h src/got7/outer/pipe_task.cpp src/got7/outer/pipe_task.h)

set(LINK_FILES -lpthread -pthread -lprotobuf)

add_executable(inner_service ${SOURCE_FILES} src/main/inner.cpp)
target_link_libraries(inner_service ${LINK_FILES})

add_executable(outer_service ${SOURCE_FILES} src/main/outer.cpp)
target_link_libraries(outer_service ${LINK_FILES})

add_executable(tests-connect_pool ${SOURCE_FILES} src/tests/connect_pool.cpp)
target_link_libraries(tests-connect_pool ${LINK_FILES})

add_executable(tests-tcp ${SOURCE_FILES} src/tests/tcp.cpp)
target_link_libraries(tests-tcp ${LINK_FILES})

add_executable(tests-protobuf ${SOURCE_FILES} src/tests/protobuf.cpp)
target_link_libraries(tests-protobuf ${LINK_FILES})

add_executable(tests-main_flow ${SOURCE_FILES} src/tests/main_flow.cpp)
target_link_libraries(tests-main_flow ${LINK_FILES})