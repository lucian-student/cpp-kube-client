add_library(
    kubernetes
    config.cpp
)


target_include_directories(
    kubernetes 
    PUBLIC 
    "${PROJECT_SOURCE_DIR}/include"
)