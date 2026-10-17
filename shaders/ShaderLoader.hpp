#pragma once

#include <vector>
#include <string>
#include <vulkan/vulkan_core.h>

#include "../scripts/headers/VulkanContext.hpp"

class ShaderLoader{
    public:
        ShaderLoader(VulkanContext& context);
        ~ShaderLoader();
        std::vector<char> readFile(const std::string& filename);

        VkShaderModule createShaderModule(const std::vector<char>& code);

        VkPipelineShaderStageCreateInfo createShaderStage(VkShaderModule module, VkShaderStageFlagBits stage);

    private:
        VulkanContext& ctx;
        std::vector<VkShaderModule> loadedModules;
        std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
};
