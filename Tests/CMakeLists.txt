find_package(OpenCV REQUIRED)
# 将 TestsService.cpp 编译为一个独立的可执行文件，命名为 AlgorithmTester
add_executable(AlgorithmTester TestsService.cpp)

# 给它链接上 OpenCV 库，这样它才能懂 cv::imread 和 cv::imshow
target_link_libraries(AlgorithmTester PRIVATE ${OpenCV_LIBS})