#pragma once
#include <memory>
#include "RenderObjectCreator.hpp"


namespace BEbraEngine {
    class VulkanRender;
    class WorkSpace;
    class Physics;
    class GameObjectFactory;
    class ScriptManager;
    class Render;
    class Camera;
    class Time;
    class GameLogic {
    private:
        std::shared_ptr<Render> render;
        std::shared_ptr<Physics> physics;
        std::shared_ptr<WorkSpace> workspace;
        std::shared_ptr<ScriptManager> scriptManager;

        std::unique_ptr<GameObjectFactory> objectFactory;
        std::shared_ptr<Camera> camera;
        RenderObjectFactory creator;
    public:

        GameLogic(VulkanRender* render, std::shared_ptr<WorkSpace> workspace);

        void ScriptInit();

        void Update();

        ~GameLogic();

    };
}
