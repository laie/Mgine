#include "StdAfx.hpp"
#include "DeviceResource.h"
#include "Renderer.h"

using namespace Mgine;
using namespace Util;

TriList<IndexBuffer*>	*DeviceResourceManager::iListIndexBuffer;
TriList<TextureStatic*>	*DeviceResourceManager::iListTexture;
TriList<Effect*>		*DeviceResourceManager::iListEffect;
TriList<Font*>			*DeviceResourceManager::iListFont;

ID3DXSprite				*DeviceResourceManager::SpriteFont;
Font					DeviceResourceManager::DefaultFont;

Effect					DeviceResourceManager::DefaultVertexShader;
bool					DeviceResourceManager::IsInited;

void DeviceResourceManager::Init()
{
	if ( IsInited ) MTHROW(InvalidStatus, "Already inited");

	MLOG("Begin");

	ListIndexBuffer();
	ListTexture();
	ListEffect();
	ListFont();

	try
	{ 
		if ( !Effect::InitBaseShaderFromFile(EngineManager::Setting->GetSetting().ShaderFilePath) )
			MTHROW(InvalidOperation, "IBSFF");

		HRESULT hr = S_OK;

		RELEASE_OBJECT(SpriteFont);
		if ( FAILED(hr = D3DXCreateSprite(EngineManager::Engine->gDevice, &SpriteFont)) ) 
			MTHROW(DirectXError, "DDXCSPR. hr:%.08x", hr);
		try
		{ DefaultFont.Init(L"Lucida Sans Unicode", 0, 21, FW_NORMAL, true); }
		catch ( BaseException & )
		{ MLOG("DF.I failed", ); throw; }
		
		try
		{ DeviceResourceManager::DefaultVertexShader.InitVertexShader("VsDefault"); }
		catch ( BaseException & )
		{ MLOG("DV.IVS", ); throw; }
		

		IsInited = true;
	}
	catch ( BaseException & )
	{
		MLOG("Failed");
		Uninit();
		throw;
	}
	
	IsInited = true;
}

void DeviceResourceManager::Uninit()
{
	// Uninit is not throwing exceptions
	IsInited = false;

	try
	{
		for ( StepIterator<TextureStatic *> elementtex; ListTexture().Step(elementtex); )
			try
			{ elementtex->Uninit(); }
			catch ( BaseException & )
			{ MLOG("Failed Uninit of texture %d", elementtex.Index); }

		for ( StepIterator<IndexBuffer *> elementidxbuf; ListIndexBuffer().Step(elementidxbuf); )
			try
			{ elementidxbuf->Uninit(); }
			catch ( BaseException & )
			{ MLOG("Failed Uninit of idxbuf %d", elementidxbuf.Index); }
			
		
		try
		{ DefaultVertexShader.Uninit(); }
		catch ( BaseException & )
		{ MLOG("Failed DVS.Un"); }

		for ( StepIterator<Effect *> elementeffect; ListEffect().Step(elementeffect); )
			try
			{ elementeffect->Uninit(); }
			catch ( BaseException & )
			{ MLOG("Failed Uninit of Effect %d", elementeffect.Index); }
		
		try
		{ Effect::UninitBaseShader(); }
		catch ( BaseException & )
		{ MLOG("Failed UBS"); }

		try
		{ DefaultFont.Uninit(); }
		catch ( BaseException & )
		{ MLOG("Failed DF.Uninit"); }

		for ( StepIterator<Font *> elementfont; ListFont().Step(elementfont); )
			try
			{ elementfont->Uninit(); }
			catch ( BaseException & )
			{ MLOG("Failed Uninit of Font %d", elementfont.Index); }

		RELEASE_OBJECT(SpriteFont);
		


	}
	catch (...)
	{
		MLOG("Failed");
		throw;
	}
}

void DeviceResourceManager::OnAppFinish()
{
	if ( iListTexture )		delete iListTexture;
	if ( iListIndexBuffer )	delete iListIndexBuffer;
	if ( iListEffect )		delete iListEffect;
	if ( iListFont )		delete iListFont;
	iListTexture		= NULL;
	iListIndexBuffer	= NULL;
	iListEffect			= NULL;
	iListFont			= NULL;

}

void DeviceResourceManager::OnDeviceLost()
{
	try
	{
		try
		{ 
			for ( StepIterator<TextureStatic *> element; ListTexture().Step(element); )
				element->RaiseDeviceLost();
		}
		catch ( BaseException & )
		{
			MLOG("Failed Lost Texture");
			throw;
		}

		try
		{
			for ( StepIterator<IndexBuffer *> element; ListIndexBuffer().Step(element); )
				element->RaiseDeviceLost();
		}
		catch ( BaseException & )
		{
			MLOG("Failed Lost IndexBuffer");
			throw;
		}

		try
		{
			for ( StepIterator<Effect *> element; ListEffect().Step(element); )
				element->RaiseDeviceLost();
		} catch ( BaseException & )
		{
			MLOG("Failed Lost Effect");
			throw;
		}

		try
		{
			for ( StepIterator<Font *> element; ListFont().Step(element); )
				element->RaiseDeviceLost();
		} catch ( BaseException & )
		{
			MLOG("Failed Lost Font");
			throw;
		}

		try
		{ SpriteFont->OnLostDevice(); }
		catch ( BaseException & )
		{
			MLOG("Failed SF.Lost", );
			throw;
		}
		
	} catch ( BaseException & )
	{
		MLOG("Failed");
		throw;
	}
}

void DeviceResourceManager::OnDeviceReset()
{
	try
	{
		try
		{ 
			for ( StepIterator<TextureStatic *> element; ListTexture().Step(element); )
				element->RaiseDeviceReset();
		}
		catch ( BaseException & )
		{
			MLOG("Failed Reset of Texture", );
			throw;
		}

		try
		{
			for ( StepIterator<IndexBuffer *> element; ListIndexBuffer().Step(element); )
				element->RaiseDeviceReset();
		}
		catch ( BaseException & )
		{
			MLOG("Failed Reset of IndexBuffer", );
			throw;
		}

		try
		{
			for ( StepIterator<Effect *> element; ListEffect().Step(element); )
				element->RaiseDeviceReset();
		} catch ( BaseException & )
		{
			MLOG("Failed Reset of Effect", );
			throw;
		}


		if ( FAILED(SpriteFont->OnResetDevice()) )
			MTHROW(DirectXError, "Failed SF.OnResetDevice");

		try
		{
			for ( StepIterator<Font *> element; ListFont().Step(element); )
				element->RaiseDeviceReset();
		} catch ( BaseException & )
		{
			MLOG("Failed Reset of Font");
			throw;
		}

	} catch ( BaseException & )
	{
		MLOG("Failed");
		throw;
	}
}

void DeviceResourceManager::OnDeviceCreate()
{
	try
	{
		try
		{ 
			for ( StepIterator<TextureStatic *> element; ListTexture().Step(element); )
				element->RaiseDeviceCreate();
		}
		catch ( BaseException & )
		{
			MLOG("%s: Failed Texture", );
			throw;
		}

		try
		{ 
			for ( StepIterator<IndexBuffer *> element; ListIndexBuffer().Step(element); )
				element->RaiseDeviceCreate();
		}
		catch ( BaseException & )
		{
			MLOG("Failed IndexBuffer", );
			throw;
		}

		try
		{
			for ( StepIterator<Effect *> element; ListEffect().Step(element); )
				element->RaiseDeviceCreate();
		} catch ( BaseException & )
		{
			MLOG("Failed Effect", );
			throw;
		}

		RELEASE_OBJECT(SpriteFont);
		
		try
		{
			for ( StepIterator<Font *> element; ListFont().Step(element); )
				element->RaiseDeviceCreate();
		}
		catch ( BaseException & )
		{
			MLOG("Failed Font", );
			throw;
		}

		if ( FAILED(D3DXCreateSprite(EngineManager::Engine->gDevice, &SpriteFont)) ) 
			MTHROW(DirectXError, "Failed DCreateSprite");
	} catch ( BaseException & )
	{
		MLOG("Failed");
		throw;
	}
}

void IDeviceResource::RaiseDeviceLost()
{
	if ( !IsInited ) return;
	ATHROWR(!IsDeviceLost, "Already Lost");
	IsDeviceLost = true;

	OnDeviceLost();
}

void IDeviceResource::RaiseDeviceReset()
{
	if ( !IsInited ) return;
	if ( !IsDeviceLost ) return;
	//ATHROWR(IsDeviceLost, "Already Reset");
	IsDeviceLost = false;

	OnDeviceReset();
}

void IDeviceResource::RaiseDeviceCreate()
{
	if ( !IsInited ) return;
	ATHROWR(IsDeviceLost, "Already Reset");
	IsDeviceLost = false;

	OnDeviceCreate();
}

void SAMPLER_STATE::Apply()
{
	Renderer::SetRawSamplerState(D3DSAMP_ADDRESSU, this->TextureAddress);
	Renderer::SetRawSamplerState(D3DSAMP_ADDRESSV, this->TextureAddress);
	Renderer::SetRawSamplerState(D3DSAMP_ADDRESSW, this->TextureAddress);
	Renderer::SetRawSamplerState(D3DSAMP_MINFILTER, this->Filter);
	Renderer::SetRawSamplerState(D3DSAMP_MAGFILTER, D3DTEXF_POINT); //this->Filter);
	// Mip filter is not used in Mgine
	if ( this->TextureAddress == D3DTADDRESS_BORDER )
		Renderer::SetRawSamplerState(D3DSAMP_BORDERCOLOR, this->BorderColor);
}

void TEXTURE_STAGE_STATE::Apply()
{
	Renderer::SetRawTextureStageState(D3DTSS_COLORARG1, ColorFunc.LeftArg);
	Renderer::SetRawTextureStageState(D3DTSS_COLOROP, ColorFunc.Operator);
	Renderer::SetRawTextureStageState(D3DTSS_COLORARG2, ColorFunc.RightArg);

	Renderer::SetRawTextureStageState(D3DTSS_ALPHAARG1, AlphaFunc.LeftArg);
	Renderer::SetRawTextureStageState(D3DTSS_ALPHAOP, AlphaFunc.Operator);
	Renderer::SetRawTextureStageState(D3DTSS_ALPHAARG2, AlphaFunc.RightArg);

}

void BLEND_STATE::Apply()
{
	Renderer::SetRawRenderState(D3DRS_ALPHABLENDENABLE, true);
	Renderer::SetRawRenderState(D3DRS_SRCBLEND, ColorFunc.OperandSrcMul);
	Renderer::SetRawRenderState(D3DRS_BLENDOP, ColorFunc.Operator);
	Renderer::SetRawRenderState(D3DRS_DESTBLEND, ColorFunc.OperandDestMul);
	if ( AlphaFunc != ColorFunc )
	{
		Renderer::SetRawRenderState(D3DRS_SEPARATEALPHABLENDENABLE, true);
		Renderer::SetRawRenderState(D3DRS_SRCBLENDALPHA, AlphaFunc.OperandSrcMul);				
		Renderer::SetRawRenderState(D3DRS_BLENDOPALPHA, AlphaFunc.Operator);
		Renderer::SetRawRenderState(D3DRS_DESTBLENDALPHA, AlphaFunc.OperandDestMul);
	} else Renderer::SetRawRenderState(D3DRS_SEPARATEALPHABLENDENABLE, false);
}

void STENCIL_STATE::Apply()
{
	//((ref & mask) compareoperator (value & mask)) == true
	// Always
	Renderer::SetRawRenderState(D3DRS_STENCILENABLE,    IsStencilEnable);
	if ( IsStencilEnable )
	{
		Renderer::SetRawRenderState(D3DRS_STENCILFUNC,		StencilPassConditionFunction.PassIf);
		Renderer::SetRawRenderState(D3DRS_STENCILREF,       StencilPassConditionFunction.ReferenceValue);
		// Alpha only?
		Renderer::SetRawRenderState(D3DRS_STENCILMASK,      CmpMask);
		Renderer::SetRawRenderState(D3DRS_STENCILWRITEMASK, WriteMask);

		Renderer::SetRawRenderState(D3DRS_STENCILFAIL,      FailOperation);
		Renderer::SetRawRenderState(D3DRS_STENCILPASS,      PassOperation);
	}
}

void LineDrawer::Clear()
{
	ArrLine.clear();
	ArrLineProperty.clear();
	return;
}

void LineDrawer::Draw()
{
	if ( !IsVisible ) return ;

	assert(ArrLineProperty.size() == ArrLine.size());

	DWORD i;
	for ( i=0; i < ArrLineProperty.size(); i++ )
	{
		if ( ArrLineProperty[i].IsDefaultColor )
		{
			ArrLine[i].V1.ColorDiffuse = DefaultColor;
			ArrLine[i].V2.ColorDiffuse = DefaultColor;
		} else {
			ArrLine[i].V1.ColorDiffuse = ArrLineProperty[i].Color1;
			ArrLine[i].V2.ColorDiffuse = ArrLineProperty[i].Color2;
		}

		ArrLine[i].V2.X = ArrLineProperty[i].X2;
		ArrLine[i].V2.Y = ArrLineProperty[i].Y2;
		if ( ArrLineProperty[i].IsPolygonPrev )
		{
			if ( !i )
			{
				ArrLine[i].V1.X = ArrLine[i].V2.X;
				ArrLine[i].V1.Y = ArrLine[i].V2.Y;
				ArrLine[i].V1.ColorDiffuse = DefaultColor;
			} else 
			{
				ArrLine[i].V1.X = ArrLine[i-1].V2.X;
				ArrLine[i].V1.Y = ArrLine[i-1].V2.Y;
				ArrLine[i].V1.ColorDiffuse = ArrLine[i-1].V2.ColorDiffuse;
			}
		} else 
		{
			ArrLine[i].V1.X = ArrLineProperty[i].X1;
			ArrLine[i].V1.Y = ArrLineProperty[i].Y1;
		}
		if ( Radian )
		{
			D3DXMATRIX matrot;
			D3DXMatrixTransformation2D(&matrot, NULL, NULL, NULL, &(const D3DXVECTOR2&)Origin, Radian, NULL);
			//D3DXVec3Transform
			ArrLine[i].Transform(matrot);

			/*QueueLine[i].V2.X += Position.X - Origin.X;
			QueueLine[i].V2.Y += Position.Y - Origin.Y;

			QueueLine[i].V2.X += Position.X - Origin.X;
			QueueLine[i].V2.Y += Position.Y - Origin.Y;
			


			QueueCalc[i*2].CmpSet(
				QueueLine[i].V1.X - OriginRotation.X - Position.X,
				QueueLine[i].V1.Y - OriginRotation.Y - Position.Y,
				Radian);
			QueueLine[i].V1.X = QueueCalc[i*2].Distance*QueueCalc[i*2].Cos + Position.X - Origin.X+OriginRotation.X;
			QueueLine[i].V1.Y = QueueCalc[i*2].Distance*QueueCalc[i*2].Sin + Position.Y - Origin.Y+OriginRotation.Y;

			QueueCalc[i*2+1].CmpSet(
				QueueLine[i].V2.X - OriginRotation.X - Position.X,
				QueueLine[i].V2.Y - OriginRotation.Y - Position.Y,
				Radian);
			QueueLine[i].V2.X = QueueCalc[i*2+1].Distance*QueueCalc[i*2+1].Cos + Position.X - Origin.X+OriginRotation.X;
			QueueLine[i].V2.Y = QueueCalc[i*2+1].Distance*QueueCalc[i*2+1].Sin + Position.Y - Origin.Y+OriginRotation.Y;
			*/
		} else 
		{
			ArrLine[i].V2.X += Position.X - Origin.X;
			ArrLine[i].V2.Y += Position.Y - Origin.Y;

			ArrLine[i].V2.X += Position.X - Origin.X;
			ArrLine[i].V2.Y += Position.Y - Origin.Y;
		}
	}

	Renderer::DrawLine(ArrLine.data(), ArrLine.size());
}

LineDrawer::LINE_PROPERTY* LineDrawer::operator[]( int Index )
{
	ATHROW(Index >= 0 && Index < (int)ArrLine.size());
	return &ArrLineProperty[Index];
}

void LineDrawer::AddLine( VEC X1, VEC Y1, VEC X2, VEC Y2, COLOR ColorDiffuse1, COLOR ColorDiffuse2, bool IsDefaultColor, bool IsPolygonPrev )
{
	LINE_PROPERTY lineprop;
	lineprop.X1 = X1;
	lineprop.Y1 = Y1;
	lineprop.X2 = X2;
	lineprop.Y2 = Y2;
	lineprop.Color1 = ColorDiffuse1;
	lineprop.Color2 = ColorDiffuse2;
	lineprop.IsDefaultColor = IsDefaultColor;
	lineprop.IsPolygonPrev = IsPolygonPrev;
	ArrLineProperty.push_back(lineprop);
	
	VERTEX_LINE tmp = { };
	ArrLine.push_back(tmp); // for just allocate memory there
	//CALC_RADIAN_VERTEX calcradiantmp = { 0, };
	//QueueCalc.Add(calcradiantmp);
	//QueueCalc.Add(calcradiantmp);

}



void RectBorderDrawer::Draw()
{
	BorderQuad[0].Draw();
	BorderQuad[1].Draw();
	BorderQuad[2].Draw();
	BorderQuad[3].Draw();
}

void Mgine::RectBorderDrawer::ApplyShape()
{
	BorderQuad[0].RectDest.X = Position().X;
	BorderQuad[0].RectDest.Y = Position().Y;
	BorderQuad[0].RectDest.Width  = Padding;
	BorderQuad[0].RectDest.Height = Position().Height;
	BorderQuad[1].RectDest.X = Position().X+Padding;
	BorderQuad[1].RectDest.Y = Position().Y;
	BorderQuad[1].RectDest.Width  = Position().Width-Padding*2;
	BorderQuad[1].RectDest.Height = Padding;
	BorderQuad[2].RectDest.X = Position().X+Position().Width-Padding;
	BorderQuad[2].RectDest.Y = Position().Y;
	BorderQuad[2].RectDest.Width  = Padding;
	BorderQuad[2].RectDest.Height = Position().Height;
	BorderQuad[3].RectDest.X = Position().X+Padding;
	BorderQuad[3].RectDest.Y = Position().Y+Position().Height-Padding;
	BorderQuad[3].RectDest.Width  = Position().Width-Padding*2;
	BorderQuad[3].RectDest.Height = Padding;
}

ReverterTexture::ReverterTexture()
{ OriginalTexture = Renderer::GetTexture(); }
ReverterTexture::~ReverterTexture()
{ Restore(); }
void ReverterTexture::Restore()
{ Renderer::SetTexture(OriginalTexture); }

ReverterRawTexture::ReverterRawTexture()
{
	for ( int i=0; i < 8; i++ )
	{
		ArrOriginalTexture[i] = NULL;
		ArrOriginalTexture[i] = Renderer::GetRawTexture(i);
		if ( ArrOriginalTexture[i] ) ArrOriginalTexture[i]->AddRef();
	}
}
ReverterRawTexture::~ReverterRawTexture()
{
	Restore();
	for ( int i=0; i < 8; i++ )
		RELEASE_OBJECT(ArrOriginalTexture[i]);
}
void ReverterRawTexture::Restore()
{ for ( int i=0; i < 8; i++ ) Renderer::SetRawTexture(i, ArrOriginalTexture[i]);
}


ReverterRenderTarget::ReverterRenderTarget()
{ OriginalRenderTarget = Renderer::GetRenderTarget(); }
ReverterRenderTarget::~ReverterRenderTarget()
{ Restore(); }
void ReverterRenderTarget::Restore()
{ Renderer::SetRenderTarget(OriginalRenderTarget); }

ReverterEffect::ReverterEffect()
{ OriginalEffect = Renderer::GetEffect(); }
ReverterEffect::~ReverterEffect()
{ Restore(); }
void ReverterEffect::Restore()
{ Renderer::SetEffect(OriginalEffect); }

ReverterBlendState::ReverterBlendState()
{ OriginalBlendState = Renderer::GetBlendState(); }
ReverterBlendState::~ReverterBlendState()
{ Restore(); }
void ReverterBlendState::Restore()
{ Renderer::SetBlendState(OriginalBlendState); }

ReverterStencilState::ReverterStencilState()
{ OriginalStencilState = Renderer::GetStencilState(); }
ReverterStencilState::~ReverterStencilState()
{ Restore(); }
void ReverterStencilState::Restore()
{ Renderer::SetStencilState(OriginalStencilState); }

ReverterSamplerState::ReverterSamplerState()
{ OriginalSamplerState = Renderer::GetSamplerState(); }
ReverterSamplerState::~ReverterSamplerState()
{ Restore(); }
void ReverterSamplerState::Restore()
{ Renderer::SetSamplerState(OriginalSamplerState); }

ReverterTextureStageState::ReverterTextureStageState()
{ OriginalTextureStageState = Renderer::GetTextureStageState(); }
ReverterTextureStageState::~ReverterTextureStageState()
{ Restore(); }
void ReverterTextureStageState::Restore()
{ Renderer::SetTextureStageState(OriginalTextureStageState); }

ReverterIndexBuffer::ReverterIndexBuffer()
{ OriginalIndexBuffer = Renderer::GetIndexBuffer(); }
ReverterIndexBuffer::~ReverterIndexBuffer()
{ Restore(); }
void ReverterIndexBuffer::Restore()
{ Renderer::SetIndexBuffer(OriginalIndexBuffer); }



ReverterRawRenderTarget::ReverterRawRenderTarget()
{
	/*
	int i;
	CountRenderTarget = Renderer::StateRenderer.CountRenderTarget;
	if ( CountRenderTarget >= 16 )
	{
		uwArrOriginalRenderTarget = new IDirect3DSurface9*[CountRenderTarget];
		for ( i=0; i < CountRenderTarget; i++ )
		{
			uwArrOriginalRenderTarget[i] = NULL;
			Renderer::GetRenderTarget(i, &uwArrOriginalRenderTarget[i]);
			if ( uwArrOriginalRenderTarget[i] ) uwArrOriginalRenderTarget[i]->AddRef();
		}
	} else 
	{
		for ( i=0; i < CountRenderTarget; i++ )
		{
			ArrOriginalRenderTarget[i] = NULL;
			Renderer::GetRenderTarget(i, &ArrOriginalRenderTarget[i]);
			if ( ArrOriginalRenderTarget[i] ) ArrOriginalRenderTarget[i]->AddRef();
		}
	}*/
	OriginalRenderTarget = NULL;
	OriginalRenderTarget = Renderer::GetRawRenderTarget();
	if ( OriginalRenderTarget ) OriginalRenderTarget->AddRef();
}

ReverterRawRenderTarget::~ReverterRawRenderTarget()
{
	Restore();
	/*int i;
	if ( CountRenderTarget >= 16 )
	{
		for ( i=0; i < CountRenderTarget; i++ )
			if ( uwArrOriginalRenderTarget[i] ) uwArrOriginalRenderTarget[i]->Release();
	} else 
	{
		for ( i=0; i < CountRenderTarget; i++ )
			if ( ArrOriginalRenderTarget[i] ) ArrOriginalRenderTarget[i]->Release();
	}*/
	OriginalRenderTarget->Release();
}

void ReverterRawRenderTarget::Restore()
{
	Renderer::SetRawRenderTarget(OriginalRenderTarget);
	/*int i;
	if ( CountRenderTarget >= 16 )
	{
		for ( i=0; i < CountRenderTarget; i++ )
		{
			if ( uwArrOriginalRenderTarget[i] )
				Renderer::SetRenderTarget(i, uwArrOriginalRenderTarget[i]);
		}
	} else 
	{
		for ( i=0; i < CountRenderTarget; i++ )
		{
			if ( ArrOriginalRenderTarget[i] )
				Renderer::SetRenderTarget(i, ArrOriginalRenderTarget[i]);
		}
	}*/
}





ReverterRawPixelShader::ReverterRawPixelShader()
{
	OriginalPixelShader = 0;
	OriginalPixelShader = Renderer::GetRawPixelShader();
	if ( OriginalPixelShader ) OriginalPixelShader->AddRef();
}

ReverterRawPixelShader::~ReverterRawPixelShader()
{
	Restore();
	RELEASE_OBJECT(OriginalPixelShader);
}

void ReverterRawPixelShader::Restore()
{
	Renderer::SetRawPixelShader(OriginalPixelShader);
}




ReverterRawVertexShader::ReverterRawVertexShader()
{
	OriginalVertexShader = 0;
	OriginalVertexShader = Renderer::GetRawVertexShader();
	if ( OriginalVertexShader ) OriginalVertexShader->AddRef();
}

ReverterRawVertexShader::~ReverterRawVertexShader()
{
	Restore();
	RELEASE_OBJECT(OriginalVertexShader);
}

void ReverterRawVertexShader::Restore()
{
	Renderer::SetRawVertexShader(OriginalVertexShader);
}





ReverterRawBlendState::ReverterRawBlendState()
{
	OriginalBlendOp = OriginalAlphaBlendable = OriginalDestBlendAlpha = OriginalDestBlend = OriginalSrcBlendAlpha = OriginalSrcBlend = 0;

	OriginalSrcBlend	= Renderer::GetRawRenderState(D3DRS_SRCBLEND);
	OriginalDestBlend	= Renderer::GetRawRenderState(D3DRS_DESTBLEND);
	OriginalSrcBlendAlpha	= Renderer::GetRawRenderState(D3DRS_SRCBLENDALPHA);
	OriginalDestBlendAlpha	= Renderer::GetRawRenderState(D3DRS_DESTBLENDALPHA);
	OriginalBlendOp			= Renderer::GetRawRenderState(D3DRS_BLENDOP);
	OriginalAlphaBlendable	= Renderer::GetRawRenderState(D3DRS_ALPHABLENDENABLE);
	OriginalColorWriteEnable	= Renderer::GetRawRenderState(D3DRS_COLORWRITEENABLE);
	OriginalSeparateAlphaBlendEnable	= Renderer::GetRawRenderState(D3DRS_SEPARATEALPHABLENDENABLE);
}

ReverterRawBlendState::~ReverterRawBlendState()
{
	Restore();
}

void ReverterRawBlendState::Restore()
{
	Renderer::SetRawRenderState(D3DRS_SRCBLEND, OriginalSrcBlend);
	Renderer::SetRawRenderState(D3DRS_DESTBLEND, OriginalDestBlend);
	Renderer::SetRawRenderState(D3DRS_SRCBLENDALPHA, OriginalSrcBlendAlpha);
	Renderer::SetRawRenderState(D3DRS_DESTBLENDALPHA, OriginalDestBlendAlpha);
	Renderer::SetRawRenderState(D3DRS_BLENDOP, OriginalBlendOp);
	Renderer::SetRawRenderState(D3DRS_ALPHABLENDENABLE, OriginalAlphaBlendable);
	Renderer::SetRawRenderState(D3DRS_COLORWRITEENABLE, OriginalColorWriteEnable);
	Renderer::SetRawRenderState(D3DRS_SEPARATEALPHABLENDENABLE, OriginalSeparateAlphaBlendEnable);
}




ReverterRawRenderState::ReverterRawRenderState()
{
	int i;
	for ( i=0; i < COUNTOF(Original); i++ )
		Original[i] = 0;
	
	for ( i=7; i <= 9; i++ )
		Original[i] = Renderer::GetRawRenderState((D3DRENDERSTATETYPE)i);
	for ( i=14; i <= 16; i++ )
		Original[i] = Renderer::GetRawRenderState((D3DRENDERSTATETYPE)i);
	for ( i=19; i <= 20; i++ )
		Original[i] = Renderer::GetRawRenderState((D3DRENDERSTATETYPE)i);
	for ( i=22; i <= 29; i++ )
		Original[i] = Renderer::GetRawRenderState((D3DRENDERSTATETYPE)i);
	for ( i=34; i <= 38; i++ )
		Original[i] = Renderer::GetRawRenderState((D3DRENDERSTATETYPE)i);
	for ( i=48; i <= 48; i++ )
		Original[i] = Renderer::GetRawRenderState((D3DRENDERSTATETYPE)i);
	for ( i=52; i <= 60; i++ )
		Original[i] = Renderer::GetRawRenderState((D3DRENDERSTATETYPE)i);
	for ( i=128; i <= 143; i++ )
		Original[i] = Renderer::GetRawRenderState((D3DRENDERSTATETYPE)i);
	for ( i=145; i <= 152; i++ )
		Original[i] = Renderer::GetRawRenderState((D3DRENDERSTATETYPE)i);
	for ( i=154; i <= 163; i++ )
		Original[i] = Renderer::GetRawRenderState((D3DRENDERSTATETYPE)i);
	for ( i=165; i <= 195; i++ )
		Original[i] = Renderer::GetRawRenderState((D3DRENDERSTATETYPE)i);
	for ( i=198; i <= 209; i++ )
		Original[i] = Renderer::GetRawRenderState((D3DRENDERSTATETYPE)i);
}

ReverterRawRenderState::~ReverterRawRenderState()
{
	Restore();
}

void ReverterRawRenderState::Restore()
{
	int i;
	for ( i=7; i <= 9; i++ )
		Renderer::SetRawRenderState((D3DRENDERSTATETYPE)i, Original[i]);
	for ( i=14; i <= 16; i++ )
		Renderer::SetRawRenderState((D3DRENDERSTATETYPE)i, Original[i]);
	for ( i=19; i <= 20; i++ )
		Renderer::SetRawRenderState((D3DRENDERSTATETYPE)i, Original[i]);
	for ( i=22; i <= 29; i++ )
		Renderer::SetRawRenderState((D3DRENDERSTATETYPE)i, Original[i]);
	for ( i=34; i <= 38; i++ )
		Renderer::SetRawRenderState((D3DRENDERSTATETYPE)i, Original[i]);
	for ( i=48; i <= 48; i++ )
		Renderer::SetRawRenderState((D3DRENDERSTATETYPE)i, Original[i]);
	for ( i=52; i <= 60; i++ )
		Renderer::SetRawRenderState((D3DRENDERSTATETYPE)i, Original[i]);
	for ( i=128; i <= 143; i++ )
		Renderer::SetRawRenderState((D3DRENDERSTATETYPE)i, Original[i]);
	for ( i=145; i <= 152; i++ )
		Renderer::SetRawRenderState((D3DRENDERSTATETYPE)i, Original[i]);
	for ( i=154; i <= 163; i++ )
		Renderer::SetRawRenderState((D3DRENDERSTATETYPE)i, Original[i]);
	for ( i=165; i <= 195; i++ )
		Renderer::SetRawRenderState((D3DRENDERSTATETYPE)i, Original[i]);
	for ( i=198; i <= 209; i++ )
		Renderer::SetRawRenderState((D3DRENDERSTATETYPE)i, Original[i]);
}




ReverterRawStencilState::ReverterRawStencilState()
{
	OriginalZWriteEnable = OriginalZFunc = OriginalZEnable = OriginalStencilEnable = OriginalStencilFail = OriginalStencilFunc = OriginalStencilMask = OriginalStencilPass = OriginalStencilRef = OriginalStencilWriteMask = OriginalStencilZFail = 0;
	
	OriginalStencilEnable	= Renderer::GetRawRenderState(D3DRS_STENCILENABLE);
	OriginalStencilFail		= Renderer::GetRawRenderState(D3DRS_STENCILFAIL);
	OriginalStencilFunc		= Renderer::GetRawRenderState(D3DRS_STENCILFUNC);
	OriginalStencilMask		= Renderer::GetRawRenderState(D3DRS_STENCILMASK);
	OriginalStencilPass		= Renderer::GetRawRenderState(D3DRS_STENCILPASS);
	OriginalStencilRef		= Renderer::GetRawRenderState(D3DRS_STENCILREF);
	OriginalStencilWriteMask	= Renderer::GetRawRenderState(D3DRS_STENCILWRITEMASK);
	OriginalStencilZFail		= Renderer::GetRawRenderState(D3DRS_STENCILZFAIL);
	OriginalZEnable				= Renderer::GetRawRenderState(D3DRS_ZENABLE);
	OriginalZFunc				= Renderer::GetRawRenderState(D3DRS_ZFUNC);
	OriginalZWriteEnable		= Renderer::GetRawRenderState(D3DRS_ZWRITEENABLE);
	
}

ReverterRawStencilState::~ReverterRawStencilState()
{
	Restore();
}

void ReverterRawStencilState::Restore()
{
	Renderer::SetRawRenderState(D3DRS_STENCILENABLE,		OriginalStencilEnable);
	Renderer::SetRawRenderState(D3DRS_STENCILFAIL,		OriginalStencilFail);
	Renderer::SetRawRenderState(D3DRS_STENCILFUNC,		OriginalStencilFunc);
	Renderer::SetRawRenderState(D3DRS_STENCILMASK,		OriginalStencilMask);
	Renderer::SetRawRenderState(D3DRS_STENCILPASS,		OriginalStencilPass);
	Renderer::SetRawRenderState(D3DRS_STENCILREF,			OriginalStencilRef);
	Renderer::SetRawRenderState(D3DRS_STENCILWRITEMASK,	OriginalStencilWriteMask);
	Renderer::SetRawRenderState(D3DRS_STENCILZFAIL,		OriginalStencilZFail);

	Renderer::SetRawRenderState(D3DRS_ZENABLE,			OriginalZEnable);
	Renderer::SetRawRenderState(D3DRS_ZFUNC,				OriginalZFunc);
	Renderer::SetRawRenderState(D3DRS_ZWRITEENABLE,		OriginalZWriteEnable);
}




ReverterRawSamplerState::ReverterRawSamplerState()
{
	int i;
	memset(Original, 0, sizeof(Original));
	
	for ( i=0; i < 8; i++ )
	{
		Original[i].AddressU		= Renderer::GetRawSamplerState(i, D3DSAMP_ADDRESSU);
		Original[i].AddressV		= Renderer::GetRawSamplerState(i, D3DSAMP_ADDRESSV);
		Original[i].BorderColor		= Renderer::GetRawSamplerState(i, D3DSAMP_BORDERCOLOR);
		Original[i].MipFilter		= Renderer::GetRawSamplerState(i, D3DSAMP_MIPFILTER);
		Original[i].MagFilter		= Renderer::GetRawSamplerState(i, D3DSAMP_MAGFILTER);
		Original[i].MinFilter		= Renderer::GetRawSamplerState(i, D3DSAMP_MINFILTER);
		Original[i].MipMapLodBias	= Renderer::GetRawSamplerState(i, D3DSAMP_MIPMAPLODBIAS);
		Original[i].MaxMipLevel		= Renderer::GetRawSamplerState(i, D3DSAMP_MAXMIPLEVEL);
		Original[i].MaxAnisotropy	= Renderer::GetRawSamplerState(i, D3DSAMP_MAXANISOTROPY);
		Original[i].SrgbTexture		= Renderer::GetRawSamplerState(i, D3DSAMP_SRGBTEXTURE);
		Original[i].ElementIndex	= Renderer::GetRawSamplerState(i, D3DSAMP_ELEMENTINDEX);
		Original[i].DMapOffset		= Renderer::GetRawSamplerState(i, D3DSAMP_DMAPOFFSET);
	}
}

ReverterRawSamplerState::~ReverterRawSamplerState()
{
	Restore();
}

void ReverterRawSamplerState::Restore()
{
	int i;
	for ( i=0; i < 8; i++ )
	{
		Renderer::SetRawSamplerState(i, D3DSAMP_ADDRESSU, Original[i].AddressU);
		Renderer::SetRawSamplerState(i, D3DSAMP_ADDRESSV, Original[i].AddressV);
		Renderer::SetRawSamplerState(i, D3DSAMP_BORDERCOLOR, Original[i].BorderColor);
		Renderer::SetRawSamplerState(i, D3DSAMP_MIPFILTER, Original[i].MipFilter);
		Renderer::SetRawSamplerState(i, D3DSAMP_MAGFILTER, Original[i].MagFilter);
		Renderer::SetRawSamplerState(i, D3DSAMP_MINFILTER, Original[i].MinFilter);
		Renderer::SetRawSamplerState(i, D3DSAMP_MIPMAPLODBIAS, Original[i].MipMapLodBias);
		Renderer::SetRawSamplerState(i, D3DSAMP_MAXMIPLEVEL, Original[i].MaxMipLevel);
		Renderer::SetRawSamplerState(i, D3DSAMP_MAXANISOTROPY, Original[i].MaxAnisotropy);
		Renderer::SetRawSamplerState(i, D3DSAMP_SRGBTEXTURE, Original[i].SrgbTexture);
		Renderer::SetRawSamplerState(i, D3DSAMP_ELEMENTINDEX, Original[i].ElementIndex);
		Renderer::SetRawSamplerState(i, D3DSAMP_DMAPOFFSET, Original[i].DMapOffset);
	}
}





ReverterRawTextureStageState::ReverterRawTextureStageState()
{
	int i;
	memset(Original, 0, sizeof(Original));
	
	for ( i=0; i < 8; i++ )
	{
		Original[i].ALPHAARG0	= Renderer::GetRawTextureStageState(i, D3DTSS_ALPHAARG0);
		Original[i].ALPHAARG1	= Renderer::GetRawTextureStageState(i, D3DTSS_ALPHAARG1);
		Original[i].ALPHAARG2	= Renderer::GetRawTextureStageState(i, D3DTSS_ALPHAARG2);
		Original[i].ALPHAOP		= Renderer::GetRawTextureStageState(i, D3DTSS_ALPHAOP);
		Original[i].BUMPENVLOFFSET	= Renderer::GetRawTextureStageState(i, D3DTSS_BUMPENVLOFFSET);
		Original[i].BUMPENVLSCALE	= Renderer::GetRawTextureStageState(i, D3DTSS_BUMPENVLSCALE);
		Original[i].BUMPENVMAT00	= Renderer::GetRawTextureStageState(i, D3DTSS_BUMPENVMAT00);
		Original[i].BUMPENVMAT01	= Renderer::GetRawTextureStageState(i, D3DTSS_BUMPENVMAT01);
		Original[i].BUMPENVMAT10	= Renderer::GetRawTextureStageState(i, D3DTSS_BUMPENVMAT10);
		Original[i].BUMPENVMAT11	= Renderer::GetRawTextureStageState(i, D3DTSS_BUMPENVMAT11);
		Original[i].COLORARG0	= Renderer::GetRawTextureStageState(i, D3DTSS_COLORARG0);
		Original[i].COLORARG1	= Renderer::GetRawTextureStageState(i, D3DTSS_COLORARG1);
		Original[i].COLORARG2	= Renderer::GetRawTextureStageState(i, D3DTSS_COLORARG2);
		Original[i].COLOROP		= Renderer::GetRawTextureStageState(i, D3DTSS_COLOROP);
		Original[i].CONSTANT	= Renderer::GetRawTextureStageState(i, D3DTSS_CONSTANT);
		//Renderer::GetTextureStageState(i, ::GetTextureStageState(i, D3DTSS_RESULTARG, &Original[i].RESULTARG);
		Original[i].TEXCOORDINDEX			= Renderer::GetRawTextureStageState(i, D3DTSS_TEXCOORDINDEX);
		Original[i].TEXTURETRANSFORMFLAGS	= Renderer::GetRawTextureStageState(i, D3DTSS_TEXTURETRANSFORMFLAGS);
	}

}

ReverterRawTextureStageState::~ReverterRawTextureStageState()
{
	Restore();
}

void ReverterRawTextureStageState::Restore()
{
	int i;
	
	for ( i=0; i < 8; i++ )
	{
		Renderer::SetRawTextureStageState(i, D3DTSS_ALPHAARG0, Original[i].ALPHAARG0);
		Renderer::SetRawTextureStageState(i, D3DTSS_ALPHAARG1, Original[i].ALPHAARG1);
		Renderer::SetRawTextureStageState(i, D3DTSS_ALPHAARG2, Original[i].ALPHAARG2);
		Renderer::SetRawTextureStageState(i, D3DTSS_ALPHAOP, Original[i].ALPHAOP);
		Renderer::SetRawTextureStageState(i, D3DTSS_BUMPENVLOFFSET, Original[i].BUMPENVLOFFSET);
		Renderer::SetRawTextureStageState(i, D3DTSS_BUMPENVLSCALE, Original[i].BUMPENVLSCALE);
		Renderer::SetRawTextureStageState(i, D3DTSS_BUMPENVMAT00, Original[i].BUMPENVMAT00);
		Renderer::SetRawTextureStageState(i, D3DTSS_BUMPENVMAT01, Original[i].BUMPENVMAT01);
		Renderer::SetRawTextureStageState(i, D3DTSS_BUMPENVMAT10, Original[i].BUMPENVMAT10);
		Renderer::SetRawTextureStageState(i, D3DTSS_BUMPENVMAT11, Original[i].BUMPENVMAT11);
		Renderer::SetRawTextureStageState(i, D3DTSS_COLORARG0, Original[i].COLORARG0);
		Renderer::SetRawTextureStageState(i, D3DTSS_COLORARG1, Original[i].COLORARG1);
		Renderer::SetRawTextureStageState(i, D3DTSS_COLORARG2, Original[i].COLORARG2);
		Renderer::SetRawTextureStageState(i, D3DTSS_COLOROP, Original[i].COLOROP);
		Renderer::SetRawTextureStageState(i, D3DTSS_CONSTANT, Original[i].CONSTANT);
		//Renderer::SetTextureStageState(i, , D3DTSS_FORCE_DWORD, Original[i].FORCE_DWORD);
		Renderer::SetRawTextureStageState(i, D3DTSS_RESULTARG, Original[i].RESULTARG);
		Renderer::SetRawTextureStageState(i, D3DTSS_TEXCOORDINDEX, Original[i].TEXCOORDINDEX);
		Renderer::SetRawTextureStageState(i, D3DTSS_TEXTURETRANSFORMFLAGS, Original[i].TEXTURETRANSFORMFLAGS); 0;
	}
}







ReverterRawIndexBuffer::ReverterRawIndexBuffer()
{
	OriginalIndexBuffer = NULL;
	OriginalIndexBuffer = Renderer::GetRawIndexBuffer();
	if ( OriginalIndexBuffer ) OriginalIndexBuffer->AddRef();
}

ReverterRawIndexBuffer::~ReverterRawIndexBuffer()
{
	Restore();
	RELEASE_OBJECT(OriginalIndexBuffer);
}

void ReverterRawIndexBuffer::Restore()
{
	//EngineManager::Engine->gDevice->SetIndices(OriginalIndexBuffer);
	Renderer::SetRawIndexBuffer(OriginalIndexBuffer);
}







ReverterRawDepthStencilSurface::ReverterRawDepthStencilSurface()
{
	OriginalDepthStencilSurface = Renderer::GetRawDepthStencilSurface();
	if ( OriginalDepthStencilSurface ) OriginalDepthStencilSurface->AddRef();
}

ReverterRawDepthStencilSurface::~ReverterRawDepthStencilSurface()
{
	Restore();
	RELEASE_OBJECT(OriginalDepthStencilSurface);
}

void ReverterRawDepthStencilSurface::Restore()
{
	Renderer::SetRawDepthStencilSurface(OriginalDepthStencilSurface);
}

