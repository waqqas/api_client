cmake_minimum_required(VERSION 3.10)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)

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


project(async_client_ssl)

add_executable(${PROJECT_NAME} async_client_ssl.cpp)
find_package(Boost REQUIRED COMPONENTS system)
target_link_libraries(${PROJECT_NAME} Boost::system)

find_package(OpenSSL REQUIRED)
target_link_libraries(${PROJECT_NAME} OpenSSL::SSL OpenSSL::Crypto)

project(http_client_async_ssl)

add_executable(${PROJECT_NAME} http_client_async_ssl.cpp)
find_package(Boost REQUIRED COMPONENTS system)
target_link_libraries(${PROJECT_NAME} Boost::system)

find_package(OpenSSL REQUIRED)
target_link_libraries(${PROJECT_NAME} OpenSSL::SSL OpenSSL::Crypto)

project(http_client_async_ssl_system_executor)

add_executable(${PROJECT_NAME} http_client_async_ssl_system_executor.cpp)
find_package(Boost REQUIRED COMPONENTS system)
target_link_libraries(${PROJECT_NAME} Boost::system)

find_package(OpenSSL REQUIRED)
target_link_libraries(${PROJECT_NAME} OpenSSL::SSL OpenSSL::Crypto)
