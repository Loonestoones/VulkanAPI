# Specify the minimum version of CMake required to build this project
cmake_minimum_required(VERSION 3.16)

# Define the project name
project(VulkanTutorial)

# Set the C++ standard to C++20 (Required for std::string::ends_with)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Export compile_commands.json for IDE and LSP support
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# VCPKG Configuration
set(VCPKG_TRIPLET "x64-windows")

# --- Vulkan SDK Dependencies ---
# Vulkan SDK
find_package(Vulkan REQUIRED)

# GLFW3 from vcpkg
set(glfw3_DIR "C:/Users/kevin/Documents/cpp/Vulkan-api-tutorial/vcpkg/installed/${VCPKG_TRIPLET}/share/glfw3")
find_package(glfw3 REQUIRED)

# SPIRV-Reflect from vcpkg (use release library to avoid MSVC debug runtime mismatch)
set(unofficial-spirv-reflect_DIR "C:/Users/kevin/Documents/cpp/Vulkan-api-tutorial/vcpkg/installed/${VCPKG_TRIPLET}/share/unofficial-spirv-reflect")
find_package(unofficial-spirv-reflect REQUIRED)

# GLM (Header-only math library) from vcpkg
find_package(glm REQUIRED)

# ASSIMP (3D model loader) from vcpkg
find_package(assimp REQUIRED)

# Function to copy GLFW DLL
function(copy_glfw_dll target_name)
    add_custom_command(TARGET ${target_name} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "C:/Users/kevin/Documents/cpp/Vulkan-api-tutorial/vcpkg/installed/${VCPKG_TRIPLET}/bin/glfw3.dll"
        $<TARGET_FILE_DIR:${target_name}>
    )
endfunction()

# Function to copy ASSIMP DLL
function(copy_assimp_dll target_name)
    add_custom_command(TARGET ${target_name} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "C:/Users/kevin/Documents/cpp/Vulkan-api-tutorial/vcpkg/installed/${VCPKG_TRIPLET}/bin/assimp.dll"
        $<TARGET_FILE_DIR:${target_name}>
    )
endfunction()

# Function to copy shader files
function(copy_shader_files target_name tutorial_number)
    set(SHADER_SRC_DIR "${PROJECT_SOURCE_DIR}/Vulkan/Tutorials/tutorial${tutorial_number}/shader")
    add_custom_command(TARGET ${target_name} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${SHADER_SRC_DIR}"
        "$<TARGET_FILE_DIR:${target_name}>/shader"
    )
endfunction()

function(build_tutorial tutorial_number)
    set(TUTORIAL_NAME "tutorial${tutorial_number}")
    set(TUTORIAL_SOURCE "Vulkan/Tutorials/${TUTORIAL_NAME}/${TUTORIAL_NAME}.cpp")

    # Sources
    set(CORE_SOURCE "${PROJECT_SOURCE_DIR}/Vulkan/VulkanCore/core.cpp")
    set(UTIL_SOURCE "${PROJECT_SOURCE_DIR}/Vulkan/VulkanCore/util.cpp")
    set(DEV_UTIL_SOURCE "${PROJECT_SOURCE_DIR}/Vulkan/VulkanCore/KevDev_util.cpp")
    set(DEVICE_SOURCE "${PROJECT_SOURCE_DIR}/Vulkan/VulkanCore/device.cpp")
    set(WRAPPER_SOURCE "${PROJECT_SOURCE_DIR}/Vulkan/VulkanCore/wrapper.cpp")
    set(QUEUE_SOURCE "${PROJECT_SOURCE_DIR}/Vulkan/VulkanCore/queue.cpp")
    set(SHADER_SOURCE "${PROJECT_SOURCE_DIR}/Vulkan/VulkanCore/shader.cpp")

    add_executable(${TUTORIAL_NAME}
        ${TUTORIAL_SOURCE}
        ${CORE_SOURCE}
        ${UTIL_SOURCE}
        ${DEV_UTIL_SOURCE}
        ${DEVICE_SOURCE}
        ${WRAPPER_SOURCE}
        ${QUEUE_SOURCE}
        ${SHADER_SOURCE}
    )

    # Link libraries
    target_link_libraries(${TUTORIAL_NAME} PRIVATE
        Vulkan::Vulkan
        glfw
        unofficial::spirv-reflect
        glm::glm
        assimp::assimp
    )

    target_include_directories(${TUTORIAL_NAME} PRIVATE
        ${Vulkan_INCLUDE_DIRS}
        ${PROJECT_SOURCE_DIR}/Vulkan/VulkanCore
    )

    copy_glfw_dll(${TUTORIAL_NAME})
    copy_assimp_dll(${TUTORIAL_NAME})
    copy_shader_files(${TUTORIAL_NAME} ${tutorial_number})
endfunction()

build_tutorial(08)
