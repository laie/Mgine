#pragma once

#ifdef _DEBUG
#define DETECT_MEMORY_LEAK MemoryLeakDetector _MemoryLeakDetector
#define _CRTDBG_MAP_ALLOC
#undef  _malloca
#include <crtdbg.h>

class MemoryLeakDetector
{
private:
public:
	inline MemoryLeakDetector()
	{
		_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
		//_CrtSetBreakAlloc( 5462 );
	}
	inline ~MemoryLeakDetector()
	{
 	}
};
#else
#define DETECT_MEMORY_LEAK
#endif