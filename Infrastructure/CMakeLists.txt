add_library(Infrastructure STATIC
    src/Logger.cpp
    include/Logger.h
)

# 公开 include 目录，让别的层能找到头文件
target_include_directories(Infrastructure PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)