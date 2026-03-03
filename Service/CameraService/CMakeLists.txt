# 1.由于要使用Qt6的信号与槽，需要寻找Qt6的库
find_package(Qt6 COMPONENTS Core REQUIRED)

# 2.开启自动元对象编译
set(CMAKE_AUTOMOC ON)

# 3.抓取源代码
file(GLOB_RECURSE CAMERA_SERVICE_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
file(GLOB_RECURSE CAMERA_SERVICE_HEADERS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")

# 4.生成动态库
add_library(CameraService STATIC
	${CAMERA_SERVICE_SOURCES}
	${CAMERA_SERVICE_HEADERS}
)

# 5.暴露头文件路径
target_include_directories(CameraService PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/include
)

# 6.链接依赖库
target_link_libraries(CameraService PUBLIC
	Qt6::Core
	CameraCore
)