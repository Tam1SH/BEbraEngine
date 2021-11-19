#pragma once
#include "BaseScriptEngine.hpp"
namespace BEbraEngine {
	class AngelScriptEngine : public BaseScriptEngine
	{
	public:
		AngelScriptEngine();
		// ������������ ����� BaseScriptEngine
		virtual BaseScript* CreateScript(std::string code) override;
	};
}


