﻿
//#include <angelscript.h>

//#define DECLARATE_SCRIPT_OBJECT_DEFAULT_BEHAVIOR(ObjectType)	\
//int refCount{};										\
//void addRef() { ++refCount; }			\
//int release() {						\
//		if (--refCount == 0)						\
//		{											\
//			delete this;							\
//			return 0;								\
//		}											\
//			return refCount;						\
//}													\



export module ScriptWrappers;
//import <tbb.h>;

//import <string>;
//import <memory>;
//import AngelScriptEngine;
//import ObjectFactoryFacade;
//import Vector3;
//import Input;
//import GameObject;
//import RigidBodyFactory;
//import RigidBody;
//import GameComponent;
/*
namespace BEbraEngine {
	namespace Wrappers {
		export template<typename T>
		T* defaultFactory() {
			return new T();
		}
		export template<typename T>
		void defaultRelease(T* t) {
			if (t) delete t;
		}

		export template<typename T>
		void defaultAllocate(void* memory) {
			
			new(memory)T();
		}
		export template<typename T>
		void defaultDeallocate(void* memory) { 
			((T*)memory)->~T();

		}
		export class _Input {
		public:


			static constexpr const char* __name = "_Input";

			static _Input& getInstance() {
				if (!instance) {
					instance = new _Input();
				}
				return *instance;
			}

			static void registerObj(asIScriptEngine* engine) {
				
				int r = engine->RegisterObjectType(__name, 0, asOBJ_REF); assert(r >= 0);

				r = engine->RegisterObjectBehaviour(__name,
					asBEHAVE_RELEASE, "void f()", asMETHOD(_Input, release), asCALL_THISCALL); assert(r >= 0);

				r = engine->RegisterObjectBehaviour(__name,
					asBEHAVE_ADDREF, "void f()", asMETHOD(_Input, addRef), asCALL_THISCALL); assert(r >= 0);

				r = engine->RegisterObjectMethod(__name,
					"bool __isKeyPressed(int)", asMETHODPR(_Input, isKeyPressed, (int), bool), asCALL_THISCALL); assert(r >= 0);
				// asMETHODPR(_Vector3, operator*, (float) const noexcept, _Vector3&&)

				r = engine->RegisterGlobalProperty("_Input@ input", &instance); assert(r >= 0);
				
			}
		private:								
			//DECLARATE_SCRIPT_OBJECT_DEFAULT_BEHAVIOR(_Input)
			bool isKeyPressed(int key) {
				return Input::isKeyPressed((KeyCode)key);
			}

		private:
			_Input() {}
			static _Input* instance;


		};

		_Input* _Input::instance;



		export class _Transform {

		};

		export class _RigidBody {
			std::shared_ptr<RigidBody> instance;
			static constexpr const char* __name = "RigidBody";

			static void registerObj(asIScriptEngine* engine, RigidBodyFactory* factory) {

				int r = engine->RegisterObjectType(__name,
					sizeof(RigidBody), asOBJ_REF); //assert(r >= 0);

				//r = engine->RegisterObjectBehaviour(__name, asBEHAVE_FACTORY, "RigidBody@ f()", asFUNCTION(RigidBodyFactory::create), asCALL_CDECL); assert(r >= 0);

			}

		};

		//template<typename T> 
		//class proxyGameObjectComponent : std::enable_if<std::is_base_of<GameObjectComponent, T>> {
//
		//};


		export class _Vector3 {
		public:
			static constexpr const char* __name = "Vector3";

			Vector3 instance{};

			static _Vector3* create(float& x, float& y, float& z) {
				auto vec = new _Vector3();
				vec->instance = { x,y,z };
				return vec;
			}

			static _Vector3* create(float& x, float& y) {
				auto vec = new _Vector3();
				vec->instance = { x,y,0 };
				return vec;
			}

			static _Vector3* create(float& all) {
				auto vec = new _Vector3();
				vec->instance = { all };
				return vec;
			}

			static void registerObj(asIScriptEngine* engine) {
				
				int r = engine->RegisterObjectType(__name,
					sizeof(_Vector3), asOBJ_REF | asOBJ_SCOPED); assert(r >= 0);

				r = engine->RegisterObjectBehaviour(__name,
					asBEHAVE_FACTORY, "Vector3@ f(float& in, float& in, float& in)", asFUNCTIONPR(create, (float& x,float& y, float& z), _Vector3*), asCALL_CDECL); assert(r >= 0);
				
				r = engine->RegisterObjectBehaviour(__name,
					asBEHAVE_FACTORY, "Vector3@ f(float& in, float& in)", asFUNCTIONPR(create, (float& x, float& y), _Vector3*), asCALL_CDECL); assert(r >= 0);
				
				r = engine->RegisterObjectBehaviour(__name,
					asBEHAVE_FACTORY, "Vector3@ f(float& in)", asFUNCTIONPR(create, (float& all), _Vector3*), asCALL_CDECL); assert(r >= 0);



				r = engine->RegisterObjectBehaviour(__name,
					asBEHAVE_RELEASE, "void f()", asFUNCTION(defaultRelease<_Vector3>), asCALL_CDECL_OBJLAST); assert(r >= 0);
				
				r = engine->RegisterObjectMethod(__name, 
					"Vector3& opAdd(const Vector3& in)", asMETHOD(_Vector3, operator+), asCALL_THISCALL); assert(r >= 0);
				
				r = engine->RegisterObjectMethod(__name,
					"Vector3& opSub(const Vector3& in)", asMETHOD(_Vector3, operator-), asCALL_THISCALL); assert(r >= 0);
				
				r = engine->RegisterObjectMethod(__name,
					"Vector3& opMul(const Vector3& in)", asMETHODPR(_Vector3, operator*, (const _Vector3&) const noexcept, _Vector3&&), asCALL_THISCALL); assert(r >= 0);
				
				r = engine->RegisterObjectMethod(__name,
					"Vector3& opMul(float)", asMETHODPR(_Vector3, operator*, (float) const noexcept, _Vector3&&), asCALL_THISCALL); assert(r >= 0);

				r = engine->RegisterObjectMethod(__name,
					"Vector3& opAddAssign(const Vector3& in)", asMETHOD(_Vector3, operator+=), asCALL_THISCALL); assert(r >= 0);
				
				r = engine->RegisterObjectMethod(__name,
					"Vector3& opMulAssign(const Vector3& in)", asMETHOD(_Vector3, operator*=), asCALL_THISCALL); assert(r >= 0);

				r = engine->RegisterObjectMethod(__name,
					"Vector3& opSubAssign(const Vector3& in)", asMETHOD(_Vector3, operator-=), asCALL_THISCALL); assert(r >= 0);


				r = engine->RegisterObjectProperty(__name,
					"float x", asOFFSET(Vector3, x), asOFFSET(_Vector3, instance), false); assert(r >= 0);

				r = engine->RegisterObjectProperty(__name,
					"float y", asOFFSET(Vector3, y), asOFFSET(_Vector3, instance), false); assert(r >= 0);

				r = engine->RegisterObjectProperty(__name,
					"float z", asOFFSET(Vector3, z), asOFFSET(_Vector3, instance), false); assert(r >= 0);
				
			}

			_Vector3(const _Vector3& vec) {
				instance = vec.instance;
			}

			_Vector3(const Vector3& vec) {
				instance = vec;
			}
			_Vector3(const Vector3&& vec) {

				instance = vec;
			}
			_Vector3() {}

			_Vector3&& operator+(_Vector3& other) const noexcept {

				return std::move(_Vector3(instance + other.instance));
			}

			_Vector3&& operator-(const _Vector3& other) const noexcept {

				return std::move(_Vector3(instance - other.instance));
			}

			_Vector3&& operator*(const _Vector3& other) const noexcept {

				return std::move(_Vector3(instance * other.instance));
			}

			_Vector3&& operator*(float scalar) const noexcept {

				return std::move(_Vector3(instance * scalar));
			}

			_Vector3* operator*=(const _Vector3& other) noexcept {
				*this = instance * other.instance;
				return this;
			}

			_Vector3* operator+=(const _Vector3& other) noexcept {
				*this = instance + other.instance;
				return this;
			}

			_Vector3* operator-=(const _Vector3& other) noexcept {
				*this = instance - other.instance;
				return this;
			}
		};


		export class _GameObjectComponent {
		public:

			static constexpr const char* __name = "GameObjectComponent";

			GameComponent* instance;

			static void registerObj(asIScriptEngine* engine) {
			//	int r = engine->RegisterObjectType(__name, 0, asOBJ_REF); assert(r >= 0);

			}
		private:
			//DECLARATE_SCRIPT_OBJECT_DEFAULT_BEHAVIOR(_GameObjectComponent)
		};

		export class _GameObject {
		public:

			static ObjectFactoryFacade* factory;

			static void registerObj(asIScriptEngine* engine) {
				
				int r = engine->RegisterObjectType(__name, 0, asOBJ_REF); assert(r >= 0);

				r = engine->RegisterObjectBehaviour(__name,
					asBEHAVE_FACTORY, "GameObject@ f(const Vector3& in)", asFUNCTION(gameObjectFactory), asCALL_CDECL); assert(r >= 0);

				r = engine->RegisterObjectBehaviour(__name,
					asBEHAVE_RELEASE, "void f()", asMETHOD(_GameObject, release1), asCALL_THISCALL); assert(r >= 0);

				r = engine->RegisterObjectBehaviour(__name,
					asBEHAVE_ADDREF, "void f()", asMETHOD(_GameObject, addRef), asCALL_THISCALL); assert(r >= 0);

			////	r = engine->RegisterObjectMethod(__name,
			//		"GameObjectComponent@ getComponentByName(const string& in)", asMETHOD(_GameObject, getComponentByName), asCALL_THISCALL); assert(r >= 0);
				
			}
		private:
		
			_GameObjectComponent* getComponentByName(const std::string& in) {
				
				auto _comp = instance->getComponentByName(in);
				if (_comp) {
					auto comp = new _GameObjectComponent();
					comp->instance = _comp;
					return comp;
				}
				else
					return 0;
			}
			_GameObject(const _Vector3& position) {
				try {
					instance = factory->create(position.instance);
				}
				catch (const std::exception& ex) {
					int s;
					int d = std::move(s);
					__LINE__;
					__FILE__;
					__func__;
				}
				//DEBUG_LOG("create script game object");

			}
			//DECLARATE_SCRIPT_OBJECT_DEFAULT_BEHAVIOR(_GameObject)

				std::shared_ptr<GameObject> instance;

			static constexpr const char* __name = "GameObject";


			int refCount;

			static _GameObject* gameObjectFactory(const _Vector3& in) {
				auto o = new _GameObject(in);
				o->refCount = 1;
				return o;
			}

			int release1() {
				if (--refCount == 0)
				{
					if (instance.get()) {
						factory->destroy(instance);
					}

					delete this;
					return 0;
				}
				return refCount;

			}
			void create() {

			}




		};
		ObjectFactoryFacade* _GameObject::factory;
	}
}
*/