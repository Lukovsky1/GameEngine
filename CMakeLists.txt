cmake_minimum_required(VERSION 4.1)
project(Game)

set(CMAKE_CXX_STANDARD 20)

find_package(glfw3 REQUIRED)
find_package(Vulkan REQUIRED)
find_program(SLANGC_EXECUTABLE NAMES slangc HINTS ENV VULKAN_SDK PATH_SUFFIXES bin)
if(NOT SLANGC_EXECUTABLE)
    message(FATAL_ERROR "slangc not found. Install Slang or set SLANGC_EXECUTABLE/VULKAN_SDK.")
endif()

include_directories(/opt/homebrew/include)
link_directories(/opt/homebrew/lib)

function (add_slang_shader_target TARGET)
    cmake_parse_arguments ("SHADER" "" "" "SOURCES" ${ARGN})
    set (SHADERS_DIR ${CMAKE_CURRENT_BINARY_DIR}/shaders)
    set (ENTRY_POINTS -entry vertMain -entry fragMain)
    set (SHADER_OUTPUT ${SHADERS_DIR}/slang.spv)
    add_custom_command (
            OUTPUT ${SHADERS_DIR}
            COMMAND ${CMAKE_COMMAND} -E make_directory ${SHADERS_DIR}
    )
    add_custom_command (
            OUTPUT  ${SHADER_OUTPUT}
            COMMAND ${SLANGC_EXECUTABLE} ${SHADER_SOURCES} -target spirv -profile spirv_1_4 -emit-spirv-directly -fvk-use-entrypoint-name ${ENTRY_POINTS} -o ${SHADER_OUTPUT}
            WORKING_DIRECTORY ${SHADERS_DIR}
            DEPENDS ${SHADERS_DIR} ${SHADER_SOURCES}
            COMMENT "Compiling Slang Shaders"
            VERBATIM
    )
    add_custom_target (${TARGET} DEPENDS ${SHADER_OUTPUT})
endfunction()

set(SHADER_SLANG_SOURCES ${CMAKE_CURRENT_LIST_DIR}/shaders/shaders.slang)

add_executable(Game main.cpp
        HelloTriangleApplication.cpp
        HelloTriangleApplication.h
)
add_slang_shader_target(foo SOURCES ${SHADER_SLANG_SOURCES})
add_dependencies(Game foo)

target_link_libraries(Game glfw Vulkan::Vulkan)

target_include_directories(Game PRIVATE /opt/homebrew/include)
