#pragma once

#include "stdafx.h"
#include "AbstractRender.hpp"

#include "Physics.hpp"
#include "BaseRenderWindow.hpp"
#include "ScriptState.hpp"
#include "VulkanRender.hpp"
#include "VulkanWindow.hpp"
#include "Time.hpp"
#include "Input.hpp"
using std::unique_ptr;

namespace BEbraEngine {


    class Engine {
    public:
        unique_ptr<AbstractRender> render1;
        unique_ptr<Physics> physics;
        unique_ptr<BaseWindow> window1;
        unique_ptr<ScriptState> gameLogic1;
        bool multiThreading = false;
    public:
        void Main() {
            init();
            start();
        }
        void init() {
            render1 = unique_ptr<VulkanRender>(new VulkanRender());
            window1 = unique_ptr<VulkanWindow>(new VulkanWindow(render1.get()));
            window1->createWindow(Vector2(1000, 1000), "BEEEBRA!!!");
            physics = unique_ptr<Physics>(new Physics());
            auto drawer = new DebugDrawer(*render1);
            physics->setDebugDrawer(drawer);
            gameLogic1 = unique_ptr<ScriptState>(new ScriptState(*render1, *physics));
            Debug::enableAll();
        }
        bool physicsEnabled = true;
        void start() {

            while (!window1->isClose()) {
                Time::updateTime();
                if (multiThreading) {


                    tbb::flow::graph g;
                    tbb::flow::broadcast_node<tbb::flow::continue_msg> input(g);
                    tbb::flow::continue_node<tbb::flow::continue_msg>
                        _window(g, [&](const tbb::flow::continue_msg&) { window1->update(); });
                    tbb::flow::continue_node<tbb::flow::continue_msg>
                        _render(g, [&](const tbb::flow::continue_msg&) { render1->update(); });
                    tbb::flow::continue_node<tbb::flow::continue_msg>
                        _physics(g, [&](const tbb::flow::continue_msg&) { physics->update(); });
                    tbb::flow::continue_node<tbb::flow::continue_msg>
                        _gameLogic(g, [&](const tbb::flow::continue_msg&) { gameLogic1->update(); });


                    tbb::flow::make_edge(input, _physics);
                    tbb::flow::make_edge(input, _render);
                    tbb::flow::make_edge(input, _gameLogic);
                    tbb::flow::make_edge(input, _window);
                    input.try_put(tbb::flow::continue_msg());
                    g.wait_for_all();
                   
                    gameLogic1->updateState();
                    render1->drawFrame();
                }
                else {


                    window1->update();

                    
                     physics->update();
                    
                    render1->update();
                    gameLogic1->update();
                    gameLogic1->updateState();
                    render1->drawFrame();
                }

            }


            //vkDeviceWaitIdle(VulkanRender::device);

        }

        ~Engine() {
            gameLogic1.reset();
            //  render1.reset();
            //  UId->Destroy();
        }
    };
}