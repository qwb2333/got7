cmake_minimum_required(VERSION 3.7)
project(got7)

set(CMAKE_CXX_STANDARD 11)
include_directories("src/")

set(SOURCE_FILES_INNER src/lib/common/logging.cpp src/lib/common/logging.h src/lib/common/base.h src/service/inner_service.cpp src/lib/common/utils.cpp src/lib/common/utils.h src/lib/common/config.cpp src/lib/common/config.h src/lib/connect/connect_pool.cpp src/lib/connect/connect_pool.h src/lib/connect/epoll.cpp src/lib/connect/epoll.h)
add_executable(inner_service ${SOURCE_FILES_INNER})
target_link_libraries(inner_service -lpthread)


set(SOURCE_FILES_OUTER src/lib/common/logging.cpp src/lib/common/logging.h src/lib/common/base.h src/service/outer_service.cpp src/lib/common/utils.cpp src/lib/common/utils.h src/lib/common/config.cpp src/lib/common/config.h src/lib/connect/connect_pool.cpp src/lib/connect/connect_pool.h src/lib/connect/epoll.cpp src/lib/connect/epoll.h src/lib/connect/task.h)
add_executable(outer_service ${SOURCE_FILES_OUTER})
target_link_libraries(outer_service -lpthread)
