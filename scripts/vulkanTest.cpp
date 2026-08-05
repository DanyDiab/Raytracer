#include <cstddef>
#include <iostream>
#include <optional>
#include <vector>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>


VkInstance instance;

struct GPU_SCORE{
    int score;
    std::string name;
};

struct QueueFamilyIndicies {
    std::optional<uint32_t> graphicsFamily;
};


QueueFamilyIndicies findQueueFam(VkPhysicalDevice device){
    QueueFamilyIndicies QueueFamilyIndicies;

    uint32_t count = 0;

    vkGetPhysicalDeviceQueueFamilyProperties(device, &count, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(count);

    vkGetPhysicalDeviceQueueFamilyProperties(device, &count, queueFamilies.data());

    int i = 0;
    for(const auto& properties : queueFamilies){
        if(properties.queueFlags & VK_QUEUE_GRAPHICS_BIT){
            QueueFamilyIndicies.graphicsFamily = i;
        }
        i++;
    }

    return QueueFamilyIndicies;
}

bool isDeviceSuitable(VkPhysicalDevice device){
    QueueFamilyIndicies famIndices = findQueueFam(device);
    
    return  famIndices.graphicsFamily.has_value();
}


GPU_SCORE rateDeviceSuitability(VkPhysicalDevice device){
    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;

    vkGetPhysicalDeviceProperties(device, &deviceProperties);
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

    GPU_SCORE score;
    score.score = 0;

    if(deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU){
        score.score += 1000;
    }
    
    score.name = deviceProperties.deviceName;

    return score;
}

int pickPhysicalDevice(){

    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    
    if(deviceCount == 0){
        std::cerr << "No GPUS THAT WORK WITH VULKAN FOUND BITCH! GET A BETTER GPU LOSER!";
        return -1;
    }

    VkPhysicalDevice pickedDevice;

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount,devices.data());

    GPU_SCORE bestScore;
    for(const auto& device : devices){
        if(!isDeviceSuitable(device)) break;

        GPU_SCORE deviceScore = rateDeviceSuitability(device);

        if(deviceScore.score > bestScore.score){
            bestScore = deviceScore;
            pickedDevice = device;
        }
    }

    std::cout << "picked GPU " << bestScore.name;

    if(pickedDevice == VK_NULL_HANDLE){
        std::cerr << "NO SUITABLE GPU | WORKS WITH VULKAN, but daddy wants M O R E requirements";
        return -1;
    }
    return 0;
}


int createInstance(){
    VkInstanceCreateInfo createInfo{};

    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    
    instance = VK_NULL_HANDLE; 

    VkResult res = vkCreateInstance(&createInfo, nullptr, &instance);

    if (res != VK_SUCCESS) {
        std::cerr << "Failed to create Vulkan instance! Error code: " << res << "\n";
        return 1;
    }
    return 0;
}

int initVulkan(){
    int createStatus = createInstance();
    int pickStatus = pickPhysicalDevice();

    return createStatus || pickStatus;
}


void cleanUp(){
    vkDestroyInstance(instance, nullptr);
}




int main(){

    initVulkan();
    cleanUp();

    return 0;

}