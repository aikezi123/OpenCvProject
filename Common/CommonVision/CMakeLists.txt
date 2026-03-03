# ==========================================
# 模块：CommonVision (底层视觉基建)
# 作用：将电脑上的 OpenCV 打包为内部模块，供全系统调用
# ==========================================

# 1.在系统中寻找OpenCv（环境变量中已经配好了OpenCV_DIR这个变量）
find_package(OpenCV REQUIRED)

# 2.创建一个“虚空”的接口库（INTERFACE意味着它不自己产出dll或lib）
add_library(CommonVision INTERFACE)

# 3.将OpenCv的头文件传给这个接口库
target_include_directories(CommonVision INTERFACE 
	${OpenCV_INCLUDE_DIRS}
)

# 4.将OpenCV的具体库文件传送给这个接口库
target_link_libraries(CommonVision INTERFACE
	${OpenCV_LIBS}
)