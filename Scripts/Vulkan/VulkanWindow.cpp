#include <stdexcept>
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#include "../Headers/Vulkan/VulkanWindow.hpp"

VulkanWindow::VulkanWindow(VulkanContext& context) : ctx(context){
    glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11);
    if(!glfwInit()){
        throw std::runtime_error("glfw init failed");
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    ctx.window = glfwCreateWindow(960, 540, "RayTracer", nullptr, nullptr);
    if(!ctx.window){
        glfwTerminate();
        throw std::runtime_error("GLFW createWindow failed");
    }
}

VulkanWindow::~VulkanWindow(){
    glfwDestroyWindow(ctx.window);
    glfwTerminate();
}
