#pragma once
#include "AbstractComponent.hpp"
#include "Vector3.hpp"
#include "Debug.hpp"
class btCollisionObject;

namespace std {
	template<typename T, class D>
	class unique_ptr;

}
namespace BEbraEngine {

	class Collider : public GameObjectComponent { DEBUG_DESTROY_CHECK_DECL()
	public:
		friend class ColliderFactory;
	public:
		btCollisionObject* get() { return _collider.get(); }

		void setScale(const Vector3& size);

		Vector3& getSize() {
			return size;
		}
		void setPosition(const Vector3& size);

		Vector3 getPosition();

		Collider();

		~Collider();

	private:
		std::unique_ptr<btCollisionObject> _collider;
		Vector3 size;
	};
}

