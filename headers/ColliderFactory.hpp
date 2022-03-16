#pragma once
#include "platform.hpp"
#include "Collider.hpp"

class btCollisionShape;

using BE_STD::optional;
using BE_STD::unique_ptr;

BE_NAMESPACE_STD_BEGIN
	template<class T>
	class shared_ptr;
BE_NAMESPACE_STD_END

namespace BEbraEngine {
	class Physics;
}

namespace BEbraEngine {

	class ColliderFactory
	{
	public:
		ColliderFactory(Physics& physics);

		optional<Collider*> create(const Collider::CreateInfo& info);

		void destroyCollider(Collider& col);

		void setShape(Collider& collider, btCollisionShape& newShape);

		optional<btCollisionShape*> getShape(Collider::Type type);
	private:

		Physics* physics;
	};

}

