#include "StdAfx.hpp"
#include "Renderer.h"
#include "Function.h"
using namespace Mgine;
using namespace Util;

bool					Renderer::IsBlur;
bool					Renderer::IsInited;
bool					Renderer::IsDeviceLost;

D3DCAPS9				Renderer::DeviceCaps;
D3DDISPLAYMODE			Renderer::DisplayMode;

PRIM_TYPE			Renderer::CurrentPrimType;
IDirect3DVertexBuffer9	*Renderer::gVertexBuffer;
IDirect3DIndexBuffer9	*Renderer::gIndexBuffer;
LPD3DXLINE				Renderer::gLineBatch;

Effect				Renderer::EffectBlur;

VEC					Renderer::ProjectionOffsetX;
VEC					Renderer::ProjectionOffsetY;
VEC					Renderer::PrimOffsetX;
VEC					Renderer::PrimOffsetY;

D3DVIEWPORT9			Renderer::Viewport;
DWORD					Renderer::ResolutionWidth;
DWORD					Renderer::ResolutionHeight;

D3DXMATRIX				Renderer::MatrixWorld;
D3DXMATRIX				Renderer::MatrixView;
D3DXMATRIX				Renderer::MatrixProjection;

RenderTarget			Renderer::TemporaryRenderTargetTexture[4];
RenderTarget			Renderer::TemporaryRenderTargetMovingScene[2];
RenderTarget			Renderer::TemporaryRenderTargetBlur[1];

IDirect3DSurface9		*Renderer::BackBuffer;
IDirect3DSurface9		*Renderer::DefaultDepthStencilSurface;

VERTEX_DOT				Renderer::DrawArray[];
DWORD					Renderer::CurrentDrawIndex;

Renderer::STATE_TEXTUREBATCH		Renderer::StateTextureBatch;
Renderer::STATE_INDEXBUFFER			Renderer::StateIndexBuffer;
Renderer::STATE_STENCILBUFFER		Renderer::StateStencilBuffer;
Renderer::STATE_ALPHASTENCIL		Renderer::StateAlphaStencil;
Renderer::STATE_CACHE				Renderer::StateCache;
Renderer::STATE_RENDERER			Renderer::StateRenderer;


void Renderer::Init()
{
	if ( IsInited ) MTHROW(InvalidStatus, "Already inited");
	MLOG("Begin");

	IsInited = true;
	IsDeviceLost = false;
}

void Renderer::Uninit()
{
	IsDeviceLost = false;
	IsInited = false;
	
	try
	{ 
		StateTextureBatch.Uninit();
		StateIndexBuffer.Uninit();

		RELEASE_OBJECT(gIndexBuffer);
		//gVertexBuffer->Unlock();
		RELEASE_OBJECT(gVertexBuffer);
		RELEASE_OBJECT(gLineBatch);

		if ( BackBuffer ) RELEASE_OBJECT(BackBuffer);
		if ( DefaultDepthStencilSurface ) RELEASE_OBJECT(DefaultDepthStencilSurface);

		for ( int i=0; i < COUNTOF(TemporaryRenderTargetTexture); i++ )
			TemporaryRenderTargetTexture[i].Uninit();
		for ( int i=0; i < COUNTOF(TemporaryRenderTargetMovingScene); i++ )
			TemporaryRenderTargetMovingScene[i].Uninit();
		for ( int i=0; i < COUNTOF(TemporaryRenderTargetBlur); i++ )
			TemporaryRenderTargetBlur[i].Uninit();
	}
	catch ( BaseException & )
	{
		MLOG("Failed");
		throw;
	}
}

void Renderer::OnDeviceLost()
{
	IsDeviceLost = true;
	
	RELEASE_OBJECT(gIndexBuffer);
	RELEASE_OBJECT(gVertexBuffer);
	if ( FAILED(gLineBatch->OnLostDevice()) ) MTHROW(DirectXError, "gLineBatch Lost Failed");

	RELEASE_OBJECT(BackBuffer);
	RELEASE_OBJECT(DefaultDepthStencilSurface); // it is related with backbuffer.
}

void Renderer::OnDeviceReset()
{
	if ( !IsDeviceLost ) return;

	try
	{ 
		ResolutionWidth  = EngineManager::Setting->GetSetting().ResolutionWidth;
		ResolutionHeight = EngineManager::Setting->GetSetting().ResolutionHeight;
		if ( FAILED(gLineBatch->OnResetDevice()) ) MTHROW(DirectXError, "gLineBatch Reset Failed");
		SubOnDeviceCreateIndexBuffer();
		SubOnDeviceCreateVertexBuffer();
	}
	catch ( BaseException & )
	{
		MLOG("Failed");
		throw;
	}
}

void Renderer::OnDeviceStart()
{
	try
	{
		if ( FAILED(EngineManager::Engine->gDevice->SetFVF( DefaultFVF )) )
			MTHROW(DirectXError, "Failed SetFVF");

		if ( FAILED(EngineManager::Engine->gD3D->GetAdapterDisplayMode(0, &DisplayMode)) )
			MTHROW(DirectXError, "GADM");
		if ( FAILED(EngineManager::Engine->gDevice->GetViewport(&Renderer::Viewport)) )
			MTHROW(DirectXError, "GVP");
		if ( FAILED(EngineManager::Engine->gDevice->GetDeviceCaps(&DeviceCaps)) )
			MTHROW(DirectXError, "GDC");
		//if ( FAILED(EngineManager::Engine->gD3D->GetAdapterDisplayMode(0, &DisplayMode)) )
			//throw Exception<DirectXError>(L"%s: Failed to get adapter display mode", __FUNCTIONW__);

		D3DXMatrixIdentity(&Renderer::MatrixWorld);
		D3DXMatrixIdentity(&Renderer::MatrixView);
		D3DXMatrixIdentity(&Renderer::MatrixProjection);

		if ( FAILED(EngineManager::Engine->gDevice->SetTransform(D3DTS_VIEW, &Renderer::GetMatrixView())) )
			MTHROW(DirectXError, "ST");
		if ( FAILED(EngineManager::Engine->gDevice->SetTransform(D3DTS_PROJECTION, &Renderer::GetMatrixProjection())) ) 
			MTHROW(DirectXError, "ST");
		if ( FAILED(EngineManager::Engine->gDevice->SetTransform(D3DTS_TEXTURE0, &Renderer::GetMatrixWorld())) ) 
			MTHROW(DirectXError, "ST");
		if ( FAILED(EngineManager::Engine->gDevice->SetTransform(D3DTS_TEXTURE1, &Renderer::GetMatrixWorld())) ) 
			MTHROW(DirectXError, "ST");
		if ( FAILED(EngineManager::Engine->gDevice->SetTransform(D3DTS_TEXTURE2, &Renderer::GetMatrixWorld())) ) 
			MTHROW(DirectXError, "ST");
		if ( FAILED(EngineManager::Engine->gDevice->SetTransform(D3DTS_TEXTURE3, &Renderer::GetMatrixWorld())) ) 
			MTHROW(DirectXError, "ST");
		if ( FAILED(EngineManager::Engine->gDevice->SetTransform(D3DTS_TEXTURE4, &Renderer::GetMatrixWorld())) ) 
			MTHROW(DirectXError, "ST");
		if ( FAILED(EngineManager::Engine->gDevice->SetTransform(D3DTS_TEXTURE5, &Renderer::GetMatrixWorld())) ) 
			MTHROW(DirectXError, "ST");
		if ( FAILED(EngineManager::Engine->gDevice->SetTransform(D3DTS_TEXTURE6, &Renderer::GetMatrixWorld())) ) 
			MTHROW(DirectXError, "ST");
		if ( FAILED(EngineManager::Engine->gDevice->SetTransform(D3DTS_TEXTURE7, &Renderer::GetMatrixWorld())) ) 
			MTHROW(DirectXError, "ST");
		if ( FAILED(EngineManager::Engine->gDevice->SetTransform(D3DTS_WORLD, &Renderer::GetMatrixWorld())) ) 
			MTHROW(DirectXError, "ST");
		if ( FAILED(EngineManager::Engine->gDevice->SetTransform(D3DTS_WORLD1, &Renderer::GetMatrixWorld())) )
			MTHROW(DirectXError, "ST");
		if ( FAILED(EngineManager::Engine->gDevice->SetTransform(D3DTS_WORLD2, &Renderer::GetMatrixWorld())) ) 
			MTHROW(DirectXError, "ST");
		if ( FAILED(EngineManager::Engine->gDevice->SetTransform(D3DTS_WORLD3, &Renderer::GetMatrixWorld())) ) 
			MTHROW(DirectXError, "ST");


		// I dunno why but it requires equal or more than 1 setrendertarget to backbuffer (and so on)
		// per one ondevicestart()
		// so cancel caching of renderer for one time
		StateCache.IsCachePause = true;
		StateCache.Cache();
		Renderer::SetBlendState(BLEND_STATE(BLEND_STATE::DEFAULT));

		try
		{
			Renderer::SetRawRenderState(D3DRS_POINTSPRITEENABLE, false);

			Renderer::SetRawRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
			Renderer::SetRawRenderState(D3DRS_LIGHTING, false);
			Renderer::SetRawRenderState(D3DRS_STENCILENABLE,	false);

			Renderer::SetStencilState(STENCIL_STATE(STENCIL_STATE::DEFAULT));
		} catch ( BaseException & )
		{
			MTHROW(DirectXError, "Failed to set RenderState");
		}

		Renderer::SetTextureStageState(TEXTURE_STAGE_STATE(TEXTURE_STAGE_STATE::DEFAULT));
		Renderer::SetTexture(0);

		Renderer::ResetRawSamplerState();

		PrepareBackBuffer();
		PrepareDefaultDepthStencilSurface();
		//SetRawDepthStencilSurface(DefaultDepthStencilSurface);
		SetRawDepthStencilSurface(NULL);

		Renderer::SetRawRenderTarget(BackBuffer); 

		Renderer::FinishFrame();
		Renderer::TestHalfPixel();

		for ( int i=0; i < COUNTOF(TemporaryRenderTargetBlur); i++ )
			TemporaryRenderTargetBlur[i].Uninit();
		for ( int i=0; i < COUNTOF(TemporaryRenderTargetTexture); i++ )
			if ( !IsInited ) TemporaryRenderTargetTexture[i].Init(0, GetResolutionWidth(), GetResolutionHeight());
		for ( int i=0; i < COUNTOF(TemporaryRenderTargetMovingScene); i++ )
			TemporaryRenderTargetMovingScene[i].Init(0, GetResolutionWidth(), GetResolutionHeight());

		StateCache.IsCachePause = false;
	}
	catch ( BaseException & )
	{
		MLOG("Failed");
		StateCache.IsCachePause = false;
		throw;
	}
}

void Renderer::SubOnDeviceCreateLine()
{
	RELEASE_OBJECT(gLineBatch);
	if ( FAILED(D3DXCreateLine(EngineManager::Engine->gDevice, &gLineBatch)) )
		MTHROW(DirectXError, "Failed to CreateLine");
}
void Renderer::SubOnDeviceCreateIndexBuffer()
{
	try
	{ 
		RELEASE_OBJECT(gIndexBuffer);
		if ( FAILED(EngineManager::Engine->gDevice->CreateIndexBuffer(
			PrimUnitCount*6*sizeof(WORD),
			D3DUSAGE_WRITEONLY,
			D3DFMT_INDEX16,
			D3DPOOL_DEFAULT,
			&gIndexBuffer,
			0)) 
			) MTHROW(DirectXError, "Failed to buffer");

		WORD *pindices = 0, n = 0;
		if ( FAILED(gIndexBuffer->Lock(0, 0, (void**)&pindices, 0)) )
			MTHROW(DirectXError, "Failed to Lock index buffer");

		for ( int i=0; i < PrimUnitCount/4; i++ )
		{
			*pindices++ = n;
			*pindices++ = n+1;
			*pindices++ = n+2;
			*pindices++ = n+1;
			*pindices++ = n+2;
			*pindices++ = n+3;
			n += 4;
		}

		if ( FAILED(gIndexBuffer->Unlock()) )
		{
			MTHROW(DirectXError, "Failed to Unlock index buffer");
		}
		if ( FAILED(EngineManager::Engine->gDevice->SetIndices(gIndexBuffer)) ) 
		{
			MTHROW(DirectXError, "Failed to SetIndices");
		}
	}
	catch ( BaseException & )
	{ 
		MLOG("Failed");
		throw;
	}
}
void Renderer::SubOnDeviceCreateVertexBuffer()
{
	try
	{ 
		RELEASE_OBJECT(gVertexBuffer);
		if ( FAILED(EngineManager::Engine->gDevice->CreateVertexBuffer(PrimUnitCount*sizeof(VERTEX_DOT), D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC, DefaultFVF, D3DPOOL_DEFAULT, &gVertexBuffer, 0)) )
			MTHROW(DirectXError, "Failed to create vertex buffer");
		if ( FAILED(EngineManager::Engine->gDevice->SetStreamSource(0, gVertexBuffer, 0, sizeof(VERTEX_DOT))) )
			MTHROW(DirectXError, "Failed to set stream source");
	}
	catch ( BaseException & )
	{ 
		MLOG("Failed");
		throw;
	}
}
void Renderer::OnDeviceCreate()
{
	try
	{  
		ResolutionWidth  = EngineManager::Setting->GetSetting().ResolutionWidth;
		ResolutionHeight = EngineManager::Setting->GetSetting().ResolutionHeight;

		SubOnDeviceCreateLine();
		SubOnDeviceCreateIndexBuffer();
		SubOnDeviceCreateVertexBuffer();

		IsDeviceLost = false;
	}
	catch ( BaseException & )
	{
		MLOG("Failed");
		throw;
	}
}

DWORD Renderer::GetResolutionWidth()
{ return ResolutionWidth; }
DWORD Renderer::GetResolutionHeight()
{ return ResolutionHeight; }
IDirect3DSurface9 * Renderer::GetRawBackBuffer()
{ return BackBuffer; }
IDirect3DSurface9 * Renderer::GetRawDefaultDepthStencilSurface()
{ return DefaultDepthStencilSurface; }
D3DDISPLAYMODE const & Renderer::GetRawDisplayMode()
{ return DisplayMode; }
D3DCAPS9 const & Renderer::GetRawDeviceCaps()
{ return DeviceCaps; }
D3DXMATRIX const & Renderer::GetMatrixWorld()
{ return MatrixWorld; }
D3DXMATRIX const & Renderer::GetMatrixView()
{ return MatrixView; }
D3DXMATRIX const & Renderer::GetMatrixProjection()
{ return MatrixProjection; }


IDirect3DBaseTexture9 *Renderer::GetRawTexture()
{ return GetRawTexture(0); }
IDirect3DBaseTexture9 *Renderer::GetRawTexture(int Stage)
{
	if ( !EngineManager::Engine->gDevice ) MTHROW(InvalidOperation, "Device is null");
	if ( Stage < 0 || Stage >= 8 ) MTHROW(OutOfRange, "Stage is out of range");
	return StateCache.Texture[Stage];
}

DWORD Renderer::GetRawTextureStageState(D3DTEXTURESTAGESTATETYPE Type)
{ return GetRawTextureStageState(0, Type); }
DWORD Renderer::GetRawTextureStageState(int Stage, D3DTEXTURESTAGESTATETYPE Type)
{
	if ( !EngineManager::Engine->gDevice ) MTHROW(InvalidOperation, "Device is null");
	if ( Stage < 0 || Stage >= 8 ) MTHROW(OutOfRange, "Stage is out of range");
	if ( Type < 0 || Type >= 40 ) MTHROW(OutOfRange, "Type is out of range");
	return StateCache.TextureStageState[Stage][Type];
}

IDirect3DIndexBuffer9 *Renderer::GetRawIndexBuffer()
{
	if ( !EngineManager::Engine->gDevice ) MTHROW(InvalidOperation, "Device is null");
	return StateCache.IndexBuffer;
}

IDirect3DSurface9 *Renderer::GetRawRenderTarget()
{
	if ( !EngineManager::Engine->gDevice ) MTHROW(InvalidOperation, "Device is null");
	return StateCache.RenderTarget;
}

IDirect3DPixelShader9 *Renderer::GetRawPixelShader()
{
	if ( !EngineManager::Engine->gDevice ) MTHROW(InvalidOperation, "Device is null");
	return StateCache.PixelShader;
}

IDirect3DVertexShader9 *Renderer::GetRawVertexShader()
{
	if ( !EngineManager::Engine->gDevice ) MTHROW(InvalidOperation, "Device is null");
	return StateCache.VertexShader;
}

DWORD Renderer::GetRawRenderState(D3DRENDERSTATETYPE Type)
{
	if ( !EngineManager::Engine->gDevice ) MTHROW(InvalidOperation, "Device is null");
	if ( Type < 0 || Type >= COUNTOF(StateCache.RenderState) ) MTHROW(OutOfRange, "Type is out of range");
	return StateCache.RenderState[Type];
}

DWORD Renderer::GetRawSamplerState(int Sampler, D3DSAMPLERSTATETYPE Type)
{
	if ( !EngineManager::Engine->gDevice ) MTHROW(InvalidOperation, "Device is null");
	if ( Type < 0 || Type >= 20 ) MTHROW(OutOfRange, "Type out of range");
	if ( Sampler < 0 || Sampler >= 8 ) MTHROW(OutOfRange, "Sampler Out of range");
	return StateCache.SamplerState[Sampler][Type];
}

IDirect3DSurface9 *Renderer::GetRawDepthStencilSurface()
{
	if ( !EngineManager::Engine->gDevice ) MTHROW(InvalidOperation, "Device is null");
	return StateCache.DepthStencilSurface;
}

void Renderer::SetRawTexture(IDirect3DBaseTexture9 *Texture)
{
	return SetRawTexture(0, Texture);
}
void Renderer::SetRawTexture(int Stage, IDirect3DBaseTexture9 *Texture)
{
	if ( !EngineManager::Engine->gDevice ) MTHROW(InvalidOperation, "Device is null");
	if ( Stage < 0 || Stage >= 8 ) MTHROW(OutOfRange, "Stage is out of range");

	try
	{ 
		IDirect3DBaseTexture9 *originalTexture = 0;
		originalTexture = GetRawTexture(Stage);

		if ( originalTexture != Texture || StateCache.IsCachePause )
		{
			FlushBatch();
			if ( FAILED(EngineManager::Engine->gDevice->SetTexture(Stage, Texture)) ) MTHROW(DirectXError, "Set Texture %d", Stage);
			else StateCache.Texture[Stage] = Texture;
		}
	}
	catch ( BaseException & )
	{
		MLOG("Failed Stage:%d", Stage);
		throw;
	}
}

void Renderer::SetRawIndexBuffer(IDirect3DIndexBuffer9 *IndexBuffer)
{
	if ( !EngineManager::Engine->gDevice ) MTHROW(InvalidOperation, "Device is null");
	try
	{
		IDirect3DIndexBuffer9 *originalib = 0;
		originalib = GetRawIndexBuffer();
		if ( originalib != IndexBuffer || StateCache.IsCachePause )
		{
			FlushBatch();
			if ( FAILED(EngineManager::Engine->gDevice->SetIndices(IndexBuffer)) ) MTHROW(DirectXError, "Set Index");
			else StateCache.IndexBuffer = IndexBuffer;
		}
	}
	catch ( BaseException & )
	{
		MLOG("Failed");
		throw;
	}

}

void Renderer::SetRawDepthStencilSurface(IDirect3DSurface9 *DepthStencilSurface)
{
	if ( !EngineManager::Engine->gDevice ) MTHROW(InvalidOperation, "Device is null");
	
	try
	{ 
		IDirect3DSurface9 *originalsurface = 0;
		originalsurface = GetRawDepthStencilSurface();
		if ( originalsurface != DepthStencilSurface || StateCache.IsCachePause )
		{
			FlushBatch();
			if ( FAILED(EngineManager::Engine->gDevice->SetDepthStencilSurface(DepthStencilSurface)) ) MTHROW(DirectXError, "Set Depth Stencil");
			else StateCache.DepthStencilSurface = DepthStencilSurface;
		}
	}
	catch ( BaseException & )
	{
		MLOG("Failed");
		throw;
	}
}

void Renderer::SetRawRenderTarget(IDirect3DSurface9 *Surface)
//{ return SetRenderTarget(0, Surface); }
//bool Renderer::SetRenderTarget(int Index, IDirect3DSurface9 *Surface)
{
	if ( !EngineManager::Engine->gDevice ) MTHROW(InvalidOperation, "Device is null");
	if ( !Surface )
	{
		//throw Exception<InvalidParameter>(L"%s: Surface is null", __FUNCTIONW__);
		Surface = BackBuffer;
	}
	//if ( Index < 0 || Index >= StateRenderer.CountRenderTarget ) return false;

	try
	{ 
		IDirect3DSurface9 *originalRenderTarget = 0;
		//if ( !GetRenderTarget(Index, &originalRenderTarget) ) isSucceeded = false;
		originalRenderTarget = GetRawRenderTarget();
		//originalRenderTarget = GetRenderTarget();
		//if ( !originalRenderTarget ) isSucceeded = false;

		D3DSURFACE_DESC descSurface = { };
		Surface->GetDesc(&descSurface);

		if ( originalRenderTarget != Surface || StateCache.IsCachePause )
		{
			FlushBatch();
			//if ( FAILED(EngineManager::Engine->gDevice->SetRenderTarget(Index, Surface)) ) isSucceeded = false;
			if ( FAILED(EngineManager::Engine->gDevice->SetRenderTarget(0, Surface)) ) MTHROW(DirectXError, "SetRT");
			//else StateRenderer.ArrRenderTarget[Index] = Surface;
			else StateCache.RenderTarget = Surface;
			SetProjection(descSurface.Width, descSurface.Height);
		}
		/*for ( DWORD i=1; i < EngineManager::Engine->DeviceCaps.NumSimultaneousRTs; i++ )
		{
			EngineManager::Engine->gDevice->SetRenderTarget(i, 0);
		}*/
	}
	catch ( BaseException & )
	{
		MLOG("Failed");
		throw;
	}
}

void Renderer::SetRawPixelShader(IDirect3DPixelShader9 *PixelShader)
{
	if ( !EngineManager::Engine->gDevice ) MTHROW(DirectXError, "Device is null");

	try
	{ 
		IDirect3DPixelShader9 *originalPixelShader = 0;
		originalPixelShader = GetRawPixelShader();

		if ( originalPixelShader != PixelShader || StateCache.IsCachePause ){
			FlushBatch();
			//StateCache.Cache();
			StateCache.CacheSamplerState();
			ResetRawSamplerState();
			if ( FAILED(EngineManager::Engine->gDevice->SetPixelShader(PixelShader)) ) 
				MTHROW(DirectXError, "SetPS");
			else StateCache.PixelShader = PixelShader;
		}
		//RELEASE_OBJECT(originalPixelShader);
	}
	catch ( BaseException & )
	{
		MLOG("Failed");
		throw;
	}

}

void Renderer::SetRawVertexShader(IDirect3DVertexShader9 *VertexShader)
{
	if ( !EngineManager::Engine->gDevice ) MTHROW(DirectXError, "Device is null");

	try
	{ 
		IDirect3DVertexShader9 *originalVertexShader = 0;
		//if ( VertexShader == NULL ) VertexShader = MgineEffect::EffectDefaultVertexShader.gVertexShader;

		originalVertexShader = GetRawVertexShader();


		if ( originalVertexShader != VertexShader || StateCache.IsCachePause ){
			FlushBatch();
			//StateCache.Cache();
			StateCache.CacheSamplerState();
			ResetRawSamplerState();
			if ( FAILED(EngineManager::Engine->gDevice->SetVertexShader(VertexShader)) )
				MTHROW(DirectXError, "SetVS");
			else StateCache.VertexShader = VertexShader;
		}
		//RELEASE_OBJECT(originalVertexShader);
	}
	catch ( BaseException & )
	{
		MLOG("Failed");
		throw;
	}
}

void Renderer::ResetRawSamplerState()
{
	//if ( !IsInited ) return ;
	if ( !EngineManager::Engine->gDevice ) MTHROW(DirectXError, "Device is null");
	
	DWORD aniso = GetRawDeviceCaps().MaxAnisotropy;
	if ( aniso > 8 ) aniso = 8; // 8 looks large enough

	try
	{ 
		for ( int i=0; i < 8; i++ )
		{
			Renderer::SetRawTexture(i, NULL);
			Renderer::SetRawSamplerState(i, D3DSAMP_ADDRESSU, D3DTADDRESS_BORDER);
			Renderer::SetRawSamplerState(i, D3DSAMP_ADDRESSV, D3DTADDRESS_BORDER);
			Renderer::SetRawSamplerState(i, D3DSAMP_BORDERCOLOR, 0);
			Renderer::SetRawSamplerState(i, D3DSAMP_MAGFILTER, D3DTEXF_POINT); // 확대시 읽는법
			Renderer::SetRawSamplerState(i, D3DSAMP_MINFILTER, D3DTEXF_ANISOTROPIC); // 축소시
			Renderer::SetRawSamplerState(i, D3DSAMP_MIPFILTER, D3DTEXF_NONE); //밉맵시
			//EngineManager::Engine->gDevice->SetSamplerState(i, D3DSAMP_MIPMAPLODBIAS, 0) || (issucceeded = false);
			//EngineManager::Engine->gDevice->SetSamplerState(i, D3DSAMP_MAXMIPLEVEL, 0) || (issucceeded = false);
			Renderer::SetRawSamplerState(i, D3DSAMP_MAXANISOTROPY, aniso);
			//EngineManager::Engine->gDevice->SetSamplerState(i, D3DSAMP_SRGBTEXTURE, 0) || (issucceeded = false);
			//EngineManager::Engine->gDevice->SetSamplerState(i, D3DSAMP_ELEMENTINDEX, 0) || (issucceeded = false);
			//EngineManager::Engine->gDevice->SetSamplerState(i, D3DSAMP_DMAPOFFSET, 256) || (issucceeded = false);

		}
		Renderer::SetSamplerState(SAMPLER_STATE(SAMPLER_STATE::DEFAULT));
	}
	catch ( BaseException & )
	{
		MLOG("Failed");
		throw;
	}
}

void Renderer::SetRawRenderState(D3DRENDERSTATETYPE Type, DWORD Value)
{
	if ( !EngineManager::Engine->gDevice ) MTHROW(InvalidOperation, "Device is null");
	if ( Type < 0 || Type >= COUNTOF(StateCache.RenderState) ) MTHROW(OutOfRange, "Type is out of range");

	DWORD old;

	try
	{ 
		old = GetRawRenderState(Type);
		if ( old != Value || StateCache.IsCachePause ) 
		{
			FlushBatch();
			if ( FAILED(EngineManager::Engine->gDevice->SetRenderState(Type, Value)) )
				MTHROW(DirectXError, "SetRS");
			else StateCache.RenderState[Type] = Value;
		}
	}
	catch ( BaseException & )
	{
		MLOG("Failed");
		throw;
	}
}

void Renderer::SetRawSamplerState(D3DSAMPLERSTATETYPE Type, DWORD Value)
{
	return SetRawSamplerState(0, Type, Value);
}

void Renderer::SetRawSamplerState(int Sampler, D3DSAMPLERSTATETYPE Type, DWORD Value)
{
	if ( !EngineManager::Engine->gDevice ) MTHROW(DirectXError, "Device is null");
	DWORD before = 0;
	//if ( SUCCEEDED(EngineManager::Engine->gDevice->GetSamplerState(Sampler, Type, &before)) ) 
	try
	{ 
		before = GetRawSamplerState(Sampler, Type);
		if ( before != Value || StateCache.IsCachePause )
		{
			FlushBatch();
			if ( FAILED(EngineManager::Engine->gDevice->SetSamplerState(Sampler, Type, Value)) ) 
				MTHROW(DirectXError, "SetSamplerState");
			else StateCache.SamplerState[Sampler][Type] = Value;
		}
	}
	catch ( BaseException & )
	{
		MLOG("Failed Sampler:%d Type:%d Value:%d", Sampler, Type, Value);
		throw;
	}
}

void Renderer::SetRawTextureStageState( D3DTEXTURESTAGESTATETYPE Type, DWORD Value )
{ return SetRawTextureStageState(0, Type, Value); }
void Renderer::SetRawTextureStageState( DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value)
{
	if ( !EngineManager::Engine->gDevice ) MTHROW(InvalidOperation, "Device is null");

	DWORD before = 0;

	try
	{ 
		before = GetRawTextureStageState(Stage, Type);
		if ( before != Value || StateCache.IsCachePause )
		{
			FlushBatch();
			if ( FAILED(EngineManager::Engine->gDevice->SetTextureStageState(Stage, Type, Value)) )
				MTHROW(DirectXError, "SetTSS");
			else StateCache.TextureStageState[Stage][Type] = Value;
		}
	}
	catch ( BaseException & )
	{
		MLOG("Failed");
		throw;
	}

}

void Renderer::SetProjection(DWORD Width, DWORD Height)
{
	// 3D works by Z

	//	↑
	//	│
	//	└─→

	//	┏─→
	//	│
	//	↓

	/*D3DXMATRIX mattmp;
	D3DXMatrixScaling(&MatrixProjection, 1.0f, -1.0f, 1.0f);
	D3DXMatrixTranslation(&mattmp, -0.5f*1*0 + ProjectionOffsetX, Height+0.5f*1*0 - ProjectionOffsetY, 0.0f);
	D3DXMatrixMultiply(&MatrixProjection, &MatrixProjection, &mattmp);
	D3DXMatrixOrthoOffCenterLH(&mattmp, 0, (float)(Width), 0, (float)(Height), Viewport.MinZ, Viewport.MaxZ);
	D3DXMatrixMultiply(&MatrixProjection, &MatrixProjection, &mattmp);
	if FAILED(EngineManager::Engine->gDevice->SetTransform(D3DTS_PROJECTION, &MatrixProjection))
		MTHROW(DirectXError, "SetTransform",  );
	*/

	VECTOR3 vec = {
		Width/2.f,
		Height/2.f,
		(float)-((Height/2.0) / sin(MF::PI/4/2) * cos(MF::PI/4/2) )
	};


	D3DXMATRIXA16 matlookat, matproject, matscale, mattranslate;
	D3DXVECTOR3 eye(vec.X, vec.Y, vec.Z),
		at(vec.X+cos(MF::PI_F*1/2), vec.Y, vec.Z+sin(MF::PI_F*1/2)),
		up(0, -1, 0);

	D3DXMatrixIdentity(&matscale);
	D3DXMatrixIdentity(&mattranslate);
	D3DXMatrixIdentity(&matlookat);
	D3DXMatrixIdentity(&matproject);

	D3DXMatrixLookAtRH(&matlookat, &eye, &at, &up);
	D3DXMatrixPerspectiveFovRH(
		&matproject,
		D3DX_PI/4,
		(float)Width/Height,
		0.1f,
		10000.f);
	D3DXMatrixTranslation(&mattranslate, ProjectionOffsetX/vec.X, -ProjectionOffsetY/vec.Y, 0);
	D3DXMatrixScaling(&matscale, 1.0f, 1.0f, 1.0f);
	
	if FAILED(EngineManager::Engine->gDevice->SetTransform(D3DTS_PROJECTION, &(matlookat * matproject * mattranslate * matscale)))
		MTHROW(DirectXError, "SetTransform");
	MatrixProjection = matlookat * matproject * mattranslate * matscale;
}


void Renderer::BeginTextureBatch(TextureStatic *TextureParam)
{
	return StateTextureBatch.Set(TextureParam);
}

void Renderer::EndTextureBatch()
{
	StateTextureBatch.Uninit();
}

void Renderer::BeginIndexBuffer(IndexBuffer *IndexBufferParam)
{
	return StateIndexBuffer.Set(IndexBufferParam);
}

void Renderer::EndIndexBuffer()
{
	StateIndexBuffer.Uninit();
}

void Renderer::BeginStencilSet(bool IsClear)
{
	return StateStencilBuffer.SetStencil(IsClear);
}

void Renderer::BeginStencilRender()
{
	return StateStencilBuffer.SetRender();
}

void Renderer::EndStencil()
{
	StateStencilBuffer.Uninit();
}

void Renderer::BeginAlphaStencilSet(bool IsClear)
{
	return StateAlphaStencil.SetStencil(IsClear);
}

void Renderer::BeginAlphaStencilRender()
{
	return StateAlphaStencil.SetRender();
}

void Renderer::EndAlphaStencil()
{
	StateAlphaStencil.Uninit();
}

void Renderer::PrepareBackBuffer()
{
	RELEASE_OBJECT(BackBuffer);
	if ( FAILED(EngineManager::Engine->gDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &BackBuffer)) ) 
		MTHROW(DirectXError, "%s: SetBackBuffer");
}

void Renderer::PrepareDefaultDepthStencilSurface()
{
	//RELEASE_OBJECT(DefaultDepthStencilSurface);
	DefaultDepthStencilSurface = NULL;
	try
	{ 
		DefaultDepthStencilSurface = GetRawDepthStencilSurface();
	}
	catch ( BaseException & )
	{
		MLOG("Failed");
		throw;
	}
}

void Renderer::Clear(DWORD Color, bool IsRenderTarget, bool IsStencil, bool IsZBuffer)
{
	DWORD flag = 0;
	if ( IsRenderTarget ) flag |= D3DCLEAR_TARGET;
	if ( IsStencil ) flag |= D3DCLEAR_STENCIL;
	if ( IsZBuffer ) flag |= D3DCLEAR_ZBUFFER;

	try
	{ 
		FlushBatch();
		if ( FAILED(EngineManager::Engine->gDevice->Clear(0, 0, flag, Color, 1.0f, 0)) )
			MTHROW(DirectXError, "%s: Clear");
	}
	catch ( BaseException & )
	{
		MLOG("Failed");
		throw;
	}
}

void Renderer::DrawQuad(VERTEX_TEXTURE *QuadArray, DWORD QuadCount)
{
	return DrawQuad(QuadArray, QuadCount, true, false);
}

void Renderer::DrawQuad(VERTEX_TEXTURE *QuadArray, DWORD QuadCount, bool IsApplyCameraTransform, bool IsTexelToPixelCorrection)
{
	if ( CurrentDrawIndex + QuadCount*4 > PrimUnitCount || CurrentPrimType != PRIM_CUSTOMTRI )
		try
		{ FlushBatch(); }
		catch ( BaseException & )
		{ }
	if ( CurrentDrawIndex + QuadCount*4 > PrimUnitCount )
	{
		DWORD countproceeded = 0;
		int available = 0;
		while ( countproceeded < QuadCount )
		{
			available = QuadCount - countproceeded;
			if ( available > PrimUnitCount ) available = PrimUnitCount;
			DrawQuad(QuadArray+countproceeded, available, IsApplyCameraTransform, IsTexelToPixelCorrection);
			countproceeded += available;
		}
		return;
	}
	CurrentPrimType = PRIM_CUSTOMTRI;

	Camera2D *camera = GetCamera();

	memcpy(DrawArray + CurrentDrawIndex, QuadArray, QuadCount*4*sizeof(VERTEX_DOT));
	if ( (IsApplyCameraTransform && camera) || !IsTexelToPixelCorrection ){
		for ( DWORD i=0; i < QuadCount; i+=1 ){
			VERTEX_TEXTURE *quad = ((VERTEX_TEXTURE*)(DrawArray + CurrentDrawIndex + i));
			if ( IsApplyCameraTransform && camera ) camera->TransformScreenCoord(*quad);
			if ( !IsTexelToPixelCorrection )
			{
				quad->V1.X += Renderer::PrimOffsetX;
				quad->V1.Y += Renderer::PrimOffsetY;
				quad->V2.X += Renderer::PrimOffsetX;
				quad->V2.Y += Renderer::PrimOffsetY;
				quad->V3.X += Renderer::PrimOffsetX;
				quad->V3.Y += Renderer::PrimOffsetY;
				quad->V4.X += Renderer::PrimOffsetX;
				quad->V4.Y += Renderer::PrimOffsetY;
			}
		}
	}
	CurrentDrawIndex += QuadCount*4;

}

void Renderer::DrawLine(VERTEX_LINE *LineArray, DWORD LineCount)
{
	return DrawLine(LineArray, LineCount, true, false);
}

void Renderer::DrawLine(VERTEX_LINE *LineArray, DWORD LineCount, bool IsApplyCameraTransform, bool IsTexelToPixelCorrection)
{
	if ( CurrentDrawIndex + LineCount*2 > PrimUnitCount || CurrentPrimType != PRIM_LINE )
		try
		{ FlushBatch(); }
		catch ( BaseException & )
		{ }

	if ( CurrentDrawIndex + LineCount*2 > PrimUnitCount )
	{
		DWORD countproceeded = 0;
		int available = 0;
		while ( countproceeded < LineCount )
		{
			available = LineCount - countproceeded;
			if ( available > PrimUnitCount ) available = PrimUnitCount;
			DrawLine(LineArray+countproceeded, available, IsApplyCameraTransform, IsTexelToPixelCorrection);
			countproceeded += available;
		}
		return;
	}
	CurrentPrimType = PRIM_LINE;

	Camera2D *camera = GetCamera();
	
	memcpy(DrawArray + CurrentDrawIndex, LineArray, LineCount*2*sizeof(VERTEX_DOT));
	if ( (IsApplyCameraTransform && camera) || !IsTexelToPixelCorrection ){
		for ( DWORD i=0; i < LineCount; i++ ){
			VERTEX_LINE *line = ((VERTEX_LINE*)(DrawArray + CurrentDrawIndex + i*2));
			if ( IsApplyCameraTransform && camera ) camera->TransformScreenCoord(*line);
			if ( !IsTexelToPixelCorrection )
			{
				line->V1.X += Renderer::PrimOffsetX;
				line->V1.Y += Renderer::PrimOffsetY;
				line->V2.X += Renderer::PrimOffsetX;
				line->V2.Y += Renderer::PrimOffsetY;
			}
		}
		
	}
	CurrentDrawIndex += LineCount*2;

}

void Renderer::DrawPoint(VERTEX_DOT *VtArray, DWORD VtCount)
{
	return DrawPoint(VtArray, VtCount, true, false);
}

void Renderer::DrawPoint(VERTEX_DOT *VtArray, DWORD VtCount, bool IsApplyCameraTransform, bool IsTexelToPixelCorrection)
{
	if ( CurrentDrawIndex + VtCount > PrimUnitCount || CurrentPrimType != PRIM_POINT )
		try
		{ FlushBatch(); }
		catch ( BaseException & )
		{ }

	if ( CurrentDrawIndex + VtCount > PrimUnitCount )
	{
		DWORD vtcountproceeded = 0;
		int available = 0;
		while ( vtcountproceeded < VtCount )
		{
			available = VtCount - vtcountproceeded;
			if ( available > PrimUnitCount ) available = PrimUnitCount;
			DrawPoint(VtArray+vtcountproceeded, available, IsApplyCameraTransform, IsTexelToPixelCorrection);
			vtcountproceeded += available;
		}
		return;
	}
	CurrentPrimType = PRIM_POINT;

	Camera2D *camera = GetCamera();
	
	memcpy(DrawArray + CurrentDrawIndex, VtArray, VtCount*sizeof(VERTEX_DOT));
	if ( (IsApplyCameraTransform && camera ) || !IsTexelToPixelCorrection ){
		for ( DWORD i=0; i < VtCount; i+=1 ){
			VERTEX_DOT *pt = ((VERTEX_DOT*)(DrawArray + CurrentDrawIndex + i));
			if ( IsApplyCameraTransform && camera ) camera->TransformScreenCoord(*pt);
			if ( !IsTexelToPixelCorrection )
			{
				pt->X += Renderer::PrimOffsetX;
				pt->Y += Renderer::PrimOffsetY;
			}
		}
		
	}
	CurrentDrawIndex += VtCount;

}

void Renderer::DrawTri(VERTEX_TRIANGLE *TriArray, DWORD TriCount)
{
	return DrawTri(TriArray, TriCount, true, false);
}

void Renderer::DrawTri(VERTEX_TRIANGLE *TriArray, DWORD TriCount, bool IsApplyCameraTransform, bool IsTexelToPixelCorrection)
{
	if ( CurrentDrawIndex + TriCount*3 >= PrimUnitCount || CurrentPrimType != PRIM_TRIPLE )
		try
		{ FlushBatch(); }
		catch ( BaseException & )
		{ }

	if ( CurrentDrawIndex + TriCount*3 >= PrimUnitCount )
	{
		DWORD tricountproceeded = 0;
		int available = 0;
		while ( tricountproceeded < TriCount )
		{ 
			available = TriCount - tricountproceeded;
			if ( available > PrimUnitCount ) available = PrimUnitCount;
			DrawTri(TriArray+tricountproceeded, available, IsApplyCameraTransform, IsTexelToPixelCorrection);
			tricountproceeded += available;
		}
		return;
	}
	CurrentPrimType = PRIM_TRIPLE;
	
	Camera2D *camera = GetCamera();

	memcpy(DrawArray + CurrentDrawIndex, TriArray, TriCount*sizeof(VERTEX_DOT)*3);
	if ( (IsApplyCameraTransform && camera ) || !IsTexelToPixelCorrection ){
		for ( DWORD i=0; i < TriCount; i++ ){
			VERTEX_TRIANGLE *tri= ((VERTEX_TRIANGLE*)(DrawArray + CurrentDrawIndex + i*3));
			if ( IsApplyCameraTransform && camera ) camera->TransformScreenCoord(*tri);
			if ( !IsTexelToPixelCorrection )
			{
				tri->V1.X += Renderer::PrimOffsetX;
				tri->V1.Y += Renderer::PrimOffsetY;
				tri->V2.X += Renderer::PrimOffsetX;
				tri->V2.Y += Renderer::PrimOffsetY;
				tri->V3.X += Renderer::PrimOffsetX;
				tri->V3.Y += Renderer::PrimOffsetY;
			}
		}
	}
	/*
	avoid culling
	for ( DWORD i=0; i < VtCount; i+=3 ){
		VERTEX_TRIANGLE *tri = ((VERTEX_TRIANGLE*)(DrawArray + CurrentDrawIndex + i));
		if ( tri->V1.X > tri->V2.X ){
			VERTEX_DOT t = tri->V1;
			tri->V1 = tri->V2;
			tri->V2 = t;
		}
		if ( tri->V2.Y > tri->V3.Y ){
			VERTEX_DOT t = tri->V2;
			tri->V2 = tri->V3;
			tri->V3 = t;
		}
		if ( tri->V3.X < tri->V1.X ){
			VERTEX_DOT t = tri->V1;
			tri->V1 = tri->V3;
			tri->V3 = t;		
		}


		if ( tri->V1.X > tri->V2.X ){
			VERTEX_DOT t = tri->V1;
			tri->V1 = tri->V2;
			tri->V2 = t;
		}
		if ( tri->V2.Y > tri->V3.Y ){
			VERTEX_DOT t = tri->V2;
			tri->V2 = tri->V3;
			tri->V3 = t;
		}
		if ( tri->V3.X < tri->V1.X ){
			VERTEX_DOT t = tri->V1;
			tri->V1 = tri->V3;
			tri->V3 = t;		
		}

	}*/
	CurrentDrawIndex += TriCount*3;

}

void Renderer::DrawPolygon(D3DXVECTOR2 *Points, int Count, DWORD Color)
{
	try
	{ 
		FlushBatch();
		if ( FAILED(gLineBatch->SetAntialias(true)) ) MTHROW(DirectXError, "Antialias");
		if ( FAILED(gLineBatch->SetWidth(1)) ) MTHROW(DirectXError, "Width");

		if ( SUCCEEDED(gLineBatch->Begin()) ){
			if ( FAILED(gLineBatch->Draw(Points, Count, Color)) ){
				gLineBatch->End();
				MTHROW(DirectXError, "Draw");
			}
			if ( FAILED(gLineBatch->End()) ) MTHROW(InvalidOperation, "End");
		} else MTHROW(DirectXError, "Begin");
	}
	catch ( BaseException & )
	{
		MLOG("Failed");
		throw;
	}
}

void Renderer::FlushBatch()
{
	if ( !CurrentDrawIndex ) return;

	try
	{ 
		void *locked = 0;

		if ( FAILED(gVertexBuffer->Lock(0, CurrentDrawIndex*sizeof(VERTEX_DOT), &locked, D3DLOCK_DISCARD)) ) 
			MTHROW(DirectXError, "VertexBuffer Lock");
		memcpy(locked, DrawArray, CurrentDrawIndex*sizeof(VERTEX_DOT));
		if ( FAILED(gVertexBuffer->Unlock()) )
			MTHROW(DirectXError, "VertexBuffer Unlock");

		switch ( CurrentPrimType )
		{
		case PRIM_POINT:
			if ( FAILED(EngineManager::Engine->gDevice->DrawPrimitive(D3DPT_POINTLIST, 0, CurrentDrawIndex)) ) MTHROW(DirectXError, "POINT");
			break;

		case PRIM_LINE:
			if ( FAILED(EngineManager::Engine->gDevice->DrawPrimitive(D3DPT_LINELIST, 0, CurrentDrawIndex >> 1)) ) MTHROW(DirectXError, "LINE");
			break;

		case PRIM_TRIPLE:
			if ( FAILED(EngineManager::Engine->gDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 0, CurrentDrawIndex/3)) ) MTHROW(DirectXError, "TRIPLE");
			break;

		case PRIM_CUSTOMTRI:
			//if ( FAILED(EngineManager::Engine->gDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, CurrentDrawIndex-2 )) ) isSucceeded = false;
			{
				DWORD countib = 0;
				IDirect3DIndexBuffer9 *ib = 0;
				if ( FAILED(EngineManager::Engine->gDevice->GetIndices(&ib)) ) MTHROW(InvalidOperation, "Get Index");
				if ( ib )
				{
					bool isdefault = ib == gIndexBuffer;
					D3DINDEXBUFFER_DESC desc = { };
					ib->GetDesc(&desc);
					RELEASE_OBJECT(ib);

					if ( isdefault )
					{
						if ( FAILED(EngineManager::Engine->gDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, CurrentDrawIndex, 0, CurrentDrawIndex >> 1)) )
							MTHROW(DirectXError, "Default Index Draw");
					}
					else
					{
						if ( FAILED(EngineManager::Engine->gDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 1, 0, desc.Size/3/sizeof(WORD) )) ) 
							MTHROW(DirectXError, "Not Default Draw Index");
					}
				}
			}
			break;

		default:break;
		}
		CurrentDrawIndex = 0;
	}
	catch (...)
	{
		MLOG("Failed");
		CurrentDrawIndex = 0;
		throw;
	}

}

void Renderer::LoadBlur()
{
	try
	{ 
		for ( int i=0; i < COUNTOF(Renderer::TemporaryRenderTargetBlur); i++ ){
			if ( !Renderer::TemporaryRenderTargetBlur[i].IsInited ){
				Renderer::TemporaryRenderTargetBlur[i].InitParam.IsRenderTarget = true;
				Renderer::TemporaryRenderTargetBlur[i].Init(0, Renderer::GetResolutionWidth(), Renderer::GetResolutionHeight());
			}
		}
		if ( Renderer::EffectBlur.IsInited == false )
			Renderer::EffectBlur.InitPixelShader("FxScreenBlur1");
		//if ( Renderer::TemporaryRenderTargetBlur[0].Drawer.Effect == NULL )
			//Renderer::TemporaryRenderTargetBlur[0].Drawer.Effect = &Renderer::EffectBlur;
	}
	catch ( BaseException & )
	{
		MLOG("Failed");
		throw;
	}
}

void Renderer::UnloadBlur()
{
	try
	{ 
		for ( int i=0; i < COUNTOF(Renderer::TemporaryRenderTargetBlur); i++ ){
			if ( Renderer::TemporaryRenderTargetBlur[i].IsInited ){
				Renderer::TemporaryRenderTargetBlur[i].Uninit();
			}
		}
		Renderer::EffectBlur.Uninit();
	}
	catch ( BaseException & )
	{
		MLOG("Failed");
		throw;
	}
}

void Renderer::ApplyBlur()
{
	try
	{ 
		if ( !IsBlur ) return ; 
		LoadBlur();

		DeviceResourceManager::DefaultVertexShader.Apply();
		Renderer::EffectBlur.Apply();
		Renderer::EffectBlur.SetParameter("fAmount", EngineManager::Setting->GetSetting().BlurFactor);

		QuadDrawer drawer;
		drawer.Init(&Renderer::TemporaryRenderTargetBlur[0]);
		drawer.Effect = &Renderer::EffectBlur;
			//Renderer::TemporaryRenderTargetBlur[0].Drawer.Effect = &Renderer::EffectBlur;
			//Renderer::TemporaryRenderTargetBlur[0].Draw();
		drawer.Draw();
		Renderer::FlushBatch();
		IDirect3DSurface9 *surface = Renderer::TemporaryRenderTargetBlur[0].GetSurface();
		if ( FAILED(D3DXLoadSurfaceFromSurface(surface, 0, 0, Renderer::BackBuffer, 0, 0, D3DX_FILTER_NONE, 0)) )
			MTHROW(DirectXError, "LoadSurfaceFromSurface");
	}
	catch ( BaseException & )
	{
		MLOG("Failed");
		throw;
	}
}

void Renderer::Present()
{
	if ( !IsInited ) MTHROW(InvalidOperation, "Not inited");
	HRESULT hr = 0;
	
	if ( (hr = EngineManager::Engine->gDevice->Present(0, 0, 0, 0)) == D3DERR_DEVICELOST/* || hr == D3DERR_DEVICENOTRESET*/ )
		try
		{ EngineManager::Engine->ReloadDevice(); }
		catch ( BaseException & )
		{
			MLOG("ReloadDevice Failed");
			throw;
		}
}

void Renderer::FinishFrame()
{
	//if ( !SetBlendOp(D3DBLENDOP_ADD) ) isSucceeded = false;
	try
	{ 
		//SetRawBlendStateDefault();
		SetBlendState(BLEND_STATE(BLEND_STATE::DEFAULT));
		SetTexture(NULL);
		SetRenderTarget(NULL);
		SetEffect(NULL);
		ResetRawSamplerState();

		if ( EngineManager::Setting->GetSetting().IsZEnableDefaultly )
		{
			Renderer::SetRawRenderState(D3DRS_ZENABLE, D3DZB_TRUE);
			Renderer::SetRawRenderState(D3DRS_ZWRITEENABLE, true);
		} else 
		{
			Renderer::SetRawRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
			Renderer::SetRawRenderState(D3DRS_ZWRITEENABLE, false);
		}
	}
	catch ( BaseException & )
	{
		MLOG("Failed");
		throw;
	}
}

void Renderer::Begin()
{
	if ( FAILED(EngineManager::Engine->gDevice->BeginScene()) )
		MTHROW(DirectXError, "BeginScene");
}

void Renderer::End()
{
	try
	{ 
		FlushBatch();
		if ( FAILED(EngineManager::Engine->gDevice->EndScene()) )
			MTHROW(DirectXError, "EndScene");
	}
	catch ( BaseException & )
	{
		MLOG("Failed");
		throw;
	}
}

void Renderer::SetBlur(bool IsEnabling)
{
	if ( IsEnabling == IsBlur ) return;

	IsBlur = IsEnabling;
	if ( IsBlur ){
		LoadBlur();
	} else {
		UnloadBlur();
	}
}

void Renderer::TestHalfPixel()
{
	ReverterRawRenderTarget RestoreRenderTarget;
	ReverterBlendState RestoreBlendState;
	ReverterRawTexture RestoreTexture;


	RenderTarget rt;
	TextureDynamic texread, tex;
	
	QuadDrawer drawer;
	VERTEX_DOT dot;

	D3DLOCKED_RECT locked;
	IDirect3DSurface9 *gsurf;

	PrimOffsetX = 0.5f;
	PrimOffsetY = 0.5f;

	ProjectionOffsetX = -0.5f;
	ProjectionOffsetY = -0.5f;

	SetProjection(Renderer::GetResolutionWidth(), Renderer::GetResolutionHeight());

	VERTEX_DOT ver, ver2;
	ver.X = 0;
	ver.Y = 0;
	ver.Z = 0;
	ver2.X = (float)GetResolutionWidth();
	ver2.Y = (float)GetResolutionHeight();
	ver2.Z = 0;
	ver.Transform(MatrixProjection);
	ver2.Transform(MatrixProjection);

	Renderer::SetBlendState(BLEND_STATE(BLEND_STATE::COPY));
	Renderer::SetRawTexture(NULL);

	rt.Init(0x00000000, 3, 3);
	texread.Init(0x00000000, 3, 3);
	SetRawRenderTarget(rt.GetSurface());

	Renderer::Begin();

	tex.Init(0xffffffff, 1, 1);
	drawer.Init(1, 1, tex.GetTexture());
	drawer.RectDest.X = (VEC)1;
	drawer.RectDest.Y = (VEC)1;
	drawer.Draw();
	
	//dot.ColorDiffuse = 0xffffffff;
	//dot.X = (VEC)1.0;
	//dot.Y = (VEC)1.0;
	//Renderer::DrawPoint(&dot, 1, false, false);
	
	FlushBatch();
	RestoreRenderTarget.Restore();

	Renderer::End();

	if ( FAILED(D3DXLoadSurfaceFromSurface(texread.GetSurface(), 0, 0, rt.GetSurface(), 0, 0, D3DX_FILTER_NONE, 0)) )
		MTHROW(DirectXError, "Failed lsfs");
	gsurf = texread.GetSurface();
	if ( FAILED(gsurf->LockRect(&locked, 0, 0)) )
		MTHROW(DirectXError, "Failed gs.l");
	if ( *((int*)((BYTE*)locked.pBits + locked.Pitch*1)+0) )
		ProjectionOffsetX += (VEC)0.5;
	else if ( *((int*)((BYTE*)locked.pBits + locked.Pitch*1)+2) )
		ProjectionOffsetX += (VEC)-0.5;
	
	if ( *((int*)((BYTE*)locked.pBits + locked.Pitch*0)+1) )
		ProjectionOffsetY += (VEC)0.5;
	else if ( *((int*)((BYTE*)locked.pBits + locked.Pitch*2)+1) )
		ProjectionOffsetY += (VEC)-0.5;
	
	if ( FAILED(gsurf->UnlockRect()) )
		MTHROW(DirectXError, "Failed gs.ul");
	/*
	SetRawRenderTarget(rt.GetSurface());

	Renderer::Clear(0, true, false, false);
	Renderer::Begin();
	tex.InitFillColor(0xffffffff, 1, 1);
	drawer.Init(1, 1, tex.GetTexture());
	drawer.RectDest.X = (VEC)1;
	drawer.RectDest.Y = (VEC)1;
	drawer.Draw();
	
	//dot.ColorDiffuse = 0xffffffff;
	//dot.X = (VEC)1.0;
	//dot.Y = (VEC)1.0;
	//Renderer::DrawPoint(&dot, 1, false, false);
	
	FlushBatch();
	RestoreRenderTarget.Restore();

	Renderer::End();

	if ( FAILED(D3DXLoadSurfaceFromSurface(texread.GetSurface(), 0, 0, rt.GetSurface(), 0, 0, D3DX_FILTER_NONE, 0)) )
		MTHROW(DirectXError, "Failed lsfs");
	gsurf = texread.GetSurface();
	if ( FAILED(gsurf->LockRect(&locked, 0, 0)) )
		MTHROW(DirectXError, "Failed gs.l");
	if ( *((int*)((BYTE*)locked.pBits + locked.Pitch*1)+0) )
		ProjectionOffsetX += (VEC)0.5;
	else if ( *((int*)((BYTE*)locked.pBits + locked.Pitch*1)+2) )
		ProjectionOffsetX += (VEC)-0.5;
	
	if ( *((int*)((BYTE*)locked.pBits + locked.Pitch*0)+1) )
		ProjectionOffsetY += (VEC)0.5;
	else if ( *((int*)((BYTE*)locked.pBits + locked.Pitch*2)+1) )
		ProjectionOffsetY += (VEC)-0.5;
	
	if ( FAILED(gsurf->UnlockRect()) )
		MTHROW(DirectXError, "Failed gs.ul");
		*/
}




void Renderer::STATE_TEXTUREBATCH::Set(TextureStatic *TextureParam)
{
	if ( !RestoreTextureBatch.Item ) RestoreTextureBatch = new ReverterTexture();
	Renderer::SetTexture(TextureParam);
}

void Renderer::STATE_TEXTUREBATCH::Uninit()
{
	RestoreTextureBatch.Set(NULL);
}

void Renderer::STATE_INDEXBUFFER::Set(IndexBuffer *IndexBufferParam)
{
	if ( !RestoreIndexBuffer.Item ) RestoreIndexBuffer = new ReverterIndexBuffer();
	
	this->IndexBufferSet = IndexBufferParam;

	try
	{ 
		SetIndexBuffer(IndexBufferParam);
	}
	catch ( BaseException & )
	{
		Uninit();
		MLOG("Failed");
		throw;
	}
}

void Renderer::STATE_INDEXBUFFER::Uninit()
{
	RestoreIndexBuffer.Set(NULL);
}

void Renderer::STATE_STENCILBUFFER::SetStencil(bool IsClear)
{
	try
	{ 
		FlushBatch();

		if ( !RestoreBlendState ) RestoreBlendState = new ReverterBlendState();
		if ( !RestoreDepthStencilSurface ) RestoreDepthStencilSurface = new ReverterRawDepthStencilSurface();
		if ( !RestoreStencilState ) RestoreStencilState = new ReverterStencilState();

		Renderer::SetRawDepthStencilSurface(DefaultDepthStencilSurface);
		if ( IsClear ) Clear(0, false, true, false);

		Renderer::SetStencilState(STENCIL_STATE(STENCIL_STATE::STENCIL_SETTING));
		Renderer::SetBlendState(BLEND_STATE(BLEND_STATE::DRAW_NOTHING));
	}
	catch ( BaseException & )
	{
		MLOG("Failed");
		throw;
	}
}

void Renderer::STATE_STENCILBUFFER::SetRender()
{
	try
	{ 
		FlushBatch();

		if ( RestoreBlendState ) RestoreBlendState.Item->Restore();
		//if ( !RestoreStencilState ) RestoreStencilState = new ReverterStencilState();

		Renderer::SetStencilState(STENCIL_STATE(STENCIL_STATE::STENCIL_MASKING));
	}
	catch ( BaseException & )
	{
		MLOG("Failed");
		throw;
	}
}

void Renderer::STATE_STENCILBUFFER::Uninit()
{
	try
	{ 
		FlushBatch();
		RestoreStencilState.Set(NULL);
		RestoreBlendState.Set(NULL);
		RestoreDepthStencilSurface.Set(NULL);
	}
	catch ( BaseException & )
	{
		MLOG("Failed");
		throw;
	}
}








void Renderer::STATE_ALPHASTENCIL::SetStencil(bool IsClear)
{
	try
	{ 
		FlushBatch();

		bool issucceeded = true;
		if ( !RestoreRenderTarget.Item ) RestoreRenderTarget = new ReverterRawRenderTarget();
		if ( !RestoreBlendState.Item ) RestoreBlendState = new ReverterRawBlendState();

		SetRenderTarget(&Renderer::TemporaryRenderTargetTexture[2]);
		if ( IsClear ) Clear(0, true, false, false);

		//if ( !Renderer::SetBlendStateOverlay() ) issucceeded = false;
	}
	catch ( BaseException & )
	{
		MLOG("Failed");
		throw;
	}

}

void Renderer::STATE_ALPHASTENCIL::SetRender()
{
	try
	{ 
		FlushBatch();

		bool issucceeded = true;
		if ( !RestoreRenderTarget.Item ) RestoreRenderTarget = new ReverterRawRenderTarget();
		if ( !RestoreBlendState.Item ) RestoreBlendState = new ReverterRawBlendState();
		else { RestoreBlendState.Item->Restore(); }

		SetRenderTarget(&Renderer::TemporaryRenderTargetTexture[3]);
		Clear(0, true, false, false);

		this->IsMask = true;
	}
	catch ( BaseException & )
	{
		MLOG("Failed");
		throw;
	}
}

void Renderer::STATE_ALPHASTENCIL::Uninit()
{
	try
	{ 
		FlushBatch();
		if ( this->IsMask )
		{
			SetRenderTarget(&Renderer::TemporaryRenderTargetTexture[2]);
			SetBlendState(BLEND_STATE(BLEND_STATE::ALPHA_STENCIL_SETTING));
			//SetBlendStateAlphaStencil();
			QuadDrawer drawer;
			drawer.Init(&Renderer::TemporaryRenderTargetTexture[3]);
			drawer.Draw();
			if ( RestoreRenderTarget.Item ) RestoreRenderTarget.Item->Restore();
			if ( RestoreBlendState.Item ) RestoreBlendState.Item->Restore();
			drawer.Init(&Renderer::TemporaryRenderTargetTexture[2]);
			drawer.Draw();
			//SetBlendStateAlphaStencilPost();
		}
		RestoreRenderTarget.Set(NULL);
		RestoreBlendState.Set(NULL);
		this->IsMask = false;
	}
	catch ( BaseException & )
	{
		MLOG("Failed");
		throw;
	}
}

void Renderer::STATE_CACHE::CacheRenderTarget()
{
	bool issuc = true;
	/*if ( CountRenderTarget != EngineManager::Engine->DeviceCaps.NumSimultaneousRTs )
	{
		CountRenderTarget = EngineManager::Engine->DeviceCaps.NumSimultaneousRTs;
		ArrRenderTarget = new IDirect3DSurface9*[CountRenderTarget];
	}

	for ( i=0; i < CountRenderTarget; i++ )
	{
		ArrRenderTarget[i] = NULL;
		HRESULT hr = EngineManager::Engine->gDevice->GetRenderTarget(i, &ArrRenderTarget[i]);
		if ( FAILED(hr) && hr != D3DERR_NOTFOUND ) issuc = false;
		if ( ArrRenderTarget[i] ) ArrRenderTarget[i]->Release();
	}*/
	RenderTarget = NULL;
	if ( FAILED(EngineManager::Engine->gDevice->GetRenderTarget(0, &RenderTarget)) ) issuc = false;
	if ( RenderTarget ) RenderTarget->Release();

	if ( !issuc ) MTHROW(InvalidOperation, "Failed");
}
void Renderer::STATE_CACHE::CacheSamplerState()
{
	int i;
	bool issuc = true;

	memset(SamplerState, 0, sizeof(SamplerState));
	memset(TextureStageState, 0, sizeof(TextureStageState));
	for ( i=0; i < 8; i++ )
	{
		Texture[i] = NULL;
		if ( FAILED(EngineManager::Engine->gDevice->GetTexture(i, (IDirect3DBaseTexture9**)&Texture[i])) ) issuc = false;
		if ( Texture[i] ) Texture[i]->Release();

		if ( FAILED(EngineManager::Engine->gDevice->GetSamplerState(i, D3DSAMP_ADDRESSU, &SamplerState[i][D3DSAMP_ADDRESSU])) ) issuc = false;
		if ( FAILED(EngineManager::Engine->gDevice->GetSamplerState(i, D3DSAMP_ADDRESSV, &SamplerState[i][D3DSAMP_ADDRESSV])) ) issuc = false;
		if ( FAILED(EngineManager::Engine->gDevice->GetSamplerState(i, D3DSAMP_BORDERCOLOR, &SamplerState[i][D3DSAMP_BORDERCOLOR])) ) issuc = false;
		if ( FAILED(EngineManager::Engine->gDevice->GetSamplerState(i, D3DSAMP_MIPFILTER, &SamplerState[i][D3DSAMP_MIPFILTER])) ) issuc = false;
		if ( FAILED(EngineManager::Engine->gDevice->GetSamplerState(i, D3DSAMP_MAGFILTER, &SamplerState[i][D3DSAMP_MAGFILTER])) ) issuc = false;
		if ( FAILED(EngineManager::Engine->gDevice->GetSamplerState(i, D3DSAMP_MINFILTER, &SamplerState[i][D3DSAMP_MINFILTER])) ) issuc = false;
		if ( FAILED(EngineManager::Engine->gDevice->GetSamplerState(i, D3DSAMP_MIPMAPLODBIAS, &SamplerState[i][D3DSAMP_MIPMAPLODBIAS])) ) issuc = false;
		if ( FAILED(EngineManager::Engine->gDevice->GetSamplerState(i, D3DSAMP_MAXMIPLEVEL, &SamplerState[i][D3DSAMP_MAXMIPLEVEL])) ) issuc = false;
		if ( FAILED(EngineManager::Engine->gDevice->GetSamplerState(i, D3DSAMP_MAXANISOTROPY, &SamplerState[i][D3DSAMP_MAXANISOTROPY])) ) issuc = false;
		if ( FAILED(EngineManager::Engine->gDevice->GetSamplerState(i, D3DSAMP_SRGBTEXTURE, &SamplerState[i][D3DSAMP_SRGBTEXTURE])) ) issuc = false;
		if ( FAILED(EngineManager::Engine->gDevice->GetSamplerState(i, D3DSAMP_ELEMENTINDEX, &SamplerState[i][D3DSAMP_ELEMENTINDEX])) ) issuc = false;
		if ( FAILED(EngineManager::Engine->gDevice->GetSamplerState(i, D3DSAMP_DMAPOFFSET, &SamplerState[i][D3DSAMP_DMAPOFFSET])) ) issuc = false;

		if ( FAILED(EngineManager::Engine->gDevice->GetTextureStageState(i, D3DTSS_ALPHAARG0, &TextureStageState[i][D3DTSS_ALPHAARG0])) ) issuc = false;
		if ( FAILED(EngineManager::Engine->gDevice->GetTextureStageState(i, D3DTSS_ALPHAARG1, &TextureStageState[i][D3DTSS_ALPHAARG1])) ) issuc = false;
		if ( FAILED(EngineManager::Engine->gDevice->GetTextureStageState(i, D3DTSS_ALPHAARG2, &TextureStageState[i][D3DTSS_ALPHAARG2])) ) issuc = false;
		if ( FAILED(EngineManager::Engine->gDevice->GetTextureStageState(i, D3DTSS_ALPHAOP, &TextureStageState[i][D3DTSS_ALPHAOP])) ) issuc = false;
		if ( FAILED(EngineManager::Engine->gDevice->GetTextureStageState(i, D3DTSS_BUMPENVLOFFSET, &TextureStageState[i][D3DTSS_BUMPENVLOFFSET])) ) issuc = false;
		if ( FAILED(EngineManager::Engine->gDevice->GetTextureStageState(i, D3DTSS_BUMPENVLSCALE, &TextureStageState[i][D3DTSS_BUMPENVLSCALE])) ) issuc = false;
		if ( FAILED(EngineManager::Engine->gDevice->GetTextureStageState(i, D3DTSS_BUMPENVMAT00, &TextureStageState[i][D3DTSS_BUMPENVMAT00])) ) issuc = false;
		if ( FAILED(EngineManager::Engine->gDevice->GetTextureStageState(i, D3DTSS_BUMPENVMAT01, &TextureStageState[i][D3DTSS_BUMPENVMAT01])) ) issuc = false;
		if ( FAILED(EngineManager::Engine->gDevice->GetTextureStageState(i, D3DTSS_BUMPENVMAT10, &TextureStageState[i][D3DTSS_BUMPENVMAT10])) ) issuc = false;
		if ( FAILED(EngineManager::Engine->gDevice->GetTextureStageState(i, D3DTSS_BUMPENVMAT11, &TextureStageState[i][D3DTSS_BUMPENVMAT11])) ) issuc = false;
		if ( FAILED(EngineManager::Engine->gDevice->GetTextureStageState(i, D3DTSS_COLORARG0, &TextureStageState[i][D3DTSS_COLORARG0])) ) issuc = false;
		if ( FAILED(EngineManager::Engine->gDevice->GetTextureStageState(i, D3DTSS_COLORARG1, &TextureStageState[i][D3DTSS_COLORARG1])) ) issuc = false;
		if ( FAILED(EngineManager::Engine->gDevice->GetTextureStageState(i, D3DTSS_COLORARG2, &TextureStageState[i][D3DTSS_COLORARG2])) ) issuc = false;
		if ( FAILED(EngineManager::Engine->gDevice->GetTextureStageState(i, D3DTSS_COLOROP, &TextureStageState[i][D3DTSS_COLOROP])) ) issuc = false;
		if ( FAILED(EngineManager::Engine->gDevice->GetTextureStageState(i, D3DTSS_CONSTANT, &TextureStageState[i][D3DTSS_CONSTANT])) ) issuc = false;
		//if ( FAILED(EngineManager::Engine->gDevice->GetTextureStageState(i, D3DTSS_FORCE_DWORD, &TextureStageState[i) ) issuc = false;
		if ( FAILED(EngineManager::Engine->gDevice->GetTextureStageState(i, D3DTSS_RESULTARG, &TextureStageState[i][D3DTSS_RESULTARG])) ) issuc = false;
		if ( FAILED(EngineManager::Engine->gDevice->GetTextureStageState(i, D3DTSS_TEXCOORDINDEX, &TextureStageState[i][D3DTSS_TEXCOORDINDEX])) ) issuc = false;
		if ( FAILED(EngineManager::Engine->gDevice->GetTextureStageState(i, D3DTSS_TEXTURETRANSFORMFLAGS, &TextureStageState[i][D3DTSS_TEXTURETRANSFORMFLAGS])) ) issuc = false;
	}
	if ( !issuc ) MTHROW(InvalidOperation, "Failed");
}
void Renderer::STATE_CACHE::CacheRenderState()
{
	int i;
	bool issuc = true;

	memset(RenderState, 0, sizeof(RenderState));
	for ( i=7; i <= 9; i++ )
		if ( FAILED(EngineManager::Engine->gDevice->GetRenderState((D3DRENDERSTATETYPE)i, &RenderState[i])) ) issuc = false;
	for ( i=14; i <= 16; i++ )
		if ( FAILED(EngineManager::Engine->gDevice->GetRenderState((D3DRENDERSTATETYPE)i, &RenderState[i])) ) issuc = false;
	for ( i=19; i <= 20; i++ )
		if ( FAILED(EngineManager::Engine->gDevice->GetRenderState((D3DRENDERSTATETYPE)i, &RenderState[i])) ) issuc = false;
	for ( i=22; i <= 29; i++ )
		if ( FAILED(EngineManager::Engine->gDevice->GetRenderState((D3DRENDERSTATETYPE)i, &RenderState[i])) ) issuc = false;
	for ( i=34; i <= 38; i++ )
		if ( FAILED(EngineManager::Engine->gDevice->GetRenderState((D3DRENDERSTATETYPE)i, &RenderState[i])) ) issuc = false;
	for ( i=48; i <= 48; i++ )
		if ( FAILED(EngineManager::Engine->gDevice->GetRenderState((D3DRENDERSTATETYPE)i, &RenderState[i])) ) issuc = false;
	for ( i=52; i <= 60; i++ )
		if ( FAILED(EngineManager::Engine->gDevice->GetRenderState((D3DRENDERSTATETYPE)i, &RenderState[i])) ) issuc = false;
	for ( i=128; i <= 143; i++ )
		if ( FAILED(EngineManager::Engine->gDevice->GetRenderState((D3DRENDERSTATETYPE)i, &RenderState[i])) ) issuc = false;
	for ( i=145; i <= 152; i++ )
		if ( FAILED(EngineManager::Engine->gDevice->GetRenderState((D3DRENDERSTATETYPE)i, &RenderState[i])) ) issuc = false;
	for ( i=154; i <= 163; i++ )
		if ( FAILED(EngineManager::Engine->gDevice->GetRenderState((D3DRENDERSTATETYPE)i, &RenderState[i])) ) issuc = false;
	for ( i=165; i <= 195; i++ )
		if ( FAILED(EngineManager::Engine->gDevice->GetRenderState((D3DRENDERSTATETYPE)i, &RenderState[i])) ) issuc = false;
	for ( i=198; i <= 209; i++ )
		if ( FAILED(EngineManager::Engine->gDevice->GetRenderState((D3DRENDERSTATETYPE)i, &RenderState[i])) ) issuc = false;

	if ( !issuc ) MTHROW(InvalidOperation, "Failed");
}
void Renderer::STATE_CACHE::CacheIndexBuffer()
{
	bool issuc = true;

	IndexBuffer = NULL;
	if ( FAILED(EngineManager::Engine->gDevice->GetIndices(&IndexBuffer)) ) issuc = false;
	if ( IndexBuffer ) IndexBuffer->Release();

	if ( !issuc ) MTHROW(InvalidOperation, "Failed");
}
void Renderer::STATE_CACHE::CacheDepthStencilSurface()
{
	bool issuc = true;
	DepthStencilSurface = NULL;
	if ( FAILED(EngineManager::Engine->gDevice->GetDepthStencilSurface(&DepthStencilSurface)) ) issuc = false;
	
	if ( !issuc ) MTHROW(InvalidOperation, "Failed");
}
void Renderer::STATE_CACHE::CacheShader()
{
	bool issuc = true;

	PixelShader = NULL;
	VertexShader = NULL;

	if ( FAILED(EngineManager::Engine->gDevice->GetPixelShader(&PixelShader)) ) issuc = false;
	if ( FAILED(EngineManager::Engine->gDevice->GetVertexShader(&VertexShader)) ) issuc = false;

	if ( PixelShader ) PixelShader->Release();
	if ( VertexShader ) VertexShader->Release();
	
	if ( !issuc ) MTHROW(InvalidOperation, "Failed");
}
void Renderer::STATE_CACHE::Cache()
{
	try
	{ 
		CacheShader();
		CacheSamplerState();
		CacheRenderTarget();
		CacheRenderState();
		CacheIndexBuffer();
		CacheDepthStencilSurface();
	}
	catch ( BaseException & )
	{
		MTHROW(InvalidOperation, "Failed");
	}
}




TextureStatic * Renderer::GetRenderTarget()
{ return StateRenderer.RenderTarget; }
TextureStatic * Renderer::GetTexture()
{ return StateRenderer.Texture; }
const SAMPLER_STATE& Renderer::GetSamplerState()
{ return StateRenderer.SamplerState; }
const TEXTURE_STAGE_STATE& Renderer::GetTextureStageState()
{ return StateRenderer.TextureStageState; }
const BLEND_STATE& Renderer::GetBlendState()
{ return StateRenderer.BlendState; }
const STENCIL_STATE& Renderer::GetStencilState()
{ return StateRenderer.StencilState; }
IndexBuffer * Renderer::GetIndexBuffer()
{ return StateRenderer.IndexBuffer; }
Effect * Renderer::GetEffect()
{ return StateRenderer.EffectShader; }
Camera2D * Renderer::GetCamera()
{ return StateRenderer.Camera; }

void Renderer::SetRenderTarget( TextureStatic *RenderTarget )
{
	TextureStatic *old = GetRenderTarget();
	if ( old != NULL ) ATHROW(old->IsInited);
	StateRenderer.RenderTarget = RenderTarget;
	if ( RenderTarget ) SetRawRenderTarget(RenderTarget->GetSurface());
	else SetRawRenderTarget(NULL);
}

void Renderer::SetTexture( TextureStatic *Texture )
{
	TextureStatic *old = GetTexture();
	if ( old != NULL ) ATHROW(old->IsInited);
	StateRenderer.Texture = Texture;
	if ( Texture ) SetRawTexture(Texture->GetTexture());
	else SetRawTexture(NULL);
}

void Renderer::SetSamplerState( const SAMPLER_STATE & SamplerState )
{
	StateRenderer.SamplerState = SamplerState;
	StateRenderer.SamplerState.Apply();
}

void Renderer::SetTextureStageState( const TEXTURE_STAGE_STATE & TextureStageState )
{
	StateRenderer.TextureStageState = TextureStageState;
	StateRenderer.TextureStageState.Apply();
}

void Renderer::SetBlendState( const BLEND_STATE & BlendState )
{
	StateRenderer.BlendState = BlendState;
	StateRenderer.BlendState.Apply();
}

void Renderer::SetStencilState( const STENCIL_STATE & StencilState )
{
	StateRenderer.StencilState = StencilState;
	StateRenderer.StencilState.Apply();
}

void Renderer::SetIndexBuffer( IndexBuffer *IndexBufferParam )
{
}

void Renderer::SetEffect( Effect *EffectParam )
{
}

void Renderer::SetCamera(Camera2D *Camera)
{
	if ( StateRenderer.Camera == Camera ) return;
	try
	{ 
		Camera2D *old = GetCamera();
		if ( old ) ATHROW(old->IsInited);
		FlushBatch();
		StateRenderer.Camera = Camera;
		//if ( Camera ) Camera->RaiseSetAsCurrentCamera();

	}
	catch ( BaseException & )
	{
		MLOG("Failed");
		throw;
	}
}
