cmake_minimum_required(VERSION 3.10)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)

macro(ADD_OSX_FRAMEWORK target fwname )
    find_library(FRAMEWORK_${fwname}
    NAMES ${fwname}
    PATHS ${CMAKE_OSX_SYSROOT}/System/Library
    PATH_SUFFIXES Frameworks
    NO_DEFAULT_PATH)
    if( ${FRAMEWORK_${fwname}} STREQUAL FRAMEWORK_${fwname}-NOTFOUND)
        MESSAGE(ERROR ": Framework ${fwname} not found")
    else()
        TARGET_LINK_LIBRARIES(${target} "${FRAMEWORK_${fwname}}")
        MESSAGE(STATUS "Framework ${fwname} found at ${FRAMEWORK_${fwname}}")
    endif()
endmacro(ADD_OSX_FRAMEWORK)

list(PREPEND CMAKE_PREFIX_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake-modules)

find_package(Certify REQUIRED)
add_dependencies(Certify OpenSSL)

project(composed_1)

add_executable(${PROJECT_NAME} composed_1.cpp)
find_package(Boost REQUIRED COMPONENTS system)
target_link_libraries(${PROJECT_NAME} Boost::system)

project(composed_2)

add_executable(${PROJECT_NAME} composed_2.cpp)
find_package(Boost REQUIRED COMPONENTS system)
target_link_libraries(${PROJECT_NAME} Boost::system)

project(composed_3)

add_executable(${PROJECT_NAME} composed_3.cpp)
find_package(Boost REQUIRED COMPONENTS system)
target_link_libraries(${PROJECT_NAME} Boost::system)

project(composed_4)

add_executable(${PROJECT_NAME} composed_4.cpp)
find_package(Boost REQUIRED COMPONENTS system)
target_link_libraries(${PROJECT_NAME} Boost::system)

project(composed_5)

add_executable(${PROJECT_NAME} composed_5.cpp)
find_package(Boost REQUIRED COMPONENTS system)
target_link_libraries(${PROJECT_NAME} Boost::system)

project(composed_6)

add_executable(${PROJECT_NAME} composed_6.cpp)
find_package(Boost REQUIRED COMPONENTS system)
target_link_libraries(${PROJECT_NAME} Boost::system)

project(composed_7)

add_executable(${PROJECT_NAME} composed_7.cpp)
find_package(Boost REQUIRED COMPONENTS system)
target_link_libraries(${PROJECT_NAME} Boost::system)

project(composed_8)

add_executable(${PROJECT_NAME} composed_8.cpp)
find_package(Boost REQUIRED COMPONENTS system)
target_link_libraries(${PROJECT_NAME} Boost::system)


project(async_client)

add_executable(${PROJECT_NAME} async_client.cpp)
find_package(Boost REQUIRED COMPONENTS system)
target_link_libraries(${PROJECT_NAME} Boost::system)

project(api_client)

add_executable(${PROJECT_NAME} api_client.cpp)
find_package(Boost REQUIRED COMPONENTS system)
target_link_libraries(${PROJECT_NAME} Boost::system)


project(api_client_example)

add_executable(${PROJECT_NAME} api_client_example.cpp)
find_package(Boost REQUIRED COMPONENTS system)
target_link_libraries(${PROJECT_NAME} Boost::system)

########################################################

project(async_client_ssl)

add_executable(${PROJECT_NAME} async_client_ssl.cpp)
find_package(Boost REQUIRED COMPONENTS system)
target_link_libraries(${PROJECT_NAME} Boost::system)
target_link_libraries(${PROJECT_NAME} OpenSSL::SSL OpenSSL::Crypto)

########################################################

project(http_client_async_ssl)

add_executable(${PROJECT_NAME} http_client_async_ssl.cpp)
find_package(Boost REQUIRED COMPONENTS system)
find_package(OpenSSL REQUIRED)
find_package(Certify REQUIRED)
add_dependencies(Certify OpenSSL)

target_link_libraries(${PROJECT_NAME} Boost::system)
target_link_libraries(${PROJECT_NAME} OpenSSL::SSL OpenSSL::Crypto)
target_include_directories(${PROJECT_NAME} PUBLIC "${Certify_INCLUDE_DIR}")

if(APPLE)
    ADD_OSX_FRAMEWORK(${PROJECT_NAME} CoreFoundation )
    ADD_OSX_FRAMEWORK(${PROJECT_NAME} Security )
endif()

########################################################
project(http_client_async_ssl_system_executor)

add_executable(${PROJECT_NAME} http_client_async_ssl_system_executor.cpp)
find_package(Boost REQUIRED COMPONENTS system)
find_package(OpenSSL REQUIRED)
find_package(Certify REQUIRED)
add_dependencies(Certify OpenSSL)

target_link_libraries(${PROJECT_NAME} Boost::system)
target_link_libraries(${PROJECT_NAME} OpenSSL::SSL OpenSSL::Crypto)
target_include_directories(${PROJECT_NAME} PUBLIC "${Certify_INCLUDE_DIR}")

if(APPLE)
    ADD_OSX_FRAMEWORK(${PROJECT_NAME} CoreFoundation )
endif()

########################################################