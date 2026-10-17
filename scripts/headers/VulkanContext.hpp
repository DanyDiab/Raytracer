#pragma once

#include <vulkan/vulkan_core.h>

struct VulkanContext{
    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkDevice Ldevice;
    VkPhysicalDevice Pdevice;
    VkQueue graphicQueue;
    VkQueue presentQueue;
    VkSurfaceKHR surface;
    VkSwapchainKHR swapChain;
    VkPipelineLayout pipelineLayout;

};
