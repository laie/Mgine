#pragma once
#include "Event.h"
#include "Animation.h"
#include "DeviceResource.h"

namespace Mgine
{
	class UIComponent;

	class ComponentManager
	{
	private:
		static bool IsInited;

	public:
		static inline bool GetIsInited()
		{ return IsInited; }

		static void Init();
		static void Uninit();
	
		static void OnAppFinish();
	
		static HRGN GetAllocRegionFromBuffer(BYTE *Buffer, int Width, int Height);

	};

	enum ROWCOL_ALIGNMENT
	{
		BOTTOM,
		TOP,

		LEFT,
		RIGHT,

		CENTER,

		STRETCH,
	};

	class ComponentPlacer
	{
		PROPERTY_PROVIDE(ComponentPlacer);
		DECLARE_PROP_TYPE_R(ComponentPlacer, UINT, IndexInOwner,
			,
			{ NRTHROW(); });
	protected:
		UIComponent *Owner;
		DECLARE_PROPERTY(IndexInOwner);


		ComponentPlacer()
		{
			this->Owner = 0;
			this->IndexInOwner.Value = -1;
		}

	public:
		virtual ~ComponentPlacer(){ };

		virtual void SetOwner(UIComponent *Owner, int IndexInOwner);

		inline virtual RECTANGLE GetComponentPosition(UIComponent const *Component) const
		{
			VEC sizeafterset = 0;
			return GetComponentPosition(Component, sizeafterset);
		}
		virtual RECTANGLE GetComponentPosition(UIComponent const *Component, VEC & SizeAfterSet) const;
		virtual RECTANGLE ComponentPositionChanged(UIComponent const *Component);
		virtual void UpdatePlacerPosition();
		virtual void PositionComponent();
	};

	// Multiple rows
	class RowPlacer
		: public ComponentPlacer
	{
		VEC MinSizeWidth, MaxSizeWidth; // Min/Max width(or height) could be. zero means default behavior
		VEC SizeWidth; // Calculated the row's width ( or col's height)
		VEC PosX;  // X or Y related on owner ui component
		ROWCOL_ALIGNMENT VerticalAlign, HorizontalAlign;

	public:
		inline RowPlacer(
			VEC MinWidth,
			VEC MaxWidth,
			ROWCOL_ALIGNMENT HorizontalAlign,
			ROWCOL_ALIGNMENT VerticalAlign
			)
		{
			ATHROW(MinWidth >= 0);
			ATHROW(MaxWidth >= 0);
			ATHROW(!(MaxWidth != 0 && MinWidth != 0) || MaxWidth >= MinWidth);
			ATHROW(VerticalAlign == TOP
				|| VerticalAlign == CENTER
				|| VerticalAlign == BOTTOM
				|| VerticalAlign == STRETCH
				);
			ATHROW(HorizontalAlign == LEFT
				|| HorizontalAlign == CENTER
				|| HorizontalAlign == RIGHT
				|| HorizontalAlign == STRETCH
				);

			//this->Owner = Owner;
			this->MinSizeWidth = MinWidth;
			this->MaxSizeWidth = MaxWidth;
			this->VerticalAlign = VerticalAlign;
			this->HorizontalAlign = HorizontalAlign;
			this->PosX = 0;

			if ( MaxWidth ) this->SizeWidth = MaxWidth;
			else if ( MinWidth ) this->SizeWidth = MinWidth;
			else this->SizeWidth = 0;
		}

		virtual RECTANGLE GetComponentPosition(UIComponent const *Component, VEC & SizeAfterSet) const;
		virtual RECTANGLE ComponentPositionChanged(UIComponent const *Component);
		virtual void UpdatePlacerPosition();
	};

	// Multiple cols
	class ColPlacer
		: public ComponentPlacer
	{
		VEC MinSizeHeight, MaxSizeHeight; // Min/Max width(or height) could be. zero means default behavior
		VEC SizeHeight; // Calculated the row's width ( or col's height)
		VEC PosY;  // X or Y related on owner ui component
		ROWCOL_ALIGNMENT VerticalAlign, HorizontalAlign;

	public:
		inline ColPlacer(
			VEC MinHeight,
			VEC MaxHeight,
			ROWCOL_ALIGNMENT HorizontalAlign,
			ROWCOL_ALIGNMENT VerticalAlign
			)
		{
			ATHROW(MinHeight >= 0);
			ATHROW(MaxHeight >= 0);
			ATHROW(!(MaxHeight != 0 && MinHeight != 0) || MaxHeight >= MinHeight);
			ATHROW(VerticalAlign == TOP
				|| VerticalAlign == CENTER
				|| VerticalAlign == BOTTOM
				|| VerticalAlign == STRETCH
				);
			ATHROW(HorizontalAlign == LEFT
				|| HorizontalAlign == CENTER
				|| HorizontalAlign == RIGHT
				|| HorizontalAlign == STRETCH
				);

			this->MinSizeHeight = MinHeight;
			this->MaxSizeHeight = MaxHeight;
			this->VerticalAlign = VerticalAlign;
			this->HorizontalAlign = HorizontalAlign;
			this->PosY = 0;

			if ( MaxHeight ) this->SizeHeight = MaxHeight;
			else if ( MinHeight ) this->SizeHeight = MinHeight;
			else this->SizeHeight = 0;
		}
		virtual RECTANGLE GetComponentPosition(UIComponent const *Component, VEC & SizeAfterSet) const;
		virtual RECTANGLE ComponentPositionChanged(UIComponent const *Component);
		virtual void UpdatePlacerPosition();
	};

	class UIComponent
	{
		PROPERTY_PROVIDE(UIComponent);

	public:
		enum STATUS : BYTE
		{
			ENDED = 0,
			STARTED 
		};

		enum CONTENT_STATUS : BYTE
		{
			UNLOADED = 0,
			LOADED_PREPARING,
			LOADED_PREPARED
		};

		enum CHILD_POSITIONING_TYPE
		{
			HORIZONTAL,
			VERTICAL,
			MANUAL
		};

		struct EVENTSTATE // Event State that should not be written externally. 
		{
		public:
			struct MOUSE
			{
			public:
				bool IsOn, IsDown;
				POINT DragStartPoint;
				VECTOR2 DragVelocityTo;
				
				inline bool operator==(MOUSE &A) const
				{ return 0 == memcmp(this, &A, sizeof(MOUSE)) ; }
				inline bool operator!=(MOUSE &A) const
				{ return 0 != memcmp(this, &A, sizeof(MOUSE)) ; }
			};
			
			MOUSE Mouse;
		};

	protected:
		#pragma region Position Property Declarations
		DECLARE_PROP_TYPE_RW(UIComponent, RECTANGLE, Position,
			{ return Position.Value; }, );
			
		DECLARE_PROP_TYPE_R(UIComponent, RECTANGLE, ActualPosition,
			{ return ActualPosition.Value; },
			);
		DECLARE_PROP_TYPE_RW(UIComponent, LAYOUT_OFFSET, LayoutOffset,
			{ return LayoutOffset.Value; },
			{
				LayoutOffset.Value = Value;
				CalcActualPosition();
			} );
		DECLARE_PROP_TYPE_RW(UIComponent, VEC, ZOrder, { return ZOrder.Value; }, { ZOrder.Value = Value; });
		#pragma endregion
		
		#pragma region Status related Property Declarations
		DECLARE_PROP_TYPE_R(UIComponent, STATUS, Status, { return Status.Value; }, { Status.Value = Value; } );
		DECLARE_PROP_TYPE_R(UIComponent, CONTENT_STATUS, StatusContent, { return StatusContent.Value; }, { StatusContent.Value = Value; } );
		DECLARE_PROP_TYPE_R(UIComponent, bool, IsVisible, { return IsVisible.Value; }, { IsVisible.Value = Value; } );
		DECLARE_PROP_TYPE_R(UIComponent, bool, IsUsingStencil, { return IsUsingStencil.Value; }, { IsUsingStencil.Value = Value; } );
		DECLARE_PROP_TYPE_R(UIComponent, bool, IsStarted, { return IsStarted.Value = (Status == STARTED); }, { NRTHROW(); } );
		DECLARE_PROP_TYPE_R(UIComponent, bool, IsInited, { return IsInited.Value = (StatusContent == LOADED_PREPARED); }, { NRTHROW(); } );
		#pragma endregion
		
		#pragma region Event State Fields
		DECLARE_PROP_TYPE_R(UIComponent, EVENTSTATE, StateEvent, { return StateEvent.Value; }, { StateEvent.Value = Value; } );
		DECLARE_PROP_TYPE_R(UIComponent, EVENTSTATE, PrevStateEvent, { return PrevStateEvent.Value; }, { PrevStateEvent.Value = Value; } );
		#pragma endregion
		
		#pragma region Parent Control Fields
		DECLARE_PROP_TYPE_R(UIComponent, UIComponent *, Parent, { return Parent.Value; }, { Parent.Value = Value; } );
		DECLARE_PROP_TYPE_R(UIComponent, UINT, IndexInParent,
			{
				if ( !Parent ) return IndexInParent.Value = -1;
				UIComponent *component = NULL;
				if ( Parent->ArrComponent().size() > IndexInParent.Value ) component = Parent->ArrComponent()[IndexInParent.Value];
				if ( component == this ) return IndexInParent.Value;
				for ( UINT i=0; i < Parent->ArrComponent().size(); i++ )
					if ( Parent->ArrComponent()[i] == this )
					{
						IndexInParent.Value = i;
						return IndexInParent.Value;
					}
				MTHROW(InvalidOperation, "Component is not found",  );

			},
			{ NRTHROW(); });
		DECLARE_PROP_TYPE_RW(UIComponent, UIComponent *, FocusedChild,
			{ return FocusedChild.Value; },
			{ ATHROW(Value->Parent == this); FocusedChild.Value = Value; });
		#pragma endregion
		
		#pragma region Child Control Fields
		//DECLARE_PROP_TYPE_R(UIComponent, TriList<UIComponent*>, ArrComponent, { return ArrComponent.Value; }, {} );
		DECLARE_PROP_TYPE_R(UIComponent, std::vector<UIComponent*>, ArrComponent, { return ArrComponent.Value; }, { ArrComponent.Value = Value; });
		DECLARE_PROP_TYPE_R(UIComponent, boost::ptr_vector<ComponentPlacer>/*(TriList<ComponentPlacer*, true>)*/, ListPlacer, { return ListPlacer.Value; }, { NRTHROW(); } );
		#pragma endregion

		#pragma region Decoration Fields
		DECLARE_PROP_TYPE_RW(UIComponent, COLOR, ForeColor, { return ForeColor.Value; }, { ForeColor.Value = Value; });
		DECLARE_PROP_TYPE_RW(UIComponent, COLOR, BackColor, { return BackColor.Value; }, { BackColor.Value = Value; });
		#pragma endregion

	protected:
		VECTOR2		GetAdjustPositionToScreen(VEC X, VEC Y);
		VECTOR2		GetAdjustPositionToMe(VEC X, VEC Y);
		RECTANGLE	GetActualPosition();
		void		CalcActualPosition();
		
		Util::EventBindHelper<UIComponent> EventBinder;
		AnimationPoolHelper AnimationProvider;
		
	public:
		#pragma region Status Properties
		DECLARE_PROPERTY(Status);
		DECLARE_PROPERTY(StatusContent);
		
		DECLARE_PROPERTY(IsVisible);
		DECLARE_PROPERTY(IsUsingStencil);
		#pragma endregion
		
		#pragma region Parent Properties
		DECLARE_PROPERTY(Parent);
		DECLARE_PROPERTY(IndexInParent);
		DECLARE_PROPERTY(FocusedChild); 
		#pragma endregion

		#pragma region Event State Properties
		DECLARE_PROPERTY(PrevStateEvent);	// Previous Event State
		DECLARE_PROPERTY(StateEvent);		// Event State
		#pragma endregion
		

		#pragma region Position Properties
		DECLARE_PROPERTY(Position);
		DECLARE_PROPERTY(ActualPosition);
		DECLARE_PROPERTY(LayoutOffset);
		DECLARE_PROPERTY(ZOrder);
		AnimationPool	AnimationPositionX;
		AnimationPool	AnimationPositionY;
		AnimationPool	AnimationPositionWidth;
		AnimationPool	AnimationPositionHeight;
		AnimationPool	AnimationMarginLeft;
		AnimationPool	AnimationMarginRight;
		AnimationPool	AnimationMarginTop;
		AnimationPool	AnimationMarginBottom;
		AnimationPool	AnimationPaddingLeft;
		AnimationPool	AnimationPaddingRight;
		AnimationPool	AnimationPaddingTop;
		AnimationPool	AnimationPaddingBottom;
		#pragma endregion
		
		#pragma region Child Control Properties
		DECLARE_PROPERTY(ArrComponent);

		DECLARE_PROPERTY(IsStarted);
		DECLARE_PROPERTY(IsInited);
		
		DECLARE_PROPERTY(ListPlacer);
		#pragma endregion

		#pragma region Decoration Properties
		DECLARE_PROPERTY(ForeColor); 
		DECLARE_PROPERTY(BackColor); 
		AnimationPool	AnimationForeColorA;
		AnimationPool	AnimationBackColorA;
		#pragma endregion

		// User own data
		boost::any UserData;

		
		inline UIComponent()
		{
			EventBinder.Init(this);

			//ComponentIndex = ComponentManager::AddComponent(this);
			
			//FocusedChildIndex = -1;
			
			Parent = NULL;
		
			ForeColor = 0xffffffff;
			BackColor = 0;
			
			Status = ENDED;
			StatusContent = UNLOADED;
			
			IsVisible = true;
			IsUsingStencil = false;
			
			RECT rectdefault = { 2, 2, 2, 2 }; // defaultly
			LayoutOffset.Value.Margin = rectdefault;

			RECTANGLE pos = RECTANGLE(0, 0, 100, 100);
			Position = pos;

			AnimationPositionX.BindMember(
				boost::function<void (VEC)>([this](VEC A) mutable { RECTANGLE tmp = Position; tmp.X = A; Position = tmp; }),
				boost::function<VEC ()>([this]() -> VEC { return Position().X; })
			);
			AnimationPositionY.BindMember(
				boost::function<void (VEC)>([this](VEC A) mutable { RECTANGLE tmp = Position; tmp.Y = A; Position = tmp; }),
				boost::function<VEC ()>([this]() -> VEC { return Position().Y; })
			);
			AnimationPositionWidth.BindMember(
				boost::function<void (VEC)>([this](VEC A) mutable { RECTANGLE tmp = Position; tmp.Width = A; Position = tmp; }),
				boost::function<VEC ()>([this]() -> VEC { return Position().Width; })
			);
			AnimationPositionHeight.BindMember(
				boost::function<void (VEC)>([this](VEC A) mutable { RECTANGLE tmp = Position; tmp.Height = A; Position = tmp; }),
				boost::function<VEC ()>([this]() -> VEC { return Position().Height; })
			);

			AnimationMarginLeft.BindMember(
				boost::function<void (VEC)>([this](VEC A) mutable { LAYOUT_OFFSET tmp = LayoutOffset; tmp.Margin.left = (LONG)A; }),
				boost::function<VEC ()>([this]() -> VEC { return (VEC)LayoutOffset().Margin.left; })
			);
			AnimationMarginRight.BindMember(
				boost::function<void (VEC)>([this](VEC A) mutable { LAYOUT_OFFSET tmp = LayoutOffset; tmp.Margin.right = (LONG)A; }),
				boost::function<VEC ()>([this]() -> VEC { return (VEC)LayoutOffset().Margin.right; })
			);
			AnimationMarginTop.BindMember(
				boost::function<void (VEC)>([this](VEC A) mutable { LAYOUT_OFFSET tmp = LayoutOffset; tmp.Margin.top = (LONG)A; }),
				boost::function<VEC ()>([this]() -> VEC { return (VEC)LayoutOffset().Margin.top; })
			);
			AnimationMarginBottom.BindMember(
				boost::function<void (VEC)>([this](VEC A) mutable { LAYOUT_OFFSET tmp = LayoutOffset; tmp.Margin.bottom = (LONG)A; }),
				boost::function<VEC ()>([this]() -> VEC { return (VEC)LayoutOffset().Margin.bottom; })
			);

			AnimationPaddingLeft.BindMember(
				boost::function<void (VEC)>([this](VEC A) mutable { LAYOUT_OFFSET tmp = LayoutOffset; tmp.Padding.left = (LONG)A; }),
				boost::function<VEC ()>([this]() -> VEC { return (VEC)LayoutOffset().Padding.left; })
			);
			AnimationPaddingRight.BindMember(
				boost::function<void (VEC)>([this](VEC A) mutable { LAYOUT_OFFSET tmp = LayoutOffset; tmp.Padding.right = (LONG)A; }),
				boost::function<VEC ()>([this]() -> VEC { return (VEC)LayoutOffset().Padding.right; })
			);
			AnimationPaddingTop.BindMember(
				boost::function<void (VEC)>([this](VEC A) mutable { LAYOUT_OFFSET tmp = LayoutOffset; tmp.Padding.top = (LONG)A; }),
				boost::function<VEC ()>([this]() -> VEC { return (VEC)LayoutOffset().Padding.top; })
			);
			AnimationPaddingBottom.BindMember(
				boost::function<void (VEC)>([this](VEC A) mutable { LAYOUT_OFFSET tmp = LayoutOffset; tmp.Padding.bottom = (LONG)A; }),
				boost::function<VEC ()>([this]() -> VEC { return (VEC)LayoutOffset().Padding.bottom; })
			);
			AnimationForeColorA.BindMember(
				boost::function<void (VEC)>([this](VEC A) mutable { COLOR tmp = ForeColor; tmp.A = (BYTE)(A*255); ForeColor = tmp; }),
				boost::function<VEC ()>([this]() -> VEC { return (VEC)255*ForeColor().A; })
			);
			AnimationBackColorA.BindMember(
				boost::function<void (VEC)>([this](VEC A) mutable { COLOR tmp = BackColor; tmp.A = (BYTE)(A*255); BackColor = tmp; }),
				boost::function<VEC ()>([this]() -> VEC { return (VEC)255*BackColor().A; })
			);

			AnimationProvider.AddPool(AnimationPositionX);
			AnimationProvider.AddPool(AnimationPositionY);
			AnimationProvider.AddPool(AnimationPositionWidth);
			AnimationProvider.AddPool(AnimationPositionHeight);
			AnimationProvider.AddPool(AnimationMarginLeft);
			AnimationProvider.AddPool(AnimationMarginRight);
			AnimationProvider.AddPool(AnimationMarginTop);
			AnimationProvider.AddPool(AnimationMarginBottom);
			AnimationProvider.AddPool(AnimationPaddingLeft);
			AnimationProvider.AddPool(AnimationPaddingRight);
			AnimationProvider.AddPool(AnimationPaddingTop);
			AnimationProvider.AddPool(AnimationPaddingBottom);
			AnimationProvider.AddPool(AnimationForeColorA);
			AnimationProvider.AddPool(AnimationBackColorA);
		}
		
		virtual ~UIComponent();

		virtual void RaiseChildContentLoad();
		virtual void RaiseContentLoad();
		virtual void RaiseChildContentPrepare(bool IsRestoreAll);
		virtual void RaiseContentPrepare(bool IsRestoreAll);
		virtual void RaiseChildContentUnload();
		virtual void RaiseContentUnload();

		virtual void RaiseChildStart();
		virtual void RaiseStart();
		virtual void RaiseChildEnd();
		virtual void RaiseEnd();

		virtual void RaiseDraw();
		virtual void RaiseDrawChild();
		virtual void RaiseUpdate(double ElapsedTime);
		virtual void RaiseUpdateChild(double ElapsedTime);

		virtual void RaiseChildCalcActualPosition();

		virtual void RaiseFocusGotEvent();
		virtual void RaiseFocusLostEvent();
		virtual void RaiseCharEvent(CharEventArg *Arg);
		virtual void RaiseMouseEventPoint(MouseEventArg *Arg);
		

		virtual bool IsMouseCollision(POINT MousePosition);
		virtual bool IsFocused();
	
		virtual void Focus();
		
		virtual void SetParent(UIComponent *NewParent);
		
		/* Used by child component's SetParent() method */
		virtual void AddChildComponent(UIComponent *Component);
		virtual void RemoveChildComponent(UIComponent *Component);

		virtual UINT AddRawPlacer(ComponentPlacer *PlacerManaged);
		virtual void SetRawPlacer(int PlacerIndex, ComponentPlacer *PlacerManaged);
		virtual void PlacerPositionChanged(ComponentPlacer *Placer);
		virtual void UpdatePlacerPosition();
		
		virtual void PlaceChildComponent(int ChildIndex);
		virtual void PlaceChildComponent();
		virtual void PlaceComponent();
		virtual void PlaceComponent(const ComponentPlacer *Placer);

	protected:
		virtual void OnContentLoad();
		virtual void OnContentPrepare(bool IsRestoreAll);
		virtual void OnContentUnload(bool *IsUnloaded);

		virtual void OnStart();
		virtual void OnEnd();

		virtual void OnDraw();
		virtual void OnDrawChildren();
		virtual void OnUpdate(double ElapsedTime);

		//virtual void OnParentInternalPositionChanged();

		virtual void OnFocusGotEvent();
		virtual void OnFocusLostEvent();
		virtual void OnCharEvent(CharEventArg *Arg);
		virtual void OnMouseEventPoint(MouseEventArg *Arg);
		virtual void OnEventPoint();

	public:
		Event EventFocusGot;
		Event EventFocusLost;
	
		Event EventStart;
		Event EventEnd;

		Event EventUpdate;
		Event EventDraw;

		PositionEvent	EventActualPositionChanged;

		MouseEvent EventMouseOn;
		MouseEvent EventMouseLeave;
		MouseEvent EventMouseDown;
		MouseEvent EventMouseUp;
		MouseEvent EventMouseClick;
		MouseEvent EventMouseMove;
	
		CharEvent EventChar;

		KeyEvent EventKeyDown; // not yet implemented
		KeyEvent EventKeyUp;
		KeyEvent EventKeyPress;

	};

	UINT &ComponentPlacer::__GetIndexInOwner() const
	{
		if ( !Owner ) return IndexInOwner.Value = -1;
		const ComponentPlacer *placer = NULL;
		if ( Owner->ListPlacer().size() > IndexInOwner.Value ) placer = &Owner->ListPlacer()[IndexInOwner.Value];
		if ( placer == this ) return IndexInOwner.Value;
		for ( UINT i=0; i < Owner->ListPlacer().size(); i++ )
			if ( &Owner->ListPlacer()[i] == this )
			{
				IndexInOwner.Value = i;
				return IndexInOwner.Value;
			}
		MTHROW(InvalidOperation, "Component is not found",  );
	}

	// This is a row, so contains horizontal cols
	class UIRow : public UIComponent
	{
	private:
		using UIComponent::AddRawPlacer;
		using UIComponent::SetRawPlacer;

	public:
		inline UIRow()
		{
#ifdef _DEBUG
#else
			ForeColor = 0;
#endif
			LayoutOffset = LAYOUT_OFFSET();
			//SetChildPositioningType(HORIZONTAL);
		}
		virtual UINT AddPlacer(VEC MinWidth, VEC MaxWidth, ROWCOL_ALIGNMENT HorizontalAlign, ROWCOL_ALIGNMENT VerticalAlign);
	};

	// This is a col, so contains vertical rows
	class UICol : public UIComponent
	{
	private:
		using UIComponent::AddRawPlacer;
		using UIComponent::SetRawPlacer;

	public:
		inline UICol()
		{
#ifdef _DEBUG
#else
			ForeColor = 0;
#endif
			LayoutOffset = LAYOUT_OFFSET();
			//SetChildPositioningType(VERTICAL);
		}
		
		virtual UINT AddPlacer(VEC MinHeight, VEC MaxHeight, ROWCOL_ALIGNMENT HorizontalAlign, ROWCOL_ALIGNMENT VerticalAlign);
	};



	class UIScroll : 
		public UIComponent
	{
	private:
	public:
		float Value;
		float ClickedX;
		bool IsScrolling;

		inline UIScroll()
		{
			Value = ClickedX = 0;
			IsScrolling = false;
		}
		virtual ~UIScroll();

		virtual void OnEventPoint();
		virtual void OnDraw();

		__event void OnScroll();
	};


	class UICheck : 
		public UIComponent
	{
	private:
	public:
		bool IsChecked;
		float ProgressCircle;

		inline UICheck()
		{
			IsChecked = false;
			ProgressCircle = 0;
		}
		virtual ~UICheck();

		virtual void OnEventPoint();

		virtual void OnUpdate(double ElapsedTime);
		virtual void OnDraw();

		__event void OnCheck();
	};

	class UIButton : 
		public UIComponent
	{
		PROPERTY_PROVIDE(UIButton);
		DECLARE_PROP_TYPE_R(UIButton, std::wstring, Caption, { return Caption.Value; }, { Caption.Value = Value; } );
		

	public:
		DECLARE_PROPERTY(Caption);
		
	
		inline UIButton(){ }
		virtual ~UIButton();

		virtual void SetCaption(const std::wstring& Caption);

		

		virtual void OnUpdate(double ElapsedTime);
		virtual void OnDraw();

		virtual void OnContentLoad();

	};

	class UILabel :
		public UIComponent
	{
	private:
		PROPERTY_PROVIDE(UILabel);

	public:
		struct MEMBER_PARAM_INIT
		{
			bool IsAutoSize;
			bool IsVCentered;
			bool IsHCentered;
			bool IsNoClip;
		};

	private:
		DECLARE_PROP_TYPE_R(UILabel, std::wstring, Caption, { return Caption.Value; }, { Caption.Value = Value; });


	public:
		DECLARE_PROPERTY(Caption);
	
		MEMBER_PARAM_INIT ParamInit;
		DWORD TextColor;
	
		inline UILabel(wchar_t *Caption)
		{
			TextColor   = 0xffffffff;
			ParamInit.IsAutoSize  = true;
			ParamInit.IsVCentered = false;
			ParamInit.IsHCentered = false;
			ParamInit.IsNoClip    = false;

			RECTANGLE pos = Position;
			pos.Height = (VEC)DeviceResourceManager::DefaultFont.FontDesc.Height;
			Position = pos;

			ApplyText(Caption);
		};
		virtual ~UILabel();

		virtual void ApplyText();
		virtual void ApplyText(const std::wstring& NewCaption);
		virtual void OnDraw();

	};

	class UILabelBackground
		: public UILabel
	{
		virtual void OnDraw();
	};

	class UIEditBox
		: public UIComponent
	{
	public:
		Util::UnwindArray<wchar_t> Content;

	};

	class UIListBox
		: public UIComponent
	{
	public:
		Util::TriList<UIListItem*, true> ListItem; // Use this for only read

		//SmoothizeVal<> Scroll; // 1.0 per item.
		//SmoothizeVal<> ScrollMax; // item count.
		VEC Scroll;
		VEC ScrollMax;
		
		AnimationPool ScrollAnimation;
		AnimationPool ScrollMaxAnimation;

		inline UIListBox()
		{
			ScrollAnimation.BindMember(Scroll);
			ScrollMaxAnimation.BindMember(ScrollMax);
		}

		virtual void OnUpdate(double ElapsedTime);
		virtual void OnDraw();

		bool AddItem(UIListItem *ListItem);
		bool RemoveItem(int Index);
	};
	class UIListItem
		: public UIComponent
	{
	public:
		UIListBox *ListBox;
		Util::UnwindArray<wchar_t> Caption;

		int OrderInList;

		bool IsSelected;

		inline UIListItem(UIListBox *ListBox)
		{
			assert(ListBox);
			this->ListBox = ListBox;
			
			IsSelected = false;
		}

		virtual void OnUpdate(double ElapsedTime);

	};

	class UIDragger
		: public UIComponent
	{
	public:
		enum TYPE_DRAGGER : BYTE
		{
			CenterToOuter = 0,
			LeftToRight,
			RightToLeft,
			TopToBottom,
			BottomToTop,
		} DraggerType;

		inline UIDragger()
		{
			DraggerType = CenterToOuter;
			RECTANGLE pos = Position();
			pos.Width  = 100;
			pos.Height = 100;
			Position = pos;
		}

		virtual void OnDraw();	
	};

}
