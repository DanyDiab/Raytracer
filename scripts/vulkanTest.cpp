#include <cstddef>
#include <iostream>
#include <optional>
#include <vector>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>


struct VulkanObjs{
    VkInstance instance;
    VkDevice Ldevice;
    VkQueue graphicQueue;
};

struct DeviceQueue{
    VkDevice device;
//   might need to a ptr later on if we have multiple queues
    VkQueue graphicsQueue;
};


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

VkPhysicalDevice pickPhysicalDevice(VkInstance instance){

    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    
    if(deviceCount == 0){
        std::cerr << "No GPUS THAT WORK WITH VULKAN FOUND BITCH! GET A BETTER GPU LOSER!";
        return physicalDevice;
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount,devices.data());

    GPU_SCORE bestScore;
    for(const auto& device : devices){
        if(!isDeviceSuitable(device)) break;

        GPU_SCORE deviceScore = rateDeviceSuitability(device);

        if(deviceScore.score > bestScore.score){
            bestScore = deviceScore;
            physicalDevice = device;
        }
    }

    std::cout << "picked GPU " << bestScore.name;

    if(physicalDevice == VK_NULL_HANDLE){
        std::cerr << "NO SUITABLE GPU | WORKS WITH VULKAN, but daddy wants M O R E requirements";
    }
    return physicalDevice;
}


DeviceQueue createLogicalDevice(VkPhysicalDevice pickedDevice){

    QueueFamilyIndicies famIndices = findQueueFam(pickedDevice);

    float queuePriority = 1.0f;

    VkDeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = famIndices.graphicsFamily.value();
    queueCreateInfo.queueCount = 1;

    queueCreateInfo.pQueuePriorities = &queuePriority;

    // no features for now
    VkPhysicalDeviceFeatures deviceFeatures{};


    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    createInfo.pQueueCreateInfos = &queueCreateInfo;
    createInfo.queueCreateInfoCount = 1;
    createInfo.pEnabledFeatures = &deviceFeatures;

    createInfo.enabledExtensionCount = 0;
    createInfo.enabledLayerCount = 0;

    VkDevice logicalDevice;
    VkResult res = vkCreateDevice(pickedDevice, &createInfo, nullptr, &logicalDevice);

    if(res != VK_SUCCESS){
        std::cerr << "something went wrong while creating the logical Device " << res;
    }

    DeviceQueue dQueue{};
    
    VkQueue graphicQueue;
    vkGetDeviceQueue(logicalDevice, famIndices.graphicsFamily.value(),0, &graphicQueue);

    dQueue.graphicsQueue = graphicQueue;
    dQueue.device = logicalDevice;

    return dQueue;
}


VkInstance createInstance(){
    VkInstanceCreateInfo createInfo{};

    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    
    VkInstance instance = VK_NULL_HANDLE; 

    VkResult res = vkCreateInstance(&createInfo, nullptr, &instance);

    if (res != VK_SUCCESS) {
        std::cerr << "Failed to create Vulkan instance! Error code: " << res << "\n";
    }

    return instance;
}

VulkanObjs initVulkan(){
    VkInstance instance = createInstance();
    VkPhysicalDevice Pdevice = pickPhysicalDevice(instance);

    DeviceQueue dq = createLogicalDevice(Pdevice);

    VulkanObjs objs{};

    objs.Ldevice = dq.device;
    objs.graphicQueue = dq.graphicsQueue;
    objs.instance = instance;

}


void cleanUp(VulkanObjs vkObjs){

    vkDestroyDevice(vkObjs.Ldevice, nullptr);
    vkDestroyInstance(vkObjs.instance, nullptr);
}

int main(){

    VulkanObjs vkObjs = initVulkan();
    cleanUp(vkObjs);

    return 0;

}