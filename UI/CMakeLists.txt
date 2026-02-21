# UI/CMakeLists.txt

# 1. 这三行必须有，且必须放在 add_executable 前面
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)   # 核心：开启 UI 自动编译
set(CMAKE_AUTORCC ON)

# 👇 【告诉 Qt 去 ui 文件夹找界面文件！】 👇
set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_CURRENT_SOURCE_DIR}/ui)

find_package(Qt6 COMPONENTS Core Gui Widgets REQUIRED)

# 2. 检查这里！必须把你画的界面文件加进源文件列表里
add_executable(AppUI
    src/main.cpp              # 你的 main 函数文件
    src/MainWindow.cpp
    include/MainWindow.h
    ui/MainWindow.ui          # <--- 【极其关键！】必须加上你的 .ui 文件路径！
)

target_include_directories(AppUI PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# 3. UI 层的依赖依然保持极简
target_link_libraries(AppUI PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    EyeTrackingApp            # 只认识 Application 层
)