# 开启 Qt 的自动化处理
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# 【关键配置】：告诉 Qt 的 AUTOUIC 工具，去 "ui" 文件夹里寻找 .ui 文件
set(CMAKE_AUTOUIC_SEARCH_PATHS "ui")

find_package(Qt6 COMPONENTS Core Gui Widgets REQUIRED)

# 1. 收集所有文件 (带上对应的前缀路径)
set(UI_SOURCES
    src/main.cpp
    src/MainWindow.cpp
    src/ImageShowView.cpp
)

set(UI_HEADERS
    include/MainWindow.h
    include/ImageShowView.h
)

set(UI_FORMS
    ui/MainWindow.ui
    ui/ImageShowView.ui
)

# 2. 生成可执行文件
add_executable(AppUI
    ${UI_SOURCES}
    ${UI_HEADERS}
    ${UI_FORMS}
)

# 3. 设置头文件搜索路径，让 src 里的 cpp 能直接 #include "MainWindow.h"
target_include_directories(AppUI PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# 4. 链接依赖
target_link_libraries(AppUI PRIVATE
    Qt6::Core Qt6::Gui Qt6::Widgets
    TargetRecognition      # 引入业务
    ImageFilterService     # 引入具体的 OpenCV 实现
    Logger                 # 引入具体的日志实现
)