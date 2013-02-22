#include "StdAfx.hpp"
#include "DeviceResource.h"
#include "Renderer.h"
#include "Function.h"

using namespace Mgine;
using namespace Util;


Effect::EffectConstant::~EffectConstant()
{
	int i, count = 0;
	for ( i=0; i < ConstantTable.Size; i++ )
	{
		if ( ConstantTable.IsElementExisting(i) )
		{
			EFFECT_CONSTANT *con = (EFFECT_CONSTANT*)ConstantTable[i];
			con->Uninit();
			delete con;
			ConstantTable.Sub(i);
		}
	}
}

void Effect::EffectConstant::AddConstant(char *Ident, EFFECT_VALUE_TYPE Type, void *Data, DWORD Cb)
{
	int i, count = 0;
	bool ismodified = false;
	for ( i=0; i < ConstantTable.Size; i++ ){
		if ( ConstantTable[i] ){
			EFFECT_CONSTANT *con = ((EFFECT_CONSTANT*)ConstantTable[i]);

			if ( !strcmp(con->Ident, Ident) ){
				if ( con->Type == Type ){
					void *newdata = new char[Cb];
					memcpy(newdata, Data, Cb);
					if ( con->Data ) delete con->Data;
					con->Data = newdata;
					con->Cb = Cb;
					ismodified = true;
					break;
				}
			}
			count++;
		}
		if ( count >= ConstantTable.Count ) break;
	}

	if ( !ismodified )
	{
		EFFECT_CONSTANT *Con = new EFFECT_CONSTANT;
		ConstantTable.Add(Con);
		Con->Type = Type;
		memset(Con->Ident, 0, 128);
		strncpy_s(Con->Ident, Ident, 128);
		Con->Data = new char[Cb];
		memcpy(Con->Data, Data, Cb);
		Con->Cb = Cb;
	}
}




char *Effect::BaseShaderFileBuffer;
DWORD Effect::BaseShaderFileBufferSize;


Effect::~Effect()
{
	if ( this == Renderer::GetEffect() ) Renderer::SetEffect(NULL);
	Uninit();
	
	if ( &DeviceResourceManager::ListEffect() )
		DeviceResourceManager::ListEffect().Sub(ListIndex);
}
Effect::Effect(wchar_t *FileName)
{
}
Effect::Effect(void *Content)
{
}
bool Effect::InitBaseShaderFromFile(wchar_t *FileName)
{
	HANDLE hf = CreateFileW(FileName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_ARCHIVE, 0);
	if ( hf == INVALID_HANDLE_VALUE )
	{
		MLOG("Failed to cfw");
		goto cleanup;
	}
	DWORD size = GetFileSize(hf, 0);
	
	BaseShaderFileBuffer = new char[size];
	BaseShaderFileBufferSize = size;

	DWORD dw;
	LPD3DXBUFFER pError = 0;
	if ( !ReadFile(hf, BaseShaderFileBuffer, size, &dw, 0) || dw < size )
	{
		MLOG("Failed");
		goto cleanup;
	}

	return true;

cleanup:
	MLOG("Failed");
	CloseHandle(hf);
	if ( BaseShaderFileBuffer ) delete BaseShaderFileBuffer;
	BaseShaderFileBuffer = 0;
	BaseShaderFileBufferSize = 0;
	return false;
}

void Effect::UninitBaseShader()
{
	if ( BaseShaderFileBuffer ) delete BaseShaderFileBuffer;
	BaseShaderFileBuffer = 0;
	BaseShaderFileBufferSize = 0;
}

bool Effect::InitBaseShaderFromMemory(void *Mem, DWORD Size)
{
	if ( BaseShaderFileBuffer ) delete BaseShaderFileBuffer;
	BaseShaderFileBuffer = new char[Size];
	BaseShaderFileBufferSize = Size;
	
	memcpy(BaseShaderFileBuffer, Mem, Size);

	return false;
}

/*bool Effect::InitFromFile(wchar_t *FileName, char *FunctionName)
{
	LPD3DXBUFFER pError = 0;
	if ( FAILED(D3DXCompileShaderFromFileW(FileName, NULL, NULL, FunctionName, "ps_3_0", 0, &gCode, &pError, &gConstantTable)) )
	{
		char *errstr = (char*)pError->GetBufferPointer();
		MessageBox(NULL, errstr, "Forgive me: Pixel Shader Compile Error",
			MB_OK|MB_ICONEXCLAMATION);
		__asm int 3;
		goto cleanup;
	}

    if ( FAILED(EngineManager::Engine->gDevice->CreatePixelShader( (DWORD*)gCode->GetBufferPointer(), &gPixelShader )) ) goto cleanup;
	if ( !SetDefaultParameter() ) goto cleanup;

	IsInited = true;
	IsDeviceLost = false;
	return true;

cleanup:
MLOG("Failed");
	if ( gCode ) gCode->Release();
	if ( pError ) pError->Release();
	if ( gConstantTable ) gConstantTable->Release();
	memset(&gConstantTable, 0, sizeof(gConstantTable));
	gCode = 0;
	pError = 0;
	gConstantTable = 0;

	Uninit();
	return false;
}

bool Effect::InitFromMemory(void *Content)
{
	IsInited = true;
	IsDeviceLost = false;
	return true;
}*/

void Effect::InitShader(char *FunctionName, bool IsVertexShader)
{
	char prof[1024] = "ps_3_0";
	if ( IsVertexShader ) strcpy_s(prof, 1024, "vs_3_0");
	LPD3DXBUFFER perror = 0;

	try
	{
		struct FINALPROC {
			boost::function<void ()> func;
			FINALPROC(decltype(func) Function){ func=Function; }
			~FINALPROC(){ func(); }
		} finalproc( [&]()
		{
			RELEASE_OBJECT(perror);
		});

		if ( FAILED(D3DXCompileShader(BaseShaderFileBuffer, BaseShaderFileBufferSize, 
			NULL, NULL, FunctionName, prof, D3DXSHADER_ENABLE_BACKWARDS_COMPATIBILITY, &gCode, &perror, &gConstantTable)) )
		{
			/*char *errstr = (char*)pError->GetBufferPointer();
			MessageBoxA(NULL, errstr, "Forgive me: Shader Compile Error",
				MB_OK|MB_ICONEXCLAMATION);
			__asm int 3;*/
			UnwindArray<wchar_t> procstr = new wchar_t[1000];
			MultiByteToWideChar(CP_ACP, 0, FunctionName, strlen(FunctionName), procstr.Arr, 1000);
			MTHROW(DirectXError, "Failed to compile shader. isvs:%d %s", IsVertexShader, procstr.Arr);
		}
		

		if ( IsVertexShader )
		{
			RELEASE_OBJECT(gVertexShader);

			if ( FAILED(EngineManager::Engine->gDevice->CreateVertexShader( (DWORD*)gCode->GetBufferPointer(), &gVertexShader)) )
			{
				UnwindArray<wchar_t> procstr = new wchar_t[1000];
				MultiByteToWideChar(CP_ACP, 0, FunctionName, strlen(FunctionName), procstr.Arr, 1000);
				MTHROW(DirectXError, "Failed to load vs. Check your graphics card is supporting vertex shader 3. %s", procstr.Arr);
			}
		}
		else 
		{
			RELEASE_OBJECT(gPixelShader);
			if ( FAILED(EngineManager::Engine->gDevice->CreatePixelShader( (DWORD*)gCode->GetBufferPointer(), &gPixelShader )) ){
				UnwindArray<wchar_t> procstr = new wchar_t[1000];
				MultiByteToWideChar(CP_ACP, 0, FunctionName, strlen(FunctionName), procstr.Arr, 1000);
				MTHROW(DirectXError, "Failed to load ps. Check your graphics card is supporting pixel shader 3. %s", procstr.Arr);
			}
		}

		__SetIsInited(true);
		//__SetIsDeviceLost(false);
		if ( IsVertexShader ) IsVertexShaderLoaded = true;
		else IsPixelShaderLoaded = true;

		SetDefaultParameter();
	}
	catch ( BaseException & )
	{
		MLOG("Failed");

		RELEASE_OBJECT(gCode);
		RELEASE_OBJECT(gConstantTable);

		__SetIsInited(false);
		//__SetIsDeviceLost(false);

		Uninit();

		throw;
	}
}

void Effect::InitPixelShader(char *FunctionName)
{
	return InitShader(FunctionName, false);
}

void Effect::InitVertexShader(char *FunctionName)
{
	return InitShader(FunctionName, true);
}

void Effect::ReloadConstant()
{
	try
	{ 
		int i, count = 0;
		for ( i=0; i < ConstantSaved.ConstantTable.Size; i++ ){
			if ( ConstantSaved.ConstantTable[i] ){
				EFFECT_CONSTANT *con = (EFFECT_CONSTANT*)ConstantSaved.ConstantTable[i];
				switch ( con->Type )
				{
				case EFFECT_ARRAY:
					SetParameter(con->Ident, con->Type, con->Data, con->Cb);
					break;

				case EFFECT_BOOL:
					SetParameter(con->Ident, con->Type, (void*)*(BOOL*)con->Data, 4);
					break;

				case EFFECT_BOOL_ARRAY:
					SetParameter(con->Ident, con->Type, con->Data, con->Cb/4);
					break;

				case EFFECT_INT:
					SetParameter(con->Ident, con->Type, (void*)*(int*)con->Data, 4);
					break;

				case EFFECT_INT_ARRAY:
					SetParameter(con->Ident, con->Type, con->Data, con->Cb/4);
					break;

				case EFFECT_FLOAT:
					SetParameter(con->Ident, con->Type, **(void***)&con->Data, 4);
					break;

				case EFFECT_FLOAT_ARRAY:
					SetParameter(con->Ident, con->Type, con->Data, con->Cb/4);
					break;

				case EFFECT_TEXTURE:
					SetParameter(con->Ident, con->Type, **(void***)&con->Data, 4);
					break;

				default:break;
				}
				count++;
			}
			if ( ConstantSaved.ConstantTable.Count <= count ) break;
		}
	}
	catch ( BaseException & )
	{
		MLOG("Failed");
		throw;
	}
}

void Effect::OnDeviceLost()
{
	//RELEASE_OBJECT(gPixelShader);
	//RELEASE_OBJECT(gVertexShader);

}

void Effect::OnDeviceReset()
{
	//__SetIsDeviceLost(false);
	ReloadConstant();
}

void Effect::OnDeviceCreate()
{
	bool issuc = true;

	RELEASE_OBJECT(gPixelShader);
	RELEASE_OBJECT(gVertexShader);
	if ( IsPixelShaderLoaded )
		if ( FAILED(EngineManager::Engine->gDevice->CreatePixelShader((DWORD*)gCode->GetBufferPointer(), &gPixelShader )) ) issuc = false;
	if ( IsVertexShaderLoaded )
		if ( FAILED(EngineManager::Engine->gDevice->CreateVertexShader((DWORD*)gCode->GetBufferPointer(), &gVertexShader )) ) issuc = false;

	try
	{ 
		ReloadConstant();
	}
	catch ( BaseException & )
	{
		issuc = false;
	}

	if ( !issuc )
	{
		RELEASE_OBJECT(gPixelShader);
		RELEASE_OBJECT(gVertexShader);
		MTHROW(InvalidOperation, "Failed");
	}
}

void Effect::OnSetAsCurrentEffect()
{
	Renderer::SetRawPixelShader(gPixelShader);
	Renderer::SetRawVertexShader(gVertexShader);
}

void Effect::OnUnsetAsCurrentEffect()
{
	//Renderer::SetRawPixelShader(NULL);
	//Renderer::SetRawVertexShader(NULL);
}

void Effect::SetParameter(char *ParamName, EFFECT_VALUE_TYPE Type, void *Value, DWORD Size)
{
	if ( !IsInited )
		MTHROW(InvalidStatus, "Not inited");

	D3DXHANDLE handle = 0;
	if ( !(handle = gConstantTable->GetConstantByName(NULL, ParamName))  ) goto cleanup;
	
	D3DXCONSTANT_DESC constDesc;
	UINT count;

	switch ( Type )
	{
		case EFFECT_ARRAY:
			if ( FAILED(gConstantTable->SetValue(EngineManager::Engine->gDevice, handle, (BYTE*)Value, Size)) ) goto cleanup;
			ConstantSaved.AddConstant(ParamName, Type, Value, Size);
			break;

		case EFFECT_BOOL:
			if ( FAILED(gConstantTable->SetBool(EngineManager::Engine->gDevice, handle, *(BOOL*)&Value)) ) goto cleanup;
			ConstantSaved.AddConstant(ParamName, Type, &Value, 4);
			break;

		case EFFECT_BOOL_ARRAY:
			if ( FAILED(gConstantTable->SetBoolArray(EngineManager::Engine->gDevice, handle, (BOOL*)Value, Size)) ) goto cleanup;
			ConstantSaved.AddConstant(ParamName, Type, Value, 4*Size);
			break;

		case EFFECT_INT:
			if ( FAILED(gConstantTable->SetInt(EngineManager::Engine->gDevice, handle, *(int*)&Value)) ) goto cleanup;
			ConstantSaved.AddConstant(ParamName, Type, &Value, 4);
			break;

		case EFFECT_INT_ARRAY:
			if ( FAILED(gConstantTable->SetIntArray(EngineManager::Engine->gDevice, handle, (int*)Value, Size)) ) goto cleanup;
			ConstantSaved.AddConstant(ParamName, Type, Value, 4*Size);
			break;

		case EFFECT_FLOAT:
			if ( FAILED(gConstantTable->SetFloat(EngineManager::Engine->gDevice, handle, *(float*)&Value)) ) goto cleanup;
			ConstantSaved.AddConstant(ParamName, Type, &Value, 4);
			break;

		case EFFECT_FLOAT_ARRAY:
			if ( FAILED(gConstantTable->SetFloatArray(EngineManager::Engine->gDevice, handle, (float*)Value, Size)) ) goto cleanup;
			ConstantSaved.AddConstant(ParamName, Type, Value, 4*Size);
			break;

		case EFFECT_TEXTURE:
			if ( FAILED(gConstantTable->GetConstantDesc( handle, &constDesc, &count )) ) goto cleanup;
			if ( constDesc.RegisterSet == D3DXRS_SAMPLER )
			{
				if ( EngineManager::Core->IsReloading ) Value = 0;
				if ( FAILED(EngineManager::Engine->gDevice->SetTexture( constDesc.RegisterIndex, (IDirect3DBaseTexture9*)Value)) ) goto cleanup;
				DWORD slen = strlen(ParamName);
				if ( slen == 6 &&
					ParamName[0] == 'N' &&
					ParamName[1] == 'o' &&
					ParamName[2] == 'i' &&
					ParamName[3] == 's' &&
					ParamName[4] == 'e' )
				{
					EngineManager::Engine->gDevice->SetSamplerState(constDesc.RegisterIndex, D3DSAMP_ADDRESSU, D3DTADDRESS_MIRROR);
					EngineManager::Engine->gDevice->SetSamplerState(constDesc.RegisterIndex, D3DSAMP_ADDRESSV, D3DTADDRESS_MIRROR);
				} else if ( slen == 7 &&
					ParamName[0] == 'S' &&
					ParamName[1] == 'o' &&
					ParamName[2] == 'u' &&
					ParamName[3] == 'r' &&
					ParamName[4] == 'c' &&
					ParamName[5] == 'e' )
				{
					EngineManager::Engine->gDevice->SetSamplerState(constDesc.RegisterIndex, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
					EngineManager::Engine->gDevice->SetSamplerState(constDesc.RegisterIndex, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
				}
			} else goto cleanup;
			//if ( FAILED(gConstantTable->SetValue(EngineManager::Engine->gDevice, handle, Value, 4)) ) goto cleanup;
			
			ConstantSaved.AddConstant(ParamName, Type, &Value, 4);
			break;

		default:goto cleanup;
	}
	
	return ;

cleanup:
	// MLOG("Failed"); 
	// don't throw exception cuz it could be failed when var is not existing.. so normal behavior
	return ;
}

void Effect::SetParameter(char *ParamName, float Value)
{
	return SetParameter(ParamName, EFFECT_FLOAT, *(void**)&Value, 0);
}

void Effect::SetParameter(char *ParamName, int Value)
{
	return SetParameter(ParamName, EFFECT_INT, *(void**)&Value, 0);
}

void Effect::SetParameter(char *ParamName, LPDIRECT3DBASETEXTURE9 Texture)
{
	return SetParameter(ParamName, EFFECT_TEXTURE, Texture, 0);
}

void Effect::Start(int MaxProgress, float MultipleProgress, int LoopCount)
{
	if ( !IsInited )
		MTHROW(InvalidStatus, "Not inited");
	if ( MaxProgress == 0 ) return ;

	this->MaxProgress = MaxProgress;
	this->MultipleProgress = MultipleProgress;
	this->LoopCount = LoopCount;
	this->Progress = 0;
	this->IsStarted = true;

}

void Effect::Stop()
{
	if ( !IsInited )
		MTHROW(InvalidStatus, "Not inited");
	this->IsStarted = false;
	this->Progress = 0;
}

void Effect::Update(double ElapsedTime)
{
	if ( !IsInited )
		MTHROW(InvalidStatus, "Not inited");
	if ( IsStarted && LoopCount )
	{
		Progress += ElapsedTime;
		if ( Progress >= MaxProgress )
		{
			if ( LoopCount > 0 )
			{
				LoopCount -= (int)(Progress / MaxProgress);
				if ( LoopCount < 0 ) LoopCount = 0;
			}
			if ( LoopCount ) Progress = fmod(Progress, MaxProgress);
			
			if ( !LoopCount && Progress > MaxProgress )
				Progress = MaxProgress;
		}

	} else if ( IsStarted ) IsStarted = false;

	return ;
}


void Effect::Apply()
{
	if ( !IsInited ) MTHROW(InvalidStatus, "Not inited");

	try
	{
		if ( !gVertexShader )
		{
			if ( DeviceResourceManager::DefaultVertexShader.IsInited )
				DeviceResourceManager::DefaultVertexShader.Apply();
		}
		else Renderer::SetRawVertexShader(gVertexShader);
		if ( gPixelShader ) Renderer::SetRawPixelShader(gPixelShader);

		ReloadConstant();
		float f[2];

		f[0] = (float)Renderer::GetResolutionWidth();
		f[1] = (float)Renderer::GetResolutionHeight();
		SetParameter("Viewport", EFFECT_FLOAT_ARRAY, f, 2);
		f[0] = 1.0f / Renderer::GetResolutionWidth();
		f[1] = 1.0f / Renderer::GetResolutionHeight();
		SetParameter("UnitViewport", EFFECT_FLOAT_ARRAY, f, 2);

		SetParameter("BackBuffer", EFFECT_TEXTURE, Renderer::GetRawBackBuffer(), 0);
		SetParameter("fProgress", (float)(Progress / MaxProgress * MultipleProgress));
		D3DXMATRIX matproj = Renderer::GetMatrixProjection();
		SetParameter("MatrixProjection", EFFECT_FLOAT_ARRAY, (float*)&matproj.m, 16);

		IDirect3DSurface9 *surface = NULL;

		try
		{  
			D3DSURFACE_DESC desc;
			if ( SUCCEEDED(EngineManager::Engine->gDevice->GetRenderTarget(0, &surface)) &&
				SUCCEEDED(surface->GetDesc(&desc)) )
			{
				f[0] = (float)desc.Width;
				f[1] = (float)desc.Height;
			} else {
				f[0] = (float)Renderer::GetResolutionWidth();
				f[1] = (float)Renderer::GetResolutionHeight();
			}
			SetParameter("RenderTargetBox", EFFECT_FLOAT_ARRAY, f, 2);
			RELEASE_OBJECT(surface);
		}
		catch ( BaseException & )
		{
			RELEASE_OBJECT(surface);
			throw;
		}
	}
	catch ( BaseException & )
	{
		MLOG("Failed");
		throw;
	}

	return;
}

void Effect::SetDefaultParameter()
{
	/*
	sampler SamplerA;
	sampler BackBuffer;

	float fProgress;
	float fAmount;
	int iSeed;

	float2 Viewport;
	float2 Origin;
	float2 ViewportRatio;
	float2 UnitViewport;

	float MF::PI_F;
	*/
	float f[2];

	try
	{ 
		SetParameter("fProgress", 0.0f);
		SetParameter("fAmount", 1.0f);
		SetParameter("iSeed", 0);

		f[0] = 0.5f; f[1] = 0.5f;
		SetParameter("Origin", EFFECT_FLOAT_ARRAY, f, 2);
		f[0] = 1.0f; f[1] = 1.0f;
		SetParameter("ViewportRatio", EFFECT_FLOAT_ARRAY, f, 2);
		SetParameter("TextureRatio",  EFFECT_FLOAT_ARRAY, f, 2);
		f[0] = 0.0f; f[1] = 0.0f;
		SetParameter("Location", EFFECT_FLOAT_ARRAY, f, 2);

		SetParameter("PI", MF::PI_F);
	}
	catch ( BaseException & )
	{
		MLOG("Failed");
		throw;
	}
}

void Effect::Uninit()
{
	RELEASE_OBJECT(gPixelShader);
	RELEASE_OBJECT(gVertexShader);
	RELEASE_OBJECT(gConstantTable);
	RELEASE_OBJECT(gCode);

	__SetIsInited(false);
	//__SetIsDeviceLost(false);
	__SetIsPixelShaderLoaded(false);
	__SetIsVertexShaderLoaded(false);
}