#include <stdexcept>
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#include "../Headers/Vulkan/VulkanSurface.hpp"

VulkanSurface::VulkanSurface(VulkanContext& context) : ctx(context){
    if(glfwVulkanSupported() == GLFW_FALSE){
        throw std::runtime_error("glfw vulkan not supported");
    }

    if(glfwCreateWindowSurface(ctx.instance, ctx.window, nullptr, &ctx.surface) != VK_SUCCESS){
        throw std::runtime_error("failed to create window surface");
    }
}

VulkanSurface::~VulkanSurface(){
    vkDestroySurfaceKHR(ctx.instance, ctx.surface, nullptr);
}
