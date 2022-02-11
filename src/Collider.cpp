#include "stdafx.h"
#include "Collider.hpp"

#include <Physics/btBulletCollisionCommon.h>
#include "IVisitorGameComponentDestroyer.hpp"
#include "Debug.hpp"
namespace BEbraEngine {
	void Collider::setScale(const Vector3& size)
	{
		this->size = size;
		_collider->getCollisionShape()->setLocalScaling(size);
	}
	void Collider::setPosition(const Vector3& position)
	{
		btTransform trans;
		trans.setOrigin(size);
		_collider->setWorldTransform(trans);
	}
	void Collider::setMass(float mass)
	{
		//TODO: ��� ��� ����� ������?
		auto vec = btVector3(0,0,0);
		_collider->getCollisionShape()->calculateLocalInertia(mass, vec);
	}
	Vector3 Collider::getPosition() const
	{

		return _collider->getWorldTransform().getOrigin();
	}
	Collider::Collider()
	{
		name_ = "Collider";
	}
	Collider::~Collider()
	{
		DEBUG_DESTROY_CHECK("Collider has not destroyed", this, "", Debug::ObjectType::Collider, Debug::MessageType::Info);
	}
	void Collider::destroy(IVisitorGameComponentDestroyer& destroyer)
	{
		destroyer.destroyColliderComponent(*this);
	}
}

