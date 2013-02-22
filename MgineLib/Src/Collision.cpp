#include "StdAfx.hpp"
#include "Collision.h"
#include "Function.h"
using namespace Mgine;

CollisionDot::CollisionDot(VEC PointX, VEC PointY)
{
	Type = DOT;
	Dot.X = PointX;
	Dot.Y = PointY;
}

CollisionDot::CollisionDot(VECTOR2 Dot)
{
	this->Dot = Dot;
}

bool CollisionDot::IsBoundingWith(VEC PointX, VEC PointY)
{
	CollisionDot dot(PointX, PointY);
	return IsBoundingWith(&dot);
}

bool CollisionDot::IsBoundingWith(Collision *OppositeCollision)
{
	switch ( OppositeCollision->Type ){
		case DOT:
			{
				CollisionDot *dot = (CollisionDot*)OppositeCollision;
				if ( dot->Dot == Dot ){
					return true;
				} else return false;
			}
			break;
		case LINE:
			{
				CollisionLine *line = (CollisionLine*)OppositeCollision;
				if ( MF::Abs( line->Line.V1.X - line->Line.V2.X ) < 0.5f && MF::Abs(line->Line.V1.X - Dot.X) < 0.5f ){
					if ( line->Line.V1.Y-0.5f <= Dot.Y && Dot.Y <= line->Line.V2.Y+0.5f ){
						return true;
					}
				} else if ( line->Line.V1.X-0.5f <= Dot.X && line->Line.V2.X <= Dot.X+0.5f ){
					if ( MF::Abs(Dot.Y - (line->FormulaInclination * Dot.X + line->FormulaOffset)) < 0.5f ){
						return true;
					}
				}
				return false;
			}
			break;
		case TRIANGLE:
			{
				CollisionTriangle *tri = (CollisionTriangle*)OppositeCollision;
				double inclination = 0;
				double offset = 0;

				if ( tri->Triangle.V1.X == tri->Triangle.V2.X ){
					if ( !(tri->Triangle.V1.X < Dot.X) ) return false;
				} else {
					inclination = (tri->Triangle.V1.Y - tri->Triangle.V2.Y) / (tri->Triangle.V1.X - tri->Triangle.V2.X);
					offset = -(tri->Triangle.V1.X * inclination)+tri->Triangle.V1.Y;
					if ( tri->IsReversedTriangle ){
						if ( !(Dot.Y > Dot.X  * inclination + offset) ) return false;
					} else {
						if ( !(Dot.Y < Dot.X  * inclination + offset) ) return false;
					}
				}
				
				if ( tri->Triangle.V2.X == tri->Triangle.V3.X ){
					if ( !(tri->Triangle.V2.X < Dot.X) ) return false;
				} else {
					inclination = (tri->Triangle.V2.Y - tri->Triangle.V3.Y) / (tri->Triangle.V2.X - tri->Triangle.V3.X);
					offset = -(tri->Triangle.V2.X * inclination)+tri->Triangle.V2.Y;
					if ( tri->IsReversedTriangle ){
						if ( !(Dot.Y > Dot.X * inclination + offset) ) return false;
					} else {
						if ( !(Dot.Y < Dot.X * inclination + offset) ) return false;
					}
				}

				if ( tri->Triangle.V1.X == tri->Triangle.V3.X ){
					if ( !(tri->Triangle.V1.X < Dot.X) ) return false;
				} else {
					inclination = (tri->Triangle.V1.Y - tri->Triangle.V3.Y) / (tri->Triangle.V1.X - tri->Triangle.V3.X);
					offset = -(tri->Triangle.V1.X * inclination)+tri->Triangle.V1.Y;
					if ( tri->IsReversedTriangle ){
						if ( !(Dot.Y < Dot.X * inclination + offset) ) return false;
					} else {
						if ( !(Dot.Y > Dot.X * inclination + offset) ) return false;
					}
				}
				
				return true;
				// V1 V2 V3 are sorted by X.

			}
			break;
		case CIRCLE:
			{
				CollisionCircle *colcircle = (CollisionCircle*)OppositeCollision;
				if ( 
					(Dot.X-colcircle->Position.X)*(Dot.X-colcircle->Position.X)+(Dot.Y-colcircle->Position.Y)*(Dot.Y-colcircle->Position.Y)
					<= colcircle->Radius*colcircle->Radius )
					return true;
			}
			break;
		default:break;
	}

	return false;
}


CollisionLine::CollisionLine(float PointX1, float PointY1, float PointX2, float PointY2)
{
	Type = LINE;
	Line.V1.X = PointX1;
	Line.V1.Y = PointY1;
	Line.V2.X = PointX2;
	Line.V2.Y = PointY2;

	if ( Line.V1.X > Line.V2.X ){
		VERTEX_DOT t = Line.V2;
		Line.V2 = Line.V1;
		Line.V1 = t;
	}

	if ( this->Line.V1.X == this->Line.V2.X &&
		this->Line.V1.Y > this->Line.V2.Y ){

		VERTEX_DOT t = this->Line.V2;
		this->Line.V2 = this->Line.V1;
		this->Line.V1 = t;
	}

	if ( Line.V1.Y > Line.V2.Y ){
		IsReversedLine = true;
	} else IsReversedLine = false;

	FormulaInclination = (Line.V1.Y - Line.V2.Y) / (Line.V1.X - Line.V2.X);
	FormulaOffset = Line.V1.Y - (Line.V1.X * FormulaInclination);
}

CollisionLine::CollisionLine(VERTEX_LINE Line)
{
	Type = LINE;
	this->Line = Line;

	if ( Line.V1.X > Line.V2.X ){
		VERTEX_DOT t = this->Line.V2;
		this->Line.V2 = this->Line.V1;
		this->Line.V1 = t;
	}

	if ( this->Line.V1.X == this->Line.V2.X &&
		this->Line.V1.Y > this->Line.V2.Y ){

		VERTEX_DOT t = this->Line.V2;
		this->Line.V2 = this->Line.V1;
		this->Line.V1 = t;
	}

	if ( this->Line.V1.Y > this->Line.V2.Y ){
		IsReversedLine = true;
	} else IsReversedLine = false;

	FormulaInclination = (this->Line.V1.Y - this->Line.V2.Y) / (this->Line.V1.X - this->Line.V2.X);
	FormulaOffset = this->Line.V1.Y - (this->Line.V1.X * FormulaInclination);
}

bool CollisionLine::IsBoundingWith(float PointX, float PointY)
{
	CollisionDot dot(PointX, PointY);
	return IsBoundingWith((Collision*)&dot);
}

bool CollisionLine::IsBoundingWith(Collision *OppositeCollision)
{
	switch ( OppositeCollision->Type ){
		case DOT:
			return OppositeCollision->IsBoundingWith((Collision*)this);
			break;
		case LINE:
			{
				CollisionLine *line = (CollisionLine*)OppositeCollision;

				if ( line->Line.V1.X - line->Line.V2.X == 0 ){
					if ( Line.V1.X - Line.V2.X == 0 ){
						if ( Line.V1.X - line->Line.V1.X == 0 ){
							if ( !IsReversedLine ){
								if ( Line.V1.Y-0.5f <= line->Line.V1.Y && line->Line.V1.Y <= Line.V2.Y+0.5f ) return true;
								if ( Line.V1.Y-0.5f <= line->Line.V2.Y && line->Line.V2.Y <= Line.V2.Y+0.5f ) return true;
								if ( line->Line.V1.Y-0.5f <= Line.V1.Y && Line.V1.Y <= line->Line.V2.Y+0.5f ) return true;
								if ( line->Line.V1.Y-0.5f <= Line.V2.Y && Line.V2.Y <= line->Line.V2.Y+0.5f ) return true;
							} else {
								if ( Line.V2.Y-0.5f <= line->Line.V1.Y && line->Line.V1.Y <= Line.V1.Y+0.5f ) return true;
								if ( Line.V2.Y-0.5f <= line->Line.V2.Y && line->Line.V2.Y <= Line.V1.Y+0.5f ) return true;
								if ( line->Line.V2.Y-0.5f <= Line.V1.Y && Line.V1.Y <= line->Line.V1.Y+0.5f ) return true;
								if ( line->Line.V2.Y-0.5f <= Line.V2.Y && Line.V2.Y <= line->Line.V1.Y+0.5f ) return true;
							}
						}
						return false;
					}

					double y = line->Line.V1.X*FormulaInclination + FormulaOffset;
					if ( !line->IsReversedLine ){
						if ( line->Line.V1.Y-0.5f <= y && y <= line->Line.V2.Y+0.5f &&
							Line.V1.X-0.5f <= line->Line.V1.X && line->Line.V1.X <= Line.V2.X+0.5f ) return true;
					} else {
						if ( line->Line.V2.Y-0.5f <= y && y <= line->Line.V1.Y+0.5f &&
							Line.V1.X-0.5f <= line->Line.V1.X && line->Line.V1.X <= Line.V2.X+0.5f ) return true;
					}
				} else {
					if ( Line.V1.X - Line.V2.X == 0 ){
						double y = Line.V1.X*line->FormulaInclination + line->FormulaOffset;
						if ( !IsReversedLine ){
							if ( Line.V1.Y-0.5f <= y && y <= Line.V2.Y+0.5f &&
								line->Line.V1.X-0.5f <= Line.V1.X && Line.V1.X <= line->Line.V2.X+0.5f ) return true;
						} else {
							if ( Line.V2.Y-0.5f <= y && y <= Line.V1.Y+0.5f &&
								line->Line.V1.X-0.5f <= Line.V1.X && Line.V1.X <= line->Line.V2.X+0.5f ) return true;
						}
						return false;
					}

					double resultx = -(FormulaOffset - line->FormulaOffset)/(FormulaInclination - line->FormulaInclination);
					double resulty = (FormulaInclination * resultx + FormulaOffset);
					if ( !(FormulaInclination - line->FormulaInclination) ){
						if ( !(FormulaOffset - line->FormulaOffset) ){

							if ( (Line.V1.X-0.5f <= line->Line.V1.X && line->Line.V1.X <= Line.V2.X+0.5f) ||
								(Line.V1.X-0.5f <= line->Line.V2.X && line->Line.V2.X <= Line.V2.X+0.5f) ||
								(line->Line.V1.X-0.5f <= Line.V1.X && Line.V1.X <= line->Line.V2.X+0.5f) ||
								(line->Line.V1.X-0.5f <= Line.V2.X && Line.V2.X <= line->Line.V2.X+0.5f) ) return true;
						}
						return false;
					} else {
						if ( line->Line.V1.X-0.5f <= resultx && resultx <= line->Line.V2.X+0.5f &&
							Line.V1.X-0.5f <= resultx && resultx <= Line.V2.X+0.5f ){
							
							
							if ( !line->IsReversedLine ){
								if ( !(line->Line.V1.Y-0.5f <= resulty && resulty <= line->Line.V2.Y+0.5f) ) return false;
							} else {
								if ( !(line->Line.V2.Y-0.5f <= resulty && resulty <= line->Line.V1.Y+0.5f) ) return false;
							}
							if ( !IsReversedLine ){
								if ( !(Line.V1.Y-0.5f <= resulty && resulty <= Line.V2.Y+0.5f) ) return false;
							} else {
								if ( !(Line.V2.Y-0.5f <= resulty && resulty <= Line.V1.Y+0.5f) ) return false;
							}
							return true;
						}
					}
				}

				return false;
			}
			break;
		case TRIANGLE:
			{
				CollisionTriangle *tri = (CollisionTriangle *)OppositeCollision;
				CollisionLine ColLine1(tri->Triangle.V1.X, tri->Triangle.V1.Y, tri->Triangle.V2.X, tri->Triangle.V2.Y);
				CollisionLine ColLine2(tri->Triangle.V2.X, tri->Triangle.V2.Y, tri->Triangle.V3.X, tri->Triangle.V3.Y);
				CollisionLine ColLine3(tri->Triangle.V3.X, tri->Triangle.V3.Y, tri->Triangle.V1.X, tri->Triangle.V1.Y);

				if ( IsBoundingWith(&ColLine1) || 
					IsBoundingWith(&ColLine2) ||
					IsBoundingWith(&ColLine3) ){
						
					return true;
				} else {
					if ( tri->IsBoundingWith(Line.V1.X, Line.V1.Y) ||
						tri->IsBoundingWith(Line.V1.X, Line.V1.Y) )
						return true;
				}
				return false;
				// V1 V2 V3 are sorted by X.
				
			}
			break;
		case CIRCLE:
			{
				CollisionCircle *colcircle = (CollisionCircle*)OppositeCollision;
				double leftrank[3] = { 0, }, rightrank[3] = { 0, };
				leftrank[2] = 1;
				leftrank[1] = -2*colcircle->Position.X;
				leftrank[0] = colcircle->Position.X*colcircle->Position.X - colcircle->Radius*colcircle->Radius;

				rightrank[2] = -1;
				rightrank[1] = 2*colcircle->Position.Y;
				rightrank[0] = -colcircle->Position.Y*colcircle->Position.Y;

				if ( Line.V1.X == Line.V2.X )
				{
					// move all to the right
					// 0 = y^2 + y + 2
					double rootx = Line.V1.X;
					leftrank[2] = rootx*rootx*leftrank[2];
					leftrank[2] += rootx*leftrank[1];
					leftrank[2] += leftrank[0];
					leftrank[2] -= rightrank[0];
					rightrank[0] = -leftrank[2];

				} else 
				{
					// y=1x+1
					//(y - Formulaoffset) / FormulaInclination= x;
					//(y^2-2yFormulaoffset+Formulaoffset^2) / Formulainclination^2
					/*rightrank[2] -= leftrank[2] * 1/(FormulaInclination*FormulaInclination);
					rightrank[1] -= leftrank[2] * -2*FormulaOffset/(FormulaInclination*FormulaInclination);
					rightrank[0] -= leftrank[2] * (FormulaOffset*FormulaOffset)/FormulaInclination;
					
					rightrank[1] -= leftrank[1] * 1/(FormulaOffset);
					rightrank[0] -= leftrank[1] * -FormulaOffset/FormulaInclination;

					rightrank[0] -= leftrank[0];
					*/

					rightrank[2] -= leftrank[2]*(1/(FormulaInclination*FormulaInclination));
					rightrank[1] -= leftrank[2]*-2*FormulaOffset*(1/(FormulaInclination*FormulaInclination));
					rightrank[0] -= leftrank[2]*FormulaOffset*FormulaOffset*(1/(FormulaInclination*FormulaInclination));

					rightrank[1] -= leftrank[1]*(1/FormulaInclination);
					rightrank[0] -= leftrank[1]*-FormulaOffset*(1/FormulaInclination);

					rightrank[0] -= leftrank[0];
				}
				double lefty, righty;
				double central;
				lefty = righty = 
					(-rightrank[1])
					/(rightrank[2]*2);
				central = sqrt(rightrank[1]*rightrank[1]-4*rightrank[2]*rightrank[0])/(rightrank[2]*2);
				lefty  -= central;
				righty += central;
				
				if ( Line.V1.Y < Line.V2.Y )
				{
					if ( (Line.V1.Y <= lefty && lefty <= Line.V2.Y) ||
						(Line.V1.Y <= righty && righty <= Line.V2.Y) )
						return true;
				} else {
					if ( (Line.V2.Y <= lefty && lefty <= Line.V1.Y) ||
						(Line.V2.Y <= righty && righty <= Line.V1.Y) )
						return true;
				}
					

			}
			break;
		default:break;
	}
	return false;
}



CollisionTriangle::CollisionTriangle(float PointX1, float PointY1, float PointX2, float PointY2, float PointX3, float PointY3)
{
	Type = TRIANGLE;
	Triangle.V1.X = PointX1;
	Triangle.V1.Y = PointY1;
	Triangle.V2.X = PointX2;
	Triangle.V2.Y = PointY2;
	Triangle.V3.X = PointX3;
	Triangle.V3.Y = PointY3;

	if ( Triangle.V1.X > Triangle.V2.X ){
		VERTEX_DOT t = Triangle.V1;
		Triangle.V1 = Triangle.V2;
		Triangle.V2 = t;
	}

	if ( Triangle.V2.X > Triangle.V3.X ){
		VERTEX_DOT t = Triangle.V2;
		Triangle.V2 = Triangle.V3;
		Triangle.V3 = t;
	}
	
	if ( Triangle.V1.X > Triangle.V2.X ){
		VERTEX_DOT t = Triangle.V1;
		Triangle.V1 = Triangle.V2;
		Triangle.V2 = t;
	}

	if ( Triangle.V2.X > Triangle.V3.X ){
		VERTEX_DOT t = Triangle.V2;
		Triangle.V2 = Triangle.V3;
		Triangle.V3 = t;
	}

	if ( Triangle.V1.X == Triangle.V2.X && Triangle.V1.Y > Triangle.V2.Y ){
		VERTEX_DOT t = Triangle.V1;
		Triangle.V1 = Triangle.V2;
		Triangle.V2 = t;
	}

	if ( Triangle.V2.X == Triangle.V3.X && Triangle.V2.Y > Triangle.V3.Y ){
		VERTEX_DOT t = Triangle.V2;
		Triangle.V2 = Triangle.V3;
		Triangle.V3 = t;
	}

	double inclination, offset;
	inclination = (Triangle.V1.Y - Triangle.V3.Y) / (Triangle.V1.X - Triangle.V3.X);
	offset = -(Triangle.V1.X * inclination) + Triangle.V1.Y;
	if ( !(Triangle.V2.Y >= Triangle.V2.X*inclination + offset ) ){
		IsReversedTriangle = true;
	} else {
		IsReversedTriangle = false;
	}

	//ColLine[0] = new clsCollisionLine(Triangle.V1.X, Triangle.V1.Y, Triangle.V2.X, Triangle.V2.Y);
	//ColLine[1] = new clsCollisionLine(Triangle.V2.X, Triangle.V2.Y, Triangle.V3.X, Triangle.V3.Y);
	//ColLine[2] = new clsCollisionLine(Triangle.V3.X, Triangle.V3.Y, Triangle.V1.X, Triangle.V1.Y);
}

CollisionTriangle::CollisionTriangle(VERTEX_TRIANGLE Tri)
{
	Type = TRIANGLE;
	this->Triangle = Tri;
	if ( Triangle.V1.X > Triangle.V2.X ){
		VERTEX_DOT t = Triangle.V1;
		Triangle.V1 = Triangle.V2;
		Triangle.V2 = t;
	}

	if ( Triangle.V2.X > Triangle.V3.X ){
		VERTEX_DOT t = Triangle.V2;
		Triangle.V2 = Triangle.V3;
		Triangle.V3 = t;
	}
	
	if ( Triangle.V1.X > Triangle.V2.X ){
		VERTEX_DOT t = Triangle.V1;
		Triangle.V1 = Triangle.V2;
		Triangle.V2 = t;
	}

	if ( Triangle.V2.X > Triangle.V3.X ){
		VERTEX_DOT t = Triangle.V2;
		Triangle.V2 = Triangle.V3;
		Triangle.V3 = t;
	}

	if ( Triangle.V1.X == Triangle.V2.X && Triangle.V1.Y > Triangle.V2.Y ){
		VERTEX_DOT t = Triangle.V1;
		Triangle.V1 = Triangle.V2;
		Triangle.V2 = t;
	}

	if ( Triangle.V2.X == Triangle.V3.X && Triangle.V2.Y > Triangle.V3.Y ){
		VERTEX_DOT t = Triangle.V2;
		Triangle.V2 = Triangle.V3;
		Triangle.V3 = t;
	}

	double inclination, offset;
	inclination = (Triangle.V1.Y - Triangle.V3.Y) / (Triangle.V1.X - Triangle.V3.X);
	offset = -(Triangle.V1.X * inclination) + Triangle.V1.Y;
	if ( !(Triangle.V2.Y >= Triangle.V2.X*inclination + offset ) ){
		IsReversedTriangle = true;
	} else {
		IsReversedTriangle = false;
	}

	//ColLine[0] = new clsCollisionLine(Triangle.V1.X, Triangle.V1.Y, Triangle.V2.X, Triangle.V2.Y);
	//ColLine[1] = new clsCollisionLine(Triangle.V2.X, Triangle.V2.Y, Triangle.V3.X, Triangle.V3.Y);
	//ColLine[2] = new clsCollisionLine(Triangle.V3.X, Triangle.V3.Y, Triangle.V1.X, Triangle.V1.Y);
}

CollisionTriangle::~CollisionTriangle()
{
	//delete ColLine[0];
	//delete ColLine[1];
	//delete ColLine[2];

	//ColLine[0] = 0;
	//ColLine[1] = 0;
	//ColLine[2] = 0;
}

bool CollisionTriangle::IsBoundingWith(float PointX, float PointY)
{
	CollisionDot dot(PointX, PointY);
	return IsBoundingWith((Collision*)&dot);
}

bool CollisionTriangle::IsBoundingWith(Collision *OppositeCollision)
{
	switch ( OppositeCollision->Type ){
		case DOT:
		case LINE:
			return OppositeCollision->IsBoundingWith((Collision*)this);
			break;
		case TRIANGLE:
			{
				CollisionTriangle *coltri = (CollisionTriangle*)OppositeCollision;
				{
					CollisionLine colLineA(coltri->Triangle.V1.X, coltri->Triangle.V1.Y,
						coltri->Triangle.V2.X, coltri->Triangle.V2.Y);
					CollisionLine colLineB(coltri->Triangle.V2.X, coltri->Triangle.V2.Y,
						coltri->Triangle.V3.X, coltri->Triangle.V3.Y);
					CollisionLine colLineC(coltri->Triangle.V3.X, coltri->Triangle.V3.Y,
						coltri->Triangle.V1.X, coltri->Triangle.V1.Y);
					CollisionLine colLineMA(Triangle.V1.X, Triangle.V1.Y,
						Triangle.V2.X, Triangle.V2.Y);
					CollisionLine colLineMB(Triangle.V2.X, Triangle.V2.Y,
						Triangle.V3.X, Triangle.V3.Y);
					CollisionLine colLineMC(Triangle.V3.X, Triangle.V3.Y,
						Triangle.V1.X, Triangle.V1.Y);

					if ( colLineA.IsBoundingWith(&colLineMA) ||
						colLineA.IsBoundingWith(&colLineMB) ||
						colLineA.IsBoundingWith(&colLineMC) ||
						colLineB.IsBoundingWith(&colLineMA) ||
						colLineB.IsBoundingWith(&colLineMB) ||
						colLineB.IsBoundingWith(&colLineMC) ||
						colLineC.IsBoundingWith(&colLineMA) ||
						colLineC.IsBoundingWith(&colLineMB) ||
						colLineC.IsBoundingWith(&colLineMC) )
					{
						return true;
					} else if ( IsBoundingWith(coltri->Triangle.V1.X, coltri->Triangle.V1.Y) ||
						IsBoundingWith(coltri->Triangle.V2.X, coltri->Triangle.V2.Y) ||
						IsBoundingWith(coltri->Triangle.V3.X, coltri->Triangle.V3.Y) ||
						coltri->IsBoundingWith(Triangle.V1.X, Triangle.V2.Y) ||
						coltri->IsBoundingWith(Triangle.V2.X, Triangle.V2.Y) ||
						coltri->IsBoundingWith(Triangle.V3.X, Triangle.V3.Y) )
					{
						return true;
					}
				}
			}
			break;
		case CIRCLE:
			{
				CollisionCircle *colcircle = (CollisionCircle*)OppositeCollision;
				CollisionLine colLineMA(Triangle.V1.X, Triangle.V1.Y,
					Triangle.V2.X, Triangle.V2.Y);
				CollisionLine colLineMB(Triangle.V2.X, Triangle.V2.Y,
					Triangle.V3.X, Triangle.V3.Y);
				CollisionLine colLineMC(Triangle.V3.X, Triangle.V3.Y,
					Triangle.V1.X, Triangle.V1.Y);

				if ( colLineMA.IsBoundingWith(colcircle) ||
					colLineMB.IsBoundingWith(colcircle) ||
					colLineMC.IsBoundingWith(colcircle) )
				{
					return true;
				} else if ( 
					colcircle->IsBoundingWith(Triangle.V1.X, Triangle.V1.Y) ||
					colcircle->IsBoundingWith(Triangle.V2.X, Triangle.V2.Y) ||
					colcircle->IsBoundingWith(Triangle.V3.X, Triangle.V3.Y) )
				{
					return true;
				} else if (
					IsBoundingWith(
					colcircle->Position.X - colcircle->Radius,
					colcircle->Position.Y) )
						return true;
			}
			break;
		default:break;
	}

	return false;
}




CollisionCircle::CollisionCircle(VECTOR2 Position, VEC Radius)
{
	this->Position = Position;
	this->Radius = Radius;
	this->Type = CIRCLE;
}

CollisionCircle::CollisionCircle(VEC PositionX, VEC PositionY, VEC Radius)
{
	this->Position.X = PositionX;
	this->Position.Y = PositionY;
	this->Radius = Radius;
	this->Type = CIRCLE;
}

CollisionCircle::~CollisionCircle()
{
}

bool CollisionCircle::IsBoundingWith(float PointX, float PointY)
{
	CollisionDot Dot(PointX, PointY);
	return Dot.IsBoundingWith((Collision*)this);
}

bool CollisionCircle::IsBoundingWith(Collision *OppositeCollision)
{
	switch ( OppositeCollision->Type ){
		case DOT:
		case LINE:
		case TRIANGLE:
			return OppositeCollision->IsBoundingWith((Collision*)this);
			break;
		case CIRCLE:
			{
				CollisionCircle *colcircle = (CollisionCircle*)OppositeCollision;
				/*colcircle->Position.X*colcircle->Position.X
					+colcircle->Position.Y*colcircle->Position.Y*/
				if ( 
					(colcircle->Position.X - Position.X)*(colcircle->Position.X - Position.X)
					+(colcircle->Position.Y - Position.Y)*(colcircle->Position.Y - Position.Y)
					<= (colcircle->Radius + Radius)*(colcircle->Radius + Radius) )
				{
					return true;
				}
			}
			break;
		default:break;
	}

	return false;
}


bool CollisionBuffer::SetSize(int Width, int Height)
{
	if ( Width  < 0 ) return false;
	if ( Height < 0 ) return false;
	this->Width = Width;
	this->Height = Height;

	if ( Buffer.Arr ) delete[] Buffer.Arr;
	Buffer.Arr = new BYTE[(Width*Height+7)/8];
	memset(Buffer.Arr, 0, (Width*Height+7)/8);
	return true;
}

void CollisionBuffer::SetBuffer(BYTE *Buffer)
{
	int x, y;
	for ( y=0; y < Height; y++ )
		for ( x=0; x < Width; x++ )
		{
			if ( Buffer[x+y*Width] )
			{
				this->Buffer.Arr[(x+y*Width)/8] |= 0x01 << ((x+y*Width) % 8);
			} else 
			{
				this->Buffer.Arr[(x+y*Width)/8] &= ~(0x01 << ((x+y*Width) % 8));
			}
		}
}

bool CollisionBuffer::SetFromSurface(IDirect3DSurface9 *Surface, int TargetWidth, int TargetHeight)
{
	if ( !Surface ) return false;

	D3DLOCKED_RECT locked;
	D3DSURFACE_DESC desc;

	if ( !SetSize(TargetWidth, TargetHeight) ) return false;
	if ( FAILED(Surface->GetDesc(&desc)) ) return false;
	if ( desc.Format != D3DFMT_A8R8G8B8 ) return false;

	if ( FAILED(Surface->LockRect(&locked, NULL, D3DLOCK_READONLY)) ) return false;
	for ( int y=0; y < (int)Height; y++ )
	{
		for ( int x=0; x < (int)Width; x++ )
		{
			if ( *((BYTE*)locked.pBits 
				+ (int)(x*(float)desc.Width/TargetWidth)*4
				+ (int)(y*(float)desc.Height/TargetHeight)*locked.Pitch
				+ 3) )
			{
				this->Buffer.Arr[(x+y*Width)/8] |= 0x01 <<  ((x+y*(Width)) % 8);
			} else 
			{
				this->Buffer.Arr[(x+y*Width)/8] &= ~( 0x01 << ((x+y*Width) % 8) );
			}

		}
	}
	//hRegionCollision = ComponentManager::GetAllocRegionFromBuffer(bufbyte.Arr, TextureFrameBorder.ImageDesc.Width, TextureFrameBorder.ImageDesc.Height);
	//SetWindowRgn(EngineManager::hWnd, hRegionCollision, false);
	Surface->UnlockRect();
	return true;
}

bool CollisionBuffer::IsCollisionWithDot(int X, int Y)
{
	if ( !Buffer.Arr ) return false;
	if ( X >= Width || Y >= Height || X < 0 || Y < 0 ) return false;
	if ( Buffer.Arr[(X+Y*Width)/8] & ( 1 << ((X+Y*Width)%8) ) ) return true;
	else return false;
}

/*HRGN clsCollisionBuffer::GetAllocRegion()
{
	UnwindArray<BYTE> aua;
	aua.Arr = new BYTE[Width*Height];
	for ( int y=0; y < Height; y++ )
		for ( int x=0; x < Width; x++ )
			aua.Arr[x+y*Width] = 
				0 != (Buffer.Arr[(x+y*Width)/8] & ( 1 << ((x+y*Width)%8) ));
	HRGN hregion = ComponentManager::GetAllocRegionFromBuffer(aua.Arr, Width, Height);
	//SetWindowRgn(EngineManager::hWnd, hRegionCollision, false);
	return hregion;
}*/

void CollisionBuffer::Uninit()
{
	if ( Buffer.Arr ) delete Buffer.Arr;
	Width = 0;
	Height = 0;
}





