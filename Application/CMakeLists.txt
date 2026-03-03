# ==========================================
# Application 层：负责最终的程序组装与依赖注入
# ==========================================

# 必须加上这两句，让 Application 层认识 Qt
find_package(Qt6 COMPONENTS Widgets REQUIRED)

set(CMAKE_AUTOMOC ON)

# 抓取源码
file(GLOB_RECURSE APP_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
file(GLOB_RECURSE APP_HEADERS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")

add_executable(MainApp ${APP_SOURCES} ${APP_HEADERS})

# 暴露 include 目录
target_include_directories(MainApp PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# 链接你所有的层
target_link_libraries(MainApp PRIVATE
    AppUI
    CameraService
    HikCamera
    Qt6::Core
    Qt6::Widgets
)
