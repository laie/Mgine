bool clsTexture::Draw()
{
	if ( !IsInited ) return false;
	bool isRestoreState = false;
	IDirect3DPixelShader9 *originalps = 0;
	IDirect3DVertexShader9 *originalvs = 0;
	IDirect3DSurface9 *originalrt = 0;

	clsRestorerTexture RestoreTexture;
	clsRestorerSamplerState RestoreSamplerState;

	MgineRenderer::SetTexture(GetTexture());
	if ( QueueEffect.Count == 0 ){
		if ( !SetVertex() ) goto cleanup;
		if ( IsRenderTarget ) AdjustTexelToPixel();
		if ( TextureAddress != MGINE_TEXTUREADDRESS_INHERIT )
		{
			MgineRenderer::SetSamplerState(D3DSAMP_ADDRESSU, TextureAddress);
			MgineRenderer::SetSamplerState(D3DSAMP_ADDRESSV, TextureAddress);
		}
		MgineRenderer::DrawQuad(&Vertex, 1, true, true, false);
		if ( IsRenderTarget ) AdjustPixelToTexel();
	} else {
		isRestoreState = true;
		MgineEngine::gDevice->GetPixelShader(&originalps);
		MgineEngine::gDevice->GetVertexShader(&originalvs);
		MgineEngine::gDevice->GetRenderTarget(0, &originalrt);

		int i;
		int effectcount = 0;
		POINTFLOAT origin = Origin;
		float radian = Radian;

		for ( i=0; i < QueueEffect.Size; i++ ){
			if ( QueueEffect.Arr[i] ){
				clsEffect *effect = ((clsEffect*)QueueEffect.Arr[i]);
				
				if ( QueueEffect.Count == 1 ){
					effect->Apply();
					if ( TextureAddress != MGINE_TEXTUREADDRESS_INHERIT )
					{
						MgineRenderer::SetSamplerState(D3DSAMP_ADDRESSU, TextureAddress);
						MgineRenderer::SetSamplerState(D3DSAMP_ADDRESSV, TextureAddress);
					}
					SetEffectTextureParameter(effect);
					SetVertex();
					if ( IsRenderTarget ) AdjustTexelToPixel();

					MgineRenderer::DrawQuad(&Vertex, 1, true, true, false);
					if ( IsRenderTarget ) AdjustPixelToTexel();

				} else if ( effectcount == 0 ){
					// firstly
					Origin.y = Origin.x = 0;
					Radian = 0;


					MGINE_RECT rectd = RectDest, rects = RectSource;
					RectDest.X = 0;
					RectDest.Y = 0;
					RectDest.Width = (float)MgineEngine::Viewport.Width;
					RectDest.Height = (float)MgineEngine::Viewport.Height;

					RectDest.X = 0;
					RectDest.Y = 0;
					RectDest.Width = (float)ImageDesc.Width;
					RectDest.Height = (float)ImageDesc.Height;

					RectSource.X = 0;
					RectSource.Y = 0;
					RectSource.Width = (float)ImageDesc.Width;
					RectSource.Height = (float)ImageDesc.Height;


					MgineRenderer::SetRenderTarget(MgineEngine::TemporaryRenderTargetTexture[effectcount % 2].GetSurface());
					effect->Apply();
					if ( TextureAddress != MGINE_TEXTUREADDRESS_INHERIT )
					{
						MgineRenderer::SetSamplerState(D3DSAMP_ADDRESSU, TextureAddress);
						MgineRenderer::SetSamplerState(D3DSAMP_ADDRESSV, TextureAddress);
					}
					SetEffectTextureParameter(effect);
					if ( !MgineRenderer::Clear(0) ) goto cleanup;
					SetVertex();
					MgineRenderer::DrawQuad(&Vertex, 1, true, true, false);
					MgineRenderer::RenderBatch();

					RectDest = rectd;
					RectSource = rects;
				} else if ( QueueEffect.Count -1 == effectcount ){
					// final
					Origin = origin;
					Radian = radian;

					MGINE_RECT rects = RectSource, rectd = RectDest;
					
					RectSource.X = 0;
					RectSource.Y = 0;
					RectSource.Width = (float)ImageDesc.Width;//RectDest.Width*RectDest.Width/(float)MgineEngine::Viewport.Width;
					RectSource.Height = (float)ImageDesc.Height;//RectDest.Height*RectDest.Height/(float)MgineEngine::Viewport.Height;

					//RectDest.Width *= (float)MgineEngine::Viewport.Width/ImageDesc.Width;
					//RectDest.Height *= (float)MgineEngine::Viewport.Height/ImageDesc.Height;

					MgineRenderer::SetTexture(MgineEngine::TemporaryRenderTargetTexture[(effectcount + 2 - 1) % 2].GetTexture());
					MgineRenderer::SetRenderTarget(originalrt);

					effect->Apply();
					if ( TextureAddress != MGINE_TEXTUREADDRESS_INHERIT )
					{
						MgineRenderer::SetSamplerState(D3DSAMP_ADDRESSU, TextureAddress);
						MgineRenderer::SetSamplerState(D3DSAMP_ADDRESSV, TextureAddress);
					}
					SetEffectTextureParameter(effect, &MgineEngine::TemporaryRenderTargetTexture[(effectcount + 2 - 1) % 2].ImageDesc);
					effect->SetParameter("SamplerA", MgineEngine::TemporaryRenderTargetTexture[(effectcount + 2 - 1) % 2].GetTexture());
					
					SetVertex(&MgineEngine::TemporaryRenderTargetTexture[(effectcount + 2 - 1) % 2].ImageDesc);
					MgineRenderer::DrawQuad(&Vertex, 1, true, true, false);
					MgineRenderer::RenderBatch();

					RectSource = rects;
					RectDest = rectd;
				} else {
					// progress
					MGINE_RECT rects = RectSource, rectd = RectDest;

					RectDest.X = 0;
					RectDest.Y = 0;
					RectDest.Width = (float)ImageDesc.Width;
					RectDest.Height = (float)ImageDesc.Height;

					RectSource.X = 0;
					RectSource.Y = 0;
					RectSource.Width = (float)ImageDesc.Width;
					RectSource.Height = (float)ImageDesc.Height;
					
					RectDest.X = 0;
					RectDest.Y = 0;
					RectDest.Width = (float)ImageDesc.Width;
					RectDest.Height = (float)ImageDesc.Height;
					
					MgineRenderer::SetTexture(MgineEngine::TemporaryRenderTargetTexture[(effectcount+1) % 2].GetTexture());
					MgineRenderer::SetRenderTarget(MgineEngine::TemporaryRenderTargetTexture[effectcount % 2].GetSurface());

					effect->Apply();
					if ( TextureAddress != MGINE_TEXTUREADDRESS_INHERIT )
					{
						MgineRenderer::SetSamplerState(D3DSAMP_ADDRESSU, TextureAddress);
						MgineRenderer::SetSamplerState(D3DSAMP_ADDRESSV, TextureAddress);
					}
					SetEffectTextureParameter(effect);//, &MgineEngine::TemporaryRenderTargetTexture[(effectcount + 2 - 1) % 2].ImageDesc);
					effect->SetParameter("SamplerA", MgineEngine::TemporaryRenderTargetTexture[(effectcount + 2 - 1) % 2].GetTexture());

					if ( !MgineRenderer::Clear(0) ) goto cleanup;
					SetVertex();
					MgineRenderer::DrawQuad(&Vertex, 1, true, true, false);
					MgineRenderer::RenderBatch();

					RectSource = rects;
					RectDest = rectd;
				}
				

				effectcount ++;
			}
			if ( effectcount >= QueueEffect.Count ) break;
		}
		Origin = origin;
		Radian = radian;
	}
	
	if ( isRestoreState ){
		MgineRenderer::SetRenderTarget(originalrt);
		MgineRenderer::SetPixelShader(originalps);
		MgineRenderer::SetVertexShader(originalvs);
		if ( originalrt ) originalrt->Release();
		if ( originalps ) originalps->Release();
		if ( originalvs ) originalvs->Release();
	}
	return true;

cleanup:
	if ( isRestoreState ){
		MgineRenderer::SetRenderTarget(originalrt);
		MgineRenderer::SetPixelShader(originalps);
		MgineRenderer::SetVertexShader(originalvs);
		if ( originalrt ) originalrt->Release();
		if ( originalps ) originalps->Release();
		if ( originalvs ) originalvs->Release();
	}
	return false;
}

void clsTexture::Update(int Stacked)
{
	/*DWORD i;
	DWORD effectcount = 0;
	for ( i=0; i < QueueEffect.Size; i++ ){
		if ( QueueEffect.Arr[i] ) {
			effectcount ++;
			((clsEffect*)QueueEffect.Arr[i])->Update(Stacked);
		}
		if ( effectcount >= QueueEffect.Count ) break;
	}
	*/
}








































#include "Mgine.h"

bool					MgineInput::IsInited;

LPDIRECTINPUT			MgineInput::gDI;
LPDIRECTINPUTDEVICE		MgineInput::gKeyboard;
LPDIRECTINPUTDEVICE		MgineInput::gMouse;

HWND					MgineInput::hWnd;
BYTE					MgineInput::KeyState[256];
MGINE_MOUSE_STATE		MgineInput::MouseState;
BYTE					MgineInput::CurrentKeyState[256];
MGINE_MOUSE_STATE		MgineInput::CurrentMouseState;
BYTE					MgineInput::BeforeKeyState[256];
MGINE_MOUSE_STATE		MgineInput::BeforeMouseState;


HANDLE					MgineInput::hKeyboardEvent, MgineInput::hMouseEvent;
HANDLE					MgineInput::HtLoopEventHandlerKeyboard, MgineInput::HtLoopEventHandlerMouse;
DWORD					MgineInput::TidLoopEventHandlerKeyboard, MgineInput::TidLoopEventHandlerMouse;

QUEUE_EVENT_KEY			MgineInput::QueueEventKey[1024];
QUEUE_EVENT_MOUSE_MOVE	MgineInput::QueueEventMouseMove[1024];
QUEUE_EVENT_MOUSE_CLICK	MgineInput::QueueEventMouseClick[1024];
int MgineInput::QueueEventKeyCount,	MgineInput::QueueEventMouseMoveCount, MgineInput::QueueEventMouseClickCount;

CRITICAL_SECTION MgineInput::CsQueueEventKey, MgineInput::CsQueueEventMouseMove, MgineInput::CsQueueEventMouseClick;
MgineInput::KeyIntegration *MgineInput::Key;

bool MgineInput::Init(HWND hWnd)
{
	HRESULT hr = 0;
	if ( IsInited ) return true;

	MgineLog::WriteLog(L"Begin %s", __FUNCTIONW__);

	MgineInput::hWnd = hWnd;
	InitializeCriticalSection(&CsQueueEventKey);
	InitializeCriticalSection(&CsQueueEventMouseMove);
	InitializeCriticalSection(&CsQueueEventMouseClick);

	if ( FAILED( hr = DirectInput8Create(MgineEngine::Setting->GetSetting().hModule, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&gDI, NULL) ) )
	{
		MgineLog::WriteLog(L"Failed to create directinput. hr: %.08x", hr);
		goto cleanup;
	}

	if ( FAILED( hr = gDI->CreateDevice(GUID_SysKeyboard, &gKeyboard, NULL ) ) )
	{
		MgineLog::WriteLog(L"Failed to create keyboard device. hr: %.08x", hr);
		goto cleanup;
	}
	if ( FAILED( hr = gDI->CreateDevice(GUID_SysMouse, &gMouse, NULL) ) )
	{
		MgineLog::WriteLog(L"Failed to create mouse device. hr: %.08x", hr);
		goto cleanup;
	}

	if ( FAILED( hr = gKeyboard->SetDataFormat(&c_dfDIKeyboard) ) )	goto cleanup;
	if ( FAILED( hr = gKeyboard->SetCooperativeLevel(hWnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND) ) ) goto cleanup;

	if ( FAILED( hr = gMouse->SetDataFormat(&c_dfDIMouse2) ) ) goto cleanup;
	if ( FAILED( hr = gMouse->SetCooperativeLevel(hWnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND) ) ) goto cleanup;

	hKeyboardEvent = CreateEvent(0, false, false, 0);
	if ( hKeyboardEvent == INVALID_HANDLE_VALUE ) goto cleanup;

	hMouseEvent = CreateEvent(0, false, false, 0);
	if ( hKeyboardEvent == INVALID_HANDLE_VALUE ) goto cleanup;

	//gKeyboard->SetEventNotification(hKeyboardEvent);
	//gMouse->SetEventNotification(hMouseEvent);

	DIPROPDWORD dipdw;
	dipdw.diph.dwSize = sizeof(DIPROPDWORD);
	dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	dipdw.diph.dwObj = 0;
	dipdw.diph.dwHow = DIPH_DEVICE;
	dipdw.dwData = MGINE_INPUT_BUFFER_SIZE;

	if ( FAILED(hr = gKeyboard->SetProperty(DIPROP_BUFFERSIZE, &dipdw.diph) ) ) goto cleanup;
	if ( FAILED(hr = gMouse->SetProperty(DIPROP_BUFFERSIZE, &dipdw.diph) ) ) goto cleanup;

	//HtLoopEventHandlerKeyboard = CreateThread(0, 0, LoopEventHandler, 0, 0, &TidLoopEventHandlerKeyboard);
	//HtLoopEventHandlerMouse = CreateThread(0, 0, LoopEventHandler, (LPVOID)1, 0, &TidLoopEventHandlerMouse);

	Key = KeyIntegration::GetInstance();

	gKeyboard->Acquire();
	gMouse->Acquire();
	
	if ( !Key->Init() ) goto cleanup;

	IsInited = true;
	return true;

cleanup:
	MgineLog::WriteLog(L"Failed %s", __FUNCTIONW__);
	Uninit();
	return false;
}

void MgineInput::Uninit()
{
	if ( Key ) Key->Uninit();
	if ( gMouse ) gMouse->Unacquire();
	if ( gKeyboard ) gKeyboard->Unacquire();

	Key = NULL;

	if ( HtLoopEventHandlerKeyboard ){
		WaitForSingleObject(HtLoopEventHandlerKeyboard, INFINITE);
		CloseHandle(HtLoopEventHandlerKeyboard);
	}
	if ( HtLoopEventHandlerMouse ){
		WaitForSingleObject(HtLoopEventHandlerMouse, INFINITE);
		CloseHandle(HtLoopEventHandlerMouse);
	}

	if ( hKeyboardEvent ) CloseHandle(hKeyboardEvent);
	if ( hMouseEvent ) CloseHandle(hMouseEvent);

	if ( gKeyboard ) RELEASE_OBJECT(gKeyboard);
	if ( gMouse ) RELEASE_OBJECT(gMouse);

	if ( gDI ) RELEASE_OBJECT(gDI);

	if ( CsQueueEventMouseClick.DebugInfo ) DeleteCriticalSection(&CsQueueEventMouseClick);
	if ( CsQueueEventMouseMove.DebugInfo ) DeleteCriticalSection(&CsQueueEventMouseMove);
	if ( CsQueueEventKey.DebugInfo ) DeleteCriticalSection(&CsQueueEventKey);

	gDI = NULL;
	gKeyboard = NULL;
	gMouse = NULL;
	hKeyboardEvent = 0;
	hMouseEvent = 0;

	HtLoopEventHandlerKeyboard = 0;
	TidLoopEventHandlerKeyboard = 0;
	HtLoopEventHandlerMouse = 0;
	TidLoopEventHandlerMouse = 0;

	IsInited = false;
}

void MgineInput::Update(int Stacked)
{
	int i;
	EnterCriticalSection(&CsQueueEventKey);
	if ( FAILED(gKeyboard->GetDeviceState(sizeof(KeyState), KeyState)) )
	{
		memset(KeyState, 0, sizeof(KeyState));
	}
	memcpy(BeforeKeyState,  CurrentKeyState, sizeof(KeyState));
	memcpy(CurrentKeyState, KeyState, sizeof(KeyState));
	LeaveCriticalSection(&CsQueueEventKey);

	EnterCriticalSection(&CsQueueEventMouseClick);
	EnterCriticalSection(&CsQueueEventMouseMove);

	//MouseState is same as the structure DIMOUSESTATE2. if dimousestate2 is changed, it must be modified adjusted to new structure
	DIMOUSESTATE2 ms;
	if ( FAILED(gMouse->GetDeviceState(sizeof(ms), &ms)) )
	{
		memset(MouseState.Key, 0, sizeof(MouseState.Key));
	} else 
	{
		//MouseState.X = ms.lX;
		//MouseState.Y = ms.lY;
		//MouseState.Z = ms.lZ;
		for ( int i=0; i < 8; i++ )
			MouseState.Key[i] = (MGINE_EVENT_KEY_DATA)ms.rgbButtons[i];
	}

	POINT pt; GetCursorPos(&pt);
	RECT rc, rcclient;
	if ( MgineEngine::D3dpp.Windowed ){
		GetClientRect(hWnd, &rcclient);
		AdjustWindowRectEx(&rcclient, GetWindowLong(hWnd, GWL_STYLE), false, GetWindowLong(hWnd, GWL_EXSTYLE));
		GetClientRect(hWnd, &rc);
		rcclient.right  -= rc.right;
		rcclient.bottom -= rc.bottom;

		GetWindowRect(hWnd, &rc);
		rc.left		-= rcclient.left;
		rc.right	-= rcclient.right;
		rc.top		-= rcclient.top;
		rc.bottom	-= rcclient.bottom;

		MouseState.X = pt.x - rc.left;
		MouseState.Y = pt.y - rc.top;
	} else {
		MouseState.X = pt.x;
		MouseState.Y = pt.y;
	}
			
	BeforeMouseState = CurrentMouseState;
	CurrentMouseState = MouseState;

	LeaveCriticalSection(&CsQueueEventMouseMove);
	LeaveCriticalSection(&CsQueueEventMouseClick);
	
	for ( i=0; i < QueueEventKeyCount; i++ ){
		MgineEngine::RaiseKeyEvent(QueueEventKey[i].KeyCode, QueueEventKey[i].Data);
	}
	QueueEventKeyCount = 0;

	for ( i=0; i < QueueEventMouseMoveCount; i++ ){
		MgineEngine::RaiseMouseMoveEvent(QueueEventMouseMove[i].Direction, QueueEventMouseMove[i].Data);
	}
	QueueEventMouseMoveCount = 0;

	for ( i=0; i < QueueEventMouseClickCount; i++ ){
		MgineEngine::RaiseMouseClickEvent(QueueEventMouseClick[i].Button, QueueEventMouseClick[i].Data);
	}
	QueueEventMouseClickCount = 0;

	if ( Key ) Key->Update(Stacked);

	return ;
}

bool MgineInput::CheckAcquire()
{
	static HWND LastHWND;

	HWND currentHWND = GetForegroundWindow();
	bool needAcquire = false;
	if ( currentHWND == hWnd ) needAcquire = true;
	//if ( currentHWND == hWnd && LastHWND != currentHWND ) needAcquire = true;
	LastHWND = currentHWND;

	HRESULT hr = gKeyboard->GetDeviceState(0, 0);
	if ( needAcquire && (hr == DIERR_NOTACQUIRED || hr == DIERR_INPUTLOST) ){
		//gKeyboard->Unacquire();
		gKeyboard->Acquire();
	}
	
	hr = gMouse->GetDeviceState(0, 0);
	if ( MgineFunction::IsCursorInClientRect() && needAcquire && (hr == DIERR_INPUTLOST || hr == DIERR_NOTACQUIRED ) ){
		gMouse->Acquire();
	}/* else if ( !MgineEngine::IsCursorInClientRect() ){
		gMouse->Unacquire();
	}*/
	
	return true;
	//GetDeviceState returns DIERR_INPUTLOST or DIERR_NOTACQUIRED 

}

ULONG WINAPI MgineInput::LoopEventHandler(void *Temp)
{
	DWORD kind = (DWORD)Temp;
	HANDLE hevent;
	DIDEVICEOBJECTDATA devData[MGINE_INPUT_BUFFER_SIZE];

	switch ( kind ){
		case 0:
			// Keyboard
			hevent = hKeyboardEvent;
			break;

		case 1:
			// Mouse
			hevent = hMouseEvent;
			break;

		default:break;
	}

	while ( !MgineEngine::IsTerminating ){
		DWORD ret = WaitForSingleObject(hevent, 10);
		if ( ret == WAIT_OBJECT_0 || ret == WAIT_TIMEOUT ){
			
			DWORD nevent = MGINE_INPUT_BUFFER_SIZE;
			switch ( kind ){
				case 0:
					if ( SUCCEEDED(gKeyboard->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), devData, &nevent, 0)) ){
						for ( DWORD i=0; i < nevent; i++ ){
							if ( devData[i].dwData == 128 ){
								// Key Down
								EnterCriticalSection(&CsQueueEventKey);
								KeyState[devData[i].dwOfs] = 1;
								//CurrentKeyState[i] = 1;
								MgineEngine::InvokeKeyEvent(devData[i].dwOfs, (MGINE_EVENT_KEY_DATA)(devData[i].dwData != 0));
								LeaveCriticalSection(&CsQueueEventKey);

							} else {
								// Key Up
								EnterCriticalSection(&CsQueueEventKey);
								KeyState[devData[i].dwOfs] = 0;
								//CurrentKeyState[i] = 0;
								MgineEngine::InvokeKeyEvent(devData[i].dwOfs, (MGINE_EVENT_KEY_DATA)(devData[i].dwData != 0));
								LeaveCriticalSection(&CsQueueEventKey);
							}
						}
					} else {
						EnterCriticalSection(&CsQueueEventKey);
						for ( DWORD i=0; i < 256; i++ ){
							if ( KeyState[i] ){
								KeyState[i] = 0;
								//CurrentKeyState[i] = 0;
								MgineEngine::InvokeKeyEvent(i, MGINE_KEY_UP);
							}
						}
						LeaveCriticalSection(&CsQueueEventKey);
						//MessageBox(0, "DI KEYBOARD ELSE", 0, 0);
						
					}
					break;

				case 1:
					if ( SUCCEEDED(gMouse->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), devData, &nevent, 0)) ){
						for ( DWORD i=0; i < nevent; i++ ){
							switch ( devData[i].dwOfs ){
								case DIMOFS_X:
									EnterCriticalSection(&CsQueueEventMouseMove);
									MouseState.X += devData[i].dwData;
									//CurrentMouseState.X = MouseState.X;
									MgineEngine::InvokeMouseMoveEvent(MGINE_MOUSE_DIRECTION_X, devData[i].dwData);
									LeaveCriticalSection(&CsQueueEventMouseMove);
									break;
								case DIMOFS_Y:
									EnterCriticalSection(&CsQueueEventMouseMove);
									MouseState.Y += devData[i].dwData;
									//CurrentMouseState.Y = MouseState.Y;
									MgineEngine::InvokeMouseMoveEvent(MGINE_MOUSE_DIRECTION_Y, devData[i].dwData);
									LeaveCriticalSection(&CsQueueEventMouseMove);
									break;
								case DIMOFS_Z:
									EnterCriticalSection(&CsQueueEventMouseMove);
									MouseState.Z += devData[i].dwData;
									//CurrentMouseState.Z = MouseState.Z;
									MgineEngine::InvokeMouseMoveEvent(MGINE_MOUSE_DIRECTION_Z, devData[i].dwData);
									LeaveCriticalSection(&CsQueueEventMouseMove);
									break;
								case DIMOFS_BUTTON0:
										// Left
								case DIMOFS_BUTTON1:
										// Right
								case DIMOFS_BUTTON2:
										// Middle
								case DIMOFS_BUTTON3:
								case DIMOFS_BUTTON4:
								case DIMOFS_BUTTON5:
								case DIMOFS_BUTTON6:
								case DIMOFS_BUTTON7:
									EnterCriticalSection(&CsQueueEventMouseClick);
									MouseState.Key[(MGINE_EVENT_MOUSE_BUTTON)(devData[i].dwOfs - DIMOFS_BUTTON0)] = (MGINE_EVENT_KEY_DATA)(devData[i].dwData != 0);
									//CurrentMouseState.Key[(MGINE_EVENT_MOUSE_BUTTON)(devData[i].dwOfs - DIMOFS_BUTTON0)] = (MGINE_EVENT_KEY_DATA)(devData[i].dwData != 0);
									MgineEngine::InvokeMouseClickEvent((MGINE_EVENT_MOUSE_BUTTON)(devData[i].dwOfs - DIMOFS_BUTTON0), (MGINE_EVENT_KEY_DATA)(devData[i].dwData != 0));
									LeaveCriticalSection(&CsQueueEventMouseClick);
									break;
							}	
						}
					} else {
						EnterCriticalSection(&CsQueueEventMouseClick);
						for ( DWORD i=0; i < 8; i++ ){
							if ( MouseState.Key[i] ){
								MouseState.Key[i] = MGINE_KEY_UP;
								//CurrentMouseState.Key[i] = MouseState.Key[i];
								MgineEngine::InvokeMouseClickEvent((MGINE_EVENT_MOUSE_BUTTON)i, MGINE_KEY_UP);
							}
						}
						LeaveCriticalSection(&CsQueueEventMouseClick);
					}
					break;

				default:break;
			}
			if ( ret != WAIT_TIMEOUT ) ResetEvent(hevent);
		}
	}
	return 0;
}




MgineInput::KeyIntegration MgineInput::KeyIntegration::Instance;

MgineInput::KeyIntegration::KeyIntegration()
{
	IsInited = false;
	memset(ArrKeyStacked, 0, sizeof(ArrKeyStacked));
	memset(ArrMouseStacked, 0, sizeof(ArrMouseStacked));
}

MgineInput::KeyIntegration::~KeyIntegration()
{
	Uninit();
}

MgineInput::KeyIntegration *MgineInput::KeyIntegration::GetInstance()
{
	return &Instance;
}

bool MgineInput::KeyIntegration::IsKeyHit(MGINE_KEYCODE Key)
{
	return ArrKeyStacked[(BYTE)Key] == 1;
}

bool MgineInput::KeyIntegration::IsKeyDown(MGINE_KEYCODE Key)
{
	return ArrKeyStacked[(BYTE)Key] != 0;
}

DWORD MgineInput::KeyIntegration::GetKeyStacked(MGINE_KEYCODE Key)
{
	return ArrKeyStacked[(BYTE)Key];
}

MGINE_KEYSTATE MgineInput::KeyIntegration::GetKeyState(MGINE_KEYCODE Key)
{
	if ( IsKeyDown(Key) )
	{
		if ( IsKeyHit(Key) )
			return MGINE_KEYSTATE_HIT;
		else return MGINE_KEYSTATE_DOWN;
	} else return MGINE_KEYSTATE_UP;
}

bool MgineInput::KeyIntegration::IsMouseHit(MGINE_EVENT_MOUSE_BUTTON Button)
{
	if ( !(0 <= Button && Button < 8) ) return false;
	return ArrMouseStacked[Button] == 1;
}

bool MgineInput::KeyIntegration::IsMouseDown(MGINE_EVENT_MOUSE_BUTTON Button)
{
	if ( !(0 <= Button && Button < 8) ) return false;
	return ArrMouseStacked[Button] != 0;
}

DWORD MgineInput::KeyIntegration::GetMouseStacked(MGINE_EVENT_MOUSE_BUTTON Button)
{
	if ( !(0 <= Button && Button < 8) ) return 0;
	return ArrMouseStacked[Button];
}

MGINE_KEYSTATE MgineInput::KeyIntegration::GetMouseState(MGINE_EVENT_MOUSE_BUTTON Button)
{
	if ( !(0 <= Button && Button < 8) ) return MGINE_KEYSTATE_UP;
	if ( IsMouseDown(Button) )
	{
		if ( IsMouseHit(Button) )
			return MGINE_KEYSTATE_HIT;
		else return MGINE_KEYSTATE_DOWN;
	} else return MGINE_KEYSTATE_UP;
}

		
bool MgineInput::KeyIntegration::Init()
{
	IsInited = true;
	return true;
}

void MgineInput::KeyIntegration::Uninit()
{
	if ( !IsInited ) return;
	IsInited = false;
	return ;
}

void MgineInput::KeyIntegration::Update(int Stacked)
{
	int i;
	for ( i=0; i < 256; i++ )
	{
		if ( MgineInput::CurrentKeyState[i] ){
			if ( !ArrKeyStacked[i] ) ArrKeyStacked[i] = 1;
			else ArrKeyStacked[i] += Stacked;
		} else {
			ArrKeyStacked[i] = 0;
		}
	}

	for ( i=0 ; i < 8; i++ )
	{
		if ( MgineInput::CurrentMouseState.Key[i] ){
			if ( !ArrMouseStacked[i] ) ArrMouseStacked[i] = 1;
			else ArrMouseStacked[i] += Stacked;
		} else {
			ArrMouseStacked[i] = 0;
		}
	}

}











// auditory masker class

class clsAudioMasker
{
public:
	/*\	Roex Filter
	 *		Based on "A Model for the Prediction of Thresholds, Loudness, and Partial Loudness"
	\*/
	class FilterRoex
	{
	private:
		static const double C1;// = 24.673;
		static const double C2;// = 4.368;
		static const double C3;// = 21.366;
		static const double LOWFREQ;
		
		int CountFilter;
		
		// Central frequency to ERB.
		double CentralFreqToERB(double CentralFrequency);
		// Frequency to ERB
		double FreqToERB(double Frequency);
		double ERBToFreq(double ERB);
		
		virtual void InitAudioFilter(double CentralFrequency, int FilterIndex);
		void InitCentralFrequency(void);
		void Init(int BaseFrequency, int FilterCount=50);
		
	protected:
		int BaseFrequency; // fs
		AutoUnwindArray<double> ArrGCoeff;	// g
		double **Filter; //!< The filters.
		
		double EvalP(double CenterFrequency);
	public:
		AutoUnwindArray<double> ArrCentralFrequency; // cf filter centre frequency arr
		AutoUnwindArray<double> ArrEdgeFrequency; // ef filter edge frequency arr
		AutoUnwindArray<double> ArrExcitation;
		AutoUnwindArray<double> ArrMask;
		AutoUnwindArray<double> ArrLvmu;
		double **Output;
		double **Spread;
		
		

		FilterRoex(int BaseFrequency, int CountFilter=50){
			Init(BaseFrequency, CountFilter);
		}
		
		
		virtual ~FilterRoex(){
			if (Filter){
				for ( int i=0; i < CountFilter; i++ )
					if (Filter[i]) delete [] Filter[i];
				delete [] Filter;
			}
			if (Output){
				for ( int i=0; i < CountFilter; i++ )
					if ( Output[i] ) delete[] Output;
				delete[] Output;
			}
			if (Spread){
				for ( int i=0; i < CountFilter; i++ )
					if ( Spread[i] ) delete[] Spread;
				delete[] Spread;
			}
		}
		
		int GetFilterCount();
		double GetMagnitude(int IndexFilter, int IndexFrequency, int CountFourierBin);
		void Excite(double *ArrFrequency, int CountFrequency);
	};
	
};


const double clsAudioMasker::FilterRoex::C1 = 24.673;
const double clsAudioMasker::FilterRoex::C2 = 4.368;
const double clsAudioMasker::FilterRoex::C3 = 21.366;
const double clsAudioMasker::FilterRoex::LOWFREQ = 50;

double clsAudioMasker::FilterRoex::CentralFreqToERB(double CentralFrequency)
{
	return 24.7*(4.37*(CentralFrequency/1000.0)+1.0);
}

double clsAudioMasker::FilterRoex::FreqToERB(double Frequency)
{
	return (C3*log10((C2 * Frequency/1000.0) + 1.0));
}

double clsAudioMasker::FilterRoex::ERBToFreq(double ERB)
{
    return 1000.0 * (pow(10.0,(ERB/C3)) - 1.0) / C2;
}

void clsAudioMasker::FilterRoex::InitAudioFilter(double CentralFrequency, int FilterIndex)
{
	double freqFact=((double)BaseFrequency/2.0)/(double)MgineMusic::PCMPerSecond;
	double freq=0.0;
	for (int i=0;i<MgineMusic::PCMPerSecond;i++){
		ArrGCoeff.Arr[i] = fabs((freq-CentralFrequency)/CentralFrequency);
		freq += freqFact;
	}
		
	double *filt = Filter[FilterIndex], p;
	freq=0.0;

	for ( int i=0; i < MgineMusic::PCMPerSecond; i++ ){
		if ( freq < CentralFrequency )
			p = /*p_l*/EvalP(CentralFrequency);
		else
			p = /*p_u*/EvalP(CentralFrequency);
		filt[i]=(1.0+p*ArrGCoeff.Arr[i])*exp(-p*ArrGCoeff.Arr[i]);
		freq += freqFact;
	}
}

void clsAudioMasker::FilterRoex::InitCentralFrequency()
{
	double step=(FreqToERB((double)BaseFrequency/2.0-1.0)-FreqToERB(LOWFREQ))/(CountFilter-1.0);
	//std::cout<<"step "<<step<<std::endl;
	double erbval=FreqToERB(LOWFREQ)-step;
	for ( int i=0; i < CountFilter; i++ ){
		ArrCentralFrequency.Arr[i] = ERBToFreq(erbval += step); //centre frequency locations
		//std::cout<<cf[i]<<std::endl;
	}

	erbval = FreqToERB(LOWFREQ)-step/2.0;
	ArrEdgeFrequency.Arr[0] = 0.0;
	for ( int i=1; i < CountFilter; i++ ){
		ArrEdgeFrequency.Arr[i] = ERBToFreq(erbval += step); //edge frequency locations
		//std::cout<<cf[i]<<'\t'<<ef[i]<<std::endl;
	}
}

double clsAudioMasker::FilterRoex::EvalP(double CenterFrequency)
{
	return 4.0*CenterFrequency/CentralFreqToERB(CenterFrequency);
}

void clsAudioMasker::FilterRoex::Init(int BaseFrequency, int FilterCount)
{
	this->CountFilter = FilterCount;
	this->BaseFrequency = BaseFrequency;
	Filter = NULL;

	ArrGCoeff.Set(new double[MgineMusic::PCMPerSecond]);

	Filter = new double*[FilterCount];
	for ( int i=0; i < FilterCount; i++ )
		Filter[i] = NULL;
	for ( int i=0; i < FilterCount; i++ )
		Filter[i] = new double[MgineMusic::PCMPerSecond];

	Output = new double*[FilterCount];
	for ( int i=0; i < FilterCount; i++ )
		Output[i] = new double[MgineMusic::FFTTransformCount];
	Spread = new double*[FilterCount];
	for ( int i=0; i < FilterCount; i++ )
		Spread[i] = new double[FilterCount];

	ArrCentralFrequency.Set(new double[FilterCount]);
    ArrEdgeFrequency.Set(new double[FilterCount]);
	ArrExcitation.Set(new double[FilterCount]);
    ArrMask.Set(new double[FilterCount]);
    ArrLvmu.Set(new double[FilterCount*FilterCount]);

    //Place the filter center freqs
    InitCentralFrequency();
    //fill each filter
    for ( int i=0; i < FilterCount; i++ )
      InitAudioFilter(ArrCentralFrequency.Arr[i], i);
}

int clsAudioMasker::FilterRoex::GetFilterCount()
{
	return CountFilter;
}

double clsAudioMasker::FilterRoex::GetMagnitude(int IndexFilter, int IndexFrequency, int CountFourierBin)
{
	int index = (int)((double)IndexFrequency*((double)MgineMusic::PCMPerSecond/(double)CountFourierBin));
	return Filter[IndexFilter][index];
}

void clsAudioMasker::FilterRoex::Excite(double *ArrFrequency, int CountFrequency)
{
	if ( MgineMusic::FFTTransformCount/2 != CountFrequency ) return;
	int i,j;
	for ( i=0; i < CountFilter; i++ )
		for ( j=0; j < CountFrequency; j++ )
			Output[i][j] = ArrFrequency[j]*GetMagnitude(i, j, CountFrequency);

	// Terhardt model
	//F2CB(f) (13.3*atan(0.75*f/1000))
	double factor = fabs(CountFilter - (13.3*atan(0.75*(BaseFrequency/2.f)/1000)) );
	for ( i=0; i < CountFilter; i++ )
	{
		ArrExcitation.Arr[i] = 0.;
		for ( j=0; j < CountFrequency; j++ )
		{
			ArrExcitation.Arr[i] += Output[i][j];
		}
		ArrExcitation.Arr[i] = 10.*log10(ArrExcitation.Arr[i]);
	}

	// excite of moore spread
	int binofinterest;
	for ( i=0; i < CountFilter; i++ )
	{
		binofinterest = (int)(ArrCentralFrequency[i]);
		for ( j=0; j < CountFilter; j++ )
		{
			Spread[i][j] = Output[j][binofinterest];
		}
	}
	// end of excite of moore spread

	//find the mask
	for ( i=0; i < CountFilter; i++ )
		for ( j=0; j < CountFilter; j++ )
			ArrLvmu[j+i*CountFilter] = ArrExcitation[j] + 10.0*log10(Spread[i][j]);
	for ( i=0; i < CountFilter; i++ )
	{
		ArrMask[i] = 0.;
		for ( j=0; j < i; j++ )
			ArrMask[i] += pow(10., ArrLvmu[i+j*CountFilter]/20.);
		for ( j=i+1; j < CountFilter; j++ )
			ArrMask[i] += pow(10., ArrLvmu[i+j*CountFilter]/20.);
		ArrMask[i] /= factor;
		//if ( ArrMask[i] > max
	}

}








// my own spread function : not correct
	/*for ( i=0; i < clsSoundAnalysisPool::WeightFrequencyDomain; i++ )
	{
		// logarithm frequencies
		double spreadfactor = 0.02*MgineFunction::Log(2., PowerSpectrum.Arr[i]);
		double loga = MgineFunction::Log(2., (double)(i+1)/clsSoundAnalysisPool::WeightFrequencyDomain/2*MgineMusic::PCMFrequency),
			logalow  = loga - spreadfactor,
			logahigh = loga + spreadfactor;
		double maskingval = PowerSpectrum.Arr[i]*0.8*0.8;

		if ( MaskingSpectrum.Arr[i] < maskingval ) MaskingSpectrum.Arr[i] = maskingval;

		for ( p=i-1; p >= 0; p-- )
		{
			loga = MgineFunction::Log(2., (double)(p+1)/clsSoundAnalysisPool::WeightFrequencyDomain/2*MgineMusic::PCMFrequency);
			if ( logalow < loga )
			{
				double degree = ((loga-logalow)/spreadfactor)*0.8;
				degree *= degree;
				maskingval = PowerSpectrum.Arr[i]*degree;
				if ( MaskingSpectrum.Arr[p] < maskingval ) MaskingSpectrum.Arr[p] = maskingval;
			} else break;
		}

		for ( p=i+1; p < clsSoundAnalysisPool::WeightFrequencyDomain; p++ )
		{
			loga = MgineFunction::Log(2., (double)(p+1)/clsSoundAnalysisPool::WeightFrequencyDomain/2*MgineMusic::PCMFrequency);
			if ( loga < logahigh )
			{
				double degree = (logahigh - loga)/spreadfactor*0.8;
				degree *= degree;
				maskingval = PowerSpectrum.Arr[i]*degree;
				if ( MaskingSpectrum.Arr[p] < maskingval ) MaskingSpectrum.Arr[p] = maskingval;
			} else break;
		}

	}*/


















ULONG WINAPI MgineEngine::LoopUpdate(void *Temp)
{
	for (; !IsStarted; ) Sleep(1);
	EnterCriticalSection(&MgineEngine::csDraw);
	MgineScreen::First();
	LeaveCriticalSection(&MgineEngine::csDraw);

	LONGLONG stacked = 0;
	LONGLONG qffreq = 0;

	if ( !QueryPerformanceFrequency((LARGE_INTEGER*)&qffreq) || qffreq == 0 )
	{
		MgineLog::WriteLog(L"QPF failed on #2. Check your system supports high resolution timers.");
		MgineEngine::Uninit();
		return 0;
	}
	double tickStart = MF::GetDoubleTick(qffreq);

	for (;;) 
	{
		//WaitForSingleObject(HTimerUpdate, 100);
		if ( IsTerminating ) return 0;

		double tick = MF::GetDoubleTick(qffreq);
		if ( tick < tickStart+Setting->GetSetting().TargetElapseTime )
		{
			MF::DelayExecution((float)(tickStart+Setting->GetSetting().TargetElapseTime-tick));
		}

		tick = MF::GetDoubleTick(qffreq);
		stacked = (LONGLONG)((tick - tickStart) / MgineEngine::Setting->GetSetting().TargetElapseTime);

		if ( stacked > 0 )
		{
			EnterCriticalSection(&MgineEngine::csDraw);

			MGINE_SETTING setting = MgineEngine::Setting->GetSetting();
			MgineSound::Update();

			MgineInput::CheckAcquire();
			MgineInput::Update((int)stacked);
			if ( setting.OnUpdate ) setting.OnUpdate((int)stacked);
			MgineComponent::Update((int)stacked);
			MgineScreen::Update((int)stacked);
			if ( setting.OnUpdateAfter ) setting.OnUpdateAfter((int)stacked);
			tickStart += stacked * (MgineEngine::Setting->GetSetting().TargetElapseTime);

			LeaveCriticalSection(&MgineEngine::csDraw);
		}
		SwitchToThread(); // if there are same or higher priority of thread, yield the execution
	}
	
	return 0;
}















ULONG WINAPI MgineEngine::LoopDraw(void *Temp)
{
	for (; !IsStarted; ) Sleep(1);

	LONGLONG stacked = 0;
	LONGLONG qffreq = 0;

	if ( !QueryPerformanceFrequency((LARGE_INTEGER*)&qffreq) || qffreq == 0 )
	{
		MgineLog::WriteLog(L"QPF failed on #1. Check your system supports high resolution timers.");
		MgineEngine::Uninit();
		return 0;
	}
	double tickStart = MF::GetDoubleTick(qffreq);

	for (;;)
	{
		//WaitForSingleObject(HTimerDraw, 100);
		if ( IsTerminating ) return 0;
		
		double tick = MF::GetDoubleTick(qffreq);
		if ( tick < tickStart+(1000.f/DisplayMode.RefreshRate) )
		{
			MF::DelayExecution((float)(tickStart+(1000.f/DisplayMode.RefreshRate)-tick));
		}
		
		tick = MF::GetDoubleTick(qffreq);
		stacked = (LONGLONG)((tick - tickStart) / (1000./DisplayMode.RefreshRate));
		
		if ( stacked > 0 )
		{
			SubReloadDevice();
			if ( IsTerminating ) return 0;
			MgineScreen::Draw();
			tickStart += stacked * (1000./DisplayMode.RefreshRate);
		}
		SwitchToThread(); // if there are same or higher priority of thread, yield the execution
	}
	return 0;
}








void MgineEngine::ProcMovingWindow()
{
	if ( !MoveWindowState.IsMovingWindow ) return;
	if ( !MgineInput::Key->IsMouseDown(MGINE_MOUSE_LEFT) )
	{
		MoveWindowState.IsMovingWindow = false;
		return;
	}
	
	POINT ptcursor = { 0, };
	if ( !GetCursorPos(&ptcursor) ) return;

	RECT wndrect = { 0, };
	if ( !GetWindowRect(hWnd, &wndrect) ) return;
	
	
	if ( ptcursor.x + MoveWindowState.CoordOffset.x != wndrect.left || 
		ptcursor.y + MoveWindowState.CoordOffset.y != wndrect.top )
	{
		SetWindowPos(hWnd, 0, 
			ptcursor.x + MoveWindowState.CoordOffset.x,
			ptcursor.y + MoveWindowState.CoordOffset.y,
			RectWindow.right - RectWindow.left, 
			RectWindow.bottom - RectWindow.top,
			NULL);
	}
}









bool InitFromTexture(clsTextureStatic *Texture);
bool clsTextureStatic::InitFromTexture(clsTextureStatic *Texture)
{
	MgineLog::WriteLog(L"%s: Not well implemented yet. Have this pretty int 3", __FUNCTIONW__);
	__asm int 3;
	if ( IsInited ) Uninit();
	ParamInit.IsRenderTarget = false;
	if ( FAILED(D3DXCreateTexture(MgineEngine::Engine->gDevice,
		Texture->ImageDesc.Width, Texture->ImageDesc.Height,
		0, Texture->TextureDesc.Usage | (ParamInit.IsRenderTarget ? D3DUSAGE_RENDERTARGET : 0), Texture->TextureDesc.Format, ParamInit.IsRenderTarget ? D3DPOOL_DEFAULT : D3DPOOL_MANAGED,
		&gTexture ))
		) goto cleanup;
	if ( FAILED( gTexture->GetSurfaceLevel(0, &gSurface) ) ) goto cleanup;
	if ( FAILED( gSurface->GetDesc(&TextureDesc) ) ) goto cleanup;
	memcpy(&ImageDesc, &Texture->ImageDesc, sizeof(D3DXIMAGE_INFO));

	if ( FAILED( MgineEngine::Engine->gDevice->UpdateTexture(Texture->gTexture, gTexture) ) ) goto cleanup;
	//if ( FAILED( MgineEngine::Engine->gDevice->CreateVertexBuffer(sizeof(Vertex), 0, D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1, IsRenderTarget ? D3DPOOL_DEFAULT : D3DPOOL_MANAGED, &gVertexBuffer, NULL) ) ) goto cleanup;
	
	IsInited = true;
	IsDeviceLost = false;
	if ( !SetDefault() ) goto cleanup;
	return true;

cleanup:
	MgineLog::WriteLog(L"%s: Failed", __FUNCTIONW__);
	Uninit();
	return false;
}










	bool AdjustTexelToPixel();
	bool AdjustPixelToTexel();
bool clsTextureStatic::AdjustTexelToPixel()
{
	/*
	Vertex.V1.X += -0.5f;
	Vertex.V2.X += -0.5f;
	Vertex.V3.X += -0.5f;
	Vertex.V4.X += -0.5f;

	Vertex.V1.Y += -0.5f;
	Vertex.V2.Y += -0.5f;
	Vertex.V3.Y += -0.5f;
	Vertex.V4.Y += -0.5f;
	*/
	//void *pLock = 0;
	//gVertexBuffer->Lock( 0, sizeof(Vertex), &pLock, 0);
	//memcpy(pLock, &Vertex, sizeof(Vertex));
	//gVertexBuffer->Unlock();

	return true;
}

bool clsTextureStatic::AdjustPixelToTexel()
{
	/*
	Vertex.V1.X += 0.5f;
	Vertex.V2.X += 0.5f;
	Vertex.V3.X += 0.5f;
	Vertex.V4.X += 0.5f;

	Vertex.V1.Y += 0.5f;
	Vertex.V2.Y += 0.5f;
	Vertex.V3.Y += 0.5f;
	Vertex.V4.Y += 0.5f;
	*/
	//void *pLock = 0;
	//gVertexBuffer->Lock( 0, sizeof(Vertex), &pLock, 0);
	//memcpy(pLock, &Vertex, sizeof(Vertex));
	//gVertexBuffer->Unlock();

	return true;
}






















#include "Mgine.h"

Queue<clsRegionEffect*> *MgineRegionEffect::iQueueRegionEffect;


Queue<clsRegionEffect*> *MgineRegionEffect::QueueRegionEffect()
{
	if ( !MgineEngine::Core->IsAppFinished && !iQueueRegionEffect ) iQueueRegionEffect = new Queue<clsRegionEffect*>;
	return iQueueRegionEffect;
}

bool MgineRegionEffect::Init()
{
	MgineLog::WriteLog(L"%s: Begin", __FUNCTIONW__);
	QueueRegionEffect();
	return true;
}

void MgineRegionEffect::Uninit()
{
	for ( int i=0; i < QueueRegionEffect()->Size; i++ ){
		if ( QueueRegionEffect()->Arr[i] ){
			((clsRegionEffect*)QueueRegionEffect()->Arr[i])->Uninit();
		}
	}
}

void MgineRegionEffect::OnAppFinish()
{
	if ( iQueueRegionEffect ) delete iQueueRegionEffect;
	iQueueRegionEffect = 0;
}


clsRegionEffect::~clsRegionEffect()
{
	MgineRegionEffect::QueueRegionEffect()->Sub(QueueIndex);
	Uninit();
}

bool clsRegionEffect::Init()
{
	IsInited = true;
	return true;
}

void clsRegionEffect::Uninit()
{
	IsInited = false;
}

bool clsRegionEffect::SetEffectTextureParameter(clsEffect *Effect, MGINE_RECT DestRect, IDirect3DBaseTexture9 *SamplerA, D3DXIMAGE_INFO *ImageDesc)
{
	D3DXIMAGE_INFO localImageDesc;
	if ( !Effect ) return true;
	if ( !ImageDesc ) localImageDesc = MgineRenderer::TemporaryRenderTargetTexture[0].ImageDesc;
	else localImageDesc = *ImageDesc;

	float f[2];
	Effect->SetParameter("SamplerA", MGINE_VALUE_TEXTURE, SamplerA, 0);
	f[0] = (float)(MgineRenderer::Viewport.Width / (DestRect.Width));
	f[1] = (float)(MgineRenderer::Viewport.Height / (DestRect.Height));
	Effect->SetParameter("ViewportRatio", MGINE_VALUE_FLOAT_ARRAY, f, 2);
	f[0] = (float)(localImageDesc.Width / (DestRect.Width));
	f[1] = (float)(localImageDesc.Height / (DestRect.Height));
	Effect->SetParameter("TextureRatio", MGINE_VALUE_FLOAT_ARRAY, f, 2);
	f[0] = (float)(1.0f / (DestRect.Width));
	f[1] = (float)(1.0f / (DestRect.Height));
	Effect->SetParameter("UnitTexture", MGINE_VALUE_FLOAT_ARRAY, f, 2);
	f[0] = (float)(DestRect.X / MgineRenderer::Viewport.Width);
	f[1] = (float)(DestRect.Y / MgineRenderer::Viewport.Height);
	Effect->SetParameter("Location", MGINE_VALUE_FLOAT_ARRAY, f, 2);
	return true;
}


void clsRegionEffect::Update(int Stacked)
{
	if ( IsStarted && LoopCount ){
		Progress += Stacked;
		if ( Progress >= MaxProgress ){
			if ( LoopCount > 0 ){
				LoopCount -= Progress / MaxProgress;
				if ( LoopCount < 0 ) LoopCount = 0;
			}
			if ( LoopCount ) Progress %= MaxProgress;
			
			if ( !LoopCount && Progress > MaxProgress )
				Progress = MaxProgress;
		}
	} else if ( IsStarted ) IsStarted = false;

}

bool clsRegionEffect::Draw()
{
	// uses TemporaryRenderTargetTexture[0~1] as disposable
	if ( !IsVisible ) return true;
	clsRestorerPixelShader RestorePixelShader;
	clsRestorerRenderTarget RestoreRenderTarget;
	clsRestorerBlendState RestoreBlendState;

	MgineRenderer::SetBlendStateOverlay();

	
	if ( QueueEffect.Count == 0 ){
		return true;
	} else {
		VERTEX_TEXTURE vertex;
		MGINE_RECT rectd, rects;
		RECT r;
		int i, effectcount = 0;

		vertex.Init();
		rects.X = rectd.X = Region.X;
		rects.Y = rectd.Y = Region.Y;
		rects.Width = rectd.Width = Region.Width;
		rects.Height = rectd.Height = Region.Height;

		r.left = (LONG)rects.X;
		r.top  = (LONG)rects.Y;
		r.right  = (LONG)(rects.X + rects.Width);
		r.bottom = (LONG)(rects.Y + rects.Height);

		D3DXIMAGE_INFO originalImageDesc;
		D3DSURFACE_DESC descSurface;
		descSurface.Width  = 0;
		descSurface.Height = 0;
		//RestoreRenderTarget.OriginalRenderTarget->GetDesc(&descSurface);
		memset(&originalImageDesc, 0, sizeof(originalImageDesc));
		originalImageDesc.Width = descSurface.Width;
		originalImageDesc.Height = descSurface.Height;

		/*MgineRenderer::SetTexture(0);
		MgineRenderer::SetPixelShader(NULL);
		MgineRenderer::SetRenderTarget(MgineRenderer::TemporaryRenderTargetTexture[(effectcount+2-1) % 2].GetSurface());
		vertex.Set(rects, rectd, &MgineRenderer::TemporaryRenderTargetTexture[(effectcount+2-1) % 2].ImageDesc);
		MgineRenderer::DrawQuad((VERTEX_DOT*)&vertex, 4);*/
		D3DXLoadSurfaceFromSurface(
			MgineRenderer::TemporaryRenderTargetTexture[(effectcount) % 2].GetSurface(),
			0, 0,
			MgineRenderer::BackBuffer,
			0, 0,
			D3DX_FILTER_NONE,
			0
		);

		for ( i=0; i < QueueEffect.Size; i++ ){
			if ( QueueEffect.Arr[i] ){
				clsEffect *effect = ((clsEffect*)QueueEffect.Arr[i]);
				effect->Apply();

				if ( QueueEffect.Count == 1 ){

					RestoreBlendState.Restore();
					SetEffectTextureParameter(effect, rectd, MgineRenderer::TemporaryRenderTargetTexture[(effectcount) % 2].GetTexture(), &originalImageDesc);

					//MgineRenderer::SetRenderTarget(RestoreRenderTarget.OriginalRenderTarget);
					//vertex.Set(rects, rectd, &MgineRenderer::TemporaryRenderTargetTexture[(effectcount) % 2].ImageDesc);
					MgineRenderer::DrawQuad(&vertex, 1);
					MgineRenderer::RenderBatch();
				} else if ( effectcount == 0 || QueueEffect.Count -1 < effectcount ){
					// firstly
					
					SetEffectTextureParameter(effect, rectd, MgineRenderer::TemporaryRenderTargetTexture[(effectcount) % 2].GetTexture(), &MgineRenderer::TemporaryRenderTargetTexture[(effectcount+1) % 2].ImageDesc);

					MgineRenderer::SetRenderTarget(MgineRenderer::TemporaryRenderTargetTexture[(effectcount+1) % 2].GetSurface());
					//vertex.Set(rects, rectd, &MgineRenderer::TemporaryRenderTargetTexture[(effectcount) % 2].ImageDesc);
					if ( !MgineRenderer::Clear(0, true, false, false) ) goto cleanup;
					MgineRenderer::DrawQuad(&vertex, 1);
					MgineRenderer::RenderBatch();
				} else if ( QueueEffect.Count -1 == effectcount ){
					// final
					RestoreBlendState.Restore();
					SetEffectTextureParameter(effect, rectd, MgineRenderer::TemporaryRenderTargetTexture[(effectcount) % 2].GetTexture(), &originalImageDesc);
					
					//MgineRenderer::SetRenderTarget(RestoreRenderTarget.OriginalRenderTarget);
					//vertex.Set(rects, rectd, &MgineRenderer::TemporaryRenderTargetTexture[(effectcount) % 2].ImageDesc);
					MgineRenderer::DrawQuad(&vertex, 1);
					MgineRenderer::RenderBatch();
				/*} else {
					// progress

					SetEffectTextureParameter(effect, rectd, (IDirect3DTexture9*)&MgineRenderer::TemporaryRenderTargetTexture[(effectcount + 2 - 1) % 2].ImageDesc);
					//effect->SetParameter("SamplerA", MgineRenderer::TemporaryRenderTargetTexture[(effectcount + 2 - 1) % 2].gTexture);

					MgineRenderer::SetRenderTarget(MgineRenderer::TemporaryRenderTargetTexture[effectcount % 2].gSurface);
					if ( !MgineRenderer::Clear(0) ) goto cleanup;
					MgineRenderer::DrawQuad((VERTEX_DOT*)&vertex, 4);
					MgineRenderer::RenderBatch();*/
				}
				

				effectcount ++;
			}
			if ( effectcount >= QueueEffect.Count ) break;
		}
	}
	
	return true;

cleanup:
	MgineLog::WriteLog(L"%s: Failed", __FUNCTIONW__);
	return true;
}

void clsRegionEffect::Start(int MaxProgress, int LoopCount)
{
	if ( MaxProgress == 0 ) return ;
	this->MaxProgress = MaxProgress;
	this->LoopCount = LoopCount;
	this->Progress = 0;
	this->IsStarted = true;

	return ;
}

bool clsRegionEffect::Stop(bool IsReset)
{
	this->IsStarted = false;
	this->Progress = IsReset ? 0 : this->Progress;
	return true;
}

int clsRegionEffect::AddEffect(clsEffect *Effect)
{
	return QueueEffect.Add(Effect);
}





class MgineRegionEffect
{
private:
	static Queue<clsRegionEffect*> *iQueueRegionEffect;

public:
	static Queue<clsRegionEffect*> *QueueRegionEffect();
	static bool Init();
	static void Uninit();
	static void OnAppFinish();
};

class clsRegionEffect
{
private:

public:
	bool IsInited, IsStarted, IsVisible;
	Queue<clsEffect*> QueueEffect;
	DWORD QueueIndex;
	int Progress, MaxProgress, LoopCount;
	MGINE_RECT Region;

	clsRegionEffect()
	{
		QueueIndex = MgineRegionEffect::QueueRegionEffect()->Add(this);

		Progress = LoopCount = 0;
		MaxProgress = 1;

		IsInited = false;
		IsStarted = false;
		IsVisible = true;
	}
	~clsRegionEffect();

	bool Init();
	void Uninit();

	bool SetEffectTextureParameter(clsEffect *Effect, MGINE_RECT DestRect, IDirect3DBaseTexture9 *SamplerA, D3DXIMAGE_INFO *ImageDesc);

	void Update(int Stacked);
	bool Draw();

	void Start(int MaxProgress, int LoopCount);
	bool Stop(bool IsReset);

	int AddEffect(clsEffect *Effect);
};






void EngineManager::WindowIntegration::DC_DESKTOP::Update()
{
	LONGLONG tick = MF::GetLongTick();
	if ( TickLastUpdate+200 >= tick )
	{
		return;
	} else 
	{
		TickLastUpdate = tick;
	}
	if ( Core->IsReloading ) return;
	if ( Setting->GetSetting().IsUseDesktopDC )
	{
		if ( !hWndDesktop || !IsWindow(hWndDesktop) )
		{
			/*ReleaseDC(hWndDesktop, hDCDesktop);*/
			HWND newhwnd = 0;
			hDCDesktop = 0;

			HWND parent = NULL;
			while ( 1 )
			{
				parent = FindWindowEx(0, parent, L"WorkerW", NULL);
				if ( parent )
				{
					newhwnd = FindWindowEx(parent, 0, L"SHELLDLL_DefView", L"");
					newhwnd = FindWindowEx(newhwnd, 0, L"SysListView32", L"FolderView");
					if ( newhwnd ) break;
				} else break;
			}


			if ( !newhwnd )
			{
				newhwnd = FindWindow(L"Progman", L"Program Manager");
				newhwnd = FindWindowEx(newhwnd, 0, L"SHELLDLL_DefView", L"");
				newhwnd = FindWindowEx(newhwnd, 0, L"SysListView32", L"FolderView");
			}

			hWndDesktop = newhwnd;
		}
		if ( !hDCDesktop )
		{
			if ( hWndDesktop )
				hDCDesktop = GetDC(hWndDesktop);
			else
			{
				if ( Texture.Item && Texture.Item->IsInited )
				{
					IDirect3DSurface9 *surface = Texture.Item->GetSurface();
					HDC hdcsurf = 0;
					if ( SUCCEEDED(surface->GetDC(&hdcsurf)) )
					{
						LOGBRUSH lb;
						lb.lbColor = 0;
						lb.lbHatch = 0;
						lb.lbStyle = 0;
						HBRUSH hbrush = CreateBrushIndirect(&lb);
						RECT rct = { };
						rct.left = 0;
						rct.top  = 0;
						rct.right  = (LONG)RectDC.Width;
						rct.bottom = (LONG)RectDC.Height;
						FillRect(hdcsurf, &rct, hbrush);
						DeleteObject(hbrush);
						BitBlt(hdcsurf, 0, 0, (int)RectDC.Width, (int)RectDC.Height, hDCDesktop, (int)RectDC.X, (int)RectDC.Y, SRCCOPY | CAPTUREBLT);
						//BitBlt(hdcsurf, 0, 0, (int)RectDC.Width, (int)RectDC.Height, hDCRead, 0, 0, SRCCOPY | CAPTUREBLT);
						surface->ReleaseDC(hdcsurf);
					}
				}
			}
		}

		//tagLVBKIMAGEW
		/*LVBKIMAGEW bkimg = { 0 };
		bkimg.ulFlags = LVBKIF_SOURCE_URL | LVBKIF_STYLE_NORMAL;
		bkimg.pszImage = L"C:\\Users\\Laie\\Desktop\\b.bmp";
		bkimg.cchImageMax = wcslen(bkimg.pszImage);
		ListView_SetBkImage(hWndDesktop, &bkimg);*/
		
		//SetWindowLong(hWndDesktop, GWL_EXSTYLE, GetWindowLong(hWndDesktop, GWL_EXSTYLE) | WS_EX_LAYERED | WS_EX_TRANSPARENT);
		//SetWindowLong(hWndDesktop, GWL_STYLE, GetWindowLong(hWndDesktop, GWL_STYLE) | WS_MAXIMIZEBOX | WS_POPUPWINDOW | WS_BORDER | WS_THICKFRAME);
		

		ListView_SetBkColor(hWndDesktop, CLR_NONE);

		::SetBkMode(hDCDesktop,       TRANSPARENT); 
		::TextOut(hDCDesktop,       100,       100,       L"123 ",       3); 
		//ListView_SetTextBkColor(hWndDesktop, 0xffffffff);


		int objectivewidth = 0,
			objectiveheight = 0;

		{
			LockBlock LockBlock(Window->CsWindow);
			objectivewidth = Renderer::GetResolutionWidth() + CacheMargin*2 + (-RectWindow.left+RectWindow.right-Renderer::GetResolutionWidth())*2;
			objectiveheight = Renderer::GetResolutionHeight() + CacheMargin*2 + (-RectWindow.top+RectWindow.bottom-Renderer::GetResolutionHeight())*2;
		}

		if ( 
			(hDCDesktop && !hDCRead) 
			|| (RectDC.Width != objectivewidth
				&& RectDC.Height != objectiveheight )
			)
		{
			RectDC.Width  = (VEC)objectivewidth;
			RectDC.Height = (VEC)objectiveheight;

			if ( hDCRead ) DeleteDC(hDCRead);
			//if ( hDIB ) DeleteObject(hDIB);
			hDCRead = NULL;
			//hDIB = NULL;

			hDCRead = CreateCompatibleDC(hDCDesktop);

			BITMAPINFO bitmapinfo = { };

			bitmapinfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
			bitmapinfo.bmiHeader.biWidth = (LONG)RectDC.Width;
			bitmapinfo.bmiHeader.biHeight = -(LONG)RectDC.Width;
			bitmapinfo.bmiHeader.biBitCount = 32;
			bitmapinfo.bmiHeader.biCompression = BI_RGB;
			bitmapinfo.bmiHeader.biPlanes = 1;

			//hDIB = CreateDIBSection(hDCRead, &bitmapinfo, DIB_RGB_COLORS, (void**)&ImageBuffer, NULL, 0);
			//HGDIOBJ oldobj = SelectObject(hDCRead, hDIB);
		}
		if ( hDCRead )
		{
			{
				LockBlock Lock(CsWindow);
				RectDC.X = (VEC)PosWindow.x-CacheMargin;
				RectDC.Y = (VEC)PosWindow.y-CacheMargin;
			}

			//BitBlt(hDCRead, 0, 0, (int)RectDC.Width, (int)RectDC.Height, hDCDesktop, (int)RectDC.X, (int)RectDC.Y, SRCCOPY | CAPTUREBLT);
			{
				if ( !Texture.Item ) 
					Texture.Set(new TextureDynamic);
			
				if ( !Texture.Item->IsInited || RectDC.Width != Texture.Item->ImageDesc.Width || RectDC.Height != Texture.Item->ImageDesc.Height )
				{
					Texture.Item->InitFillColor(0x00000000, (DWORD)RectDC.Width, (DWORD)RectDC.Height);
					Texture.Item->Drawer.Origin.X = 0;
					Texture.Item->Drawer.Origin.Y = 0;
				}

				if ( Texture.Item->IsInited )
				{
					IDirect3DSurface9 *surface = Texture.Item->GetSurface();
					HDC hdcsurf = 0;
					if ( SUCCEEDED(surface->GetDC(&hdcsurf)) )
					{
						BitBlt(hdcsurf, 0, 0, (int)RectDC.Width, (int)RectDC.Height, hDCDesktop, (int)RectDC.X, (int)RectDC.Y, SRCCOPY | CAPTUREBLT);
						//BitBlt(hdcsurf, 0, 0, (int)RectDC.Width, (int)RectDC.Height, hDCRead, 0, 0, SRCCOPY | CAPTUREBLT);
						surface->ReleaseDC(hdcsurf);
					}
				}
				/*
				D3DLOCKED_RECT locked;
				if ( SUCCEEDED(surface->LockRect(&locked, 0, 0)) )
				{
					for ( int y=0; y < (int)Sync.RectDC.Height; y++ )
					{
						for ( int x=0; x < (int)Sync.RectDC.Width; x++ )
						{
							*((DWORD*)((BYTE*)locked.pBits+locked.Pitch*y)+x) = *(ImageBuffer+x+y*(int)Sync.RectDC.Width)+0x80101010;
						}
					}
					surface->UnlockRect();
				}*/
			}
		} else 
		{

		}
	}
}

void EngineManager::WindowIntegration::DC_DESKTOP::Draw(int X, int Y, int U, int V, int Width, int Height)
{
	int offsetx = 0, offsety = 0;
	int x, y;
	x = U - (int)RectDC.X;
	y = V - (int)RectDC.Y;

	int maxx = (int)RectDC.X + (int)RectDC.Width,
		maxy = (int)RectDC.Y + (int)RectDC.Height;
	if ( maxx > U + Width+CacheMargin ) maxx = U + Width+CacheMargin;
	if ( maxy > V + Height+CacheMargin ) maxy = V + Height+CacheMargin;

	if ( U < 0 ) offsetx = -U;
	if ( V < 0 ) offsety = -V;



	/*int xfirst = x;
		
	static UnwindArray<VERTEX_DOT> dots;
	static int dotssz = 0;
	if ( dotssz < Sync.RectDC.Height*Sync.RectDC.Width ) 
	{
		dots.Set(new VERTEX_DOT[(DWORD)(Sync.RectDC.Height*Sync.RectDC.Width)]);
		dotssz = (int)(Sync.RectDC.Height*Sync.RectDC.Width);
	}
	
	int dotcount = 0;*/

	/*for ( ; y < Sync.RectDC.Y + Sync.RectDC.Height && y < V + Height+CacheMargin; y++ )
	{
		for ( x = xfirst; x < Sync.RectDC.X + Sync.RectDC.Width && x < U + Width+CacheMargin; x++ ) 
		{
			if ( (y-(int)Sync.RectDC.Y-CacheMargin < 0 ||
					Sync.RectDC.Height <= y-(int)Sync.RectDC.Y-CacheMargin) ||
				(x-(int)Sync.RectDC.X-CacheMargin < 0 ||
					Sync.RectDC.Width <= x-(int)Sync.RectDC.X-CacheMargin)
				)
				continue;
			
			dots[dotcount].Init();
			dots[dotcount].X = (VEC)x + X-U-CacheMargin;
			dots[dotcount].Y = (VEC)y + Y-V-CacheMargin;
			//if ( y ) __asm int 3;
			dots[dotcount].ColorDiffuse = *(Sync.ImageBuffer.Arr + 
				(y-(int)Sync.RectDC.Y-CacheMargin)*(int)Sync.RectDC.Width 
				+ (x-(int)Sync.RectDC.X-CacheMargin)
				);
			//if ( dot.ColorDiffuse ) __asm int 3;
			//if ( ((dot.ColorDiffuse >> 24) & 0xff) == 0 )
			//dots[dotcount].ColorDiffuse |= 0xff000000;
			dotcount++;
		}
	}
	Renderer::DrawPoint(dots.Arr, dotcount);*/
	if ( !Texture.Item ) return;
	Texture.Item->Drawer.Origin.X = 0;
	Texture.Item->Drawer.Origin.Y = 0;
	Texture.Item->Drawer.RectDest.X = (VEC)X+offsetx;
	Texture.Item->Drawer.RectDest.Y = (VEC)Y+offsety;
	Texture.Item->Drawer.RectSource.X = (VEC)x+offsetx;// + X+U-CacheMargin;
	Texture.Item->Drawer.RectSource.Y = (VEC)y+offsety;// + Y+V-CacheMargin;
	Texture.Item->Drawer.RectDest.Width = (VEC)Width-offsetx;
	Texture.Item->Drawer.RectDest.Height = (VEC)Height-offsety;
	Texture.Item->Drawer.RectSource.Width = (VEC)Width-offsetx;
	Texture.Item->Drawer.RectSource.Height = (VEC)Height-offsety;
	Texture.Item->Draw();
}

EngineManager::WindowIntegration::DC_DESKTOP::~DC_DESKTOP()
{
	//SelectObject(hdc, oldobj);
	//DeleteObject(hDIB);
	DeleteDC(hDCRead);
	ReleaseDC(hWndDesktop, hDCDesktop);

	hWndDesktop = NULL;
	hDCDesktop = NULL;
	hDCRead = NULL;
	//hDIB = NULL;
	//ImageBuffer = NULL;
}

			static struct DC_DESKTOP
			{
				static const int CacheMargin = 50;

				LONGLONG	TickLastUpdate;

				HWND	hWndDesktop;
				HDC		hDCDesktop;
				HDC		hDCRead;
				MGINE_RECT RectDC;
			
				UnwindItem<TextureStatic> Texture;

				void Update(); // should be called in window thread
				void Draw(int X, int Y, int U, int V, int Width, int Height);
				~DC_DESKTOP();
			} DesktopDC;





	class EventArg
	{
	public:
		void *SenderObject;
		bool IsHandled;

		inline EventArg()
		{
			this->SenderObject = NULL;
			this->IsHandled = false;
		}
	};

	class MouseEventArg
		: public EventArg
	{
	public:
		MGINE_MOUSE_STATE State;

		inline MouseEventArg(MGINE_MOUSE_STATE State)
			: EventArg()
		{
			this->State = State;
		}
	};

	class KeyEventArg
		: public EventArg
	{
	public:
		MGINE_KEYCODE Key;
		MGINE_KEYSTATE KeyState;

		inline KeyEventArg(MGINE_KEYCODE Key, MGINE_KEYSTATE KeyState)
			: EventArg()
		{
			this->Key = Key;
			this->KeyState = KeyState;
		}
	};

	class CharEventArg
		: public EventArg
	{
	public:
		MGINE_CHARCODE Char; // Character pressed. Captured by WndProc

		inline CharEventArg(MGINE_CHARCODE Char)
			: EventArg()
		{
			this->Char = Char;
		}
	};

	class ItemSelectEventArg
		: public EventArg
	{
	public:
		enum ITEM_SELECT_STATE : BYTE
		{
			Unselected = 0,
			Selected = 1
		} State;

		UIListItem *Item;

		inline ItemSelectEventArg(UIListItem *ListItem, ITEM_SELECT_STATE State)
		{
			this->State = State;
			this->Item  = ListItem;

			assert(Item);
		}
	};

	class ItemActionEventArg // Double clicked, entered, clearly decided.
		: public EventArg
	{
	public:
		enum ITEM_ACTION_STATE : BYTE
		{
			EnterKey = 0,
			DblClick = 1
		} State;

		UIListItem *Item;

		inline ItemActionEventArg(UIListItem *ListItem, ITEM_ACTION_STATE State)
		{
			this->State = State;
			Item = ListItem;

			assert(Item);
		}
	};

	class PositionEventArg
		: public EventArg
	{
	public:
		MGINE_RECT OldPosition;

		inline PositionEventArg(MGINE_RECT& OldPosition)
		{
			this->OldPosition = OldPosition;
		}

	};

	/*\
	 *	Event
	 *		Event dispatching class for native C++.
	 *		Event Handler method can be member function, and must have just ONE parameter
	\*/
	class Event
	{
	private:
		virtual int AddHandlerRaw(void *Handler, void *ObjectPointer);

	public:
		struct CHUNK
		{
			void *Handler;
			void *ObjectPointer;
		};
	
		TriList<CHUNK> ListEvent;

		
		inline Event() : ListEvent(16) { }

		/* !! Note that AddHandler() is not a virtual.
		 *	When you are doing inheritance(overriding), 
		 *	be sure you hide this method or just do work for AddHandlerRaw() 
		 * !!
		 *	*/
		template<typename TPROC>
		int AddHandler(TPROC Handler, void *ObjectPointer)
		{
			/* casting tricks to treat of member functions of classes */
			//auto fp = &clsMainScreen::OnClick;
			//void *v = &fp;
			void *casttmpvoid = &Handler;
			return AddHandlerRaw(*(void**)casttmpvoid, ObjectPointer);
		}

		virtual bool SubHandler(int Index);
	
		virtual void Raise(void *SenderObject, EventArg *Arg);
		virtual void Raise(void *SenderObject);


	};

	class MouseEvent
		: public Event
	{
	public:
		virtual void Raise(void *SenderObject, MouseEventArg *Arg);
	};

	class KeyEvent
		: public Event
	{
	public:
		virtual void Raise(void *SenderObject, KeyEventArg *Arg);
	};

	class CharEvent
		: public Event
	{
	public:
		virtual void Raise(void *SenderObject, CharEventArg *Arg);
	};

	class ItemSelectEvent // Selected, Deselected
		: public Event
	{
	public:
		virtual void Raise(void *SenderObject, ItemSelectEventArg *Arg);
	};

	class ItemActionEvent
		: public Event
	{
		virtual void Raise(void *SenderObject, ItemActionEventArg *Arg);
	};

	class PropertyEvent
		: public Event
	{
	public:
		virtual void Raise(void *SenderObject, PositionEventArg *Arg);
	};


/*\
 *	Event
\*/
int Event::AddHandlerRaw(void *Handler, void *ObjectPointer)
{
	CHUNK chunk;
	chunk.Handler = Handler;
	chunk.ObjectPointer = ObjectPointer;
	return ListEvent.Add(chunk); 
}

bool Event::SubHandler(int Index)
{ return ListEvent.Sub(Index); }

void Event::Raise(void *SenderObject, EventArg *Arg)
{
	if ( !ListEvent.GetElementCount() ) return;
	Arg->SenderObject = SenderObject;
	int i, count = 0;
	for ( i=0; i < ListEvent.GetTotalAllocated(); i++ )
	{
		CHUNK chunk;
		bool isexisting = ListEvent.Get(i, &chunk);
		void *object = chunk.ObjectPointer;
		void *handler = chunk.Handler;
		
		if ( isexisting )
		{
			__asm
			{
				push eax;
				push ecx;
				mov ecx,[object];
				push [Arg];
				call [handler];
			}
			__asm
			{
				pop ecx;
				pop eax;
			}
				
			count++;
			if ( count >= ListEvent.GetElementCount() )
				break;
		}
	}
}
void Event::Raise(void *SenderObject)
{
	EventArg arg;
	Raise(SenderObject, &arg);
}

/*\
 *	MouseEvent : Event
\*/
void MouseEvent::Raise(void *SenderObject, MouseEventArg *Arg)
{
	Arg->SenderObject = SenderObject;
	Event::Raise(SenderObject, Arg);
}

/*\
 *	KeyEvent : Event
\*/
void KeyEvent::Raise(void *SenderObject, KeyEventArg *Arg)
{
	Arg->SenderObject = SenderObject;
	Event::Raise(&Arg);
}

/*\
 *	CharEvent : Event
\*/
void CharEvent::Raise(void *SenderObject, CharEventArg *Arg)
{
	Arg->SenderObject = SenderObject;
	Event::Raise(&Arg);
}

/*\
 *	ItemSelectEvent : Event
\*/
void ItemSelectEvent::Raise(void *SenderObject, ItemSelectEventArg *Arg)
{
	Arg->SenderObject = SenderObject;
	Event::Raise(&Arg);
}

/*\
 *	ItemActionEvent : Event
\*/
void ItemActionEvent::Raise(void *SenderObject, ItemActionEventArg *Arg)
{
	Arg->SenderObject = SenderObject;
	Event::Raise(&Arg);
}

/*\
 *	PropertyEvent : Event
\*/
void PropertyEvent::Raise( void *SenderObject, PositionEventArg *Arg )
{
	Arg->SenderObject = SenderObject;
	Event::Raise(&Arg);
}















	/*\
	 *	clsLambda
	 *		This class stands for wrapping Lambda's wrappers.
	 *		Means, clsLambdaWhateverFunction's instance is directly corresponded to '[]{}(whatever)'.
	 *		And this is abstract version for those classes(clsLambdaWhateverFunction). To make get them in one array with using up/down casting.
	 *		clsLambdaWhateverFunction defines Execute() and its overloads to define how to call and treat lambda they are wrapping.
	 *		clsLambdaFunction defines nothing and just for inheritance.
	\*/
	class Lambda
	{
	public:
		/*\	
		 *	calling directly this method is useless.
		 *	cuz 'clsLambdaFunction' does not implements Execute() method to how to call the lambda that class is also for inheritance.
		 *	so, it is just for inheritance 
		 *  method for executing the lambda is not defined. the lambda will never be called even Execute() called
		\*/
		template<class F>
		static inline Lambda *WrapAllocLambda(const F & LambdaFunction)
		{
			return new LambdaFunction<F>(LambdaFunction);
		}

		virtual void Execute()
		{ }
		virtual ~Lambda(){ };
	};

	/*\
	 *	clsDefaultLambda
	 *		For void (*)(void)
	\*/
	class DefaultLambda
	{
	public:
		template<class F>
		static inline Lambda *WrapAllocLambda(const F & LambdaFunction)
		{
			return new LambdaDefaultFunction<F>(LambdaFunction);
		}

		/*\
		 *	Actually in this case(No arguments are there), this declaration is not necessarily needed.
		 *	But by declaration of overloads of Execute(), you can call lambda by clsDefaultLambda::Execute(overloads);
		 *	So if you wanna use argument for lambda, add below line.
		 *	virtual void Execute(ARG *Arg){  }
		 *	And override this method in clsLambdaDefaultFunction to specify how to call and treat the lambda.
		\*/
		virtual void Execute(){ }
	};

	template<class F>
	class LambdaFunction
		: public Lambda
	{
	public:
		F *LambdaExpression;

		LambdaFunction(const F & LambdaExpression)
		{ this->LambdaExpression = new F(LambdaExpression); }
		virtual ~LambdaFunction()
		{ if ( this->LambdaExpression ) delete this->LambdaExpression; }

		virtual void Execute(){ } // It doesn't call the Lambda. This just stand for inheritance.
	};

	/*\
	 *	clsLambdaDefaultFunction
	 *		For void (*)(void)
	\*/
	template<class F>
	class LambdaDefaultFunction
		: public LambdaFunction<F>, public DefaultLambda
	{
	public:
		LambdaDefaultFunction(const F & LambdaExpression)
			: LambdaFunction(LambdaExpression)
		{ }

		/*\
		 *	Reference comments of 'clsLambda' to get detail.
		 *	And override this method in clsLambdaDefaultFunction to specify how to call and treat the lambda.
		 *	For example, virtual void Execute(ARG *Arg){ (*LambdaExpression)(Arg); }
		 *	Execution method(especially arguments number) for a lambda should be one and the others should be empty.
		 *	Or compiler will occur an error.
		\*/
		virtual void Execute()
		{
			(*LambdaExpression)();
		}
	};

	template<>
	class LambdaDefaultFunction<int>
		: public LambdaFunction<int>, public DefaultLambda
	{
	public:
		LambdaDefaultFunction(int *LambdaExpression)
			: LambdaFunction(*LambdaExpression)
		{ }
		virtual void Execute()
		{ } // NULL Exception process
	};


















void UIComponent::ANCHOR::SetMyRect( RECTANGLE Rect )
{
	if ( MyRect.X != Rect.X ||
		MyRect.Y != Rect.Y ||
		MyRect.Width != Rect.Width ||
		MyRect.Height != Rect.Height )
	{
		// something's changed
		MyRect = Rect;
	}
}

void UIComponent::ANCHOR::SetParentRect( RECTANGLE Rect )
{
	if ( ParentRect.Width != Rect.Width ||
		ParentRect.Height != Rect.Height )
	{
		if ( ParentRect.Width != Rect.Width )
		{
			if ( Right )
			{
				MyRect.Width += Rect.Width-ParentRect.Width; // if increased, also increase
				if ( !Left )
				{
					MyRect.X += Rect.Width-ParentRect.Width;
				}
			} else if ( !Left )
			{
				MyRect.X = ((MyRect.X+MyRect.Width)/2)/(ParentRect.Width)*Rect.Width*2-MyRect.Width;
			}
		}

		if ( ParentRect.Height != Rect.Height )
		{
			if ( Bottom )
			{
				MyRect.Height += Rect.Height-ParentRect.Height; // if increased, also increase
				if ( !Top )
				{
					MyRect.Y += Rect.Height-ParentRect.Height;
				}
			} else if ( !Top )
			{
				MyRect.Y = ((MyRect.Y+MyRect.Height)/2)/(ParentRect.Height)*Rect.Height*2-MyRect.Height;
			}
		}

		ParentRect = Rect;
	}
}






/*
HRGN ComponentManager::GetAllocRegionFromBuffer(BYTE *Buffer, int Width, int Height)
{
	if ( !Buffer ) return NULL;
	if ( Width <= 0 || Height <= 0 ) return NULL;

	HRGN hRgn = NULL;
	// For better performances, we will use the ExtCreateRegion() function to create th
	// region. This function take a RGNDATA structure on entry. We will add rectangles b
	// amount of ALLOC_UNIT number in this structure #define ALLOC_UNIT 100
	DWORD maxRects = 10000;
	HANDLE hData = GlobalAlloc(GMEM_MOVEABLE, sizeof(RGNDATAHEADER) + (sizeof(RECT) * maxRects)); 
	RGNDATA *pData = (RGNDATA *)GlobalLock(hData);
	pData->rdh.dwSize = sizeof(RGNDATAHEADER);
	pData->rdh.iType  = RDH_RECTANGLES;
	pData->rdh.nCount = pData->rdh.nRgnSize = 0;
	//SetRect(&pData->rdh.rcBound, MAXLONG, MAXLONG, 0, 0);
	SetRect(&pData->rdh.rcBound, 0, 0, Width/2, Height/2);
	// Keep on hand highest and lowest values for the "transparent" pixel
	// Scan each bitmap row from bottom to top (the bitmap is inverted vertically
	BYTE *p32 = Buffer;
	for (int y = 0; y < Height; y++) {
		// Scan each bitmap pixel from left to righ
		for (int x = 0; x < Width; x++) {
			// Search for a continuous range of "non transparent pixels"
			int x0 = x;
			BYTE *p = (BYTE *)p32 + x;
			while (x < Width) {
				if ( *p == 0 ) 
					break;
				p++; x++;
			}
			if (x > x0) {
				// Add the pixels (x0, y) to (x, y+1) as a new rectangle in the regio
				if (pData->rdh.nCount >= maxRects) {
					GlobalUnlock(hData);
					maxRects += 10000;
					//VERIFY(
					hData = GlobalReAlloc(hData, sizeof(RGNDATAHEADER) + (sizeof(RECT) * maxRects), GMEM_MOVEABLE);
					pData = (RGNDATA *)GlobalLock(hData);
					//ASSERT(pData);
				}
				RECT *pr = (RECT *)&pData->Buffer;
				SetRect(&pr[pData->rdh.nCount], x0, y, x, y+1);
				//if (x0 < pData->rdh.rcBound.left)
					//pData->rdh.rcBound.left = x0;
				//if (y < pData->rdh.rcBound.top) pData->rdh.rcBound.top = y;
				//if (x > pData->rdh.rcBound.right) pData->rdh.rcBound.right = x;
				//if (y+1 > pData->rdh.rcBound.bottom) pData->rdh.rcBound.bottom = y+1;
				pData->rdh.nCount++;
				// On Windows98, ExtCreateRegion() may fail if the number of rectangles is to 
				// large (ie: > 4000). Therefore, we have to create the region by multiple steps 
				if (pData->rdh.nCount == 1000) {
					HRGN h = ExtCreateRegion(NULL, sizeof(RGNDATAHEADER) + (sizeof(RECT) * maxRects), pData);
					//ASSERT(h);
					if (hRgn) {
						CombineRgn(hRgn, hRgn, h, RGN_OR); 
						DeleteObject(h);
					} else hRgn = h;
					pData->rdh.nCount = 0;
					SetRect(&pData->rdh.rcBound, 0, 0, Width/2, Height/2);
				}
			}
		}
		// Go to next row (remember, the bitmap is inverted vertically 
		p32 += Width;
	} // Create or extend the region with the remaining rectangle 
	HRGN h = ExtCreateRegion(NULL, sizeof(RGNDATAHEADER) + (sizeof(RECT) * maxRects), pData);
	//ASSERT(h);
	if (hRgn) { 
		CombineRgn(hRgn, hRgn, h, RGN_OR); 
		DeleteObject(h); 
	} else hRgn = h;
	GlobalFree(hData);
	return hRgn; 
} */


/*
void UIComponent::RaiseUpdateChildPosition()
{
	if ( ChildPositioningType == MANUAL ) return ;
	DATHROW(PositioningRestrictLockCount >= 0);
	if ( PositioningRestrictLockCount ) return;

	try
	{
		struct LOCK_COUNTER
		{
			int & counter;
			LOCK_COUNTER(int & Counter):counter(Counter) { counter++; }
			~LOCK_COUNTER(){ counter--; }
		} lockcounter(PositioningRestrictLockCount);

		int i = 0;
		VEC x = 0, y = 0;
		MGINE_RECT pos = { 0, 0, 0, 0, };
		StepIndexer<UIComponent *> element;
		for ( ; ListComponent().Step(element); )
		{
			ROWCOL_DEF *align = NULL;
			bool isalignexisting = ListRowColDefinition().Get(element.Index, &align);

			if ( !isalignexisting ) align = &DefaultRowColDef.Value;
			ATHROW(align);

			ATHROWR(element->SelfPositioningType == UIComponent::SELF_POSITION_CUSTOM,
				"Self Positioning Type should be custom");
			VEC left = 0, right = 0,
				top = 0, bottom = 0;

			left = 0;
			right = element->LayoutOffset().Margin.left
				+ element->Position().Width
				+ element->LayoutOffset().Margin.right;
			top = 0;
			bottom = element->LayoutOffset().Margin.top
				+ element->Position().Height
				+ element->LayoutOffset().Margin.bottom;



			VEC width = 0, height = 0;
			switch ( ChildPositioningType() )
			{
			case HORIZONTAL:
				if ( align->MaxSize && right > align->MaxSize )
					right = align->MaxSize;
				if ( align->MinSize && right < align->MinSize )
					right = align->MinSize;
				if ( ActualPosition().Width < left+right )
					right = ActualPosition().Width-left;
				if ( align->Size < right )
					align->Size = right;
				width = align->Size;
				height = ActualPosition().Height;
				if ( height < bottom ) bottom = height;
				break;
			case VERTICAL:
				if ( (align->MaxSize && bottom > align->MaxSize) )
					bottom = align->MaxSize;
				if ( align->MinSize && bottom < align->MinSize )
					bottom = align->MinSize;
				if ( ActualPosition().Height < top+bottom )
					bottom = ActualPosition().Height-top;
				if ( align->Size < bottom )
					align->Size = bottom;
				width = ActualPosition().Width;
				height = align->Size;
				if ( width < right ) right = width;
				break;
			}


			switch ( align->HorizontalAlign )
			{
			case LEFT:
				break;
			case CENTER:
				left += (width - right)/2;
				right += (width - right)/2;
				break;
			case RIGHT:
				{
					VEC oldright = right;
					right += (width - right);
					left += right - oldright;
				}
				break;
			case STRETCH:
				left = 0;
				right = width;
				break;
			default:
				throw Exception<InvalidOperation>(L"%s{%d}: Horizontal Align is invalid in this state, %d", __FUNCTIONW__, __LINE__, align->HorizontalAlign);
				break;
			}

			switch ( align->VerticalAlign )
			{
			case TOP:
				break;
			case CENTER:
				top += (height - bottom)/2;
				bottom += (height - bottom)/2;
				break;
			case BOTTOM:
				{
					VEC oldbottom = bottom;
					bottom += (height - bottom);
					top = bottom - oldbottom;
				}
				break;
			case STRETCH:
				top = 0;
				bottom = height;
				break;
			default:
				throw Exception<InvalidOperation>(L"%s{%d}: Vertical Align is invalid in this state, %d", __FUNCTIONW__, __LINE__, align->HorizontalAlign);
				break;
			}

			pos.Width =
				(VEC)(
				right - left
				-element->LayoutOffset().Margin.left
				-element->LayoutOffset().Margin.right
				);
			pos.Height =
				(VEC)(
				bottom - top
				-element->LayoutOffset().Margin.top
				-element->LayoutOffset().Margin.bottom
				);

			pos.X = x + left + element->LayoutOffset().Margin.left;
			pos.Y = y + top + element->LayoutOffset().Margin.top;

			switch ( ChildPositioningType )
			{
			case HORIZONTAL:
				x += align->Size;
				break;
			case VERTICAL:
				y += align->Size;
				break;
			default:
				throw Exception<InvalidOperation>(L"%s{%d}: Invalid Positioning Type", __FUNCTIONW__, __LINE__);
			}


			element->Position = pos;
			++i;
		}
		if ( element.Index != -1 )
		{
			MGINE_RECT originpos = Position;
			if ( Position().X + Position().Width < x )
				originpos.Width += x - (originpos.X+originpos.Width);
			if ( Position().Y + Position().Height < y )
				originpos.Height += y - (originpos.Y+originpos.Height);
			//if ( Position() != originpos )
			// 				Position = originpos;
			
			Position = originpos;
		}
	}
	catch ( BaseException & )
	{
		Log::WriteLog(L"%s{%d}: Failed", __FUNCTIONW__, __LINE__);
		throw;
	}
	
}*/







	class AutomatedVal // this should be improved for far
	{
	public:
		int ValIndex;

		inline AutomatedVal()
		{
			ValIndex = ComponentManager::ListAutomatedVal().Add(this);
		};
		inline ~AutomatedVal()
		{
			ComponentManager::ListAutomatedVal().Sub(ValIndex);
		}

	public:
		virtual void Update(int Stacked) = NULL;
	};

	template<typename T = float>
	class SmoothizeVal
		: AutomatedVal
	{
	public:
		T Decay;
		T Val; // Objective
		T Cur; // Progress Following
		
		static_assert( (T)0.5 == 0.5, "Only floating types are allowed." );

		SmoothizeVal()
		{ 
			Decay = (T)0;
			Val = Cur = (T)0;
		}
		SmoothizeVal(T Value){ Reset(Value); }

		void Update(int Stacked)
		{
			if ( ABS(Cur-Val) < 1 ) Cur = Val;
			else if ( Stacked == 1 )
			{
				Cur += (Val-Cur)*1/Decay; 
			} else Cur += (Val-Cur)*1/pow(Decay, (T)Stacked); 
		}

		void Reset(){ Cur = Val; }
		void Reset(T Value){ Cur = Val = Value; }

		T& operator	=	(T &Value){ return Val = Value; }
		T& operator	<<	(T &Value){ return Val = Cur = Value; }
	};

	template<typename T = float>
	class StepizeVal
		: AutomatedVal // This system should be improved
	{
	public:
		T Step;
		T Val; // Objective
		T Cur; // Progress Following

		StepizeVal()
		{ 
			static_assert( (T)0.5 == 0.5, "Only floating types are allowed." );
			Step = (T)1;
			Val = Cur = (T)0;
		}
		StepizeVal(T Value){ Reset(Value); }

		void Update(int Stacked)
		{
			if ( ABS(Cur-Val) < Step ) Cur = Val;
			if ( Stacked == 1 )
			{
				if ( Cur < Val )
				{
					Cur += Step 
				} else 
				{
					Cur -= Step;
				}
			} else 
			{
				if ( Cur < Val )
				{
					if ( Cur + Step*Stacked > Val)
					{
						Cur = Val;
					} else Cur += Step 
				} else 
				{
					if ( Cur - Step*Stacked < Val)
					{
						Cur = Val;
					} else Cur -= Step 
				}
			}
		}

		void Reset(){ Cur = Val; }
		void Reset(T Value){ Cur = Val = Value; }

		T& operator	=	(T &Value){ return Val = Value; }
		T& operator	<<	(T &Value){ return Val = Cur = Value; }
	};

















bool TextureStatic::ProcInitFromFilePng(wchar_t *FileName)
{
	return false;
	png_byte fileheader[8];
	FILE *fp = 0;
	png_structp ptrread = NULL;
	png_infop   ptrinfo = NULL;

    png_uint_32 imgWidth  = 0;
    png_uint_32 imgHeight = 0;

    //bits per CHANNEL! note: not per pixel!
    png_uint_32 bitdepth   = 0;
    //Number of channels
    png_uint_32 channels   = 0;
    //Color type. (RGB, RGBA, Luminance, luminance alpha... palette... etc)
    png_uint_32 color_type = 0;

	png_bytep *rowPtrs = 0;
	char *data = 0;

	_wfopen_s(&fp, FileName, L"rb");
	if ( !fp ) goto cleanup;

	if ( fread(fileheader, 1, 8, fp) != 8 ) goto cleanup;
	if ( png_sig_cmp(fileheader, 0, 8) ) goto cleanup;

	ptrread = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if ( !ptrread ) goto cleanup;
	ptrinfo = png_create_info_struct(ptrread);
	if ( !ptrinfo ) goto cleanup;

	if ( setjmp(png_jmpbuf(ptrread)) )
	{
		goto cleanup;
	}

	png_init_io(ptrread, fp);
	png_set_sig_bytes(ptrread, 8);
	png_read_info(ptrread, ptrinfo);

    imgWidth  = png_get_image_width(ptrread, ptrinfo);
    imgHeight = png_get_image_height(ptrread, ptrinfo);

    //bits per CHANNEL! note: not per pixel!
    bitdepth   = png_get_bit_depth(ptrread, ptrinfo);
    //Number of channels
    channels   = png_get_channels(ptrread, ptrinfo);
    //Color type. (RGB, RGBA, Luminance, luminance alpha... palette... etc)
    color_type = png_get_color_type(ptrread, ptrinfo);

    switch (color_type) {
        case PNG_COLOR_TYPE_PALETTE:
            png_set_palette_to_rgb(ptrread);
            //Don't forget to update the channel info (thanks Tom!)
            //It's used later to know how big a buffer we need for the image
            channels = 3;           
            break;
        case PNG_COLOR_TYPE_GRAY:
            if (bitdepth < 8)
            png_set_expand_gray_1_2_4_to_8(ptrread);
            //And the bitdepth info
            bitdepth = 8;
            break;
    }


    /*if the image has a transperancy set.. convert it to a full Alpha channel..*/
//    if (png_get_valid(ptrread, ptrinfo, PNG_INFO_tRNS)) {
//        png_set_tRNS_to_alpha(ptrread);
//        channels += 1;
//    }

    //We don't support 16 bit precision.. so if the image Has 16 bits per channel
        //precision... round it down to 8.
    if (bitdepth == 16)
        png_set_strip_16(ptrread);
	
    //Array of row pointers. One for every row.
    rowPtrs = new png_bytep[imgHeight];

    //Alocate a buffer with enough space.
    //(Don't use the stack, these blocks get big easilly)
    //This pointer was also defined in the error handling section, so we can clean it up on error.
    data = new char[imgWidth * imgHeight * bitdepth * channels / 8];
    //This is the length in bytes, of one row.
    const unsigned int stride = imgWidth * bitdepth * channels / 8;
	const unsigned int unit   = bitdepth * channels / 8;
    //A little for-loop here to set all the row pointers to the starting
    //Adresses for every row in the buffer

    for (size_t i = 0; i < imgHeight; i++) {
        //Set the pointer to the data pointer + i times the row stride.
        //Notice that the row order is reversed with q.
        //This is how at least OpenGL expects it,
        //and how many other image loaders present the data.
        DWORD q = (i/*imgHeight- i - 1*/) * stride;
        rowPtrs[i] = (png_bytep)data + q;
    }

	png_read_image(ptrread, rowPtrs);

	//http://www.piko3d.com/tutorials/libpng-tutorial-loading-png-files-from-streams

	if ( FAILED(D3DXCreateTexture(EngineManager::Engine->gDevice, 
		imgWidth, imgHeight,
		0, 
		SelectedInitParam().IsRenderTarget ? D3DUSAGE_RENDERTARGET : 0,
		D3DFMT_A8R8G8B8,
		SelectedInitParam().IsRenderTarget ? D3DPOOL_DEFAULT : D3DPOOL_MANAGED,
		&gTexture)) ) goto cleanup;

	ImageDesc.Format = D3DFMT_A8R8G8B8;
	ImageDesc.Depth  = bitdepth;
	ImageDesc.Width  = imgWidth;
	ImageDesc.Height = imgHeight;
	ImageDesc.ImageFileFormat = D3DXIFF_PNG;
	ImageDesc.MipLevels = 1;
	ImageDesc.ResourceType = D3DRTYPE_TEXTURE;

	D3DLOCKED_RECT locked;
	RECT rect;
	rect.left   = 0;
	rect.top    = 0;
	rect.right  = imgWidth;
	rect.bottom = imgHeight;
	gTexture->LockRect(0, &locked, &rect, 0);

	for ( DWORD i=0; i < imgHeight; i++ )
	{
		for ( DWORD p=0; p < imgWidth; p++ )
		{
			if ( channels == 4 )
			{
				if ( !*(data + i*stride + p*unit+3) )
				{
					*(DWORD*)(data + i*stride + p*unit) = 0;
				}

				*(DWORD*)((char*)locked.pBits + i*locked.Pitch + p*4) = 
					(((DWORD)*(BYTE*)(data + i*stride + p*unit))   << 16) + 
					(((DWORD)*(BYTE*)(data + i*stride + p*unit+1)) << 8) +
					(((DWORD)*(BYTE*)(data + i*stride + p*unit+2)) << 0) +
					(((DWORD)*(BYTE*)(data + i*stride + p*unit+3)) << 24);
				//*(DWORD*)((char*)locked.pBits + i*locked.Pitch + p*4) = 
					//*(DWORD*)(data + i*stride + p*unit);

			} else if ( channels == 3 )
			{
				*(DWORD*)((char*)locked.pBits + i*locked.Pitch + p*4) = 
					(((DWORD)*(BYTE*)(data + i*stride + p*unit))   << 16) | 
					(((DWORD)*(BYTE*)(data + i*stride + p*unit+1)) << 8)  | 
					(((DWORD)*(BYTE*)(data + i*stride + p*unit+2)) << 0)  | 
					(0xff << 24);
				/**(WORD*)((char*)locked.pBits + i*locked.Pitch + p*4) = 
					*(WORD*)(data + i*stride + p*unit);
				*(WORD*)((char*)locked.pBits + i*locked.Pitch + p*4+2) = 
					*(BYTE*)(data + i*stride + p*unit+2) | 0xff00;
					*/
			}
		}
		//memcpy(
			//((BYTE*)locked.pBits + i*locked.Pitch),
			//data + i*imgWidth*bitdepth*channels/8,
			//imgWidth*bitdepth*channels/8);
	}

	gTexture->UnlockRect(0);

	
	png_destroy_read_struct(&ptrread, &ptrinfo, 0);
	if ( rowPtrs ) delete rowPtrs;
	if ( data    ) delete data;
	fclose(fp);
	return true;

cleanup:
	MLOG("Failed", );
	png_destroy_read_struct(&ptrread, &ptrinfo, 0);
	if ( rowPtrs ) delete rowPtrs;
	if ( data    ) delete data;
	if ( fp		 ) fclose(fp);
	return false;
}





/*
	
	class Progressor // I think it should be improved
	{
	private:
		bool IsMoveForced;
		int MoveObjective;

	public:
		Queue<DWORD> QueuePlan;
		double MaxProgress;
		double LevelProgress;

		double Progress;
		float DegreeProgress;

		bool IsStarted;

		Progressor()
		{
			LevelProgress = MaxProgress = Progress = 0;
			DegreeProgress = 0.f;
			IsStarted = false;
			IsMoveForced = false;
			MoveObjective = 0;
		}
		virtual ~Progressor();
		virtual void AddProgress(int MaxProgress);
		virtual void Next();
		virtual void Previous();
		virtual void Move(int Objective);
		virtual void MoveRelative(int Objective);
		virtual void Clear();
	
		virtual void Update(double ElapsedTime);
	};
Progressor::~Progressor()
{
}

void Progressor::AddProgress(int MaxProgress)
{
	if ( MaxProgress == 0 ) return ;
	if ( !IsStarted )
	{
		this->MaxProgress = MaxProgress;
		this->Progress = 0;
		this->IsStarted = true;
		this->MoveObjective = 0;
	}
	QueuePlan.Add(MaxProgress);

}

void Progressor::Clear()
{
	IsStarted = false;
	QueuePlan.Clear();
	Progress = 0;
	DegreeProgress = 0;
	MoveObjective = 0;
}

void Progressor::Next()
{
	Move(MoveObjective+1);
}

void Progressor::Previous()
{
	Move(MoveObjective-1);
}

void Progressor::Move(int Objective)
{
	if ( !IsStarted ) return;
	IsMoveForced = true;
	MoveObjective = Objective;
	if ( MoveObjective > QueuePlan.Count ) MoveObjective = QueuePlan.Count;
}

void Progressor::MoveRelative(int Objective)
{
	Move(MoveObjective+Objective);
}

void Progressor::Update(double ElapsedTime)
{
	if ( IsStarted ){
		Progress += Stacked;
		if ( Progress >= MaxProgress || IsMoveForced ){
			if ( IsMoveForced )
			{
				if ( MoveObjective > QueuePlan.Count ) MoveObjective = QueuePlan.Count;
				LevelProgress = MoveObjective;
				IsMoveForced = false;
			} else {
				LevelProgress ++;
			}
			MoveObjective = LevelProgress;
			if ( LevelProgress >= QueuePlan.Count )
			{
				Clear();
				return;
			}
			if ( LevelProgress >= QueuePlan.Count ) LevelProgress = QueuePlan.Count;
			MaxProgress = QueuePlan[LevelProgress];
			Progress = 0;
			DegreeProgress = 0;
		}
	}

	DegreeProgress = MaxProgress ? Progress*1. / MaxProgress : 0;

	return ;
}

*/






	class clsBitStreamReader
	{
	public:
		int		BitIndex;
		int		ByteIndex;
		BYTE	*Buffer;
		int		MaxIndex;
		bool	IsBufferManaged;
	
		clsBitStreamReader()
		{
			BitIndex = ByteIndex = MaxIndex = 0;
			Buffer = 0;
			IsBufferManaged = false;
		}
		clsBitStreamReader(int Size)
		{
			BitIndex = ByteIndex = 0;
			Buffer = new BYTE[Size];
			MaxIndex = Size;
			IsBufferManaged = true;
		}
		virtual ~clsBitStreamReader()
		{
			if ( IsBufferManaged )
			{
				if ( Buffer ) delete Buffer;
				Buffer = 0;
			}
		}

		void	SetBuffer(BYTE *Buffer, int Size);
		DWORD	GetBits(int Bits);
		BYTE	GetByteWithoutBits();
	};

	class clsByteStreamReader
	{
	public:
		int		ByteIndex;
		BYTE	*Buffer;
		int		MaxIndex;
		bool	IsBufferManaged;

		clsByteStreamReader()
		{
			ByteIndex = MaxIndex = 0;
			Buffer = 0;
			IsBufferManaged = false;
		}
		clsByteStreamReader(int Size)
		{
			ByteIndex = 0;
			Buffer = new BYTE[Size];
			MaxIndex = Size;
			IsBufferManaged = true;
		}
		virtual ~clsByteStreamReader()
		{
			if ( IsBufferManaged )
			{
				if ( Buffer ) delete Buffer;
				Buffer = 0;
			}
		}

		void	SetBuffer(BYTE *Buffer, int Size);
		BYTE	GetByte();
		WORD	GetWord();
		DWORD	GetDword();
		bool	GetArr(BYTE *Buf, int ByteToRead);
	};


void clsBitStreamReader::SetBuffer(BYTE *Buffer, int Size)
{
	if ( IsBufferManaged )
	{
		if ( this->Buffer ) delete this->Buffer;
		this->Buffer = 0;
	}
	if ( !Buffer )
	{
		this->Buffer = new BYTE[Size];
		this->MaxIndex = Size;
		this->IsBufferManaged = true;
	} else {
		this->Buffer = Buffer;
		this->MaxIndex = Size;
		this->IsBufferManaged = false;
	}
}

DWORD clsBitStreamReader::GetBits(int Bits)
{
	DWORD ret = 0;
	int bitsbytes = ((int)(Bits+BitIndex+7))/8;
	
	for ( int i=0; 
		i < MaxIndex-ByteIndex && i < bitsbytes && i < 4;
		i++ )
	{
		if ( 0 <= 3 - i && 3 - i < 4 )
			((BYTE*)(&ret))[3-i] = *(Buffer + ByteIndex + i);
	}
	ret <<= BitIndex;
	ret >>= (32-Bits);
	if ( (32-Bits) >= 32 ) ret = 0;

	BitIndex += Bits;
	if ( BitIndex < 0 )
		ByteIndex += ((BitIndex-7)/8);
	else ByteIndex += (BitIndex/8);
	BitIndex  &= 7;

	return ret;
}

BYTE clsBitStreamReader::GetByteWithoutBits()
{
	BYTE ret = 0;
	if ( MaxIndex <= ByteIndex+1 ) return 0;
	ret = Buffer[ByteIndex];
	ByteIndex++;
	return ret;
}


BYTE clsByteStreamReader::GetByte()
{
	BYTE ret = 0;
	if ( MaxIndex <= ByteIndex+1 ) return 0;
	ret = Buffer[ByteIndex];
	ByteIndex++;
	return ret;
}

WORD clsByteStreamReader::GetWord()
{
	WORD ret = 0;
	if ( MaxIndex <= ByteIndex+2 ) return 0;
	ret = *(WORD*)(Buffer+ByteIndex);
	ByteIndex += 2;
	return ret;
}

DWORD clsByteStreamReader::GetDword()
{
	DWORD ret = 0;
	if ( MaxIndex <= ByteIndex+4 ) return 0;
	ret = *(DWORD*)(Buffer+ByteIndex);
	ByteIndex += 4;
	return ret;
}

bool clsByteStreamReader::GetArr(BYTE *Buf, int ByteToRead)
{
	if ( MaxIndex <= ByteIndex+ByteToRead ) return false;
	memcpy(Buf, Buffer+ByteIndex, ByteToRead);
	ByteIndex += ByteToRead;
	return true;
}


void clsByteStreamReader::SetBuffer(BYTE *Buffer, int Size)
{
	if ( IsBufferManaged )
	{
		if ( this->Buffer ) delete this->Buffer;
		this->Buffer = 0;
	}
	if ( !Buffer )
	{
		this->Buffer = new BYTE[Size];
		this->MaxIndex = Size;
		this->IsBufferManaged = true;
	} else {
		this->Buffer = Buffer;
		this->MaxIndex = Size;
		this->IsBufferManaged = false;
	}
}












