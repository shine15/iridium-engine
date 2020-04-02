# Check dependencies & link to iridium lib
file(GLOB HEADER_LIST CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/include/iridium/*.hpp")
set(LIBRARY_OUTPUT_PATH  ${CMAKE_BINARY_DIR}/lib)
aux_source_directory(. DIR_LIB_SRCS)
add_library(iridium_lib STATIC ${DIR_LIB_SRCS} ${HEADER_LIST})
target_include_directories(iridium_lib PUBLIC ../include)

# HDF5
target_include_directories(iridium_lib PUBLIC ${HDF5_INCLUDE_DIR})
target_link_libraries(iridium_lib PUBLIC ${HDF5_LIBS})

# Boost
find_package(Boost REQUIRED COMPONENTS system)
if(Boost_FOUND)
    message("Boost Found")
    target_include_directories(iridium_lib PUBLIC ${Boost_INCLUDE_DIRS})
    target_link_libraries(iridium_lib PUBLIC ${Boost_LIBRARIES})
elseif(NOT Boost_FOUND)
    error("Boost Not Found")
endif()