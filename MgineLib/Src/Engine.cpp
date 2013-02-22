#pragma once
#include "StdAfx.hpp"
#include "Engine.h"
#include "Screen.h"
#include "Input.h"
#include "Function.h"
#include "Renderer.h"
#include "LocalSetting.h"
#include "Sound.h"
#include "Music.h"
#include "GameObject.h"
#include <dwmapi.h>
#include <ShellAPI.h>
#include <CommDlg.h>

DETECT_MEMORY_LEAK(-1);

using namespace Mgine;
using namespace Util;

typedef	EngineManager::SettingIntegration IgSetting;


class MonitorApplication
{
public:
	MonitorApplication()
	{ EngineManager::DoAppBegin(); }
	
	~MonitorApplication()
	{ EngineManager::DoAppFinish(); }
} MonitorApplication;

IgSetting			*EngineManager::Setting;


RECT				EngineManager::WindowIntegration::RectWindow;
POINT				EngineManager::WindowIntegration::PosWindow;
decltype(EngineManager::WindowIntegration::WindowHitTest) EngineManager::WindowIntegration::WindowHitTest;
decltype(EngineManager::WindowIntegration::WindowHitTestObjective) EngineManager::WindowIntegration::WindowHitTestObjective;
CriticalSection		EngineManager::WindowIntegration::CsWindow;
CriticalSection		EngineManager::WindowIntegration::CsHitTest;
DWORD				EngineManager::WindowIntegration::TidWnd;
HWND				EngineManager::WindowIntegration::hWnd;

EngineManager::WindowIntegration::STATE_INTERNAL	EngineManager::WindowIntegration::Internal;



CriticalSection		EngineManager::EngineIntegration::CsEngine;
Thread				EngineManager::EngineIntegration::EngineThread;
Thread				EngineManager::EngineIntegration::ThreadCheckSpeedHack;
HANDLE				EngineManager::EngineIntegration::hTimerSpeedHack;
LPDIRECT3D9			EngineManager::EngineIntegration::gD3D;
LPDIRECT3DDEVICE9	EngineManager::EngineIntegration::gDevice;

D3DPRESENT_PARAMETERS	EngineManager::EngineIntegration::D3dpp;

EngineManager::EngineIntegration::LOOP_STATE	EngineManager::EngineIntegration::LoopState;



bool				EngineManager::CoreIntegration::IsTerminating;
bool				EngineManager::CoreIntegration::IsReloading;

bool				EngineManager::CoreIntegration::IsAppFinished;
bool				EngineManager::CoreIntegration::IsVistaDwmFeatureEnabled;
HMODULE				EngineManager::CoreIntegration::hModuleDwm;


DECLDEF(EngineManager::CoreIntegration::Status);



void EngineManager::WindowIntegration::SetWindow()
{
	int width = 0, height = 0;
	{
		LockBlock Lock(CsWindow);

		RectWindow.left = 0;
		RectWindow.top = 0;
		RectWindow.right = Setting->GetSetting().ResolutionWidth;
		RectWindow.bottom = Setting->GetSetting().ResolutionHeight;
		AdjustWindowRectEx(&RectWindow, GetWindowLong(hWnd, GWL_STYLE), false, GetWindowLong(hWnd, GWL_EXSTYLE));

		width = RectWindow.right - RectWindow.left;
		height = RectWindow.bottom - RectWindow.top;
	}

	if ( 
		SetWindowPos(hWnd, 
		NULL,
		0, 0,
		width, height,
		SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOZORDER)
		== 0 
		)
	{
		MTHROW(Win32Error, "SetWindowPos");
	}
}

LRESULT CALLBACK EngineManager::WindowIntegration::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CREATE:
		break;

	case WM_DESTROY:
	case WM_QUIT:
		EngineManager::Core->IsTerminating = true;
		return 0;
		break;

	case WM_PAINT:
	case WM_ERASEBKGND:
		break;

	case WM_CHAR:
		ScreenManager::RaiseComponentCharEventInvoke((CHARCODE)wParam);
		break;

	case WM_SETCURSOR:
		if ( EngineManager::Setting->GetSetting().IsCursorVisible == false )
		{
			if ( !EngineManager::Engine->D3dpp.Windowed )
			{
				SetCursor(NULL);
				return 1;
			}
			if ( Input::GetIsCursorInClient() )
			{
				SetCursor(NULL);
				return 1;
			}
		}
		break;

	case WM_LBUTTONDOWN:
		if ( !Input::GetIsCursorInClient() )
			return SendMessage(hWnd, WM_NCLBUTTONDOWN, HTCAPTION, NULL);
		break;

	case WM_NCHITTEST:
		{
			DWORD ret = DefWindowProc(hWnd, message, wParam, lParam);
			switch ( GetMouseHitTest() )
			{
				case HITTEST_CONTENT:
					return HTCLIENT;
				case HITTEST_MOVE:
					return HTCAPTION;
				case HITTEST_RESIZE_LEFT:
					return HTLEFT;
				case HITTEST_RESIZE_RIGHT:
					return HTRIGHT;
				case HITTEST_RESIZE_TOPLEFT:
					return HTTOPLEFT;
				case HITTEST_RESIZE_TOP:
					return HTTOP;
				case HITTEST_RESIZE_TOPRIGHT:
					return HTTOPRIGHT;
				case HITTEST_RESIZE_BOTTOMLEFT:
					return HTBOTTOMLEFT;
				case HITTEST_RESIZE_BOTTOM:
					return HTBOTTOM;
				case HITTEST_RESIZE_BOTTOMRIGHT:
					return HTBOTTOMRIGHT;
				default:
					break;
			}
			return ret;
		}
		break;

	case WM_GETMINMAXINFO:
		{
			MINMAXINFO *mmi = (MINMAXINFO*)lParam;

			DWORD widthsame = 0, heightsame = 0;
			widthsame = GetRectWindow().right - GetRectWindow().left;
			heightsame = GetRectWindow().bottom - GetRectWindow().top;

			MGINE_SETTING setting = Setting->GetSetting();

			if ( setting.ResolutionMaxWidth == -1 ) 
				mmi->ptMaxTrackSize.x = widthsame;
			else if ( setting.ResolutionMaxWidth != 0 ) mmi->ptMaxTrackSize.x = setting.ResolutionMaxWidth;
			if ( setting.ResolutionMaxHeight == -1 ) 
				mmi->ptMaxTrackSize.y = heightsame;
			else if ( setting.ResolutionMaxHeight != 0 ) mmi->ptMaxTrackSize.y = setting.ResolutionMaxHeight;

			if ( setting.ResolutionMinWidth == -1 ) 
				mmi->ptMinTrackSize.x = widthsame;
			else mmi->ptMinTrackSize.x = setting.ResolutionMinWidth;
			if ( setting.ResolutionMinHeight == -1 ) 
				mmi->ptMinTrackSize.y = heightsame;
			else mmi->ptMinTrackSize.y = setting.ResolutionMinHeight;

			return 0;
		}
		break;
	case WM_SIZE:
		if ( Core->Status < Starting ) break;
		if ( lParam )
		{
			ScreenManager::Invoke(
				[lParam]()
			{
				Setting->SetResolution((int)LOWORD(lParam), (int)HIWORD(lParam));
			});

			//ScreenManager::IncSuppressDrawCount();
		}
		break;
	case WM_DROPFILES:
		{
			if ( !Setting->GetSetting().IsDragDropEnabled ) break;
			int i;
			HDROP hdrop = (HDROP)wParam;
			DROPFILE_INFO dropinfo = {0};

			dropinfo.FileCount = DragQueryFile(hdrop, 0xffffffff, NULL, 0);
			if ( dropinfo.FileCount )
			{
				dropinfo.FilePath = new wchar_t[dropinfo.FileCount][260];
				for ( i=0; i < dropinfo.FileCount; i++ )
				{
					DragQueryFile(hdrop, i, dropinfo.FilePath[i], MAX_PATH);
				}
				if ( Setting->GetSetting().OnDropFileAsync ) Setting->GetSetting().OnDropFileAsync(&dropinfo);

				delete[] dropinfo.FilePath;
			}
			return 1;
		}
		break;

	case WM_SYSCOMMAND:
		{
			switch ( Setting->GetSetting().WindowZOrderRule )
			{
			case BottomMost:
				if ( ((int)wParam & 0xfffffff0) == SC_MINIMIZE )
				{
					Window->Internal.IsIgnoreZOrder = false;
					SetWindowPos(Window->hWnd, HWND_TOPMOST, 0, 0, 0, Window->RectWindow.bottom-Window->RectWindow.top, SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE);
					Window->Internal.IsIgnoreZOrder = true;

					Internal.IsStateRestoreMinimize = true;
					return 0;
				}
			}
		}
		break;
	case WM_MOVING:
		{
			LockBlock Lock(CsWindow);
			RECT *lprct = (RECT*)lParam;
			PosWindow.x = lprct->left;
			PosWindow.y = lprct->top;
		}
		break;
	case WM_MOVE:
		{
			LockBlock Lock(CsWindow);
			RECT *lprct = (RECT*)lParam;
			if ( (short)LOWORD(lParam) == -32000 && (short)HIWORD(lParam) == -32000)
			{
			} else 
			{
				PosWindow.x = (short)LOWORD(lParam) + RectWindow.left;
				PosWindow.y = (short)HIWORD(lParam) + RectWindow.top;
			}
		}
		break;

	case WM_WINDOWPOSCHANGING:
		{
			WINDOWPOS *pos = (WINDOWPOS*)lParam;
			
			if ( Setting->GetSetting().WindowZOrderRule == BottomMost )
			{
				if ( Window->Internal.IsIgnoreZOrder )
				{
					pos->flags |= SWP_NOZORDER;
				}
			}
		}
		break;
	case WM_WINDOWPOSCHANGED:
		break;

	default:
		break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

void EngineManager::WindowIntegration::Init(MGINE_SETTING *Setting)
{
	if ( !Setting )
		MTHROW(InvalidParameter, "Setting is null");

	HMODULE hmodule = Setting->hModule;
	assert(hmodule);

	try
	{ 
		TidWnd = GetCurrentThreadId();
		WNDCLASS wc = { };
		wc.style			= CS_DBLCLKS | CS_SAVEBITS;
		wc.lpfnWndProc		= WndProc;
		wc.cbClsExtra		= 0;
		wc.cbWndExtra		= 0;
		wc.hInstance		= hmodule;
		wc.hIcon			= 0;
		wc.hCursor			= LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground	= (HBRUSH)0;//(COLOR_WINDOW+1);
		//wc.lpszMenuName	= MAKEINTRESOURCE(IDC_CMGINE);
		wc.lpszMenuName		= NULL;
		wc.lpszClassName	= Setting->AppName; // could be failed when gamename is empty? //GameName;
		//wcex.hIconSm		= hIcon;//LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

		if ( RegisterClass(&wc) ){ }
		else if ( GetLastError() == ERROR_CLASS_ALREADY_EXISTS )
		{
			if ( GetWindowLong(hWnd, GWL_WNDPROC) != (LONG)WndProc )
			{
				MTHROW(Win32Error, "Already registered by other in the same module. Unable to continue.");
			} else 
			{
				MLOG(
					"RC: Class already exists and looks like made by Mgine.\n" \
					L"This may can be ignored.");
			}
		} else MTHROW(Win32Error, "Failed RC. GLE: %.08x", GetLastError());

		{
			// for uac this should enable dragdrop
			// filter register with ChangeWindowMessageFilter
			typedef BOOL (WINAPI *CHANGEWINDOWMESSAGEFILTER)( UINT message, DWORD dwFlag ); 
			HMODULE hdll = GetModuleHandle(L"user32.dll");
			CHANGEWINDOWMESSAGEFILTER ChangeWindowMessageFilter;
			ChangeWindowMessageFilter = (CHANGEWINDOWMESSAGEFILTER)GetProcAddress(hdll, "ChangeWindowMessageFilter");
			if ( ChangeWindowMessageFilter )
			{
				ChangeWindowMessageFilter(WM_DROPFILES, MSGFLT_ADD);
				ChangeWindowMessageFilter(WM_COPYDATA, MSGFLT_ADD);
				ChangeWindowMessageFilter(0x0049, MSGFLT_ADD);
			}
		}

		SetRect( &RectWindow, 0, 0, Setting->ResolutionWidth, Setting->ResolutionHeight );
		if ( !EngineManager::Core->IsVistaDwmFeatureEnabled )
		{
			AdjustWindowRectEx(&RectWindow, 
				WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX, 
				false,
				WS_EX_DLGMODALFRAME | WS_EX_WINDOWEDGE | WS_EX_CLIENTEDGE);

			hWnd = CreateWindowEx(
				WS_EX_DLGMODALFRAME | WS_EX_WINDOWEDGE | WS_EX_CLIENTEDGE,
				Setting->AppName, Setting->AppName, 
				WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,
				CW_USEDEFAULT, CW_USEDEFAULT,
				RectWindow.right - RectWindow.left,
				RectWindow.bottom - RectWindow.top,
				0, 0,
				hmodule,
				0);
		}
		else
		{
			DWORD screenx = GetSystemMetrics(SM_CXSCREEN),
				screeny = GetSystemMetrics(SM_CYSCREEN);

			AdjustWindowRectEx(&RectWindow, 
				WS_POPUP | WS_THICKFRAME | WS_MINIMIZEBOX, 
				false, 
				0);

			hWnd = CreateWindowEx(
				0,
				Setting->AppName, Setting->AppName,
				WS_POPUP | WS_THICKFRAME | WS_MINIMIZEBOX,
				screenx/2 - (RectWindow.right - RectWindow.left)/2,
				screeny/2 - (RectWindow.bottom - RectWindow.top)/2,
				RectWindow.right - RectWindow.left,
				RectWindow.bottom - RectWindow.top,
				0, 0,
				hmodule,
				0);
		}

		if ( !hWnd )
			MTHROW(Win32Error, "Failed to create window. GLE: %.08x", GetLastError());
	}
	catch ( BaseException & )
	{
		MLOG("Failed");
		throw;
	}
}

void EngineManager::WindowIntegration::Uninit()
{
	if ( hWnd )
	{
		if ( DestroyWindow(hWnd) == FALSE )
		{
			PostMessage(hWnd, WM_DESTROY, 0, 0);
			PostMessage(hWnd, WM_QUIT, 0, 0);
		} else hWnd = 0;
	}
	hWnd = 0;	
}

bool EngineManager::Init(MGINE_SETTING *Setting)
{
	if ( Core->Status != NotLoaded ) return false;
	if ( Core->IsTerminating ) return false;
	if ( !Setting ) return false;

	MLOG("Begin");

	try
	{ 
		assert(!Engine->EngineThread.IsAlive());
		if ( Engine->EngineThread.IsAlive() )
		{
			MLOG("Invalid thread status. Unexpected", );
			return false; // what is this!?
		}
		Core->IsReloading = false;
		Core->Status = Loading;

		SettingIntegration::Init();
		EngineManager::Setting->SetInitialSetting(Setting);

		Core->Init();
		Window->Init(&EngineManager::Setting->GetSetting());
		Engine->Init();

		MLOG("Succeeded");
		return true;
	}
	catch ( BaseException & )
	{
		MLOG("Failed");
		EngineManager::Uninit();
		return false;
	}
}

void EngineManager::Uninit()
{
	if ( Core->Status == NotLoaded ) return;

	Core->IsTerminating = true;

	DWORD currenttid = GetCurrentThreadId();
	if ( currenttid == Window->TidWnd )
	{
		// in start thread(window thread), don't call this except finishing of Init() and Start()

		EngineManager::Core->Status = Finishing;

		EngineManager::Engine->EngineThread.CloseWait(); // this thread will uninit its own contents(EngineIntegration) itself

		try { Engine->Uninit(); }	catch ( BaseException & ) { }
		try { Window->Uninit(); }	catch ( BaseException & ) { }
		try { Core->Uninit(); }		catch ( BaseException & ) { }
		try { SettingIntegration::Uninit(); }	catch ( BaseException & ) { }
		
		Core->Status = NotLoaded;
		return;
	} else if ( currenttid == Engine->EngineThread.Tid() )
	{
		if ( EngineManager::Core->Status != Finishing ) return;
		else 
		{
			try { DoUninit(); }	catch ( BaseException & ) { }
			EngineManager::Core->Status = Finished;
		}
	} else 
	{
		MLOG("Unknown thread %d called this. Not expected", currenttid);
		return ; // failed but the thread would not use Mgine Exception handling. so just return not throw
	}
	
	return;
}

bool EngineManager::Start()
{
	if ( EngineManager::Core->Status != Loading && EngineManager::Core->Status != Loaded ) return false;
	
	try { Window->Start(); } catch ( BaseException & ) {  }
	EngineManager::Uninit();
	return true;
}

D3DMULTISAMPLE_TYPE EngineManager::EngineIntegration::FindFitMultiSampleType()
{
	int i;
	D3DMULTISAMPLE_TYPE Arr[] = 
	{
		D3DMULTISAMPLE_NONE,
		D3DMULTISAMPLE_2_SAMPLES,
		D3DMULTISAMPLE_3_SAMPLES,
		D3DMULTISAMPLE_4_SAMPLES,
		D3DMULTISAMPLE_5_SAMPLES,
		D3DMULTISAMPLE_6_SAMPLES,
		D3DMULTISAMPLE_7_SAMPLES,
		D3DMULTISAMPLE_8_SAMPLES,
		//D3DMULTISAMPLE_9_SAMPLES,
		//D3DMULTISAMPLE_10_SAMPLES,
		//D3DMULTISAMPLE_11_SAMPLES,
		//D3DMULTISAMPLE_12_SAMPLES,
		//D3DMULTISAMPLE_13_SAMPLES,
		//D3DMULTISAMPLE_14_SAMPLES,
		//D3DMULTISAMPLE_15_SAMPLES,
		//D3DMULTISAMPLE_16_SAMPLES
	};

	for ( i = COUNTOF(Arr) - 1; i >= 0; i-- ){
		if ( SUCCEEDED(gD3D->CheckDeviceMultiSampleType(
			0, D3DDEVTYPE_HAL, D3DFMT_A8R8G8B8,
			true,
			Arr[i],
			0)) )
				break;
	}
	if ( i == -1 ) return D3DMULTISAMPLE_NONE;
	else return Arr[i];
}

void EngineManager::EngineIntegration::Init()
{
	EngineThread.Init([](boost::any&){ ProcThreadEngine(); });
	EngineThread.Prepare();
	EngineThread.SetPriority(THREAD_PRIORITY_TIME_CRITICAL);
	EngineThread.Resume();
}

void EngineManager::EngineIntegration::Uninit()
{
	
}

void EngineManager::EngineIntegration::ProcThreadCheckSpeedHack()
{
	double	  tickStartDouble = MF::GetDoubleTick();
	ULONGLONG tickStartLong   = MF::GetLongTick();
	double	  tickCumDouble	 = 0;
	ULONGLONG tickCumLong	 = 0;


	for(;;)
	{
		if ( Core->IsTerminating ) break;
		if ( WaitForSingleObject(hTimerSpeedHack, 0) != WAIT_TIMEOUT )
		{
			tickCumDouble += 200;
			tickCumLong   += 200;
		}
		double calcdouble  = tickStartDouble + tickCumDouble - MF::GetDoubleTick();
		LONGLONG  calclong = tickStartLong + tickCumLong - MF::GetLongTick();
		calcdouble = MF::Abs(calcdouble);
		calclong   = MF::Abs(calclong);
#ifndef _DEBUG
		if ( calcdouble > 5000 )
			MLOG("CL");	// speedhack or critical lag detected
		if ( calclong > 5000 )
			MLOG("CL");	// speedhack or critical lag detected
#endif
		MF::DelayExecution(100.f);
	}

}

void EngineManager::EngineIntegration::ProcThreadEngine()
{
	while ( !Core->IsTerminating ) 
	{
		switch ( EngineManager::Core->Status )
		{
		case Loading:
			try { DoInit(); }
			catch ( BaseException & ) 
			{ 
				MLOG("DoInit Failed", ); 
				goto cleanup;
			}
			EngineManager::Core->Status = Loaded;
			break;

		case Starting:
			try
			{ DoStart(); }
			catch ( BaseException & ) 
			{ 
				MLOG("DoStart Failed", ); 
				goto cleanup;
			}
			EngineManager::Core->Status = Started;
			break;

		case Started:
			try
			{ DoLoop(); }
			catch ( BaseException & )
			{ 
				MLOG("DoLoop Failed", ); 
				goto cleanup;
			}
			break;
		}
	}

cleanup:
	Core->Status = Finishing;
	EngineManager::Uninit();

}
void EngineManager::EngineIntegration::InitDevice(IDirect3DDevice9 *NewDevice)
{
	MLOG("Begin");
	HRESULT hr = 0;
	D3DXMATRIX mattmp;
	IDirect3DDevice9 *oldDevice = 0;

	try
	{
		DoDeviceCreatePrepare();
		MLOG("Creating dx device ..");
		if ( !NewDevice )
		{
			if ( FAILED( hr = gD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, Window->hWnd, D3DCREATE_MIXED_VERTEXPROCESSING | D3DCREATE_MULTITHREADED | D3DCREATE_FPU_PRESERVE, &D3dpp, &gDevice )) )
				MTHROW(DirectXError, "Failed to create dx device. hr:%.08x", hr);
		} else {
			oldDevice = gDevice;
			gDevice = NewDevice;
		}

		DoDeviceCreate();
		DoDeviceStart();

		int remainedobject = 0;
		if ( oldDevice && (remainedobject = oldDevice->Release()) != 0 )
		{
			wchar_t substr[1024];
			swprintf_s(substr, L"Memory leak detected while recreating device: %d objects are remained. But continue on.", remainedobject);
			MLOG("%s", substr);
#ifdef _DEBUG
			MessageBox(0, substr, L"Forgive me", 0);
			MF::BreakExecution();
			//goto cleanup;
#endif
			while ( oldDevice->Release() );
		}
		oldDevice = NULL;
	}
	catch ( BaseException & )
	{
		MLOG("Failed");
		EngineManager::Uninit();
		//DXTRACE_ERR_MSGBOX(procstr, hr);
		throw;
	}
}

void EngineManager::WindowIntegration::MouseHitTestUpdate()
{ SetMouseHitTest(WindowHitTestObjective); }

void EngineManager::WindowIntegration::FlushMessage()
{
	MSG msg;
	while ( PeekMessage(&msg, hWnd, 0, 0, PM_REMOVE) )
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}


void EngineManager::WindowIntegration::Start()
{
	if ( Window->TidWnd != GetCurrentThreadId() )
		MTHROW(InvalidStatus, "Thread is different with when Init()");

	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);

	MSG msg; memset(&msg, 0, sizeof(MSG));
	
	try
	{ 
		for (;;)
		{
			SetLastError(0);
			if ( MsgWaitForMultipleObjectsEx(0, 0, 50, QS_ALLINPUT, 0) == WAIT_TIMEOUT )
			{
				switch ( Core->Status )
				{
				case Loaded:
					{
						Window->SetWindow();

						{
							LockBlock Lock(Window->CsWindow);

							RECT rct = { };
							GetWindowRect(Window->hWnd, &rct);
							Window->PosWindow.x = rct.left;
							Window->PosWindow.y = rct.top;
						}

						if ( Setting->GetSetting().WindowZOrderRule == BottomMost )
						{
							Window->Internal.IsIgnoreZOrder = false;
							SetWindowPos(Window->hWnd, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE); 
							Window->Internal.IsIgnoreZOrder = true;
						}

						while ( !ShowWindow(Window->hWnd, SW_SHOW) );
						UpdateWindow(Window->hWnd);

						SetForegroundWindow(Window->hWnd);
						SetFocus(Window->hWnd);

						Core->Status = Starting;
					}
					break;

				case Started:
					{
						if ( Window->Internal.IsStateRestoreMinimize )
						{
							Window->Internal.IsStateRestoreMinimize = false;
							SendMessage(HWND_BROADCAST, WM_NCHITTEST, 0, 0); // a great way to resolve it! my god i ate almost 5 days with this!

							Window->Internal.IsIgnoreZOrder = false;
							SetWindowPos(Window->hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE);
							Window->Internal.IsIgnoreZOrder = true;
						}

					}

					break;
				}

				//MF::DelayExecution(10.f); // 1 ms, shorter enough?
				//SwitchToThread(); // if there are same or higher priority of thread, yield the execution
			} else 
			{
				SetLastError(0);
				while ( PeekMessage(&msg, Window->hWnd, 0, 0, PM_REMOVE) )
				{
					TranslateMessage(&msg);
					DispatchMessage(&msg);
					SetLastError(0);
				}
				int error = GetLastError();
				if ( error == ERROR_INVALID_WINDOW_HANDLE ) 
					break; // windows destroyed
			}
			//if ( msg.message == WM_QUIT )
				//break;
			if ( Core->IsTerminating ) break;
		}
	
		/*while ( GetMessage(&msg, hWnd, 0, 0) != -1 )
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			if ( IsTerminating ) break;
		}*/
	}
	catch ( BaseException & )
	{
		MLOG("Failed");
		throw;
	}
}

std::wstring EngineManager::WindowIntegration::OpenFileDialog(LPCWSTR Filter)
{
	wchar_t strfile[16384] = L"";
	OPENFILENAME ofn = { };
	ofn.lStructSize		= sizeof(ofn);
	ofn.hwndOwner		= hWnd;
	ofn.lpstrFilter		= Filter;
	ofn.lpstrFile		= strfile;
	ofn.nMaxFile		= COUNTOF(strfile);
	ofn.Flags			|= OFN_EXPLORER;
	GetOpenFileName(&ofn);
	//OFN.lpstrInitialDir	= "c:\\";
	return std::wstring(strfile);
}

bool EngineManager::WindowIntegration::GetIsCursorInClientRect()
{
	if ( EngineManager::Core->Status < Started ) return false;
	POINT point; GetCursorPos(&point);
	RECT rc, rcclient;

	GetClientRect(EngineManager::Window->hWnd, &rcclient);
	AdjustWindowRectEx(&rcclient, GetWindowLong(EngineManager::Window->hWnd, GWL_STYLE), false, GetWindowLong(EngineManager::Window->hWnd, GWL_EXSTYLE));
	GetClientRect(EngineManager::Window->hWnd, &rc);
	rcclient.right  -= rc.right;
	rcclient.bottom -= rc.bottom;

	GetWindowRect(EngineManager::Window->hWnd, &rc);
	rc.left		-= rcclient.left;
	rc.right	-= rcclient.right;
	rc.top		-= rcclient.top;
	rc.bottom	-= rcclient.bottom;

	if ( PtInRect(&rc, point) ) return true;
	else return false;
}


void EngineManager::EngineIntegration::SubReloadDevice()
{
	if ( Core->Status < Started ) return ;
	if ( Setting->GetSetting().ResolutionWidth != Renderer::GetResolutionWidth() ||
		Setting->GetSetting().ResolutionHeight != Renderer::GetResolutionHeight() )
	{
		try
		{ ReloadDevice(); }
		catch ( BaseException & )
		{ 
			MLOG("ReloadDevice Failed");
			throw;
		}
	}
	if ( !Core->IsReloading ) return ;

	IDirect3DDevice9 *newDevice = 0;

	HRESULT devicestate = gDevice->TestCooperativeLevel();
	HRESULT hr = 0;

	try
	{ 
		if ( devicestate != D3DERR_DEVICELOST ){
			if ( devicestate == D3DERR_DEVICENOTRESET || SUCCEEDED(devicestate) )
			{
				DoDeviceCreatePrepare();
				hr = gDevice->Reset(&D3dpp);
				if ( hr == D3DERR_DEVICELOST )
				{
					// device is lost again while reloading.. try more loop
					return;
				} else if ( hr == D3DERR_DEVICEREMOVED )
				{
					// device is completely removed. need to create all the resources and device
					hr = gD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, Window->hWnd, D3DCREATE_MIXED_VERTEXPROCESSING | D3DCREATE_MULTITHREADED | D3DCREATE_FPU_PRESERVE, &D3dpp, &newDevice);
					if ( hr == D3DERR_DEVICELOST )
					{
						// try again later
						RELEASE_OBJECT(newDevice);
						return;
					} else if ( FAILED(hr) )
					{
						RELEASE_OBJECT(newDevice);
						MTHROW(DirectXError, "Device was lost, removed and is unresolvable hr:%.08x", hr);
					}
					InitDevice(newDevice);
					Core->IsReloading = false;
					return ;
				}
				else if ( FAILED(hr) ) //if ( devicestate == D3DERR_DEVICEREMOVED || other cases )
				{
					MTHROW(DirectXError, "Device needed to be reloaded, but unavailable. hr: %.08x", hr);
				} else 
				{
					DoDeviceReset();
					DoDeviceStart();

					Core->IsReloading = false;
					return;
				}

			}
		}
	}
	catch ( BaseException & )
	{
		MLOG("Failed"); 
		EngineManager::Uninit(); // Unload cuz it is critical
		Core->IsReloading = false;
		throw;
	}
	
	return ;
}
void EngineManager::EngineIntegration::ReloadDevice()
{
	LockBlock lock(CsEngine);
	
	if ( Core->Status < Started )
		MTHROW(InvalidStatus, "Status is less than Started");
	if ( Core->IsReloading ) return;

	Core->IsReloading = true;

	/*Renderer::Begin();
	Renderer::Clear(0x00000000);
	Renderer::ClearZStencil();
	MgineFont::DefaultFont.Draw(L":: Loading ::", -1,
		Renderer::GetResolutionWidth()/2, Renderer::GetResolutionHeight()/2,
		0, 0, 0xffffffff, DT_NOCLIP | DT_VCENTER | DT_CENTER, false);
	Renderer::End();
	Renderer::Present();*/
	//Window->SetWindow();

	try
	{ DoDeviceLost(); }
	catch ( BaseException & )
	{ 
		MLOG("Failed OnDeviceLost()", );
		EngineManager::Uninit();
		throw;
	}
}

void EngineManager::DoInit()
{
	MLOG("Begin");

	try
	{ 
		Engine->gD3D = Direct3DCreate9(D3D9b_SDK_VERSION);
		if ( !Engine->gD3D )
			MTHROW(DirectXError, "Failed to create d3d");


// 		if ( FAILED(Engine->gD3D->GetAdapterDisplayMode(0, &Engine->DisplayMode)) )
// 			throw Exception<DirectXError>(L"%s: Failed to get adapter display mode", __FUNCTIONW__);


		MLOG("Loading Modules..", );

		LocalSettingManager::Init(Setting->GetSetting().AppName);
		Input::Init(Window->hWnd);
		Sound::Init(Window->hWnd);
		MusicManager::Init();
		//MgineEffect::Init();
		Engine->InitDevice(NULL);
		DeviceResourceManager::Init();
		GameObjectManager::Init();
		Renderer::Init();
		ScreenManager::Init();
		ComponentManager::Init();

		MLOG("Loading defvs..", );
		EngineManager::Setting->ApplySettingInitial();

		//AdjustProjection();
		OnInit();
	}
	catch ( BaseException & )
	{
		MLOG("Failed");
		throw;
	}


}

void EngineManager::DoStart()
{
	if ( EngineManager::Core->Status != Starting )
		MTHROW(InvalidStatus, "Not Starting");

	try
	{
		Engine->LoopState.OnStart();

		OnStart();

		LARGE_INTEGER li;
		li.QuadPart = -20000*0;
		EngineManager::Engine->hTimerSpeedHack = CreateWaitableTimer(NULL, false, NULL);
		if ( !SetWaitableTimer(EngineManager::Engine->hTimerSpeedHack, &li, 200, 0, 0, false) )
			MTHROW(Win32Error, "SWT");

		Engine->ThreadCheckSpeedHack.Init([](boost::any&){ EngineManager::EngineIntegration::ProcThreadCheckSpeedHack(); });
		Engine->ThreadCheckSpeedHack.Prepare();
		Engine->ThreadCheckSpeedHack.SetPriority(THREAD_PRIORITY_LOWEST);
		Engine->ThreadCheckSpeedHack.Resume();
		//	MTHROW(InvalidOperation, "Failed to start tcsh");
	}
	catch ( BaseException & )
	{
		MLOG("Failed");
		EngineManager::Uninit();
		throw;
	}




}

void EngineManager::DoUninit()
{
	OnUninit();

	Engine->ThreadCheckSpeedHack.CloseWait();
	/*while( WaitForSingleObject(Engine->EngineThread.Handle(), 1) == WAIT_TIMEOUT )
	{
		Window->FlushMessage();
	}*/ // don't do this cuz it is waiting for itself to close

	if ( Engine->hTimerSpeedHack == INVALID_HANDLE_VALUE ) Engine->hTimerSpeedHack = 0;
	if ( Engine->hTimerSpeedHack ) CloseHandle(Engine->hTimerSpeedHack);
	Engine->hTimerSpeedHack = NULL;


	ComponentManager::Uninit();
	ScreenManager::Uninit();
	DeviceResourceManager::Uninit();
	Renderer::Uninit();
	GameObjectManager::Uninit();
	RELEASE_OBJECT(Engine->gDevice);
	//MgineEffect::Uninit();
	MusicManager::Uninit();
	Sound::Uninit();
	Input::Uninit();
	LocalSettingManager::Uninit();

	RELEASE_OBJECT(Engine->gD3D);
}

void EngineManager::DoLoop()
{
	ATHROW(Core->Status == Started);
	if ( Engine->EngineThread.Tid() != GetCurrentThreadId() )
		MTHROW(InvalidOperation, "Tid should be equal to engine"); // it is necessary cuz of QueryPerformanceCounter's strange behavior

	double tick = MF::GetDoubleTick(Engine->LoopState.QFFrequency),
		elapsed = tick-Engine->LoopState.UpdateProceededTick;
	Engine->LoopState.UpdateProceededTick = tick;

	if ( elapsed > 0 )
	{
		{
			LockBlock LockBlock(Engine->CsEngine);

			try
			{
				MGINE_SETTING setting = EngineManager::Setting->GetSetting();
				Sound::Update(elapsed);

				Input::CheckAcquire();
				Input::Update(elapsed);

				if ( setting.OnUpdate ) setting.OnUpdate(elapsed);
				ScreenManager::Update(elapsed);
				EngineManager::Window->MouseHitTestUpdate();

				if ( setting.OnUpdateAfter ) setting.OnUpdateAfter(elapsed);
			}
			catch ( BaseException & )
			{
				MLOG("Failed Update", );
				throw;
			}
		}

		{
			LockBlock LockBlock(Engine->CsEngine);
			try
			{ 
				Engine->SubReloadDevice();
				if ( Core->IsTerminating ) return;
				ScreenManager::Draw();
			}
			catch ( BaseException & )
			{
				MLOG("Failed Draw", );
				throw;
			}
		}
	}


	SwitchToThread();

	return;
}

void EngineManager::DoAppBegin()
{
	OnAppBegin();
}

void EngineManager::DoAppFinish()
{
	OnAppFinish();
	Core->IsAppFinished = true;
}

void EngineManager::DoDeviceLost()
{
	OnDeviceLost();
}

void EngineManager::DoDeviceReset()
{
	OnDeviceReset();
}

void EngineManager::DoDeviceCreatePrepare()
{
	memset(&Engine->D3dpp, 0, sizeof(Engine->D3dpp));
	Engine->D3dpp.Windowed = TRUE;
	Engine->D3dpp.BackBufferWidth = Setting->GetSetting().ResolutionWidth;
	Engine->D3dpp.BackBufferHeight = Setting->GetSetting().ResolutionHeight;
	Engine->D3dpp.BackBufferFormat = D3DFMT_A8R8G8B8; //DisplayMode.Format;

	Engine->D3dpp.BackBufferCount = 1;
	Engine->D3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;

	//Engine->D3dpp.MultiSampleQuality = 0;
	Engine->D3dpp.MultiSampleType = EngineIntegration::FindFitMultiSampleType();
	//Engine->D3dpp.MultiSampleType = D3DMULTISAMPLE_NONE;

	Engine->D3dpp.hDeviceWindow = Window->hWnd;

	// stencil buffer changed into manual
	Engine->D3dpp.EnableAutoDepthStencil = true;
	Engine->D3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;
	Engine->D3dpp.Flags |= D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL;

	//D3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE; //D3DPRESENT_INTERVAL_IMMEDIATE
	Engine->D3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE; //Engine->D3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
}

void EngineManager::DoDeviceCreate()
{
	try
	{ OnDeviceCreate(); }
	catch ( BaseException & )
	{
		MLOG("Failed");
		throw;
	}
}

void EngineManager::DoDeviceStart()
{
	HRESULT hr = 0;

	try
	{
		OnDeviceStart();
	}
	catch ( BaseException & )
	{
		MLOG("Failed");
		throw;
	}
}

void EngineManager::OnInit()
{
	if ( !EngineManager::Setting->GetSetting().OnInit )
		MTHROW(InvalidOperation, "OnInit callback of setting should be set. Check your main procedure.");
	try
	{ 
		EngineManager::Setting->GetSetting().OnInit();
	}
	catch ( BaseException & )
	{
		MLOG("Failed");
		throw;
	}
}

void EngineManager::OnStart()
{
	if ( !EngineManager::Setting->GetSetting().OnStart ) return;

	try
	{ 
		EngineManager::Setting->GetSetting().OnStart();
	}
	catch ( BaseException & )
	{
		MLOG("Failed");
		throw;
	}
}

void EngineManager::OnUninit()
{
	if ( !Setting->GetSetting().OnUninit ) return;
	try
	{ EngineManager::Setting->GetSetting().OnUninit(); }
	catch ( BaseException & )
	{
		MLOG("Failed");
		throw;
	}
}

void EngineManager::OnAppBegin()
{
}

void EngineManager::OnAppFinish()
{
	try
	{ 
		Sound::OnAppFinish();
		DeviceResourceManager::OnAppFinish();
		//MgineEffect::OnAppFinish();
		ComponentManager::OnAppFinish();
		GameObjectManager::OnAppFinish();
	}
	catch ( BaseException & )
	{
		MLOG("Failed");
		throw;
	}
}

void EngineManager::OnDeviceLost()
{
	try
	{
		DeviceResourceManager::OnDeviceLost();
		//MgineEffect::OnDeviceLost();
		Renderer::OnDeviceLost();
		ScreenManager::OnDeviceLost();
	} catch ( BaseException & )
	{
		MLOG("Failed");
		throw;
	}
}

void EngineManager::OnDeviceReset()
{
	try
	{
		DeviceResourceManager::OnDeviceReset();
		//MgineEffect::OnDeviceReset();
		Renderer::OnDeviceReset();
		ScreenManager::OnDeviceReset();
		ScreenManager::IncSuppressDrawCount();
	}
	catch ( BaseException & )
	{
		MLOG("Failed");
		throw;
	}

}

void EngineManager::OnDeviceCreate()
{
	try
	{ 
		//MgineEffect::OnDeviceCreate();
		Renderer::OnDeviceCreate();
		ScreenManager::OnDeviceCreate();

		ScreenManager::IncSuppressDrawCount();
	}
	catch ( BaseException & )
	{
		MLOG("Failed");
		throw;
	}
}

void EngineManager::OnDeviceStart()
{
	try
	{ 
		Renderer::OnDeviceStart();
		ScreenManager::IncSuppressDrawCount();
	}
	catch ( BaseException & )
	{
		MLOG("Failed");
		throw;
	}
}










EngineManager::MGINE_SETTING IgSetting::iSetting;
EngineManager::MGINE_SETTING IgSetting::iSettingSync;
CriticalSection IgSetting::CsSetting;
CriticalSection IgSetting::CsSettingGet;

void IgSetting::Init()
{
	MLOG("Begin");
	//EngineManager::Setting = &Instance;

	EngineManager::Core->hModuleDwm = LoadLibrary(L"dwmapi.dll");
	if ( Core->hModuleDwm ) Core->IsVistaDwmFeatureEnabled = true;
	MLOG("ModDwm: %.08x", EngineManager::Core->hModuleDwm);
}

void IgSetting::Uninit()
{
}

void IgSetting::SetInitialSetting(EngineManager::MGINE_SETTING *Setting)
{
	if ( !Setting ) MTHROW(InvalidParameter, "Setting is null");
	if ( Setting->hModule == NULL ) Setting->hModule = GetModuleHandle(NULL);
	if ( !Setting->hModule ) MTHROW(Win32Error, "Failed to GetMH");
	iSetting = *Setting;
	SetSettingSync();
}

void IgSetting::ApplySettingInitial()
{
	LockBlock lock(CsSetting);

	try
	{ 
		SetAppName(iSetting.AppName);
		SetShaderFilePath(iSetting.ShaderFilePath);
		//SetTargetElapseTime(iSetting.TargetElapseTime);
		SetResolution(iSetting.ResolutionWidth, iSetting.ResolutionHeight);
		SetResolutionMinMax(iSetting.ResolutionMinWidth, iSetting.ResolutionMinHeight, iSetting.ResolutionMaxWidth, iSetting.ResolutionMaxHeight);
		SetIsCursorVisible(iSetting.IsCursorVisible);
		SetIsZEnableDefaultly(iSetting.IsZEnableDefaultly);
		SetIsDwmWindow(iSetting.IsDwmWindow);
		SetIsDwmWindowFrameless(iSetting.IsDwmWindowFrameless);
		SetIsDragDropEnabled(iSetting.IsDragDropEnabled);
		SetZOrderRule(iSetting.WindowZOrderRule);
		SetBlurFactor(iSetting.BlurFactor);
		SetOnInit(iSetting.OnInit);
		SetOnStart(iSetting.OnStart);
		SetOnUninit(iSetting.OnUninit);
		SetOnUpdate(iSetting.OnUpdate);
		SetOnUpdateAfter(iSetting.OnUpdateAfter);
		SetOnDraw(iSetting.OnDraw);
		SetOnDrawAfter(iSetting.OnDrawAfter);
		SetOnStreamUpdateAsync(iSetting.OnStreamUpdateAsync);
		SetOnDropFileAsync(iSetting.OnDropFileAsync);
	}
	catch ( BaseException & )
	{
		MLOG("Failed");
		throw;
	}
}


void IgSetting::SetAppName(wchar_t *GameName)
{
	LockBlock lock(CsSetting);
	if ( !GameName ) MTHROW(InvalidParameter, "GameName is null");
	wcsncpy_s(iSetting.AppName, GameName, COUNTOF(iSetting.AppName));

	SetSettingSync();
}

void IgSetting::SetShaderFilePath(wchar_t *ShaderFilePath)
{
	LockBlock lock(CsSetting);
	if ( !ShaderFilePath ) MTHROW(InvalidParameter, "FilePath is null");
	wcsncpy_s(iSetting.ShaderFilePath, ShaderFilePath, COUNTOF(iSetting.ShaderFilePath));

	SetSettingSync();
}

void IgSetting::SetResolution(DWORD Width, DWORD Height)
{
	LockBlock lock(CsSetting);
	if ( Width <= 0 || Height <= 0 )
		MTHROW(InvalidParameter, "Minus");

	if ( iSetting.ResolutionWidth == Width &&
		iSetting.ResolutionHeight == Height ) return;
	//if ( Core->IsReloading ) return false;

	iSetting.ResolutionWidth = Width;
	iSetting.ResolutionHeight = Height;

	SetSettingSync();
}

void IgSetting::SetResolutionMinMax(DWORD MinWidth, DWORD MinHeight, DWORD MaxWidth, DWORD MaxHeight)
{
	LockBlock lock(CsSetting);
	if ( MinWidth < 0 || MinHeight < 0 || MaxWidth < 0 || MaxHeight < 0 )
		MTHROW(InvalidParameter, "Minus");

	if ( iSetting.ResolutionMinWidth == MinWidth &&
		iSetting.ResolutionMinHeight == MinHeight &&
		iSetting.ResolutionMaxWidth == MaxWidth && 
		iSetting.ResolutionMaxHeight == MaxHeight ) return;
	//if ( Core->IsReloading ) return false;

	iSetting.ResolutionMinWidth = MinWidth;
	iSetting.ResolutionMinHeight = MinHeight;
	iSetting.ResolutionMaxWidth = MaxWidth;
	iSetting.ResolutionMaxHeight = MaxHeight;

	SetSettingSync();
}

void IgSetting::SetIsCursorVisible(bool IsCursorVisible)
{
	LockBlock lock(CsSetting);

	if ( IsCursorVisible )
	{
		if ( !SetCursor(LoadCursor(NULL, IDC_WAIT)) )
			MTHROW(Win32Error, "Enabling SetCursor");
	}
	else 
	{
		if ( GetForegroundWindow() == Window->hWnd )
			MTHROW(Win32Error, "Disabling SetCursor");
	}
	iSetting.IsCursorVisible = IsCursorVisible;

	SetSettingSync();
}

void IgSetting::SetIsZEnableDefaultly(bool IsZEnableDefaultly)
{
	LockBlock lock(CsSetting);

	try
	{ 
		if ( IsZEnableDefaultly )
		{
			Renderer::SetRawRenderState(D3DRS_ZENABLE, D3DZB_TRUE);
			Renderer::SetRawRenderState(D3DRS_ZWRITEENABLE, true);
		}
		else
		{
			Renderer::SetRawRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
			Renderer::SetRawRenderState(D3DRS_ZWRITEENABLE, false);
		}

		iSetting.IsZEnableDefaultly = IsZEnableDefaultly;

		SetSettingSync();
	}
	catch ( BaseException & )
	{
		MLOG("Failed");
		throw;
	}
}

void IgSetting::SetIsDwmWindow(bool IsDwmWindow)
{
	LockBlock lock(CsSetting);
	
	try
	{ 
		if ( EngineManager::Core->IsVistaDwmFeatureEnabled )
		{
			typedef enum _DWMWINDOWATTRIBUTE {
				DWMWA_NCRENDERING_ENABLED           = 1,
				DWMWA_NCRENDERING_POLICY,
				DWMWA_TRANSITIONS_FORCEDISABLED,
				DWMWA_ALLOW_NCPAINT,
				DWMWA_CAPTION_BUTTON_BOUNDS,
				DWMWA_NONCLIENT_RTL_LAYOUT,
				DWMWA_FORCE_ICONIC_REPRESENTATION,
				DWMWA_FLIP3D_POLICY,
				DWMWA_EXTENDED_FRAME_BOUNDS,
				DWMWA_HAS_ICONIC_BITMAP,
				DWMWA_DISALLOW_PEEK,
				DWMWA_EXCLUDED_FROM_PEEK,
				DWMWA_CLOAK,
				DWMWA_CLOAKED,
				DWMWA_FREEZE_REPRESENTATION,
				DWMWA_LAST 
			} DWMWINDOWATTRIBUTE;

			typedef enum _DWMNCRENDERINGPOLICY {
				DWMNCRP_USEWINDOWSTYLE,
				DWMNCRP_DISABLED,
				DWMNCRP_ENABLED,
				DWMNCRP_LAST 
			} DWMNCRENDERINGPOLICY;

			typedef HRESULT (__stdcall *FDwmSetWindowAttribute)(HWND, _DWMWINDOWATTRIBUTE, void *, DWORD);

			static FDwmSetWindowAttribute pdwmsetwindowattribute = (FDwmSetWindowAttribute)GetProcAddress(Core->hModuleDwm, "DwmSetWindowAttribute");

			static HRESULT (__stdcall *dwmextendframeintoclientarea)(HWND, MARGINS*);
			if ( EngineManager::Core->hModuleDwm && !dwmextendframeintoclientarea ) 
				dwmextendframeintoclientarea =
				(HRESULT (__stdcall*)(HWND, MARGINS*))
				GetProcAddress(Core->hModuleDwm, "DwmExtendFrameIntoClientArea");
			if ( IsDwmWindow )
			{
				MARGINS marg = {-1, -1, -1, -1};
				DWMNCRENDERINGPOLICY var = DWMNCRP_ENABLED;

				if ( //FAILED(pdwmsetwindowattribute(Window->hWnd, DWMWA_NCRENDERING_POLICY, &var, sizeof(var)))
					/*||*/ FAILED(dwmextendframeintoclientarea(EngineManager::Window->hWnd, &marg))
					) 
				{
					EngineManager::Core->IsVistaDwmFeatureEnabled = false;
					return;
				} else {
					SetWindowLong(EngineManager::Window->hWnd, GWL_STYLE, GetWindowLong(EngineManager::Window->hWnd, GWL_STYLE) & ~WS_SYSMENU & ~WS_CAPTION);
					SetWindowLong(EngineManager::Window->hWnd, GWL_EXSTYLE, GetWindowLong(EngineManager::Window->hWnd, GWL_EXSTYLE) | WS_EX_COMPOSITED);
				}
			} else {
				MARGINS marg = {0, 0, 0, 0};

				if ( FAILED(dwmextendframeintoclientarea(Window->hWnd, &marg)) )
					MTHROW(Win32Error, "ExtendFrameIntoClientArea");
				SetWindowLong(EngineManager::Window->hWnd, GWL_STYLE, GetWindowLong(EngineManager::Window->hWnd, GWL_STYLE) | WS_SYSMENU | WS_CAPTION);
				SetWindowLong(EngineManager::Window->hWnd, GWL_EXSTYLE, GetWindowLong(EngineManager::Window->hWnd, GWL_EXSTYLE) & ~WS_EX_COMPOSITED);
			}
			EngineManager::Window->SetWindow();

			iSetting.IsDwmWindow = IsDwmWindow;
		} else iSetting.IsDwmWindow = false;

		SetSettingSync();
	}
	catch ( BaseException & )
	{
		MLOG("Failed");
		throw;
	}
}

void IgSetting::SetIsDwmWindowFrameless(bool IsDwmWindowFrameless)
{
	LockBlock lock(CsSetting);

	try
	{ 
		if ( EngineManager::Core->IsVistaDwmFeatureEnabled && iSetting.IsDwmWindow )
		{
			if ( IsDwmWindowFrameless )
			{
				SetWindowLong(EngineManager::Window->hWnd, GWL_STYLE, GetWindowLong(EngineManager::Window->hWnd, GWL_STYLE) &~ WS_THICKFRAME);
			} else {
				SetWindowLong(EngineManager::Window->hWnd, GWL_STYLE, GetWindowLong(EngineManager::Window->hWnd, GWL_STYLE) | WS_THICKFRAME);
			}
			EngineManager::Window->SetWindow();

			iSetting.IsDwmWindowFrameless = IsDwmWindowFrameless;
		} else iSetting.IsDwmWindowFrameless = false;

		SetSettingSync();
	}
	catch ( BaseException & )
	{
		MLOG("Failed");
		throw;
	}
}

void IgSetting::SetIsDragDropEnabled(bool IsDragDropEnabled)
{
	LockBlock lock(CsSetting);

	DragAcceptFiles(EngineManager::Window->hWnd, IsDragDropEnabled);
	iSetting.IsDragDropEnabled = IsDragDropEnabled;
	SetSettingSync();
}

void IgSetting::SetZOrderRule(EngineManager::WINDOW_ZORDER_RULE Rule)
{
	LockBlock lock(CsSetting);
	iSetting.WindowZOrderRule = Rule;
	switch ( Rule )
	{
	case BottomMost:
		if ( EngineManager::Core->Status >= Started )
		{
			Window->Internal.IsIgnoreZOrder = false;
			SetWindowPos(Window->hWnd, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE); 
			Window->Internal.IsIgnoreZOrder = true;
		}
	}
	SetSettingSync();
}

void IgSetting::SetBlurFactor(float BlurFactor)
{
	LockBlock lock(CsSetting);
	try
	{ 
		if ( BlurFactor > 0 ){
			Renderer::SetBlur(true);
		} else {
			Renderer::SetBlur(false);
		}
	}
	catch ( BaseException & )
	{
		MLOG("Failed");
		throw;
	}
	
	iSetting.BlurFactor = BlurFactor;

	SetSettingSync();
}

void IgSetting::SetOnInit(CallbackInit ProcInit)
{
	LockBlock lock(CsSetting);
	iSetting.OnInit = ProcInit;
	SetSettingSync();
}

void IgSetting::SetOnStart(CallbackStart ProcStart)
{
	LockBlock lock(CsSetting);
	iSetting.OnStart = ProcStart;
	SetSettingSync();
}

void IgSetting::SetOnUninit(CallbackUninit ProcUninit)
{ 
	LockBlock lock(CsSetting);
	iSetting.OnUninit = ProcUninit;
	SetSettingSync();
}

void IgSetting::SetOnUpdate(CallbackUpdate ProcUpdate)
{
	LockBlock lock(CsSetting);
	iSetting.OnUpdate = ProcUpdate;
	SetSettingSync();
}

void IgSetting::SetOnUpdateAfter(CallbackUpdate ProcUpdateAfter)
{
	LockBlock lock(CsSetting);
	iSetting.OnUpdateAfter = ProcUpdateAfter;
	SetSettingSync();
}

void IgSetting::SetOnDraw(CallbackDraw ProcDraw)
{
	LockBlock lock(CsSetting);
	iSetting.OnDraw = ProcDraw;
	SetSettingSync();
}

void IgSetting::SetOnDrawAfter(CallbackDraw ProcDrawAfter)
{
	LockBlock lock(CsSetting);
	iSetting.OnDrawAfter = ProcDrawAfter;
	SetSettingSync();
}

void IgSetting::SetOnStreamUpdateAsync(CallbackStreamUpdate ProcStreamUpdateAsync)
{
	LockBlock lock(CsSetting);
	iSetting.OnStreamUpdateAsync = ProcStreamUpdateAsync;
	SetSettingSync();
}

void IgSetting::SetOnDropFileAsync(CallbackDropFile ProcDropFile)
{
	LockBlock lock(CsSetting);
	iSetting.OnDropFileAsync = ProcDropFile;
	SetSettingSync();
}


void EngineManager::EngineIntegration::LOOP_STATE::OnStart()
{
	if ( !QueryPerformanceFrequency((LARGE_INTEGER*)&LoopState.QFFrequency) || LoopState.QFFrequency == 0 )
	{
		MLOG("QPF failed on #2. Check your system supports high resolution timers.");
		MTHROW(Win32Error, "QPF Failed. Check your system supports high resolution timers.");
	}
	LoopState.UpdateProceededTick = MF::GetDoubleTick(LoopState.QFFrequency);
}

void EngineManager::CoreIntegration::Init()
{ timeBeginPeriod(1); }

void EngineManager::CoreIntegration::Uninit()
{ timeEndPeriod(1); }






void EngineManager::MGINE_SETTING::Default()
{
	static wchar_t *pathShader = L"Shader\\Shader.sdr";
	static wchar_t *gamenameDefault = L"Mgine Default App Name";

	wcscpy_s(ShaderFilePath, MAX_PATH, pathShader);
	wcscpy_s(AppName, 1024, gamenameDefault);

	hModule = NULL;

	ResolutionWidth  = 1024;
	ResolutionHeight = 768;
	ResolutionMaxHeight = ResolutionMaxWidth = ResolutionMinHeight = ResolutionMinWidth = -1;
	//SetCursor(LoadCursor(NULL, IDC_WAIT));

	IsCursorVisible		= true;
	IsZEnableDefaultly	= false;
	IsDwmWindow			= false;
	IsDwmWindowFrameless = false;
	IsDragDropEnabled	= false;
	WindowZOrderRule = Normal;

	BlurFactor = 0;

	OnInit			= 0;
	OnStart			= 0;
	OnUninit		= 0;
	OnUpdate		= 0;
	OnUpdateAfter	= 0;
	OnDraw			= 0;
	OnDrawAfter		= 0;
	OnStreamUpdateAsync	= 0;
	OnDropFileAsync		= 0;
}

