#pragma once

#include <vector>

#include "VulkanContext.hpp"

class VulkanInstance{
    public:
        VulkanInstance(VulkanContext& context);
        ~VulkanInstance();

    private:
        VulkanContext& ctx;

        const std::vector<const char*> validationLayers = {
            "VK_LAYER_KHRONOS_validation"
        };

        void createInstance();
        void setupDebugMessenger();
        bool checkValidationLayerSupport() const;
        std::vector<const char*> getRequiredExtensions() const;
};
