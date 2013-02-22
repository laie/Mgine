#if 0
#pragma once
#include "Mgine.h" // Uses only stuff in SndfileLibLink.h

/*\	SndFileLibLink
 *		Link with open sourced libraries.
 *		This stuff stands for supporting further development of open source features.
 *		Usually link statically, but there are some dynamic ways, prefer dynamic way.
\*/
# pragma comment (lib, "libsndfile_1_0_25_port.lib")
#define ENABLE_SNDFILE_WINDOWS_PROTOTYPES 1
# include <sndfile.h>

#ifdef MGINESNDFILELINK_EXPORTS
#define DLL_LINK __declspec(dllexport)
#else
#define DLL_LINK
#endif

namespace Mgine
{
	typedef sf_count_t T_SF_COUNT;
	typedef SNDFILE HSNDFILE;
	typedef SF_INFO SNDFILE_INFO;
	//typedef SF_VIRTUAL_IO SF_VIRTUAL_IO;

	enum SFM_FILEMODE
	{
		READ	= 0x10,
		WRITE	= 0x20,
		RDWR	= 0x30,
	};

	class LinkSndFile
	{
	private:
	static LinkSndFile Instance;
	#ifndef MGINESNDFILELINK_EXPORTS
		LinkSndFile();
		virtual ~LinkSndFile();
	public:
		static LinkSndFile & Link;
	private:
	#else
		
	#endif
	public:
		virtual HSNDFILE*	Open(const wchar_t *wpath, SFM_FILEMODE mode, SNDFILE_INFO *sfinfo);
		virtual HSNDFILE*	OpenFD(int fd, SFM_FILEMODE mode, SNDFILE_INFO *sfinfo, int close_desc) ;

		virtual HSNDFILE*	OpenVirtual(SF_VIRTUAL_IO *sfvirtual, SFM_FILEMODE mode, SNDFILE_INFO *sfinfo, void *user_data) ;
		virtual int			Error(HSNDFILE *sndfile) ;
		virtual const char*	StrError(HSNDFILE *sndfile) ;
		virtual const char*	ErrorNumber(int errnum) ;
		virtual int			PError(HSNDFILE *sndfile) ;
		virtual int			ErrorStr(HSNDFILE *sndfile, char* str, size_t len) ;
		virtual int			Command(HSNDFILE *sndfile, int command, void *data, int datasize) ;
		virtual int			FormatCheck(const SNDFILE_INFO *info) ;
		virtual T_SF_COUNT	Seek(HSNDFILE *sndfile, T_SF_COUNT frames, int whence) ;
		virtual int			SetString(HSNDFILE *sndfile, int str_type, const char* str) ;

		virtual const char*	GetString(HSNDFILE *sndfile, int str_type) ;
		virtual const char*	VersionString(void) ;
		virtual T_SF_COUNT	ReadRaw(HSNDFILE *sndfile, void *ptr, T_SF_COUNT bytes) ;
		virtual T_SF_COUNT	WriteRaw(HSNDFILE *sndfile, const void *ptr, T_SF_COUNT bytes) ;
		virtual T_SF_COUNT	ReadFShort(HSNDFILE *sndfile, short *ptr, T_SF_COUNT frames) ;
		virtual T_SF_COUNT	WriteFShort(HSNDFILE *sndfile, const short *ptr, T_SF_COUNT frames) ;

		virtual T_SF_COUNT	ReadFInt(HSNDFILE *sndfile, int *ptr, T_SF_COUNT frames) ;
		virtual T_SF_COUNT	WriteFInt(HSNDFILE *sndfile, const int *ptr, T_SF_COUNT frames) ;

		virtual T_SF_COUNT	ReadFFloat(HSNDFILE *sndfile, float *ptr, T_SF_COUNT frames) ;
		virtual T_SF_COUNT	WriteFFloat(HSNDFILE *sndfile, const float *ptr, T_SF_COUNT frames) ;

		virtual T_SF_COUNT	ReadFDouble(HSNDFILE *sndfile, double *ptr, T_SF_COUNT frames) ;
		virtual T_SF_COUNT	WriteFDouble(HSNDFILE *sndfile, const double *ptr, T_SF_COUNT frames) ;
		virtual T_SF_COUNT	ReadShort(HSNDFILE *sndfile, short *ptr, T_SF_COUNT items) ;
		virtual T_SF_COUNT	WriteShort(HSNDFILE *sndfile, const short *ptr, T_SF_COUNT items) ;

		virtual T_SF_COUNT	ReadInt(HSNDFILE *sndfile, int *ptr, T_SF_COUNT items) ;
		virtual T_SF_COUNT	WriteInt(HSNDFILE *sndfile, const int *ptr, T_SF_COUNT items) ;

		virtual T_SF_COUNT	ReadFloat(HSNDFILE *sndfile, float *ptr, T_SF_COUNT items) ;
		virtual T_SF_COUNT	WriteFloat(HSNDFILE *sndfile, const float *ptr, T_SF_COUNT items) ;

		virtual T_SF_COUNT	ReadDouble(HSNDFILE *sndfile, double *ptr, T_SF_COUNT items) ;
		virtual T_SF_COUNT	WriteDouble(HSNDFILE *sndfile, const double *ptr, T_SF_COUNT items) ;

		virtual int			Close(HSNDFILE *sndfile) ;
		virtual void		WriteSync(HSNDFILE *sndfile) ;
	};
}

#endif