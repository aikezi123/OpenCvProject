# 将 Logger.cpp 编译成一个静态库（STATIC），取名为 Logger。
# 静态库不会生成 .dll，而是会像积木一样直接打包进最终的 .exe 里，方便部署。
add_library(Logger STATIC
    src/Logger.cpp
    include/Logger.h
)
# 【核心解耦技术】向外暴露头文件路径
# PUBLIC 意味着：不仅我自己能找到 Logger.h，
# 任何 link (链接) 了 Infrastructure 的上一层模块，也能直接 #include "Logger.h"。
# 公开 include 目录，让别的层能找到头文件
target_include_directories(Logger
	PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)