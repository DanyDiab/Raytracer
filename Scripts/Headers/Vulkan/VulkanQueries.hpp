#pragma once

#include <vector>
#include <vulkan/vulkan_core.h>

#include "VulkanContext.hpp"

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

QueueFamilyIndicies findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);
bool checkDeviceExtensionSupport(VkPhysicalDevice device, const std::vector<const char*>& requiredExtensions);
SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);
