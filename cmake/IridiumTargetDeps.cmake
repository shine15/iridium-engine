# Check dependencies
FILE(COPY resources/date_time_zonespec.csv DESTINATION "${CMAKE_BINARY_DIR}/resources")

# HDF5
set(HDF5_VERSION "1.12.0")
set(_hdf5_version_folder_name "1_12_0")
set(DEPS_DIR ${PROJECT_SOURCE_DIR}/deps)
set(HDF5_DIR ${DEPS_DIR}/CMake-hdf5-${HDF5_VERSION})
set(HDF5_LIB_DIR "${HDF5_DIR}/build/bin")
set(HDF5_SOURCE_CODE_DIR "${HDF5_DIR}/hdf5-${HDF5_VERSION}")

if(EXISTS ${HDF5_DIR})
    message(STATUS "HDF5 Found")
else()
    set(_zip_file "CMake-hdf5-${HDF5_VERSION}.tar.gz")
    set(filename "https://hdf-wordpress-1.s3.amazonaws.com/wp-content/uploads/manual/HDF5/HDF5_${_hdf5_version_folder_name}/source/${_zip_file}")
    file(DOWNLOAD ${filename} "${DEPS_DIR}/${_zip_file}" SHOW_PROGRESS)
    execute_process(COMMAND ${CMAKE_COMMAND} -E tar -xf ${_zip_file}
            WORKING_DIRECTORY ${DEPS_DIR})
    execute_process(COMMAND rm ${_zip_file}
            WORKING_DIRECTORY ${DEPS_DIR})
    execute_process(COMMAND ./build-unix.sh
            WORKING_DIRECTORY ${HDF5_DIR})
endif()

set(HDF5_INCLUDE_DIR
        ${HDF5_SOURCE_CODE_DIR}/src
        ${HDF5_SOURCE_CODE_DIR}/c++/src
        ${HDF5_DIR}/build
        )
set(HDF5_LIBS
        ${HDF5_LIB_DIR}/libhdf5.a
        ${HDF5_LIB_DIR}/libhdf5_cpp.a
        ${HDF5_LIB_DIR}/libz.a
        ${HDF5_LIB_DIR}/libszip.a
        )

set(TA_LIB_DIR ${DEPS_DIR}/ta-lib)
set(TA_LIB_INCLUDE_DIR ${TA_LIB_DIR}/include)
set(TA_LIB_STATIC_LIB_PATH ${TA_LIB_DIR}/libta_lib.a)