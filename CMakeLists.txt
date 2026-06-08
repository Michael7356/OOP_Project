cmake_minimum_required(VERSION 4.1)
project(OOP_Project)

set(CMAKE_CXX_STANDARD 20)

add_executable(OOP_Project src/Transaction.h
        src/Transaction.cpp
        src/main.cpp
        src/DataManager.cpp
        src/DataManager.h
        src/PdfParser.cpp
        src/PdfParser.h
        src/DisplayUtil.cpp
        src/DisplayUtil.h
        src/Deposit.cpp
        src/Deposit.h
)

set(MSYS_PATH "C:/msys64/ucrt64")

find_package(PkgConfig REQUIRED)
pkg_check_modules(POPPLER_CPP REQUIRED poppler-cpp)

include_directories("${MSYS_PATH}/include")
include_directories("${MSYS_PATH}/include/poppler/cpp")
link_directories("${MSYS_PATH}/lib")

target_link_libraries(OOP_Project poppler-cpp)

if(WIN32)
    add_definitions(-D_WIN32_WINNT=0x0A00)
endif()

target_link_libraries(OOP_Project ws2_32)