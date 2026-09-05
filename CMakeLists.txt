cmake_minimum_required(VERSION 3.18)
project(cuda_raw_processor LANGUAGES CXX CUDA)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CUDA_STANDARD 17)

find_package(CUDAToolkit REQUIRED)
find_package(libraw CONFIG REQUIRED)

if(NOT DEFINED CMAKE_CUDA_ARCHITECTURES)
    set(CMAKE_CUDA_ARCHITECTURES 61;75;80;86;89)
endif()

add_executable(cuda_raw_processor
    main.cpp
    im_process.cu
)

target_include_directories(cuda_raw_processor PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(cuda_raw_processor PRIVATE CUDA::cudart libraw::raw_r)