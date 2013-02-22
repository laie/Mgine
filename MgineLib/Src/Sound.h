#pragma once

#include "Engine.h"

namespace sf { namespace priv { class SoundFile; } }
namespace Mgine
{
	class SoundFileReader;
	class SoundBuffer;

	class Sound
	{
	public:
		class Category;
		static const int UnitSampleRate = 44100;
		static const int UnitChannelCount = 2;
		typedef short TSample;
		static const int UnitSize = sizeof(TSample)*UnitChannelCount;
	private:
		static HWND hWnd;
		static Util::Queue<SoundBuffer*> *pQueueSound;

	public:
		static Util::Queue<Category*> QueueCategory;
		static bool IsInited;
		static LPDIRECTSOUND8 gDS;
		//static LPDIRECTSOUND gDS;

		static void Init(HWND hWnd);
		static void Uninit();
		static void OnAppFinish();

		static void Update(double ElapsedTime);
		static DWORD AddCategory();
		static void AdjustCategorySound(float Sound);

		static inline Util::Queue<SoundBuffer*> & QueueSound()
		{
			if ( !EngineManager::Core->IsAppFinished && !pQueueSound ) pQueueSound = new Util::Queue<SoundBuffer*>;
			return *pQueueSound;
		}


		static bool LoadSoundFile(wchar_t *FileName, MGINE_SOUND_DESC *Result);


		class Category
		{
		private:

		public:
			float Multiply;
			int CategoryNo;

			Category();
			void AdjustMultiply(float Multiply);
		};

	};

	class SoundBuffer
	{
		PROPERTY_PROVIDE(SoundBuffer);
	public:
		struct INIT_PARAM
		{
			bool Is3D;
			bool IsStreaming;
			bool IsNotification;
		};

	private:
		void InitDescBuffer();

	protected:
		DECLARE_PROP_TYPE_R(SoundBuffer, bool, IsInited, { return IsInited.Value; }, { IsInited.Value = Value; });

		DECLARE_PROP_TYPE_RW(SoundBuffer, float, Volume, { return Volume.Value; }, { Volume.Value = Value; });

		DECLARE_PROP_TYPE_RW(SoundBuffer, int, CategoryNo, { return CategoryNo.Value; }, { CategoryNo.Value = Value; });
		std::list<sf::priv::SoundFile*> ListStreamFile;
		DECLARE_PROP_TYPE_R(SoundBuffer, DWORD, CurrentStreamingPos, { return CurrentStreamingPos.Value; }, { CurrentStreamingPos.Value = Value; });
		DECLARE_PROP_TYPE_RW(SoundBuffer, bool, IsStreamingRepeat, { return IsStreamingRepeat.Value; }, { IsStreamingRepeat.Value = Value; });

		DECLARE_PROP_TYPE_R(SoundBuffer, INIT_PARAM, SelectedInitParam, { return SelectedInitParam.Value; }, { SelectedInitParam.Value = Value; });



		LPDIRECTSOUNDBUFFER		gSoundBuffer;
		LPDIRECTSOUND3DBUFFER	gSoundBuffer3D;
		LPDIRECTSOUNDNOTIFY		gDsNotification;
		DSBUFFERDESC			DescBuffer;
		MGINE_SOUND_DESC		DescFile;
		int SoundIndex;


	public:
		DECLARE_PROPERTY(IsInited);

		DECLARE_PROPERTY(Volume);

		DECLARE_PROPERTY(CategoryNo);

		DECLARE_PROPERTY(CurrentStreamingPos);
		DECLARE_PROPERTY(IsStreamingRepeat); 

		INIT_PARAM InitParam;
		DECLARE_PROPERTY(SelectedInitParam);

		inline SoundBuffer()
		{
			Volume = 1;

			IsInited = false;

			gSoundBuffer = 0;
			gSoundBuffer3D = 0;
			gDsNotification = 0;

			InitParam = INIT_PARAM();

			DescBuffer = DSBUFFERDESC();
			DescFile = MGINE_SOUND_DESC();

			CategoryNo = 0;
			SoundIndex = Sound::QueueSound().Add(this);
		}
		virtual ~SoundBuffer();

		inline HRESULT SetNotificationPositions(DWORD PositionNotifies, LPDSBPOSITIONNOTIFY pcPositionNotifies)
		{ return gDsNotification->SetNotificationPositions(PositionNotifies, pcPositionNotifies); }

		// Buffer initialization for static usage.
		// Fills internal buffer with the parameter File's content.
		virtual void Init(sf::priv::SoundFile & File);
		// Buffer initialization for static usage.
		// Fills internal buffer with the parameter Buffer.
		// remark: this only gets just word stereo 44100/s pcm data
		virtual void Init(const std::stringstream & Buffer);
		// Buffer initialization for static/dynamic(interactive such as streaming) usage.
		// Reserves(allocates) internal buffer by BufferSize.
		// Streaming functions use this internally.
		// Buffer with 0 will fill internal buffer with 0
		// remark: this only gets just word stereo 44100/s pcm data
		virtual void Init(Sound::TSample *Buffer, UINT BufferSampleUnitCount);
		virtual void Uninit();

		virtual void LockWriteSample(BYTE *Buffer, DWORD Count, int Position);
		virtual void LockReadSample(BYTE *Buffer, DWORD Count, int Position);

		virtual void Play(bool IsLoop);
		virtual void Stop();
		virtual void SetPos(DWORD Pos);
		virtual DWORD GetPos();
		virtual void SetVolume(float Volume);
		virtual void ResetVolume();
		virtual float GetVolume();
		virtual bool GetIsPlaying();
		virtual void SetPan(int Pan);

		virtual void OnDeviceCreate();
	};

	/*class SampleRateConverter
	{
	private:
		sf::priv::SoundFile & SoundFile;
		DWORD ObjectiveSampleRate;
		Mgine::VEC LastAlias;
		std::stringstream FeedStream;

	public:
		SampleRateConverter(sf::priv::SoundFile & SoundFile, DWORD ObjectiveSampleRate)
			: SoundFile(SoundFile),
			FeedStream(std::ios::binary | std::ios::in | std::ios::out)
		{
			this->ObjectiveSampleRate = ObjectiveSampleRate;
			LastAlias = 0;
		}
		inline size_t Read(WORD *BufferReceive, DWORD SampleCount)
		{
			if ( ObjectiveSampleRate == SoundFile.GetSampleRate() )
				return SoundFile.Read(BufferReceive, SampleCount);
			else if ( ObjectiveSampleRate < SoundFile.GetSampleRate() )
			{
				int resamplecount = SampleCount*ObjectiveSampleRate/SoundFile.GetSampleRate();
				int mod = SampleCount*ObjectiveSampleRate % SoundFile.GetSampleRate();
				if ( mod )
				{
					resamplecount ++;
				}

				boost::scoped_array<WORD> rebuf(new WORD[resamplecount]);
				size_t ret = SoundFile.Read((sf::Int16*)rebuf.get(), resamplecount);

				if ( mod )
				{
					if ( ret == resamplecount )
					{
						ret
					} else LastAlias = 0;
				}
				LastAlias = (Mgine::VEC)mod/SoundFile.GetSampleRate();
			}
		}
	};*/

	class SoundFile : boost::noncopyable
	{
	private:
	public:
		static SoundFile *Open(const std::wstring & FileName);

		bool IsInited;
		
		struct
		{
			Util::UnwindArray<wchar_t> AlbumTitle;
			Util::UnwindArray<wchar_t> Artist;
			Util::UnwindArray<wchar_t> Genre;
			Util::UnwindArray<wchar_t> Title;
			Util::UnwindArray<wchar_t> TrackNoStr;
			Util::UnwindArray<wchar_t> Year;
			LONGLONG	DurationMs;
		} MetaData;
		std::wstring FilePath;

		inline SoundFile()
		{
			IsInited = false; 
			memset(&MetaData, 0, sizeof(MetaData));
		}
		virtual ~SoundFile()
		{
		}
		
		virtual bool Init(const std::wstring & FileName) = 0;
		virtual void Uninit() = 0;

		virtual SoundFileReader *CreateRead() = 0;
	};

	class SoundFileReader : boost::noncopyable
	{
	private:
	protected:
		bool IsInited;
		SoundFile *MySoundFile;
		inline SoundFileReader(SoundFile *SoundFile)
			: IsInited(false), MySoundFile(SoundFile)
		{
		}
	public:
		virtual ~SoundFileReader(){ };

		//virtual bool ScanAll() = 0; // SEEK Indices, tag process and etc
		virtual void	Seek(ULONGLONG ObjectiveSample) = 0;
		virtual DWORD	Read(Sound::TSample *BufResult, DWORD SampleUnitCount) = 0;

		virtual ULONGLONG	GetSampleCount()	= 0;
		virtual BYTE		GetChannelCount()	= 0;
		virtual UINT		GetSampleRate()		= 0;
		virtual ULONGLONG	GetSeekPos()		= 0;
	};

}



