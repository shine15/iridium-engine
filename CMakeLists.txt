cmake_minimum_required(VERSION 3.15)
project(iridium VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_CXX_FLAGS "--coverage")

# INCLUDES
list(APPEND CMAKE_MODULE_PATH
        ${PROJECT_SOURCE_DIR}/cmake
        ${PROJECT_SOURCE_DIR}/cmake/portability
        ${PROJECT_SOURCE_DIR}/cmake/config)

include(${PROJECT_SOURCE_DIR}/cmake/IridiumTargetDeps.cmake)

add_subdirectory(src)
add_subdirectory(test)
add_subdirectory(apps)




