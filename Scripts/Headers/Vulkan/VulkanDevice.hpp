#pragma once

#include "VulkanContext.hpp"

class VulkanDevice{
    public:
        VulkanDevice(VulkanContext& context);
        ~VulkanDevice();

    private:
        VulkanContext& ctx;

        void pickPhysicalDevice();
        void createLogicalDevice();
        bool isDeviceSuitable(VkPhysicalDevice device) const;
        int rateDeviceSuitability(VkPhysicalDevice device) const;
};
