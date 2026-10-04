#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <optional>
#include <vector>
#include <string>
#include <vulkan/vulkan_core.h>

struct VulkanObjs{
    VkInstance instance;
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
    std::vector<VkImageView> imageViews;
    VkExtent2D swapChainExtent;
    VkFormat swapChainImageFormat;
};


class Vulkan{
    public:
        Vulkan();
        ~Vulkan();
    private:
        // vars
        VulkanObjs vkObjs;
        GLFWwindow* window;

        VkQueue graphicsQueue;
        VkQueue presentQueue;

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
        bool isDeviceSuitable(VkPhysicalDevice pDeviceToCheck);
        QueueFamilyIndicies findQueueFam(VkPhysicalDevice pDevice);
        GPU_SCORE rateDeviceSuitability(VkPhysicalDevice pDevice);
};
