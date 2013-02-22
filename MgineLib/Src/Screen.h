#pragma once

#include "DeviceResource.h"
#include "Component.h"

namespace Mgine
{
	class Screen;

	class ScreenManager
	{
	private:
		static bool IsInited;

		static LONGLONG SuppressDrawCount;
		static bool IsFpsShowing;

		static Screen *CurrentScreen;

		static Util::CriticalSection			CsInvoke;
		static Util::CriticalSection			CsSuppressDrawCount;
		static boost::signal<void ()>	SignalInvokeProcedure;

	public:
		static struct MOVING_STATE
		{
		private:
			static double MovingFrameCount;
			static double MovingFrameCountMax;
			static Effect *MovingEffect;
			static Screen *MovingScreen;

		public:
			static inline bool GetIsMoving()
			{ return MovingScreen != NULL; };

			static void Update(double ElapsedTime);
			static void Draw();
			static void MoveScene(Screen *MovingScene, int FrameCount, Effect *Effect);
			static void MoveScene(Screen *MovingScene);
		} StateMoving;

		static Util::OrderSafe<DeviceLostEvent>		EventDeviceLost;
		static Util::OrderSafe<DeviceResetEvent>		EventDeviceReset;
		static Util::OrderSafe<DeviceCreateEvent>		EventDeviceCreate;

		static inline void IncSuppressDrawCount()
		{ Util::LockBlock lock(CsSuppressDrawCount); SuppressDrawCount++; }
		static inline Screen *GetCurrentScreen()
		{ return CurrentScreen; }

		static void Init();
		static void Uninit();

		static void RaiseComponentCharEventInvoke( CHARCODE Char );

		static void Update(double ElapsedTime);
		static void Draw();

		static void MoveScreen(Screen *DestinationScreen);
		
		static void Invoke( const boost::function<void ()> & Lambda );

		static void OnDeviceLost();
		static void OnDeviceReset();
		static void OnDeviceCreate();
	};

	class Screen
	{
	public:
		enum STATUS : BYTE
		{
			ENDED = 0,
			STARTED_LOADING,
			STARTED_LOADED,
		};
		enum STATUS_CONTENT : BYTE
		{
			UNLOADED = 0,
			LOADING,
			LOADED_PREPARING,
			LOADED_PREPARED,
		};

	private:
		PROPERTY_PROVIDE(Screen);

		void ThreadProcContentLoadStarter();

		#pragma region Status related
		DECLARE_PROP_TYPE_RW(Screen, STATUS, Status, { return Status.Value; }, { Status.Value = Value; } );
		DECLARE_PROP_TYPE_RW(Screen, STATUS_CONTENT, StatusContent, { return StatusContent.Value; }, { StatusContent.Value = Value; } );
		Util::Thread ThreadContentLoad;
		#pragma endregion
		#pragma region Component related
		DECLARE_PROP_TYPE_RW(Screen, WINDOW_HITTEST, HitTestObjective, { return HitTestObjective.Value; }, { HitTestObjective.Value = Value; });
		#pragma endregion

		
	public:
		#pragma region Helper
		Util::EventBindHelper<Screen> EventBinder;
		#pragma endregion
		#pragma region Status related
		DECLARE_PROPERTY(Status);
		DECLARE_PROPERTY(StatusContent);
		Util::CriticalSection CsContentLoading;
		#pragma endregion
		#pragma region Component related
		UIComponent RootFrame;
		DECLARE_PROPERTY(HitTestObjective); 
		#pragma endregion

		
		Screen();
		virtual ~Screen();

		// Asynchronous Content Load.
		virtual void RaiseContentLoadAsync();
		// Not always unloads the content. Because of optimization of 'not' unloading contents.
		virtual void RaiseContentUnload();
		
		virtual void RaiseStart();
		virtual void RaiseEnd();
		virtual void RaiseLoaded();

		virtual void RaiseUpdate(double ElapsedTime);
		virtual void UpdateComponent(double ElapsedTime);
		virtual void RaiseDraw();

		virtual void RaisePrepareContent(bool IsRestoreAll);

		virtual void RaiseDeviceLost();
		virtual void RaiseDeviceReset();
		virtual void RaiseDeviceCreate();

		virtual void StartContentLoad();
		virtual void WaitForContentLoadFinish();

	private:
		// Raw device lost event. Just don't override.
		void OnRawDeviceLost();
		// Raw device reset event. Just don't override.
		void OnRawDeviceReset();
		// Raw device create event. Just don't override.
		void OnRawDeviceCreate();

	protected:
		// Initialize your graphic resource here. Note that this routine is asynchronous, so pay attention on it. Of coursely, you should not do anything except loading texture or such as.
		virtual void OnContentLoadAsync();
		// Fill your resource(such as texture), position your components depending on screen size on here. When device is lost and restored, it will be called so you can refill lost contents of textures and so on. And it matches the point when resolution is changed. As the result: fill, position your resource here.
		virtual void OnPrepareContent(bool IsRestoreAll);
		// Unload your graphic resource here if needed. You can optimize making your resource not be loaded/unloaded at each time by not unloading. If you did unload your resource, set *IsResourceUnloaded to true
		virtual void OnContentUnload(bool *IsResourceUnloaded);

		// After finished content loading.
		virtual void OnStart();
		// Moved to the other screen
		virtual void OnEnd();

		// Update routine that called per a frame. Update your logic here.
		virtual void OnUpdate(double ElapsedTime);
		// Draw your component here
		virtual void OnDraw();
		
		// When moved on from the other screen, before content loading routine.
		virtual void OnLoadingStart();
		// Update while content loading thread is running
		virtual void OnLoadingUpdate(double ElapsedTime);
		// Draw while content loading thread is running
		virtual void OnLoadingDraw();

		// Device Lost Event. This is handled with Load-Prepare-Start sequence. Don't recommend to use if you have no idea with it.
		virtual void OnDeviceLost();
		// Device Lost Event. This is handled with Load-Prepare-Start sequence. Don't recommend to use if you have no idea with it.
		virtual void OnDeviceReset();
		// Device Lost Event. This is handled with Load-Prepare-Start sequence. Don't recommend to use if you have no idea with it.
		virtual void OnDeviceCreate();
	};
}

