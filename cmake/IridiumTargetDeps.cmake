# Check dependencies
FILE(COPY resources DESTINATION "${CMAKE_BINARY_DIR}")

set(DEPS_DIR ${PROJECT_SOURCE_DIR}/deps)

# Use linux docker image for live trading deployment
if (UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif ()

# HDF5
if (LINUX)
    set(HDF5_DIR /usr/local/HDF_Group/HDF5/1.12.0)
    set(HDF5_INCLUDE_DIR ${HDF5_DIR}/include)
else ()
    set(HDF5_DIR ${DEPS_DIR}/hdf5)
    set(HDF5_INCLUDE_DIR
            ${HDF5_DIR}/include
            ${HDF5_DIR}/include/c
            ${HDF5_DIR}/include/c++)
endif ()

if (WIN32)
    if (${CMAKE_BUILD_TYPE} MATCHES "Debug")
        set(HDF5_LIB_DIR "${HDF5_DIR}/lib/windows/Debug")
        set(HDF5_LIBS
                ${HDF5_LIB_DIR}/libhdf5_D.lib
                ${HDF5_LIB_DIR}/libhdf5_cpp_D.lib
                ${HDF5_LIB_DIR}/libzlib_D.lib
                ${HDF5_LIB_DIR}/libszip_D.lib)
    endif ()
    if (${CMAKE_BUILD_TYPE} MATCHES "Release")
        set(HDF5_LIB_DIR "${HDF5_DIR}/lib/windows/Release")
        set(HDF5_LIBS
                ${HDF5_LIB_DIR}/libhdf5.lib
                ${HDF5_LIB_DIR}/libhdf5_cpp.lib
                ${HDF5_LIB_DIR}/libzlib.lib
                ${HDF5_LIB_DIR}/libszip.lib)
    endif ()
endif ()

if (UNIX)
    if (LINUX)
        set(HDF5_LIB_DIR "${HDF5_DIR}/lib")
        set(HDF5_LIBS
                ${HDF5_LIB_DIR}/libhdf5.so
                ${HDF5_LIB_DIR}/libhdf5_cpp.so
                ${HDF5_LIB_DIR}/libz.so
                ${HDF5_LIB_DIR}/libszip.so)
    else ()
        set(HDF5_LIB_DIR "${HDF5_DIR}/lib/unix")
        set(HDF5_LIBS
                ${HDF5_LIB_DIR}/libhdf5.a
                ${HDF5_LIB_DIR}/libhdf5_cpp.a
                ${HDF5_LIB_DIR}/libz.a
                ${HDF5_LIB_DIR}/libszip.a)
    endif ()
endif ()

# TA Lib
if (LINUX)
    set(TA_LIB_INCLUDE_DIR /usr/local/include/ta-lib)
    set(TA_LIB_STATIC_LIB_PATH /usr/local/lib/libta_lib.a)
else ()
    set(TA_LIB_DIR ${DEPS_DIR}/ta-lib/lib)
    set(TA_LIB_INCLUDE_DIR ${DEPS_DIR}/ta-lib/include)
    if (UNIX)
        set(TA_LIB_STATIC_LIB_PATH ${TA_LIB_DIR}/libta_lib.a)
    endif ()
    if (WIN32)
        set(TA_LIB_STATIC_LIB_PATH
                ${TA_LIB_DIR}/ta_abstract_cdr.lib
                ${TA_LIB_DIR}/ta_common_cdr.lib
                ${TA_LIB_DIR}/ta_func_cdr.lib
                ${TA_LIB_DIR}/ta_libc_cdr.lib)
    endif ()
endif ()