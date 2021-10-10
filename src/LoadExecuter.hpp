#pragma once
#include <vulkan/vulkan.h>
#include <functional>
#include <mutex>
#include "CommandBuffer.hpp"
#include <tbb.h>
#include "BaseVulkanRender.hpp"
namespace BEbraEngine {
    
    
    class LoadExecuter {
    public:

        typedef std::function<void(CommandBuffer&)> Task;

        typedef std::function<void()> Callback;
    public:

        LoadExecuter() { }

        void Create(VkDevice& device, BaseVulkanRender* render);

        void AddToQueue(Task func);

        void OnCompletedAll(Callback func);

        void OnCompletedPart(Callback func);

        void WriteAllBuffers();
    private:

        bool AllTasksCompleted();

        void ExecuteAll();

        VkCommandBuffer CreateBuffer();

    private:
        std::mutex m;

        int maxTasks;
        int totalTasks;
        std::atomic<int> taskCompleted;

        VkDevice device;
        VkCommandPool _pool;

        tbb::concurrent_queue<CommandBuffer> RecordedBuffers;
        tbb::concurrent_queue<CommandBuffer> ExecuteBuffers;
        tbb::concurrent_queue<Task> tasks;

        tbb::task_arena taskPool;

        BaseVulkanRender* render;

        Callback _OnCompletedAll;
        Callback _OnCompletedPart;
    };
    

}