#pragma once

namespace Mgine
{
	class Collision
	{
	protected:
		Collision(){ }
	public:
		enum TYPE 
		{
			NONE = 0,
			DOT,
			LINE,
			TRIANGLE,
			CIRCLE
		};
		TYPE Type;
		virtual bool IsBoundingWith(float PointX, float PointY){ return false; }
		virtual bool IsBoundingWith(Collision *){ return false; }
	};

	class CollisionDot : public Collision
	{
	public:
		VECTOR2 Dot;
	
		CollisionDot(VEC PointX, VEC PointY);
		CollisionDot(VECTOR2 Dot);

		bool IsBoundingWith(VEC PointX, VEC PointY);
		bool IsBoundingWith(Collision *OppositeCollision);
	};

	class CollisionLine : public Collision
	{
	private:
	public:
		VERTEX_LINE Line; // Sorted by X, Y
		bool IsReversedLine;
		// y == Inclination*x + Offset
		double FormulaInclination, 
			FormulaOffset;

		CollisionLine(VERTEX_LINE Line);
		CollisionLine(float PointX1, float PointY1, float PointX2, float PointY2);

		bool IsBoundingWith(float PointX, float PointY);
		bool IsBoundingWith(Collision *OppositeCollision);
	};

	class CollisionTriangle : public Collision
	{
	private:
	public:

		VERTEX_TRIANGLE Triangle; // Sorted by X, Y
		//clsCollisionLine *ColLine[3];
		bool IsReversedTriangle;
		CollisionTriangle(VERTEX_TRIANGLE Triangle);
		CollisionTriangle(float PointX1, float PointY1, float PointX2, float PointY2, float PointX3, float PointY3);
		~CollisionTriangle();
		bool IsBoundingWith(float PointX, float PointY);
		bool IsBoundingWith(Collision *OppositeCollision);
	};

	class CollisionCircle : public Collision
	{
	private:
	public:

		VECTOR2 Position;
		float Radius;
		CollisionCircle(VECTOR2 Position, VEC Radius);
		CollisionCircle(VEC PositionX, VEC PositionY, VEC Radius);
		~CollisionCircle();
		bool IsBoundingWith(float PointX, float PointY);
		bool IsBoundingWith(Collision *OppositeCollision);
	};




	class CollisionBuffer
	{
	private:
		int Width, Height;
		Util::UnwindArray<BYTE> Buffer;
	
	public:
		inline CollisionBuffer()
		{ SetSize(0, 0); }
		inline CollisionBuffer(int Width, int Height)
		{ SetSize(Width, Height); }
	
		bool SetSize(int Width, int Height);
		void SetBuffer(BYTE *Buffer);
		bool SetFromSurface(IDirect3DSurface9 *Surface, int TargetWidth, int TargetHeight);
		bool IsCollisionWithDot(int X, int Y);
		//HRGN GetAllocRegion();
		void Uninit();
	};
}
