


#include <cstdint>
#include <fstream>
#include <stdexcept>
#include <vector>
#include <string>
#include <vulkan/vulkan_core.h>

#include "ShaderLoader.hpp"

ShaderLoader::ShaderLoader(VulkanContext& context) : ctx(context){}

std::vector<char> ShaderLoader::readFile(const std::string& filename){
    // start at ending and is binary
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if(!file.is_open()){
        throw std::runtime_error("failed to open file" + filename);
    }

    // read cursor position (at the end)
    size_t fileSize = (size_t) file.tellg();

    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();

    return buffer;
}

VkShaderModule ShaderLoader::createShaderModule(const std::vector<char>& code){

    VkShaderModuleCreateInfo createInfo{};

    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;

    createInfo.codeSize = code.size();

    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shaderModule;

    if(vkCreateShaderModule(ctx.Ldevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS){
        throw std::runtime_error("failed to create shader module");
    }

    loadedModules.push_back(shaderModule);

    return shaderModule;
}

VkPipelineShaderStageCreateInfo ShaderLoader::createShaderStage(VkShaderModule module, VkShaderStageFlagBits stage){
    VkPipelineShaderStageCreateInfo shaderStageInfo{};

    shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStageInfo.stage = stage;
    shaderStageInfo.module = module;
    shaderStageInfo.pName = "main";

    shaderStages.push_back(shaderStageInfo);

    return shaderStageInfo;
}


ShaderLoader::~ShaderLoader(){
    for(auto module : loadedModules){
        vkDestroyShaderModule(ctx.Ldevice, module, nullptr);
    }
}
