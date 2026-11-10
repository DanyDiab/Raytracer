#pragma once

#include "VulkanContext.hpp"

class VulkanPipeline{
    public:
        VulkanPipeline(VulkanContext& context);
        ~VulkanPipeline();

    private:
        VulkanContext& ctx;

        void createGraphicsPipeline();
};
