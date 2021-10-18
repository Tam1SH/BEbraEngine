#pragma once
#include "AbstractComponent.hpp"
#include <list>
#include "GameObject.hpp"
//TODO: �� ��� �����, � ���� ���� �� �����?
namespace BEbraEngine {
    class WorkSpace : public AbstractComponent {
    protected:
        void _addComponent(std::shared_ptr<AbstractComponent> component) override {

            Components.push_back(component);
        }
        void _removeComponent(std::shared_ptr<AbstractComponent> component) override {
            Components.remove(component);
        }
    public:
        void AddComponent(std::shared_ptr<AbstractComponent> component) {
            _addComponent(component);
            component->SetParent(this);
        }
        void RemoveComponent(std::shared_ptr<AbstractComponent> component) {
            _removeComponent(component);
            component->SetParent(nullptr);
        }
        std::list<std::shared_ptr<AbstractComponent>>& GetList() {
            return Components;
        }
        std::shared_ptr<GameObject> FindObject(GameObject* object) {
            auto _object = static_cast<AbstractComponent*>(object);
            for (auto component : Components) {
                if (_object == component.get()) {
                    return std::static_pointer_cast<GameObject>(component);
                }


            }
        }
        std::shared_ptr<GameObject> FindObject(AbstractComponent* object) {
            for (auto component : Components) {
                if (object == component.get()) {
                    return std::static_pointer_cast<GameObject>(component);
                }
            }
            throw std::exception();
        }
        std::shared_ptr<GameObject> FindObject(std::string name) {
            for (auto component : Components) {
                if (name == component->GetName()) {
                    return std::static_pointer_cast<GameObject>(component);
                }
            }
        }
        void RemoveAll() {
            Components.clear();
        }
        WorkSpace() {
            name = "Workspace";
        }
    };
}

