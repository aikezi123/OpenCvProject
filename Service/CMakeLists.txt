find_package(OpenCV REQUIRED)

add_library(Service STATIC
    src/ImageFilterService.cpp  # 注意这里的路径，如果你没有建 src 文件夹，就把 src/ 删掉
    include/ImageFilterService.h # 同理，如果没有 include 文件夹，就把 include/ 删掉
)

# ---------------------------------------------------------
# 👇 就是改这里：用 ${OpenCV_LIBS} 替换原来的 OpenCV::World
# ---------------------------------------------------------
target_link_libraries(Service PUBLIC 
    ${OpenCV_LIBS}   
    Infrastructure  
)

target_include_directories(Service PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/include # 如果你没有 include 文件夹，改成 ${CMAKE_CURRENT_SOURCE_DIR}
)