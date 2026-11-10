#include <algorithm>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <vector>
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#include "../Headers/Vulkan/VulkanSwapChain.hpp"
#include "../Headers/Vulkan/VulkanQueries.hpp"

VulkanSwapChain::VulkanSwapChain(VulkanContext& context) : ctx(context){
    createSwapChain();
    createImageViews();
    createRenderPass();
    createFrameBuffers();
}

VulkanSwapChain::~VulkanSwapChain(){
    for(auto frameBuffer : ctx.swapChainFrameBuffers){
        vkDestroyFramebuffer(ctx.Ldevice, frameBuffer, nullptr);
    }

    vkDestroyRenderPass(ctx.Ldevice, ctx.renderPass, nullptr);

    for(auto imageView : ctx.swapChainInfo.views){
        vkDestroyImageView(ctx.Ldevice, imageView, nullptr);
    }

    vkDestroySwapchainKHR(ctx.Ldevice, ctx.swapChain, nullptr);
}

VkSurfaceFormatKHR VulkanSwapChain::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) const{
    for(const auto& format : availableFormats){
        if(format.format == VK_FORMAT_R8G8B8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR){
            return format;
        }
    }
    // the format we wanted was not specified, so take the first one
    return availableFormats.at(0);
}

VkPresentModeKHR VulkanSwapChain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availableModes) const{
    for(const auto& mode : availableModes){
        // if triple buffering is available
        if(mode == VK_PRESENT_MODE_MAILBOX_KHR){
            return mode;
        }
    }
    // fallback; all GPUs support double buffering
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VulkanSwapChain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) const{
    // resolution and width/height match
    if(capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()){
        return capabilities.currentExtent;
    }

    // dont match, pick best match
    int width, height;
    glfwGetFramebufferSize(ctx.window, &width, &height);

    VkExtent2D actualExtent = {
        static_cast<uint32_t>(width),
        static_cast<uint32_t>(height)
    };

    // clamp between min and max extents
    actualExtent.width = std::clamp(
        actualExtent.width,
        capabilities.minImageExtent.width,
        capabilities.maxImageExtent.width
    );

    actualExtent.height = std::clamp(
        actualExtent.height,
        capabilities.minImageExtent.height,
        capabilities.maxImageExtent.height
    );

    return actualExtent;
}

void VulkanSwapChain::createSwapChain(){
    SwapChainSupportDetails scDetails = querySwapChainSupport(ctx.Pdevice, ctx.surface);
    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(scDetails.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(scDetails.presentModes);
    VkExtent2D extent = chooseSwapExtent(scDetails.capabilities);

    // pick how many images in the swap chain
    uint32_t imageCount = scDetails.capabilities.minImageCount + 1;

    // if we exceeded max
    // NOTE maxImageCount == 0 means no max
    if(scDetails.capabilities.maxImageCount > 0 && imageCount > scDetails.capabilities.maxImageCount){
        imageCount = scDetails.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = ctx.surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    // we are rendering already, so this likely needs to be swapped to VK_IMAGE_USAGE_TRANSFER_DST_BIT
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    uint32_t queueFamilyIndices[] = {
        ctx.queueFamilies.graphicsFamily.value(),
        ctx.queueFamilies.presentFamily.value()
    };

    if(ctx.queueFamilies.graphicsFamily != ctx.queueFamilies.presentFamily){
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else{
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        // these are optional
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
    }

    // apply a transform to all images in swap chain
    createInfo.preTransform = scDetails.capabilities.currentTransform;
    // ignore alpha channel for now
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;

    // this is used if we create a new swap chain, aka if the old swap chain is invalid, maybe due to window resizing
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if(vkCreateSwapchainKHR(ctx.Ldevice, &createInfo, nullptr, &ctx.swapChain) != VK_SUCCESS){
        throw std::runtime_error("failed to create swap chain");
    }

    // grab images
    vkGetSwapchainImagesKHR(ctx.Ldevice, ctx.swapChain, &imageCount, nullptr);
    ctx.swapChainInfo.images.resize(imageCount);
    vkGetSwapchainImagesKHR(ctx.Ldevice, ctx.swapChain, &imageCount, ctx.swapChainInfo.images.data());

    ctx.swapChainInfo.swapChainExtent = extent;
    ctx.swapChainInfo.swapChainImageFormat = surfaceFormat.format;
}

void VulkanSwapChain::createImageViews(){
    ctx.swapChainInfo.views.resize(ctx.swapChainInfo.images.size());

    for(size_t i = 0; i < ctx.swapChainInfo.images.size(); i++){
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = ctx.swapChainInfo.images[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = ctx.swapChainInfo.swapChainImageFormat;

        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        if(vkCreateImageView(ctx.Ldevice, &createInfo, nullptr, &ctx.swapChainInfo.views[i]) != VK_SUCCESS){
            throw std::runtime_error("failed to create image views");
        }
    }
}

void VulkanSwapChain::createRenderPass(){
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = ctx.swapChainInfo.swapChainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;

    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;

    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if(vkCreateRenderPass(ctx.Ldevice, &renderPassInfo, nullptr, &ctx.renderPass) != VK_SUCCESS){
        throw std::runtime_error("failed to create a render pass!");
    }
}

void VulkanSwapChain::createFrameBuffers(){
    ctx.swapChainFrameBuffers.resize(ctx.swapChainInfo.views.size());

    for(size_t i = 0; i < ctx.swapChainInfo.views.size(); i++){
        VkImageView attachment[] = {ctx.swapChainInfo.views[i]};

        VkFramebufferCreateInfo frameBufferInfo{};
        frameBufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        frameBufferInfo.renderPass = ctx.renderPass;
        frameBufferInfo.attachmentCount = 1;
        frameBufferInfo.pAttachments = attachment;
        frameBufferInfo.width = ctx.swapChainInfo.swapChainExtent.width;
        frameBufferInfo.height = ctx.swapChainInfo.swapChainExtent.height;
        frameBufferInfo.layers = 1;

        if(vkCreateFramebuffer(ctx.Ldevice, &frameBufferInfo, nullptr, &ctx.swapChainFrameBuffers[i]) != VK_SUCCESS){
            throw std::runtime_error("failed to create frame buffer!");
        }
    }
}
