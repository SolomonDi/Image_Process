cmake_minimum_required(VERSION 3.18)

if(NOT DEFINED CMAKE_CUDA_ARCHITECTURES)
    set(CMAKE_CUDA_ARCHITECTURES 75;80;86;89;120)
endif()

project(cuda_raw_processor LANGUAGES CXX CUDA)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CUDA_STANDARD 17)


set(CMAKE_PREFIX_PATH "C:/Qt/6.7.3/msvc2019_64" ${CMAKE_PREFIX_PATH})

find_package(CUDAToolkit REQUIRED)
find_package(libraw CONFIG REQUIRED)
find_package(TIFF REQUIRED)
find_package(Qt6 COMPONENTS Widgets REQUIRED)

add_executable(cuda_raw_processor
    main.cpp
    im_process.cu
)

target_include_directories(cuda_raw_processor PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(cuda_raw_processor PRIVATE
    CUDA::cudart
    libraw::raw_r
    TIFF::TIFF
)

# --- GUI версия на Qt ---
add_executable(cuda_raw_gui
    gui.cpp
    im_process.cu
)

target_include_directories(cuda_raw_gui PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(cuda_raw_gui PRIVATE
    CUDA::cudart
    libraw::raw_r
    TIFF::TIFF
    Qt6::Widgets
)

set_target_properties(cuda_raw_gui PROPERTIES AUTOMOC ON)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set_target_properties(cuda_raw_gui PROPERTIES WIN32_EXECUTABLE ON)
    target_link_options(cuda_raw_gui PRIVATE /ENTRY:mainCRTStartup)
endif()
