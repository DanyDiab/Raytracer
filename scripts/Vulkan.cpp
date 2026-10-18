#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <limits>
#include <set>
#include <stdexcept>
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

#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif


void Vulkan::createWindow(){
    GLFWwindow *window;
    glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11);
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

    vkObjs.surface = surface;
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

SwapChainSupportDetails Vulkan::querySwapChainSupport(VkPhysicalDevice pDeviceToCheck){
    SwapChainSupportDetails details;

    VkSurfaceKHR surface = vkObjs.surface;

    VkResult res = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(pDeviceToCheck, surface, &details.capabilities);

    if(res != VK_SUCCESS){
        std::cerr << "ERROR: SwapChain Get Surface Capabilities " << res;
    }

    uint32_t formatCount;
    VkResult res1 = vkGetPhysicalDeviceSurfaceFormatsKHR(pDeviceToCheck, surface, &formatCount,nullptr);

    if(res1 != VK_SUCCESS){
        std::cerr << "ERROR: SwapChain Get Surface Formats " << res;
    }

    if(formatCount != 0){
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(pDeviceToCheck, surface ,&formatCount, details.formats.data());
    }

    uint32_t modeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(pDeviceToCheck, surface,&modeCount, nullptr);

    if(modeCount != 0){
        details.presentModes.resize(modeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(pDeviceToCheck, surface,&modeCount, details.presentModes.data());
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
    SwapChainSupportDetails scDetails = querySwapChainSupport(vkObjs.Pdevice);
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

    vkObjs.swapChain = swapChain;
}


bool Vulkan::isDeviceSuitable(VkPhysicalDevice pDeviceToCheck){
    QueueFamilyIndicies famIndices = findQueueFam(pDeviceToCheck);

    bool ExtensionsSupported = checkDeviceExtensionSupport(pDeviceToCheck);

    bool swapChainAdequate = false;

    if(ExtensionsSupported){
        SwapChainSupportDetails scDetails = querySwapChainSupport(pDeviceToCheck);
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

        if(deviceScore.score >= bestScore.score){
            bestScore = deviceScore;
            physicalDevice = device;
        }
    }

    std::cout << "picked GPU " << bestScore.name;

    if(physicalDevice == VK_NULL_HANDLE){
        std::cerr <<  "NO SUITABLE GPU | WORKS WITH VULKAN, but daddy wants M O R E requirements";
    }

    vkObjs.Pdevice = physicalDevice;
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
        queueCreateInfo.queueFamilyIndex = queue;
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
    vkObjs.graphicQueue = graphicQueue;

    VkQueue presentQueue;
    vkGetDeviceQueue(logicalDevice, famIndices.presentFamily.value(),0, &presentQueue);
    vkObjs.presentQueue = presentQueue;

    vkObjs.Ldevice = logicalDevice;
}

void createImageViews(){

}

void Vulkan::createInstance(){

    if(enableValidationLayers && !checkValidationLayerSupport()){
        throw std::runtime_error("validation layers requested, but not available");
    }

    VkInstanceCreateInfo createInfo{};

    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

    std::vector<const char*> extensions = getRequiredExtenstions();

    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    if(enableValidationLayers){
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    }
    else{
        createInfo.enabledLayerCount = 0;
    }

    VkInstance instance = VK_NULL_HANDLE;

    VkResult res = vkCreateInstance(&createInfo, nullptr, &instance);

    if (res != VK_SUCCESS) {
        std::string err = "Failed to create Vulkan instance! Error code:" + std::to_string(res) + "\n";
        throw err;
    }

    vkObjs.instance = instance;
}

std::vector<const char*> Vulkan::getRequiredExtenstions(){
    uint32_t glfwExtensionsCount = 0;

    const char** glfwExtensions;

    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionsCount);

    std::vector<const char*>extensions(glfwExtensions, glfwExtensions + glfwExtensionsCount);

    if(enableValidationLayers){
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}

bool Vulkan::checkValidationLayerSupport(){
    uint32_t layerCount;

    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

   std::vector<VkLayerProperties> availableLayers(layerCount);

    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for(const auto& layerName : validationLayers){

        bool foundLayer = false;

        for(const auto& layerProperties : availableLayers){
            if(strcmp(layerName, layerProperties.layerName) == 0){
                foundLayer = true;
                break;
            }
        }

        if(!foundLayer){
            return false;
        }
    }

    return true;
}

VKAPI_ATTR VkBool32 VKAPI_CALL Vulkan::debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData){


   if(messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT){
       std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
   }
// always return VK_FALSE, this is used to test the validation layers themselves
   return VK_FALSE;
}

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger){

    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDEbugUTilsMessengerEXT");
    if(func!= nullptr){
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    else{
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}


void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator){
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestoryDebugUtilsMessengerEXT");

    if(func!= nullptr){
        func(instance,debugMessenger, pAllocator);
    }
    else{
        throw std::runtime_error("Failed To Locate vkDestoryDebugUtilsMessengerEXT function");
    }
}


void Vulkan::setupDebugMessenger(){
    if(!enableValidationLayers) return;

    VkDebugUtilsMessengerCreateInfoEXT createInfo{};

    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
   createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
   createInfo.pfnUserCallback = debugCallback;
   createInfo.pUserData = nullptr;

   if(CreateDebugUtilsMessengerEXT(vkObjs.instance, &createInfo, nullptr, &vkObjs.debugMessenger) != VK_SUCCESS){
       throw std::runtime_error("failed to setup debug messenger");
   }
}

void Vulkan::createImageViews(){
    scInfo.views.resize(scInfo.images.size());

    int32_t i = 0;
    for(const auto& image : scInfo.images){
        VkImageViewCreateInfo createInfo{};

        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;

        createInfo.image = image;

        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;

        createInfo.format = scInfo.swapChainImageFormat;

        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        if(vkCreateImageView(vkObjs.Ldevice, &createInfo, nullptr, &scInfo.views.at(i))!= VK_SUCCESS){
            throw std::runtime_error("failed to craete image views");;
        }
    }
}

void Vulkan::createGraphicsPipeline(){
    ShaderLoader loader(vkObjs);

    auto vertShaderCode = loader.readFile("./shaders/tri.vert.spv");
    auto fragShaderCode = loader.readFile("./shaders/tri.frag.spv");

    VkShaderModule vertModule = loader.createShaderModule(vertShaderCode);
    VkShaderModule fragModule = loader.createShaderModule(fragShaderCode);

    VkPipelineShaderStageCreateInfo vertShaderStage = loader.createShaderStage(vertModule, VK_SHADER_STAGE_VERTEX_BIT);
    VkPipelineShaderStageCreateInfo fragShaderStage = loader.createShaderStage(fragModule, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStage, fragShaderStage};

    // dyanmic allows for speciofication during draw time for an easier setup
    std::vector<VkDynamicState> dynamicStates{
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dynamicState{};

    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};

    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 0;
    // this data is prebaked in the shaders for now
    vertexInputInfo.pVertexBindingDescriptions = nullptr;
    vertexInputInfo.vertexAttributeDescriptionCount = 0;
    vertexInputInfo.pVertexAttributeDescriptions = nullptr;


    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};

    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;


    VkPipelineViewportStateCreateInfo viewportState{};

    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    // set up rasterizer
    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;

    // dont rasterize, maybe useful?
    rasterizer.rasterizerDiscardEnable = VK_FALSE;

    // to change this, we need to enable a GPU feature
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;

    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;

    rasterizer.depthBiasEnable = VK_FALSE;

    // anti aliasing, to enable we need a GPU feature, keep off for now
    VkPipelineMultisampleStateCreateInfo multiSampling{};
    multiSampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multiSampling.sampleShadingEnable = VK_FALSE;
    multiSampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

// per frame buffer
    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
// dont blend fragmnets
    colorBlendAttachment.blendEnable = VK_FALSE;

    // global
    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;

    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;



    if(vkCreatePipelineLayout(vkObjs.Ldevice, &pipelineLayoutInfo, nullptr, &vkObjs.pipelineLayout) != VK_SUCCESS){
        throw std::runtime_error("failed to crate pipeline layout");
    }

    VkGraphicsPipelineCreateInfo pipelineInfo{};

    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;

    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multiSampling;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;

    pipelineInfo.layout = vkObjs.pipelineLayout;

    pipelineInfo.renderPass = vkObjs.renderPass;
    pipelineInfo.subpass = 0;

    if(vkCreateGraphicsPipelines(vkObjs.Ldevice, VK_NULL_HANDLE, 1, &pipelineInfo,nullptr, &vkObjs.graphicsPipeline) != VK_SUCCESS){
        throw std::runtime_error("failed to create pgraphics pipeline!");
    }


}

void Vulkan::createRenderPass(){
    VkAttachmentDescription colorAttachment{};

    colorAttachment.format = scInfo.swapChainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;

    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmnetRef{};
    colorAttachmnetRef.attachment = 0;
    colorAttachmnetRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmnetRef;

    VkRenderPassCreateInfo renderPassInfo{};

    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;

    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;

    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;

    if(vkCreateRenderPass(vkObjs.Ldevice, &renderPassInfo, nullptr, &vkObjs.renderPass) != VK_SUCCESS){
        throw std::runtime_error("failed to create a render pass!");
    }
}

void Vulkan::createFrameBuffers(){
    vkObjs.swapChainFrameBuffers.resize(scInfo.views.size());

    int idx = 0;

    for(const auto view : scInfo.views){
        VkImageView attachment[] = {view};

        VkFramebufferCreateInfo frameBufferInfo{};

        frameBufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        frameBufferInfo.renderPass = vkObjs.renderPass;
        frameBufferInfo.attachmentCount = 1;
        frameBufferInfo.pAttachments = attachment;
        frameBufferInfo.width = scInfo.swapChainExtent.width;
        frameBufferInfo.height = scInfo.swapChainExtent.height;
        frameBufferInfo.layers = 1;

        if(vkCreateFramebuffer(vkObjs.Ldevice, &frameBufferInfo, nullptr, &vkObjs.swapChainFrameBuffers[idx]) != VK_SUCCESS){
            throw std::runtime_error("failed to create frame buffer!");
        }

        idx++;
    }
}

Vulkan::Vulkan(){
    createWindow();
    createInstance();
    createSurface();


    pickPhysicalDevice();
    createLogicalDevice();

    createSwapChain();
    createImageViews();

    createRenderPass();
    createGraphicsPipeline();
    createFrameBuffers();
}

Vulkan::~Vulkan(){
    if(enableValidationLayers){
        DestroyDebugUtilsMessengerEXT(vkObjs.instance, vkObjs.debugMessenger, nullptr);
    }

    for(auto frameBuffer : vkObjs.swapChainFrameBuffers){
        vkDestroyFramebuffer(vkObjs.Ldevice, frameBuffer, nullptr);
    }

    vkDestroyPipeline(vkObjs.Ldevice, vkObjs.graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(vkObjs.Ldevice, vkObjs.pipelineLayout, nullptr);
    vkDestroyRenderPass(vkObjs.Ldevice, vkObjs.renderPass, nullptr);

    for(auto imageView : scInfo.views){
        vkDestroyImageView(vkObjs.Ldevice, imageView, nullptr);
    }

    vkDestroySwapchainKHR(vkObjs.Ldevice, vkObjs.swapChain, nullptr);
    vkDestroyDevice(vkObjs.Ldevice, nullptr);
    vkDestroySurfaceKHR(vkObjs.instance, vkObjs.surface, nullptr);
    vkDestroyInstance(vkObjs.instance, nullptr);
}



int main(){
    Vulkan vk;

    while (!glfwWindowShouldClose(vk.window)) {
        glfwPollEvents();
    }
    return 0;

}
