# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

#set(INC_DIR ${PROJECT_SOURCE_DIR}/libs/include)
set(LINK_DIR ${CMAKE_SOURCE_DIR}/libs/${ANDROID_ABI})
#引入ffmpeg头文件
include_directories(${CMAKE_SOURCE_DIR}/src/main/cpp/include)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -L${CMAKE_SOURCE_DIR}/libs/${CMAKE_ANDROID_ARCH_ABI}")


# TODO 添加so库对应的头文件目录
#include_directories(${INC_DIR})

# TODO 引入so库,IMPORT代表从第三方引入的意思
add_library( avcodec-58 SHARED IMPORTED)
# TODO 设置编译的库文件存放的目录
set_target_properties( avcodec-58 PROPERTIES IMPORTED_LOCATION ${LINK_DIR}/libavcodec-58.so)

add_library( avfilter-7 SHARED IMPORTED)
set_target_properties( avfilter-7 PROPERTIES IMPORTED_LOCATION ${LINK_DIR}/libavfilter-7.so)

add_library( avformat-58 SHARED IMPORTED)
set_target_properties( avformat-58 PROPERTIES IMPORTED_LOCATION ${LINK_DIR}/libavformat-58.so)

add_library( avutil-56 SHARED IMPORTED)
set_target_properties( avutil-56 PROPERTIES IMPORTED_LOCATION ${LINK_DIR}/libavutil-56.so)

add_library( swresample-3 SHARED IMPORTED)
set_target_properties( swresample-3 PROPERTIES IMPORTED_LOCATION ${LINK_DIR}/libswresample-3.so)

add_library( swscale-5 SHARED IMPORTED)
set_target_properties( swscale-5 PROPERTIES IMPORTED_LOCATION ${LINK_DIR}/libswscale-5.so)

add_library( # Sets the name of the library.
        native-lib

        # Sets the library as a shared library.
        SHARED

        # Provides a relative path to your source file(s).
        src/main/cpp/native-lib.cpp )


find_library( # Sets the name of the path variable.
        log-lib

        # Specifies the name of the NDK library that
        # you want CMake to locate.
        log )


target_link_libraries( native-lib
        avcodec-58
        avfilter-7
        avformat-58
        avutil-56
        swresample-3
        swscale-5
        ${log-lib} )