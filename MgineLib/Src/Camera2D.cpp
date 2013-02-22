#include "StdAfx.hpp"
#include "DeviceResource.h"
#include "Renderer.h"

using namespace Mgine;

Camera2D::~Camera2D()
{
	if ( this == Renderer::GetCamera() ) Renderer::SetCamera(NULL);
	Uninit();
}

void Camera2D::Init()
{
	return Init(Renderer::GetResolutionWidth(), Renderer::GetResolutionHeight());
}

void Camera2D::Init(DWORD Width, DWORD Height)
{
	if ( IsInited ) MTHROW(InvalidStatus, "Already Inited");
	/*try
	{ 
		mRenderTarget.Value.ParamInit.IsRenderTarget = true;
		mRenderTarget.Value.InitFillColor(0, Width, Height);
		__SetIsInited(true);
	}
	catch ( BaseException & )
	{
		Uninit();
		MLOG("Failed");
		throw;
	}*/
	__SetIsInited(true);

	VECTOR2 originvec = { };
	OriginRotate = originvec;

	RECTANGLE posrect = RECTANGLE( 0, 0, (VEC)Width, (VEC)Height );
	Position = posrect;

	Radian = 0;

}


void Camera2D::Uninit()
{
	//try { mRenderTarget.Value.Uninit(); } catch ( BaseException & ) {  }
	VECTOR2 originvec = { };
	OriginRotate = originvec;

	RECTANGLE posrect;
	Position = posrect;
	
	Radian = 0;
	D3DXMatrixIdentity(&MatrixTransform);
	
	__SetIsInited(false);

}

/*MGINE_VECTOR2 Camera2D::TransformScreenCoord(MGINE_VECTOR2 Pos)
{
	Pos.X = Position.X
		+ (Pos.X - (mRenderTarget().Drawer.RectDest.X - mRenderTarget().Drawer.Origin.X))
		*(mRenderTarget().Drawer.RectSource.Width/mRenderTarget().Drawer.RectDest.Width);
	Pos.Y = Position.Y
		+ (Pos.Y - (mRenderTarget().Drawer.RectDest.Y - mRenderTarget().Drawer.Origin.Y))
		*(mRenderTarget().Drawer.RectSource.Height/mRenderTarget().Drawer.RectDest.Height);
	return Pos;
}*/

void Camera2D::SetMatrix()
{
	D3DXMATRIXA16 matrotate, mattranslatefirst, mattranslatesecond;
	D3DXMatrixTranslation(&mattranslatefirst,
		-Position().X
		-OriginRotate().X/2
		,
		-Position().Y
		-OriginRotate().Y/2
		,
		0);
	D3DXMatrixRotationZ(&matrotate, Radian);
	D3DXMatrixTranslation(&mattranslatesecond,
		+OriginRotate().X
		+Position().Width/2
		,
		+OriginRotate().Y
		+Position().Height/2
		,
		0);

	D3DXMatrixIdentity(&MatrixTransform);
	MatrixTransform *= mattranslatefirst;
	MatrixTransform *= matrotate;
	MatrixTransform *= mattranslatesecond;
}


void Camera2D::TransformScreenCoord(VERTEX_DOT & Vertex)
{ Vertex.Transform(MatrixTransform); }
void Camera2D::TransformScreenCoord(VERTEX_LINE & Line)
{ Line.Transform(MatrixTransform); }
void Camera2D::TransformScreenCoord(VERTEX_TRIANGLE & Triangle)
{ Triangle.Transform(MatrixTransform); }
void Camera2D::TransformScreenCoord(VERTEX_TEXTURE & Quad)
{ Quad.Transform(MatrixTransform); }

void Camera2D::OnDeviceLost()
{ }
void Camera2D::OnDeviceReset()
{ }
void Camera2D::OnDeviceCreate()
{ }


