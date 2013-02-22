#include "StdAfx.hpp"
using namespace Mgine;



void MGINE_SOUND_DESC::Uninit()
{
	Format = SOUND_FORMAT_UNKNOWN;
	if ( Buffer ) delete Buffer;
	Buffer = NULL;
	memset(&Wave, 0, sizeof(Wave));
}



void EFFECT_CONSTANT::Uninit()
{
	if ( Data ) delete[] Data;
	Data = 0;
	Cb = 0;
	Type = (EFFECT_VALUE_TYPE)0;
	memset(Ident, 0, 128);
}



void Mgine::VERTEX_DOT::Default()
{
	Y = X = 0;
	ColorDiffuse = 0xffffffff;
	/*Rhw = 1;*/
	Z = (decltype(X))0.0;

	U = 0;
	V = 0;
}

void Mgine::VERTEX_DOT::Transform( const D3DXMATRIXA16 & Matrix )
{
	D3DXVECTOR4 resultvec;
	D3DXVECTOR4 vec;
	vec.x = X;
	vec.y = Y;
	vec.z = Z;
	vec.w = 1;
	D3DXVec4Transform(&resultvec, &vec, &Matrix);
	X = resultvec.x/resultvec.w;
	Y = resultvec.y/resultvec.w;
	Z = resultvec.z/resultvec.w;
}
void VERTEX_DOT::AdjustRadian(VEC Radian)
{
	AdjustRadian(Radian, 0, 0);
}
void VERTEX_DOT::AdjustRadian(VEC Radian, VEC CenterX, VEC CenterY)
{
// 	VEC theta = MF::Theta(X - CenterX, Y - CenterY);
// 	VEC distance = MF::Distance(X - CenterX, Y - CenterY);
// 
// 	X = (VEC)(cos((double)theta+Radian)*distance + CenterX);
// 	Y = (VEC)(sin((double)theta+Radian)*distance + CenterY);
	VEC costheta = cos(Radian),
		sintheta = sin(Radian);
	X = (X - CenterX)*costheta - (Y - CenterY)*sintheta + CenterX;
	Y = (X - CenterX)*sintheta - (Y - CenterY)*costheta + CenterY;
}



void Mgine::VERTEX_LINE::Transform( const D3DXMATRIXA16 & Matrix )
{
	V1.Transform(Matrix);
	V2.Transform(Matrix);
}

void Mgine::VERTEX_TRIANGLE::Transform( const D3DXMATRIXA16 & Matrix )
{
	V1.Transform(Matrix);
	V2.Transform(Matrix);
	V3.Transform(Matrix);
}

void Mgine::VERTEX_TEXTURE::Transform( const D3DXMATRIXA16 & Matrix )
{
	V1.Transform(Matrix);
	V2.Transform(Matrix);
	V3.Transform(Matrix);
	V4.Transform(Matrix);
}
