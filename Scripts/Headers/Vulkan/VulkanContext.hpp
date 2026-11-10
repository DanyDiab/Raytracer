#pragma once

#include <cstdint>
#include <optional>
#include <vector>
#include <vulkan/vulkan_core.h>
#include <GLFW/glfw3.h>

struct QueueFamilyIndicies {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool hasAllQueues() const {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

struct SwapChainInfo {
    std::vector<VkImage> images;
    std::vector<VkImageView> views;
    VkExtent2D swapChainExtent{};
    VkFormat swapChainImageFormat = VK_FORMAT_UNDEFINED;
};

struct VulkanContext{
    GLFWwindow* window = nullptr;

    VkInstance instance = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;
    VkSurfaceKHR surface = VK_NULL_HANDLE;

    VkPhysicalDevice Pdevice = VK_NULL_HANDLE;
    QueueFamilyIndicies queueFamilies;
    VkDevice Ldevice = VK_NULL_HANDLE;
    VkQueue graphicQueue = VK_NULL_HANDLE;
    VkQueue presentQueue = VK_NULL_HANDLE;

    VkSwapchainKHR swapChain = VK_NULL_HANDLE;
    SwapChainInfo swapChainInfo;
    VkRenderPass renderPass = VK_NULL_HANDLE;
    std::vector<VkFramebuffer> swapChainFrameBuffers;

    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    VkPipeline graphicsPipeline = VK_NULL_HANDLE;

    VkCommandPool commandPool = VK_NULL_HANDLE;
    VkCommandBuffer commandBuffer = VK_NULL_HANDLE;

    VkSemaphore imageAvailableSemaphore = VK_NULL_HANDLE;
    VkSemaphore renderFinishedSemaphore = VK_NULL_HANDLE;
    VkFence inFlightFence = VK_NULL_HANDLE;
};
