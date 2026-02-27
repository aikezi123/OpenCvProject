# ==========================================
# 1. 声明寻找 Qt6 
# ==========================================
find_package(Qt6 COMPONENTS Core Gui Widgets REQUIRED)

# ==========================================
# 2. 开启 Qt 自动编译黑科技
# ==========================================
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# ==========================================
# 3. 递归搜索所有代码和 UI 文件
# ==========================================
file(GLOB_RECURSE UI_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB_RECURSE UI_HEADERS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
file(GLOB_RECURSE UI_FORMS   CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.ui")
file(GLOB_RECURSE UI_RESOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.qrc")

# ==========================================
# 4. 生成执行程序
# ==========================================
add_executable(AppUI WIN32 ${UI_SOURCES} ${UI_HEADERS} ${UI_FORMS} ${UI_RESOURCES})

# ==========================================
# 5. 【修复 include 寻址】：自动解析并绑定所有的 include 路径
# ==========================================
set(UI_INCLUDE_DIRS "")
foreach(_header_file ${UI_HEADERS})
    get_filename_component(_dir ${_header_file} DIRECTORY)
    list(APPEND UI_INCLUDE_DIRS ${_dir})
endforeach()

if(UI_INCLUDE_DIRS)
    list(REMOVE_DUPLICATES UI_INCLUDE_DIRS)
    target_include_directories(AppUI PRIVATE ${UI_INCLUDE_DIRS})
endif()

# ==========================================
# 6. 【UI 寻址】：告诉 AUTOUIC 所有的 .ui 文件在哪
# ==========================================
set(UI_FORM_DIRS "")
foreach(_ui_file ${UI_FORMS})
    get_filename_component(_dir ${_ui_file} DIRECTORY)
    list(APPEND UI_FORM_DIRS ${_dir})
endforeach()

if(UI_FORM_DIRS)
    list(REMOVE_DUPLICATES UI_FORM_DIRS)
    # 把提取到的所有 ui 文件夹路径，告诉 Qt 的自动编译工具！
    set_property(TARGET AppUI PROPERTY AUTOUIC_SEARCH_PATHS ${UI_FORM_DIRS})
endif()

# ==========================================
# 7. 严谨的防腐层链接
# ==========================================
target_link_libraries(AppUI PRIVATE 
    Qt6::Core 
    Qt6::Gui 
    Qt6::Widgets 
    CommonLogger
    EyeTrackingApp
)