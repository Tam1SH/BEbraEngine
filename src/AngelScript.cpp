#include "stdafx.h"
#include "AngelScript.hpp"
#include <angelscript.h>
namespace BEbraEngine {

	asIScriptContext* AngelScript::getContext()
	{
		return context;
	}
	AngelScript::AngelScript(asIScriptContext* context)
	{
		this->context = context;
	}
	void AngelScript::destroy(IVisitorGameComponentDestroyer& destroyer)
	{
	}
}
