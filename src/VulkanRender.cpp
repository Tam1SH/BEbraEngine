#include "stdafx.h"

#include "VulkanRender.hpp"
#include "VulkanWindow.hpp"
#include "VkBuffer.hpp"
#include "Camera.hpp"
#include "matrix.hpp"
#include "Image.hpp"
#include "DescriptorSetLayouts.hpp"
#include "RenderBuffer.hpp"
#include "RenderObject.hpp"
#include "VulkanRenderObjectFactory.hpp"
#include "CommandBuffer.hpp"
#include "CommandPool.hpp"
#include <stdexcept>
#include <array>
#include <set>
#include <SDL_vulkan.h>
#include "Vertex.hpp"
#include "CreateInfoStructures.hpp"
#include "DescriptorSet.hpp"
#include "RenderObject.hpp"
#include "DescriptorPool.hpp"
#include "VulkanObjects.hpp"
#include "Shader.hpp"
#include "Pipeline.hpp"
namespace BEbraEngine {
    
    int getCurrentThreadIndex()
    {

        int index = tbb::this_task_arena::current_thread_index();
        if (index >= tbb::this_task_arena::max_concurrency())
            index = 0;

        return index;
    }

    VkDevice VulkanRender::device;

    VkPhysicalDevice VulkanRender::physicalDevice;

    VkInstance VulkanRender::instance;

    VulkanRender::QueueFamilyIndices VulkanRender::FamilyIndices;
                                      

    void VulkanRender::generateMipmaps(VulkanTexture* texture, VkFormat imageFormat, CommandBuffer& buffer) {
        // Check if image format supports linear blitting
        VkFormatProperties formatProperties;
        vkGetPhysicalDeviceFormatProperties(physicalDevice, imageFormat, &formatProperties);

        if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
            throw std::runtime_error("texture image format does not support linear blitting!");
        }

        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.image = texture->image;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;
        barrier.subresourceRange.levelCount = 1;

        int32_t mipWidth = texture->width();
        int32_t mipHeight = texture->height();

        for (uint32_t i = 1; i < texture->mipLevels; i++) {
            barrier.subresourceRange.baseMipLevel = i - 1;
            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

            vkCmdPipelineBarrier(buffer,
                VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
                0, nullptr,
                0, nullptr,
                1, &barrier);

            VkImageBlit blit{};
            blit.srcOffsets[0] = { 0, 0, 0 };
            blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
            blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            blit.srcSubresource.mipLevel = i - 1;
            blit.srcSubresource.baseArrayLayer = 0;
            blit.srcSubresource.layerCount = 1;
            blit.dstOffsets[0] = { 0, 0, 0 };
            blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
            blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            blit.dstSubresource.mipLevel = i;
            blit.dstSubresource.baseArrayLayer = 0;
            blit.dstSubresource.layerCount = 1;

            vkCmdBlitImage(buffer,
                texture->image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                texture->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                1, &blit,
                VK_FILTER_LINEAR);

            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            vkCmdPipelineBarrier(buffer,
                VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                0, nullptr,
                0, nullptr,
                1, &barrier);

            if (mipWidth > 1) mipWidth /= 2;
            if (mipHeight > 1) mipHeight /= 2;
        }

        barrier.subresourceRange.baseMipLevel = texture->mipLevels - 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(buffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
            0, nullptr,
            0, nullptr,
            1, &barrier);

    }

    void VulkanRender::createVkImage(unsigned char* data, VulkanTexture* texture, VkDeviceSize imageSize)
    {
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;

        _createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
        void* _data;

        vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &_data);
        memcpy(_data, data, static_cast<size_t>(imageSize));
        vkUnmapMemory(device, stagingBufferMemory);
        if (texture->mipLevels != 1)
            createImage(texture, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        else 
            createImage(texture, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);


        auto buffer = concurrentCommandPools_TransferQueue[getCurrentThreadIndex()]->createCommandBuffer(CommandBuffer::Type::Primary, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
        buffer.startRecord();

        transitionImageLayout(texture, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, buffer);
        copyBufferToImage(stagingBuffer, texture->image, static_cast<uint32_t>(texture->width()), static_cast<uint32_t>(texture->height()), buffer);
        
        if(texture->mipLevels != 1)
            generateMipmaps(texture, VK_FORMAT_R8G8B8A8_SRGB, buffer);
        else 
            transitionImageLayout(texture, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, buffer);

        buffer.endRecord();
        buffer.onCompleted([=] {
            vkDestroyBuffer(device, stagingBuffer, 0);
            vkFreeMemory(device, stagingBufferMemory, 0);
            });
        addBufferToQueue(buffer);
    }

    void VulkanRender::addBufferToQueue(CommandBuffer buffer)
    {
        BufferQueue.push(buffer);
    }

    VkFence* VulkanRender::getCurrentFence()
    {
        return inFlightFences.data();
    }

    void VulkanRender::recreateSwapChain(uint32_t width, uint32_t height)
    {
        vkDeviceWaitIdle(device);

        cleanupSwapChain();

        createSwapChain(width, height);
        createImageViews();
        createDepthResources();
        createRenderPass();
        createGraphicsPipeline();
        createFramebuffers();
        createCmdBuffers();
        imagesInFlight.resize(swapChainImages.size(), VK_NULL_HANDLE);
        createPools();
        recreateRenderObjects();
        for (auto& camera : cameras) {
            camera->resize(Vector2(width, height));
        }
    }

    void VulkanRender::recreateRenderObjects() {

        //auto& dec1 = globalLight.lock()->descriptor;
        auto info1 = LightDescriptorInfo();
        info1.bufferView = globalLight.lock()->data.lock().get();
        info1.type = LightDescriptorInfo::Type::Direction;

        freeDescriptor(globalLight.lock().get());
        globalLight.lock()->descriptor = createDescriptor(&info1);
        for (auto& camera : cameras) {
            camera->descriptor = createDescriptor(camera->cameraData->buffer.get());
            if (camera->isMain())
                setMainCamera = camera->descriptor;
        }

        for (auto light = lights.begin(); light != lights.end(); ++light) {
            
            auto info = LightDescriptorInfo();
            info.bufferView = (*light)->data.lock().get();
            info.type = LightDescriptorInfo::Type::Point;

            freeDescriptor((*light).get());
            (*light)->descriptor = createDescriptor(&info);

        }
        for (auto object = objects.begin(); object != objects.end(); ++object) {
             factory->CreateObjectSet((*object).get());

        }
    }

    void VulkanRender::createDepthResources()
    {
        for (int i = 0; i < swapChainImages.size(); i++) {
            auto depth = new VulkanTexture();
            depth->mipLevels = 1;

            depth->setWidth(currentRenderResolution.width);
            depth->setHeight(currentRenderResolution.height);

            depthAttachments.push_back(std::unique_ptr<VulkanTexture>(depth));

            auto color = new VulkanTexture();
            color->mipLevels = 1;

            color->setWidth(currentRenderResolution.width);
            color->setHeight(currentRenderResolution.height);

            colorAttachments.push_back(std::unique_ptr<VulkanTexture>(color));
        }
    }

    VkFormat VulkanRender::findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
    {
        for (VkFormat format : candidates) {
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

            if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
                return format;
            }
            else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
                return format;
            }
        }

        throw std::runtime_error("failed to find supported format!");
    }

    VkFormat VulkanRender::findDepthFormat()
    {
        return findSupportedFormat(
            { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
            VK_IMAGE_TILING_OPTIMAL,
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
        );
    }

    bool VulkanRender::hasStencilComponent(VkFormat format)
    {
        return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
    }




    void VulkanRender::createPools()
    {
        VulkanDescriptorPoolInfo object;
        VulkanDescriptorPoolInfo light;
        VulkanDescriptorPoolInfo camera;
        VulkanDescriptorPoolInfo texture;
        if (VulkanRenderBufferPool.get() && lightPool.get() && cameraPool.get() && attachmentsSetPool.get()) {
            object = VulkanRenderBufferPool->getInfo();
            light = lightPool->getInfo();
            camera = cameraPool->getInfo();
            texture = attachmentsSetPool->getInfo();
        }
        else {
            object.types.resize(3);
            auto size = MAX_COUNT_OF_OBJECTS;
            VkDescriptorPoolSize poolSize;
            poolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            poolSize.descriptorCount = 1;
            object.types[0] = poolSize;
            object.layout = layouts[DescriptorLayoutType::Object];

            poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            poolSize.descriptorCount = 1;
            object.types[1] = poolSize;

            poolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            poolSize.descriptorCount = 1;
            object.types[2] = poolSize;



            light.types.resize(1);
            poolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            poolSize.descriptorCount = 1;
            light.types[0] = poolSize;
            light.layout = layouts[DescriptorLayoutType::LightPoint];


            camera.types.resize(1);
            poolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            poolSize.descriptorCount = 1;
            camera.types[0] = poolSize;
            camera.layout = layouts[DescriptorLayoutType::Camera];


            texture.types.resize(2);
            poolSize.type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
            poolSize.descriptorCount = 1;
            texture.types[0] = poolSize;
            poolSize.type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
            poolSize.descriptorCount = 1;
            texture.types[1] = poolSize;

            texture.layout = layouts[DescriptorLayoutType::Attachments];


        }

        VulkanRenderBufferPool.reset();
        VulkanRenderBufferPool = std::unique_ptr<DescriptorPool>(new DescriptorPool(object));
        VulkanRenderBufferPool->allocate(MAX_COUNT_OF_OBJECTS);

        cameraPool.reset();
        cameraPool = std::unique_ptr<DescriptorPool>(new DescriptorPool(camera));
        cameraPool->allocate(1);

        lightPool.reset();
        lightPool = std::unique_ptr<DescriptorPool>(new DescriptorPool(light));
        lightPool->allocate(MAX_COUNT_OF_LIGHTS);

        attachmentsSetPool.reset();
        attachmentsSetPool = std::unique_ptr<DescriptorPool>(new DescriptorPool(texture));
        attachmentsSetPool->allocate(100);

    }

    void VulkanRender::cleanupSwapChain()
    {

        for (auto framebuffer : swapChainFramebuffers) {
            vkDestroyFramebuffer(device, framebuffer, nullptr);
        }

        //vkFreeCommandBuffers(device, _commandPool, RenderBuffers.size(), RenderBuffers.data());
        vkDestroyPipeline(device, graphicsPipeline, nullptr);
        vkDestroyPipeline(device, graphicsPipeline2, nullptr);
        vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
        vkDestroyRenderPass(device, renderPass, nullptr);

        for (auto imageView : swapChainImageViews) {
            vkDestroyImageView(device, imageView, nullptr);
        }

        vkDestroySwapchainKHR(device, swapChain, nullptr);
    }

    void VulkanRender::cleanUpDefault()
    {
        executeQueues_Objects.reset();
        cleanupSwapChain();
        for (auto& [name, layout] : layouts) {
            vkDestroyDescriptorSetLayout(device, layout, 0);
        }

        for (auto& object : objects) {
            factory->destroyObject(object);
        }

        VulkanRenderBufferPool.reset();
        cameraPool.reset();
        lightPool.reset();
        attachmentsSetPool.reset();
        for (int i = 0; i < depthAttachments.size(); i++) {
            destroyTexture(depthAttachments[i].get());
        }

        objects.clear();
        
        factory.reset();
        cameras.clear();

        lights.clear();
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
            vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
            vkDestroyFence(device, inFlightFences[i], nullptr);
        }

        for (int i = 0; i < concurrentCommandPools_RenderQueue.size(); i++) {
            concurrentCommandPools_RenderQueue[i].reset();
            concurrentCommandPools_TransferQueue[i].reset();
        }

        vkDestroyDevice(device, nullptr);

        if (enableValidationLayers) {
            DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
        }

        vkDestroySurfaceKHR(instance, surface, nullptr);
        vkDestroyInstance(instance, nullptr);

    }


    void VulkanRender::_createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
    {

        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to create buffer!");
        }


        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;

        allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

        if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate buffer memory!");
        }

        vkBindBufferMemory(device, buffer, bufferMemory, 0);
    }

    uint32_t VulkanRender::alignmentBuffer(uint32_t originalSize, AbstractRender::TypeBuffer type)
    {
        VkDeviceSize alignment = 0;
        if(type == AbstractRender::TypeBuffer::Uniform)
            alignment = GPU_properties.limits.minUniformBufferOffsetAlignment;
        if(type == AbstractRender::TypeBuffer::Storage)
            alignment = GPU_properties.limits.minStorageBufferOffsetAlignment;

        VkDeviceSize alignedSize = originalSize;
        if (alignment > 0) {
            alignedSize = (alignedSize + alignment - 1) & ~(alignment - 1);
        }
        return alignedSize;
    }

    void VulkanRender::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
    {
        auto commandBuffer = concurrentCommandPools_TransferQueue[getCurrentThreadIndex()]->createCommandBuffer(
            CommandBuffer::Type::Primary, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

        commandBuffer.startRecord();

        VkBufferCopy copyRegion{};
        copyRegion.size = size;
        vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
        commandBuffer.endRecord();
        addBufferToQueue(commandBuffer);

    }

    void VulkanRender::copyBuffer1(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, VkCommandBuffer& cmdBuffer)
    {
        VkBufferCopy copyRegion{};
        copyRegion.size = size;
        vkCmdCopyBuffer(cmdBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    }


    VkImageView VulkanRender::createImageView(VulkanTexture* texture, VkFormat format, VkImageAspectFlags aspectFlags)
    {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = texture->image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = format;
        viewInfo.subresourceRange.aspectMask = aspectFlags;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = texture->mipLevels;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        VkImageView imageView;
        if (vkCreateImageView(device, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture image view!");
        }

        return imageView;
    }

    void VulkanRender::createTextureSampler(VulkanTexture* texture)
    {
        VkPhysicalDeviceProperties properties{};
        vkGetPhysicalDeviceProperties(physicalDevice, &properties);

        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.anisotropyEnable = VK_FALSE;
        samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = static_cast<float>(texture->mipLevels);
        samplerInfo.mipLodBias = 0.0f;
        if (vkCreateSampler(device, &samplerInfo, nullptr, &texture->sampler) != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture sampler!");
        }
    }

    void VulkanRender::createInstance()
    {
        if (enableValidationLayers && !checkValidationLayerSupport()) {
            throw std::runtime_error("validation layers requested, but not available!");
        }

        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "bebrA";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "BEBRA Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        auto extensions = getRequiredExtensions();
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();

        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
        if (enableValidationLayers) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();

            populateDebugMessengerCreateInfo(debugCreateInfo);
            createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
        }
        else {
            createInfo.enabledLayerCount = 0;

            createInfo.pNext = nullptr;
        }

        if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
            throw std::runtime_error("failed to create instance!");
        }
    }

    void VulkanRender::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
    {
        createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = debugCallback;
    }

    void VulkanRender::setupDebugMessenger()
    {
        if (!enableValidationLayers) return;

        VkDebugUtilsMessengerCreateInfoEXT createInfo;
        populateDebugMessengerCreateInfo(createInfo);

        if (createDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
            throw std::runtime_error("failed to set up debug messenger!");
        }
    }

    void VulkanRender::pickPhysicalDevice()
    {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

        if (deviceCount == 0) {
            throw std::runtime_error("failed to find GPUs with Vulkan support!");
        }

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

        for (const auto& d : devices) {
            if (isDeviceSuitable(d)) {
                physicalDevice = d;
                break;
            }
        }

        if (physicalDevice == VK_NULL_HANDLE) {
            throw std::runtime_error("failed to find a suitable GPU!");
        }
        vkGetPhysicalDeviceProperties(physicalDevice, &GPU_properties);

    }

    VulkanRender::QueueFamilyIndices VulkanRender::findQueueFamilies(VkPhysicalDevice device)
    {
        /*
    
        const VkQueueFlagBits askingFlags[3] = { VK_QUEUE_GRAPHICS_BIT, VK_QUEUE_COMPUTE_BIT, VK_QUEUE_TRANSFER_BIT };
        uint32_t queuesIndices[3] = { ~0u, ~0u, ~0u };

        uint32_t queueFamilyPropertyCount;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyPropertyCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyPropertyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyPropertyCount, queueFamilyProperties.data());

        for (size_t i = 0; i < 3; ++i) {
            const VkQueueFlagBits flag = askingFlags[i];
            uint32_t& queueIdx = queuesIndices[i];

            if (flag == VK_QUEUE_COMPUTE_BIT) {
                for (uint32_t j = 0; j < queueFamilyPropertyCount; ++j) {
                    if ((queueFamilyProperties[j].queueFlags & VK_QUEUE_COMPUTE_BIT) &&
                        !(queueFamilyProperties[j].queueFlags & VK_QUEUE_GRAPHICS_BIT)) {
                        queueIdx = j;
                        break;
                    }
                }
            }
            else if (flag == VK_QUEUE_TRANSFER_BIT) {
                for (uint32_t j = 0; j < queueFamilyPropertyCount; ++j) {
                    if ((queueFamilyProperties[j].queueFlags & VK_QUEUE_TRANSFER_BIT) &&
                        !(queueFamilyProperties[j].queueFlags & VK_QUEUE_GRAPHICS_BIT) &&
                        !(queueFamilyProperties[j].queueFlags & VK_QUEUE_COMPUTE_BIT)) {
                        queueIdx = j;
                        break;
                    }
                }
            }

            if (queueIdx == ~0u) {
                for (uint32_t j = 0; j < queueFamilyPropertyCount; ++j) {
                    if (queueFamilyProperties[j].queueFlags & flag) {
                        queueIdx = j;
                        break;
                    }
                }
            }
        }
        QueueFamilyIndices indices;
        indices.graphicsFamily = queuesIndices[0];
        indices.transferFamily = queuesIndices[2];
        indices.graphicsFamily = queuesIndices[0];
        return indices;
        */
        QueueFamilyIndices indices;
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
        int i = 0;
        for (const auto& queueFamily : queueFamilies) {
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                indices.graphicsFamily = i;
            }
            if (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT) {
                indices.transferFamily = i;
            }

            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

            if (presentSupport) {
                indices.presentFamily = i;
            }
            if (indices.isComplete()) {
                break;
            }
            i++;
        }
        if (!indices.isComplete()) {
            std::cout << "NOT ALL QUEUE ARE SELECTED" << std::endl;
        }

        return indices;
     
    }

    void VulkanRender::createLogicalDevice()
    {
        QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value(), indices.transferFamily.value() };

        float queuePriority = 1.0f;
        for (uint32_t queueFamily : uniqueQueueFamilies) {
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }

        VkPhysicalDeviceFeatures deviceFeatures{};

        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();

        createInfo.pEnabledFeatures = &deviceFeatures;

        createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
        createInfo.ppEnabledExtensionNames = deviceExtensions.data();

        if (enableValidationLayers) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();
        }
        else {
            createInfo.enabledLayerCount = 0;
        }

        if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
            throw std::runtime_error("failed to create logical device!");
        }

        vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
        vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
        vkGetDeviceQueue(device, indices.transferFamily.value(), 0, &transferQueue);
    }


    void VulkanRender::createSwapChain(uint32_t width, uint32_t height)
    {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

        VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
        VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
        VkExtent2D extent;
        extent.height = height;
        extent.width = width;

        uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
        if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
            imageCount = swapChainSupport.capabilities.maxImageCount;
        }
        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = surface;
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
        uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

        if (indices.graphicsFamily != indices.presentFamily) {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        }
        else {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        }

        createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;

        if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
            throw std::runtime_error("failed to create swap chain!");
        }

        vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
        swapChainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

        swapChainImageFormat = surfaceFormat.format;
        currentRenderResolution = extent;
    }

    void VulkanRender::createImageViews()
    {
        swapChainImageViews.resize(swapChainImages.size());

        for (size_t i = 0; i < swapChainImages.size(); i++) {
            VkImageViewCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image = swapChainImages[i];
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.format = swapChainImageFormat;
            createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.levelCount = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount = 1;

            if (vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create image views!");
            }
        }
    }

    void VulkanRender::createAttachment(VkFormat format, VkImageUsageFlags usage, VulkanTexture* attachment)
    {
        VkImageAspectFlags aspectMask = 0;
        VkImageLayout imageLayout;

        if (usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) {
            aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        }
        if (usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) {
            aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
            imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        }

        VkImageCreateInfo imageCI{};
        imageCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageCI.imageType = VK_IMAGE_TYPE_2D;
        imageCI.format = format;
        imageCI.extent.width = attachment->width();
        imageCI.extent.height = attachment->height();
        imageCI.extent.depth = 1;
        imageCI.mipLevels = 1;
        imageCI.arrayLayers = 1;
        imageCI.samples = VK_SAMPLE_COUNT_1_BIT;
        imageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageCI.usage = usage | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
        imageCI.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        vkCreateImage(device, &imageCI, nullptr, &attachment->image);

        VkMemoryAllocateInfo memAlloc{};
        VkMemoryRequirements memReqs;
        vkGetImageMemoryRequirements(device, attachment->image, &memReqs);
        memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        memAlloc.allocationSize = memReqs.size;
        memAlloc.memoryTypeIndex = findMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        vkAllocateMemory(device, &memAlloc, nullptr, &attachment->memory);
        vkBindImageMemory(device, attachment->image, attachment->memory, 0);

        VkImageViewCreateInfo imageViewCI{};
        imageViewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewCI.format = format;
        imageViewCI.subresourceRange = {};
        imageViewCI.subresourceRange.aspectMask = aspectMask;
        imageViewCI.subresourceRange.baseMipLevel = 0;
        imageViewCI.subresourceRange.levelCount = 1;
        imageViewCI.subresourceRange.baseArrayLayer = 0;
        imageViewCI.subresourceRange.layerCount = 1;
        imageViewCI.image = attachment->image;

        vkCreateImageView(device, &imageViewCI, nullptr, &attachment->imageView);
    }

    void VulkanRender::createRenderPass()
    {
        Debug::log("Start createRenderPass \n\n\n\n\n\n");
        for (int i = 0; i < swapChainImages.size(); i++) {
            createAttachment(VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, colorAttachments[i].get());
            createAttachment(findDepthFormat(), VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, depthAttachments[i].get());
            
        }
        VkAttachmentDescription swapChainImageAttachment{};
        swapChainImageAttachment.format = swapChainImageFormat;
        swapChainImageAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        swapChainImageAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        swapChainImageAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        swapChainImageAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        swapChainImageAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        swapChainImageAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        swapChainImageAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = VK_FORMAT_R8G8B8A8_UNORM;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentDescription depthAttachment{};
        depthAttachment.format = findDepthFormat();
        depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference swapChainColorAttachmentRef{};
        swapChainColorAttachmentRef.attachment = 0;
        swapChainColorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 1;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentReference depthAttachmentRef{};
        depthAttachmentRef.attachment = 2;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;



        std::array<VkSubpassDescription,2> subpassDescriptions{};
        subpassDescriptions[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpassDescriptions[0].colorAttachmentCount = 1;
        subpassDescriptions[0].pColorAttachments = &colorAttachmentRef;
        subpassDescriptions[0].pDepthStencilAttachment = &depthAttachmentRef;

        subpassDescriptions[1].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpassDescriptions[1].colorAttachmentCount = 1;
        subpassDescriptions[1].pColorAttachments = &swapChainColorAttachmentRef;

        std::array<VkAttachmentReference,2> inputReferences{};
        inputReferences[0].attachment = 1;
        inputReferences[0].layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        inputReferences[1].attachment = 2;
        inputReferences[1].layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        subpassDescriptions[1].inputAttachmentCount = inputReferences.size();
        subpassDescriptions[1].pInputAttachments = inputReferences.data();

        std::array<VkSubpassDependency, 3> dependencies{};//[2];

        dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[0].dstSubpass = 0;
        dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        // This dependency transitions the input attachment from color attachment to shader read
        dependencies[1].srcSubpass = 0;
        dependencies[1].dstSubpass = 1;
        dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        dependencies[2].srcSubpass = 0;
        dependencies[2].dstSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[2].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[2].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        dependencies[2].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[2].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        dependencies[2].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        
        std::array<VkAttachmentDescription, 3> attachments = { swapChainImageAttachment, colorAttachment, depthAttachment };

        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = attachments.size();
        renderPassInfo.pAttachments = attachments.data();
        renderPassInfo.subpassCount = subpassDescriptions.size();
        renderPassInfo.pSubpasses = subpassDescriptions.data();
        renderPassInfo.dependencyCount = dependencies.size();
        renderPassInfo.pDependencies = dependencies.data();

        if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
            throw std::runtime_error("failed to create render pass!");
        }
        Debug::log("End createRenderPass \n\n\n\n\n\n");
    }

    void VulkanRender::createAttachmentsSet()
    {
        attachmentsSets.resize(swapChainImages.size());
        for (int i = 0; i < swapChainImages.size(); i++) {
            VkDescriptorSet setAttachments{};
            auto opt_setAttachments = attachmentsSetPool->get();
            
            if (opt_setAttachments.has_value()) {
                setAttachments = opt_setAttachments.value();

                attachmentsSets[i] = setAttachments;

                std::array<VkDescriptorImageInfo, 2> descriptors{};
                descriptors[0].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                descriptors[0].imageView = colorAttachments[i]->imageView;
                descriptors[0].sampler = VK_NULL_HANDLE;

                descriptors[1].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                descriptors[1].imageView = depthAttachments[i]->imageView;
                descriptors[1].sampler = VK_NULL_HANDLE;
                
                std::array<VkWriteDescriptorSet, 2> writeDescriptorSets{};
                writeDescriptorSets[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                writeDescriptorSets[0].dstSet = setAttachments;
                
                writeDescriptorSets[0].descriptorType = attachmentsSetPool->getInfo().types[0].type;
                writeDescriptorSets[0].descriptorCount = 1;
                writeDescriptorSets[0].dstBinding = 0;
                writeDescriptorSets[0].pImageInfo = &descriptors[0];

                writeDescriptorSets[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                writeDescriptorSets[1].dstSet = setAttachments;
                writeDescriptorSets[1].descriptorType = attachmentsSetPool->getInfo().types[1].type;
                writeDescriptorSets[1].descriptorCount = 1;
                writeDescriptorSets[1].dstBinding = 1;
                writeDescriptorSets[1].pImageInfo = &descriptors[1];

                vkUpdateDescriptorSets(device, writeDescriptorSets.size(), writeDescriptorSets.data(), 0, nullptr);

            }
            else throw std::runtime_error("PIzda nakrilsa naxyu");
        }
    }

    void VulkanRender::createGraphicsPipeline()
    {
        {


            VulkanShader* vertShaderModule{}, * fragShaderModule{};

            vertShaderModule = VulkanShader::createFromFile(device, "writeVert.spv");
            fragShaderModule = VulkanShader::createFromFile(device, "writeFrag.spv");

            VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
            vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
            vertShaderStageInfo.module = vertShaderModule->module;
            vertShaderStageInfo.pName = "main";

            VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
            fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            fragShaderStageInfo.module = fragShaderModule->module;
            fragShaderStageInfo.pName = "main";

            VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

            VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
            vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

            VkVertexInputBindingDescription bindingDescription = Vertex::getBindingDescription();
            auto attributeDescriptions = Vertex::getAttributeDescriptions();

            vertexInputInfo.vertexBindingDescriptionCount = 1;
            vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
            vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
            vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

            VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
            inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
            inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            inputAssembly.primitiveRestartEnable = VK_FALSE;

            VkViewport viewport{};
            viewport.x = 0.0f;
            viewport.y = (float)currentRenderResolution.height;
            viewport.width = (float)currentRenderResolution.width;
            viewport.height = -(float)currentRenderResolution.height;
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;

            VkRect2D scissor{};
            scissor.offset = { 0, 0 };
            scissor.extent = currentRenderResolution;

            VkPipelineViewportStateCreateInfo viewportState{};
            viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
            viewportState.viewportCount = 1;
            viewportState.pViewports = &viewport;
            viewportState.scissorCount = 1;
            viewportState.pScissors = &scissor;

            VkPipelineRasterizationStateCreateInfo rasterizer{};
            rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
            rasterizer.depthClampEnable = VK_FALSE;
            rasterizer.rasterizerDiscardEnable = VK_FALSE;
            rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
            rasterizer.lineWidth = 1.0f;
            rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
            rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
            rasterizer.depthBiasEnable = VK_FALSE;

            VkPipelineMultisampleStateCreateInfo multisampling{};
            multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
            multisampling.sampleShadingEnable = false;
            multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

            VkPipelineDepthStencilStateCreateInfo depthStencil{};
            depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
            depthStencil.depthTestEnable = VK_TRUE;
            depthStencil.depthWriteEnable = VK_TRUE;
            depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
            depthStencil.depthBoundsTestEnable = VK_FALSE;
            depthStencil.stencilTestEnable = VK_FALSE;
            depthStencil.minDepthBounds = 0.0f;
            depthStencil.maxDepthBounds = 1.0f;

            VkPipelineColorBlendAttachmentState colorBlendAttachment{};
            colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
            colorBlendAttachment.blendEnable = VK_FALSE;

            VkPipelineColorBlendStateCreateInfo colorBlending{};
            colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
            colorBlending.logicOpEnable = VK_FALSE;
            colorBlending.logicOp = VK_LOGIC_OP_COPY;
            colorBlending.attachmentCount = 1;
            colorBlending.pAttachments = &colorBlendAttachment;
            colorBlending.blendConstants[0] = 0.0f;
            colorBlending.blendConstants[1] = 0.0f;
            colorBlending.blendConstants[2] = 0.0f;
            colorBlending.blendConstants[3] = 0.0f;

            VkPipelineLayoutCreateInfo pipelineLayoutInfo{};


            VkPushConstantRange push_constant;
            push_constant.offset = 0;
            push_constant.size = sizeof(int);
            push_constant.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;



            VkDescriptorSetLayout layouts[] = {
                this->layouts[DescriptorLayoutType::Object],
                this->layouts[DescriptorLayoutType::Camera],
                this->layouts[DescriptorLayoutType::LightPoint],
                this->layouts[DescriptorLayoutType::DirectionLight],
                this->layouts[DescriptorLayoutType::Attachments]
            };
            pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            pipelineLayoutInfo.pNext = nullptr;
            pipelineLayoutInfo.setLayoutCount = 5;
            pipelineLayoutInfo.pSetLayouts = layouts;
            pipelineLayoutInfo.pPushConstantRanges = &push_constant;
            pipelineLayoutInfo.pushConstantRangeCount = 1;

            if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
                throw std::runtime_error("failed to create pipeline layout!");
            }
            VkGraphicsPipelineCreateInfo pipelineInfo{};
            pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
            pipelineInfo.stageCount = 2;
            pipelineInfo.pStages = shaderStages;
            pipelineInfo.pVertexInputState = &vertexInputInfo;
            pipelineInfo.pInputAssemblyState = &inputAssembly;
            pipelineInfo.pViewportState = &viewportState;
            pipelineInfo.pRasterizationState = &rasterizer;
            pipelineInfo.pMultisampleState = &multisampling;
            pipelineInfo.pColorBlendState = &colorBlending;
            pipelineInfo.layout = pipelineLayout;
            pipelineInfo.renderPass = renderPass;
            pipelineInfo.subpass = 0;
            pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
            pipelineInfo.pDepthStencilState = &depthStencil;

            if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
                throw std::runtime_error("failed to create graphics pipeline!");
            }



            vkDestroyShaderModule(device, fragShaderModule->module, nullptr);
            vkDestroyShaderModule(device, vertShaderModule->module, nullptr);
            //���������
            pipelineInfo.subpass = 1;
            pipelineInfo.layout = pipelineLayout;

            VkPipelineVertexInputStateCreateInfo emptyInputStateCI{};
            emptyInputStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

            pipelineInfo.pVertexInputState = &emptyInputStateCI;
            colorBlending.attachmentCount = 1;
            rasterizer.cullMode = VK_CULL_MODE_NONE;
            depthStencil.depthWriteEnable = VK_FALSE;

            //���������
            {
                VulkanShader* vertShaderModule{}, * fragShaderModule{};

                vertShaderModule = VulkanShader::createFromFile(device, "readVert.spv");
                fragShaderModule = VulkanShader::createFromFile(device, "readFrag.spv");

                VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
                vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
                vertShaderStageInfo.module = vertShaderModule->module;
                vertShaderStageInfo.pName = "main";

                VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
                fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
                fragShaderStageInfo.module = fragShaderModule->module;
                fragShaderStageInfo.pName = "main";

                VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };
                pipelineInfo.pStages = shaderStages;
                if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline2) != VK_SUCCESS) {
                    throw std::runtime_error("failed to create graphics pipeline!");
                }
            }
        }
        //������


    }

    void VulkanRender::createFramebuffers()
    {
        swapChainFramebuffers.resize(swapChainImageViews.size());

        for (size_t i = 0; i < swapChainImageViews.size(); i++) {
            std::array<VkImageView, 3> attachments = {
                swapChainImageViews[i],
                colorAttachments[i]->imageView,
                depthAttachments[i]->imageView

            };

            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = renderPass;
            framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
            framebufferInfo.pAttachments = attachments.data();
            framebufferInfo.width = currentRenderResolution.width;
            framebufferInfo.height = currentRenderResolution.height;
            framebufferInfo.layers = 1;

            if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create framebuffer!");
            }
           // auto frame = new VulkanTexture;
          //  vkCopyImage
           // createVkImage(0, frame, 4);
           // frameBuffers.push_back(std::unique_ptr<VulkanTexture>(frame));
        }

    }

    void VulkanRender::createCommandPool()
    {
        QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

        if (vkCreateCommandPool(device, &poolInfo, nullptr, &_commandPool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create graphics command pool!");
        }
    }


    void VulkanRender::createObjectDescriptorSetLayout()
    {
        VkDescriptorSetLayout layout;
        std::array<VkDescriptorSetLayoutBinding, 2> uboLayoutBinding{};
        uboLayoutBinding[0].binding = 0;
        uboLayoutBinding[0].descriptorCount = 1;
        uboLayoutBinding[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        uboLayoutBinding[0].pImmutableSamplers = nullptr;
        uboLayoutBinding[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

        uboLayoutBinding[1].binding = 1;
        uboLayoutBinding[1].descriptorCount = 1;
        uboLayoutBinding[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        uboLayoutBinding[1].pImmutableSamplers = nullptr;
        uboLayoutBinding[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;


        std::array<VkDescriptorSetLayoutBinding, 2> bindings = uboLayoutBinding;
        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        layoutInfo.pBindings = bindings.data();

        if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &layout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor set layout!");
        }
        layouts[DescriptorLayoutType::Object] = layout;

    }

    void VulkanRender::createLightDescriptorSetLayout()
    {
        VkDescriptorSetLayout layoutPoint;
        VkDescriptorSetLayout layoutDirection;
        std::array<VkDescriptorSetLayoutBinding, 1> LightLayoutBinding{};
        LightLayoutBinding[0].binding = 0;
        LightLayoutBinding[0].descriptorCount = 1;
        LightLayoutBinding[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        LightLayoutBinding[0].pImmutableSamplers = nullptr;
        LightLayoutBinding[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;


        std::array<VkDescriptorSetLayoutBinding, 1> bindings = LightLayoutBinding;
        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        layoutInfo.pBindings = bindings.data();

        vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &layoutPoint);
        vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &layoutDirection);

        layouts[DescriptorLayoutType::LightPoint] = layoutPoint;
        layouts[DescriptorLayoutType::DirectionLight] = layoutDirection;
    }

    void VulkanRender::createCameraDescriptorSetLayout()
    {
        VkDescriptorSetLayout layout;
        std::array<VkDescriptorSetLayoutBinding, 1> CameraLayoutBinding{};
        CameraLayoutBinding[0].binding = 0;
        CameraLayoutBinding[0].descriptorCount = 1;
        CameraLayoutBinding[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        CameraLayoutBinding[0].pImmutableSamplers = nullptr;
        CameraLayoutBinding[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;


        std::array<VkDescriptorSetLayoutBinding, 1> bindings = CameraLayoutBinding;
        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        layoutInfo.pBindings = bindings.data();

        if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &layout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor set layout!");
        }
        layouts[DescriptorLayoutType::Camera] = layout;
    }

    void VulkanRender::createSyncObjects()
    {
        imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

        imagesInFlight.resize(swapChainImages.size(), VK_NULL_HANDLE);

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
                vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
                vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create synchronization objects for a frame!");
            }
        }
    }

    void VulkanRender::createImage(VulkanTexture* texture, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties)
    {
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = texture->width();
        imageInfo.extent.height = texture->height();
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = texture->mipLevels;
    
        imageInfo.arrayLayers = 1;
        imageInfo.format = format;
        imageInfo.tiling = tiling;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = usage;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateImage(device, &imageInfo, nullptr, &texture->image) != VK_SUCCESS) {
            throw std::runtime_error("failed to create image!");
        }

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(device, texture->image, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

        if (vkAllocateMemory(device, &allocInfo, nullptr, &texture->memory) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate image memory!");
        }

        vkBindImageMemory(device, texture->image, texture->memory, 0);
    }

    void VulkanRender::transitionImageLayout(VulkanTexture* texture, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, VkCommandBuffer& buffer)
    {

        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = //VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = texture->image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = texture->mipLevels;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        VkPipelineStageFlags sourceStage;
        VkPipelineStageFlags destinationStage;





        if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        }
        else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }
        else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        }
        else {
            throw std::invalid_argument("unsupported layout transition!");
        }


        vkCmdPipelineBarrier(buffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
    
    }

    void VulkanRender::createCmdBuffers()
    {
        RenderBuffers.resize(swapChainFramebuffers.size());

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = _commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = static_cast<uint32_t>(RenderBuffers.size());


    }

    void VulkanRender::drawFrame()
    {

        updateCmdBuffers();


        vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);
        /*
        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            recreateSwapChain(window->getDrawableSize().x, window->getDrawableSize().y);
            return;
        }
        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("failed to acquire swap chain image!");
        }
        */

        if (imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
            vkWaitForFences(device, 1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
        }
        imagesInFlight[imageIndex] = inFlightFences[currentFrame];

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

        std::vector<VkCommandBuffer> buffers;
        std::vector<CommandBuffer> bufDestroy;
        while (!BufferQueue.empty()) {
            CommandBuffer buffer;
            BufferQueue.try_pop(buffer);
            buffers.push_back(buffer);
            bufDestroy.push_back(buffer);
        }
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
    
        submitInfo.commandBufferCount = static_cast<uint32_t>(buffers.size());
        submitInfo.pCommandBuffers = buffers.data();

        VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;
        vkResetFences(device, 1, &inFlightFences[currentFrame]);

        if (auto res = vkQueueSubmit(getGraphicsQueue(), 1, &submitInfo, inFlightFences[currentFrame]); res != VK_SUCCESS) {}

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapChains[] = { swapChain };
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
    
        presentInfo.pImageIndices = &imageIndex;

        result = vkQueuePresentKHR(presentQueue, &presentInfo);
        /*
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
            framebufferResized = false;
            recreateSwapChain();
        }
        else if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to present swap chain image!");
        }
        */

        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
        vkQueueWaitIdle(getGraphicsQueue());
        for (auto& buf : bufDestroy)
            buf.destroy();
        executeQueues_Objects.execute();

    }

    void VulkanRender::updateCmdBuffers()
    {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
        for (auto& camera : cameras) {
            camera->update();
        }

        globalLight.lock()->update();
        for (auto light = lights.begin(); light != lights.end(); ++light) {
            (*light)->update();
        }
        for (auto object = objects.begin(); object != objects.end(); ++object) {
            (*object)->update();
        }
        for (int i = 0; i < swapChainFramebuffers.size(); i++) {

            VkRenderPassBeginInfo renderPassInfo{};

            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassInfo.renderPass = renderPass;
            renderPassInfo.framebuffer = swapChainFramebuffers[i];
            renderPassInfo.renderArea.offset = { 0, 0 };

            renderPassInfo.renderArea.extent = {
                currentRenderResolution.width, currentRenderResolution.height
            };
            std::array<VkClearValue, 3> clearValues{};

            clearValues[0].color = { { 255 / 255.f, 174 / 255.f, 201 / 255.f, 1.0f } };
            clearValues[1].color = { { 255 / 255.f, 174 / 255.f, 201 / 255.f, 1.0f } };
            clearValues[2].depthStencil = { 1.0f, 0 };
            renderPassInfo.clearValueCount = clearValues.size();
            renderPassInfo.pClearValues = clearValues.data();
            
            VkViewport viewport{};
            viewport.x = 0.0f;
            viewport.y = (float)currentRenderResolution.height;
            viewport.width = //(float)300;
                (float)currentRenderResolution.width;
            viewport.height = //-(float)600;
                -(float)currentRenderResolution.height;
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;
            
            auto buffer = concurrentCommandPools_RenderQueue[getCurrentThreadIndex()]->createCommandBuffer(CommandBuffer::Type::Primary,
                VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
            buffer.startRecord();
            
            vkCmdSetViewport(buffer, 0, 1, &viewport);
            vkCmdBeginRenderPass(buffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
            vkCmdBindPipeline(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

            uint32_t count = lights.size();
            vkCmdPushConstants(buffer, pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(uint32_t), &count);
            
            {
                
                vkCmdBindDescriptorSets(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 3, 1, &globalLight.lock()->descriptor, 0, 0);

                vkCmdBindDescriptorSets(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 1, 1, &setMainCamera, 0, nullptr);
                if(!objects.empty())
                    vkCmdBindDescriptorSets(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &(*objects.begin())->descriptor, 0, nullptr);
                //tbb::parallel_for<size_t>(0, 12, [](size_t i) {
                for (auto light = lights.begin(); light != lights.end(); ++light) {
                    vkCmdBindDescriptorSets(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 2, 1, &(*light)->descriptor, 0, nullptr);
                }
                for (auto object = objects.begin(); object != objects.end(); ++object) {
                    (*object)->draw(buffer);
                }

                vkCmdNextSubpass(buffer, VK_SUBPASS_CONTENTS_INLINE);
                vkCmdBindPipeline(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline2);

                vkCmdBindDescriptorSets(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 4, 1, &attachmentsSets[i], 0, NULL);
                vkCmdDraw(buffer, 3, 1, 0, 0);

                vkCmdEndRenderPass(buffer);
                
                //      auto data = ImGui::GetDrawData();
                 //     if (data)
                 //         ImGui_ImplVulkan_RenderDrawData(data, RenderBuffers[i]);

                
            }

            vkEndCommandBuffer(buffer);
            BufferQueue.push(buffer);
        }

    }

    VkShaderModule VulkanRender::createShaderModule(const std::vector<char>& code)
    {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

        VkShaderModule shaderModule;
        if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
            throw std::runtime_error("failed to create shader module!");
        }

        return shaderModule;
    }

    uint32_t VulkanRender::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
    {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(VulkanRender::physicalDevice, &memProperties);

        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
            if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }

        throw std::runtime_error("failed to find suitable memory type!");
    }

    void VulkanRender::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, VkCommandBuffer& cmdbuffer)
    {

        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;
        region.imageOffset = { 0, 0, 0 };
        region.imageExtent = {
            width,
            height,
            1
        };

        vkCmdCopyBufferToImage(cmdbuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
    }

    VkSurfaceFormatKHR VulkanRender::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
    {
        for (const auto& availableFormat : availableFormats) {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return availableFormat;
            }
        }

        return availableFormats[0];
    }

    VkPresentModeKHR VulkanRender::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
    {
        for (const auto& availablePresentMode : availablePresentModes) {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                return availablePresentMode;
            }
        }

        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VulkanRender::SwapChainSupportDetails VulkanRender::querySwapChainSupport(VkPhysicalDevice device)
    {

        SwapChainSupportDetails details;

        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

        if (formatCount != 0) {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
        }

        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

        if (presentModeCount != 0) {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
        }

        return details;
    }

    bool VulkanRender::isDeviceSuitable(VkPhysicalDevice device)
    {
        QueueFamilyIndices indices = findQueueFamilies(device);

        bool extensionsSupported = checkDeviceExtensionSupport(device);

        bool swapChainAdequate = false;
        if (extensionsSupported) {
            SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
            swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        }

        return indices.isComplete() && extensionsSupported && swapChainAdequate;
    }

    bool VulkanRender::checkDeviceExtensionSupport(VkPhysicalDevice device)
    {
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

        std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

        for (const auto& extension : availableExtensions) {
            requiredExtensions.erase(extension.extensionName);
        }

        return requiredExtensions.empty();
    }

    std::vector<const char*> VulkanRender::getRequiredExtensions()
    {
        std::vector<const char*> extensions;
        extensions = window->vulkan_GetInstanceExtensions();

        if (enableValidationLayers) {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        return extensions;
    }

    bool VulkanRender::checkValidationLayerSupport()
    {
        uint32_t layerCount;

        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        for (const char* layerName : validationLayers) {
            bool layerFound = false;

            for (const auto& layerProperties : availableLayers) {
                if (strcmp(layerName, layerProperties.layerName) == 0) {
                    layerFound = true;
                    break;
                }
            }

            if (!layerFound) {
                return false;
            }
        }

        return true;
    }

    VKAPI_ATTR VkBool32 VKAPI_CALL VulkanRender::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
    {
        if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
        {
            std::stringstream ss;
            ss << pCallbackData->pMessage << std::endl;

            Debug::log(ss.str());
            Debug::log("\n\n\n\n\n");
        }
        else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        {
            std::cout << "Validation Layer: Warning: " << pCallbackData->pMessage << std::endl;
        }
        else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT)
        {
            std::cout << "Validation Layer: Performance warning: " << pCallbackData->pMessage << std::endl;
        }
        else
        {
            std::cout << "Validation Layer: Information: " << pCallbackData->pMessage << std::endl;
        }

        return VK_FALSE;
    }

    std::vector<char> VulkanRender::readFile(const std::string& filename)
    {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);

        if (!file.is_open()) {
            throw std::runtime_error("failed to open file!");
        }

        size_t fileSize = (size_t)file.tellg();
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);

        file.close();

        return buffer;
    }

    VkResult VulkanRender::createDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
    {
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
        if (func != nullptr) {
            return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
        }
        else {
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }
    }

    void VulkanRender::DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
    {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr) {
            func(instance, debugMessenger, pAllocator);
        }
    }

    void VulkanRender::addCamera(std::shared_ptr<Camera> camera)
    {
        camera->resize({ static_cast<float>(currentRenderResolution.width), 
                         static_cast<float>(currentRenderResolution.height) });
        cameras.push_back(std::dynamic_pointer_cast<VulkanCamera>(camera));
    
    }

    void VulkanRender::selectMainCamera(Camera* camera)
    {
        auto vCamera = static_cast<VulkanCamera*>(camera);
        setMainCamera = vCamera->descriptor;
        vCamera->setMain(true);
    }

    void VulkanRender::removeCamera(std::shared_ptr<Camera> camera)
    {
        cameras.remove(std::dynamic_pointer_cast<VulkanCamera>(camera));
    }


    RenderBuffer* VulkanRender::createBufferAsync(void* data, uint32_t size, VkBufferUsageFlags usage)
    {
        VulkanBuffer* buffer = new VulkanBuffer();
        VkDeviceSize bufferSize = size;
        buffer->size = bufferSize;
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        _createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

        void* _data;
        vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &_data);
        memcpy(_data, data, (size_t)bufferSize);
        vkUnmapMemory(device, stagingBufferMemory);

        _createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | usage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, buffer->self, buffer->memory);

        auto commandBuffer = concurrentCommandPools_TransferQueue[getCurrentThreadIndex()]->createCommandBuffer(
            CommandBuffer::Type::Primary, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
        commandBuffer.startRecord();

        VkBufferCopy copyRegion{};
        copyRegion.size = bufferSize;
        vkCmdCopyBuffer(commandBuffer, stagingBuffer, buffer->self, 1, &copyRegion);
        commandBuffer.endRecord();
        commandBuffer.onCompleted([=] {
            vkDestroyBuffer(device, stagingBuffer, nullptr);
            vkFreeMemory(device, stagingBufferMemory, nullptr);
            });
        addBufferToQueue(commandBuffer);

        return buffer;
    }


    VkDevice VulkanRender::getDevice()
    {
        return device;
    }

    VulkanRender::~VulkanRender()
    {

        cleanUpDefault();
    }

    VkQueue VulkanRender::getGraphicsQueue()
    {
        return graphicsQueue;
    }

    void VulkanRender::createAllDescriptorLayouts()
    {
        createObjectDescriptorSetLayout();
        createCameraDescriptorSetLayout();
        createLightDescriptorSetLayout();
        createTextureDescriptorSetLayout();
    }
    void VulkanRender::createTextureDescriptorSetLayout()
    {
        VkDescriptorSetLayout layout;
        std::array<VkDescriptorSetLayoutBinding, 2> textureLayoutBinding{};
        textureLayoutBinding[0].binding = 0;
        textureLayoutBinding[0].descriptorCount = 1;
        textureLayoutBinding[0].descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
        textureLayoutBinding[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        textureLayoutBinding[1].binding = 1;
        textureLayoutBinding[1].descriptorCount = 1;
        textureLayoutBinding[1].descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
        textureLayoutBinding[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(textureLayoutBinding.size());
        layoutInfo.pBindings = textureLayoutBinding.data();

        if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &layout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor set layout!");
        }
        layouts[DescriptorLayoutType::Attachments] = layout;
    }
    void VulkanRender::create(BaseWindow* window)
    {
        std::system("C:/.BEbraEngine/src/shaders/sh.bat");

        factory = std::unique_ptr<VulkanRenderObjectFactory>(new VulkanRenderObjectFactory());

        this->window = dynamic_cast<VulkanWindow*>(window);
        auto size = this->window->getDrawableSize();// * (1/2.f);
        this;
        currentRenderResolution = { static_cast<uint32_t>(size.x),static_cast<uint32_t>(size.y) };
        createInstance();
        this->window->vulkan_CreateSurface(VulkanRender::instance, &surface);
        pickPhysicalDevice();
        createLogicalDevice();
        setupDebugMessenger();
        createSwapChain(currentRenderResolution.width, currentRenderResolution.height);
        createDepthResources();
        createImageViews();

        createRenderPass();

        //SetDescriptorLayouts();
        createAllDescriptorLayouts();

        createGraphicsPipeline();
        // createLineGraphicsPipeline();
        createFramebuffers();
        //createCommandPool();

        createCmdBuffers();
        createSyncObjects();
        FamilyIndices = findQueueFamilies(physicalDevice);

        createPools();
        createAttachmentsSet();
        concurrentCommandPools_RenderQueue.resize(tbb::this_task_arena::max_concurrency());
        concurrentCommandPools_TransferQueue.resize(tbb::this_task_arena::max_concurrency());
        for (int i = 0; i < concurrentCommandPools_RenderQueue.size(); i++) {
            concurrentCommandPools_RenderQueue[i] = std::make_unique<CommandPool>();
            concurrentCommandPools_TransferQueue[i] = std::make_unique<CommandPool>();
            concurrentCommandPools_RenderQueue[i]->create(FamilyIndices.graphicsFamily.value());
            concurrentCommandPools_TransferQueue[i]->create(FamilyIndices.graphicsFamily.value());
        }
        executeQueues_Objects.setStrategy(ExecuteType::Single);
    }

    RenderBuffer* VulkanRender::createIndexBuffer(std::vector<uint32_t> indices)
    {
        return createBufferAsync(indices.data(), sizeof(indices[0]) * indices.size(), VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
    }

    VkDescriptorSet VulkanRender::createDescriptor(LightDescriptorInfo* info)
    {
        VkDescriptorSet set{};
        auto opt_set = lightPool->get();
        if (opt_set.has_value()) {
            set = opt_set.value();

            auto vkbuffer = static_cast<VulkanBuffer*>(info->bufferView->buffer.get());

            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = vkbuffer->self;
            bufferInfo.offset = info->bufferView->offset;
            bufferInfo.range = vkbuffer->size;

            std::array<VkWriteDescriptorSet, 1> descriptorWrites{};

            descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[0].dstSet = set;
            descriptorWrites[0].dstBinding = 0;
            descriptorWrites[0].dstArrayElement = 0;
            descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

            descriptorWrites[0].descriptorCount = 1;
            descriptorWrites[0].pBufferInfo = &bufferInfo;

            vkUpdateDescriptorSets(getDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);

        }
        else
            Debug::log("DescriptorPool is empty (Lights)",
                VulkanRenderBufferPool.get(),
                "DescriptorPool", Debug::ObjectType::DescriptorPool, Debug::MessageType::Error);
        

        return set;
    }
    VkDescriptorSet VulkanRender::createDescriptor(RenderBuffer* buffer)
    {
        VkDescriptorSet set{};
        auto opt_set = cameraPool->get();
        if (opt_set.has_value()) {
            set = opt_set.value();

            auto _buf = static_cast<VulkanBuffer*>(buffer);
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = _buf->self;
            bufferInfo.offset = 0;
            bufferInfo.range = _buf->size;

            std::array<VkWriteDescriptorSet, 1> descriptorWrites{};

            descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[0].dstSet = set;
            descriptorWrites[0].dstBinding = 0;
            descriptorWrites[0].dstArrayElement = 0;
            descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            descriptorWrites[0].descriptorCount = 1;
            descriptorWrites[0].pBufferInfo = &bufferInfo;

            vkUpdateDescriptorSets(getDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
        }
        else
            Debug::log("DescriptorPool is empty (Camera)",
                VulkanRenderBufferPool.get(),
                "DescriptorPool", Debug::ObjectType::DescriptorPool, Debug::MessageType::Error);
        return set;
    }

    IRenderObjectFactory* VulkanRender::getRenderObjectFactory()
    {
        return factory.get();
    }
    
    void VulkanRender::addGlobalLight(std::shared_ptr<DirectionLight> globalLight)
    {
        this->globalLight = std::static_pointer_cast<VulkanDirLight>(globalLight);
    }
    VkDescriptorSet VulkanRender::createDescriptor(VulkanDescriptorSetInfo* info)
    {
        VkDescriptorSet set{};
        auto opt_set = VulkanRenderBufferPool->get();
        if (opt_set.has_value()) {
            set = opt_set.value();

            auto vkbuffer = static_cast<VulkanBuffer*>(info->bufferView->buffer.get());
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = vkbuffer->self;
            bufferInfo.offset = info->bufferView->offset;
            bufferInfo.range = info->bufferView->availableRange;

            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = info->imageView;
            imageInfo.sampler = info->sampler;

            std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

            auto types = VulkanRenderBufferPool->getInfo().types;
            descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[0].dstSet = set;
            descriptorWrites[0].dstBinding = 0;
            descriptorWrites[0].dstArrayElement = 0;
            descriptorWrites[0].descriptorType = types[0].type;
            descriptorWrites[0].descriptorCount = 1;
            descriptorWrites[0].pBufferInfo = &bufferInfo;

            descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[1].dstSet = set;
            descriptorWrites[1].dstBinding = 1;
            descriptorWrites[1].dstArrayElement = 0;
            descriptorWrites[1].descriptorType = types[1].type;
            descriptorWrites[1].descriptorCount = 1;
            descriptorWrites[1].pImageInfo = &imageInfo;
            vkUpdateDescriptorSets(getDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
        }
        else
            Debug::log("DescriptorPool is empty (RenderObject)", 
                VulkanRenderBufferPool.get(), 
                "DescriptorPool", Debug::ObjectType::DescriptorPool, Debug::MessageType::Error);

       

        return set;
    }
    void VulkanRender::freeDescriptor(VulkanRenderObject* set)
    {
        VulkanRenderBufferPool->free(set->descriptor);
    }
    void VulkanRender::freeDescriptor(VulkanDirLight* set)
    {
        lightPool->free(set->descriptor);
    }
    void VulkanRender::freeDescriptor(VulkanPointLight* set)
    {
        lightPool->free(set->descriptor);
    }
    void VulkanRender::destroyTexture(VulkanTexture* texture)
    {
        vkFreeMemory(device, texture->memory, 0);
        vkDestroyImage(device, texture->image, 0);
        vkDestroySampler(device, texture->sampler, 0);
        vkDestroyImageView(device, texture->imageView, 0);
        texture->memory = 0;
        texture->image = 0;
        texture->sampler = 0;
        texture->imageView = 0;
    }
    void VulkanRender::destroyBuffer(RenderBuffer* buffer)
    {
        auto vkBuffer = static_cast<VulkanBuffer*>(buffer);
        vkDestroyBuffer(device, vkBuffer->self, 0);
        vkFreeMemory(device, vkBuffer->memory, 0);
        delete buffer;
    }
    RenderBuffer* VulkanRender::createStorageBuffer(uint32_t size)
    {
        VulkanBuffer* buffer = new VulkanBuffer();
        buffer->size = size;
        _createBuffer(size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, buffer->self, buffer->memory);
        return buffer;
    }
    RenderBuffer* VulkanRender::createUniformBuffer(uint32_t size)
    {
        VulkanBuffer* buffer = new VulkanBuffer();
        buffer->size = size;
        _createBuffer(size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, buffer->self, buffer->memory);
        return buffer;
    }
    RenderBuffer* VulkanRender::createVertexBuffer(std::vector<Vertex> vertices)
    {
        return createBufferAsync(vertices.data(), sizeof(vertices[0]) * vertices.size(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    }
    void VulkanRender::addObject(std::shared_ptr<RenderObject> object)
    {
        executeQueues_Objects.addTask([=]() {
            objects.push_back(std::static_pointer_cast<VulkanRenderObject>(object));
            });
    }
    void VulkanRender::addLight(std::shared_ptr<PointLight> light)
    {
        executeQueues_Objects.addTask([=]() {
            lights.push_back(std::static_pointer_cast<VulkanPointLight>(light));
            });
    }

    void VulkanRender::removeObject(std::shared_ptr<RenderObject> object)
    {

        executeQueues_Objects.addTask([=]() {
            auto item = std::remove(objects.begin(), objects.end(), object);
            factory->destroyObject(object);
            objects.erase(item);
            
            });
    }

    void VulkanRender::removeLight(std::shared_ptr<PointLight> light)
    {
        executeQueues_Objects.addTask([=]() {
            auto item = std::remove(lights.begin(), lights.end(), light);
            lights.erase(item);
            });
    }
    VulkanRender::VulkanRender() {}


}