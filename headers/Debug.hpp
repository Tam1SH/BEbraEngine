#pragma once
#include "stdafx.h"
#include "platform.hpp"

using BE_STD::vector;
using BE_STD::string;

#define DEBUG_LOG1(msg) BEbraEngine::Debug::log(__LINE__, __FILE__, msg);
#define DEBUG_LOG2(msg, pointer, name, oType, mType) BEbraEngine::Debug::log(__LINE__, __FILE__, msg, pointer, name, oType, mType)
#define DEBUG_LOG3(msg, pointer) BEbraEngine::Debug::log(__LINE__, __FILE__, msg, pointer);

#ifdef _DEBUG
#define DEBUG_DESTROY_CHECK_DECL() public: bool isDestroyed; private:
#define DEBUG_DESTROY_CHECK(msg, pointer, name, oType, mType) if(!isDestroyed) BEbraEngine::Debug::log(__LINE__, __FILE__, msg, pointer, name, oType, mType)	

#else
#define DEBUG_DESTROY_CHECK(msg, pointer, name, oType, mType) 
#define DEBUG_DESTROY_CHECK_DECL()
#endif
namespace BEbraEngine {




	class Debug
	{
	public:
		enum class ObjectType {
			Empty,
			GameObject,
			RenderObject,
			Transform,
			Collider,
			RigidBody,
			Script,
			Model,
			Buffer,
			BufferPool,
			DescriptorPool,
			MaxValue
		};
		enum class MessageType {
			Error,
			Info
		};
	public:
		static void enableAll();

		static void disableAll();

		static void disableLog(ObjectType type);

		static void enableLog(ObjectType type);

		static void log(int line, const char* nameFile, const std::stringstream& stream);

		static void log(const std::string& text);

		static void log(int line, const char* nameFile, const std::string&& text);


		static void log(int line, const char* nameFile,
			const std::string& text, const void* pointer = {}, const std::string& name = {}, ObjectType oType = {}, MessageType mType = MessageType::Info);

		static void log(const std::string& text, const void* handle, const std::string& name, ObjectType oType, MessageType mType);

	private:
		static void _log(const std::string& str);


		static std::string to_string(ObjectType type);

		static std::string to_string(MessageType type);

		static std::mutex m;

		static std::vector<ObjectType> _disableLog;

	};
	

}