#pragma once

#include "VulkanContext.hpp"

class VulkanWindow{
    public:
        VulkanWindow(VulkanContext& context);
        ~VulkanWindow();

    private:
        VulkanContext& ctx;
};
