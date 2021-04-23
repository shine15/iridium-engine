aux_source_directory(. EXECUTABLE_SRCS)
add_executable(iridium ${EXECUTABLE_SRCS})

target_link_libraries(
        iridium PUBLIC strategy_lib
)

target_link_libraries(
        iridium PUBLIC iridium_lib
)


