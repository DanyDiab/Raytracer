#pragma once

#include <cstdint>
#include <vulkan/vulkan_core.h>

#include "VulkanContext.hpp"

class VulkanRenderer{
    public:
        VulkanRenderer(VulkanContext& context);
        ~VulkanRenderer();

        void drawFrame();

    private:
        VulkanContext& ctx;

        void createCommandPool();
        void createCommandBuffer();
        void createSyncObjects();
        void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
};
