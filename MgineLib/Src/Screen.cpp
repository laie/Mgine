#include "StdAfx.hpp"
#include "Screen.h"
#include "Input.h"
#include "Renderer.h"
#if _DEBUG
#include "Function.h"
#endif
using namespace Mgine;
using namespace Util;

bool ScreenManager::IsInited;

LONGLONG ScreenManager::SuppressDrawCount;
bool ScreenManager::IsFpsShowing;

Screen* ScreenManager::CurrentScreen;

decltype(ScreenManager::SignalInvokeProcedure) ScreenManager::SignalInvokeProcedure;
decltype(ScreenManager::CsInvoke) ScreenManager::CsInvoke;
decltype(ScreenManager::CsSuppressDrawCount) ScreenManager::CsSuppressDrawCount;

decltype(ScreenManager::EventDeviceLost)	ScreenManager::EventDeviceLost;
decltype(ScreenManager::EventDeviceReset)	ScreenManager::EventDeviceReset;
decltype(ScreenManager::EventDeviceCreate)	ScreenManager::EventDeviceCreate;

double ScreenManager::MOVING_STATE::MovingFrameCount;
double ScreenManager::MOVING_STATE::MovingFrameCountMax;
Effect *ScreenManager::MOVING_STATE::MovingEffect;
Screen *ScreenManager::MOVING_STATE::MovingScreen;

void ScreenManager::Init()
{
	MLOG("Begin");
	//InitializeCriticalSection(&CsInvoke);
	IsFpsShowing = true;
	IsInited = true;
}

void ScreenManager::Uninit()
{
	//DeleteCriticalSection(&CsInvoke);
	if ( CurrentScreen ) CurrentScreen->RaiseEnd();
	IsInited = false;
}

void ScreenManager::MOVING_STATE::MoveScene(Screen *MovingScene)
{
	if ( MovingScene == NULL ) MTHROW(InvalidStatus, "Unknown state");

	bool issuc = true;

	ScreenManager::MOVING_STATE::MovingEffect = 0;
	ScreenManager::MOVING_STATE::MovingFrameCount = 0;
	ScreenManager::MOVING_STATE::MovingFrameCountMax = 0;

	try { if ( CurrentScreen && CurrentScreen->Status != Screen::ENDED ) CurrentScreen->RaiseEnd(); } catch ( BaseException & ) { issuc = false; }
	try { if ( MovingScene->Status == Screen::ENDED ) MovingScene->RaiseStart(); } catch ( BaseException & ){ issuc = false; }
	//IsSuppressDrawOnce = true;
	CurrentScreen = MovingScene;

	ScreenManager::MOVING_STATE::MovingScreen = NULL;

	if ( !issuc ) MTHROW(InvalidOperation, "Failed");
}

void ScreenManager::MOVING_STATE::MoveScene(Screen *MovingScene, int FrameCount, Effect *Effect)
{
	if ( ScreenManager::MOVING_STATE::MovingScreen != NULL ) MTHROW(InvalidStatus, "Already Moving");
	if ( CurrentScreen == MovingScene ) MTHROW(InvalidOperation, "Screen moving to same screen");

	bool issuc = true;

	MovingFrameCount = 0;
	MovingFrameCountMax = FrameCount;
	ScreenManager::MOVING_STATE::MovingScreen = MovingScene;
	MovingEffect = Effect;
	if ( Effect ) try { Effect->Start(FrameCount, 1, 1); } catch ( BaseException & ) { issuc = false; }
	try { if ( MovingScene->Status == Screen::ENDED ) MovingScene->RaiseStart(); } catch ( BaseException & ) { issuc = false; }
	//IsSuppressDrawOnce = true;

	if ( !issuc ) MTHROW(InvalidOperation, "Failed");
}

void ScreenManager::RaiseComponentCharEventInvoke( CHARCODE Char )
{
	Invoke(
		[Char]()
		{
			try
			{
				if ( !CurrentScreen ) return;

				CharEventArg arg(Char);
				CurrentScreen->RootFrame.RaiseCharEvent(&arg);
			}
			catch ( BaseException & )
			{
				MLOG("Failed");
				throw;
			}
		}
	);
}

void ScreenManager::MOVING_STATE::Update(double ElapsedTime)
{
	// Screen fading process
	if ( MovingScreen != NULL )
	{
		MovingFrameCount += ElapsedTime;
		if ( MovingFrameCount > MovingFrameCountMax ) MoveScene(MovingScreen);
		else
		{
			if ( MovingScreen ) MovingScreen->RaiseUpdate(ElapsedTime);
			if ( MovingEffect ) MovingEffect->Update(ElapsedTime);
		}
	}
}

void ScreenManager::Update(double ElapsedTime)
{
	if ( !ScreenManager::IsInited ) MTHROW(InvalidStatus, "Not inited");

	try
	{
		LockBlock lockblock(CsInvoke);
		SignalInvokeProcedure();
		SignalInvokeProcedure.disconnect_all_slots();
	}
	catch ( ... )
	{
		MLOG("Invoke");
		throw;
	}

	try
	{
		if ( CurrentScreen ) CurrentScreen->RaiseUpdate(ElapsedTime);
		StateMoving.Update(ElapsedTime);

		// Debugging control
		if ( Input::Key[KeyF10].Hit() )
			IsFpsShowing = !IsFpsShowing;

		{
			LockBlock lock(CsSuppressDrawCount);
			SuppressDrawCount--;
			if ( SuppressDrawCount < 0 ) SuppressDrawCount = 0;
		}
	}
	catch ( BaseException & )
	{
		MLOG("Failed");
		throw;
	}
	return;
}

void ScreenManager::MOVING_STATE::Draw()
{
	Renderer::SetRawRenderTarget(Renderer::TemporaryRenderTargetMovingScene[0].GetSurface());
	Renderer::Clear(D3DCOLOR_ARGB(0, 0, 0, 0), true, false, false);
	if ( CurrentScreen ) CurrentScreen->RaiseDraw();
	Renderer::FlushBatch();
	Renderer::FinishFrame();

	if ( MovingEffect )
	{
		Renderer::SetRawRenderTarget(Renderer::TemporaryRenderTargetMovingScene[1].GetSurface());
		Renderer::Clear(D3DCOLOR_ARGB(0, 0, 0, 0), true, false, false);
		if ( MovingScreen ) MovingScreen->RaiseDraw();
		Renderer::FlushBatch();
		Renderer::FinishFrame();

		MovingEffect->Apply();
		MovingEffect->SetParameter("SamplerB", Renderer::TemporaryRenderTargetMovingScene[1].GetTexture());
	}
	//Renderer::TemporaryRenderTargetMovingScene[0].RenderToRenderTarget(0, true, MovingEffect);
	Renderer::SetRawRenderTarget(Renderer::GetRawBackBuffer());
	Renderer::Clear(D3DCOLOR_ARGB(0, 0, 0, 0), true, false, false);

	QuadDrawer drawer;
	drawer.Init(&Renderer::TemporaryRenderTargetMovingScene[0]);
	drawer.Draw();
	//Renderer::TemporaryRenderTargetMovingScene[0].Draw();
}

void ScreenManager::Draw()
{
	if ( !ScreenManager::IsInited ) MTHROW(InvalidStatus, "Not inited");
	if ( SuppressDrawCount ) return;
	if ( EngineManager::Core->IsReloading ) return;

	/*{
		LOGBRUSH logbrush;
		logbrush.lbStyle = BS_SOLID;
		logbrush.lbColor = 0x00000000;
		logbrush.lbHatch = 0;
		static HBRUSH hbrushblack = CreateBrushIndirect(&logbrush);
		static HPEN   hpenblack   = CreatePen(PS_SOLID, 1, 0);
		HDC hdc = GetDC(EngineManager::hWnd);
		HGDIOBJ hbrushselected = SelectObject(hdc, hbrushblack);
		HGDIOBJ hpenselected = SelectObject(hdc, hpenblack);

		Rectangle(hdc, 0, 0, 1024, 768);

		SelectObject(hdc, hpenselected);
		SelectObject(hdc, hbrushselected);
		ReleaseDC(EngineManager::hWnd, hdc);
	}*/

	//EngineManager::Engine->CsEngine.Enter();


#ifdef _DEBUG
	class clsTimer
	{
	private:
	public:
		double LastTick;
		ULONGLONG ExpectedMaxElapseTick;

		inline clsTimer()
		{
			LastTick = MF::GetDoubleTick();
			ExpectedMaxElapseTick = -1;
		}

		inline clsTimer(ULONGLONG ExpectedElapseTick)
		{
			LastTick = MF::GetDoubleTick();
			this->ExpectedMaxElapseTick = ExpectedElapseTick;
		}

		inline void SetTick()
		{ LastTick = MF::GetDoubleTick(); }

		inline double GetElapsedTime()
		{
			double ret = MF::GetDoubleTick() - LastTick;
			#ifdef _DEBUG
			if ( ExpectedMaxElapseTick != -1 &&
				ret > ExpectedMaxElapseTick ) __asm int 3;
			#endif
			return ret;
		}

	};

	clsTimer timer;
#endif

	try
	{ 
		Renderer::Begin();
		Renderer::FinishFrame();
		ReverterRawBlendState RestoreBlendState;
		if ( StateMoving.GetIsMoving() )
			StateMoving.Draw();
		else
		{
			Renderer::Clear(D3DCOLOR_ARGB(0, 0, 0, 0), true, false, false);
			if ( CurrentScreen ) CurrentScreen->RaiseDraw();
		}
		Renderer::FlushBatch();
		Renderer::ApplyBlur();




#ifdef _DEBUG
		IDirect3DSurface9 *surface = NULL;
		IDirect3DSurface9 *rendertarget = NULL;	

		static double	BeforeElapsed = 0;
		static int		CountFrame = 0;
		static double	CumulativeElapsed = 0;
		static double	Fps = 0;
		static double	AverageElapsed = 0;
		static double	MaxElapsedToken = 0;
		static double	MaxElapsed = 0;
		static double	LastTick = MF::GetDoubleTick();
		static RECT		rt;
		static wchar_t	procstr[1024];
		double NowTick = MF::GetDoubleTick();

		rt.left		= 10;
		rt.top		= 10;
		rt.right	= Renderer::GetResolutionWidth();
		rt.bottom	= Renderer::GetResolutionHeight();
		CountFrame ++;

		if ( BeforeElapsed > MaxElapsedToken ) MaxElapsedToken = BeforeElapsed;
		CumulativeElapsed += BeforeElapsed;

		if ( LastTick + 500 < NowTick )
		{
			Fps = CountFrame / ((NowTick - LastTick)/1000);
			AverageElapsed = CumulativeElapsed/CountFrame;
			LastTick += 500*((NowTick - LastTick)/500);
			CountFrame = 0;
			CumulativeElapsed = 0;
			MaxElapsed = MaxElapsedToken;
			MaxElapsedToken = 0;
		}
	
		Renderer::SetBlendState(BLEND_STATE(BLEND_STATE::OVERLAY));
		swprintf_s(procstr, 1024,
			L"Fps: %.1f, ms: %05.02lf(max%05.02lf<%d)\n(%5d,%5d)", 
			Fps, BeforeElapsed, MaxElapsed,
			(int)(1000.f/Renderer::GetRawDisplayMode().RefreshRate),
			Input::Mouse.X(), Input::Mouse.Y()
			);
		// %llu to ULONGLONG
 
		double starttick = MF::GetDoubleTick();
		if ( IsFpsShowing ) DeviceResourceManager::DefaultFont.Draw(
			procstr,
			10, 10,
			Renderer::GetResolutionWidth() - 10, Renderer::GetResolutionHeight() - 10,
			D3DCOLOR_ARGB(180, 100, 255, 100),
			DT_LEFT, false);

		//float tempprogress = 0;
		//VERTEX_DOT vt;
		/*for ( tempprogress = 0; tempprogress < 1; tempprogress += 0.01f )
		{
			vt.Init();
			vt.ColorDiffuse = 0xffffffff;
			vt.X = 10+tempprogress*100;
			vt.Y = 230-MF::GetCurveCycleA(tempprogress, 3)*100;
			Renderer::DrawPoint(&vt, 1);
		}
	
		{
			VERTEX_LINE line;
			line.Init();
			line.V1.ColorDiffuse = 0xffffffff;
			line.V2.ColorDiffuse = 0xffffffff;
			line.V1.X = 10;
			line.V1.Y = 130;
			line.V2.X = 10;
			line.V2.Y = 230;
			Renderer::DrawLine(&line, 1);

			line.Init();
			line.V1.ColorDiffuse = 0xffffffff;
			line.V2.ColorDiffuse = 0xffffffff;
			line.V1.X = 10;
			line.V1.Y = 230;
			line.V2.X = 110;
			line.V2.Y = 230;
			Renderer::DrawLine(&line, 1);
		}*/
		//Renderer::FinishFrame();
#endif

		RestoreBlendState.Restore();
		Renderer::End();


		//EngineManager::Engine->CsEngine.Leave();
		if ( !SuppressDrawCount )
			Renderer::Present();

#ifdef _DEBUG
		BeforeElapsed = timer.GetElapsedTime();
#endif
	}
	catch ( BaseException & )
	{
		MLOG("Failed");
		throw;
	}
}

void Mgine::ScreenManager::MoveScreen(Screen *DestinationScreen)
{
	StateMoving.MoveScene(DestinationScreen);
}

void Mgine::ScreenManager::Invoke( const boost::function<void ()> & Lambda )
{
	if ( !IsInited ) return;
	{
		LockBlock lockblock(CsInvoke);
		SignalInvokeProcedure.connect(Lambda);
	}
}

void ScreenManager::OnDeviceLost()
{ EventDeviceLost()(); }
void ScreenManager::OnDeviceReset()
{ EventDeviceReset()(); }
void ScreenManager::OnDeviceCreate()
{ EventDeviceCreate()(); }






void Screen::ThreadProcContentLoadStarter()
{
	try
	{
		RaiseContentLoadAsync();
	}
	catch ( BaseException & )
	{
		MLOG("Failed to load screen %.08x", this);
		throw;
	}
}


Screen::Screen()
{
	ThreadContentLoad.Init([this](boost::any&){ ThreadProcContentLoadStarter(); });
	RootFrame.ForeColor = 0;

	EventBinder.Init(this);
	
	EventBinder.Bind(*ScreenManager::EventDeviceLost, &Screen::OnRawDeviceLost);
	EventBinder.Bind(*ScreenManager::EventDeviceReset, &Screen::OnRawDeviceReset);
	EventBinder.Bind(*ScreenManager::EventDeviceCreate, &Screen::OnRawDeviceCreate);
}
Screen::~Screen()
{
	//if ( ScreenManager::GetCurrentScreen() == this ) MTHROW(InvalidOperation, "Screen is being deleted while currently being used as Current Screen. Not allowed.",  );
	try
	{
		//WaitForContentLoadFinish();
		if ( StatusContent != UNLOADED ) RaiseContentUnload();
		if ( Status != ENDED ) RaiseEnd();
	}
	catch ( BaseException & )
	{
		MLOG("Failed tcl.cw", __LINE__);
	}
}

void Screen::RaiseContentLoadAsync()
{
	{
		LockBlock Lock(CsContentLoading);
		if ( StatusContent() != LOADING ) MTHROW(InvalidStatus, "Not loading");
		OnContentLoadAsync();

		StatusContent = LOADED_PREPARING;
	}
}

void Screen::RaiseContentUnload()
{
	if ( StatusContent() == UNLOADED ) MTHROW(InvalidStatus, "Already unloaded");
	WaitForContentLoadFinish();

	bool isunloaded = false;
	RootFrame.RaiseContentUnload();
	OnContentUnload(&isunloaded);
	if ( isunloaded )
		StatusContent = UNLOADED;
}

void Screen::RaiseStart()
{
	if ( Status() != ENDED ) MTHROW(InvalidStatus, "Already started");
	Status = STARTED_LOADING;

	ThreadContentLoad.CloseWait();

	OnLoadingStart();

	switch ( StatusContent )
	{
	case UNLOADED:
		StartContentLoad();
		break;

	//case LOADING: break;

	case LOADED_PREPARING:
		RaisePrepareContent(true);

	case LOADED_PREPARED:
		RaiseLoaded();
		break;
	default: MTHROW(InvalidStatus, "Invalid StatusContent on this point");
	}



	return ;
}
void Screen::RaiseEnd()
{
	if ( Status == ENDED ) MTHROW(InvalidStatus, "Already ended");
	WaitForContentLoadFinish();

	OnEnd();
	RootFrame.RaiseEnd();
	Status = ENDED ;

	RaiseContentUnload();
	return ;
}

void Screen::RaiseLoaded()
{
	Status = STARTED_LOADED;
	RootFrame.RaiseStart();
	OnStart();
}

void Screen::RaiseUpdate(double ElapsedTime)
{
	if ( Status == ENDED ) MTHROW(InvalidOperation, "Not started");
	
	try
	{
		switch ( StatusContent )
		{
		case UNLOADED:
			MTHROW(InvalidStatus, "Content load not initiated. At least should be loading");
			break;

		case LOADING:
			OnLoadingUpdate(ElapsedTime);
			break;

		case LOADED_PREPARING:
			RaisePrepareContent(true);

		case LOADED_PREPARED:
			if ( Status == STARTED_LOADING ) RaiseLoaded();
			if ( Status == STARTED_LOADED )
			{
				//RootFrame.RaiseUpdate(Stacked);
				OnUpdate(ElapsedTime);
				UpdateComponent(ElapsedTime);
			}
			break;
		default: MTHROW(InvalidStatus, "Invaild state to update");
		}
	}
	catch ( BaseException & )
	{
		MLOG("Failed", __LINE__);
		throw;
	}
}
void Screen::UpdateComponent(double ElapsedTime)
{
	RootFrame.RaiseUpdate(ElapsedTime);

	if ( Input::Mouse.IsChanged() )
	{
		MouseEventArg mousearg(Input::Mouse.GetMouseState());
		RootFrame.RaiseMouseEventPoint(&mousearg);	// Not handled events are also needed for releasing the mouse.

		HitTestObjective = mousearg.WindowHittestObjective;

		// TODO: when two screens are present, it should work improperly
		if ( HitTestObjective == HITTEST_UNSET ) HitTestObjective = HITTEST_CONTENT;
		if ( HitTestObjective != HITTEST_UNSET ) EngineManager::Window->SetMouseHitTestObjective(HitTestObjective);
	}
}
void Screen::RaiseDraw()
{
	try
	{
		switch ( Status )
		{
		case ENDED:
			MTHROW(InvalidStatus, "Not started");
			break;
		case STARTED_LOADING:
			OnLoadingDraw();
			break;
		case STARTED_LOADED:
			OnDraw();
			RootFrame.RaiseDraw();
			break;
		}
	}
	catch ( BaseException & )
	{
		MLOG("Failed", __LINE__);
		throw;
	}
}

void Screen::RaisePrepareContent(bool IsRestoreAll)
{
	if ( StatusContent == UNLOADED ) MTHROW(InvalidStatus, "Resource not loaded");
	WaitForContentLoadFinish();
	
	RECTANGLE pos = RECTANGLE(0, 0, (VEC)Renderer::GetResolutionWidth(), (VEC)Renderer::GetResolutionHeight());
	RootFrame.Position = pos;
	if ( RootFrame.StatusContent == UIComponent::UNLOADED ) RootFrame.RaiseContentLoad();
	if ( RootFrame.StatusContent >= UIComponent::LOADED_PREPARING ) RootFrame.RaiseContentPrepare(IsRestoreAll);
	OnPrepareContent(IsRestoreAll);
	StatusContent = LOADED_PREPARED;
}
void Screen::RaiseDeviceLost()
{
	WaitForContentLoadFinish();
	OnDeviceLost();
}
void Screen::RaiseDeviceReset()
{
	WaitForContentLoadFinish();
	OnDeviceReset();
	if ( StatusContent != UNLOADED ) ScreenManager::Invoke( [this](){ RaisePrepareContent(false); } );
}
void Screen::RaiseDeviceCreate()
{
	WaitForContentLoadFinish();
	OnDeviceCreate();
	if ( StatusContent != UNLOADED ) ScreenManager::Invoke( [this](){ RaisePrepareContent(true); } );
}

void Screen::StartContentLoad()
{
	if ( StatusContent != UNLOADED ) MTHROW(InvalidStatus, "Already loaded or loading");


	{
		LockBlock Lock(CsContentLoading);
		if ( StatusContent != UNLOADED ) MTHROW(InvalidStatus, "Already loaded or loading");
		StatusContent = LOADING;
	}

	ThreadContentLoad.Start();
}
void Screen::WaitForContentLoadFinish()
{
	if ( StatusContent == UNLOADED ) MTHROW(InvalidStatus, "Not loaded");

	ThreadContentLoad.CloseWait();
}

void Mgine::Screen::OnRawDeviceLost()
{
	if ( StatusContent == UNLOADED ) return;
	WaitForContentLoadFinish();
	RaiseDeviceLost();
}
void Mgine::Screen::OnRawDeviceReset()
{
	if ( StatusContent == UNLOADED ) return;
	WaitForContentLoadFinish();
	RaiseDeviceReset();
}
void Mgine::Screen::OnRawDeviceCreate()
{
	if ( StatusContent == UNLOADED ) return;
	WaitForContentLoadFinish();
	RaiseDeviceCreate();
}

void Screen::OnContentLoadAsync()
{ }
void Screen::OnContentUnload(bool *IsResourceUnloaded)
{ }
void Screen::OnStart()
{ }
void Screen::OnEnd()
{ }
void Screen::OnUpdate(double ElapsedTime)
{ }
void Screen::OnDraw()
{ }
void Screen::OnLoadingStart()
{ }
void Screen::OnLoadingUpdate(double ElapsedTime)
{ }
void Screen::OnLoadingDraw()
{ }
void Screen::OnPrepareContent(bool IsRestoreAll)
{ }
void Screen::OnDeviceLost()
{ }
void Screen::OnDeviceReset()
{ }
void Screen::OnDeviceCreate()
{ }



