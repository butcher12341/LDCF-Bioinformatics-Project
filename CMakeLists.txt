cmake_minimum_required(VERSION 3.16)
project(ldcf)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(OpenSSL REQUIRED)

add_executable(ldcf
    src/main.cpp
    src/cuckoo_filter.cpp
    src/genome_loader.cpp
    src/ldcf.cpp
    src/benchmark.cpp
)

target_compile_options(ldcf PRIVATE
    -Wno-deprecated-declarations
)

target_include_directories(ldcf PRIVATE
    ${PROJECT_SOURCE_DIR}/src
)

target_link_libraries(ldcf PRIVATE
    OpenSSL::Crypto
)