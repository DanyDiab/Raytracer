#pragma once

#include <vector>

#include "VulkanContext.hpp"

class VulkanSwapChain{
    public:
        VulkanSwapChain(VulkanContext& context);
        ~VulkanSwapChain();

    private:
        VulkanContext& ctx;

        void createSwapChain();
        void createImageViews();
        void createRenderPass();
        void createFrameBuffers();

        VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) const;
        VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availableModes) const;
        VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) const;
};
