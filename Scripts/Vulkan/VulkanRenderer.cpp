#include <stdexcept>
#include <vulkan/vulkan.h>

#include "../Headers/Vulkan/VulkanRenderer.hpp"

VulkanRenderer::VulkanRenderer(VulkanContext& context) : ctx(context){
    createCommandPool();
    createCommandBuffer();
    createSyncObjects();
}

VulkanRenderer::~VulkanRenderer(){
    vkDestroySemaphore(ctx.Ldevice, ctx.imageAvailableSemaphore, nullptr);
    vkDestroySemaphore(ctx.Ldevice, ctx.renderFinishedSemaphore, nullptr);
    vkDestroyFence(ctx.Ldevice, ctx.inFlightFence, nullptr);

    vkDestroyCommandPool(ctx.Ldevice, ctx.commandPool, nullptr);
}

void VulkanRenderer::createCommandPool(){
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = ctx.queueFamilies.graphicsFamily.value();

    if(vkCreateCommandPool(ctx.Ldevice, &poolInfo, nullptr, &ctx.commandPool) != VK_SUCCESS){
        throw std::runtime_error("failed to create command pool");
    }
}

void VulkanRenderer::createCommandBuffer(){
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = ctx.commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    if(vkAllocateCommandBuffers(ctx.Ldevice, &allocInfo, &ctx.commandBuffer) != VK_SUCCESS){
        throw std::runtime_error("failed to allocate command buffers");
    }
}

void VulkanRenderer::createSyncObjects(){
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    // start signaled to run the first frame
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    if(vkCreateSemaphore(ctx.Ldevice, &semaphoreInfo, nullptr, &ctx.imageAvailableSemaphore) != VK_SUCCESS ||
        vkCreateSemaphore(ctx.Ldevice, &semaphoreInfo, nullptr, &ctx.renderFinishedSemaphore) != VK_SUCCESS){
        throw std::runtime_error("failed to create semaphores!");
    }

    if(vkCreateFence(ctx.Ldevice, &fenceInfo, nullptr, &ctx.inFlightFence) != VK_SUCCESS){
        throw std::runtime_error("failed to create fence!");
    }
}

void VulkanRenderer::recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex){
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if(vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS){
        throw std::runtime_error("failed to begin recording command buffer");
    }

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = ctx.renderPass;
    renderPassInfo.framebuffer = ctx.swapChainFrameBuffers[imageIndex];

    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = ctx.swapChainInfo.swapChainExtent;

    VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, ctx.graphicsPipeline);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(ctx.swapChainInfo.swapChainExtent.width);
    viewport.height = static_cast<float>(ctx.swapChainInfo.swapChainExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = ctx.swapChainInfo.swapChainExtent;

    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    vkCmdDraw(commandBuffer, 6, 1, 0, 0);

    vkCmdEndRenderPass(commandBuffer);

    if(vkEndCommandBuffer(commandBuffer) != VK_SUCCESS){
        throw std::runtime_error("failed to record command buffer!");
    }
}

void VulkanRenderer::drawFrame(){
    vkWaitForFences(ctx.Ldevice, 1, &ctx.inFlightFence, VK_TRUE, UINT64_MAX);
    vkResetFences(ctx.Ldevice, 1, &ctx.inFlightFence);

    uint32_t imageIndex = 0;
    vkAcquireNextImageKHR(ctx.Ldevice, ctx.swapChain, UINT64_MAX, ctx.imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

    // no flags
    vkResetCommandBuffer(ctx.commandBuffer, 0);

    recordCommandBuffer(ctx.commandBuffer, imageIndex);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {ctx.imageAvailableSemaphore};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &ctx.commandBuffer;

    VkSemaphore signalSemaphores[] = {ctx.renderFinishedSemaphore};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if(vkQueueSubmit(ctx.graphicQueue, 1, &submitInfo, ctx.inFlightFence) != VK_SUCCESS){
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {ctx.swapChain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;

    vkQueuePresentKHR(ctx.presentQueue, &presentInfo);
}
