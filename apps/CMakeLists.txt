aux_source_directory(. EXECUTABLE_SRCS)
add_executable(iridium ${EXECUTABLE_SRCS})
target_link_libraries(
        iridium PUBLIC iridium_lib
)

include_directories(${TA_LIB_INCLUDE_DIR})
target_link_libraries(
        iridium PUBLIC ${TA_LIB_STATIC_LIB_PATH}
)