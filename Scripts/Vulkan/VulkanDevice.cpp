#include <cstdint>
#include <iostream>
#include <set>
#include <stdexcept>
#include <vector>
#include <vulkan/vulkan.h>

#include "../Headers/Vulkan/VulkanDevice.hpp"
#include "../Headers/Vulkan/VulkanQueries.hpp"

namespace {

const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

}

VulkanDevice::VulkanDevice(VulkanContext& context) : ctx(context){
    pickPhysicalDevice();
    createLogicalDevice();
}

VulkanDevice::~VulkanDevice(){
    vkDestroyDevice(ctx.Ldevice, nullptr);
}

bool VulkanDevice::isDeviceSuitable(VkPhysicalDevice device) const{
    QueueFamilyIndicies famIndices = findQueueFamilies(device, ctx.surface);
    if(!famIndices.hasAllQueues()){
        return false;
    }

    if(!checkDeviceExtensionSupport(device, deviceExtensions)){
        return false;
    }

    SwapChainSupportDetails scDetails = querySwapChainSupport(device, ctx.surface);
    return !scDetails.formats.empty() && !scDetails.presentModes.empty();
}

int VulkanDevice::rateDeviceSuitability(VkPhysicalDevice device) const{
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);

    int score = 0;

    if(deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU){
        score += 1000;
    }

    // if we get to here, we have all queues
    // prefer if the graphics family and the present family are the same queue (more performance)
    QueueFamilyIndicies famIndices = findQueueFamilies(device, ctx.surface);
    if(famIndices.graphicsFamily == famIndices.presentFamily){
        score += 500;
    }

    return score;
}

void VulkanDevice::pickPhysicalDevice(){
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(ctx.instance, &deviceCount, nullptr);

    if(deviceCount == 0){
        throw std::runtime_error("no GPUs with Vulkan support found");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(ctx.instance, &deviceCount, devices.data());

    VkPhysicalDevice bestDevice = VK_NULL_HANDLE;
    int bestScore = -1;

    for(const auto& device : devices){
        if(!isDeviceSuitable(device)) continue;

        int score = rateDeviceSuitability(device);

        if(score > bestScore){
            bestScore = score;
            bestDevice = device;
        }
    }

    if(bestDevice == VK_NULL_HANDLE){
        throw std::runtime_error("no suitable GPU found");
    }

    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(bestDevice, &deviceProperties);
    std::cout << "picked GPU " << deviceProperties.deviceName << std::endl;

    ctx.Pdevice = bestDevice;
}

void VulkanDevice::createLogicalDevice(){
    ctx.queueFamilies = findQueueFamilies(ctx.Pdevice, ctx.surface);

    std::set<uint32_t> uniqueQueues = {
        ctx.queueFamilies.graphicsFamily.value(),
        ctx.queueFamilies.presentFamily.value()
    };

    float queuePriority = 1.0f;

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

    // create all the info needed for each queue
    for(const auto& queue : uniqueQueues){
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queue;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;

        queueCreateInfos.push_back(queueCreateInfo);
    }

    // no features for now
    VkPhysicalDeviceFeatures deviceFeatures{};

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());

    createInfo.pEnabledFeatures = &deviceFeatures;

    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    createInfo.enabledLayerCount = 0;

    if(vkCreateDevice(ctx.Pdevice, &createInfo, nullptr, &ctx.Ldevice) != VK_SUCCESS){
        throw std::runtime_error("failed to create the logical device");
    }

    vkGetDeviceQueue(ctx.Ldevice, ctx.queueFamilies.graphicsFamily.value(), 0, &ctx.graphicQueue);
    vkGetDeviceQueue(ctx.Ldevice, ctx.queueFamilies.presentFamily.value(), 0, &ctx.presentQueue);
}
