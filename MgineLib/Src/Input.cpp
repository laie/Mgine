#include "StdAfx.hpp"
#include "Input.h"
using namespace Mgine;
using namespace Util;

bool					Input::IsInited;

LPDIRECTINPUT			Input::gDI;
LPDIRECTINPUTDEVICE		Input::gKeyboard;
LPDIRECTINPUTDEVICE		Input::gMouse;

HWND					Input::hWnd;
bool					Input::IsCursorInClient;

decltype(Input::Key)	Input::Key;
decltype(Input::Mouse)	Input::Mouse;

void Input::Init(HWND hWnd)
{
	HRESULT hr = 0;
	if ( IsInited ) MTHROW(InvalidStatus, "Already inited");

	MLOG("Begin");

	try
	{
		Input::hWnd = hWnd;

		if ( FAILED( hr = DirectInput8Create(EngineManager::Setting->GetSetting().hModule, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&gDI, NULL)) )
			MTHROW(DirectXError, "Failed to create di. hr:%.08x", hr);

		if ( FAILED( hr = gDI->CreateDevice(GUID_SysKeyboard, &gKeyboard, NULL )) )
			MTHROW(DirectXError, "Failed to create kd. hr:%.08x", hr);
		if ( FAILED( hr = gDI->CreateDevice(GUID_SysMouse, &gMouse, NULL)) )
			MTHROW(DirectXError, "Failed to create md. hr:%.08x", hr);

		if ( FAILED( hr = gKeyboard->SetDataFormat(&c_dfDIKeyboard)) )
			MTHROW(DirectXError, "K_SDF. hr:%.08x", hr);
		if ( FAILED( hr = gKeyboard->SetCooperativeLevel(hWnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND)) )
			MTHROW(DirectXError, "K_SCL. hr:%.08x", hr);

		if ( FAILED( hr = gMouse->SetDataFormat(&c_dfDIMouse2)) )
			MTHROW(DirectXError, "M_SDF. hr:%.08x", hr);
		if ( FAILED( hr = gMouse->SetCooperativeLevel(hWnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND)) )
			MTHROW(DirectXError, "M_SCL. hr:%.08x", hr);


		DIPROPDWORD dipdw;
		dipdw.diph.dwSize = sizeof(DIPROPDWORD);
		dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
		dipdw.diph.dwObj = 0;
		dipdw.diph.dwHow = DIPH_DEVICE;
		dipdw.dwData = InputBufferSize;

		if ( FAILED(hr = gKeyboard->SetProperty(DIPROP_BUFFERSIZE, &dipdw.diph)) )
			MTHROW(DirectXError, "K_SP. hr:%.08x", hr);
		if ( FAILED(hr = gMouse->SetProperty(DIPROP_BUFFERSIZE, &dipdw.diph)) )
			MTHROW(DirectXError, "M_SP. hr:%.08x", hr);

		gKeyboard->Acquire();
		gMouse->Acquire();

		IsInited = true;
	}
	catch ( BaseException & )
	{
		MLOG("Failed");
		Uninit();
		throw;
	}
}

void Input::Uninit()
{
	if ( gMouse ) gMouse->Unacquire();
	if ( gKeyboard ) gKeyboard->Unacquire();

	if ( gKeyboard ) RELEASE_OBJECT(gKeyboard);
	if ( gMouse ) RELEASE_OBJECT(gMouse);

	if ( gDI ) RELEASE_OBJECT(gDI);

	gDI = NULL;
	gKeyboard = NULL;
	gMouse = NULL;

	IsInited = false;
}

void Input::Update(double ElapsedTime)
{
	bool isinclient = EngineManager::Window->GetIsCursorInClientRect();
	{
		LockBlock lb(EngineManager::Window->CsWindow);
		IsCursorInClient = isinclient;
	}

	KEY_STATE keystate;
	if ( FAILED(gKeyboard->GetDeviceState(sizeof(KEY_STATE), &keystate)) )
		memset(&keystate, 0, sizeof(keystate));

	DIMOUSESTATE2 ms;
	if ( FAILED(gMouse->GetDeviceState(sizeof(ms), &ms)) )
		memset(&ms, 0, sizeof(ms));
	/*else 
	{
		//CurrentMouseState.X = ms.lX;
		//CurrentMouseState.Y = ms.lY;
		//CurrentMouseState.Z = ms.lZ;
		for ( int i=0; i < 8; i++ )
			CurrentMouseState.Key[i] = (KEY_RAW_DATA)ms.rgbButtons[i];
	}*/

	POINT pt; GetCursorPos(&pt);
	if ( EngineManager::Engine->D3dpp.Windowed )
	{
		RECT rc = GetContentRect();
		ms.lX = pt.x - rc.left;
		ms.lY = pt.y - rc.top;
		//CurrentMouseState.X = pt.x - rc.left;
		//CurrentMouseState.Y = pt.y - rc.top;
	} else {
		//CurrentMouseState.X = pt.x;
		//CurrentMouseState.Y = pt.y;
		ms.lX = pt.x;
		ms.lY = pt.y;
	}
			

	//if ( Key ) Key->Update(ElapsedTime);

	Key.Update(keystate, ElapsedTime);
	Mouse.Update((MOUSE_STATE&)ms, ElapsedTime);

	return ;
}

bool Input::CheckAcquire()
{
	HWND currentHWND = GetForegroundWindow();
	bool iscurrentwndpresent = false;
	bool isinclientrect = false;

	if ( currentHWND == hWnd ) iscurrentwndpresent = true;
	{
		LockBlock lb(EngineManager::Window->CsWindow);
		isinclientrect = IsCursorInClient;
	}

	HRESULT hr = gKeyboard->GetDeviceState(0, 0);
	if ( iscurrentwndpresent && (hr == DIERR_NOTACQUIRED || hr == DIERR_INPUTLOST) )
		gKeyboard->Acquire();
	
	hr = gMouse->GetDeviceState(0, 0);
	if ( isinclientrect && iscurrentwndpresent && (hr == DIERR_INPUTLOST || hr == DIERR_NOTACQUIRED ) )
		gMouse->Acquire();
	return true;
	//GetDeviceState returns DIERR_INPUTLOST or DIERR_NOTACQUIRED 

}