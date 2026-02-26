# 将 DbClient 编译成一个独立的静态库
add_library(DbClient SHARED
	src/DbClient.cpp
	include/DbClient.h
)

# 暴露 include 目录，让链接了 DbClient 的上层模块能直接 #include "DbClient.h"
target_include_directories(DbClient 
	PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
	PRIVATE ${CMAKE_CURRENT_DIRECTORY_DIR}/src
)