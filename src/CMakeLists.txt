add_library(
    kubernetes
    config.cpp
)


target_include_directories(
    kubernetes 
    PUBLIC 
    "${PROJECT_SOURCE_DIR}/include"
)

find_package(glaze CONFIG REQUIRED)

target_link_libraries(
    kubernetes
    PUBLIC
    glaze::glaze
)