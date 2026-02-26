# 必须开启自动 MOC，否则含有 Q_OBJECT 的类无法链接！
set(CMAKE_AUTOMOC ON)
find_package(Qt6 COMPONENTS Core Gui REQUIRED)

add_library(EyeTrackingApp SHARED
    src/EyeWorker.cpp
    include/EyeWorker.h
)

# 👇 加上这一行！告诉编译器：“兄弟，我们现在正在造 DLL，请把护照设为 EXPORT 模式！”
target_compile_definitions(EyeTrackingApp PRIVATE BUILD_EYE_APP_DLL)

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
        OpenCvVision    # 核心：为了实例化具体的工具进行依赖注入
        ${OpenCV_LIBS}        # 因为 EyeWorker 里的 cv::imdecode 用到了 OpenCV
)                             