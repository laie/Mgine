#pragma once

// #define MGINE_INPUT_BUFFER_SIZE			20
// #define MGINE_INPUT_QUEUE_EVENT_SIZE	1024
#include "Engine.h"

namespace Mgine
{
	class Input
	{
	public:
		struct KEY_CHUNK
		{
			KEYSTATUS Status;
			double Elapsed;

			inline bool Hit() const { return Status == KEYSTATUS_HIT; }
			inline bool Down() const { return Status != KEYSTATUS_UP; }
			inline bool Up() const { return Status == KEYSTATUS_UP; }
		};
	private:
		static HWND hWnd;
		static bool IsCursorInClient;

		static bool IsInited;

		static LPDIRECTINPUT gDI;
		static LPDIRECTINPUTDEVICE gKeyboard;
		static LPDIRECTINPUTDEVICE gMouse;

		static HANDLE hMouseEvent, hKeyboardEvent;

	public:
		static const int InputBufferSize	= 20;

		static void Init(HWND hWnd);
		static void Uninit();

		static void Update(double ElapsedTime);
		static bool CheckAcquire();

		static inline bool GetIsCursorInClient()
		{ Util::LockBlock lb(EngineManager::Window->CsWindow); return IsCursorInClient; }

		static inline RECT GetContentRect()
		{
			RECT rc, rcclient, rcwindow;
			ATHROW(GetClientRect(hWnd, &rcclient));
			rc = rcclient;
			ATHROW(GetWindowRect(hWnd, &rcwindow));

			ATHROW(AdjustWindowRectEx(&rcclient, GetWindowLong(hWnd, GWL_STYLE), false, GetWindowLong(hWnd, GWL_EXSTYLE)));
			
			rcwindow.left	-= rcclient.left;
			rcwindow.right	-= rcclient.right  - rc.right;
			rcwindow.top	-= rcclient.top;
			rcwindow.bottom	-= rcclient.bottom - rc.bottom;

			return rcwindow;
		}

		static class Mouse
		{
		public:

		private:
			PROPERTY_PROVIDE(Mouse);

			KEY_CHUNK ArrChunk[8];
			KEY_CHUNK ArrBeforeChunk[8];

			DECLARE_PROP_TYPE_R(Mouse, LONG, X, { return X.Value; }, { X.Value = Value; });
			DECLARE_PROP_TYPE_R(Mouse, LONG, Y, { return Y.Value; }, { Y.Value = Value; });
			DECLARE_PROP_TYPE_R(Mouse, LONG, Z, { return Z.Value; }, { Z.Value = Value; });
			DECLARE_PROP_TYPE_R(Mouse, LONG, BeforeX, { return BeforeX.Value; }, { BeforeX.Value = Value; });
			DECLARE_PROP_TYPE_R(Mouse, LONG, BeforeY, { return BeforeY.Value; }, { BeforeY.Value = Value; });
			DECLARE_PROP_TYPE_R(Mouse, LONG, BeforeZ, { return BeforeZ.Value; }, { BeforeZ.Value = Value; });

		public:
			DECLARE_PROPERTY(X);
			DECLARE_PROPERTY(Y);
			DECLARE_PROPERTY(Z);
			DECLARE_PROPERTY(BeforeX);
			DECLARE_PROPERTY(BeforeY);
			DECLARE_PROPERTY(BeforeZ);

			inline const KEY_CHUNK & operator[] (UINT Param) const
			{
				ATHROWR(0 <= Param && Param < COUNTOF(ArrChunk), "Param %d", Param);
				return ArrChunk[Param];
			}

			inline bool IsChanged() const
			{
				if (   X != BeforeX
					|| Y != BeforeY
					|| Z != BeforeZ
					|| memcmp(ArrChunk, ArrBeforeChunk, sizeof(ArrChunk))
				) return true;
				else return false;
			}

			inline MOUSE_STATE GetMouseState() const
			{
				MOUSE_STATE ms;
				for ( int i=0; i < COUNTOF(ArrChunk); i++ )
					if ( ArrChunk[i].Down() )
						ms.Key[i] = KEY_RAW_DOWN;
					else ms.Key[i] = KEY_RAW_UP;
				ms.X = X;
				ms.Y = Y;
				ms.Z = Z;

				return ms;
			}

			inline void Update(const MOUSE_STATE & MouseState, double ElapsedTime)
			{
				memcpy(ArrBeforeChunk, ArrChunk, sizeof(ArrChunk));
				BeforeX = X;
				BeforeY = Y;
				BeforeZ = Z;
				X = MouseState.X;
				Y = MouseState.Y;
				Z = MouseState.Z;

				for ( int i=0; i < COUNTOF(ArrChunk); i++ )
				{
					if ( MouseState.Key[i] )
					{
						if ( ArrChunk[i].Status == KEYSTATUS_UP )
						{
							ArrChunk[i].Elapsed = ElapsedTime;
							ArrChunk[i].Status = KEYSTATUS_HIT;
						} else 
						{
							ArrChunk[i].Elapsed += ElapsedTime;
							ArrChunk[i].Status = KEYSTATUS_DOWN;
						}
					} else 
					{
						ArrChunk[i].Status = KEYSTATUS_UP;
						ArrChunk[i].Elapsed = 0;
					}
				}
			}
		public:
			inline Mouse()
			{
				memset(ArrChunk, 0, sizeof(ArrChunk));
				memset(ArrBeforeChunk, 0, sizeof(ArrBeforeChunk));
				X = Y = Z = 0;
				BeforeX = BeforeY = BeforeZ = 0;
			}
		} Mouse;

		static class Key
		{
		private:
			KEY_CHUNK ArrChunk[256];
			KEY_CHUNK ArrBeforeChunk[256];

		public:
			inline Key()
			{
				memset(ArrChunk, 0, sizeof(ArrChunk));
				memset(ArrBeforeChunk, 0, sizeof(ArrBeforeChunk));
			}

			inline const KEY_CHUNK & operator[](KEYCODE KeyCode) const
			{
				ATHROWR(0 <= KeyCode && KeyCode < COUNTOF(ArrChunk), "Key is out of range %d", KeyCode);
				return ArrChunk[(BYTE)KeyCode];
			}

			inline bool IsChanged() const
			{
				if ( memcmp(ArrChunk, ArrBeforeChunk, sizeof(ArrChunk)) )
					return true;
			}

			inline void Update(const KEY_STATE & KeyState, double ElapsedTime)
			{
				memcpy(ArrBeforeChunk, ArrChunk, sizeof(ArrChunk));

				for ( int i=0; i < COUNTOF(ArrChunk); i++ )
				{
					if ( KeyState.Key[i] )
					{
						if ( ArrChunk[i].Status == KEYSTATUS_UP )
						{
							ArrChunk[i].Elapsed = ElapsedTime;
							ArrChunk[i].Status = KEYSTATUS_HIT;
						} else 
						{
							ArrChunk[i].Elapsed += ElapsedTime;
							ArrChunk[i].Status = KEYSTATUS_DOWN;
						}
					} else 
					{
						ArrChunk[i].Status = KEYSTATUS_UP;
						ArrChunk[i].Elapsed = 0;
					}
				}
			}
		} Key;
	};
}