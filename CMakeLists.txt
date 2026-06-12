cmake_minimum_required(VERSION 3.16)
set(CMAKE_CUDA_HOST_COMPILER g++-15)
set(CMAKE_CUDA_ARCHITECTURES 89)
project(renderer LANGUAGES CXX CUDA)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# On macOS, Apple clang ships without OpenMP — requires: brew install libomp
if(APPLE)
    execute_process(
        COMMAND brew --prefix libomp
        OUTPUT_VARIABLE HOMEBREW_LIBOMP_PREFIX
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    set(OpenMP_CXX_FLAGS "-Xpreprocessor -fopenmp -I${HOMEBREW_LIBOMP_PREFIX}/include")
    set(OpenMP_CXX_LIB_NAMES "omp")
    set(OpenMP_omp_LIBRARY "${HOMEBREW_LIBOMP_PREFIX}/lib/libomp.dylib")
endif()

find_package(OpenMP REQUIRED)
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(GLEW REQUIRED)

set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/vendor/imgui)
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

# renderer exec
add_executable(renderer src/main_gpu.cpp src/gpu_render.cu ${IMGUI_SOURCES})

target_include_directories(renderer PRIVATE src)
target_include_directories(renderer SYSTEM PRIVATE 
	vendor
	${IMGUI_DIR}
	${IMGUI_DIR}/backends
)

target_compile_options(renderer PRIVATE -Wall -Wextra -O2)

target_link_libraries(renderer PRIVATE 
	OpenMP::OpenMP_CXX
	OpenGL::GL
	glfw
    GLEW::GLEW
)

# interactive renderer exec
add_executable(renderer_interactive src/main_interactive.cpp src/gpu_render.cu ${IMGUI_SOURCES})

target_include_directories(renderer_interactive PRIVATE src)
target_include_directories(renderer_interactive SYSTEM PRIVATE
    vendor
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)

target_include_directories(renderer_interactive PRIVATE ${CMAKE_CUDA_TOOLKIT_INCLUDE_DIRECTORIES})

target_compile_options(renderer_interactive PRIVATE -Wall -Wextra -O2)

target_link_libraries(renderer_interactive PRIVATE
    OpenMP::OpenMP_CXX
    OpenGL::GL
    glfw
    GLEW::GLEW
)

# Run target
add_custom_target(run
    COMMAND renderer_interactive
    DEPENDS renderer_interactive
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

# Format target
find_program(CLANG_FORMAT clang-format)
if(CLANG_FORMAT)
    file(GLOB_RECURSE ALL_SOURCE_FILES
        ${CMAKE_SOURCE_DIR}/src/*.cpp
        ${CMAKE_SOURCE_DIR}/src/*.h
		${CMAKE_SOURCE_DIR}/src/*.cu
    )
    list(FILTER ALL_SOURCE_FILES EXCLUDE REGEX "stb_image_write\\.h$")
    add_custom_target(format
        COMMAND ${CLANG_FORMAT} -i ${ALL_SOURCE_FILES}
        COMMENT "Formatting source files with clang-format"
    )
else()
    message(WARNING "clang-format not found, format target unavailable")
endif()
