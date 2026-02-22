find_package(Qt6 COMPONENTS Core REQUIRED)

add_library(TargetRecognition STATIC
    src/TargetRecognition.cpp
    include/TargetRecognition.h
    include/EyeDataTypes.h
    include/IEyeProcessor.h
)

target_include_directories(TargetRecognition 
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(TargetRecognition 
    PUBLIC Qt6::Core
    CommonLogger
)

