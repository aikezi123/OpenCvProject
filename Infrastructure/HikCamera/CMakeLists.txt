# 1. 抓取你的源代码
file(GLOB_RECURSE HIK_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB_RECURSE HIK_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

# 2. 生成 Infrastructure 的海康插件库 (动态或静态)
add_library(HikCamera STATIC
    ${HIK_SOURCES} 
    ${HIK_HEADERS}
)


# 3.对外暴露自己的头文件路径
target_include_directories(HikCamera PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# 4.链接核心依赖库
# 引入我们刚建立的 CameraCore（为了继承 ICamera）
# 以及 Qt6 的 Core（因为我们要继承 QObject 和使用信号槽
target_link_libraries(HikCamera PUBLIC
    CameraCore
    CommonVision
)

# ==========================================
# 5. 接入海康第三方 SDK
# ==========================================
# 设置海康 SDK 的根目录变量 (基于项目根目录计算)
set(HIK_SDK_DIR "${CMAKE_SOURCE_DIR}/3rdparty/Hikvision")

# A. 包含头文件目录 (让编译器能找到 MvCameraControl.h)
target_include_directories(HikCamera PRIVATE 
    "${HIK_SDK_DIR}/include"
)
#直接把路径和文件名拼在一起传给 target_link_libraries
target_link_libraries(HikCamera PRIVATE 
    "${HIK_SDK_DIR}/libraries/MvCameraControl.lib"
)