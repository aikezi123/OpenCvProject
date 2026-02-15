#根目录CMakeLists文件夹作用：
#统筹全局。它不直接编译任何 C++ 代码，而是负责设置全局的环境变量（如 C++ 标准、OpenCV 路径、输出目录），
#并规定各个子模块的加载顺序。


# 规定编译此项目所需的最低 CMake 版本，低于此版本会报错
cmake_minimum_required(VERSION 3.16)

# 定义项目的整体名称为 OpenCvProject，并声明这是一个 C++ (CXX) 项目
project(OpenCvProject VERSION 1.0.0 LANGUAGES CXX)

# 设置全局 C++ 标准为 C++17，这在现代 Qt 和 OpenCV 开发中是基础
set(CMAKE_CXX_STANDARD 17)


# 【魔法指令】统一输出目录
# 这三行非常关键！它把所有子模块生成的 .exe (运行时) 和静态库 (存档) 
# 全部集中输出到你项目里的 build/bin 目录下。
# 好处：运行 exe 时，再也不会报“找不到同目录下的某个 dll”这种错了！
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# 按照“从最底层到最顶层”的依赖顺序，将子文件夹加入编译队列。
# 如果顺序写反了（比如先加 UI 再加 Infrastructure），CMake 会因为找不到底层的库而报错。
add_subdirectory(Infrastructure)
add_subdirectory(Service)
add_subdirectory(Business)
add_subdirectory(UI)