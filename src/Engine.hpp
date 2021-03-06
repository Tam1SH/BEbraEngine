
#include <tbb.h>
#include "Physics.hpp"
import BaseRenderWindow;
import Debug;
import ScriptState;
import VulkanRender;
import VulkanWindow;
import Time;
import Input;

using std::unique_ptr;

namespace BEbraEngine {

    struct EngineCreateInfo {
        bool Debug;
        bool Test;
    };

    class Engine {
    public:
        unique_ptr<AbstractRender> render1;
        unique_ptr<Physics> physics;
        unique_ptr<BaseWindow> window1;
        unique_ptr<ScriptState> gameState;
        bool multiThreading = true;
    public:
        void Main(const EngineCreateInfo& info) {
            init(info);
            if(!info.Test)
                start();
        }
        void init(const EngineCreateInfo& info) {
            render1 = unique_ptr<VulkanRender>(new VulkanRender());
            window1 = unique_ptr<VulkanWindow>(new VulkanWindow(render1.get()));
            window1->createWindow(Vector2(1000, 1000), "BEEEBRA!!!");
            physics = unique_ptr<Physics>(new Physics());
            auto drawer = new DebugDrawer(*render1);
            physics->setDebugDrawer(drawer);
            gameState = unique_ptr<ScriptState>(new ScriptState(*render1, *physics, *window1));
            Debug::enableAll();
        }
        
        void start() {
            
            while (!window1->isClose()) {
                Time::updateTime();
                if (multiThreading) {

                    window1->update();
                    render1->prepareDraw();

                    tbb::flow::graph g;
                    tbb::flow::broadcast_node<tbb::flow::continue_msg> input(g);
                    tbb::flow::continue_node<tbb::flow::continue_msg>
                        _window(g, [&](const tbb::flow::continue_msg&) {  });
                    tbb::flow::continue_node<tbb::flow::continue_msg>
                        _render(g, [&](const tbb::flow::continue_msg&) { render1->update(); });
                    tbb::flow::continue_node<tbb::flow::continue_msg>
                        _physics(g, [&](const tbb::flow::continue_msg&) { physics->update(); });
                    tbb::flow::continue_node<tbb::flow::continue_msg>
                        _gameLogic(g, [&](const tbb::flow::continue_msg&) { gameState->update(); });


                    tbb::flow::make_edge(input, _physics);
                    tbb::flow::make_edge(input, _render);
                    tbb::flow::make_edge(input, _gameLogic);
                    tbb::flow::make_edge(input, _window);
                    input.try_put(tbb::flow::continue_msg());
                    g.wait_for_all();
                   
                    gameState->updateState();
                    render1->drawFrame();

                }
                else {


                    window1->update();

                    
                     physics->update();
                    
                    render1->update();
                    gameState->update();
                    gameState->updateState();
                    render1->drawFrame();

                    render1->prepareDraw();
                }

            }
            

            //vkDeviceWaitIdle(VulkanRender::device);

        }

        ~Engine() {
            gameState.reset();
            render1.reset();
        }
    };
}