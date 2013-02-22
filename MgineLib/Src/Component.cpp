#include "StdAfx.hpp"
#include "Component.h"
#include "Input.h"
#include "Renderer.h"
#include "Function.h"
#include <typeinfo>

using namespace Mgine;
using namespace Util;

decltype(ComponentManager::IsInited) ComponentManager::IsInited;

void ComponentManager::Init()
{
	if ( IsInited ) MTHROW(InvalidStatus, "Already inited");
	MLOG("Begin");
	IsInited = true;
}

void ComponentManager::Uninit()
{
	IsInited = false;
}

void ComponentManager::OnAppFinish()
{
	//if ( iListAutomatedVal ) delete iListAutomatedVal;
	//iListAutomatedVal = NULL;
}




/*\
 *	Component
\*/
UIComponent::~UIComponent()
{
	if ( Status != ENDED ) RaiseEnd();
	if ( StatusContent > UNLOADED ) RaiseContentUnload();
	//ComponentManager::SubComponent(ComponentIndex);
}

void UIComponent::OnContentLoad()
{ }
void UIComponent::OnContentPrepare(bool IsRestoreAll)
{ }
void UIComponent::OnContentUnload(bool *IsUnloaded)
{ }
void UIComponent::OnStart()
{ }
void UIComponent::OnEnd()
{ }
void UIComponent::OnUpdate(double ElapsedTime)
{
	//Progress += Stacked;
}
void UIComponent::OnDraw()
{
	if ( ForeColor() )
	{
		RectBorderDrawer drawer;
		drawer.Padding = 2;
		drawer.Position = ActualPosition;
		drawer.ColorDiffuse = ForeColor;
		drawer.ApplyShape();
		drawer.Draw();
	}
}
void UIComponent::OnFocusGotEvent()
{ }
void UIComponent::OnFocusLostEvent()
{ }
void UIComponent::OnCharEvent( CharEventArg *Arg )
{ }
void UIComponent::OnMouseEventPoint( MouseEventArg *Arg )
{
	POINT pt;
	
	pt.x = Arg->State.X;
	pt.y = Arg->State.Y;

	bool isinrect = false;
	
	if ( !Arg->IsHandled ) isinrect = IsMouseCollision(pt);
	if ( isinrect ) Arg->IsHandled = true;

	if ( !StateEvent().Mouse.IsOn && isinrect )
	{
		StateEvent.Value.Mouse.IsOn = true;
		EventMouseOn(Arg);
	} else if ( StateEvent().Mouse.IsOn && !isinrect )
	{
		StateEvent.Value.Mouse.IsOn = false;
		EventMouseLeave(Arg);
	}

	if ( isinrect ) EventMouseMove(Arg);
	
	if ( StateEvent().Mouse.IsOn )
	{
		if ( !StateEvent().Mouse.IsDown && Input::Mouse[MOUSE_LEFT].Hit() )
		{
			StateEvent.Value.Mouse.IsDown = true;
			StateEvent.Value.Mouse.DragStartPoint.x = Input::Mouse.X;
			StateEvent.Value.Mouse.DragStartPoint.y = Input::Mouse.Y;
			EventMouseDown(Arg);
		}

		if ( StateEvent().Mouse.IsDown )
		{
			StateEvent.Value.Mouse.DragVelocityTo.X = (VEC)Input::Mouse.X - StateEvent().Mouse.DragStartPoint.x;
			StateEvent.Value.Mouse.DragVelocityTo.Y = (VEC)Input::Mouse.Y - StateEvent().Mouse.DragStartPoint.y;
		}
	}

	if ( StateEvent().Mouse.IsDown && Input::Mouse[MOUSE_LEFT].Up() )
	{
		StateEvent.Value.Mouse.IsDown = false;
		EventMouseUp(Arg);
		if ( isinrect ) EventMouseClick(Arg);
	}
}

void UIComponent::OnEventPoint()
{ }
bool UIComponent::IsMouseCollision(POINT MousePosition)
{
	if ( !IsVisible ) return false;
	//CalcInternalPosition();
	RECT rt;

	rt.left = (int)ActualPosition().X;
	rt.top  = (int)ActualPosition().Y;
	rt.right  = (int)ActualPosition().X + (int)ActualPosition().Width;
	rt.bottom = (int)ActualPosition().Y + (int)ActualPosition().Height;

	return PtInRect(&rt, MousePosition) != 0;
}

bool UIComponent::IsFocused()
{
	if ( !Parent ) return true; // Top components are just focused ( and all of the components should be set as child in RootFrame of Screen )
	else if ( Parent()->FocusedChild == this ) return true;
	else return false;
}

void UIComponent::Focus()
{
	if ( !Parent )
	{
		//ComponentManager::SetFocus(ComponentIndex);
	} else
	{
		if ( Parent->FocusedChild == this ) return; // no change
		Parent->Focus();
		if ( Parent->FocusedChild ) Parent->FocusedChild->RaiseFocusLostEvent();
		Parent->FocusedChild = this;
		Parent->FocusedChild->RaiseFocusGotEvent();
	}
}

void UIComponent::SetParent(UIComponent *NewParent)
{
	if ( this->Parent ) this->Parent->RemoveChildComponent(this);
	this->Parent = NewParent;
	if ( this->Parent )
	{
		this->Parent->AddChildComponent(this);
		this->Parent->PlaceChildComponent();
		
		//UINT indexinparent = IndexInParent;
		//ComponentPlacer * pplacer = NULL;
		//if ( Parent->ListPlacer().size() > IndexInParent ) pplacer = &Parent->ListPlacer()[indexinparent];
		Position = RECTANGLE((VEC)LayoutOffset().Margin.left, (VEC)LayoutOffset().Margin.top, Position().Width, Position().Height);
	} // else is Parent removing
}

VECTOR2 UIComponent::GetAdjustPositionToScreen(VEC X, VEC Y)
{
	CalcActualPosition();

	VECTOR2 ret;
	ret.X = X + ActualPosition().X;
	ret.Y = Y + ActualPosition().Y;
	return ret;
}

VECTOR2 UIComponent::GetAdjustPositionToMe(VEC X, VEC Y)
{
	CalcActualPosition();

	VECTOR2 ret;
	ret.X = X - ActualPosition().X;
	ret.Y = Y - ActualPosition().Y;

	return ret;
}

void UIComponent::CalcActualPosition()
{
	//MGINE_RECT oldipos = ActualPosition;
	ActualPosition = GetActualPosition();
}

RECTANGLE UIComponent::GetActualPosition()
{
	RECTANGLE ret;
	RECTANGLE posparent;
	if ( Parent )
	{
		posparent = Parent()->GetActualPosition();
		ret.X += posparent.X;
		ret.Y += posparent.Y;
	}

	ret.X += Position().X + LayoutOffset().Padding.left;
	ret.Y += Position().Y + LayoutOffset().Padding.top;
	ret.Width = Position().Width - LayoutOffset().Padding.left - LayoutOffset().Padding.right;
	ret.Height = Position().Height - LayoutOffset().Padding.top - LayoutOffset().Padding.bottom;
	//ret.X -= Origin().x;
	//ret.Y -= Origin().y;
	return ret;
}

void UIComponent::RaiseChildContentLoad()
{
	try
	{
		for ( auto i=ArrComponent().begin(); i != ArrComponent().end(); i++ )
			if ( (*i)->StatusContent == UNLOADED ) (*i)->RaiseContentLoad();
		//if ( ((UIComponent*)BaseComponent)->ComponentType == UIComponent::Frame ) ArrComponent[i]->SetParent((UIFrame*)BaseComponent);
	}
	catch ( BaseException & )
	{
		MLOG("Failed", );
		throw;
	}
}
void UIComponent::RaiseContentLoad()
{
	if ( StatusContent != UNLOADED ) MTHROW(InvalidStatus, "Already inited");
	RaiseChildContentLoad();
	OnContentLoad();
	StatusContent = LOADED_PREPARING;
}

void UIComponent::RaiseChildContentPrepare(bool IsRestoreAll)
{
	try
	{
		for ( auto i=ArrComponent().begin(); i != ArrComponent().end(); i++ )
			if ( (*i)->StatusContent >= LOADED_PREPARING ) (*i)->RaiseContentPrepare(IsRestoreAll);
	}
	catch ( BaseException & )
	{
		MLOG("Failed");
		throw;
	}
}
void UIComponent::RaiseContentPrepare(bool IsRestoreAll)
{
	ATHROW(StatusContent >= LOADED_PREPARING);
	RaiseChildContentPrepare(IsRestoreAll);
	OnContentPrepare(IsRestoreAll);
	StatusContent = LOADED_PREPARED;
}

void UIComponent::RaiseChildContentUnload()
{
	for ( auto i=ArrComponent().begin(); i != ArrComponent().end(); i++ )
		try
		{ if ( (*i)->StatusContent != UNLOADED ) (*i)->RaiseContentUnload(); }
		catch ( BaseException & )
		{ MLOG("Failed uninit of child"); }
}
void UIComponent::RaiseContentUnload()
{
	if ( StatusContent == UNLOADED ) MTHROW(InvalidStatus, "Already uninited");
	bool isuninited = false;
	OnContentUnload(&isuninited);
	if ( isuninited )
	{
		RaiseChildContentUnload();
		StatusContent = UNLOADED;
	}
}

void UIComponent::RaiseChildStart()
{
	try
	{
		for ( auto i=ArrComponent().begin(); i != ArrComponent().end(); i++ )
			(*i)->RaiseStart();
	}
	catch ( BaseException & )
	{
		MLOG("Failed");
		throw;
	}
}
void UIComponent::RaiseStart()
{
	if ( Status == STARTED ) MTHROW(InvalidStatus, "Already started");
	if ( StatusContent == UNLOADED ) RaiseContentLoad();

	RaiseChildStart();
	OnStart();

	EventStart();
	Status = STARTED;
}

void UIComponent::RaiseChildEnd()
{
	try
	{
		for ( auto i=ArrComponent().begin(); i != ArrComponent().end(); i++ )
			(*i)->RaiseEnd();
	}
	catch ( BaseException & )
	{
		MLOG("Failed");
		throw;
	}
}
void UIComponent::RaiseEnd()
{
	if ( Status == ENDED ) MTHROW(InvalidStatus, "Already ended");
	
	Status = ENDED;
	EventEnd();
	OnEnd();
	RaiseChildEnd();
	if ( StatusContent != UNLOADED ) RaiseContentUnload();
}

void UIComponent::RaiseDraw()
{
	if ( !Status ) MTHROW(InvalidStatus, "Not Inited");
	if ( StatusContent == LOADED_PREPARED && IsVisible )
	{
		OnDraw();
		RaiseDrawChild();

		EventDraw();
	} else MTHROW(InvalidStatus, "Not Started");
}
void UIComponent::RaiseDrawChild()
{
	ATHROW(Status);
	ATHROW(StatusContent == LOADED_PREPARED);

	OnDrawChildren();
}

void UIComponent::RaiseUpdate( double ElapsedTime )
{
	ATHROW(Status);
	ATHROW(StatusContent == LOADED_PREPARED);
	
	AnimationProvider.Update(ElapsedTime);
	OnUpdate(ElapsedTime);
	RaiseUpdateChild(ElapsedTime);
	OnEventPoint();

	EventUpdate();
}
void UIComponent::RaiseUpdateChild(double ElapsedTime)
{
	ATHROW(Status);
	ATHROW(StatusContent == LOADED_PREPARED);

	for ( auto i=ArrComponent().begin(); i != ArrComponent().end(); i++ )
		if ( (*i)->Status == STARTED && (*i)->StatusContent == LOADED_PREPARED ) (*i)->RaiseUpdate(ElapsedTime);
}

void UIComponent::RaiseChildCalcActualPosition()
{
	ATHROW(Status);
	ATHROW(StatusContent == LOADED_PREPARED);
	for ( auto i=ArrComponent().begin(); i != ArrComponent().end(); i++ )
		(*i)->CalcActualPosition();
}

void UIComponent::RaiseFocusGotEvent()
{
	ATHROW(Status);
	ATHROW(StatusContent == LOADED_PREPARED);

	OnFocusGotEvent();
	EventFocusGot();
}

void UIComponent::RaiseFocusLostEvent()
{
	ATHROW(Status);
	ATHROW(StatusContent == LOADED_PREPARED);

	OnFocusLostEvent();
	EventFocusLost();
}

void UIComponent::RaiseCharEvent( CharEventArg *Arg )
{
	ATHROW(Status);
	ATHROW(StatusContent == LOADED_PREPARED);
	ATHROW(Arg != NULL);

	UIComponent *component = NULL;
	if ( FocusedChild )
	{
		FocusedChild->RaiseCharEvent(Arg);
	}
	if ( !Arg->IsHandled ) OnCharEvent(Arg);
	EventChar(Arg);
}

void UIComponent::RaiseMouseEventPoint( MouseEventArg *Arg )
{
	if ( !Status ) MTHROW(InvalidStatus, "Not Inited");
	else if ( !StatusContent ) MTHROW(InvalidStatus, "Not Loaded");
	else if ( !Arg ) MTHROW(InvalidParameter, "Arg is null");

	PrevStateEvent.Value.Mouse = StateEvent().Mouse;
	for ( auto i=ArrComponent().rbegin(); i != ArrComponent().rend(); i++ )
		(*i)->RaiseMouseEventPoint(Arg);

	if ( !Arg->IsHandled ) OnMouseEventPoint(Arg);
}

void UIComponent::OnDrawChildren()
{
	if ( ArrComponent().size() )
	{
		if ( IsUsingStencil )
		{
			QuadDrawer drawer;
			Renderer::BeginStencilSet(true);
			drawer.Init(Position().Width, Position().Height, 0);
			drawer.RectDest = ActualPosition;
			drawer.Draw();
		}

		if ( IsUsingStencil ) Renderer::BeginStencilRender();
		for ( auto i=ArrComponent().begin(); i != ArrComponent().end(); i++ )
			(*i)->RaiseDraw();

		if ( IsUsingStencil ) Renderer::EndStencil();
	}
}

void UIComponent::AddChildComponent(UIComponent *Component)
{
	if ( !Component ) MTHROW(InvalidParameter, "Component is null");
	if ( Component->Parent != this )
		MTHROW(InvalidOperation, "Child's parent is not matched. Don't call this method directly and use SetParent() method instead.");

	try
	{
		if ( Status == STARTED && Component->Status != STARTED ) Component->RaiseStart();
		else if ( Status == ENDED && Component->Status != ENDED ) Component->RaiseEnd();
		switch ( StatusContent )
		{
		case UNLOADED:
			if ( Component->StatusContent < Component->StatusContent ) Component->RaiseContentUnload();
			break;
		case LOADED_PREPARING:
			if ( Component->StatusContent < StatusContent )
			{
				Component->RaiseContentLoad();
			} else if ( StatusContent < Component->StatusContent )
			{
				// there's no way and need to un'prepare'
			}
			break;
		case LOADED_PREPARED:
			if ( Component->StatusContent == UNLOADED )
				Component->RaiseContentLoad();
			if ( Component->StatusContent == LOADED_PREPARING )
				Component->RaiseContentPrepare(true);
			break;
		default: NRTHROW();
		}

		ArrComponent.Value.push_back(Component);
	}
	catch ( BaseException & )
	{
		MLOG("Failed");
		throw;
	}

}

void UIComponent::RemoveChildComponent(UIComponent *Component)
{
	//ATHROW(Parent());
	for ( auto i=ArrComponent().begin(); i != ArrComponent().end(); i++ )
		if ( *i == Component )
		{
			DATHROW(Component->Parent == this);
			if ( Component->Status == UIComponent::STARTED ) Component->RaiseEnd();
			if ( Component->StatusContent >= UIComponent::LOADED_PREPARING ) Component->RaiseContentUnload();
			ArrComponent.Value.erase(i);
			PlaceChildComponent();

			return;
		}

	MTHROW(InvalidOperation, "component not found!",  );


}

// void UIComponent::SetChildPositioningType( CHILD_POSITIONING_TYPE Type )
// {
// 	ChildPositioningType = Type;
// 	//RaiseUpdateChildPosition();
// 	PlaceChildComponent();
// }

UINT UIComponent::AddRawPlacer(ComponentPlacer *PlacerManaged)
{
	ComponentPlacer *prcd = PlacerManaged; //new Placer(MinWidth, MaxWidth, HorizontalAlign, VerticalAlign);
	ListPlacer.Value.push_back(prcd);
	UINT ret = ListPlacer().size()-1;
	prcd->SetOwner(this, ret);
	UpdatePlacerPosition();
	return ret;
}

void UIComponent::SetRawPlacer( int PlacerIndex, ComponentPlacer *PlacerManaged )
{
	ListPlacer.Value.c_array()[PlacerIndex] = PlacerManaged;
	//&ListPlacer.Value[PlacerIndex] = PlacerManaged; //MTHROW(InvalidOperation, "LRCD set failed.");
	PlacerManaged->SetOwner(this, PlacerIndex);
	PlaceChildComponent(PlacerIndex);
}

void UIComponent::PlacerPositionChanged(ComponentPlacer *Placer)
{
	UpdatePlacerPosition();
}

void UIComponent::UpdatePlacerPosition()
{
	for ( auto i=ListPlacer.Value.begin(); i != ListPlacer.Value.end(); i++ )
		i->UpdatePlacerPosition();
}

void UIComponent::PlaceChildComponent( int ChildIndex )
{
	//UIComponent *& compo = ArrComponent().Get(ChildIndex);
	//if ( !&compo ) MTHROW(InvalidOperation, "Unable to get child %d", ChildIndex);
	//compo->PlaceComponent();
	ArrComponent()[ChildIndex]->PlaceComponent();
}

void UIComponent::PlaceChildComponent()
{
	for ( auto i=ArrComponent().begin(); i != ArrComponent().end(); i++ )
		(*i)->PlaceComponent();
}


void Mgine::UIComponent::PlaceComponent()
{
	if ( !Parent() ) return;

	if ( IndexInParent < Parent->ListPlacer().size() )
	{
		//ComponentPlacer const *& rcd = const_cast<ComponentPlacer const *&>(Parent->ListPlacer().Get(IndexInParent));
		const ComponentPlacer * rcd = &Parent->ListPlacer()[IndexInParent];
		PlaceComponent(rcd);
	} else CalcActualPosition();
}
void UIComponent::PlaceComponent(const ComponentPlacer *Placer)
{
	Position = Placer->GetComponentPosition(this);
	//CalcActualPosition();
}

void UIComponent::__SetPosition( RECTANGLE const & Value )
{
	Position.Value = Value;

	if ( Parent() )
	{
		if ( IndexInParent < Parent->ListPlacer().size() )
		{
			ComponentPlacer & def = Parent->ListPlacer.Value[IndexInParent];
			Position.Value = def.ComponentPositionChanged(this);
		}

		/*VEC minwidth, maxwidth, minheight, maxheight;
		switch ( Parent()->ChildPositioningType )
		{
		case UIComponent::HORIZONTAL:
			minwidth = def->MinSize;
			maxwidth = def->MaxSize;
			minheight = 0;
			maxheight = this->Parent()->ActualPosition().Height;
			break;
		case UIComponent::VERTICAL:
			minwidth = 0;
			maxwidth = this->Parent()->ActualPosition().Width;
			minheight = def->MinSize;
			maxheight = def->MaxSize;
			break;
		default:
			MTHROW(InvalidOperation, "Invalid Positioning Type");
		}

		if ( maxwidth && maxwidth
			< Position.Value.Width
				+ LayoutOffset().Padding.left + LayoutOffset().Padding.right
			)
			Position.Value.Width
				= maxwidth - (LayoutOffset().Padding.left + LayoutOffset().Padding.right);
		if ( maxheight && maxheight
			< Position.Value.Height
				+ LayoutOffset().Padding.top + LayoutOffset().Padding.bottom
			)
			Position.Value.Height = maxheight - (LayoutOffset().Padding.top + LayoutOffset().Padding.bottom);
		if ( minwidth && minwidth > Position.Value.Width )
			Position.Value.Width = minwidth;
		if ( minheight && minheight > Position.Value.Height )
			Position.Value.Height = minheight;
		*/
	}
	CalcActualPosition();

	//Position.Value = Value;
	//CalcActualPosition();
}

void UIComponent::__SetActualPosition( RECTANGLE const & Value )
{
	if ( ActualPosition() != Value )
	{
		RECTANGLE oldactualpos = ActualPosition.Value;
		ActualPosition.Value = Value;

		//if ( Parent() ) Parent()->RaiseUpdateChildPosition();
		//RaiseUpdateChildPosition();

		UpdatePlacerPosition();
		PlaceChildComponent();
		//RaiseChildCalcActualPosition();
		
		PositionEventArg arg(oldactualpos);
		EventActualPositionChanged(&arg);
	}
}





/*\
 *	Scroll Object
\*/
UIScroll::~UIScroll(){ }

void UIScroll::OnEventPoint()
{
	UIComponent::OnEventPoint();

	if ( !PrevStateEvent().Mouse.IsDown && StateEvent().Mouse.IsDown )
	{
		// Hit
		IsScrolling = true;
	} else if ( IsScrolling )
	{
		Value = (Input::Mouse.X - Position().X)/(Position().Width);

		if ( Value < 0 ) Value = 0;
		if ( Value > 1 ) Value = 1;
		OnScroll();

		if ( !StateEvent().Mouse.IsDown ){
			IsScrolling = false;
			/*if ( IsMouseOn )
			{
				OnScroll();
			}*/
		}

	}
	

}

void UIScroll::OnDraw()
{
	if ( !IsVisible ) return;
	if ( !StatusContent ) return;

	//CalcInternalPosition();

	VERTEX_LINE lineCenter[4];
	lineCenter[0].V1.X = ActualPosition().X;
	lineCenter[0].V2.X = ActualPosition().X + ActualPosition().Width;
	lineCenter[0].V1.Y = ActualPosition().Y + ActualPosition().Height;
	lineCenter[0].V2.Y = ActualPosition().Y + ActualPosition().Height;
	lineCenter[0].V1.ColorDiffuse = D3DCOLOR_ARGB(150, 200, 200, 200);
	lineCenter[0].V2.ColorDiffuse = D3DCOLOR_ARGB(150, 200, 200, 200);

	int i;
	for ( i=1; i < 4; i++ )
	{
		lineCenter[i].V1.X = ActualPosition().X;
		lineCenter[i].V2.X = ActualPosition().X + ActualPosition().Width;
		lineCenter[i].V1.Y = ActualPosition().Y + ActualPosition().Height + i;
		lineCenter[i].V2.Y = ActualPosition().Y + ActualPosition().Height + i;
		lineCenter[i].V1.ColorDiffuse = D3DCOLOR_ARGB(100+i*i*4, 100, 100, 100);
		lineCenter[i].V2.ColorDiffuse = D3DCOLOR_ARGB((int)(70+i*i*7.5f), 70, 70, 70);
	}

	Renderer::DrawLine(lineCenter, 4);

	VERTEX_LINE lineBar[4];
	lineBar[0].V1.X = ActualPosition().X+(ActualPosition().Width-4)*Value;
	lineBar[0].V2.X = ActualPosition().X+(ActualPosition().Width-4)*Value;
	lineBar[0].V1.Y = ActualPosition().Y;
	lineBar[0].V2.Y = ActualPosition().Y + ActualPosition().Height;
	lineBar[0].V1.ColorDiffuse = D3DCOLOR_ARGB(150, 200, 200, 200);
	lineBar[0].V2.ColorDiffuse = D3DCOLOR_ARGB(150, 200, 200, 200);

	if ( StateEvent().Mouse.IsDown )
	{
		lineBar[0].V1.ColorDiffuse.R -= 0x40;
		lineBar[0].V1.ColorDiffuse.G -= 0x40;
		lineBar[0].V1.ColorDiffuse.B -= 0x40;
		lineBar[0].V2.ColorDiffuse.R -= 0x40;
		lineBar[0].V2.ColorDiffuse.G -= 0x40;
		lineBar[0].V2.ColorDiffuse.B -= 0x40;
	} else if ( StateEvent().Mouse.IsOn )
	{
		lineBar[0].V1.ColorDiffuse.R -= 0x20;
		lineBar[0].V1.ColorDiffuse.G -= 0x20;
		lineBar[0].V1.ColorDiffuse.B -= 0x20;
		lineBar[0].V2.ColorDiffuse.R -= 0x20;
		lineBar[0].V2.ColorDiffuse.G -= 0x20;
		lineBar[0].V2.ColorDiffuse.B -= 0x20;
	}

	for ( i=1; i < 4; i++ )
	{
		lineBar[i].V1.X = ActualPosition().X+(ActualPosition().Width-4)*Value+i;
		lineBar[i].V2.X = ActualPosition().X+(ActualPosition().Width-4)*Value+i;
		lineBar[i].V1.Y = ActualPosition().Y;
		lineBar[i].V2.Y = ActualPosition().Y + ActualPosition().Height;

		lineBar[i].V1.ColorDiffuse = D3DCOLOR_ARGB(i*i*7+100, 100, 100, 100);
		lineBar[i].V2.ColorDiffuse = D3DCOLOR_ARGB(i*i*3+100, 200, 200, 200);

		if ( StateEvent().Mouse.IsDown )
		{
			lineBar[i].V1.ColorDiffuse.R -= 0x40;
			lineBar[i].V1.ColorDiffuse.G -= 0x40;
			lineBar[i].V1.ColorDiffuse.B -= 0x40;
			lineBar[i].V2.ColorDiffuse.R -= 0x40;
			lineBar[i].V2.ColorDiffuse.G -= 0x40;
			lineBar[i].V2.ColorDiffuse.B -= 0x40;
		} else if ( StateEvent().Mouse.IsOn )
		{
			lineBar[i].V1.ColorDiffuse.R -= 0x20;
			lineBar[i].V1.ColorDiffuse.G -= 0x20;
			lineBar[i].V1.ColorDiffuse.B -= 0x20;
			lineBar[i].V2.ColorDiffuse.R -= 0x20;
			lineBar[i].V2.ColorDiffuse.G -= 0x20;
			lineBar[i].V2.ColorDiffuse.B -= 0x20;
		}
	}
	Renderer::DrawLine(lineBar, 4);
}

/*\
 *	Check Object
\*/
UICheck::~UICheck(){ } 
void UICheck::OnEventPoint()
{
	UIComponent::OnEventPoint();

	if ( PrevStateEvent().Mouse.IsDown && !StateEvent().Mouse.IsDown && StateEvent().Mouse.IsOn )
	{
		IsChecked = !IsChecked;
		OnCheck();
	}
}

void UICheck::OnDraw()
{
	if ( !IsVisible ) return ;
	if ( !StatusContent ) return;

	int degreeMouse = 0;
	if ( StateEvent().Mouse.IsDown )
		degreeMouse = 0x10;
	else if ( StateEvent().Mouse.IsOn )
		degreeMouse = -0x20;

	//CalcInternalPosition();

	VERTEX_LINE lineBorder[8];
	lineBorder[0].V1.X = ActualPosition().X;
	lineBorder[0].V1.Y = ActualPosition().Y;
	lineBorder[0].V2.X = ActualPosition().X + ActualPosition().Width;
	lineBorder[0].V2.Y = ActualPosition().Y;

	lineBorder[1].V1 = lineBorder[0].V2;
	lineBorder[1].V2.X = ActualPosition().X + ActualPosition().Width;
	lineBorder[1].V2.Y = ActualPosition().Y + ActualPosition().Height;

	lineBorder[2].V1 = lineBorder[1].V2;
	lineBorder[2].V2.X = ActualPosition().X;
	lineBorder[2].V2.Y = ActualPosition().Y + ActualPosition().Height;

	lineBorder[3].V1 = lineBorder[2].V2;
	lineBorder[3].V2.X = ActualPosition().X;
	lineBorder[3].V2.Y = ActualPosition().Y;

	lineBorder[4] = lineBorder[0];
	lineBorder[5] = lineBorder[1];
	lineBorder[6] = lineBorder[2];
	lineBorder[7] = lineBorder[3];

	lineBorder[4].V1.Y ++;
	lineBorder[4].V2.Y ++;
	lineBorder[5].V1.X --;
	lineBorder[5].V2.X --;
	lineBorder[6].V1.Y --;
	lineBorder[6].V2.Y --;
	lineBorder[7].V1.X ++;
	lineBorder[7].V2.X ++;

	lineBorder[0].V1.ColorDiffuse = D3DCOLOR_ARGB(150, 200-degreeMouse, 200-degreeMouse, 200-degreeMouse);
	lineBorder[0].V2.ColorDiffuse = D3DCOLOR_ARGB(150, 90-degreeMouse, 90-degreeMouse, 90-degreeMouse);
	lineBorder[1].V1.ColorDiffuse = lineBorder[0].V2.ColorDiffuse;
	lineBorder[1].V2.ColorDiffuse = D3DCOLOR_ARGB(150, 100-degreeMouse, 100-degreeMouse, 100-degreeMouse);
	lineBorder[2].V1.ColorDiffuse = lineBorder[1].V2.ColorDiffuse;
	lineBorder[2].V2.ColorDiffuse = D3DCOLOR_ARGB(150, 170-degreeMouse, 170-degreeMouse, 170-degreeMouse);
	lineBorder[3].V1.ColorDiffuse = lineBorder[2].V2.ColorDiffuse;
	lineBorder[3].V2.ColorDiffuse = lineBorder[0].V1.ColorDiffuse;

	lineBorder[4].V1.ColorDiffuse = D3DCOLOR_ARGB(120, 50-degreeMouse, 50-degreeMouse, 50-degreeMouse);
	lineBorder[4].V2.ColorDiffuse = D3DCOLOR_ARGB(120, 30-degreeMouse, 30-degreeMouse, 30-degreeMouse);
	lineBorder[5].V1.ColorDiffuse = lineBorder[4].V2.ColorDiffuse;
	lineBorder[5].V2.ColorDiffuse = D3DCOLOR_ARGB(120, 70-degreeMouse, 70-degreeMouse, 70-degreeMouse);
	lineBorder[6].V1.ColorDiffuse = lineBorder[5].V2.ColorDiffuse;
	lineBorder[6].V2.ColorDiffuse = D3DCOLOR_ARGB(120, 40-degreeMouse, 40-degreeMouse, 40-degreeMouse);
	lineBorder[7].V1.ColorDiffuse = lineBorder[6].V2.ColorDiffuse;
	lineBorder[7].V2.ColorDiffuse = lineBorder[4].V1.ColorDiffuse;


	
	Renderer::DrawLine(lineBorder, 8);

	if ( IsChecked )
	{
		int degreeCheckColor = 0x60;
		if ( StateEvent().Mouse.IsDown )
			degreeCheckColor = 0x10;
		else if ( StateEvent().Mouse.IsOn ) 
			degreeCheckColor = 0x35;

		VERTEX_LINE lineCheck[10];
		int i;
		for ( i=0; i < 5; i++ )
		{
			int colora = i + degreeCheckColor;
			int colorb = 30-i + degreeCheckColor;
			lineCheck[i].V1.X = ActualPosition().X+ActualPosition().Width/2  + cosf(i*2*MF::PI_F/5+ProgressCircle*2*MF::PI_F)*(ActualPosition().Width/2-1);
			lineCheck[i].V1.Y = ActualPosition().Y+ActualPosition().Height/2 + sinf(i*2*MF::PI_F/5+ProgressCircle*2*MF::PI_F)*(ActualPosition().Height/2-1);
			lineCheck[i].V2.X = ActualPosition().X+ActualPosition().Width/2  + cosf(i*2*MF::PI_F/5+1.5f+ProgressCircle*2*MF::PI_F)*(ActualPosition().Width/2-1);
			lineCheck[i].V2.Y = ActualPosition().Y+ActualPosition().Height/2 + sinf(i*2*MF::PI_F/5+1.5f+ProgressCircle*2*MF::PI_F)*(ActualPosition().Height/2-1);
			
			lineCheck[i].V1.ColorDiffuse = D3DCOLOR_ARGB(120, 0x30+colora, 0x30+colora, 0x30+colora);
			lineCheck[i].V2.ColorDiffuse = D3DCOLOR_ARGB(120, 0x20+colorb, 0x20+colorb, 0x20+colorb);
		}

		for ( ; i < 10; i++ )
		{
			int colora = 30+i+degreeCheckColor;
			int colorb = 20+i*2+degreeCheckColor;
			lineCheck[i].V1.X = ActualPosition().X+ActualPosition().Width/2  + cosf(i*2*MF::PI_F/5+MF::PI_F-ProgressCircle*2*MF::PI_F)*(ActualPosition().Width/2-4);
			lineCheck[i].V1.Y = ActualPosition().Y+ActualPosition().Height/2 + sinf(i*2*MF::PI_F/5+MF::PI_F-ProgressCircle*2*MF::PI_F)*(ActualPosition().Height/2-4);
			lineCheck[i].V2.X = ActualPosition().X+ActualPosition().Width/2  + cosf(i*2*MF::PI_F/5+1.5f+MF::PI_F-ProgressCircle*2*MF::PI_F)*(ActualPosition().Width/2-4);
			lineCheck[i].V2.Y = ActualPosition().Y+ActualPosition().Height/2 + sinf(i*2*MF::PI_F/5+1.5f+MF::PI_F-ProgressCircle*2*MF::PI_F)*(ActualPosition().Height/2-4);

			lineCheck[i].V1.ColorDiffuse = D3DCOLOR_ARGB(180, colora, colora, colora);
			lineCheck[i].V2.ColorDiffuse = D3DCOLOR_ARGB(180, colorb, colorb, colorb);
		}

		Renderer::DrawLine(lineCheck, 10);
	}

}

void UICheck::OnUpdate(double ElapsedTime)
{
	UIComponent::OnUpdate(ElapsedTime);
	ProgressCircle += 0.01f;
	if ( ProgressCircle > 1 ) ProgressCircle -= 1;
	return ;
}

/*\
 *	Button Object
\*/

UIButton::~UIButton()
{ }

void UIButton::SetCaption( const std::wstring& NewCaption )
{
	this->Caption = NewCaption;
	// Text
	SIZE size = DeviceResourceManager::DefaultFont.Draw(
		this->Caption,
		(int)ActualPosition().X, (int)ActualPosition().Y,
		(int)ActualPosition().Width, (int)ActualPosition().Height,
		0,
		DT_CENTER | DT_VCENTER | DT_CALCRECT, false);

	size.cx += 6;
	size.cy += 6;

	if ( ActualPosition().Width < size.cx )
	{
		RECTANGLE pos = Position();
		pos.Width += size.cx - ActualPosition().Width;
		Position = pos;
	}
	if ( ActualPosition().Height < size.cy )
	{
		RECTANGLE pos = Position();
		pos.Height += size.cy - ActualPosition().Height;
		Position = pos;
	}
}

void UIButton::OnContentLoad()
{
	SetCaption(L"Button");
}

void UIButton::OnUpdate(double ElapsedTime)
{ UIComponent::OnUpdate(ElapsedTime); }

void UIButton::OnDraw()
{
	if ( !IsVisible ) return;
	if ( !StatusContent ) return;

	// Border
	QuadDrawer lineBorder[4];
	lineBorder[0].Init(0, 0, NULL);
	lineBorder[1].Init(0, 0, NULL);
	lineBorder[2].Init(0, 0, NULL);
	lineBorder[3].Init(0, 0, NULL);

	lineBorder[0].RectDest.X = ActualPosition().X;
	lineBorder[0].RectDest.Y = ActualPosition().Y;
	lineBorder[0].RectDest.Width = ActualPosition().Width-3;
	lineBorder[0].RectDest.Height = 3;

	lineBorder[1].RectDest.X = ActualPosition().X + ActualPosition().Width-3;
	lineBorder[1].RectDest.Y = ActualPosition().Y;
	lineBorder[1].RectDest.Width = 3;
	lineBorder[1].RectDest.Height = ActualPosition().Height-3;

	lineBorder[2].RectDest.X = ActualPosition().X + ActualPosition().Width;
	lineBorder[2].RectDest.Y = ActualPosition().Y + ActualPosition().Height-3;
	lineBorder[2].RectDest.Width  = -(ActualPosition().Width-3);
	lineBorder[2].RectDest.Height = 3;

	lineBorder[3].RectDest.X = ActualPosition().X;
	lineBorder[3].RectDest.Y = ActualPosition().Y + ActualPosition().Height;
	lineBorder[3].RectDest.Width  = 3;
	lineBorder[3].RectDest.Height = -(ActualPosition().Height-3);


	int color = 0xd0;
	if ( StateEvent().Mouse.IsDown ) color -= 0x40;
	else if ( StateEvent().Mouse.IsOn ) color += 0x20;

	lineBorder[0].ColorDiffuse = D3DCOLOR_ARGB(255, color, color, color);
	lineBorder[0].ColorDiffuse = D3DCOLOR_ARGB(255, color-20, color-20, color-20);
	lineBorder[1].ColorDiffuse = lineBorder[0].ColorDiffuse;
	lineBorder[1].ColorDiffuse = D3DCOLOR_ARGB(255, color-40, color-40, color-40);
	lineBorder[2].ColorDiffuse = lineBorder[1].ColorDiffuse;
	lineBorder[2].ColorDiffuse = D3DCOLOR_ARGB(255, color+10, color+10, color+10);
	lineBorder[3].ColorDiffuse = lineBorder[2].ColorDiffuse;
	lineBorder[3].ColorDiffuse = lineBorder[0].ColorDiffuse;
	
	lineBorder[0].ApplyColorDiffuse();
	lineBorder[1].ApplyColorDiffuse();
	lineBorder[2].ApplyColorDiffuse();
	lineBorder[3].ApplyColorDiffuse();

	lineBorder[0].Draw();
	lineBorder[1].Draw();
	lineBorder[2].Draw();
	lineBorder[3].Draw();

	// Text
	DeviceResourceManager::DefaultFont.Draw(
		Caption(),
		(int)ActualPosition().X, (int)ActualPosition().Y,
		(int)ActualPosition().Width, (int)ActualPosition().Height,
		D3DCOLOR_ARGB(color, color, color, color),
		DT_CENTER | DT_VCENTER, false);

}

/*\
 *	Label Object
\*/
UILabel::~UILabel()
{ }

void UILabel::ApplyText()
{
	if ( ParamInit.IsAutoSize )
	{
		ParamInit.IsVCentered = false;
		ParamInit.IsHCentered = false;

		/*
		HDC hdc = MgineFont::DefaultFont.gFont->GetDC();
		SIZE size;
		size.cy = size.cx = 0;

		wchar_t *strproc  = Caption.Arr;
		wchar_t *lastproc = Caption.Arr;

		for ( ; ; strproc++ )
		{
			if ( *strproc == L'\n' )
			{
				SIZE tempsize;
				tempsize.cy = tempsize.cx = 0;
				GetTextExtentPoint(hdc, lastproc, strproc-lastproc, &tempsize);
				if ( !tempsize.cy ) tempsize.cy = MgineFont::DefaultFont.FontDesc.Height;
				if ( tempsize.cx > size.cx ) size.cx = tempsize.cx;
				size.cy += tempsize.cy;

				lastproc = strproc+1;
			} else if ( *strproc == 0 )
			{
				SIZE tempsize;
				tempsize.cy = tempsize.cx = 0;
				GetTextExtentPoint(hdc, lastproc, strproc-lastproc, &tempsize);
				if ( !tempsize.cy ) tempsize.cy = MgineFont::DefaultFont.FontDesc.Height;

				if ( tempsize.cx > size.cx ) size.cx = tempsize.cx;
				size.cy += tempsize.cy;

				lastproc = strproc;
				break;
			}
		}*/

		RECT drawrect = DeviceResourceManager::DefaultFont.GetTextDrawRect(Caption, 0, 0, 0, 0, 0, DT_CALCRECT | DT_NOCLIP | DT_LEFT);

		RECTANGLE pos = RECTANGLE( (VEC)Position().X, (VEC)Position().Y, (VEC)drawrect.right - drawrect.left, (VEC)drawrect.bottom - drawrect.top );
		Position = pos;
	}

}

void UILabel::ApplyText(const std::wstring& NewCaption)
{
	Caption = NewCaption;
	ApplyText();
}

void UILabel::OnDraw()
{
	if ( !IsVisible ) return;
	if ( !StatusContent ) return;

	//CalcInternalPosition();

	if ( ParamInit.IsAutoSize )
	{
		DeviceResourceManager::DefaultFont.Draw(Caption,
			(int)ActualPosition().X, (int)ActualPosition().Y, (int)ActualPosition().Width, (int)ActualPosition().Height,
			TextColor,
			NULL, true);
	}
	else 
	{
		DWORD flag = NULL;
		if ( ParamInit.IsVCentered ) flag |= DT_VCENTER;
		if ( ParamInit.IsHCentered ) flag |= DT_CENTER;
		if ( ParamInit.IsNoClip    ) flag |= DT_NOCLIP;

		DeviceResourceManager::DefaultFont.Draw(Caption,
			(int)ActualPosition().X, (int)ActualPosition().Y, (int)ActualPosition().Width, (int)ActualPosition().Height,
			TextColor,
			flag, true);
	}
}

void UILabelBackground::OnDraw()
{
	if ( !IsVisible || 
		!StatusContent ) return;

	VERTEX_TEXTURE quad;
	quad.V1.X = Position().X;
	quad.V1.Y = Position().Y;
	quad.V2.X = Position().X;
	quad.V2.Y = Position().Y+Position().Height;
	quad.V3.X = Position().X+Position().Width;
	quad.V3.Y = Position().Y;
	quad.V4.X = Position().X+Position().Width;
	quad.V4.Y = Position().Y+Position().Height;

	quad.V1.ColorDiffuse = ForeColor;
	quad.V2.ColorDiffuse = ForeColor;
	quad.V3.ColorDiffuse = ForeColor;
	quad.V4.ColorDiffuse = ForeColor;

	VERTEX_LINE line[4];
	line[0].V1.X = Position().X;
	line[0].V1.Y = Position().Y;
	line[0].V2.X = Position().X+Position().Width;
	line[0].V2.Y = Position().Y;
	
	line[1].V1 = line[0].V1;
	line[1].V2.X = Position().X+Position().Width;
	line[1].V2.Y = Position().Y+Position().Height;

	line[2].V1 = line[1].V1;
	line[2].V2.X = Position().X+Position().Width;
	line[2].V2.Y = Position().Y+Position().Height;

	line[3].V1 = line[2].V1;
	line[3].V2.X = Position().X+Position().Width;
	line[3].V2.Y = Position().Y+Position().Height;

	Renderer::DrawQuad(&quad, 1);
	UILabel::OnDraw();
}
void UIListBox::OnUpdate( double ElapsedTime )
{

}

void UIListBox::OnDraw()
{
	/*
	int i, count;
	UIListItem *item;

	for (
		ListItem.EnumStepInit(i, count);
		ListItem.EnumStep(i, count, &item);
		)
	{
		item->Draw();
	}*/
	OnDrawChildren();
}

bool UIListBox::AddItem( UIListItem *Item )
{
	if ( !Item ) return false;

	ListItem.Add(Item);

	for ( StepIterator<UIListItem *> item; ListItem.Step(item); )
		item->OrderInList = item.Index;

	return true;
}

bool UIListBox::RemoveItem( int Index )
{
	bool issucceeded = ListItem.Sub(Index);
	if ( issucceeded )
	{
		for ( StepIterator<UIListItem *> item; ListItem.Step(item); )
			item->OrderInList = item.Index;
	}

	return issucceeded;
}

void UIListItem::OnUpdate( double ElapsedTime )
{

}

void UIDragger::OnDraw()
{
	QuadDrawer drawer;
	//CalcInternalPosition();
	
	drawer.Init(ActualPosition().Width, ActualPosition().Height, NULL);
	drawer.RectDest.X = ActualPosition().X;
	drawer.RectDest.Y = ActualPosition().Y;
	drawer.ColorDiffuse = 0xffffffff;
	drawer.ApplyColorDiffuse();
	drawer.Draw();
}












void ComponentPlacer::SetOwner( UIComponent *Owner, int IndexInOwner )
{
	this->Owner = Owner;
	this->IndexInOwner.Value = IndexInOwner;
	ATHROWR(Owner && &Owner->ListPlacer()[IndexInOwner] == this, "RowCol and Index is not matched");
}

RECTANGLE ComponentPlacer::GetComponentPosition(UIComponent const *Component, VEC & SizeAfterSet) const
{
	return RECTANGLE();
}


RECTANGLE ComponentPlacer::ComponentPositionChanged( UIComponent const *Component )
{
	DATHROW(IndexInOwner != -1);
	return RECTANGLE();
}


void ComponentPlacer::UpdatePlacerPosition()
{
	PositionComponent();
}

void ComponentPlacer::PositionComponent()
{
	//UIComponent *& compo = Owner->ArrComponent().Get(IndexInOwner);
	//if ( !&compo ) return; // Element isn't there
	//compo->PlaceComponent(this);
	if ( Owner->ArrComponent().size() <= IndexInOwner ) return;
	Owner->ArrComponent()[IndexInOwner]->PlaceComponent(this);
}

RECTANGLE RowPlacer::GetComponentPosition(UIComponent const *Component, VEC & SizeAfterSet) const
{
	SizeAfterSet = this->SizeWidth;

	RECTANGLE pos;
	VEC left = 0, right = 0,
		top = 0, bottom = 0,
		basex = 0, basey = 0;

	left = 0;
	right = Component->LayoutOffset().Margin.left
		+ Component->Position().Width
		+ Component->LayoutOffset().Margin.right;
	top = 0;
	bottom = Component->LayoutOffset().Margin.top
		+ Component->Position().Height
		+ Component->LayoutOffset().Margin.bottom;

	VEC width = 0, height = 0;
	if ( this->MaxSizeWidth && right > this->MaxSizeWidth )
		right = this->MaxSizeWidth;
	if ( this->MinSizeWidth && right < this->MinSizeWidth )
		right = this->MinSizeWidth;
	if ( SizeAfterSet < right )
		SizeAfterSet = right;
	basex = this->PosX;
	width = SizeAfterSet;
	height = Component->Parent->ActualPosition().Height;
	if ( height < bottom ) bottom = height;


	switch ( this->HorizontalAlign )
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
		MTHROW(InvalidOperation, "Horizontal Align is invalid in this state, %d", this->HorizontalAlign);
		break;
	}

	switch ( this->VerticalAlign )
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
		MTHROW(InvalidOperation, "Vertical Align is invalid in this state, %d", this->HorizontalAlign);
		break;
	}

	
	pos.Width =
		(VEC)(
		right - left
		-Component->LayoutOffset().Margin.left
		-Component->LayoutOffset().Margin.right
		);
	pos.Height =
		(VEC)(
		bottom - top
		-Component->LayoutOffset().Margin.top
		-Component->LayoutOffset().Margin.bottom
		);

	pos.X = basex + left + Component->LayoutOffset().Margin.left;
	pos.Y = basey + top + Component->LayoutOffset().Margin.top;
	return pos;
}

RECTANGLE RowPlacer::ComponentPositionChanged(UIComponent const *Component)
{
	VEC sizeafterset = this->SizeWidth;
	RECTANGLE ret = GetComponentPosition(Component, sizeafterset);
	if ( this->SizeWidth != sizeafterset )
	{
		this->SizeWidth = sizeafterset;
		if ( Owner ) Owner->PlacerPositionChanged(this);
	}
	return ret;
}

void RowPlacer::UpdatePlacerPosition()
{
	DATHROW(IndexInOwner != -1);

	VEC beforepos = this->PosX;

	if ( IndexInOwner != 0 )
	{
		int idx = IndexInOwner-1;
		add_const_ptr<decltype(this)>::type beforercd = NULL;
		 
		while ( 0 <= idx )
		{
			if ( beforercd = dynamic_cast<decltype(beforercd)>
								(&Owner->ListPlacer()[idx])
				)
				break;
			--idx;
		}
		if ( idx == -1 ) this->PosX = 0;
		else this->PosX = beforercd->PosX + beforercd->SizeWidth;
	} else this->PosX = 0;

	PositionComponent();
}

RECTANGLE ColPlacer::GetComponentPosition(UIComponent const *Component, VEC & SizeAfterSet) const
{
	SizeAfterSet = this->SizeHeight;

	RECTANGLE pos;
	VEC left = 0, right = 0,
		top = 0, bottom = 0,
		basex = 0, basey = 0;

	left = 0;
	right = Component->LayoutOffset().Margin.left
		+ Component->Position().Width
		+ Component->LayoutOffset().Margin.right;
	top = 0;
	bottom = Component->LayoutOffset().Margin.top
		+ Component->Position().Height
		+ Component->LayoutOffset().Margin.bottom;

	VEC width = 0, height = 0;
	if ( this->MaxSizeHeight && bottom > this->MaxSizeHeight )
		bottom = this->MaxSizeHeight;
	if ( this->MinSizeHeight && bottom < this->MinSizeHeight )
		bottom = this->MinSizeHeight;
	if ( SizeAfterSet < bottom )
		SizeAfterSet = bottom;
	basey = this->PosY;
	width = Component->Parent->ActualPosition().Width;
	height = SizeAfterSet;
	if ( width < right ) right = width;


	switch ( this->HorizontalAlign )
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
		MTHROW(InvalidOperation, "Horizontal Align is invalid in this state, %d", this->HorizontalAlign);
		break;
	}

	switch ( this->VerticalAlign )
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
		MTHROW(InvalidOperation, "Vertical Align is invalid in this state, %d", this->HorizontalAlign);
		break;
	}

	
	pos.Width =
		(VEC)(
		right - left
		-Component->LayoutOffset().Margin.left
		-Component->LayoutOffset().Margin.right
		);
	pos.Height =
		(VEC)(
		bottom - top
		-Component->LayoutOffset().Margin.top
		-Component->LayoutOffset().Margin.bottom
		);

	pos.X = basex + left + Component->LayoutOffset().Margin.left;
	pos.Y = basey + top + Component->LayoutOffset().Margin.top;
	return pos;
}

RECTANGLE ColPlacer::ComponentPositionChanged(UIComponent const *Component)
{
	VEC sizeafterset = this->SizeHeight;
	RECTANGLE ret = GetComponentPosition(Component, sizeafterset);
	if ( this->SizeHeight != sizeafterset )
	{
		this->SizeHeight = sizeafterset;
		if ( Owner ) Owner->PlacerPositionChanged(this);
	}
	return ret;
}

void ColPlacer::UpdatePlacerPosition()
{
	DATHROW(IndexInOwner != -1);

	VEC beforepos = this->PosY;

	if ( IndexInOwner != 0 )
	{
		int idx = IndexInOwner-1;
		add_const_ptr<decltype(this)>::type beforercd = NULL;

		while ( 0 <= idx )
		{
			if ( beforercd = dynamic_cast<decltype(beforercd)>(&Owner->ListPlacer()[idx]) )
				break;
			--idx;
		}
		if ( idx == -1 ) this->PosY = 0;
		else this->PosY = beforercd->PosY + beforercd->SizeHeight;
	} else this->PosY = 0;

	PositionComponent();
}
UINT Mgine::UIRow::AddPlacer(VEC MinWidth, VEC MaxWidth, ROWCOL_ALIGNMENT HorizontalAlign, ROWCOL_ALIGNMENT VerticalAlign)
{
	return AddRawPlacer(new RowPlacer(MinWidth, MaxWidth, HorizontalAlign, VerticalAlign));
}

UINT Mgine::UICol::AddPlacer(VEC MinHeight, VEC MaxHeight, ROWCOL_ALIGNMENT HorizontalAlign, ROWCOL_ALIGNMENT VerticalAlign)
{
	return AddRawPlacer(new ColPlacer(MinHeight, MaxHeight, HorizontalAlign, VerticalAlign));
}

