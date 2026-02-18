find_package(OpenCV REQUIRED)

add_library(ImageFilterService STATIC
    src/ImageFilterService.cpp  # 注意这里的路径，如果你没有建 src 文件夹，就把 src/ 删掉
    include/ImageFilterService.h # 同理，如果没有 include 文件夹，就把 include/ 删掉
)

target_include_directories(ImageFilterService 
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# ---------------------------------------------------------
# 👇 就是改这里：用 ${OpenCV_LIBS} 替换原来的 OpenCV::World
# ---------------------------------------------------------
target_link_libraries(ImageFilterService PUBLIC 
    TargetRecognition   # 【反向依赖】：依赖业务层去获取接口头文件！
    ${OpenCV_LIBS}  
)

