#include <set>
#include <stdexcept>
#include <string>
#include <vector>
#include <vulkan/vulkan.h>

#include "../Headers/Vulkan/VulkanQueries.hpp"

QueueFamilyIndicies findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface){
    QueueFamilyIndicies indices;

    uint32_t count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &count, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(count);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &count, queueFamilies.data());

    for(uint32_t i = 0; i < count; i++){
        // can we do graphics? aka buffers etc...
        if(queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT){
            indices.graphicsFamily = i;
        }

        // check for presentation support to the window surface
        VkBool32 presentSupport = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

        if(presentSupport){
            indices.presentFamily = i;
        }
    }

    return indices;
}

bool checkDeviceExtensionSupport(VkPhysicalDevice device, const std::vector<const char*>& requiredExtensions){
    uint32_t extensionCount = 0;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> remaining(requiredExtensions.begin(), requiredExtensions.end());

    for(const auto& extension : availableExtensions){
        remaining.erase(extension.extensionName);
    }

    // if set empty, we have all extensions
    return remaining.empty();
}

SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface){
    SwapChainSupportDetails details;

    if(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities) != VK_SUCCESS){
        throw std::runtime_error("failed to query swap chain surface capabilities");
    }

    uint32_t formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

    if(formatCount != 0){
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
    }

    uint32_t modeCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &modeCount, nullptr);

    if(modeCount != 0){
        details.presentModes.resize(modeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &modeCount, details.presentModes.data());
    }

    return details;
}
