#pragma once
#include "Engine.h"
//#define MGINE_CONTAINER_DEFAULT_DEPTH	6

namespace Mgine
{
	class QuadTree;
	class GameObject;

	class GameObjectManager
	{
	private:
		static Util::TriList<GameObject*> *iListObject;
		static Util::TriList<QuadTree*>	*iListQuadTree;

		static QuadTree *pRootQuadTree[2][2];

		static bool IsInited;

	public:
		static inline Util::TriList<GameObject*> & ListObject()
		{
			if ( !EngineManager::Core->IsAppFinished && !iListObject ) iListObject = new Util::TriList<GameObject*>;
			return *iListObject;
		}
		static inline Util::TriList<QuadTree*> & ListQuadTree()
		{
			if ( !EngineManager::Core->IsAppFinished && !iListQuadTree ) iListQuadTree = new Util::TriList<QuadTree*>;
			return *iListQuadTree;
		}
		static inline bool GetIsInited()
		{ return IsInited; }




		static void Init();
		static void Uninit();

		static void OnAppFinish();

		static void Update(double ElapsedTime);
		static void Clear();


		static QuadTree *GetCreateQuadTreeByPosition(float X, float Y);
		static void GetCreateQuadTreeByRect(float X, float Y, float Width, float Height, Util::Queue<QuadTree*> *ResultQueue);
	};

	class QuadTree
	{
	private:
		void Constructor();
		void Destructor();
	public:
		static const int DefaultDepth = 6;

		LONGLONG Depth; // Zero for TOP
		LONGLONG UnitX, UnitY, UnitInterval;
		LONG		RefCount;
		DWORD		QuadTreeIndex;
		VECTOR2		Position;
		QuadTree *Parent;
		QuadTree *Child[2][2];
		Util::Queue<GameObject*>	QueueGameObject;
		Util::Queue<GameObject*>	QueueCollisionObject;

		inline QuadTree(LONGLONG Depth, LONGLONG UnitX, LONGLONG UnitY, QuadTree *Parent)
		{
			Constructor();
			this->Parent = Parent;
			this->UnitX = UnitX; this->UnitY = UnitY; this->Depth = Depth;
			Set();
			if ( Parent ) Parent->IncRef();
		}
		~QuadTree(){ Destructor(); }
		void Set();
		QuadTree *GetCreateChild(BYTE RelativeX, BYTE RelativeY);
		QuadTree *GetCreateChild(float PositionX, float PositionY);
		bool Contain(QuadTree *Child);
		bool IsContaining(float PositionX, float PositionY);
		LONG IncRef();
		LONG DecRef();
	};

	class GameObject
	{
	private:
		virtual void Constructor();
		virtual void Destructor();
	protected:
	public:
		bool IsInited;
		bool IsKilling;

		bool IsCollision;
		bool IsCollisionExceptSameType;

		VECTOR3 Position;
		VECTOR3 Velocity;

		QuadTree *pQuadTree;
		Util::Queue<QuadTree*>		QueueQuadTree;
		Util::Queue<int>				QueueQuadTreeObjectIndex;
		DWORD		QuadTreeIndex;
		RECTANGLE	BoundingBox; //Relative to Position
	

		float		Radian;
		float		Weight;

		double		Progress;
		float		DegreeKill;

		int			ObjectIndex;
		LONG		RefCount;

		int			FrameStucked;

		GameObject *Owner;

		DWORD ObjectType;
		DWORD ObjectFlag;
		DWORD ObjectTeam;

		inline GameObject()
		{ Constructor(); /*IncRef();*/ }
		virtual ~GameObject(){ Destructor();  }

		virtual DWORD IncRef(){ return ++RefCount; }
		virtual DWORD DecRef(){ return --RefCount; }

		virtual void Destroy(){ if ( !IsInited ) return; DecRef(); IsInited = false; }
		virtual void OnCollision(GameObject *OppositeObject, D3DXVECTOR3 OppositePosition, D3DXVECTOR3 OppositeVelocity, int *Depth, bool *Stucked){ }
		virtual bool CheckForCollision(GameObject *OppositeObject, int *Depth, bool *Stucked, bool IsAlternative)
		{
			if ( !IsCollision ) return false;
			//if ( Depth ) return;
			if ( !IsAlternative ) return OppositeObject->CheckForCollision(this, Depth, Stucked, true);
			return false;
		}

		virtual void Move();
		virtual void Move(int *Depth, bool *Stucked);

		virtual void SetPosition();

		virtual void GiveDamage(float Damage){ }
		virtual void Kill(){ IsKilling = true; }

		virtual void Update(double ElapsedTime)
		{ 
			if ( Position.X < -0x1000000000000000 ) Position.X = (VEC)-0x1000000000000000;
			if ( Position.X >= 0x1000000000000000 ) Position.X = (VEC)0x1000000000000000;
			if ( Position.Y < -0x1000000000000000 ) Position.Y = (VEC)-0x1000000000000000;
			if ( Position.Y >= 0x1000000000000000 ) Position.Y = (VEC)0x1000000000000000;
			if ( Velocity.X < -0x1000000000000000 ) Velocity.X = (VEC)-0x1000000000000000;
			if ( Velocity.X >= 0x1000000000000000 ) Velocity.X = (VEC)0x1000000000000000;
			if ( Velocity.Y < -0x1000000000000000 ) Velocity.Y = (VEC)-0x1000000000000000;
			if ( Velocity.Y >= 0x1000000000000000 ) Velocity.Y = (VEC)0x1000000000000000;
			Progress += ElapsedTime;
		}
		virtual void Draw(){  }

		virtual void EnumAssociatedObject(Util::Queue<GameObject*> *QueueResult);
		virtual void EnumAssociatedObject(Util::Queue<GameObject*> *QueueResult, bool IsOnlyIsCollision, bool IsExceptSameType);

	
	};
}