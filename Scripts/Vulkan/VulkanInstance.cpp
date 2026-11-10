#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#include "../Headers/Vulkan/VulkanInstance.hpp"

#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif

namespace {

VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
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

VkResult createDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger){

    auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
    if(func == nullptr){
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
    return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
}

void destroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator){
    auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));
    if(func != nullptr){
        func(instance, debugMessenger, pAllocator);
    }
}

}

VulkanInstance::VulkanInstance(VulkanContext& context) : ctx(context){
    createInstance();
    setupDebugMessenger();
}

VulkanInstance::~VulkanInstance(){
    if(ctx.debugMessenger != VK_NULL_HANDLE){
        destroyDebugUtilsMessengerEXT(ctx.instance, ctx.debugMessenger, nullptr);
    }
    vkDestroyInstance(ctx.instance, nullptr);
}

void VulkanInstance::createInstance(){
    if(enableValidationLayers && !checkValidationLayerSupport()){
        throw std::runtime_error("validation layers requested, but not available");
    }

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

    std::vector<const char*> extensions = getRequiredExtensions();

    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    if(enableValidationLayers){
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    }
    else{
        createInfo.enabledLayerCount = 0;
    }

    VkResult res = vkCreateInstance(&createInfo, nullptr, &ctx.instance);

    if(res != VK_SUCCESS){
        throw std::runtime_error("Failed to create Vulkan instance! Error code: " + std::to_string(res));
    }
}

std::vector<const char*> VulkanInstance::getRequiredExtensions() const{
    uint32_t glfwExtensionsCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionsCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionsCount);

    if(enableValidationLayers){
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}

bool VulkanInstance::checkValidationLayerSupport() const{
    uint32_t layerCount = 0;
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

void VulkanInstance::setupDebugMessenger(){
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

    if(createDebugUtilsMessengerEXT(ctx.instance, &createInfo, nullptr, &ctx.debugMessenger) != VK_SUCCESS){
        throw std::runtime_error("failed to setup debug messenger");
    }
}
