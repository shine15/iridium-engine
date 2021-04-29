# Check dependencies & link to iridium lib
file(GLOB HEADER_LIST CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/include/iridium/*.hpp")
set(LIBRARY_OUTPUT_PATH  ${CMAKE_BINARY_DIR}/lib)
aux_source_directory(. DIR_LIB_SRCS)
add_library(iridium_lib STATIC ${DIR_LIB_SRCS} ${HEADER_LIST})
target_include_directories(iridium_lib PUBLIC ../include)

# HDF5
cmake_print_variables(HDF5_INCLUDE_DIR)
cmake_print_variables(HDF5_LIBS)
target_include_directories(iridium_lib PUBLIC ${HDF5_INCLUDE_DIR})
target_link_libraries(iridium_lib PRIVATE ${HDF5_LIBS})

# Boost
find_package(Boost 1.75.0 REQUIRED COMPONENTS date_time filesystem)
if(Boost_FOUND)
    target_include_directories(iridium_lib PUBLIC ${Boost_INCLUDE_DIRS})
    target_link_libraries(iridium_lib PUBLIC Boost::date_time Boost::filesystem)
endif()
