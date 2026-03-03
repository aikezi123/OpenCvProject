# 1. 抓取你的源代码
file(GLOB_RECURSE HIK_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB_RECURSE HIK_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

# 2. 生成 Infrastructure 的海康插件库 (动态或静态)
add_library(HikCamera SHARED
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

# B. 链接库文件目录 (让连接器能找到 .lib 所在的文件夹)
# 注意区分 64 位还是 32 位，这里假设是 64 位 win64
target_link_directories(HikCamera PRIVATE 
    "${HIK_SDK_DIR}/libraries"
)

# C. 链接具体的 .lib 文件
target_link_libraries(HikCamera PRIVATE 
    MvCameraControl.lib 
)