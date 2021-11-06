#include "stdafx.h"
#define NOMINMAX
#include "Transform.hpp"
#include "GameObject.hpp"
#include "RigidBoby.hpp"
#include "TransformFactory.hpp"
#include "btBulletDynamicsCommon.h"
#include "VulkanRender.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include<glm/gtc/quaternion.hpp>
#include "VkBuffer.hpp"
#include "RenderBuffer.hpp"
namespace BEbraEngine {
    TransformFactory* Transform::factory;

    void Transform::SetFactory(TransformFactory* factory) noexcept 
    {
        Transform::factory = factory;
    }

    Transform* Transform::New(const Vector3& position)
    {
        return factory->Create(position);
    }

    Matrix4 Transform::getMatrix()
    {
        auto model = glm::mat4(1);
        glm::vec3 _s = scale;
        glm::vec3 _p = position;
        model = glm::translate(model, _p);
        model = glm::scale(model, _s);
        model *= glm::toMat4(quartion);

        return Matrix4(model);
    }

    Transform::Transform(const Vector3& position)
    {
        this->position = position;
        scale = Vector3(1);
        rotation = Vector3(1);
    }

    void Transform::UpdatePosition(const Vector3& position, const Vector4& quat)
    {
        this->position = position;

        auto model = glm::mat4(1);
        glm::vec3 _s = scale;
        glm::vec3 _p = position;
        quartion.x = quat.x;
        quartion.y = quat.y;
        quartion.z = quat.z;
        quartion.w = quat.w;
        //model = glm::translate(model, _p);
        //model = glm::scale(model, _s);
        //model *= glm::toMat4(quartion);
        //buffer->setData(&model, sizeof(Matrix4));
    }

    void Transform::SetPosition(const Vector3& position) noexcept
    {
        this->position = position;
    }


    void Transform::SetScale(const Vector3& scale) noexcept
    {
        this->scale = scale;
    }

    Vector3 Transform::GetPosition() const noexcept
    {
        return position;
    }
}
