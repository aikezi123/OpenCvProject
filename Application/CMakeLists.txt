# ==========================================
# Application 层：负责最终的程序组装与依赖注入
# ==========================================

# 必须加上这两句，让 Application 层认识 Qt
find_package(Qt6 COMPONENTS Widgets REQUIRED)

# 1. 生成最终的可执行程序 (WIN32 参数隐藏 Windows 黑色控制台)
add_executable(MainApp WIN32 
    src/main.cpp
)

# 2. 链接所有业务模块和 UI 库
target_link_libraries(MainApp PRIVATE 
    AppUI            # 链接刚刚在 UI 层生成的静态库
    Qt6::Widgets   # 链接 Qt 库
    # 如果你在 main 函数中需要直接实例化底层的服务来进行依赖注入，这里也需要链接它们：
    # VideoFilterService 
    # DbClient
    # CommonLogger
)
