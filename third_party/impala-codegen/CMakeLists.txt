cmake_minimum_required (VERSION 3.10)

# projectname is the same as the main-executable
project(impala-codegen)
set(CMAKE_CXX_STANDARD 14)

set(CODEGEN_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/ErrorCodes_constants.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ErrorCodes_types.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ExecStats_types.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Metrics_types.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/RuntimeProfile_types.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Status_types.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Types_types.cpp
)

add_library(impala_thrift STATIC ${CODEGEN_SRCS})
target_link_libraries(impala_thrift thriftz)


