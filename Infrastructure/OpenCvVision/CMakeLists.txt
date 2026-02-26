find_package(OpenCV REQUIRED)

add_library(OpenCvVision SHARED
    src/OpenCvEyeDetector.cpp  # 注意这里的路径，如果你没有建 src 文件夹，就把 src/ 删掉
    include/OpenCvEyeDetector.h # 同理，如果没有 include 文件夹，就把 include/ 删掉
)

target_include_directories(OpenCvVision 
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# ---------------------------------------------------------
# 👇 就是改这里：用 ${OpenCV_LIBS} 替换原来的 OpenCV::World
# ---------------------------------------------------------
target_link_libraries(OpenCvVision
    PUBLIC 
    ${OpenCV_LIBS} 
    CommonLogger 
    TargetRecognition  # <--- 【极度关键】必须加上这个，才能打通头文件搜索路径！
)

