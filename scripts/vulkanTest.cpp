#include <cstddef>
#include <cstdint>
#include <iostream>
#include <optional>
#include <set>
#include <vector>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

// eventually make this a class?

struct VulkanObjs{
    VkInstance instance;
    VkDevice Ldevice;
    VkQueue graphicQueue;
    VkSurfaceKHR surface;
};

struct DeviceQueue{
    VkDevice device;
//   might need to a ptr later on if we have multiple queues
    VkQueue graphicsQueue;
    VkQueue presentQueue;
};


struct GPU_SCORE{
    int score;
    std::string name;
};

struct QueueFamilyIndicies {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool hasAllQueues(){
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};


GLFWwindow* createWindow(){
    GLFWwindow *window;
    if (!glfwInit()) return window;
    window = glfwCreateWindow(960, 540, "RayTracer", NULL, NULL);

    if (!window){
        glfwTerminate();
        return nullptr;
    }
    
    glfwMakeContextCurrent(window);

    return window;
}

VkSurfaceKHR createSurface(VkInstance instance){
    GLFWwindow* window = createWindow();

    VkSurfaceKHR surface{};
    
    int supportRes = glfwVulkanSupported();
    if(supportRes == GLFW_FALSE){
        std::cerr << "glfw vulkan not supported :(";
    }

    uint32_t count;
    const char** requiredRes = glfwGetRequiredInstanceExtensions(&count);
    if(requiredRes == nullptr){
        std::cerr << "the required res are null\n";
    }

    if(count == 0){
        std::cerr << "the count of required instance extensions is 0? why?\n";
    }
    VkResult createRes = glfwCreateWindowSurface(instance, window, nullptr, &surface);

    if(createRes != VK_SUCCESS){
        std::cerr << "somthing went weong with window surface creation using GLFW " << supportRes; 
    }

    return surface;
}


QueueFamilyIndicies findQueueFam(VkPhysicalDevice device, VkSurfaceKHR surface){
    QueueFamilyIndicies QueueFamilyIndicies;

    uint32_t count = 0;

    vkGetPhysicalDeviceQueueFamilyProperties(device, &count, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(count);

    vkGetPhysicalDeviceQueueFamilyProperties(device, &count, queueFamilies.data());

    int i = 0;
    for(const auto& properties : queueFamilies){

        uint32_t queueFlags = properties.queueFlags;
        
        // can we do graphics? aka buffers etc...
        if(queueFlags & VK_QUEUE_GRAPHICS_BIT){
            QueueFamilyIndicies.graphicsFamily = i;
        }

        // check for presentation support to the window surface
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

        if(presentSupport){
            QueueFamilyIndicies.presentFamily = i;
        }
        
        i++;
    }

    return QueueFamilyIndicies;
}

bool isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface){
    QueueFamilyIndicies famIndices = findQueueFam(device, surface);
    
    return  famIndices.hasAllQueues();
}


GPU_SCORE rateDeviceSuitability(VkPhysicalDevice device, VkSurfaceKHR surface){


    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;

    vkGetPhysicalDeviceProperties(device, &deviceProperties);
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

    GPU_SCORE score;
    score.score = 0;


    if(deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU){
        score.score += 1000;
    }

    // maybe this can be cached somewhere?? we call this many times
    QueueFamilyIndicies famIndices = findQueueFam(device, surface);

    // if we get to here, we have all queues
    // prefer if the graphics family and the present family are the same queue (more performance)
    if(famIndices.graphicsFamily.value() == famIndices.presentFamily.value()){
        score.score += 500;
    }
    
    score.name = deviceProperties.deviceName;

    return score;
}

VkPhysicalDevice pickPhysicalDevice(VkInstance instance, VkSurfaceKHR surface){

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
        if(!isDeviceSuitable(device, surface)) break;

        GPU_SCORE deviceScore = rateDeviceSuitability(device, surface);

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


DeviceQueue createLogicalDevice(VkPhysicalDevice pickedDevice, VkSurfaceKHR surface){

    QueueFamilyIndicies famIndices = findQueueFam(pickedDevice, surface);

    std::set<uint32_t>uniqueQueues = {famIndices.graphicsFamily.value(), famIndices.presentFamily.value()};

    float queuePriority = 1.0f;

    std::vector<VkDeviceQueueCreateInfo> queueCreateinfos;

    // create all the info needed for each queue
    for(const auto& queue : uniqueQueues){
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = famIndices.graphicsFamily.value();
        queueCreateInfo.queueCount = 1;

        queueCreateInfo.pQueuePriorities = &queuePriority;

        
        queueCreateinfos.push_back(queueCreateInfo);
    }

    // no features for now
    VkPhysicalDeviceFeatures deviceFeatures{};

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    createInfo.pQueueCreateInfos = queueCreateinfos.data();
    createInfo.queueCreateInfoCount = queueCreateinfos.size();

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
    
    VkQueue presentQueue;
    vkGetDeviceQueue(logicalDevice, famIndices.presentFamily.value(),0, &presentQueue);\
    dQueue.presentQueue = presentQueue;

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
    VkSurfaceKHR surface = createSurface(instance);

    VkPhysicalDevice Pdevice = pickPhysicalDevice(instance, surface);

    DeviceQueue dq = createLogicalDevice(Pdevice, surface);

    VulkanObjs objs{};

    objs.surface = surface;
    objs.Ldevice = dq.device;
    objs.graphicQueue = dq.graphicsQueue;
    objs.instance = instance;

}


void cleanUp(VulkanObjs vkObjs){

    vkDestroyDevice(vkObjs.Ldevice, nullptr);
    vkDestroySurfaceKHR(vkObjs.instance, vkObjs.surface, nullptr);
    vkDestroyInstance(vkObjs.instance, nullptr);
}

int main(){

    VulkanObjs vkObjs = initVulkan();
    cleanUp(vkObjs);

    return 0;

}