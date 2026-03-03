# 在当前源文件目录及其子目录中，递归查找所有的.cpp文件，并保存到HIKSOURCE变量中
# CONFIGURE_DEPENDS：每次新增或删除新文件时会重新编译新文件
file(GLOB_RECURSE CAMERA_CORE_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
file(GLOB_RECURSE CAMERA_CORE_HEADERS CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

# 2. 创建动态库
add_library(CameraCore STATIC
    ${CAMERA_CORE_SOURCES}
    ${CAMERA_CORE_HEADERS}
)

# ==========================================
# 3. 暴露头文件目录 (修复点)
# ==========================================
# 直接指定 include 文件夹所在的目录即可，不需要传文件列表。
target_include_directories(CameraCore PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# ==========================================
# 4. 链接依赖 (修复点)
# ==========================================
# 必须使用 PUBLIC！让上层(Service/Application)也能顺藤摸瓜找到 cv::Mat 和日志工具
target_link_libraries(CameraCore PUBLIC
    CommonVision
    CommonLogger
)