cmake_minimum_required(VERSION 3.7)
project(got7)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES_INNER src/lib/common/logging.cpp src/lib/common/logging.h src/lib/common/base.h src/service/inner_service.cpp src/lib/common/utils.cpp src/lib/common/utils.h)
set(SOURCE_FILES_OUTER src/lib/common/logging.cpp src/lib/common/logging.h src/lib/common/base.h src/service/outer_service.cpp src/lib/common/utils.cpp src/lib/common/utils.h)
include_directories("src/")

add_executable(inner_service ${SOURCE_FILES_INNER})
add_executable(outer_service ${SOURCE_FILES_OUTER})