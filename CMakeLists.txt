cmake_minimum_required(VERSION 3.15)
if(NOT DEFINED CMAKE_CUDA_HOST_COMPILER)
    set(CMAKE_CUDA_HOST_COMPILER /usr/bin/g++-14)
endif()
project(Raytracer VERSION 1.0 LANGUAGES CXX CUDA)

set(CMAKE_CUDA_ARCHITECTURES native)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CUDA_STANDARD 17)
set(CMAKE_CUDA_STANDARD_REQUIRED ON)
set(CMAKE_CUDA_SEPARATE_COMPILATION ON)


find_package(glfw3 3.3.10 REQUIRED)
find_package(CUDAToolkit REQUIRED)
find_package(VulkanHeaders REQUIRED)
find_package(Vulkan REQUIRED)

set(SCRIPTSDIR "${CMAKE_CURRENT_SOURCE_DIR}/Scripts")
set(CAMERA_RAYS_DIR "${SCRIPTSDIR}/CameraRays")
set(HITTABLES_DIR "${SCRIPTSDIR}/Hittables")
set(MISC_DIR "${SCRIPTSDIR}/Misc")
set(VULKAN_DIR "${SCRIPTSDIR}/Vulkan")
set(SCENE_DIR "${SCRIPTSDIR}/Scenes")
set(SHADER_DIR "${SCRIPTSDIR}/Shaders")
set(HEADERS_DIR "${SCRIPTSDIR}/Headers")

add_executable(Raytracer
    "${SCRIPTSDIR}/Main.cpp"
    "${SCENE_DIR}/SceneLoader.cpp"
    "${SCENE_DIR}/BVHTestScene.cpp"
    "${SCENE_DIR}/BunnyMirrorScene.cpp"

    "${SHADER_DIR}/ShaderLoader.cpp"
    "${VULKAN_DIR}/Vulkan.cpp"
    "${VULKAN_DIR}/VulkanWindow.cpp"
    "${VULKAN_DIR}/VulkanInstance.cpp"
    "${VULKAN_DIR}/VulkanSurface.cpp"
    "${VULKAN_DIR}/VulkanDevice.cpp"
    "${VULKAN_DIR}/VulkanSwapChain.cpp"
    "${VULKAN_DIR}/VulkanPipeline.cpp"
    "${VULKAN_DIR}/VulkanRenderer.cpp"
    "${VULKAN_DIR}/VulkanQueries.cpp"

    "${CAMERA_RAYS_DIR}/Camera.cu"
    "${CAMERA_RAYS_DIR}/Ray.cu"
    "${CAMERA_RAYS_DIR}/RayAveraging.cu"

    "${HITTABLES_DIR}/Hittable.cu"
    "${HITTABLES_DIR}/Sphere.cu"
    "${HITTABLES_DIR}/Triangle.cu"
    "${HITTABLES_DIR}/Mesh.cpp"

    "${MISC_DIR}/PRNG.cu"
    "${MISC_DIR}/Window.cpp"
    "${MISC_DIR}/ObjReader.cpp"
    "${MISC_DIR}/PpmWriter.cpp"
    "${MISC_DIR}/CPUTimer.cpp"
    "${MISC_DIR}/GPUTimer.cu"


)
set_target_properties(Raytracer PROPERTIES
    CUDA_SEPARATE_COMPILATION ON
    CUDA_RESOLVE_DEVICE_SYMBOLS ON
    LINKER_LANGUAGE CUDA
)


target_include_directories(Raytracer PRIVATE
    "${HEADERS_DIR}/Camera"
    "${HEADERS_DIR}/Hittables"
    "${HEADERS_DIR}/RayHits"
    "${HEADERS_DIR}/Util"
)

target_compile_options(Raytracer PRIVATE $<$<COMPILE_LANGUAGE:CUDA>:--relocatable-device-code=true --expt-relaxed-constexpr>)
target_compile_options(Raytracer PRIVATE $<$<AND:$<COMPILE_LANGUAGE:CUDA>,$<CONFIG:Debug>>:-G>)

target_link_libraries(Raytracer PRIVATE glfw CUDA::cudart Vulkan::Vulkan)

set(SHADER_SOURCES
    "${SHADER_DIR}/tri.vert"
    "${SHADER_DIR}/tri.frag"
)

set(SHADER_BINARIES "")
foreach(SHADER ${SHADER_SOURCES})
    get_filename_component(SHADER_NAME ${SHADER} NAME)
    set(SPIRV "${SHADER_DIR}/${SHADER_NAME}.spv")

    add_custom_command(
        OUTPUT ${SPIRV}
        COMMAND ${Vulkan_GLSLC_EXECUTABLE} ${SHADER} -o ${SPIRV}
        DEPENDS ${SHADER}
        COMMENT "Compiling ${SHADER_NAME}"
    )
    list(APPEND SHADER_BINARIES ${SPIRV})
endforeach()

add_custom_target(Shaders DEPENDS ${SHADER_BINARIES})
add_dependencies(Raytracer Shaders)

target_compile_options(Raytracer PRIVATE -g)
