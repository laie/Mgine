#include "StdAfx.hpp"
#include "Music.h"
//#include "MusicAnalysis.h"

using namespace Mgine;
using namespace Util;

const double MusicManager::BeatDetectionDecayPerSecond = 2.;
const double MusicManager::BeatDetectionMinGap = 1000.*60./260./4.; // 16 beat of over 260: 57.692307692307... ms

CRITICAL_SECTION	MusicManager::csStream;
CapacityStack<BYTE> MusicManager::StackPCM(StreamingUnitCount*Sound::UnitSize);
SoundBuffer MusicManager::SoundStream;
HANDLE MusicManager::hEventNotification[4];
DWORD MusicManager::tidThread;
HANDLE MusicManager::hThread;
bool MusicManager::IsInited;
bool MusicManager::IsTerminating;
int  MusicManager::StreamingState;

MusicManager::STREAMING_SLICE MusicManager::SliceStreaming[2];

const double MusicManager::EQUAL_LOUDNESS_CURVE_INFO::F[] =
		{
			20, 25, 31.5, 40, 50, 63, 80, 100, 125, 160, 200, 250, 315, 400, 500, 630, 800,
			1000, 1250, 1600, 2000, 2500, 3150, 4000, 5000, 6300, 8000, 10000, 12500 
		};

const double MusicManager::EQUAL_LOUDNESS_CURVE_INFO::AF[] =
		{
			0.532, 0.506, 0.480, 0.455, 0.432, 0.409, 0.387, 0.367, 0.349, 0.330, 0.315,
			0.301, 0.288, 0.276, 0.267, 0.259, 0.253, 0.250, 0.246, 0.244, 0.243, 0.243,
			0.243, 0.242, 0.242, 0.245, 0.254, 0.271, 0.301
		};

const double MusicManager::EQUAL_LOUDNESS_CURVE_INFO::LU[] =
		{
			-31.6, -27.2, -23.0, -19.1, -15.9, -13.0, -10.3, -8.1, -6.2, -4.5, -3.1, 
			-2.0,  -1.1,  -0.4,   0.0,   0.3,   0.5,   0.0, -2.7, -4.1, -1.0,  1.7,
			2.5,   1.2,  -2.1,  -7.1, -11.2, -10.7,  -3.1,
		};

const double MusicManager::EQUAL_LOUDNESS_CURVE_INFO::TF[] =
		{
			78.5,  68.7,  59.5,  51.1,  44.0,  37.5,  31.5,  26.5,  22.1,  17.9,  14.4,
			11.4,   8.6,   6.2,   4.4,   3.0,   2.2,   2.4,   3.5,   1.7,  -1.3,  -4.2,
			-6.0,  -5.4,  -1.5,   6.0,  12.6,  13.9,  12.3
		};

static_assert(29 == COUNTOF(MusicManager::EQUAL_LOUDNESS_CURVE_INFO::F) && sizeof(MusicManager::EQUAL_LOUDNESS_CURVE_INFO::F) == sizeof(MusicManager::EQUAL_LOUDNESS_CURVE_INFO::AF) && sizeof(MusicManager::EQUAL_LOUDNESS_CURVE_INFO::AF) == sizeof(MusicManager::EQUAL_LOUDNESS_CURVE_INFO::LU) && sizeof(MusicManager::EQUAL_LOUDNESS_CURVE_INFO::LU) == sizeof(MusicManager::EQUAL_LOUDNESS_CURVE_INFO::TF),
	"Equal Loudness Curve Data Lengths are not equals each other.");

MusicManager::EQUAL_LOUDNESS_CURVE_INFO MusicManager::EqualLoudnessCurve;

void MusicManager::EQUAL_LOUDNESS_CURVE_INFO::Init()
{
	static_assert(0 <= Phon && Phon <= 90, "Phon is invalid. 0~90.");
	if ( IsInited ) return;

	int i;
	for ( i=0; i < COUNTOF(MusicManager::EQUAL_LOUDNESS_CURVE_INFO::Curve); i++ )
	{
		double af = 4.47E-3 * (pow(10., 0.025*Phon) - 1.15) + pow(0.4*pow(10.,((TF[i]+LU[i])/10)-9 ), AF[i]);
		double db = ((10./AF[i])*log10(af)) - LU[i] + 94;
		Curve[i].Y = pow(10. ,(db-94)/10);
		
		Curve[i].SPLdB = db;
		Curve[i].Frequency = F[i];
	}
	IsInited = true;
}

double MusicManager::EQUAL_LOUDNESS_CURVE_INFO::GetCurveValue(double X)
{
	int indexpre = 0, indexpost = COUNTOF(Curve);
	int index = indexpost/2;
	int indexmostclose = -1;

	if ( indexpost < 0 ) return 0.;
	//if ( Phon < 0 || 91 <= Phon ) return 0.;

	double objective = X;

	while ( indexpre < indexpost ) 
	{
		if ( objective < Curve[index].Frequency )
		{
			indexpost = index;
			index = indexpre + (index-indexpre)/2;
		} else
		{
			indexmostclose = index;
			indexpre = index+1;
			index = index + (indexpost-index)/2;
		}
	}

	if ( indexmostclose != -1 )
	{
		assert(0 <= indexmostclose);
		assert(indexmostclose < COUNTOF(Curve));
	}
	if ( indexmostclose != -1 )
	{
		double degree = (X-Curve[indexmostclose].Frequency)/(Curve[indexmostclose+1].Frequency-Curve[indexmostclose].Frequency);
		//double ret = Curve[indexmostclose].Y*(1-degree)+Curve[indexmostclose+1].Y*degree;
		double ret = Curve[indexmostclose].SPLdB*(1-degree)+Curve[indexmostclose+1].SPLdB*degree;
		//Curve[indexmostclose].Y
		return ret;
	} else return 0.;
}

DWORD MusicManager::ThreadStream(void *Arg)
{
	BYTE procbuf[StreamingUnitCount*Sound::UnitSize/2];
	int procbufcount = 0;
	for ( ; !IsTerminating; )
	{
		switch ( WaitForMultipleObjects(4, hEventNotification, false, 100) )
		{
			case WAIT_OBJECT_0:
				EnterCriticalSection(&csStream);
				StreamingState = 0;
				SliceStreaming[0].IsPlaying = true;
				SliceStreaming[1].IsPlaying = false;
				SliceStreaming[1].PCMFill = 0;
				LeaveCriticalSection(&csStream);
				break;
			case WAIT_OBJECT_0+1:
				EnterCriticalSection(&csStream);
				StreamingState = 1;
				if ( StackPCM.GetElementCount() )
				{
					int available = sizeof(SliceStreaming[1].PCM) - SliceStreaming[1].PCMFill;
					if ( SliceStreaming[1].PCMFill + available >= sizeof(SliceStreaming[1]) )
						available = sizeof(SliceStreaming[1]) - SliceStreaming[1].PCMFill;
					if ( available )
					{
						procbufcount = StackPCM.Pick(procbuf, available);
					
						memcpy(&SliceStreaming[1].PCM[SliceStreaming[1].PCMFill], procbuf, procbufcount);
						SliceStreaming[1].PCMFill += procbufcount;
					}
					memset(&SliceStreaming[1].PCM[SliceStreaming[1].PCMFill], 0, sizeof(SliceStreaming[1].PCM)-SliceStreaming[1].PCMFill);
					SoundStream.LockWriteSample(SliceStreaming[1].PCM, sizeof(SliceStreaming[1].PCM)/*PCMFILL*/, 2*StreamingUnitCount*Sound::UnitSize/4);
					
				} else 
				{
					memset(procbuf, 0, sizeof(procbuf));
					SoundStream.LockWriteSample(procbuf, sizeof(procbuf), 2*StreamingUnitCount*Sound::UnitSize/4);
				}
				LeaveCriticalSection(&csStream);
				break;
			case WAIT_OBJECT_0+2:
				EnterCriticalSection(&csStream);
				StreamingState = 2;
				SliceStreaming[1].IsPlaying = true;
				SliceStreaming[0].IsPlaying = false;
				SliceStreaming[0].PCMFill = 0;
				LeaveCriticalSection(&csStream);
				break;
			case WAIT_OBJECT_0+3:
				EnterCriticalSection(&csStream);
				StreamingState = 3;
				if ( StackPCM.GetElementCount() )
				{
					int available = sizeof(SliceStreaming[0].PCM) - SliceStreaming[0].PCMFill;
					if ( SliceStreaming[0].PCMFill + available >= sizeof(SliceStreaming[0]) )
						available = sizeof(SliceStreaming[0]) - SliceStreaming[0].PCMFill;
					if ( available )
					{
						procbufcount = StackPCM.Pick(procbuf, available);
					
						memcpy(&SliceStreaming[0].PCM[SliceStreaming[0].PCMFill], procbuf, procbufcount);
						SliceStreaming[0].PCMFill += procbufcount;
					}
					memset(&SliceStreaming[0].PCM[SliceStreaming[0].PCMFill], 0, sizeof(SliceStreaming[0].PCM)-SliceStreaming[0].PCMFill);
					SoundStream.LockWriteSample(SliceStreaming[0].PCM, sizeof(SliceStreaming[0].PCM)/*PCMFILL*/, 0*StreamingUnitCount*Sound::UnitSize/4);
					
				} else 
				{
					memset(procbuf, 0, sizeof(procbuf));
					SoundStream.LockWriteSample(procbuf, sizeof(procbuf), 0*StreamingUnitCount*Sound::UnitSize/4);
				}
				LeaveCriticalSection(&csStream);
				break;
			case WAIT_TIMEOUT:
				{
					CallbackStreamUpdate streamupdate = EngineManager::Setting->GetSetting().OnStreamUpdateAsync;
					if ( streamupdate )
					{
						LockBlock Lock(csStream);
						if ( SoundStream.GetIsPlaying() )
							streamupdate();
					}
				}
				break;
			default:break;
		}
	}
	return 0;
}

void MusicManager::Init()
{
	if ( IsInited ) MTHROW(InvalidStatus, "Already inited");
	MLOG("Begin");

	try
	{
		InitializeCriticalSection(&csStream);
		if ( !EqualLoudnessCurve.IsInited ) EqualLoudnessCurve.Init();
		
		int i;
		for ( i=0; i < COUNTOF(hEventNotification); i++ )
		{
			hEventNotification[i] = CreateEvent(NULL, false, false, NULL);
			if ( !hEventNotification[i] ) MTHROW(Win32Error, "CE");
		}

		SoundStream.InitParam.IsNotification = true;
		SoundStream.Init(0, StreamingUnitCount); // ) MTHROW(InvalidOperation, "SS.IFM");

		DSBPOSITIONNOTIFY notify[4];
		notify[0].dwOffset = 0*StreamingUnitCount*Sound::UnitSize/4;
		notify[0].hEventNotify = hEventNotification[0];
		notify[1].dwOffset = 1*StreamingUnitCount*Sound::UnitSize/4;
		notify[1].hEventNotify = hEventNotification[1];
		notify[2].dwOffset = 2*StreamingUnitCount*Sound::UnitSize/4;
		notify[2].hEventNotify = hEventNotification[2];
		notify[3].dwOffset = 3*StreamingUnitCount*Sound::UnitSize/4;
		notify[3].hEventNotify = hEventNotification[3];

		if ( FAILED(SoundStream.SetNotificationPositions(4, notify)) )
			MTHROW(DirectXError, "SS.DSN_SNP");

		//hThread = CreateThread(NULL, 0, ThreadStream, 0, 0, &tidThread);
		//if ( !hThread ) MTHROW(Win32Error, "CT");

		IsInited = true;
	}
	catch ( BaseException & )
	{
		MLOG("Failed");
		Uninit();
		throw;
	}
}

void MusicManager::Uninit()
{
	IsInited = false;
	IsTerminating = true;

	WaitForSingleObject(hThread, INFINITE);
	CloseHandle(hThread);

	SoundStream.Uninit();

	int i;
	for ( i=0; i < COUNTOF(hEventNotification); i++ )
	{
		CloseHandle(hEventNotification[i]);
		hEventNotification[i] = NULL;
	}

	DeleteCriticalSection(&csStream);

	IsTerminating = false;
}

void MusicManager::Play()
{
	ATHROW(IsInited);
	try
	{
		BYTE procbuf[StreamingUnitCount*Sound::UnitSize/2];
		int procbufcount = 0;
		EnterCriticalSection(&csStream);
		if ( !SoundStream.GetIsPlaying() )
		{
			CallbackStreamUpdate streamupdate = EngineManager::Setting->GetSetting().OnStreamUpdateAsync;
			if ( streamupdate ) streamupdate();

			StreamingState = 0;
			SliceStreaming[0].IsPlaying = true;
			SliceStreaming[0].PCMFill = StackPCM.Pick(SliceStreaming[0].PCM, sizeof(SliceStreaming[0].PCM));
			SoundStream.LockWriteSample(SliceStreaming[0].PCM, SliceStreaming[0].PCMFill, 0);
			if ( SliceStreaming[0].PCMFill < sizeof(SliceStreaming[0].PCM) )
			{
				memset(procbuf, 0, sizeof(SliceStreaming[0].PCM)-SliceStreaming[0].PCMFill);
				SoundStream.LockWriteSample(procbuf, sizeof(SliceStreaming[0].PCM)-SliceStreaming[0].PCMFill, SliceStreaming[0].PCMFill);
			}
			SliceStreaming[1].IsPlaying = false;
			SliceStreaming[1].PCMFill = 0;
			SoundStream.SetPos(0);
			SoundStream.Play(true);
		}
		LeaveCriticalSection(&csStream);
	}
	catch ( BaseException & )
	{
		MLOG("Failed", );
		throw;
	}
}

void MusicManager::Pause()
{
	ATHROW(IsInited);
	try
	{
		EnterCriticalSection(&csStream);
		SoundStream.Stop();
		int posinslice = SoundStream.GetPos();
		posinslice %= StreamingUnitCount*Sound::UnitSize/2;
		if ( StreamingState < 2 )
		{
			StackPCM.Paste(SliceStreaming[1].PCM, SliceStreaming[1].PCMFill);
			SliceStreaming[1].PCMFill = 0;
			SliceStreaming[1].IsPlaying = false;
			StackPCM.Paste(SliceStreaming[0].PCM+posinslice, SliceStreaming[0].PCMFill-posinslice);
			SliceStreaming[0].PCMFill = 0;
			SliceStreaming[0].IsPlaying = false;
		} else {
			StackPCM.Paste(SliceStreaming[0].PCM, SliceStreaming[0].PCMFill);
			SliceStreaming[0].PCMFill = 0;
			SliceStreaming[0].IsPlaying = false;
			StackPCM.Paste(SliceStreaming[1].PCM+posinslice, SliceStreaming[1].PCMFill-posinslice);
			SliceStreaming[1].PCMFill = 0;
			SliceStreaming[1].IsPlaying = false;
		}
		LeaveCriticalSection(&csStream);
	}
	catch ( BaseException & )
	{
		MLOG("Failed", );
		throw;
	}
}

void MusicManager::ClearPCM()
{
	ATHROW(IsInited);
	StackPCM.Clear();
}

void MusicManager::StreamPCM(BYTE *Buffer, int SizeBuffer)
{
	ATHROW(IsInited);
	EnterCriticalSection(&csStream);
	try
	{
		StackPCM.Push(Buffer, SizeBuffer);
	}
	catch ( BaseException & )
	{

	}
	LeaveCriticalSection(&csStream);
}

int MusicManager::GetStreamNeededByte()
{
	if ( !IsInited ) return 0;

	EnterCriticalSection(&csStream);

	int posinslice = SoundStream.GetPos();
	posinslice %= StreamingUnitCount*Sound::UnitSize/2;

	int ret = 0;
	ret = StackPCM.GetCapacity() - StackPCM.GetElementCount();
	ret += StreamingUnitCount*Sound::UnitSize/2 - posinslice;

	if ( ret < 0 ) ret = 0;
	LeaveCriticalSection(&csStream);
	return ret;
}

int MusicManager::ReadCurrentPCM(BYTE *Result, int ResultCount)
{
	if ( !IsInited ) return 0;
	if ( !Result ) return 0;
	if ( !ResultCount ) return 0;
	EnterCriticalSection(&csStream);

	int proceeded = 0;
	int posinslice = SoundStream.GetPos();
	posinslice %= StreamingUnitCount*Sound::UnitSize/2;

	if ( StreamingState < 2 )
	{
		int available = SliceStreaming[0].PCMFill-posinslice;
		if ( available < 0 ) available = 0;
		if ( available+proceeded > ResultCount ) available = ResultCount-proceeded;
		memcpy(Result+proceeded, SliceStreaming[0].PCM+posinslice, available);
		proceeded += available;
		available = SliceStreaming[1].PCMFill;
		if ( available+proceeded > ResultCount ) available = ResultCount-proceeded;
		memcpy(Result+proceeded, SliceStreaming[1].PCM, available);
		proceeded += available;
	} else {
		int available = SliceStreaming[1].PCMFill-posinslice;
		if ( available < 0 ) available = 0;
		if ( available+proceeded > ResultCount ) available = ResultCount-proceeded;
		memcpy(Result+proceeded, SliceStreaming[1].PCM+posinslice, available);
		proceeded += available;
		available = SliceStreaming[0].PCMFill;
		if ( available+proceeded > ResultCount ) available = ResultCount-proceeded;
		memcpy(Result+proceeded, SliceStreaming[0].PCM, available);
		proceeded += available;
	}
	if ( proceeded < ResultCount )
	{
		proceeded += StackPCM.ReadFromStart(Result+proceeded, ResultCount-proceeded);
	}
	LeaveCriticalSection(&csStream);
	return proceeded;
}

void MusicManager::PCMNormalize(Sound::TSample *Buf, BYTE ChanCount, double *Result, DWORD UnitCount)
{
	ATHROW(Buf);
	ATHROW(Result);
	ATHROW(ChanCount >= 1);

	DWORD i;
	if ( ChanCount >= 2 )
	{
		for ( i=0; i < UnitCount; i++ )
		{
			double sum = 0;
			for ( int p=0; p < ChanCount; p++ )
			{
				if ( Buf[i*ChanCount+p] >= 0 )
					sum += Buf[i*ChanCount+p]/32767.;
				else sum += Buf[i*ChanCount+p]/32768.;
			}
			sum /= ChanCount;
			Result[i] = sum;
		}
	} else if ( ChanCount == 1 )
	{
		for ( i=0; i < UnitCount; i++ )
		{
			double sum = 0;
			if ( Buf[i] >= 0 )
				sum += Buf[i]/32767.;
			else sum += Buf[i]/32768.;
			Result[i] = sum;
		}
	} else NRTHROW(); // never be happened

}




