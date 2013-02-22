#include "StdAfx.hpp"
#include "DeviceResource.h"
#include "Renderer.h"
#include "Function.h"
using namespace Mgine;
using namespace Util;

	
Font::~Font()
{
	if ( &DeviceResourceManager::ListFont() )
		DeviceResourceManager::ListFont().Sub(FontIndex);
	Uninit();
}

void Font::Init(wchar_t *FaceName, DWORD Width, int Height, DWORD Weight, bool IsClearType)
{
	//AddFontResource
	if ( IsInited ) Uninit();

	try
	{

	}
	catch ( BaseException & )
	{
		throw;
	}
	LPD3DXFONT g_pFont = NULL;

	FontDesc.CharSet = HANGUL_CHARSET;
	wcscpy_s(FontDesc.FaceName, LF_FACESIZE, FaceName);
	FontDesc.Width = Width;
	FontDesc.Height = Height;
	FontDesc.Weight = Weight; //FW_NORMAL;
	if ( IsClearType )
		FontDesc.Quality = CLEARTYPE_NATURAL_QUALITY; //DEFAULT_QUALITY;
	else FontDesc.Quality = DEFAULT_QUALITY;
	FontDesc.MipLevels = 1;
	FontDesc.Italic = this->IsItalic;
	FontDesc.OutputPrecision = OUT_DEFAULT_PRECIS;
	FontDesc.PitchAndFamily = FF_DONTCARE | DEFAULT_PITCH;

	try
	{ 
		if ( FAILED(D3DXCreateFontIndirect(EngineManager::Engine->gDevice, &FontDesc,&gFont)) )
			MTHROW(DirectXError, "Failed DXCFI");
	}
	catch ( BaseException & )
	{
		MLOG("Failed");
		Uninit();
		throw;
	}

	__SetIsInited(true);
	//__SetIsDeviceLost(false);
}

void Font::Uninit()
{
	__SetIsInited(false);
	RELEASE_OBJECT(gFont);
}

void Font::OnDeviceLost()
{
	ATHROW(SUCCEEDED(gFont->OnLostDevice()));
}

void Font::OnDeviceReset()
{
	ATHROW(SUCCEEDED(gFont->OnResetDevice()));
}

void Font::OnDeviceCreate()
{
	RELEASE_OBJECT(gFont);
	ATHROW(SUCCEEDED(D3DXCreateFontIndirect(EngineManager::Engine->gDevice, &FontDesc, &gFont)) );
}

SIZE Font::Draw(const std::wstring & DrawStr, int X, int Y, int Width, int Height, COLOR Color, DWORD Flags, bool IsUsingZBuffer)
{
	SIZE ret = { 0, 0 };
	if ( !IsInited )
		MTHROW(InvalidOperation, "Not inited");

	try
	{
		RECT rt = { X, Y, X+Width, Y+Height }, rtret;

		if ( !gFont->DrawTextW(NULL, DrawStr.c_str(), DrawStr.size(), &rt, Flags | DT_CALCRECT, Color) )
			MTHROW(DirectXError, "DrawText");
		rtret		= rt;
		rt.left		= (LONG)MF::MinMax(rt.left, (LONG)0, (LONG)Renderer::GetResolutionWidth());
		rt.top		= (LONG)MF::MinMax(rt.top , (LONG)0, (LONG)Renderer::GetResolutionHeight());
		rt.right	= (LONG)MF::MinMax(rt.right, (LONG)0, (LONG)Renderer::GetResolutionWidth());
		rt.bottom	= (LONG)MF::MinMax(rt.bottom, (LONG)0, (LONG)Renderer::GetResolutionHeight());

		ReverterRawRenderState	RestoreRenderState;

		Renderer::SetRawRenderState(D3DRS_ZENABLE, false);
		Renderer::SetRawRenderState(D3DRS_ZWRITEENABLE, false);
		/*HDC dc = gFont->GetDC();
		float	dpiscalex = GetDeviceCaps(dc, LOGPIXELSX) / 96.0f,
				dpiscaley = GetDeviceCaps(dc, LOGPIXELSY) / 96.0f;
	
		int i, p;
		for ( i=0; i < lenstr; i++ )
		{
			for ( p=i; p < lenstr; p++ )
				if ( DrawStr[p] == '\n' )
					break;
				else if ( DrawStr[p] == '\n\r' ) // \r\n is '\n\r'
					break;
			//i ~ p
			SIZE size = { 0, 0 };
			if ( !GetTextExtentPoint(dc, DrawStr+i, p-i, &size) ) issucceeded = false;

			ret.cy += size.cy;
			if ( size.cx > ret.cx ) ret.cx = size.cx;
			i = p;
		}*/

		Renderer::FlushBatch();


		if ( IsUsingZBuffer )
		{
			if ( Z[0] < 0 ) Z[0] = 0;
			if ( Z[0] > 1 ) Z[0] = 1;
			if ( Z[1] < 0 ) Z[1] = 0;
			if ( Z[1] > 1 ) Z[1] = 1;
			if ( Z[2] < 0 ) Z[2] = 0;
			if ( Z[2] > 1 ) Z[2] = 1;
			if ( Z[3] < 0 ) Z[3] = 0;
			if ( Z[3] > 1 ) Z[3] = 1;

			ReverterRawRenderTarget RestoreRenderTarget;
			ReverterRawBlendState	RestoreBlendState;

		

			/*if ( Flags & DT_NOCLIP )
			{
				rt.right = X + ret.cx;
				rt.right = Y + ret.cy;
			}
			if ( rt.right  > X+ret.cx ) rt.right  = X+ret.cx;
			if ( rt.bottom > Y+ret.cy ) rt.bottom = Y+ret.cy;
			*/
			EngineManager::Engine->gDevice->ColorFill(Renderer::TemporaryRenderTargetTexture[0].GetSurface(), &rt, 0x00000000);
			Renderer::SetRawRenderTarget(Renderer::TemporaryRenderTargetTexture[0].GetSurface());

	/*		if ( !gFont->DrawTextW(NULL, DrawStr, lenstr, &rt, Flags | DT_CALCRECT, Color) ) issucceeded = false;
	#ifdef _DEBUG
			if ( issucceeded )
			{
				if ( rt.right-rt.left  != ret.cx ) __asm int 3;
				if ( rt.bottom-rt.top  != ret.cy ) __asm int 3;
			}
	#endif*/
			if ( FAILED(DeviceResourceManager::SpriteFont->Begin(0)) )
				MTHROW(DirectXError, "sf Begin");
			if ( !gFont->DrawTextW(NULL, DrawStr.c_str(), DrawStr.size(), &rtret, Flags, Color) )
				MTHROW(DirectXError, "DrawText");
			if ( FAILED(DeviceResourceManager::SpriteFont->End()) )
				MTHROW(DirectXError, "sf End");

			RestoreRenderTarget.Restore();
			RestoreRenderState.Restore();

			Renderer::SetBlendState(BLEND_STATE(BLEND_STATE::COPY));
		
			QuadDrawer drawer;
			drawer.Init(&Renderer::TemporaryRenderTargetTexture[0]);

			drawer.RectDest.X = (float)rt.left;
			drawer.RectDest.Y = (float)rt.top;
			drawer.RectDest.Width  = (float)rt.right-rt.left;
			drawer.RectDest.Height = (float)rt.bottom-rt.top;
			drawer.RectSource = drawer.RectDest;
			drawer.Vertex.V1.Z = Z[0];
			drawer.Vertex.V2.Z = Z[1];
			drawer.Vertex.V3.Z = Z[2];
			drawer.Vertex.V4.Z = Z[3];
			drawer.Draw();
		
		} else 
		{
	/*		if ( !gFont->DrawTextW(NULL, DrawStr, lenstr, &rt, Flags | DT_CALCRECT, Color) ) issucceeded = false;
	#ifdef _DEBUG
			if ( issucceeded )
			{
				if ( rt.right-rt.left  != ret.cx ) __asm int 3;
				if ( rt.bottom-rt.top  != ret.cy ) __asm int 3;
			}
	#endif*/
			if ( FAILED(DeviceResourceManager::SpriteFont->Begin(0)) )
				MTHROW(DirectXError, "sf Begin !z");
			if ( FAILED(gFont->DrawTextW(DeviceResourceManager::SpriteFont, DrawStr.c_str(), DrawStr.size(), &rtret, Flags, Color)) )
				MTHROW(DirectXError, "DrawText !z");
			if ( FAILED(DeviceResourceManager::SpriteFont->End()) )
				MTHROW(DirectXError, "sf End !z");
		}
		/*if ( !issucceeded )
		{
			return retfailed;
		}*/
		ret.cx = rtret.right  - rtret.left;
		ret.cy = rtret.bottom - rtret.top;
		return ret;
	}
	catch ( BaseException & )
	{
		MLOG("Failed");
		throw;
	}

}

RECT Font::GetTextDrawRect(const std::wstring& DrawStr, int X, int Y, int Width, int Height, COLOR Color, DWORD Flags)
{
	RECT retfailed = { -1, -1, -1, -1 };
	if ( !IsInited ) return retfailed;

	bool issucceeded = true;

	RECT rt = { X, Y, X+Width, Y+Height };
	if ( !gFont->DrawTextW(NULL, DrawStr.c_str(), DrawStr.size(), &rt, Flags | DT_CALCRECT, Color) ) issucceeded = false;

	if ( !issucceeded ) return retfailed;
	return rt;
}
