#include <GLFW/glfw3.h>
#include <vulkan/vk_platform.h>
#include <vulkan/vulkan.h>
#include <optional>
#include <vector>
#include <string>
#include <vulkan/vulkan_core.h>

struct VulkanObjs{
    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkDevice Ldevice;
    VkPhysicalDevice Pdevice;
    VkQueue graphicQueue;
    VkSurfaceKHR surface;
    VkSwapchainKHR swapChain;
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

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

struct SwapChainInfo{
    std::vector<VkImage> images;
    std::vector<VkImageView> Views;
    VkExtent2D swapChainExtent;
    VkFormat swapChainImageFormat;
};

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);

class Vulkan{
    public:
        GLFWwindow* window;

        Vulkan();
        ~Vulkan();
    private:
        // vars
        VulkanObjs vkObjs;

        VkQueue graphicsQueue;
        VkQueue presentQueue;

        const std::vector<const char*> deviceExtensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };

        const std::vector<const char*> validationLayers = {
            "VK_LAYER_KHRONOS_validation"
        };

        SwapChainInfo scInfo;
        // MAIN FUNCTIONS
        void createWindow();
        void createInstance();
        void createSurface();
        void pickPhysicalDevice();
        void createLogicalDevice();
        void createSwapChain();

        // HELPER FUNCTIONS

        SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice pDeviceToCheck);
        VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
        VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availableModes);

        VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window);

        bool checkDeviceExtensionSupport(VkPhysicalDevice pDeviceToCheck);
        bool checkValidationLayerSupport();
        std::vector<const char*> getRequiredExtenstions();

        bool isDeviceSuitable(VkPhysicalDevice pDeviceToCheck);
        QueueFamilyIndicies findQueueFam(VkPhysicalDevice pDevice);
        GPU_SCORE rateDeviceSuitability(VkPhysicalDevice pDevice);

        void setupDebugMessenger();

        // STATIC FUNCTIONS

        static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT messageType,
            const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
            void* pUserData
        );



};
