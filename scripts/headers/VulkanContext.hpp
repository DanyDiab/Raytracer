#pragma once

#include <vulkan/vulkan_core.h>
#include <vector>

struct VulkanContext{
    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkDevice Ldevice;
    VkPhysicalDevice Pdevice;
    VkQueue graphicQueue;
    VkQueue presentQueue;
    VkSurfaceKHR surface;
    VkSwapchainKHR swapChain;
    VkRenderPass renderPass;
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;

    std::vector<VkFramebuffer> swapChainFrameBuffers;

};
