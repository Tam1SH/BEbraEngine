#include "stdafx.h"
#include "RenderObject.hpp"
#include "Transform.hpp"
#include "Image.hpp"
#include "VkBuffer.hpp"
#include "VulkanRenderObjectFactory.hpp"
namespace BEbraEngine {
    void RenderObject::update()
    {
        ShaderData data;
        data.model = transform->getBasis();
        data.color = color;
        matrix->setData(&data, sizeof(ShaderData));
    }

    RenderObject::~RenderObject()
    {
#ifdef _DEBUG
        Debug::log("Destroy", this, "", Debug::ObjectType::RenderObject, Debug::MessageType::Info);
#endif 
    }

    RenderObject::RenderObject()
    {
#ifdef _DEBUG
        Debug::log("Create", this, "", Debug::ObjectType::RenderObject, Debug::MessageType::Info);
#endif 
    }

    

    void PointLight::release()
    {
        ShaderData _data;
        if (!data.expired()) {
            _data.position = Vector3();
            _data.ambient = Vector3();
            _data.diffuse = Vector3();
            _data.specular = Vector3();
            _data.constant = 1.0f;
            _data.linear = 0;
            _data.quadratic = 0;
            data.lock()->setData(&_data, sizeof(ShaderData));
        }
    }

    void PointLight::update()
    {
        ShaderData _data;
        if (!transform.expired() && !data.expired()) {
            _data.position = transform.lock()->getPosition();
            _data.ambient = color;
            _data.diffuse = color;
            _data.specular = color;
            _data.constant = 1.f;
            _data.linear = 0.022f;
            _data.quadratic = 0.0019f;
            data.lock()->setData(&_data, sizeof(ShaderData));
        }
        else
            Debug::log(getName() + " has invalid data");
    }
}