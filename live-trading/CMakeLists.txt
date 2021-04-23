aux_source_directory(. EXECUTABLE_SRCS)
add_executable(live-trading ${EXECUTABLE_SRCS})

if (APPLE)
    SET(OPENSSL_ROOT_DIR /usr/local/opt/openssl)
endif ()

find_package(OpenSSL REQUIRED COMPONENTS SSL Crypto)
find_package(Poco REQUIRED COMPONENTS NetSSL Crypto)
target_link_libraries(
        live-trading PRIVATE Poco::NetSSL Poco::Crypto Poco::Util)

target_link_libraries(
        live-trading PRIVATE strategy_lib)

target_include_directories(live-trading PUBLIC ${TA_LIB_INCLUDE_DIR})
target_link_libraries(
        live-trading PRIVATE ${TA_LIB_STATIC_LIB_PATH}
)