#pragma once
#include "AbstractComponent.hpp"
#include <vulkan/vulkan.h>
#include "Vector3.hpp"
#include "matrix.hpp"
#include "RenderBuffer.hpp"
namespace BEbraEngine {
    class TransformFactory;
    class Matrix4;

}
namespace BEbraEngine {

    class Transform : public GameObjectComponent {
    private:
        friend class TransformFactory;

        friend class VulkanRenderObjectFactory;

        static TransformFactory* factory;

        std::shared_ptr<RenderBuffer> buffer;

        std::shared_ptr<RenderBufferView> _buffer;
    private:
        Vector3 position;

        Vector3 scale;

        Vector3 rotation;

        Matrix4 model;
    private:

        static void SetFactory(TransformFactory* factory) noexcept;

        Transform(const Vector3& position);

    public:

        static Transform* New(const Vector3& position = Vector3(0));

        void UpdatePosition(const Vector3& position);

        void SetPosition(const Vector3& position) noexcept;

        void SetScale(const Vector3& scale) noexcept;

        Vector3 GetPosition() const noexcept;

        ~Transform() { };
    };
}