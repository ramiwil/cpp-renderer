cmake_minimum_required(VERSION 3.16)
project(raytracer LANGUAGES CXX)

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

add_executable(raytracer src/main.cpp)

target_include_directories(raytracer PRIVATE src vendor)

target_compile_options(raytracer PRIVATE -Wall -Wextra -O2)

target_link_libraries(raytracer PRIVATE OpenMP::OpenMP_CXX)

# Run target
add_custom_target(run
    COMMAND raytracer
    DEPENDS raytracer
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

# Format target
find_program(CLANG_FORMAT clang-format)
if(CLANG_FORMAT)
    file(GLOB_RECURSE ALL_SOURCE_FILES
        ${CMAKE_SOURCE_DIR}/src/*.cpp
        ${CMAKE_SOURCE_DIR}/src/*.h
    )
    list(FILTER ALL_SOURCE_FILES EXCLUDE REGEX "stb_image_write\\.h$")
    add_custom_target(format
        COMMAND ${CLANG_FORMAT} -i ${ALL_SOURCE_FILES}
        COMMENT "Formatting source files with clang-format"
    )
else()
    message(WARNING "clang-format not found, format target unavailable")
endif()
