cmake_minimum_required(VERSION 3.7)
project(got7)

set(CMAKE_CXX_STANDARD 11)
include_directories("src/")

set(SOURCE_FILES src/lib/common/logging.cpp src/lib/common/logging.h src/lib/common/base.h src/lib/common/utils.cpp src/lib/common/utils.h src/lib/common/config.cpp src/lib/common/config.h src/lib/connect/connect_pool.cpp src/lib/connect/connect_pool.h src/lib/connect/epoll.cpp src/lib/connect/epoll.h src/lib/connect/task.h src/lib/common/exit.h src/lib/connect/tcp.cpp src/lib/connect/tcp.cpp src/lib/connect/tcp.h)

add_executable(inner_service ${SOURCE_FILES} src/service/inner_service.cpp)
target_link_libraries(inner_service -lpthread)

add_executable(outer_service ${SOURCE_FILES} src/service/outer_service.cpp)
target_link_libraries(outer_service -lpthread)

add_executable(tests-connect_pool_tests ${SOURCE_FILES} src/tests/connect_pool_tests.cpp)
target_link_libraries(tests-connect_pool_tests -lpthread)

add_executable(tests-tcp_tests ${SOURCE_FILES} src/tests/tcp_tests.cpp)
target_link_libraries(tests-tcp_tests -lpthread)