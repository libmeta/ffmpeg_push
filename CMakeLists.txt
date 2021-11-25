cmake_minimum_required(VERSION 3.14)
project(push)
ENABLE_LANGUAGE(ASM)

set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} ${CMAKE_CXX_FLAGS}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++${CMAKE_CXX_STANDARD}  -fPIC")


set(DYNAMIC_LIB_SUFFIX .so)
set(STATIC_LIB_SUFFIX .a)
set(LIBS_INCLUDE_DIR /usr/include) 
set(LIBS_DIR /usr/lib) 

macro(add_dynamic_lib LIB_NAME)
    add_library(${LIB_NAME} SHARED IMPORTED)
    set_target_properties(${LIB_NAME} PROPERTIES IMPORTED_LOCATION ${LIBS_DIR}/${LIB_NAME}${DYNAMIC_LIB_SUFFIX})
    message("Add dynamic library: ${LIB_NAME}")
endmacro()

macro(add_static_lib LIB_NAME)
    add_library(${LIB_NAME} STATIC IMPORTED)
    set_target_properties(${LIB_NAME} PROPERTIES IMPORTED_LOCATION ${LIBS_DIR}/${LIB_NAME}${STATIC_LIB_SUFFIX})
    message("Add static library: ${LIB_NAME}")
endmacro()

macro(include_directory DIR_PATH)
    include_directories(${DIR_PATH})
    message("Include Directory: ${DIR_PATH}")
endmacro()

macro(target_link_add_executable EXE_NAME SOURCE_PATH)
add_executable(${EXE_NAME}  ${SOURCE_PATH})
target_link_libraries(${EXE_NAME} PRIVATE ${PROJECT_NAME})
endmacro()

include_directory(${LIBS_INCLUDE_DIR})
include_directory(${CMAKE_SOURCE_DIR})
set(NATIVE_DIR ${CMAKE_SOURCE_DIR}/native/) 
include_directory(${NATIVE_DIR})

file(GLOB_RECURSE NATIVE_SOURCES ${NATIVE_DIR}/*.cpp)

add_library(
    ${PROJECT_NAME} SHARED
    ${NATIVE_SOURCES}
    )

add_dynamic_lib(libavcodec)
add_dynamic_lib(libavfilter)
add_dynamic_lib(libavformat)
add_dynamic_lib(libavutil)
add_dynamic_lib(libswresample)
add_dynamic_lib(libswscale)
add_dynamic_lib(libpostproc)

target_link_libraries(
        ${PROJECT_NAME}
        libavcodec
        libavfilter
        libavformat
        libavutil
        libswresample
        libswscale
        libpostproc
        )

set(TEST_DIR ${CMAKE_SOURCE_DIR}/test/) 
target_link_add_executable(test_push ${TEST_DIR}/test_push.cpp)