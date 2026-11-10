#include <stdexcept>
#include <vector>
#include <vulkan/vulkan.h>

#include "../Headers/Vulkan/VulkanPipeline.hpp"
#include "../Headers/Shaders/ShaderLoader.hpp"

VulkanPipeline::VulkanPipeline(VulkanContext& context) : ctx(context){
    createGraphicsPipeline();
}

VulkanPipeline::~VulkanPipeline(){
    vkDestroyPipeline(ctx.Ldevice, ctx.graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(ctx.Ldevice, ctx.pipelineLayout, nullptr);
}

void VulkanPipeline::createGraphicsPipeline(){
    ShaderLoader loader(ctx);

    auto vertShaderCode = loader.readFile("Scripts/Shaders/tri.vert.spv");
    auto fragShaderCode = loader.readFile("Scripts/Shaders/tri.frag.spv");

    VkShaderModule vertModule = loader.createShaderModule(vertShaderCode);
    VkShaderModule fragModule = loader.createShaderModule(fragShaderCode);

    VkPipelineShaderStageCreateInfo vertShaderStage = loader.createShaderStage(vertModule, VK_SHADER_STAGE_VERTEX_BIT);
    VkPipelineShaderStageCreateInfo fragShaderStage = loader.createShaderStage(fragModule, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStage, fragShaderStage};

    // dynamic allows for specification during draw time for an easier setup
    std::vector<VkDynamicState> dynamicStates{
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 0;
    // this data is prebaked in the shaders for now
    vertexInputInfo.pVertexBindingDescriptions = nullptr;
    vertexInputInfo.vertexAttributeDescriptionCount = 0;
    vertexInputInfo.pVertexAttributeDescriptions = nullptr;

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    // set up rasterizer
    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;

    // dont rasterize, maybe useful?
    rasterizer.rasterizerDiscardEnable = VK_FALSE;

    // to change this, we need to enable a GPU feature
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;

    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;

    rasterizer.depthBiasEnable = VK_FALSE;

    // anti aliasing, to enable we need a GPU feature, keep off for now
    VkPipelineMultisampleStateCreateInfo multiSampling{};
    multiSampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multiSampling.sampleShadingEnable = VK_FALSE;
    multiSampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    // per frame buffer
    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    // dont blend fragments
    colorBlendAttachment.blendEnable = VK_FALSE;

    // global
    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

    if(vkCreatePipelineLayout(ctx.Ldevice, &pipelineLayoutInfo, nullptr, &ctx.pipelineLayout) != VK_SUCCESS){
        throw std::runtime_error("failed to create pipeline layout");
    }

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;

    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multiSampling;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;

    pipelineInfo.layout = ctx.pipelineLayout;

    pipelineInfo.renderPass = ctx.renderPass;
    pipelineInfo.subpass = 0;

    if(vkCreateGraphicsPipelines(ctx.Ldevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &ctx.graphicsPipeline) != VK_SUCCESS){
        throw std::runtime_error("failed to create graphics pipeline!");
    }
}
