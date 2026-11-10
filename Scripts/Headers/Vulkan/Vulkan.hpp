#pragma once

#include "VulkanContext.hpp"
#include "VulkanWindow.hpp"
#include "VulkanInstance.hpp"
#include "VulkanSurface.hpp"
#include "VulkanDevice.hpp"
#include "VulkanSwapChain.hpp"
#include "VulkanPipeline.hpp"
#include "VulkanRenderer.hpp"

class Vulkan{
    public:
        Vulkan();
        ~Vulkan();

        void run();

    private:
        // declaration order is creation order; destruction runs in reverse
        VulkanContext ctx;

        VulkanWindow window;
        VulkanInstance instance;
        VulkanSurface surface;
        VulkanDevice device;
        VulkanSwapChain swapChain;
        VulkanPipeline pipeline;
        VulkanRenderer renderer;
};
