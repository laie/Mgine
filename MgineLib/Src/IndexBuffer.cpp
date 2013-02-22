#include "StdAfx.hpp"
#include "DeviceResource.h"
#include "Renderer.h"
using namespace Mgine;
using namespace Util;

IndexBuffer::~IndexBuffer()
{
	if ( this == Renderer::GetIndexBuffer() ) Renderer::SetIndexBuffer(NULL);
	DeviceResourceManager::ListIndexBuffer().Sub(IndexBufferIndex);
	Uninit();
}

void IndexBuffer::Init(WORD *Indices, DWORD CountIndices)
{
	if ( IsInited ) MTHROW(InvalidStatus, "Already inited");
	try
	{
		if ( FAILED(EngineManager::Engine->gDevice->CreateIndexBuffer(CountIndices*sizeof(WORD), D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED, &gIndexBuffer, 0)) )
			MTHROW(InvalidOperation, "Failed CreateIndexBuffer");
		if ( FAILED(gIndexBuffer->GetDesc(&DescBuffer)) )
			MTHROW(InvalidOperation, "GetDesc");

		WORD *pindices = 0, n = 0;
		if ( FAILED(gIndexBuffer->Lock(0, 0, (void**)&pindices, 0)) )
			MTHROW(InvalidOperation, "Lock");
		memcpy(pindices, Indices, CountIndices*sizeof(WORD));
		if ( FAILED(gIndexBuffer->Unlock()) )
			MTHROW(InvalidOperation, "Unlock");

		__SetIsInited(true);
	}
	catch (...)
	{
		Uninit();
		throw;
	}
}

void IndexBuffer::Uninit()
{
	Renderer::SetRawIndexBuffer(NULL);

	__SetIsInited(false);
	RELEASE_OBJECT(gIndexBuffer);
}

void IndexBuffer::OnDeviceLost()
{
}

void IndexBuffer::OnDeviceReset()
{
}

void IndexBuffer::OnDeviceCreate()
{
	bool issuc = true;

	WORD *pindices = 0;
	UnwindArray<WORD> cpybuf = 0;

	if ( FAILED(gIndexBuffer->Lock(0, 0, (void**)&pindices, 0)) ) issuc = false;
	cpybuf.Set((WORD*)(new char[DescBuffer.Size]));
	memcpy(cpybuf.Arr, pindices, DescBuffer.Size);
	if ( FAILED(gIndexBuffer->Unlock()) ) issuc = false;

	
	RELEASE_OBJECT(gIndexBuffer);
	if ( FAILED(EngineManager::Engine->gDevice->CreateIndexBuffer(DescBuffer.Size, 
		DescBuffer.Usage, DescBuffer.Format, DescBuffer.Pool, 
		&gIndexBuffer, NULL)) 
	) issuc = false;
	
	pindices = 0;

	if ( FAILED(gIndexBuffer->Lock(0, 0, (void**)&pindices, 0)) ) issuc = false;
	memcpy(pindices, cpybuf.Arr, DescBuffer.Size);
	if ( FAILED(gIndexBuffer->Unlock()) ) issuc = false;
	
	if ( !issuc )
	{
		RELEASE_OBJECT(gIndexBuffer);
		MTHROW(InvalidOperation, "Failed");
	}
}

void IndexBuffer::OnSetAsCurrentIndexBuffer()
{
	Renderer::SetRawIndexBuffer(gIndexBuffer);
}

void IndexBuffer::OnUnsetAsCurrentIndexBuffer()
{
	Renderer::SetRawIndexBuffer(NULL);
}

IDirect3DIndexBuffer9 *IndexBuffer::GetIndexBuffer()
{
	return gIndexBuffer;
}