#include "StdAfx.hpp"
#include "Sound.h"
#include "SoundFile.h"

using namespace Mgine;
using namespace Util;

bool			Sound::IsInited;
LPDIRECTSOUND8	Sound::gDS;
HWND			Sound::hWnd;
Queue<SoundBuffer*>				*Sound::pQueueSound;
Queue<Sound::Category*>	Sound::QueueCategory;


void Sound::Init(HWND hWnd)
{
	ATHROWR(!IsInited, "Already inited");
	MLOG("Begin");

	HRESULT hr = 0;
	LPDIRECTSOUNDBUFFER soundBuffer = 0;

	try
	{
		Sound::hWnd = hWnd;
		ATHROWR(SUCCEEDED(hr = DirectSoundCreate8(NULL, &gDS, 0)), "Failed CDS. hr:&.08x", hr);
		ATHROWR(SUCCEEDED(hr = gDS->SetCooperativeLevel(Sound::hWnd,DSSCL_NORMAL)), "Failed DS_SCL. hr:%.08x", hr);

		//ReaderIntegration::Init();

		IsInited = true;
		QueueSound();
	}
	catch ( BaseException & )
	{
		MLOG("Failed. Please check your audio device is now working properly.");
		RELEASE_OBJECT(soundBuffer);
		Uninit();
		//throw;
	}
}

void Sound::Uninit()
{
	int i;
	for ( i=0; i < QueueSound().Size; i++ )
		if ( QueueSound().IsElementExisting(i) )
			QueueSound()[i]->Uninit();

	for ( i=0; i < QueueCategory.Size; i++ )
		if ( QueueCategory.IsElementExisting(i) )
		{
			 delete QueueCategory[i];
			 QueueCategory.Sub(i);
		}
	RELEASE_OBJECT(gDS);
	

	IsInited = false;
}

void Sound::OnAppFinish()
{
	if ( pQueueSound ) delete pQueueSound;
	pQueueSound = 0;
}

void Sound::Update(double ElapsedTime)
{
	//int i;
	//for ( i=0; i < QueueSound().Size; i++ ){
		//if ( QueueSound().IsElementExisting(i) ){
			//QueueSound()[i]->Update();
		//}
	//}
}

DWORD Sound::AddCategory()
{
	return QueueCategory.Add(new Category());
}


bool Sound::LoadSoundFile(wchar_t *FileName, MGINE_SOUND_DESC *Result)
{
	MGINE_SOUND_DESC desc; memset(&desc, 0, sizeof(desc));
	DWORD filesize, chkformat, dw;

	HANDLE hFile = CreateFileW(FileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_ARCHIVE, 0);
	if ( hFile == INVALID_HANDLE_VALUE ) goto cleanup;


	if ( !ReadFile(hFile, &chkformat, 4, &dw, 0) ) goto cleanup;

	switch ( chkformat )
	{
		case 'FFIR':
			if ( !ReadFile(hFile, &filesize, 4, &dw, 0 ) ) goto cleanup;
			filesize += 8;
			if ( GetFileSize(hFile, 0) < filesize ) goto cleanup;
			if ( !ReadFile(hFile, &chkformat, 4, &dw, 0) ) goto cleanup;
			if ( chkformat != 'EVAW' ) goto cleanup;
			
			{
				//it is wavefile
				//find for chunks
				DWORD samplerate = 0, bytespersec = 0;
				WORD blockalign = 0, nchannel = 0, significantbps = 0, compresscode = 0;

				for ( ;; )
				{
					DWORD chunkName = 0, chunkSize = 0;
					DWORD read = 0;
					if ( !ReadFile(hFile, &chunkName, 4, &dw, 0) ) break;
					if ( !ReadFile(hFile, &chunkSize, 4, &dw, 0) ) goto cleanup;
					if ( read >= chunkSize ) break;

					switch ( chunkName )
					{
						case ' tmf':
							/*
								Format Chunk - "fmt "
								The format chunk contains information about how the waveform data is stored and should be played back including the type of compression used, number of channels, sample rate, bits per sample and other attributes.
 
								Offset	Size	Description					Value
								0x00	4		Chunk ID					"fmt " (0x666D7420)
								0x04	4		Chunk Data Size				16 + extra format bytes
								0x08	2		Compression code			1 - 65,535
								0x0a	2		Number of channels			1 - 65,535
								0x0c	4		Sample rate					1 - 0xFFFFFFFF
								0x10	4		Average bytes per second	1 - 0xFFFFFFFF
								0x14	2		Block align					1 - 65,535
								0x16	2		Significant bits per sample	2 - 65,535
								0x18	2		Extra format bytes			0 - 65,535
								0x1a			Extra format bytes * 
							*/
							if ( !ReadFile(hFile, &compresscode, 2, &dw, 0) ) goto cleanup; read += dw;
							if ( read >= chunkSize ) break;

							if ( !ReadFile(hFile, &nchannel, 2, &dw, 0) ) goto cleanup; read += dw;
							if ( read >= chunkSize ) break;

							if ( !ReadFile(hFile, &samplerate, 4, &dw, 0) ) goto cleanup; read += dw;
							if ( read >= chunkSize ) break;

							if ( !ReadFile(hFile, &bytespersec, 4, &dw, 0) ) goto cleanup; read += dw;
							if ( read >= chunkSize ) break;

							if ( !ReadFile(hFile, &blockalign, 2, &dw, 0) ) goto cleanup; read += dw;
							if ( read >= chunkSize ) break;

							if ( !ReadFile(hFile, &significantbps, 2, &dw, 0) ) goto cleanup; read += dw;
							if ( read >= chunkSize ) break;

							if ( !SetFilePointer(hFile, chunkSize - read, 0, SEEK_CUR ) ) goto cleanup;
							break;
						case 'atad':
							if ( desc.Buffer )
							{
								MLOG("data chunk is more than one.");
								goto cleanup;
							}
							desc.BufferSize = chunkSize;
							desc.Buffer = new BYTE[chunkSize];
							if ( !ReadFile(hFile, desc.Buffer, chunkSize, &dw, 0) ) goto cleanup;
							break;
						default:

							break;
					}
				}
				if ( compresscode != 1 ) goto cleanup;
				desc.Format = SOUND_FORMAT_WAV;
				desc.Wave.cbSize = sizeof(tWAVEFORMATEX);
				desc.Wave.nAvgBytesPerSec = bytespersec;
				desc.Wave.nBlockAlign = blockalign;
				desc.Wave.nChannels = nchannel;
				desc.Wave.nSamplesPerSec = samplerate;
				desc.Wave.wBitsPerSample = significantbps;
				desc.Wave.wFormatTag = compresscode; 
			}
			break;
		default: goto cleanup;
	};

	CloseHandle(hFile);
	memcpy(Result, &desc, sizeof(desc));
	return true;

cleanup:
	MLOG("Failed");
	if ( hFile == INVALID_HANDLE_VALUE ) hFile = 0;
	if ( hFile ) CloseHandle(hFile);
	if ( desc.Buffer ) delete desc.Buffer;

	desc.Buffer = 0;
	desc.BufferSize = 0;

	return false;
}

SoundFile *SoundFile::Open(const std::wstring & FileName)
{
	MTHROW(NotImplemented, "Not Supported or Implemented.", );
	//Mp3File *file = new Mp3File();
	//file->Init(FileName);
	//return file;
}


Sound::Category::Category()
{
	CategoryNo = QueueCategory.Count;
	Multiply = 0;
}

void Sound::Category::AdjustMultiply(float Multiply)
{
	if ( !Sound::IsInited ) return;
	if ( Multiply < 0 ) Multiply = 0;
	if ( Multiply > 1 ) Multiply = 1;

	this->Multiply = Multiply;
	int i;
	for ( i=0; i < pQueueSound->Size; i++ )
	{
		if ( pQueueSound->IsElementExisting(i) )
		{
			SoundBuffer *sound = ((SoundBuffer*)pQueueSound->Get(i));
			if ( sound->CategoryNo == CategoryNo )
				sound->ResetVolume();
		}
	}

}



SoundBuffer::~SoundBuffer()
{
	Sound::QueueSound().Sub(SoundIndex);
	Uninit();
}

void SoundBuffer::InitDescBuffer()
{
	memset(&DescBuffer, 0, sizeof(DSBUFFERDESC));
	DescBuffer.dwSize = sizeof(DescBuffer);
	DescBuffer.dwFlags = DSBCAPS_CTRLVOLUME | DSBCAPS_GLOBALFOCUS | DSBCAPS_GETCURRENTPOSITION2;// | DSBCAPS_CTRL3D;// | DSBCAPS_CTRLFREQUENCY /*| DSBCAPS_CTRLFX*/ | DSBCAPS_CTRLPAN | DSBCAPS_CTRLPOSITIONNOTIFY | DSBCAPS_CTRLVOLUME | DSBCAPS_STATIC | DSBCAPS_GETCURRENTPOSITION2;
	if ( EngineManager::Core->IsVistaDwmFeatureEnabled ) DescBuffer.dwFlags |= DSBCAPS_TRUEPLAYPOSITION;
	if ( InitParam.Is3D ) DescBuffer.dwFlags |= DSBCAPS_CTRL3D;
	if ( InitParam.IsNotification ) DescBuffer.dwFlags |= DSBCAPS_CTRLPOSITIONNOTIFY;
	if ( !InitParam.IsStreaming ) DescBuffer.dwFlags |= DSBCAPS_STATIC;

	DescBuffer.guid3DAlgorithm = GUID_NULL;
	if ( InitParam.Is3D ) DescBuffer.guid3DAlgorithm = DS3DALG_HRTF_LIGHT; //DS3DALG_HRTF_FULL;

	DescBuffer.dwBufferBytes = DescFile.BufferSize;
	DescBuffer.lpwfxFormat = &DescFile.Wave;
}

void SoundBuffer::Init(sf::priv::SoundFile & File)
{
	if ( !Sound::IsInited ) return; // I will handle this far far later.
	if ( IsInited ) Uninit();

	try
	{
		//ATHROW(Sound::LoadSoundFile(FileName, &DescFile));
		if ( InitParam.IsStreaming ) MTHROW(InvalidOperation, "InitParam.IsStreaming is set. Use AddStreamSource() instead.", );
		ATHROW(File.Read((sf::Int16*)(DescFile.Buffer = new BYTE[16384]), 16384/sizeof(sf::Int16)));
		InitDescBuffer();

		void *ptrAudio1, *ptrAudio2;
		DWORD sizeAudio1, sizeAudio2;
	
		ATHROW( SUCCEEDED(Sound::gDS->CreateSoundBuffer(&DescBuffer, &gSoundBuffer, 0)) );

		HRESULT hr = DS_OK;

		do
		{
			if ( hr == DSERR_BUFFERLOST ) ATHROW(SUCCEEDED(gSoundBuffer->Restore()));
			hr = gSoundBuffer->Lock(0, DescFile.BufferSize, &ptrAudio1, &sizeAudio1, &ptrAudio2, &sizeAudio2, 0);
		} while ( hr == DSERR_BUFFERLOST );

		memcpy(ptrAudio1, (BYTE*)DescFile.Buffer, sizeAudio1);
		memcpy(ptrAudio2, (BYTE*)DescFile.Buffer + sizeAudio1, sizeAudio2);

		ATHROW(SUCCEEDED(gSoundBuffer->Unlock(ptrAudio1, sizeAudio1, ptrAudio2, sizeAudio2)));
		
		gSoundBuffer->SetCurrentPosition(0);
		//gSoundBuffer->SetVolume(-1000);
		if ( InitParam.Is3D ) ATHROW(SUCCEEDED(gSoundBuffer->QueryInterface(IID_IDirectSound3DBuffer, (void**)&gSoundBuffer3D)));
		if ( InitParam.IsNotification ) ATHROW(SUCCEEDED(gSoundBuffer->QueryInterface(IID_IDirectSoundNotify, (void**)&gDsNotification)));
	
		IsInited = true;
		ResetVolume();
	}
	catch ( BaseException & )
	{
		MLOG("Failed");
		Uninit();
		throw;
	}
}



void SoundBuffer::Init(const std::stringstream & Buffer)
{
	if ( !Sound::IsInited ) return;
	if ( IsInited ) Uninit();

	try
	{
		std::string buf = Buffer.str();
		if ( InitParam.IsStreaming ) MTHROW(InvalidOperation, "InitParam.IsStreaming is set. Use AddStreamSource() instead.", );

		DescFile.Buffer = new BYTE[buf.length()];
		memcpy(DescFile.Buffer, buf.data(), buf.length()*sizeof(char));

		DescFile.BufferSize = buf.length();
		DescFile.Format = SOUND_FORMAT_WAV;
		DescFile.Wave.cbSize = sizeof(DescFile.Wave);
		DescFile.Wave.nChannels = 2;
		DescFile.Wave.nBlockAlign = 2*DescFile.Wave.nChannels;
		DescFile.Wave.nSamplesPerSec = 44100;
		DescFile.Wave.nAvgBytesPerSec = DescFile.Wave.nBlockAlign*DescFile.Wave.nSamplesPerSec;
		DescFile.Wave.wBitsPerSample = DescFile.Wave.nBlockAlign*8/DescFile.Wave.nChannels;
		DescFile.Wave.wFormatTag = 1;

		InitDescBuffer();

		void *ptrAudio1, *ptrAudio2;
		DWORD sizeAudio1, sizeAudio2;
	
		ATHROW(SUCCEEDED(Sound::gDS->CreateSoundBuffer(&DescBuffer, &gSoundBuffer, 0)));

		HRESULT hr = DS_OK;
		do {
			if ( hr == DSERR_BUFFERLOST ) ATHROW(SUCCEEDED(gSoundBuffer->Restore()));
			if ( Buffer ) hr = gSoundBuffer->Lock(0, DescFile.BufferSize, &ptrAudio1, &sizeAudio1, &ptrAudio2, &sizeAudio2, 0);
		} while ( hr == DSERR_BUFFERLOST );

		if ( Buffer )
		{
			memcpy(ptrAudio1, (BYTE*)DescFile.Buffer, sizeAudio1);
			memcpy(ptrAudio2, (BYTE*)DescFile.Buffer + sizeAudio1, sizeAudio2);
			ATHROW(SUCCEEDED(gSoundBuffer->Unlock(ptrAudio1, sizeAudio1, ptrAudio2, sizeAudio2)));
		}

		gSoundBuffer->SetCurrentPosition(0);
		//gSoundBuffer->SetVolume(-1000);
		if ( InitParam.Is3D ) ATHROW(SUCCEEDED(gSoundBuffer->QueryInterface(IID_IDirectSound3DBuffer, (void**)&gSoundBuffer3D)));
		if ( InitParam.IsNotification ) ATHROW(SUCCEEDED(gSoundBuffer->QueryInterface(IID_IDirectSoundNotify, (void**)&gDsNotification)));
	
		IsInited = true;
		ResetVolume();
	}
	catch ( BaseException & )
	{
		MLOG("Failed");
		Uninit();
		throw;
	}
}

void SoundBuffer::Init(Sound::TSample *Buffer, UINT BufferSampleUnitCount)
{
	if ( !Sound::IsInited ) return;
	if ( IsInited ) Uninit();
	//ATHROW(BufferSampleUnitCount % Sound::UnitChannelCount == 0);

	HRESULT hr = DS_OK;
	try
	{
		if ( InitParam.IsStreaming ) MTHROW(InvalidOperation, "InitParam.IsStreaming is set. Use AddStreamSource() instead.", );

		DescFile.Buffer = (BYTE*)new Sound::TSample[BufferSampleUnitCount*Sound::UnitChannelCount];
		if ( !Buffer ) memset(DescFile.Buffer, 0, BufferSampleUnitCount*Sound::UnitSize);
		else memcpy(DescFile.Buffer, Buffer, BufferSampleUnitCount*Sound::UnitSize);

		DescFile.BufferSize = BufferSampleUnitCount*Sound::UnitSize;
		DescFile.Format = SOUND_FORMAT_WAV;
		DescFile.Wave.cbSize = sizeof(DescFile.Wave);
		DescFile.Wave.nChannels = Sound::UnitChannelCount;
		DescFile.Wave.nBlockAlign = sizeof(Sound::TSample)*DescFile.Wave.nChannels;
		DescFile.Wave.nSamplesPerSec = Sound::UnitSampleRate;
		DescFile.Wave.nAvgBytesPerSec = DescFile.Wave.nBlockAlign*DescFile.Wave.nSamplesPerSec;
		DescFile.Wave.wBitsPerSample = DescFile.Wave.nBlockAlign*8/DescFile.Wave.nChannels;
		DescFile.Wave.wFormatTag = 1;

		InitDescBuffer();

		void *ptrAudio1, *ptrAudio2;
		DWORD sizeAudio1, sizeAudio2;
	
		ATHROW(SUCCEEDED(hr = Sound::gDS->CreateSoundBuffer(&DescBuffer, &gSoundBuffer, 0)));

		do {
			if ( hr == DSERR_BUFFERLOST ) ATHROW(SUCCEEDED(gSoundBuffer->Restore()));
			if ( Buffer ) hr = gSoundBuffer->Lock(0, DescFile.BufferSize, &ptrAudio1, &sizeAudio1, &ptrAudio2, &sizeAudio2, 0);
		} while ( hr == DSERR_BUFFERLOST );

		if ( Buffer )
		{
			memcpy(ptrAudio1, (BYTE*)DescFile.Buffer, sizeAudio1);
			memcpy(ptrAudio2, (BYTE*)DescFile.Buffer + sizeAudio1, sizeAudio2);
			ATHROW(SUCCEEDED(gSoundBuffer->Unlock(ptrAudio1, sizeAudio1, ptrAudio2, sizeAudio2)));
		}

		gSoundBuffer->SetCurrentPosition(0);
		//gSoundBuffer->SetVolume(-1000);
		if ( InitParam.Is3D ) ATHROW(SUCCEEDED(gSoundBuffer->QueryInterface(IID_IDirectSound3DBuffer, (void**)&gSoundBuffer3D)));
		if ( InitParam.IsNotification ) ATHROW(SUCCEEDED(gSoundBuffer->QueryInterface(IID_IDirectSoundNotify, (void**)&gDsNotification)));
	
		IsInited = true;
		ResetVolume();
	}
	catch ( BaseException & )
	{
		MLOG("Failed");
		Uninit();
		throw;
	}
}

void SoundBuffer::Uninit()
{
	if ( IsInited ) gSoundBuffer->Stop();

	IsInited = false;

	RELEASE_OBJECT(gSoundBuffer);
	RELEASE_OBJECT(gSoundBuffer3D);
	RELEASE_OBJECT(gDsNotification);
	ListStreamFile.clear();
	CurrentStreamingPos = 0;
	DescFile.Uninit();
}

void SoundBuffer::OnDeviceCreate()
{
	// TODO: improve code quality greatly
	if ( !Sound::IsInited ) return; // I will handle this far far later, told you already
	if ( !IsInited ) return;
	
	IDirectSoundBuffer *pSoundBuffer = 0;

	void *ptrAudio1, *ptrAudio2;
	DWORD sizeAudio1, sizeAudio2;

	void *ptrOld1, *ptrOld2;
	DWORD sizeOld1, sizeOld2;
	
	DWORD play = -1, write = -1;

	if ( FAILED(Sound::gDS->CreateSoundBuffer(&DescBuffer, &pSoundBuffer, 0) ) ) goto cleanup;
	if ( FAILED(gSoundBuffer->GetCurrentPosition(&play, &write)) ) goto cleanup;
	pSoundBuffer->SetCurrentPosition(play);
	if ( FAILED(pSoundBuffer->Lock(0, DescFile.BufferSize, &ptrAudio1, &sizeAudio1, &ptrAudio2, &sizeAudio2, 0)) ) goto cleanup;
	if ( FAILED(gSoundBuffer->Lock(0, DescFile.BufferSize, &ptrOld1, &sizeOld1, &ptrOld2, &sizeOld2, 0)) ) goto cleanup;
	
	if ( sizeOld1 != sizeAudio1 || sizeOld2 != sizeAudio2 ) goto cleanup;

	memcpy(ptrAudio1, ptrOld1, sizeOld1);
	memcpy(ptrAudio2, ptrOld2, sizeOld2);

	if ( FAILED(gSoundBuffer->Unlock(ptrOld1, sizeOld1, ptrOld2, sizeOld2)) ) goto cleanup;
	if ( FAILED(pSoundBuffer->Unlock(ptrAudio1, sizeAudio1, ptrAudio2, sizeAudio2)) ) goto cleanup;

	gSoundBuffer->Release();
	gSoundBuffer = pSoundBuffer;
	pSoundBuffer = 0;

	ResetVolume();

	return ;

cleanup:
	MLOG("Failed");
	MessageBox(0, L"OnDeviceCreate() failed.", L"Forgive me", 0);
	__asm int 3;
	return ;
}

void SoundBuffer::LockWriteSample(BYTE *Buffer, DWORD Count, int Position)
{
	ATHROW(IsInited);
	ATHROW(Buffer);
	ATHROW(Count);
	ATHROW(DescBuffer.dwBufferBytes >= Count);

	bool issucceeded = true;
	void *pvaudio1 = 0, *pvaudio2 = 0;
	DWORD sizeAudio1 = 0, sizeAudio2 = 0;

	if ( FAILED(gSoundBuffer->Lock(Position, Count, &pvaudio1, &sizeAudio1, &pvaudio2, &sizeAudio2, 0)) ) issucceeded = false;
	memcpy(pvaudio1, Buffer, sizeAudio1);
	memcpy(pvaudio2, Buffer + sizeAudio1, sizeAudio2);
	if ( FAILED(gSoundBuffer->Unlock(pvaudio1, sizeAudio1, pvaudio2, sizeAudio2)) ) issucceeded = false;
	ATHROWR(issucceeded, "Failed to Lock and Unlock DSB.");
} 

void SoundBuffer::LockReadSample(BYTE *Buffer, DWORD Count, int Position)
{
	ATHROW(IsInited);
	ATHROW(Buffer);
	ATHROW(Count);
	ATHROW(DescBuffer.dwBufferBytes >= Count);

	bool issucceeded = true;
	void *pvaudio1 = 0, *pvaudio2 = 0;
	DWORD sizeAudio1 = 0, sizeAudio2 = 0;

	
	if ( FAILED(gSoundBuffer->Lock(Position, Count, &pvaudio1, &sizeAudio1, &pvaudio2, &sizeAudio2, 0)) ) issucceeded = false;
	memcpy(Buffer, pvaudio1, sizeAudio1);
	memcpy(Buffer + sizeAudio1, pvaudio2, sizeAudio2);
	if ( FAILED(gSoundBuffer->Unlock(pvaudio1, sizeAudio1, pvaudio2, sizeAudio2)) ) issucceeded = false;
	
	ATHROWR(issucceeded, "Failed to Lock and Unlock DSB.");
} 

void SoundBuffer::Play(bool IsLoop)
{
	// TODO: improve its design greatly!!
	if ( !Sound::IsInited ) return;
	ATHROW(IsInited);
	ATHROW(SUCCEEDED(gSoundBuffer->Play(0, 0, DSBPLAY_LOOPING * IsLoop)));
}

void SoundBuffer::Stop()
{
	if ( !Sound::IsInited ) return;
	ATHROW(IsInited);
	ATHROW(SUCCEEDED(gSoundBuffer->Stop()));
}

void SoundBuffer::SetPos(DWORD Pos)
{
	if ( !Sound::IsInited ) return;
	ATHROW(IsInited);
	ATHROW(SUCCEEDED(gSoundBuffer->SetCurrentPosition(Pos)));
}

DWORD SoundBuffer::GetPos()
{
	if ( !Sound::IsInited ) return 0;
	ATHROW(IsInited);

	DWORD play, write;
	ATHROW(SUCCEEDED(gSoundBuffer->GetCurrentPosition(&play, &write)));
	return play;
}

void SoundBuffer::SetVolume(float Volume)
{
	if ( !Sound::IsInited ) return;
	ATHROW(IsInited);

	if ( Volume < 0 ) Volume = 0;
	if ( Volume < 1 ) Volume = 1;
	this->Volume = Volume;

	if ( 0 <= CategoryNo && CategoryNo < Sound::QueueCategory.Count )
		Volume *= Sound::QueueCategory.Get(CategoryNo)->Multiply;
	Volume = 0.5f+log10f(Volume*10)/2;
	int intvolume = (int)(Volume*10000-10000);

	if ( intvolume <= -10000 ) intvolume = -9999;
	ATHROW(SUCCEEDED(gSoundBuffer->SetVolume(intvolume)));
}

void SoundBuffer::ResetVolume()
{
	if ( !Sound::IsInited ) return;
	ATHROW(IsInited);
	return SetVolume(GetVolume());
}

float SoundBuffer::GetVolume()
{ return Volume; }

void SoundBuffer::SetPan(int Pan)
{
	if ( !Sound::IsInited ) return;
	ATHROW(IsInited);
	ATHROW(SUCCEEDED(gSoundBuffer->SetPan(Pan)));
}

bool SoundBuffer::GetIsPlaying()
{
	if ( !Sound::IsInited ) return false;
	ATHROW(IsInited);

	DWORD st = 0;
	ATHROW(SUCCEEDED(gSoundBuffer->GetStatus(&st)));
	if ( st & DSBSTATUS_PLAYING ) return true;
	else return false;
}


