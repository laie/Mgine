#include "StdAfx.hpp"
#include "Script.h"

using namespace Mgine;

DECLDEF(ScriptManager::IsInited);


void ScriptManager::Init()
{
	ATHROW(!IsInited);
	IsInited = true;

}

void ScriptManager::Uninit()
{
	ATHROW(IsInited);
	IsInited = false;

}
