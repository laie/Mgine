#pragma once

#define RELEASE_OBJECT(obj)	{ if ((obj) && (obj)->Release()){ }; (obj) = 0; }

	namespace Mgine
{
	class EngineManager
	{
	public:
		enum WINDOW_ZORDER_RULE : BYTE
		{
			Normal = 0,
			BottomMost,
			TopMost
		};
		enum ENGINE_STATUS : BYTE
		{
			NotLoaded = 0,
			Loading,
			Loaded,
			Starting,
			Started,
			Finishing,
			Finished
		};

		class SettingIntegration;
		class WindowIntegration;
		class EngineIntegration;
		class CoreIntegration;
		struct MGINE_SETTING;

	public:
		static SettingIntegration	*Setting;
		static WindowIntegration	*Window;
		static EngineIntegration	*Engine;
		static CoreIntegration		*Core;

		static bool Init(MGINE_SETTING *Setting); // note that is not throwing exception
		static void Uninit();
		static bool Start();

		static void DoInit();
		static void DoStart();
		static void DoUninit();
		static void DoLoop();
		static void DoAppBegin();
		static void DoAppFinish();
		static void DoDeviceLost();
		static void DoDeviceReset();
		static void DoDeviceCreatePrepare();
		static void DoDeviceCreate();
		static void DoDeviceStart();

		static void OnInit();
		static void OnStart();
		static void OnUninit();
		static void OnAppBegin();
		static void OnAppFinish();
		static void OnDeviceLost();
		static void OnDeviceReset();
		static void OnDeviceCreate();
		static void OnDeviceStart();

		class SettingIntegration
		{
		private:
			inline SettingIntegration(){ }

			static MGINE_SETTING iSetting;
			static MGINE_SETTING iSettingSync;
			static inline void SetSettingSync()
			{ Util::LockBlock lock(CsSettingGet); iSettingSync = iSetting; }

		public:
			static Util::CriticalSection CsSetting;
			static Util::CriticalSection CsSettingGet;

			static void Init();
			static void Uninit();

			static void SetInitialSetting(MGINE_SETTING *Setting);
			static void ApplySettingInitial();

			static inline MGINE_SETTING GetSetting()
			{	Util::LockBlock lock(CsSettingGet); return iSettingSync; }

			static void SetAppName(wchar_t *GameName);
			static void SetShaderFilePath(wchar_t *ShaderFilePath);
			/*static void SethModule(HMODULE hModule); But i don't wanna set this*/
			static void SetResolution(DWORD Width, DWORD Height);
			static void SetResolutionMinMax(DWORD MinWidth, DWORD MinHeight, DWORD MaxWidth, DWORD MaxHeight);
			static void SetIsCursorVisible(bool IsCursorVisible);
			static void SetIsZEnableDefaultly(bool IsZEnableDefaultly);
			static void SetIsDwmWindow(bool IsDwmWindow);
			static void SetIsDwmWindowFrameless(bool IsDwmWindowFrameless);
			static void SetIsDragDropEnabled(bool IsDragDropEnabled);
			static void SetZOrderRule(EngineManager::WINDOW_ZORDER_RULE Rule);
			static void SetBlurFactor(float BlurFactor);
			static void SetOnInit(CallbackInit ProcInit);
			static void SetOnStart(CallbackStart ProcStart);
			static void SetOnUninit(CallbackUninit ProcUninit);
			static void SetOnUpdate(CallbackUpdate ProcUpdate);
			static void SetOnUpdateAfter(CallbackUpdate ProcUpdateAfter);
			static void SetOnDraw(CallbackDraw ProcDraw);
			static void SetOnDrawAfter(CallbackDraw ProcDrawAfter);
			static void SetOnStreamUpdateAsync(CallbackStreamUpdate ProcStreamUpdateAsync);
			static void SetOnDropFileAsync(CallbackDropFile ProcDropFile);
		};

		class WindowIntegration
		{
		private:
			static RECT RectWindow;
			static POINT PosWindow;

			static WINDOW_HITTEST WindowHitTest;
			static WINDOW_HITTEST WindowHitTestObjective;

			static inline void SetMouseHitTest(WINDOW_HITTEST MouseHitTest)
			{ Util::LockBlock lock(CsHitTest); EngineManager::WindowIntegration::WindowHitTest = MouseHitTest; }
			static inline WINDOW_HITTEST GetMouseHitTest()
			{ Util::LockBlock lock(CsHitTest); return WindowHitTest; }


		public:
			static Util::CriticalSection CsWindow;
			static Util::CriticalSection CsHitTest;
			static DWORD TidWnd;
			static HWND hWnd;

			static inline RECT GetRectWindow()
			{ Util::LockBlock lock(CsWindow); return RectWindow; }
			static inline POINT GetPosWindow()
			{ Util::LockBlock lock(CsWindow); return PosWindow; }

			static inline void SetMouseHitTestObjective(WINDOW_HITTEST MouseHitTestObjecitve)
			{ WindowIntegration::WindowHitTestObjective = MouseHitTestObjecitve; }

			// should be called in engine's loop thread. otherwise, don't guarantee async problem.
			static void MouseHitTestUpdate();

			static struct STATE_INTERNAL
			{
				bool IsIgnoreZOrder;
				bool IsStateRestoreMinimize;
			} Internal;


			static void Init(MGINE_SETTING *Setting);
			static void Start();
			static void Uninit();

			static void FlushMessage();
			static void SetWindow();
			static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

			static std::wstring OpenFileDialog(LPCWSTR Filter = L"All files(*)\0*\0Text files(*.txt, *.log)\0*.txt;*.log\0");
			
			static bool GetIsCursorInClientRect();
		};

		class EngineIntegration
		{
		public:
			static Util::CriticalSection CsEngine;

			static Util::Thread EngineThread;

			static Util::Thread ThreadCheckSpeedHack;
			static HANDLE hTimerSpeedHack;

			static LPDIRECT3D9 gD3D;
			static LPDIRECT3DDEVICE9 gDevice;

			static D3DPRESENT_PARAMETERS D3dpp;


			static D3DMULTISAMPLE_TYPE FindFitMultiSampleType();

			static void Init();
			static void Uninit();

			static void InitDevice(IDirect3DDevice9 *NewDevice);

			static void SubReloadDevice();
			static void ReloadDevice(); // this should not be called in draw procedures.

			static struct LOOP_STATE
			{
				double UpdateProceededTick;
				LONGLONG QFFrequency;

				void OnStart();
			} LoopState;

			static void ProcThreadEngine();
			static void ProcThreadCheckSpeedHack();
		};

		class CoreIntegration
		{
		public:
			static void Init();
			static void Uninit();

			static ENGINE_STATUS Status;

			static HMODULE hModuleDwm;
			static bool IsVistaDwmFeatureEnabled;

			static bool IsAppFinished;

			static bool IsTerminating;
			static bool IsReloading;

		};

		typedef struct MGINE_SETTING
		{
			void Default();

			wchar_t AppName[1024]; 
			wchar_t ShaderFilePath[MAX_PATH];

			HMODULE	hModule;

			DWORD ResolutionWidth;
			DWORD ResolutionHeight;
			DWORD ResolutionMinWidth;
			DWORD ResolutionMinHeight;
			DWORD ResolutionMaxWidth;
			DWORD ResolutionMaxHeight;

			bool IsCursorVisible;
			bool IsZEnableDefaultly;
			bool IsDwmWindow; // Glass
			bool IsDwmWindowFrameless; // Depends on isdwmwindow
			bool IsDragDropEnabled;

			WINDOW_ZORDER_RULE WindowZOrderRule;

			float BlurFactor;

			CallbackInit   OnInit;
			CallbackStart  OnStart;
			CallbackUninit OnUninit;
			CallbackUpdate OnUpdate;
			CallbackUpdate OnUpdateAfter;
			CallbackDraw   OnDraw;
			CallbackDraw   OnDrawAfter;
			CallbackStreamUpdate	OnStreamUpdateAsync;
			CallbackDropFile		OnDropFileAsync;

			inline MGINE_SETTING(){ Default(); }
		} *PMGINE_SETTING;


	};

}