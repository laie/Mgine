////////////////////////////////////////////////////////////
//
// SFML - Simple and Fast Multimedia Library
// Copyright (C) 2007-2009 Laurent Gomila (laurent.gom@gmail.com)
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it freely,
// subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented;
//    you must not claim that you wrote the original software.
//    If you use this software in a product, an acknowledgment
//    in the product documentation would be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such,
//    and must not be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//
////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SoundFileDefault.h"
#include <iostream>
#include <string.h>
#include <locale>


namespace sf
{
namespace priv
{
////////////////////////////////////////////////////////////
/// Default constructor
////////////////////////////////////////////////////////////
SoundFileDefault::SoundFileDefault() :
myFile(NULL)
{

}


////////////////////////////////////////////////////////////
/// Destructor
////////////////////////////////////////////////////////////
SoundFileDefault::~SoundFileDefault()
{
    if (myFile)
        sf_close(myFile);
}


////////////////////////////////////////////////////////////
/// Check if a given file is supported by this loader
////////////////////////////////////////////////////////////
bool SoundFileDefault::IsFileSupported(const std::wstring& Filename, bool Read)
{
    if (Read)
    {
        // Open the sound file
        SF_INFO FileInfos;
        SNDFILE* File = sf_wchar_open(Filename.c_str(), SFM_READ, &FileInfos);

        if (File)
        {
            sf_close(File);
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        // Check the extension
        return GetFormatFromFilename(Filename) != -1;
    }
}


////////////////////////////////////////////////////////////
/// Check if a given file in memory is supported by this loader
////////////////////////////////////////////////////////////
bool SoundFileDefault::IsFileSupported(const char* Data, std::size_t SizeInBytes)
{
    // Define the I/O custom functions for reading from memory
    SF_VIRTUAL_IO VirtualIO;
    VirtualIO.get_filelen = &SoundFileDefault::MemoryGetLength;
    VirtualIO.read        = &SoundFileDefault::MemoryRead;
    VirtualIO.seek        = &SoundFileDefault::MemorySeek;
    VirtualIO.tell        = &SoundFileDefault::MemoryTell;
    VirtualIO.write       = &SoundFileDefault::MemoryWrite;

    // Initialize the memory data
    MemoryInfos Memory;
    Memory.DataStart = Data;
    Memory.DataPtr   = Data;
    Memory.TotalSize = SizeInBytes;

    // Open the sound file
    SF_INFO FileInfos;
    SNDFILE* File = sf_open_virtual(&VirtualIO, SFM_READ, &FileInfos, &Memory);

    if (File)
    {
        sf_close(File);
        return true;
    }
    else
    {
        return false;
    }
}


////////////////////////////////////////////////////////////
/// Open the sound file for reading
////////////////////////////////////////////////////////////
bool SoundFileDefault::OpenRead(const std::wstring& Filename, std::size_t& NbSamples, unsigned int& ChannelsCount, unsigned int& SampleRate)
{
    // If the file is already opened, first close it
    if (myFile)
        sf_close(myFile);

    // Open the sound file
    SF_INFO FileInfos;
    myFile = sf_wchar_open(Filename.c_str(), SFM_READ, &FileInfos);
    if (!myFile)
    {
        std::wcerr << L"Failed to read sound file \"" << Filename << L"\"" << std::endl;
        return false;
    }

    // Set the sound parameters
    ChannelsCount = FileInfos.channels;
    SampleRate    = FileInfos.samplerate;
    NbSamples     = static_cast<std::size_t>(FileInfos.frames) * ChannelsCount;

    return true;
}


////////////////////////////////////////////////////////////
/// /see sf::SoundFile::OpenRead
////////////////////////////////////////////////////////////
bool SoundFileDefault::OpenRead(const char* Data, std::size_t SizeInBytes, std::size_t& NbSamples, unsigned int& ChannelsCount, unsigned int& SampleRate)
{
    // If the file is already opened, first close it
    if (myFile)
        sf_close(myFile);

    // Define the I/O custom functions for reading from memory
    SF_VIRTUAL_IO VirtualIO;
    VirtualIO.get_filelen = &SoundFileDefault::MemoryGetLength;
    VirtualIO.read        = &SoundFileDefault::MemoryRead;
    VirtualIO.seek        = &SoundFileDefault::MemorySeek;
    VirtualIO.tell        = &SoundFileDefault::MemoryTell;
    VirtualIO.write       = &SoundFileDefault::MemoryWrite;

    // Initialize the memory data
    myMemory.DataStart = Data;
    myMemory.DataPtr   = Data;
    myMemory.TotalSize = SizeInBytes;

    // Open the sound file
    SF_INFO FileInfos;
    myFile = sf_open_virtual(&VirtualIO, SFM_READ, &FileInfos, &myMemory);
    if (!myFile)
    {
        std::cerr << "Failed to read sound file from memory" << std::endl;
        return false;
    }

    // Set the sound parameters
    ChannelsCount = FileInfos.channels;
    SampleRate    = FileInfos.samplerate;
    NbSamples     = static_cast<std::size_t>(FileInfos.frames) * ChannelsCount;

    return true;
}


////////////////////////////////////////////////////////////
/// Open the sound file for writing
////////////////////////////////////////////////////////////
bool SoundFileDefault::OpenWrite(const std::wstring& Filename, unsigned int ChannelsCount, unsigned int SampleRate)
{
    // If the file is already opened, first close it
    if (myFile)
        sf_close(myFile);

    // Find the right format according to the file extension
    int Format = GetFormatFromFilename(Filename);
    if (Format == -1)
    {
        // Error : unrecognized extension
        std::wcerr << L"Failed to create sound file \"" << Filename << L"\" : unknown format" << std::endl;
        return false;
    }

    // Fill the sound infos with parameters
    SF_INFO FileInfos;
    FileInfos.channels   = ChannelsCount;
    FileInfos.samplerate = SampleRate;
    FileInfos.format     = Format | SF_FORMAT_PCM_16;

    // Open the sound file for writing
    myFile = sf_wchar_open(Filename.c_str(), SFM_WRITE, &FileInfos);
    if (!myFile)
    {
        std::wcerr << L"Failed to create sound file \"" << Filename << L"\"" << std::endl;
        return false;
    }

    return true;
}


////////////////////////////////////////////////////////////
/// Read samples from the loaded sound
////////////////////////////////////////////////////////////
std::size_t SoundFileDefault::Read(Int16* Data, std::size_t NbSamples)
{
    if (myFile && Data && NbSamples)
        return static_cast<std::size_t>(sf_read_short(myFile, Data, NbSamples));
    else
        return 0;
}


////////////////////////////////////////////////////////////
/// Write samples to the file
////////////////////////////////////////////////////////////
void SoundFileDefault::Write(const Int16* Data, std::size_t NbSamples)
{
    if (myFile && Data && NbSamples)
        sf_write_short(myFile, Data, NbSamples);
}


////////////////////////////////////////////////////////////
/// Get the internal format of an audio file according to
/// its filename extension
////////////////////////////////////////////////////////////
int SoundFileDefault::GetFormatFromFilename(const std::wstring& Filename)
{
    // Extract the extension
    std::wstring Ext = L"wav";
    std::wstring::size_type Pos = Filename.find_last_of(L".");
    std::wstring::size_type PosRS = Filename.find_last_of(L"\\");
    if (Pos != std::wstring::npos)
        Ext = Filename.substr(Pos + 1);
	if ( Pos < PosRS ) Ext = L"";
	for ( _Uint32t i=0; i < Ext.length(); i++ ) Ext[i] = std::tolower(Ext[i], std::locale());
	//static_assert(false, "오류가 있을수있음, 게다가 대소문자 구분을 함..");

    // Match every supported extension with its format constant
    if      (Ext == L"wav"  ) return SF_FORMAT_WAV;
    else if (Ext == L"aif"  ) return SF_FORMAT_AIFF;
    else if (Ext == L"aiff" ) return SF_FORMAT_AIFF;
    else if (Ext == L"au"   ) return SF_FORMAT_AU;
    else if (Ext == L"raw"  ) return SF_FORMAT_RAW;
    else if (Ext == L"paf"  ) return SF_FORMAT_PAF;
    else if (Ext == L"svx"  ) return SF_FORMAT_SVX;
    else if (Ext == L"voc"  ) return SF_FORMAT_VOC;
    else if (Ext == L"sf"   ) return SF_FORMAT_IRCAM;
    else if (Ext == L"w64"  ) return SF_FORMAT_W64;
    else if (Ext == L"mat4" ) return SF_FORMAT_MAT4;
    else if (Ext == L"mat5" ) return SF_FORMAT_MAT5;
    else if (Ext == L"pvf"  ) return SF_FORMAT_PVF;
    else if (Ext == L"htk"  ) return SF_FORMAT_HTK;
    else if (Ext == L"caf"  ) return SF_FORMAT_CAF;
    else if (Ext == L"nist" ) return SF_FORMAT_NIST; // SUPPORTED ?
    else if (Ext == L"sds"  ) return SF_FORMAT_SDS;  // SUPPORTED ?
    else if (Ext == L"avr"  ) return SF_FORMAT_AVR;  // SUPPORTED ?
    else if (Ext == L"sd2"  ) return SF_FORMAT_SD2;  // SUPPORTED ?
    else if (Ext == L"flac" ) return SF_FORMAT_FLAC; // SUPPORTED ?
	
    return -1;
}


////////////////////////////////////////////////////////////
/// Functions for implementing custom read and write to memory files
///
////////////////////////////////////////////////////////////
sf_count_t SoundFileDefault::MemoryGetLength(void* UserData)
{
    MemoryInfos* Memory = static_cast<MemoryInfos*>(UserData);

    return Memory->TotalSize;
}
sf_count_t SoundFileDefault::MemoryRead(void* Ptr, sf_count_t Count, void* UserData)
{
    MemoryInfos* Memory = static_cast<MemoryInfos*>(UserData);

    sf_count_t Position = Memory->DataPtr - Memory->DataStart;
    if (Position + Count >= Memory->TotalSize)
        Count = Memory->TotalSize - Position;

    memcpy(Ptr, Memory->DataPtr, static_cast<std::size_t>(Count));

    Memory->DataPtr += Count;

    return Count;
}
sf_count_t SoundFileDefault::MemorySeek(sf_count_t Offset, int Whence, void* UserData)
{
    MemoryInfos* Memory = static_cast<MemoryInfos*>(UserData);

    sf_count_t Position = 0;
    switch (Whence)
    {
        case SEEK_SET :
            Position = Offset;
            break;
        case SEEK_CUR :
            Position = Memory->DataPtr - Memory->DataStart + Offset;
            break;
        case SEEK_END :
            Position = Memory->TotalSize - Offset;
            break;
        default :
            Position = 0;
            break;
    }

    if (Position >= Memory->TotalSize)
        Position = Memory->TotalSize - 1;
    else if (Position < 0)
        Position = 0;

    Memory->DataPtr = Memory->DataStart + Position;

    return Position;
}
sf_count_t SoundFileDefault::MemoryTell(void* UserData)
{
    MemoryInfos* Memory = static_cast<MemoryInfos*>(UserData);

    return Memory->DataPtr - Memory->DataStart;
}
sf_count_t SoundFileDefault::MemoryWrite(const void*, sf_count_t, void*)
{
    return 0;
}


} // namespace priv

} // namespace sf
