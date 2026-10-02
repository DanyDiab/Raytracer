#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <limits>
#include <set>
#include <string>
#include <sys/types.h>
#include <vector>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <iostream>
#include <fmt/format.h>

#include "headers/Vulkan.hpp"


// eventually make this a class?

const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};


void Vulkan::createWindow(){
    GLFWwindow *window;
    if (!glfwInit()){
        std::string err =  "ERROR: glfw init failed\n";
        throw err;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    window = glfwCreateWindow(960, 540, "RayTracer", NULL, NULL);

    if (!window){
        glfwTerminate();
        std::string err = "ERROR: GLFW createWindow Failed";
        throw err;
    }

    glfwMakeContextCurrent(window);

    this->window = window;
}

void Vulkan::createSurface(){

    VkSurfaceKHR surface{};

    int supportRes = glfwVulkanSupported();
    if(supportRes == GLFW_FALSE){
        std::string err = "ERROR: glfw vulkan not supported :(\n";
        throw err;
    }

    uint32_t count;
    const char** requiredRes = glfwGetRequiredInstanceExtensions(&count);

    if(requiredRes == nullptr){
        std::string err = "ERROR: the required instance extensions are null\n";
        throw err;
    }

    if(count == 0){
        std::string err = "ERROR: the count of required instance extensions is 0? why?\n";
        throw err;
    }

    VkResult createRes = glfwCreateWindowSurface(vkObjs.instance, window, nullptr, &surface);

    if(createRes != VK_SUCCESS){
        std::cerr << "somthing went weong with window surface creation using GLFW " << "\n";
        printf("%d\n", createRes);
    }
}


QueueFamilyIndicies Vulkan::findQueueFam(VkPhysicalDevice pDevice){
    QueueFamilyIndicies QueueFamilyIndicies;

    uint32_t count = 0;

    vkGetPhysicalDeviceQueueFamilyProperties(pDevice, &count, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(count);

    vkGetPhysicalDeviceQueueFamilyProperties(pDevice, &count, queueFamilies.data());

    int i = 0;
    for(const auto& properties : queueFamilies){

        uint32_t queueFlags = properties.queueFlags;

        // can we do graphics? aka buffers etc...
        if(queueFlags & VK_QUEUE_GRAPHICS_BIT){
            QueueFamilyIndicies.graphicsFamily = i;
        }

        // check for presentation support to the window surface
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(pDevice, i, vkObjs.surface, &presentSupport);

        if(presentSupport){
            QueueFamilyIndicies.presentFamily = i;
        }

        i++;
    }

    return QueueFamilyIndicies;
}

bool Vulkan::checkDeviceExtensionSupport(VkPhysicalDevice pDeviceToCheck){
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(pDeviceToCheck, nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> allExtensions = std::vector<VkExtensionProperties>(extensionCount);

   vkEnumerateDeviceExtensionProperties(pDeviceToCheck, nullptr, &extensionCount, allExtensions.data());

   std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

   for(const auto& extension : allExtensions){
       requiredExtensions.erase(extension.extensionName);
   }

   // if set empty, we have all extensions
   return requiredExtensions.empty();
}
SwapChainSupportDetails Vulkan::querySwapChainSupport(){
    SwapChainSupportDetails details;

    VkPhysicalDevice pDevice = vkObjs.Pdevice;
    VkSurfaceKHR surface = vkObjs.surface;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(pDevice, surface, &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(pDevice, surface, &formatCount,nullptr);

    if(formatCount != 0){
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(pDevice, surface ,&formatCount, details.formats.data());
    }

    uint32_t modeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(pDevice, surface,&modeCount, nullptr);

    if(modeCount != 0){
        details.presentModes.resize(modeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(pDevice, surface,&modeCount, details.presentModes.data());
    }

    return details;
}

VkSurfaceFormatKHR  Vulkan::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats){
    for(const auto& format : availableFormats){
        VkFormat colorLayout = format.format;
        VkColorSpaceKHR colorSpace = format.colorSpace;

        if(colorLayout == VK_FORMAT_R8G8B8A8_SRGB && colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR){
            return format;
        }
    }
    // the format we wanted was not specified, and so we can just take the first one and pray its good enough lol
    return  availableFormats.at(0);
}

VkPresentModeKHR  Vulkan::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availableModes){
    for(const auto& mode : availableModes){
        // if triple buffering is available
        if(mode == VK_PRESENT_MODE_MAILBOX_KHR){
            return mode;
        }
    }
    // fallback; all GPUs support double buffering
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D Vulkan::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window){
    // resolution and width/height match
    if(capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()){
        return capabilities.currentExtent;
    }
    // dont match, pick best match

    int width, height;

    glfwGetFramebufferSize(window, &width, &height);

    VkExtent2D actualExtent = {
        static_cast<uint32_t>(width),
        static_cast<uint32_t>(height)
    };

    //clamp between min anbd max extents
    actualExtent.width = std::clamp(
        actualExtent.width,
        capabilities.minImageExtent.width,
        capabilities.maxImageExtent.width
    );

    actualExtent.height = std::clamp(
        actualExtent.height,
        capabilities.minImageExtent.height,
        capabilities.maxImageExtent.height
    );

    return actualExtent;
}

void Vulkan::createSwapChain(){
    SwapChainSupportDetails scDetails = querySwapChainSupport();
    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(scDetails.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(scDetails.presentModes);
    VkExtent2D extent = chooseSwapExtent(scDetails.capabilities, window);

    // pick hopw many images in the swap chain
    uint32_t imageCount = scDetails.capabilities.minImageCount + 1;

    // if we exceeded max
    // NOTE maxImageCount == 0 means no max
    if(scDetails.capabilities.maxImageCount > 0 && imageCount > scDetails.capabilities.maxImageCount){
        imageCount = scDetails.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = vkObjs.surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    // we are rendering already, so this likely needs to be swapped to VK_IMAGE_USAGE_TRANSFER_DST_BIT
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndicies indices = findQueueFam(vkObjs.Pdevice);

    uint32_t indiciesArr[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    if(indices.graphicsFamily != indices.presentFamily){
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = indiciesArr;
    }
    else{
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        // these are optional
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
    }
    // apply a transform to all images in swap chain
    createInfo.preTransform = scDetails.capabilities.currentTransform;
    //ignore alpha channel for now
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;

    // this is used if we create a new swap chain, aka if the old swap chain is invalid, maybe due to window resizing
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    VkSwapchainKHR swapChain;
    VkResult createRes = vkCreateSwapchainKHR(vkObjs.Ldevice, &createInfo, nullptr, &swapChain);
    if(createRes != VK_SUCCESS){
        std::cerr << "something went wrong while creating swap chain " << createRes;
    }

    // grab images
    vkGetSwapchainImagesKHR(vkObjs.Ldevice, swapChain, &imageCount, nullptr);
    scInfo.images.resize(imageCount);
    vkGetSwapchainImagesKHR(vkObjs.Ldevice, swapChain, &imageCount, scInfo.images.data());

    scInfo.swapChainExtent = extent;
    scInfo.swapChainImageFormat = surfaceFormat.format;
}


bool Vulkan::isDeviceSuitable(VkPhysicalDevice pDeviceToCheck){
    QueueFamilyIndicies famIndices = findQueueFam(pDeviceToCheck);

    bool ExtensionsSupported = checkDeviceExtensionSupport(pDeviceToCheck);

    bool swapChainAdequate = false;

    if(ExtensionsSupported){
        SwapChainSupportDetails scDetails = querySwapChainSupport();
        swapChainAdequate = !scDetails.formats.empty() && !scDetails.presentModes.empty();
    }

    return  famIndices.hasAllQueues() && swapChainAdequate && ExtensionsSupported;
}


GPU_SCORE Vulkan::rateDeviceSuitability(VkPhysicalDevice pDevice){
    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;

    vkGetPhysicalDeviceProperties(pDevice, &deviceProperties);
    vkGetPhysicalDeviceFeatures(pDevice, &deviceFeatures);

    GPU_SCORE score;
    score.score = 0;

    if(deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU){
        score.score += 1000;
    }

    // maybe this can be cached somewhere?? we call this many times
    QueueFamilyIndicies famIndices = findQueueFam(pDevice);

    // if we get to here, we have all queues
    // prefer if the graphics family and the present family are the same queue (more performance)
    if(famIndices.graphicsFamily.value() == famIndices.presentFamily.value()){
        score.score += 500;
    }

    score.name = deviceProperties.deviceName;

    return score;
}

void Vulkan::pickPhysicalDevice(){

    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(vkObjs.instance, &deviceCount, nullptr);

    if(deviceCount == 0){
        std::string err = "No GPUS THAT WORK WITH VULKAN FOUND BITCH! GET A BETTER GPU LOSER!";
        throw err;
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(vkObjs.instance, &deviceCount,devices.data());

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

}



void Vulkan::createLogicalDevice(){

    QueueFamilyIndicies famIndices = findQueueFam(vkObjs.Pdevice);

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

    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    createInfo.enabledLayerCount = 0;

    VkDevice logicalDevice;
    VkResult res = vkCreateDevice(vkObjs.Pdevice, &createInfo, nullptr, &logicalDevice);

    if(res != VK_SUCCESS){
        std::cerr << "something went wrong while creating the logical Device " << res;
    }

    VkQueue graphicQueue;
    vkGetDeviceQueue(logicalDevice, famIndices.graphicsFamily.value(),0, &graphicQueue);
    this->graphicsQueue = graphicQueue;

    VkQueue presentQueue;
    vkGetDeviceQueue(logicalDevice, famIndices.presentFamily.value(),0, &presentQueue);
    this->presentQueue = presentQueue;

    vkObjs.Ldevice = logicalDevice;
}

void createImageViews(){

}

void Vulkan::createInstance(){
    VkInstanceCreateInfo createInfo{};

    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

    uint32_t count;
    const char** requiredRes = glfwGetRequiredInstanceExtensions(&count);

    createInfo.enabledExtensionCount = count;
    createInfo.ppEnabledExtensionNames = requiredRes;

    VkInstance instance = VK_NULL_HANDLE;

    VkResult res = vkCreateInstance(&createInfo, nullptr, &instance);

    if (res != VK_SUCCESS) {
        std::string err = "Failed to create Vulkan instance! Error code:" + std::to_string(res) + "\n";
        throw err;
    }

    vkObjs.instance = instance;
}


Vulkan::Vulkan(){
    createWindow();
    createInstance();
    createSurface();


    pickPhysicalDevice();
    createLogicalDevice();

    createSwapChain();
}

Vulkan::~Vulkan(){
    vkDestroySwapchainKHR(vkObjs.Ldevice, vkObjs.swapChain, nullptr);
    vkDestroyDevice(vkObjs.Ldevice, nullptr);
    vkDestroySurfaceKHR(vkObjs.instance, vkObjs.surface, nullptr);
    vkDestroyInstance(vkObjs.instance, nullptr);
}

int main(){
    Vulkan vk;
    return 0;

}
