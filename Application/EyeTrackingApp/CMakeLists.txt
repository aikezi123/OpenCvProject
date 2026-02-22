# 必须开启自动 MOC，否则含有 Q_OBJECT 的类无法链接！
set(CMAKE_AUTOMOC ON)
find_package(Qt6 COMPONENTS Core Gui REQUIRED)

add_library(EyeTrackingApp STATIC
    src/EyeWorker.cpp
    include/EyeWorker.h
)

target_include_directories(EyeTrackingApp 
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(EyeTrackingApp
	PUBLIC 
		Qt6::Core 
        Qt6::Gui
        CommonLogger
        TargetRecognition     # 核心：必须调用业务逻辑
        ImageFilterService    # 核心：为了实例化具体的工具进行依赖注入
        ${OpenCV_LIBS}        # 因为 EyeWorker 里的 cv::imdecode 用到了 OpenCV
)                             