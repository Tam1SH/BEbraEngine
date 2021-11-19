#include "stdafx.h"
#define NOMINMAX
#include "GameObjectFactory.hpp"
#include "TransformFactory.hpp"
#include "VulkanRenderObjectFactory.hpp"
#include "GameObject.hpp"
#include "RigidBoby.hpp"
#include "RenderObject.hpp"
#include "Transform.hpp"
#include "VulkanRender.hpp"
#include "Physics.hpp"
#include "WorkSpace.hpp"
#include "AbstractRender.hpp"
namespace BEbraEngine {
	GameObjectFactory::GameObjectFactory(std::shared_ptr<AbstractRender> render, std::shared_ptr<Physics> physics)
		: render(render), physics(physics)
	{

		renderFactory = render->getRenderObjectFactory();
		renderFactory->setContext(render.get());

		colliderFactory = physics->getColliderFactory();
		rigidBodyFactory = physics->getRigidBodyFactory();
		transFactory = std::unique_ptr<TransformFactory>(new TransformFactory());

	}
	std::shared_ptr<GameObject> GameObjectFactory::create(const Vector3& position)
	{
		auto obj = std::shared_ptr<GameObject>(new GameObject());
		auto name = obj->GetName();

		obj->SetName(name + std::to_string(workspace->GetSize()));
		//workspace->addComponent(obj);
		ColliderInfo info;
		info.scale = Vector3(1);
		info.position = position;

		auto collider = std::shared_ptr<Collider>(colliderFactory->create(&info));
		auto transform = std::shared_ptr<Transform>(Transform::New(position));
		auto renderObj = std::shared_ptr<RenderObject>(renderFactory->createObject());
		auto rigidbody = std::shared_ptr<RigidBody>(rigidBodyFactory->create(collider.get()));

		renderFactory->BindTransform(renderObj, transform);

		rigidbody->SetTransform(transform);

		obj->addComponent(renderObj);
		obj->addComponent(rigidbody);
		obj->addComponent(transform);
		obj->addComponent(collider);
		btTransform trans;
		trans.setIdentity();
		trans.setOrigin(btVector3(position.x, position.y, position.z));
		rigidbody->GetRigidBody()->setWorldTransform(trans);
		render->addObject(renderObj);
		physics->addRigidBody(rigidbody);

		return obj;
	}

	std::shared_ptr<PointLight> GameObjectFactory::createLight(const Vector3& position)
	{
		auto transform = std::shared_ptr<Transform>(Transform::New(position));
		auto light = std::shared_ptr<PointLight>(renderFactory->createLight(Vector3(1), position));
		light->addComponent(transform);

		auto name = light->GetName();

		light->SetName(name + std::to_string(workspace->GetSize()));
		//workspace->addComponent(light);
		renderFactory->BindTransform(light, transform);

		light->update();
		render->addLight(light);
		return light;
	}

	std::shared_ptr<DirectionLight> GameObjectFactory::createDirLight(const Vector3& direction)
	{
		auto light = std::shared_ptr<DirectionLight>(renderFactory->createDirLight(Vector3(0.1f), direction));

		auto name = light->GetName();

		light->SetName(name + std::to_string(workspace->GetSize()));
		workspace->addComponent(light);

		render->addGlobalLight(light);
		return light;
	}

	void GameObjectFactory::setModel(GameObject* object, std::string&& path)
	{
		renderFactory->setModel(object->getComponent<RenderObject>().get(), path);
	}

	void GameObjectFactory::destroyObject(GameObject* object)
	{
		renderFactory->destroyObject(object->getComponent<RenderObject>());
		physics->removeRigidBody(object->getComponent<RigidBody>());
	}

	void GameObjectFactory::destroyObject(std::shared_ptr<GameObject> object)
	{
		auto begin = workspace->GetList().begin();
		auto end = workspace->GetList().end();

		/*
		if (workspace->GetSize() != 0) {
			workspace->GetList().erase(
				std::remove_if(begin, end,
					[&](std::shared_ptr<GameObjectComponent> component) {
						if (component.get() == object.get()) {
							std::cout << "GAMEOBJECT FOUNDED" << std::endl;
							return true;
						}
						return false;
					})
			);
		}
		*/
		renderFactory->destroyObject(object->getComponent<RenderObject>());
		physics->removeRigidBody(object->getComponent<RigidBody>());
		render->removeObject(object->getComponent<RenderObject>());
	}

	void GameObjectFactory::destroyPointLight(std::shared_ptr<PointLight> light)
	{
		light->release();
		renderFactory->destroyPointLight(light);
	}

	void GameObjectFactory::SetWorkSpace(std::shared_ptr<WorkSpace> workspace)
	{
		this->workspace = workspace;
	}

	GameObjectFactory::~GameObjectFactory()
	{
	}
}