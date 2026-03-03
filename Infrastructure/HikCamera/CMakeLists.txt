# 1. 抓取你的源代码
file(GLOB_RECURSE HIK_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB_RECURSE HIK_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

# 2. 生成 Infrastructure 的海康插件库 (动态或静态)
add_library(HikCameraImpl SHARED
    ${HIK_SOURCES} 
    ${HIK_HEADERS}
)

# 3. 引入 Business 层，因为你要继承 ICamera 接口
# target_link_libraries(HikCameraImpl PRIVATE Business)

# ==========================================
# 4. 接入海康第三方 SDK
# ==========================================
# 设置海康 SDK 的根目录变量 (基于项目根目录计算)
set(HIK_SDK_DIR "${CMAKE_SOURCE_DIR}/3rdparty/Hikvision")

# A. 包含头文件目录 (让编译器能找到 MvCameraControl.h)
target_include_directories(HikCameraImpl PRIVATE 
    "${HIK_SDK_DIR}/include"
)

# B. 链接库文件目录 (让连接器能找到 .lib 所在的文件夹)
# 注意区分 64 位还是 32 位，这里假设是 64 位 win64
target_link_directories(HikCameraImpl PRIVATE 
    "${HIK_SDK_DIR}/libraries"
)

# C. 链接具体的 .lib 文件
target_link_libraries(HikCameraImpl PRIVATE 
    MvCameraControl.lib 
)