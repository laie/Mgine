#include "StdAfx.hpp"
#include "Security.h"
using namespace Mgine;

DECLDEF(Sec::pGetTickCount) = &GetTickCount;


// #include <sstream>
// class DummyClass // for test security module
// {
// public:
// 	DummyClass()
// 	{
//  	int i=0;
// 		int outer = 0;
// 		std::wstringstream ss;
// 		//ss << "hi";
// 		double t = Mgine::Func::GetDoubleTick();
// 		//for ( i=0; i < 100; i++ )
// 		{
// 			TWIST(ss << L"<" << i << L">\t";)
// 		}
// 		t = Func::GetDoubleTick() - t;
// 		ss << t;
// 		MessageBox(0, ss.str().c_str(), 0, 0);
// 	}
// } InstanceDummyClass;

