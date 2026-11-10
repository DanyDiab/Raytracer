#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#include "../Headers/Vulkan/Vulkan.hpp"

Vulkan::Vulkan() : window(ctx), instance(ctx), surface(ctx), device(ctx), swapChain(ctx), pipeline(ctx), renderer(ctx){}

Vulkan::~Vulkan(){
    vkDeviceWaitIdle(ctx.Ldevice);
}

void Vulkan::run(){
    while(!glfwWindowShouldClose(ctx.window)){
        glfwPollEvents();
        renderer.drawFrame();
    }
}
