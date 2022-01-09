#pragma once
#include "stdafx.h"
#include "CommandBuffer.hpp"


//TODO: ���� ��� ����������, �� �����.
namespace BEbraEngine {
    class CommandPool final {

    public:

        operator VkCommandPool() {
            return pool;
        }

        CommandPool() {}

        void create(uint32_t queueFamilyIndex);

        CommandBuffer createCommandBuffer(CommandBuffer::Type type, VkCommandBufferUsageFlagBits bits);

        void destroyCmdBuffer(VkCommandBuffer buffer);

        void destroyCmdBuffers(std::vector<VkCommandBuffer> buffers);

        ~CommandPool();
    private:

        VkCommandPool pool{};

        size_t countBuffers{};
    };
}


