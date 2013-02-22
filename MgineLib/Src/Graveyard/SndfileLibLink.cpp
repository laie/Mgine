#if 0
#include "Mgine.h" // Uses only stuff in SndfileLibLink.h

#ifdef MGINESNDFILELINK_EXPORTS
# include "SndfileLibLink.h"
#endif

using namespace Mgine;

LinkSndFile LinkSndFile::Instance;
#ifndef MGINESNDFILELINK_EXPORTS
LinkSndFile & LinkSndFile::Link = Instance;

LinkSndFile::LinkSndFile()
{

}
LinkSndFile::~LinkSndFile()
{

}
#endif



HSNDFILE	* Mgine::LinkSndFile::Open( const wchar_t *wpath, SFM_FILEMODE mode, SNDFILE_INFO *sfinfo )
{ return sf_wchar_open(wpath, (int)mode, sfinfo); }
HSNDFILE	* Mgine::LinkSndFile::OpenFD( int fd, SFM_FILEMODE mode, SNDFILE_INFO *sfinfo, int close_desc )
{ return sf_open_fd(fd, (int)mode, sfinfo, close_desc); }
HSNDFILE	* Mgine::LinkSndFile::OpenVirtual( SF_VIRTUAL_IO *sfvirtual, SFM_FILEMODE mode, SNDFILE_INFO *sfinfo, void *user_data )
{ return sf_open_virtual(sfvirtual, (int)mode, sfinfo, user_data); }
int Mgine::LinkSndFile::Error( HSNDFILE *sndfile )
{ return sf_error(sndfile); }
const char * Mgine::LinkSndFile::StrError( HSNDFILE *sndfile )
{ return sf_strerror(sndfile); }
const char * Mgine::LinkSndFile::ErrorNumber( int errnum )
{ return sf_error_number(errnum); }
int Mgine::LinkSndFile::PError( HSNDFILE *sndfile )
{ return sf_perror(sndfile); }
int Mgine::LinkSndFile::ErrorStr( HSNDFILE *sndfile, char* str, size_t len )
{ return sf_error_str(sndfile, str, len); }
int Mgine::LinkSndFile::Command( HSNDFILE *sndfile, int command, void *data, int datasize )
{ return sf_command(sndfile, command, data, datasize); }
int Mgine::LinkSndFile::FormatCheck( const SNDFILE_INFO *info )
{ return sf_format_check(info); }
Mgine::T_SF_COUNT Mgine::LinkSndFile::Seek( HSNDFILE *sndfile, T_SF_COUNT frames, int whence )
{ return sf_seek(sndfile, frames, whence); }
int Mgine::LinkSndFile::SetString( HSNDFILE *sndfile, int str_type, const char* str )
{ return sf_set_string(sndfile, str_type, str); }
const char * Mgine::LinkSndFile::GetString( HSNDFILE *sndfile, int str_type )
{ return sf_get_string(sndfile, str_type); }
const char * Mgine::LinkSndFile::VersionString( void )
{ return sf_version_string(); }
Mgine::T_SF_COUNT Mgine::LinkSndFile::ReadRaw( HSNDFILE *sndfile, void *ptr, T_SF_COUNT bytes )
{ return sf_read_raw(sndfile, ptr, bytes); }
Mgine::T_SF_COUNT Mgine::LinkSndFile::WriteRaw( HSNDFILE *sndfile, const void *ptr, T_SF_COUNT bytes )
{ return sf_write_raw(sndfile, ptr, bytes); }
Mgine::T_SF_COUNT Mgine::LinkSndFile::ReadFShort( HSNDFILE *sndfile, short *ptr, T_SF_COUNT frames )
{ return sf_readf_short(sndfile, ptr, frames); }
Mgine::T_SF_COUNT Mgine::LinkSndFile::WriteFShort( HSNDFILE *sndfile, const short *ptr, T_SF_COUNT frames )
{ return sf_writef_short(sndfile, ptr, frames); }
Mgine::T_SF_COUNT Mgine::LinkSndFile::ReadFInt( HSNDFILE *sndfile, int *ptr, T_SF_COUNT frames )
{ return sf_readf_int(sndfile, ptr, frames); }
Mgine::T_SF_COUNT Mgine::LinkSndFile::WriteFInt( HSNDFILE *sndfile, const int *ptr, T_SF_COUNT frames )
{ return sf_writef_int(sndfile, ptr, frames); }
Mgine::T_SF_COUNT Mgine::LinkSndFile::ReadFFloat( HSNDFILE *sndfile, float *ptr, T_SF_COUNT frames )
{ return sf_readf_float(sndfile, ptr, frames); }
Mgine::T_SF_COUNT Mgine::LinkSndFile::WriteFFloat( HSNDFILE *sndfile, const float *ptr, T_SF_COUNT frames )
{ return sf_writef_float(sndfile, ptr, frames); }
Mgine::T_SF_COUNT Mgine::LinkSndFile::ReadFDouble( HSNDFILE *sndfile, double *ptr, T_SF_COUNT frames )
{ return sf_readf_double(sndfile, ptr, frames); }
Mgine::T_SF_COUNT Mgine::LinkSndFile::WriteFDouble( HSNDFILE *sndfile, const double *ptr, T_SF_COUNT frames )
{ return sf_writef_double(sndfile, ptr, frames); }
Mgine::T_SF_COUNT Mgine::LinkSndFile::ReadShort( HSNDFILE *sndfile, short *ptr, T_SF_COUNT items )
{ return sf_read_short(sndfile, ptr, items); }
Mgine::T_SF_COUNT Mgine::LinkSndFile::WriteShort( HSNDFILE *sndfile, const short *ptr, T_SF_COUNT items )
{ return sf_write_short(sndfile, ptr, items); }
Mgine::T_SF_COUNT Mgine::LinkSndFile::ReadInt( HSNDFILE *sndfile, int *ptr, T_SF_COUNT items )
{ return sf_read_int(sndfile, ptr, items); }
Mgine::T_SF_COUNT Mgine::LinkSndFile::WriteInt( HSNDFILE *sndfile, const int *ptr, T_SF_COUNT items )
{ return sf_write_int(sndfile, ptr, items); }
Mgine::T_SF_COUNT Mgine::LinkSndFile::ReadFloat( HSNDFILE *sndfile, float *ptr, T_SF_COUNT items )
{ return sf_read_float(sndfile, ptr, items); }
Mgine::T_SF_COUNT Mgine::LinkSndFile::WriteFloat( HSNDFILE *sndfile, const float *ptr, T_SF_COUNT items )
{ return sf_write_float(sndfile, ptr, items); }
Mgine::T_SF_COUNT Mgine::LinkSndFile::ReadDouble( HSNDFILE *sndfile, double *ptr, T_SF_COUNT items )
{ return sf_read_double(sndfile, ptr, items); }
Mgine::T_SF_COUNT Mgine::LinkSndFile::WriteDouble( HSNDFILE *sndfile, const double *ptr, T_SF_COUNT items )
{ return sf_write_double(sndfile, ptr, items); }
int Mgine::LinkSndFile::Close( HSNDFILE *sndfile )
{ return sf_close(sndfile); }
void Mgine::LinkSndFile::WriteSync( HSNDFILE *sndfile )
{ return sf_write_sync(sndfile); }
#endif