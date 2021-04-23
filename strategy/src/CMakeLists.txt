file(GLOB STRATEGY_LIB_HEADER_LIST CONFIGURE_DEPENDS "include/*.hpp")
aux_source_directory(. STRATEGY_LIB_SRCS_DIR)
add_library(strategy_lib STATIC ${STRATEGY_LIB_SRCS_DIR} ${STRATEGY_LIB_HEADER_LIST})
target_include_directories(strategy_lib PUBLIC ${STRATEGY_LIB_HEADER_LIST})

# TA Lib
cmake_print_variables(TA_LIB_INCLUDE_DIR)
cmake_print_variables(TA_LIB_STATIC_LIB_PATH)
target_include_directories(strategy_lib PUBLIC ${TA_LIB_INCLUDE_DIR})
target_link_libraries(strategy_lib PRIVATE ${TA_LIB_STATIC_LIB_PATH})

# Iridium
target_link_libraries(strategy_lib PUBLIC iridium_lib)
