#include "StdAfx.hpp"
#include "GameObject.h"
#include "Function.h"
#include "EnhancedNative.h"

using namespace Mgine;
using namespace Util;

bool   GameObjectManager::IsInited;
TriList<GameObject*> *GameObjectManager::iListObject;
TriList<QuadTree*>  *GameObjectManager::iListQuadTree;
QuadTree *GameObjectManager::pRootQuadTree[2][2];


void GameObjectManager::Init()
{
	MLOG("Begin");
	IsInited = true;
}

void GameObjectManager::Uninit()
{
	IsInited = false;
	for ( StepIterator<GameObject *> element; ListObject().Step(element); )
		try 
		{ delete element; }
		catch ( BaseException & )
		{ MLOG("Failed object %d", element.Index); }

	for ( StepIterator<QuadTree *> element; ListQuadTree().Step(element); )
		try 
		{ 
			element->Parent = 0;
			delete element;
		} catch ( BaseException & )
		{
			MLOG("Failed quadtree %d", element.Index);
		}

	memset(pRootQuadTree, 0, sizeof(pRootQuadTree));
}

void GameObjectManager::OnAppFinish()
{
	delete iListObject;
	iListObject = NULL;
	delete iListQuadTree;
	iListQuadTree = NULL;
}

void GameObjectManager::Update(double ElapsedTime)
{
	for ( StepIterator<GameObject *> element; ListObject().Step(element); )
		if ( element->RefCount )
			element->Update(ElapsedTime);
		else delete element;

	
	for ( StepIterator<QuadTree *> element; ListQuadTree().Step(element); )
		if ( !element->RefCount )
		{
			QuadTree *parent = element->Parent;
			if ( parent )
			{
				if      ( parent->Child[0][0] == element ) parent->Child[0][0] = 0;
				else if ( parent->Child[0][1] == element ) parent->Child[0][1] = 0;
				else if ( parent->Child[1][0] == element ) parent->Child[1][0] = 0;
				else if ( parent->Child[1][1] == element ) parent->Child[1][1] = 0;
#ifdef _DEBUG
				else __asm int 3;
#endif
			} else {
				if      ( pRootQuadTree[0][0] == element ) pRootQuadTree[0][0] = 0;
				else if ( pRootQuadTree[0][1] == element ) pRootQuadTree[0][1] = 0;
				else if ( pRootQuadTree[1][0] == element ) pRootQuadTree[1][0] = 0;
				else if ( pRootQuadTree[1][1] == element ) pRootQuadTree[1][1] = 0;
#ifdef _DEBUG
				else __asm int 3;
#endif
			}

		}

}

void GameObjectManager::Clear()
{
	for ( StepIterator<GameObject *> element; ListObject().Step(element); )
		delete element;
	ListObject().Clear();

	for ( StepIterator<QuadTree *> element; ListQuadTree().Step(element); )
	{
		element->Parent = 0;
		delete element;
	}
	ListQuadTree().Clear();

	memset(pRootQuadTree, 0, sizeof(pRootQuadTree));
}




QuadTree *GameObjectManager::GetCreateQuadTreeByPosition(float X, float Y)
{
	QuadTree *contTarget = 0;
	float logx = log(MF::Abs(X))/log(2.f), logy = log(MF::Abs(Y))/log(2.f);
	logx = logx; logy = logy;
	if ( X == 0 ) logx = 0;
	if ( Y == 0 ) logy = 0;

	LONGLONG depth = 0;
	if ( logx > logy ) depth = -(LONGLONG)(logx+1) + QuadTree::DefaultDepth;
	else depth = -(LONGLONG)(logy+1) + QuadTree::DefaultDepth;

	if ( depth > 0 ) depth = 0;
	LONGLONG unitInterval = (LONGLONG)pow(2., (int)(QuadTree::DefaultDepth-depth));
	LONGLONG minInterval = (LONGLONG)pow(2., (int)(QuadTree::DefaultDepth));
	char mulx, muly;
	char dx, dy;

	if ( X < 0 ) dx = 0;
	else dx = 1;
	if ( Y < 0 ) dy = 0;
	else dy = 1;

	mulx = dx-1;
	muly = dy-1;

	if ( !pRootQuadTree[dx][dy] ) pRootQuadTree[dx][dy] = new QuadTree(depth, (LONGLONG)(unitInterval/minInterval*mulx), (LONGLONG)(unitInterval/minInterval*muly), 0);
	if ( pRootQuadTree[dx][dy]->Depth > depth ){
		QuadTree *tcontainer = new QuadTree(depth, (LONGLONG)(unitInterval/minInterval*mulx), (LONGLONG)(unitInterval/minInterval*muly), 0);
		if ( !tcontainer->Contain(pRootQuadTree[dx][dy]) ){
			delete tcontainer;
			tcontainer = 0;
			return NULL;
/*#ifdef _DEBUG
			__asm int 3;
#endif*/
		}
		pRootQuadTree[dx][dy] = tcontainer;
	}
	contTarget = pRootQuadTree[dx][dy];

	return contTarget->GetCreateChild(X, Y);
}

void GameObjectManager::GetCreateQuadTreeByRect(float X, float Y, float Width, float Height, Queue<QuadTree*> *ResultQueue)
{
	if ( Width < 0 ) return ;
	if ( Height < 0 ) return ;
	if ( X < (float)-0x1000000000000000 ||
		X + Width  >= (float)0x1000000000000000 ) return ;
	if ( Y < (float)-0x1000000000000000 ||
		Y + Height >= (float)0x1000000000000000 ) return ;

	LONGLONG minInterval = (LONGLONG)pow(2., (int)(QuadTree::DefaultDepth));
	QuadTree *contTarget = 0;
	float logx, logy;
	float originX = X, originY = Y;

	LONGLONG xgap, ygap;
	LONGLONG tmpLL;
	
	xgap = (LONGLONG)(X+Width)/minInterval;
	if ( X+Width < 0 ) xgap--;
	tmpLL = (LONGLONG)(X)/minInterval;
	if ( X < 0 ) tmpLL--;
	xgap -= tmpLL;

	ygap = (LONGLONG)(Y+Height)/minInterval;
	if ( Y+Height < 0 ) ygap--;
	tmpLL = (LONGLONG)(Y)/minInterval;
	if ( Y < 0 ) tmpLL--;
	ygap -= tmpLL;

	for ( LONGLONG i=0; i <= MF::Abs(xgap); i++ ){
		for ( LONGLONG p=0; p <= MF::Abs(ygap); p++ ){
			X = originX + i*minInterval*MF::Sgn(xgap);
			Y = originY + p*minInterval*MF::Sgn(ygap);
			logx = log(MF::Abs(X))/log(2.f);
			logy = log(MF::Abs(Y))/log(2.f);
			if ( X == 0 ) logx = 0;
			if ( Y == 0 ) logy = 0;
			LONGLONG depth = 0;
			if ( logx > logy ) depth = -(LONGLONG)(logx+1) + QuadTree::DefaultDepth;
			else depth = -(LONGLONG)(logy+1) + QuadTree::DefaultDepth;

			if ( depth > 0 ) depth = 0;
			LONGLONG unitInterval = (LONGLONG)pow(2., (int)(QuadTree::DefaultDepth-depth));
			char mulx, muly;
			char dx, dy;

			if ( X < 0 ) dx = 0;
			else dx = 1;
			if ( Y < 0 ) dy = 0;
			else dy = 1;

			mulx = dx-1;
			muly = dy-1;

			if ( !pRootQuadTree[dx][dy] ) pRootQuadTree[dx][dy] = new QuadTree(depth, (LONGLONG)(unitInterval/minInterval*mulx), (LONGLONG)(unitInterval/minInterval*muly), 0);
			if ( pRootQuadTree[dx][dy]->Depth > depth ){
				QuadTree *tcontainer = new QuadTree(depth, (LONGLONG)(unitInterval/minInterval*mulx), (LONGLONG)(unitInterval/minInterval*muly), 0);
				if ( !tcontainer->Contain(pRootQuadTree[dx][dy]) ){
					delete tcontainer;
					return ;
/*#ifdef _DEBUG
					__asm int 3;
#endif*/
				}
				pRootQuadTree[dx][dy] = tcontainer;
			}
			contTarget = pRootQuadTree[dx][dy];

			ResultQueue->Add(contTarget->GetCreateChild(X, Y));
		}
	}
}





void QuadTree::Constructor()
{
	UnitInterval = UnitY = UnitX = Depth = 0;
	RefCount = 0;
	Parent = 0;
	memset(Child, 0, sizeof(Child));

	QuadTreeIndex = GameObjectManager::ListQuadTree().Add(this);
}

void QuadTree::Destructor()
{
	if ( Parent ) Parent->DecRef();
	GameObjectManager::ListQuadTree().Sub(QuadTreeIndex);
}

void QuadTree::Set()
{
	UnitInterval = (LONGLONG)pow(2., (int)(QuadTree::DefaultDepth-Depth));
	Position.X = UnitX*(VEC)pow(2., (int)(QuadTree::DefaultDepth));
	Position.Y = UnitY*(VEC)pow(2., (int)(QuadTree::DefaultDepth));
}

QuadTree *QuadTree::GetCreateChild(BYTE RelativeX, BYTE RelativeY)
{
	if ( RelativeX > 1 || RelativeY > 1 ) return NULL;
	if ( Depth > 0 && !Child[RelativeX][RelativeY] ){
		Child[RelativeX][RelativeY] = new QuadTree(Depth+1, UnitX+RelativeX, UnitY+RelativeY, this);
	}
	return Child[RelativeX][RelativeY];
}

QuadTree *QuadTree::GetCreateChild(float PositionX, float PositionY)
{
	if ( PositionX < (float)-0x1000000000000000 ||
		PositionX >= (float)0x1000000000000000 ) return NULL;
	if ( PositionY < (float)-0x1000000000000000 ||
		PositionY >= (float)0x1000000000000000 ) return NULL;

	LONGLONG i;
	QuadTree *res = 0;
	QuadTree *tcont = this;

	LONGLONG minInterval = (LONGLONG)pow(2., QuadTree::DefaultDepth);

	if ( !(tcont->UnitX*minInterval <= PositionX && PositionX < tcont->UnitX*minInterval + tcont->UnitInterval &&
		tcont->UnitY*minInterval <= PositionY && PositionY < tcont->UnitY*minInterval + tcont->UnitInterval ) ) 
		return NULL;

	for ( i=Depth; i < 0; i++ ){
		char dx = 0, dy = 0;
		if ( tcont->UnitX*minInterval <= PositionX && PositionX < tcont->UnitX*minInterval + tcont->UnitInterval/2 ) dx = 0;
		else dx = 1;
		if ( tcont->UnitY*minInterval <= PositionY && PositionY < tcont->UnitY*minInterval + tcont->UnitInterval/2 ) dy = 0;
		else dy = 1;

		if ( !tcont->Child[dx][dy] ) tcont->Child[dx][dy] = new QuadTree(tcont->Depth+1, tcont->UnitX + tcont->UnitInterval/minInterval/2*dx, tcont->UnitY + tcont->UnitInterval/minInterval/2*dy, tcont);
		if ( (DWORD)tcont->Child[dx][dy] > 0xf0000000 ) __asm int 3;
		tcont = tcont->Child[dx][dy];
	}
	return tcont;
}

bool QuadTree::Contain(QuadTree *Child)
{
	if ( !Child || Child->Parent ) return false;
	LONGLONG i;
	bool isSucceeded = false;
	LONGLONG ux, uy, ui;
	QuadTree *contBefore = this;

	LONGLONG minInterval = (LONGLONG)pow(2., QuadTree::DefaultDepth);

	if ( UnitX*minInterval <= Child->UnitX*minInterval && Child->UnitX*minInterval < UnitX*minInterval + UnitInterval &&
		UnitY*minInterval <= Child->UnitY*minInterval && Child->UnitY*minInterval < UnitY*minInterval + UnitInterval ){
		
		ux = UnitX; uy = UnitY; ui = UnitInterval;
		for ( i=Depth+1; i < Child->Depth; i++ ){
			char dx = 0, dy = 0;
			if ( ux <= Child->UnitX && Child->UnitX + Child->UnitInterval/minInterval <= ux + ui/minInterval/2 ) dx = 0;
			else dx = 1;
			if ( uy <= Child->UnitY && Child->UnitY + Child->UnitInterval/minInterval <= uy + ui/minInterval/2 ) dy = 0;
			else dy = 1;
				
			if ( !contBefore->Child[dx][dy] ) contBefore->Child[dx][dy] = new QuadTree(i, ux+ui/minInterval/2*dx, uy+ui/minInterval/2*dy, contBefore);

			ux = contBefore->Child[dx][dy]->UnitX;
			uy = contBefore->Child[dx][dy]->UnitY;
			ui = contBefore->Child[dx][dy]->UnitInterval;

			contBefore = contBefore->Child[dx][dy];
		}
		char dx = 0, dy = 0;
		if ( ux <= Child->UnitX && Child->UnitX + Child->UnitInterval/minInterval <= ux + ui/minInterval/2 ) dx = 0;
		else dx = 1;
		if ( uy <= Child->UnitY && Child->UnitY + Child->UnitInterval/minInterval <= uy + ui/minInterval/2 ) dy = 0;
		else dy = 1;

#ifdef _DEBUG
		if ( contBefore->Child[dx][dy] ) __asm int 3;
#endif
		contBefore->Child[dx][dy] = Child;
		Child->Parent = contBefore;
		contBefore->IncRef();
		


		isSucceeded = true;
	}
#ifdef _DEBUG
	else __asm int 3;
#endif
	return isSucceeded;
}

bool QuadTree::IsContaining(float PositionX, float PositionY)
{
	LONGLONG minInterval = (LONGLONG)pow(2., QuadTree::DefaultDepth);
	return	UnitX*minInterval <= PositionX && PositionX < UnitX*minInterval + UnitInterval &&
			UnitY*minInterval <= PositionY && PositionY < UnitY*minInterval + UnitInterval ;
}

LONG QuadTree::IncRef()
{
	return ++RefCount;
}

LONG QuadTree::DecRef()
{
	return --RefCount;
}





void GameObject::EnumAssociatedObject(Queue<GameObject*> *QueueResult)
{
	EnumAssociatedObject(QueueResult, false, false);
}

void GameObject::EnumAssociatedObject(Queue<GameObject*> *QueueResult, bool IsOnlyCollision, bool IsExceptSameType)
{
	if ( !QueueResult ) return ;
	for ( int i=0; i < QueueQuadTree.Count; i++ ){
		int countp = 0;
		for ( int p=0; p < QueueQuadTree[i]->QueueCollisionObject.Size; p++ ){
			if ( countp >= QueueQuadTree[i]->QueueCollisionObject.Count ) break;
			GameObject *object = QueueQuadTree[i]->QueueCollisionObject[p];
			if ( object && 
				object != this &&
				((IsOnlyCollision  && object->IsCollision) || !IsOnlyCollision) &&
				((IsExceptSameType && object->ObjectType != ObjectType) || !IsExceptSameType) &&
				object->IsInited ){
				int j;
				countp++;
				for ( j=0; j < QueueResult->Count; j++ )
					if ( QueueResult->IsElementExisting(j) && QueueResult->Get(j) == object ) break;
				if ( j < QueueResult->Count ) continue;
				QueueResult->Add(object);
			}
		}
	}
}

void GameObject::Constructor()
{
	if ( !GameObjectManager::GetIsInited() )
		MTHROW(InvalidStatus, "GameObject is cereated before initialization. Note that declaration of static game object is prohibited.");

	TEB *teb = NtCurrentTeb();
	if ( (DWORD)teb->Tib.StackBase < (DWORD)this && (DWORD)this < (DWORD)teb->Tib.StackLimit )
		MTHROW(InvalidOperation, "GameObject is created as a local variable. This is prohibited because it would be a problem with garbage collection system. Use 'new' Keyword to create a GameObject.");

	ObjectTeam = ObjectFlag = 0;
	Owner = NULL;
	Weight = 0;
	Progress = 0;
	BoundingBox.Height = BoundingBox.Width = BoundingBox.Y = BoundingBox.X = 0;
	Radian = Position.Z = Position.Y = Position.X = Velocity.Z = Velocity.Y = Velocity.X = 0;
	FrameStucked = 0;
	DegreeKill = 0;

	QuadTreeIndex = 0;

	ObjectType = 0;

	IsCollision = IsInited = true;
	IsCollisionExceptSameType = false;
	IsKilling = false;
	pQuadTree = 0;
	RefCount = 1;

	ObjectIndex = GameObjectManager::ListObject().Add(this);

	SetPosition();
}

void GameObject::Destructor()
{
	if ( pQuadTree )
	{
		pQuadTree->QueueGameObject.Sub(QuadTreeIndex);
		pQuadTree->DecRef();
	}

#if _DEBUG
	if ( QueueQuadTree.Count != QueueQuadTreeObjectIndex.Count )
		MF::BreakExecution();
#endif
	for ( DWORD i=0, count=QueueQuadTree.Count; QueueQuadTree.Count; i++ )
	{
		((QuadTree*)QueueQuadTree[i])->DecRef();
		//((clsQuadTree*)QueueQuadTree[i])->QueueCollisionObject.Sub(QueueQuadTree.Arr[i+count/2]);
		((QuadTree*)QueueQuadTree[i])->QueueCollisionObject.Sub(QueueQuadTreeObjectIndex[i]);
		QueueQuadTree.Sub(i);
		QueueQuadTreeObjectIndex.Sub(i);
		//QueueQuadTree.Sub(i+count/2);
	}

	GameObjectManager::ListObject().Sub(ObjectIndex);
}

void GameObject::Move()
{
	if ( !IsCollision ){
		Position.X += Velocity.X;
		Position.Y += Velocity.Y;
		SetPosition();
	} else {
		bool stucked = false;
		int depth = 0;
		Move(&depth, &stucked);
		if ( stucked ) FrameStucked++;
		else FrameStucked = 0;
	}
}

void GameObject::Move(int *Depth, bool *Stucked)
{
	if ( !Depth ) return;
	bool col = !*Depth;
	(*Depth)++;
	if ( *Depth >= 10 ){
		if ( Stucked ) *Stucked = true;
		return;
	} else if ( !*Depth ){
		if ( Stucked ) *Stucked = false;
	}

	bool isCol = false;
	VECTOR3 vel = Velocity;

	Queue<GameObject*> resQueue;
	EnumAssociatedObject(&resQueue, true, IsCollisionExceptSameType);

	for ( int i=0; i < resQueue.Count; i++ ){
		if ( ((GameObject*)resQueue[i])->IsCollision &&
			resQueue[i] != this ){

			if ( CheckForCollision((GameObject*)resQueue[i], Depth, Stucked, 0) ){
				isCol = true;
			} else {
				
			}
		}
	}
	if ( !isCol /*&& col*/ ){
		Position += Velocity;
		//D3DXVec3Add(&Position, &Position, &Velocity);
		SetPosition();
	}
}

void GameObject::SetPosition()
{
	//MgineGameObject::GetCreateQuadTreeByPosition(100, 31);
	//MgineGameObject::GetCreateQuadTreeByPosition(500000, 31);
	if ( !pQuadTree ){
		pQuadTree = GameObjectManager::GetCreateQuadTreeByPosition(Position.X, Position.Y);
		if ( pQuadTree )
		{
			pQuadTree->IncRef();
			QuadTreeIndex = pQuadTree->QueueGameObject.Add(this);
		}
	} else {
		if ( !pQuadTree->IsContaining(Position.X, Position.Y) ){
			pQuadTree->QueueGameObject.Sub(QuadTreeIndex);
			pQuadTree->DecRef();

			pQuadTree = GameObjectManager::GetCreateQuadTreeByPosition(Position.X, Position.Y);
			if ( pQuadTree )
			{
				QuadTreeIndex = pQuadTree->QueueGameObject.Add(this);
				pQuadTree->IncRef();
			}
		}
	}
	
#if _DEBUG
	if ( QueueQuadTree.Count != QueueQuadTreeObjectIndex.Count )
		MF::BreakExecution();
#endif
	for ( DWORD i=0, count=QueueQuadTree.Count; QueueQuadTree.Count; i++ ){
		QueueQuadTree[i]->DecRef();
		//((clsQuadTree*)QueueQuadTree[i])->QueueCollisionObject.Sub(QueueQuadTree.Arr[i+count/2]);
		QueueQuadTree[i]->QueueCollisionObject.Sub(QueueQuadTreeObjectIndex[i]);
		QueueQuadTree.Sub(i);
		QueueQuadTreeObjectIndex.Sub(i);
		//QueueQuadTree.Sub(i+count/2);
	}

	QueueQuadTreeObjectIndex.Clear();

	GameObjectManager::GetCreateQuadTreeByRect(
		Position.X + BoundingBox.X,
		Position.Y + BoundingBox.Y,
		BoundingBox.Width,
		BoundingBox.Height,
		&QueueQuadTree);

	for ( DWORD i=0, count=QueueQuadTree.Count; i < count; i++ ){
		QueueQuadTree[i]->IncRef();
		QueueQuadTreeObjectIndex.Add(
			QueueQuadTree[i]->QueueCollisionObject.Add(this));
	}
}





