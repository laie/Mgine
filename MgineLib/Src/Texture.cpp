#include "StdAfx.hpp"
#include "DeviceResource.h"
#include "Renderer.h"
#include "Function.h"
//#include <png.h>
using namespace Mgine;
using namespace Util;

TextureStatic::~TextureStatic()
{
	if ( this == Renderer::GetTexture() ) Renderer::SetTexture(NULL);
	if ( this == Renderer::GetRenderTarget() ) Renderer::SetRenderTarget(NULL);
	DeviceResourceManager::ListTexture().Sub(ListIndex);
	Uninit();
}

void TextureStatic::Init(wchar_t *FileName)
{
	if ( IsInited ) Uninit();
	ATHROW(FileName);
	ATHROW(!InitParam.IsRenderTarget);

	D3DPOOL pool = D3DPOOL_DEFAULT;
	DWORD usage = 0;


	try
	{
		__SetIsInited(true);
		//__SetIsDeviceLost(false);

		RestoreMethod.SetFromFile(FileName);
		InitPost();
		OnDeviceCreate();
	}
	catch (...)
	{
		MLOG("Failed");
		Uninit();
		throw;
	}
	

}

void TextureStatic::Init(COLOR Color, DWORD Width, DWORD Height)
{
	if ( IsInited ) Uninit();

	ImageDesc.Format = SurfaceDesc.Format;
	ImageDesc.Depth = 1;
	ImageDesc.MipLevels = 0;
	ImageDesc.Width = Width; //TextureDesc.Width;
	ImageDesc.Height = Height; //TextureDesc.Height;
	ImageDesc.ImageFileFormat = (D3DXIMAGE_FILEFORMAT)-1;
	ImageDesc.ResourceType = D3DRTYPE_TEXTURE;


	try
	{
		__SetIsInited(true);
		//__SetIsDeviceLost(false);

		SIZE size = { Width, Height };
		RestoreMethod.SetColorFill(Color, size);
		InitPost();
		OnDeviceCreate();
	}
	catch ( ... )
	{
		MLOG("Failed");
		Uninit();
		throw;
	}
}



void TextureStatic::InitPost()
{
	try
	{
		//if ( !SelectedInitParam().IsReliable ) // ParamInit.IsRenderTarget )
		ATHROW(
			(InitParam.IsInteractive && !InitParam.IsReliable) 
			|| !InitParam.IsInteractive
			);
		ATHROW(
			(InitParam.IsRenderTarget && !InitParam.IsReliable)
			|| !InitParam.IsRenderTarget
			);
		SelectedInitParam = InitParam;
		EventPrepareContent();
	} catch (...)
	{
		MLOG("Failed");
		throw;
	}
}

void TextureStatic::Uninit()
{
	RELEASE_OBJECT(gSurface);
	RELEASE_OBJECT(gTexture);

	//Drawer.Uninit();

	__SetIsInited(false);
	//__SetIsDeviceLost(false);
}

void TextureStatic::OnDeviceLost()
{
	if ( !SelectedInitParam().IsReliable )
	{
		RELEASE_OBJECT(gSurface);
		RELEASE_OBJECT(gTexture);
	}
}

void TextureStatic::OnDeviceReset()
{
	try
	{
		if ( !SelectedInitParam().IsReliable )
		{
			OnDeviceCreate();
		} else 
		{
		}
	}
	catch ( BaseException & )
	{
		MLOG("Failed");
		throw;
	}
}

void TextureStatic::OnDeviceCreate()
{
	IDirect3DTexture9 *pTexture = 0;
	IDirect3DSurface9 *pSurface = 0;

	D3DPOOL pool = D3DPOOL_DEFAULT;
	DWORD usage = 0;

	if ( SelectedInitParam().IsReliable )
		pool = D3DPOOL_MANAGED;
	else 
		pool = D3DPOOL_DEFAULT;
	if ( SelectedInitParam().IsInteractive ) 
		usage |= D3DUSAGE_DYNAMIC;

	if ( SelectedInitParam().IsRenderTarget )
	{
		if ( RestoreMethod.Type == RESTORE_METHOD::FromFile )
		{
			MTHROW(InvalidOperation, "RT with file loading method not allowed");
			//ParamInit.IsRenderTarget = false;
		}

	} else 
	{
		if ( SelectedInitParam().IsReliable )
			pool = D3DPOOL_MANAGED;
		else 
			pool = D3DPOOL_DEFAULT;
		if ( SelectedInitParam().IsInteractive ) usage |= D3DUSAGE_DYNAMIC;

	}

	try
	{ 
		switch ( RestoreMethod.Type )
		{
		case RESTORE_METHOD::FromFile:
			if ( SelectedInitParam().IsRenderTarget )
				MTHROW(InvalidOperation, "Render target & FromFile not allowed", __FUNCTIONW__);// impossible
			else
			{
				RELEASE_OBJECT(gTexture);
				RELEASE_OBJECT(gSurface);

				if ( FAILED(D3DXCreateTextureFromFileEx(EngineManager::Engine->gDevice,
					RestoreMethod.FromFileType.Path.Arr,
					D3DX_DEFAULT, D3DX_DEFAULT,
					0,
					usage,
					D3DFMT_A8R8G8B8,
					pool,
					D3DX_DEFAULT, D3DX_DEFAULT,
					0x00000000,
					&ImageDesc, 0, &gTexture))
				) MTHROW(DirectXError, "ff CreateTexureFFE");
				if ( FAILED(gTexture->GetSurfaceLevel(0, &gSurface)) )
					MTHROW(DirectXError, "ff g_tex GetSurfaceLevel");
				if ( FAILED( gSurface->GetDesc(&SurfaceDesc) ) )
					MTHROW(DirectXError, "ff g_suf GetDesc");
			}
			break;
		case RESTORE_METHOD::FillColor:
			if ( SelectedInitParam().IsRenderTarget )
			{
				RELEASE_OBJECT(gTexture);
				RELEASE_OBJECT(gSurface);
				if ( FAILED(EngineManager::Engine->gDevice->CreateRenderTarget(
					RestoreMethod.FillColorType.Size.cx, RestoreMethod.FillColorType.Size.cy,
					D3DFMT_A8R8G8B8,
					!SelectedInitParam().IsInteractive ? EngineManager::Engine->D3dpp.MultiSampleType : D3DMULTISAMPLE_NONE, 
					!SelectedInitParam().IsInteractive ? EngineManager::Engine->D3dpp.MultiSampleQuality : 0,
					SelectedInitParam().IsInteractive, &gSurface, 0)) 
				) MTHROW(DirectXError, "fc CreateRT");
				if ( FAILED(gSurface->GetDesc(&SurfaceDesc)) ) MTHROW(DirectXError, "fc gSurface GetDesc");

				if ( RestoreMethod.FillColorType.Color != 0x00000000 )
				{
					if ( FAILED(EngineManager::Engine->gDevice->ColorFill(gSurface, 0, RestoreMethod.FillColorType.Color)) )
						MTHROW(DirectXError, "fc ColorFill");
				}
			} else {
				RELEASE_OBJECT(gTexture);
				RELEASE_OBJECT(gSurface);
				if ( FAILED( D3DXCreateTexture(
					EngineManager::Engine->gDevice, 
					RestoreMethod.FillColorType.Size.cx, 
					RestoreMethod.FillColorType.Size.cy, 
					0,
					usage, 
					D3DFMT_A8R8G8B8, 
					pool,
					&gTexture))
					) 
					MTHROW(DirectXError, "fc CreateTexture");
				if ( FAILED(gTexture->GetSurfaceLevel(0, &gSurface)) )
					MTHROW(DirectXError, "fc gTexture GetSurfaceLevel");
				if ( FAILED(gSurface->GetDesc(&SurfaceDesc)) )
					MTHROW(DirectXError, "fc gSurface GetDesc");

				if ( RestoreMethod.FillColorType.Color != 0x00000000 )
				{
					if ( SelectedInitParam().IsRenderTarget )
					{
						ATHROW(SUCCEEDED(EngineManager::Engine->gDevice->ColorFill(gSurface, 0, RestoreMethod.FillColorType.Color)));

					} else 
					{
						/*if ( SelectedInitParam().IsInteractive )
						{
							D3DLOCKED_RECT locked;

							ATHROW(SUCCEEDED(gSurface->LockRect(&locked, NULL, NULL)));
					
							for ( int y=0; y < (int)SurfaceDesc.Height; y++ )
								for ( int x=0; x < (int)SurfaceDesc.Width; x++ )
									*((DWORD*)((BYTE*)locked.pBits + y*locked.Pitch) + x) = Color;

							ATHROW(SUCCEEDED(gSurface->UnlockRect()));
						} else
						{
							// Color is not filled
						}*/
						if ( SelectedInitParam().IsInteractive )
						{
							D3DLOCKED_RECT locked;

							ATHROW(SUCCEEDED(gSurface->LockRect(&locked, NULL, NULL)));
					
							for ( int y=0; y < (int)SurfaceDesc.Height; y++ )
								for ( int x=0; x < (int)SurfaceDesc.Width; x++ )
									*((DWORD*)((BYTE*)locked.pBits + y*locked.Pitch) + x) = RestoreMethod.FillColorType.Color;

							ATHROW(SUCCEEDED(gSurface->UnlockRect()));
						} else MTHROW(InvalidOperation, "Unable to fill color", );
					}
					/*if ( ParamInit.IsInteractive )
					{
						D3DLOCKED_RECT locked;
						if ( FAILED(gSurface->LockRect(&locked, NULL, NULL)) ) MTHROW(DirectXError, "gSur Lock");
						for ( int y=0; y < (int)SurfaceDesc.Height; y++ )
							for ( int x=0; x < (int)SurfaceDesc.Width; x++ )
								*((DWORD*)((BYTE*)locked.pBits + y*locked.Pitch) + x) = RestoreMethod.FillColorType.Color;
						if ( FAILED(gSurface->UnlockRect()) ) MTHROW(DirectXError, "gSur Unlock");
					} else 
					{
						// Color is not filled
						MTHROW(InvalidOperation, "fc Color Fill texture is not allowed", __FUNCTIONW__);
					}*/
				}
			}
			break;
		default:
			break;
		}
		EventPrepareContent();
	}
	catch ( BaseException & )
	{
		MLOG("Failed");
		RELEASE_OBJECT(gSurface);
		RELEASE_OBJECT(pTexture);
		Uninit();
		throw;
	}
}

/*int clsTexture::AddEffect(Effect *Effect)
{
	//if ( QueueEffect.Count < 2 )
		//EngineManager::Engine->gDevice->CreateTexture(ImageDesc.Width, TextureDesc.Height, 0, D3DUSAGE_RENDERTARGET, TextureDesc.Format, D3DPOOL_DEFAULT, &gRenderTarget[QueueEffect.Count], 0);
	return QueueEffect.Add(Effect);
}

void clsTexture::SubEffect(int EffectIndex)
{
	QueueEffect.Sub(EffectIndex);
}

void clsTexture::ClearEffect()
{
	QueueEffect.Clear();
}*/

IDirect3DTexture9 *TextureStatic::GetTexture()
{
	if ( !IsInited ) return NULL;

	IDirect3DSurface9 *pSurface = 0;
	//D3DSURFACE_DESC descSurface;

	if ( SelectedInitParam().IsRenderTarget )
	{
		ReverterRawRenderTarget RestoreRenderTarget;
		ReverterRawPixelShader	RestorePixelShader;

		if ( !gTexture )
		{
			if ( FAILED(D3DXCreateTexture(
				EngineManager::Engine->gDevice,
				ImageDesc.Width, ImageDesc.Height,
				0,
				D3DUSAGE_RENDERTARGET,
				D3DFMT_A8R8G8B8,
				D3DPOOL_DEFAULT,
				&gTexture))
				) goto ret;
		}
		if ( FAILED( gTexture->GetSurfaceLevel(0, &pSurface)) ) goto ret;
		//if ( FAILED( pSurface->GetDesc(&descSurface)) ) goto ret;

		D3DXLoadSurfaceFromSurface(pSurface, 0, 0, gSurface, 0, 0, D3DX_DEFAULT, 0);

	} else {
	}
ret:
	
	RELEASE_OBJECT(pSurface);
	return gTexture;
}

IDirect3DSurface9 *TextureStatic::GetSurface()
{
	return gSurface;
}

UINT TextureStatic::GetWidth()
{ return ImageDesc.Width; }
UINT TextureStatic::GetHeight()
{ return ImageDesc.Height; }

void TextureStatic::RESTORE_METHOD::SetFromFile( wchar_t *FileName )
{
	if ( !FileName ) MTHROW(InvalidParameter, "FileName");
	size_t len = wcslen(FileName);

	Type = FromFile;
	FromFileType.Path = new wchar_t[len+1];
	wcscpy_s(FromFileType.Path.Arr, len+1, FileName);
}

void TextureStatic::RESTORE_METHOD::SetColorFill( COLOR Color, SIZE Size )
{
	Type = FillColor;
	this->FillColorType.Color = Color;
	this->FillColorType.Size = Size;
}




void CALC_RADIAN_VERTEX::CmpSet(VEC X, VEC Y, VEC RadianAdditive)
{
	if ( Cmp.X != X
		|| Cmp.Y != Y
		|| Cmp.RadianAdditive != RadianAdditive
		 )
	{
		Theta = MF::Theta(X, Y);
		Distance = MF::Distance(X, Y);
		Cos = (VEC)cos((double)Theta+RadianAdditive);
		Sin = (VEC)sin((double)Theta+RadianAdditive);

		Cmp.X = X;
		Cmp.Y = Y;
		Cmp.RadianAdditive = RadianAdditive;
	}
}

void QuadDrawer::Init(VEC Width, VEC Height, IDirect3DTexture9 *gTexture)
{
	this->gTexture = gTexture;
	Size.X = Width;
	Size.Y = Height;
	Set();
}
void QuadDrawer::Init(VEC Width, VEC Height)
{
	return Init(Width, Height, gTexture);
}
void QuadDrawer::Init(TextureStatic *Texture)
{
	this->gTexture = Texture->GetTexture(); // no, GetTexture should be called just before draw...
	Size.X = (VEC)Texture->GetWidth();
	Size.Y = (VEC)Texture->GetHeight();
	Set();
}

void QuadDrawer::Uninit()
{
	Size.X = 0;
	Size.Y = 0;
	gTexture = NULL;
	Set();
}

void QuadDrawer::Draw()
{
	if ( !IsVisible ) return;
	
	try
	{ 
		ReverterRawTexture		RestoreTexture;
		ReverterRawPixelShader	RestorePixelShader;
		ReverterRawVertexShader RestoreVertexShader;
		ReverterRawSamplerState RestoreSamplerState;

		SetVertex();
		if ( Effect )
		{
			Effect->Apply();
			SetEffectQuadParameter();
		}
		if ( gTexture ) Renderer::SetRawTexture(gTexture);
		if ( TextureAddress != TEXTUREADDRESS_INHERIT )
		{
			Renderer::SetRawSamplerState(D3DSAMP_ADDRESSU, TextureAddress);
			Renderer::SetRawSamplerState(D3DSAMP_ADDRESSV, TextureAddress);
		}
		Renderer::DrawQuad(&Vertex, 1, true, true);
	}
	catch ( BaseException & )
	{
		MLOG("Failed");
		throw;
	}
}

void QuadDrawer::Set()
{
	RectSource.X = RectSource.Y = 0;
	RectSource.Width = Size.X;
	RectSource.Height = Size.Y;

	RectDest.Y = RectDest.X = 0;
	RectDest.Width = Size.X;
	RectDest.Height = Size.Y;

	Origin.X = 0;
	Origin.Y = 0;
	OriginRotation.X = 0;
	OriginRotation.Y = 0;


	Vertex.V1.Z = (VEC)0; /*Vertex.V1.Rhw = 1;*/ Vertex.V1.ColorDiffuse = ColorDiffuse;
	Vertex.V2.Z = (VEC)0; /*Vertex.V2.Rhw = 1;*/ Vertex.V2.ColorDiffuse = ColorDiffuse;
	Vertex.V3.Z = (VEC)0; /*Vertex.V3.Rhw = 1;*/ Vertex.V3.ColorDiffuse = ColorDiffuse;
	Vertex.V4.Z = (VEC)0; /*Vertex.V4.Rhw = 1;*/ Vertex.V4.ColorDiffuse = ColorDiffuse;
}

void QuadDrawer::ApplyColorDiffuse()
{
	Vertex.V1.ColorDiffuse = ColorDiffuse;
	Vertex.V2.ColorDiffuse = ColorDiffuse;
	Vertex.V3.ColorDiffuse = ColorDiffuse;
	Vertex.V4.ColorDiffuse = ColorDiffuse;
}

void QuadDrawer::SetVertex()
{
	return SetVertex((int)Size.X, (int)Size.Y);
}

void QuadDrawer::SetVertex(int ImageWidth, int ImageHeight)
{
	iRectDest.X = (VEC)(RectDest.X);
	iRectDest.Y = (VEC)(RectDest.Y);
	iRectDest.Width = (VEC)(RectDest.Width);
	iRectDest.Height = (VEC)(RectDest.Height);
	
	iRectSource.X = (VEC)(RectSource.X);
	iRectSource.Y = (VEC)(RectSource.Y);
	iRectSource.Width = (VEC)(RectSource.Width);
	iRectSource.Height = (VEC)(RectSource.Height);
	
	if ( Radian )
	{
		Vertex.V1.X = iRectDest.X - Origin.X;
		Vertex.V2.X = iRectDest.X - Origin.X;
		Vertex.V3.X = iRectDest.X - Origin.X + iRectDest.Width;//+0.5f;
		Vertex.V4.X = iRectDest.X - Origin.X + iRectDest.Width;//+0.5f;
		Vertex.V1.Y = iRectDest.Y - Origin.Y + iRectDest.Height;//+0.5f;
		Vertex.V2.Y = iRectDest.Y - Origin.Y;
		Vertex.V3.Y = iRectDest.Y - Origin.Y + iRectDest.Height;//+0.5f;
		Vertex.V4.Y = iRectDest.Y - Origin.Y;

		VertexRadianCalc[0].CmpSet(
			Vertex.V1.X - OriginRotation.X - iRectDest.X,
			Vertex.V1.Y - OriginRotation.Y - iRectDest.Y,
			Radian);
		Vertex.V1.X = VertexRadianCalc[0].Distance*VertexRadianCalc[0].Cos + iRectDest.X - Origin.X+OriginRotation.X;
		Vertex.V1.Y = VertexRadianCalc[0].Distance*VertexRadianCalc[0].Sin + iRectDest.Y - Origin.Y+OriginRotation.Y;

		VertexRadianCalc[1].CmpSet(
			Vertex.V2.X - OriginRotation.X - iRectDest.X,
			Vertex.V2.Y - OriginRotation.Y - iRectDest.Y,
			Radian);
		Vertex.V2.X = VertexRadianCalc[1].Distance*VertexRadianCalc[1].Cos + iRectDest.X - Origin.X+OriginRotation.X;
		Vertex.V2.Y = VertexRadianCalc[1].Distance*VertexRadianCalc[1].Sin + iRectDest.Y - Origin.Y+OriginRotation.Y;

		VertexRadianCalc[2].CmpSet(
			Vertex.V3.X - OriginRotation.X - iRectDest.X,
			Vertex.V3.Y - OriginRotation.Y - iRectDest.Y,
			Radian);
		Vertex.V3.X = VertexRadianCalc[2].Distance*VertexRadianCalc[2].Cos + iRectDest.X - Origin.X+OriginRotation.X;
		Vertex.V3.Y = VertexRadianCalc[2].Distance*VertexRadianCalc[2].Sin + iRectDest.Y - Origin.Y+OriginRotation.Y;

		VertexRadianCalc[3].CmpSet(
			Vertex.V4.X - OriginRotation.X - iRectDest.X,
			Vertex.V4.Y - OriginRotation.Y - iRectDest.Y,
			Radian);
		Vertex.V4.X = VertexRadianCalc[3].Distance*VertexRadianCalc[3].Cos + iRectDest.X - Origin.X+OriginRotation.X;
		Vertex.V4.Y = VertexRadianCalc[3].Distance*VertexRadianCalc[3].Sin + iRectDest.Y - Origin.Y+OriginRotation.Y;
	} else 
	{
		Vertex.V1.X = iRectDest.X - Origin.X;
		Vertex.V2.X = iRectDest.X - Origin.X;
		Vertex.V3.X = iRectDest.X - Origin.X + iRectDest.Width;//+0.5f;
		Vertex.V4.X = iRectDest.X - Origin.X + iRectDest.Width;//+0.5f;
		Vertex.V1.Y = iRectDest.Y - Origin.Y + iRectDest.Height;//+0.5f;
		Vertex.V2.Y = iRectDest.Y - Origin.Y;
		Vertex.V3.Y = iRectDest.Y - Origin.Y + iRectDest.Height;//+0.5f;
		Vertex.V4.Y = iRectDest.Y - Origin.Y;
	}

	/*theta = MF::GetTheta(Vertex.V2.X - OriginRotation.X, Vertex.V2.Y - OriginRotation.y, iRectDest.X, iRectDest.Y);
	distance = MF::GetDistance(Vertex.V2.X - OriginRotation.X, Vertex.V2.Y - OriginRotation.y, iRectDest.X, iRectDest.Y);
	Vertex.V2.X = distance*cosf(theta+Radian) + iRectDest.X - Origin.X+OriginRotation.X;
	Vertex.V2.Y = distance*sinf(theta+Radian) + iRectDest.Y - Origin.y+OriginRotation.y;
	theta = MF::GetTheta(Vertex.V3.X - OriginRotation.X, Vertex.V3.Y - OriginRotation.y, RectDest.X, RectDest.Y);
	distance = MF::GetDistance(Vertex.V3.X - OriginRotation.X, Vertex.V3.Y - OriginRotation.y, iRectDest.X, RectDest.Y);
	Vertex.V3.X = distance*cosf(theta+Radian) + iRectDest.X - Origin.x+OriginRotation.x;
	Vertex.V3.Y = distance*sinf(theta+Radian) + iRectDest.Y - Origin.y+OriginRotation.y;
	theta = MF::GetTheta(Vertex.V4.X - OriginRotation.x, Vertex.V4.Y - OriginRotation.y, RectDest.X, RectDest.Y);
	distance = MF::GetDistance(Vertex.V4.X - OriginRotation.x, Vertex.V4.Y - OriginRotation.y, iRectDest.X, RectDest.Y);
	Vertex.V4.X = distance*cosf(theta+Radian) + iRectDest.X - Origin.x+OriginRotation.x;
	Vertex.V4.Y = distance*sinf(theta+Radian) + iRectDest.Y - Origin.y+OriginRotation.y;*/
	
	
	Vertex.V1.U = Size.X ? iRectSource.X / (Size.X) : 0;							Vertex.V1.V = Size.Y ? (iRectSource.Y + iRectSource.Height) / (Size.Y) : 0;
	Vertex.V2.U = Size.X ? iRectSource.X / (Size.X) : 0;							Vertex.V2.V = Size.Y ? iRectSource.Y / (Size.Y) : 0;
	Vertex.V3.U = Size.X ? (iRectSource.X + iRectSource.Width) / (Size.X) : 0;		Vertex.V3.V = Size.Y ? (iRectSource.Y + iRectSource.Height) / (Size.Y) : 0;
	Vertex.V4.U = Size.X ? (iRectSource.X + iRectSource.Width) / (Size.X) : 0;		Vertex.V4.V = Size.Y ? iRectSource.Y / (Size.Y) : 0;
}

void QuadDrawer::SetEffectQuadParameter()
{
	if ( !Effect ) MTHROW(InvalidParameter, "Effect is null");

	float f[2];
	try
	{ 
		Effect->SetParameter("SamplerA", EFFECT_TEXTURE, gTexture, 0);
		f[0] = (float)(Renderer::GetResolutionWidth() / (RectDest.Width));
		f[1] = (float)(Renderer::GetResolutionHeight() / (RectDest.Height));
		Effect->SetParameter("ViewportRatio", EFFECT_FLOAT_ARRAY, f, 2);
		f[0] = (float)(Size.X / (RectDest.Width));
		f[1] = (float)(Size.Y / (RectDest.Height));
		Effect->SetParameter("TextureRatio", EFFECT_FLOAT_ARRAY, f, 2);
		f[0] = (float)(1.0f / (RectDest.Width));
		f[1] = (float)(1.0f / (RectDest.Height));
		Effect->SetParameter("UnitTexture", EFFECT_FLOAT_ARRAY, f, 2);
		f[0] = (float)((RectDest.X - Origin.X) / Renderer::GetResolutionWidth());
		f[1] = (float)((RectDest.Y - Origin.Y) / Renderer::GetResolutionHeight());
		Effect->SetParameter("Location", EFFECT_FLOAT_ARRAY, f, 2);
	}
	catch ( BaseException & )
	{
		MLOG("Failed");
		throw;
	}
}

