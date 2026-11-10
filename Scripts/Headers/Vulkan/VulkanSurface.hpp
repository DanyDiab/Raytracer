#pragma once

#include "VulkanContext.hpp"

class VulkanSurface{
    public:
        VulkanSurface(VulkanContext& context);
        ~VulkanSurface();

    private:
        VulkanContext& ctx;
};
