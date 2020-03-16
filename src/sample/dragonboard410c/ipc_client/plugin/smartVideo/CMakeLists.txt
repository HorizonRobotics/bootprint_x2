cmake_minimum_required(VERSION 3.10)
project(smartvideo)

set(CMAKE_CXX_STANDARD 11)
add_definitions(-D BUILDING_PLUGIN)
# 编译模式
if (${RELEASE_LIB})
    set(CMAKE_BUILD_TYPE Release)
else ()
    set(CMAKE_BUILD_TYPE Debug)
endif ()

include_directories(./include)
link_directories(./)

SET(CMAKE_C_COMPILER /home/yulingong/gcc-linaro-6.5.0-2018.12-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-gcc)
SET(CMAKE_CXX_COMPILER /home/yulingong/gcc-linaro-6.5.0-2018.12-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-g++)

#头文件
file(GLOB PROJECT_INC
        "*.h"
        "*.hpp"
        )

#源文件
file(GLOB PROJECT_SRC
        "*.cpp"
        "*.c"
        )
set(SOURCE_FILES
        ${PROJECT_SRC}
        ${PROJECT_INC}
        )
set(BASE_LIBRARIES xpluginflow pthread)

add_library(smartvideo SHARED ${SOURCE_FILES})
#add_executable(smartvideo ${SOURCE_FILES})

target_link_libraries(smartvideo ${BASE_LIBRARIES})
