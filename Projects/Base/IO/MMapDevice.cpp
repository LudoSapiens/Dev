/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Base/IO/MMapDevice.h>

#include <Base/Dbg/Defs.h>
#include <Base/IO/TextStream.h>

// Some interesting link:
//   http://duartes.org/gustavo/blog/post/page-cache-the-affair-between-memory-and-files

#if PLAT_POSIX
#include <cstdio>
#include <sys/mman.h>

#if defined(__CYGWIN__) && (__GNUC_VERSION__ == 40503)
// Missing routine.
extern "C" int fileno( FILE* );
#endif

#elif PLAT_WINDOWS
#include <windows.h>
#include <io.h>

#endif

USING_NAMESPACE

/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN

//------------------------------------------------------------------------------
//! Converts the Mode into the required string used in C's fopen() routine.
//! Returns a String in order to be thread-safe.
inline const char* modeToFOpen( IODevice::Mode mode )
{
   static const char* str[] = {
            //    NEWFILE
            //   /   WRITE
            //   |  /   READ
            //   |  |  /
      "",   //   0  0  0  (UNKNOWN)
      "r",  //   0  0  1
      "w+", //   0  1  0  (Keep previous content; user flag will prevent reads)
      "r+", //   0  1  1
      "",   //   1  0  0  (INVALID)
      "r",  //   1  0  1  (Ignore NEWFILE bit)
      "w",  //   1  1  0
      "w+"  //   1  1  1  (Keep previous content)
   };

   IODevice::Mode mask = IODevice::MODE_READ_WRITE | IODevice::MODE_NEWFILE;
   return str[ mode & mask ];
}

#if PLAT_POSIX
//------------------------------------------------------------------------------
//! Converts the Mode into the required code used in mmap().
inline int modeToProt( IODevice::Mode mode )
{
   static const int str[] = {
                            //    NEWFILE
                            //   /   WRITE
                            //   |  /   READ
                            //   |  |  /
      PROT_NONE,            //   0  0  0  (UNKNOWN)
      PROT_READ,            //   0  0  1
      PROT_WRITE,           //   0  1  0
      PROT_READ|PROT_WRITE, //   0  1  1
      PROT_NONE,            //   1  0  0  (INVALID)
      PROT_READ,            //   1  0  1
      PROT_WRITE,           //   1  1  0
      PROT_READ|PROT_WRITE  //   1  1  1
   };

   MMapDevice::Mode mask = MMapDevice::MODE_READ_WRITE | MMapDevice::MODE_NEWFILE;
   return str[ mode & mask ];
}
#elif PLAT_WINDOWS

//------------------------------------------------------------------------------
//! Returns the high bits of a size_t.
inline DWORD high( size_t v )
{
   if( sizeof(v) > 4 )
   {
      v >>= 32;
      return DWORD(v);
   }
   else
   {
      return 0;
   }
}

//------------------------------------------------------------------------------
//! Returns the low bits of a size_t.
inline DWORD low( size_t v )
{
   return DWORD(v);
}

// Some refs:
//   http://msdn.microsoft.com/en-us/library/aa366548(v=vs.85).aspx
//   http://msdn.microsoft.com/en-us/library/aa366537(v=vs.85).aspx
//   http://msdn.microsoft.com/en-us/library/aa366761(v=vs.85).aspx
//   http://msdn.microsoft.com/en-us/library/aa366563(v=vs.85).aspx

//------------------------------------------------------------------------------
//! Converts the Mode into the required code used in CreateFileMapping().
inline DWORD modeToProt( IODevice::Mode mode )
{
   static const DWORD map[] = {
                            //    NEWFILE
                            //   /   WRITE
                            //   |  /   READ
                            //   |  |  /
      DWORD(0),             //   0  0  0  (UNKNOWN)
      PAGE_READONLY,        //   0  0  1
      PAGE_WRITECOPY,       //   0  1  0
      PAGE_READWRITE,       //   0  1  1
      DWORD(0),             //   1  0  0  (INVALID)
      PAGE_READONLY,        //   1  0  1
      PAGE_WRITECOPY,       //   1  1  0
      PAGE_READWRITE        //   1  1  1
   };

   MMapDevice::Mode mask = MMapDevice::MODE_READ_WRITE | MMapDevice::MODE_NEWFILE;
   return map[ mode & mask ];
}

//------------------------------------------------------------------------------
//! Converts the Mode into the required access used in MapViewOfFile().
inline DWORD modeToAccess( IODevice::Mode mode )
{
   static const DWORD map[] = {
                            //    NEWFILE
                            //   /   WRITE
                            //   |  /   READ
                            //   |  |  /
      DWORD(0),             //   0  0  0  (UNKNOWN)
      FILE_MAP_READ,        //   0  0  1
      FILE_MAP_WRITE,       //   0  1  0
      FILE_MAP_ALL_ACCESS,  //   0  1  1
      DWORD(0),             //   1  0  0  (INVALID)
      FILE_MAP_READ,        //   1  0  1
      FILE_MAP_WRITE,       //   1  1  0
      FILE_MAP_ALL_ACCESS   //   1  1  1
   };

   MMapDevice::Mode mask = MMapDevice::MODE_READ_WRITE | MMapDevice::MODE_NEWFILE;
   return map[ mode & mask ];
}

const char* errorToStr( DWORD err )
{
   switch( err )
   {
      case ERROR_SUCCESS            : return "Success";
      case ERROR_INVALID_FUNCTION   : return "Invalid function";
      case ERROR_FILE_NOT_FOUND     : return "File not found";
      case ERROR_PATH_NOT_FOUND     : return "Path not found";
      case ERROR_TOO_MANY_OPEN_FILES: return "Too many open files";
      case ERROR_ACCESS_DENIED      : return "Access denied";
      case ERROR_INVALID_HANDLE     : return "Invalid handle";
      case ERROR_ARENA_TRASHED      : return "Arena thrased";
      case ERROR_NOT_ENOUGH_MEMORY  : return "Not enough memory";
      case ERROR_INVALID_BLOCK      : return "Invalid block";
      case ERROR_FILE_INVALID       : return "File invalid";
      case ERROR_ALREADY_EXISTS     : return "Already exists";
      case ERROR_DISK_FULL          : return "Disk full";
      default                       : return "unknown";
   }
}

String toStr( DWORD err )
{
   return String().format( "%d - %s", err, errorToStr(err) );
}

String lastErr()
{
   return toStr( GetLastError() );
}

#endif

UNNAMESPACE_END

//------------------------------------------------------------------------------
//!
MMapDevice::MMapDevice( const FS::Entry& entry, Mode mode ):
   IODevice( mode ),
   _file( NULL ),
   _data( NULL )
{
   open( entry, mode ); // Call this to skip to the end.
}

//------------------------------------------------------------------------------
//!
MMapDevice::~MMapDevice()
{
   close();
}

//------------------------------------------------------------------------------
//!
bool
MMapDevice::open( const FS::Entry& entry, Mode mode )
{
   close();
   setMode( mode );
   _entry = entry;

#if PLAT_POSIX
   _file = fopen( entry.path().cstr(), modeToFOpen(mode) );
   if( _file == NULL )
   {
      StdErr << "ERROR - Could not open file '" << entry.path().string() << "'." << nl;
      setState( STATE_BAD );
      return false;
   }

   _data = mmap( 0, _entry.size(), modeToProt(mode), MAP_SHARED, fileno(_file), 0 ); // MAP_PRIVATE (copy-on-write), MAP_SHARED
   if( _data == MAP_FAILED )
   {
      StdErr << "ERROR - Could not map file '" << entry.path().string() << "'." << nl;
      _data = NULL;
      _pos  = NULL;
      setState( STATE_BAD );
      return false;
   }
   //if( madvise( _data, _entry.size(), MADV_SEQUENTIAL ) != 0 )
   //{
   //   StdErr << "ERROR - madvise() failed." << nl;
   //}
   _pos = (char*)_data;

   setState( STATE_OK );
   return true;

#elif PLAT_WINDOWS
   _file = fopen( entry.path().cstr(), modeToFOpen(mode) );
   if( _file == NULL )
   {
      StdErr << "ERROR - Could not open file '" << entry.path().string() << "'." << nl;
      setState( STATE_BAD );
      return false;
   }
   
   HANDLE handle = (HANDLE)_get_osfhandle( _fileno(_file) );
   if( handle == NULL )
   {
      StdErr << "ERROR - Could not retrieve file handle for '" << entry.path().string() << "', " << lastErr() << "." << nl;
      setState( STATE_BAD );
      return false;
   }

   _mapping = CreateFileMapping(
      handle,               // File handle.
      NULL,                 // Default security.
      modeToProt(mode),     // Page protection.
      high(_entry.size()),  // Max size, high.
      low (_entry.size()),  // Max size, low.
      NULL                  // No file mapping object name.
   );
   if( _mapping == NULL )
   {
      StdErr << "ERROR - Could not create file mapping for '" << entry.path().string() << "', " << lastErr() << "." << nl;
      setState( STATE_BAD );
      return false;
   }

   _data = MapViewOfFile(
      (HANDLE)_mapping,   // File mapping handle.
      modeToAccess(mode), // Desired access.
      0,                  // File offset, high.
      0,                  // File offset, low.
      _entry.size()       // Size of the mapping.
   );
   if( _data == NULL )
   {
      StdErr << "ERROR - Could not map file '" << entry.path().string() << "', " << lastErr() << "." << nl;
      if( !CloseHandle( (HANDLE)_mapping ) )
      {
         StdErr << "ERROR - Failed to close mapping for '" << entry.path().string() << "', " << lastErr() << "." << nl;
      }
      _mapping = NULL;
      _data = NULL;
      _pos  = NULL;
      setState( STATE_BAD );
      return false;
   }

   _pos = (char*)_data;

   setState( STATE_OK );
   return true;

#else
   return false;
#endif

}

//------------------------------------------------------------------------------
//!
void
MMapDevice::close()
{
   if( _data )
   {
#if PLAT_POSIX
      if( munmap( _data, _entry.size() ) != 0 )
      {
         StdErr << "ERROR - Could not unmap file '" << _entry.path().string() << "'." << nl;
         CHECK( false );
      }
#elif PLAT_WINDOWS
      if( !UnmapViewOfFile(_data) )
      {
         StdErr << "ERROR - Could not unmap file '" << _entry.path().string() << "', " << lastErr() << "." << nl;
         CHECK( false );
      }
      if( !CloseHandle( (HANDLE)_mapping ) )
      {
         StdErr << "ERROR - Failed to close mapping for '" << _entry.path().string() << "', " << lastErr() << "." << nl;
      }
      _mapping = NULL;
#else
      CHECK( false );
#endif
      _data = NULL;
      _pos  = NULL;
   }

   if( _file )
   {
      if( fclose( _file ) != 0 )
      {
         StdErr << "ERROR - Could not close file '" << _entry.path().string() << "'." << nl;
         CHECK( false );
      }
      _file = NULL;
   }

   setState( STATE_BAD );
}

//------------------------------------------------------------------------------
//!
bool
MMapDevice::doSeek( size_t pos )
{
   _pos += pos;
   return true;
}

//------------------------------------------------------------------------------
//!
size_t
MMapDevice::doPos() const
{
   return _pos - (char*)_data;
}

//------------------------------------------------------------------------------
//!
size_t
MMapDevice::doRead( char* data, size_t n )
{
   memcpy( data, _pos, n );
   _pos += n;
   return n;
}

//------------------------------------------------------------------------------
//!
size_t
MMapDevice::doPeek( char* data, size_t n )
{
   memcpy( data, _pos, n );
   return n;
}

//------------------------------------------------------------------------------
//!
size_t
MMapDevice::doWrite( const char* data, size_t n )
{
   memcpy( _pos, data, n );
   _pos += n;
   return n;
}

//------------------------------------------------------------------------------
//!
bool
MMapDevice::doFlush()
{
#if PLAT_POSIX
   return msync( _data, _entry.size(), MS_SYNC ) == 0;
#elif PLAT_WINDOWS
   return FlushViewOfFile( _data, 0 ) == TRUE;
#else
   return false;
#endif
}
