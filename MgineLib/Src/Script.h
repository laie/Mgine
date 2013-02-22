#pragma once

class ScriptManager
{
private:
	static bool IsInited;

public:
	static void Init();
	static void Uninit();
};