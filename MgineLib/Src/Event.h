#pragma once

namespace Mgine
{
	class UIListItem;

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
		MOUSE_STATE State;
		WINDOW_HITTEST WindowHittestObjective;

		inline MouseEventArg(MOUSE_STATE State)
			: EventArg()
		{
			this->State = State;
			this->WindowHittestObjective = HITTEST_UNSET;
		}
	};

	class KeyEventArg
		: public EventArg
	{
	public:
		KEYCODE KeyCode;
		KEYSTATUS KeyStatus;

		inline KeyEventArg(KEYCODE KeyCode, KEYSTATUS KeyStatus)
			: EventArg()
		{
			this->KeyCode = KeyCode;
			this->KeyStatus = KeyStatus;
		}
	};

	class CharEventArg
		: public EventArg
	{
	public:
		CHARCODE Char; // Character pressed. Captured by WndProc

		inline CharEventArg(CHARCODE Char)
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
		RECTANGLE OldPosition;

		inline PositionEventArg(RECTANGLE& OldPosition)
		{
			this->OldPosition = OldPosition;
		}

	};


	typedef boost::signal<void ()>						Event;
	typedef boost::signal<void (MouseEventArg *)>		MouseEvent;
	typedef boost::signal<void (KeyEventArg *)>			KeyEvent;
	typedef boost::signal<void (CharEventArg *)>		CharEvent;
	typedef boost::signal<void (ItemSelectEventArg *)>	ItemSelectEvent;
	typedef boost::signal<void (ItemActionEventArg *)>	ItemActionEvent;
	typedef boost::signal<void (PositionEventArg *)>	PositionEvent;

	typedef boost::signal<void ()>						DeviceLostEvent;
	typedef boost::signal<void ()>						DeviceResetEvent;
	typedef boost::signal<void ()>						DeviceCreateEvent;
}

