#pragma once
#include "AbstractComponent.hpp"
#include <Physics/btBulletDynamicsCommon.h>
#include <memory>
namespace BEbraEngine {
	class Transform;
	class Physics;
	class Vector3;

}

namespace BEbraEngine {

	class RigidBody : public GameObjectComponent
	{
	public:
		friend class RigidBodyFactory;
	public:

		void resetState();

		btRigidBody* getRigidBody() { return body; }

		void setTransform(std::shared_ptr<Transform> transform);

		void setShape(btCollisionShape* newShape);

		void setDynamic(bool isActive);

		bool getDynamic() { return isActive; }

		void setPosition(const Vector3& position);

		void applyImpulse(const Vector3& force, const Vector3& direction);

		void applyImpulseToPoint(float force, const Vector3& point);

		Transform* getTransform() { return transform.get(); }

		RigidBody();

		~RigidBody();

	private:

		btRigidBody* body;

		std::shared_ptr<Transform> transform;

		btVector3 linearFactor;

		btVector3 AngularFactor;


		bool isActive = true;

	};
}

