cmake_minimum_required(VERSION 4.1)
project(OOP_Project)

set(CMAKE_CXX_STANDARD 20)

add_executable(OOP_Project Transaction.h
        Transaction.cpp
        main.cpp
        Store.cpp
        Store.h
        PdfParser.cpp
        PdfParser.h)

set(MSYS_PATH "C:/msys64/ucrt64")

find_package(PkgConfig REQUIRED)
pkg_check_modules(POPPLER_CPP REQUIRED poppler-cpp)

include_directories("${MSYS_PATH}/include")
include_directories("${MSYS_PATH}/include/poppler/cpp")
link_directories("${MSYS_PATH}/lib")

target_link_libraries(OOP_Project poppler-cpp)