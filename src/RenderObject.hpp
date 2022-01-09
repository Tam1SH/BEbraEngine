#pragma once
#include "stdafx.h"
#include "GameComponent.hpp"
#include "RenderBuffer.hpp"
#include "IReusable.hpp"
#include "Model.hpp"
#include "matrix.hpp"
#include "Debug.hpp"
namespace BEbraEngine {
    class Transform;
    class Texture;
}

//TODO: draw is bad bleat.
namespace BEbraEngine {


    class PointLight : public GameComponent, public IReusable {
    public:
        struct ShaderData {
            alignas(16) Vector3 position;

            //���������� �����
            alignas(16) Vector3 ambient;
            alignas(16) Vector3 diffuse;
            alignas(16) Vector3 specular;

            //TODO: ��� �������, ��� ����� ������� �� ������ ������ ������� ��������� �����
            alignas(4) float constant;
            alignas(4) float linear;
            alignas(4) float quadratic;
        };
        
        struct PointLightCreateInfo {
            const Vector3 lightColor{ 1 };
            float constant{};
            float linear{};
            float quadratic{};
        };
    public:

        void destroy(IVisitorGameComponentDestroyer* destroyer) override;

        std::weak_ptr<RenderBufferView> data;

        std::weak_ptr<Transform> transform;

        void release() override;

        void setColor(const Vector3& color) {

            this->color = color;
        }
        Vector3& getColor() {
            return color;
        }
        void update();


        PointLight() { name = "Light"; }
        virtual ~PointLight() {}
    private:
        Vector3 color;

    };


    class DirectionLight : public GameComponent {
    public:
        struct ShaderData {
            alignas(16) Vector3 direction;

            //���������� �����
            alignas(16) Vector3 ambient;
            alignas(16) Vector3 diffuse;
            alignas(16) Vector3 specular;

        };
        struct DirectionLightCreateInfo {
            const Vector3 lightColor{ 1 };
        };


    public:
        std::weak_ptr<RenderBufferView> data;

        void setColor(const Vector3& color) {

            this->color = color;
        }
        void setDirection(const Vector3& direction) {
            _direction = direction;
        }
        void update() {
            ShaderData data1;
            if (!data.expired()) {
                data1.direction = _direction;
                data1.ambient = color;
                data1.diffuse = color;
                data1.specular = color;
                data.lock()->setData(&data1, sizeof(ShaderData));
            }
            else
                Debug::log(getName() + " has invalid data");

        }
        Vector3& getDirection() {
            return _direction;
        }
        Vector3& getColor() {
            return color;
        }

        DirectionLight() { name = "DirectionLight"; }
        virtual ~DirectionLight() {}

    private:
        Vector3 color;
        Vector3 _direction;

        // ������������ ����� GameObjectComponent
        virtual void destroy(IVisitorGameComponentDestroyer* destroyer) override;
    };

    class RenderObject : public GameComponent, public IReusable {
        DEBUG_DESTROY_CHECK_DECL()
    public:
        struct ShaderData {
            Matrix4 model;
            alignas(16) Vector3 color;

        };

        struct RenderObjectCreateInfo {

            const PointLight::PointLightCreateInfo* pointLightInfo;
            const DirectionLight::DirectionLightCreateInfo* directionLightInfo;
        };
    public:


        void destroy(IVisitorGameComponentDestroyer* destroyer) override;

        //����������� �����
        std::shared_ptr<Texture> texture;

        std::shared_ptr<Model> model;

        std::shared_ptr<RenderBufferView> matrix;

        std::shared_ptr<Transform> transform;

        void update();

        virtual ~RenderObject();

        RenderObject();

        void setColor(const Vector3& color) {

            _color = color;
        }
        Vector3& getColor() {
            return _color;
        }
    private:
        Vector3 _color;
    };
}
