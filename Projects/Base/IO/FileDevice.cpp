/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Base/IO/FileDevice.h>

#include <Base/Dbg/Defs.h>
#include <Base/Util/Compiler.h>

//------------------------------------------------------------------------------
//! Since this file is a basis for the DebugStream routines, all debug messages
//! probably need to be sent through fprintf( stderr, ... ).

#if defined(_MSC_VER) || (defined(__CYGWIN__) && (__GNUC_VERSION__ == 40503))
// Visual Studio (and Cygwin+GCC) doesn't support the off_t variants.

inline size_t ftello( FILE* stream )
{
   return ftell( stream );
}

inline int fseeko( FILE* stream, size_t offset, int whence )
{
   return fseek( stream, (long)offset, whence );
}

#endif

USING_NAMESPACE

/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN

//------------------------------------------------------------------------------
//! Converts the Mode into the required string used in C's fopen() routine.
//! Returns a String in order to be thread-safe.
inline const char* convert( FileDevice::Mode mode )
{
   static const char* str[] = {
             //    NEWFILE
             //   /   WRITE
             //   |  /   READ
             //   |  |  /
      "",    //   0  0  0  (UNKNOWN)
      "rb",  //   0  0  1
      "wb+", //   0  1  0  (Keep previous content; user flag will prevent reads)
      "rb+", //   0  1  1
      "",    //   1  0  0  (INVALID)
      "rb",  //   1  0  1  (Ignore NEWFILE bit)
      "wb",  //   1  1  0
      "wb+"  //   1  1  1  (Keep previous content)
   };

   FileDevice::Mode mask = FileDevice::MODE_READ_WRITE | FileDevice::MODE_NEWFILE;
   return str[ mode & mask ];
}

UNNAMESPACE_END

//------------------------------------------------------------------------------
//!
FileDevice::FileDevice( FILE* file, Mode mode ):
   IODevice( mode ),
   _file( NULL ),
   _skipClose( true )
{
   open( file, mode ); // Call this to skip to the end.
}

//------------------------------------------------------------------------------
//!
FileDevice::FileDevice( const char* filename, Mode mode ):
   _file( NULL ),
   _skipClose( false )
{
   open( filename, mode );
}

//------------------------------------------------------------------------------
//!
FileDevice::FileDevice( const String& filename, Mode mode ):
   _file( NULL ),
   _skipClose( false )
{
   open( filename, mode );
}

//------------------------------------------------------------------------------
//!
FileDevice::FileDevice( const Path& path, Mode mode ):
   _file( NULL ),
   _skipClose( false )
{
   open( path, mode );
}

//------------------------------------------------------------------------------
//!
FileDevice::~FileDevice()
{
   close();
}

//------------------------------------------------------------------------------
//!
bool
FileDevice::open( FILE* file, Mode mode )
{
   close();
   setMode( mode );
   _file = file;
   if( _file != NULL )
   {
      setState( STATE_OK );
      if( mode & FileDevice::MODE_MOVE_TO_END )
      {
         // FIXME: Seek to the end.
         fprintf( stderr, "FileDevice::open() - MOVE_TO_END flag not yet honored.\n" );
      }
      return true;
   }
   else
   {
      setState( STATE_BAD );
      return false;
   }
}

//------------------------------------------------------------------------------
//!
bool
FileDevice::open( const char* filename, Mode mode )
{
   return open( fopen( filename, convert(mode) ), mode );
}

//------------------------------------------------------------------------------
//!
bool
FileDevice::open( const String& filename, Mode mode )
{
   return open( filename.cstr(), mode );
}

//------------------------------------------------------------------------------
//!
bool
FileDevice::open( const Path& path, Mode mode )
{
   return open( path.cstr(), mode );
}

//------------------------------------------------------------------------------
//!
void
FileDevice::close()
{
   if( _file && !_skipClose )
   {
      if( fclose( _file ) != 0 )
      {
         fprintf( stderr, "FileDevice::close - Error: %d.\n", ferror(_file) );
         CHECK( false );
      }
   }
   _file = NULL;
   setState( STATE_BAD );
}

//------------------------------------------------------------------------------
//!
bool
FileDevice::doSeek( size_t pos )
{
   if( !isStrict() )
   {
      int err = fseeko( _file, pos, SEEK_SET );
      if( feof(_file) )  addState( STATE_EOF );
      return err == 0;
   }
   return false;
}

//------------------------------------------------------------------------------
//!
size_t
FileDevice::doPos() const
{
   return ftello( _file );
}

//------------------------------------------------------------------------------
//!
size_t
FileDevice::doRead( char* data, size_t n )
{
   size_t b = fread( data, sizeof(char), n, _file );
   if( feof(_file) )  addState( STATE_EOF );
   return b;
}

//------------------------------------------------------------------------------
//!
size_t
FileDevice::doPeek( char* data, size_t n )
{
   size_t p = pos();
   size_t b = fread( data, sizeof(char), n, _file );
   seek( p );  // Bring back position to where it was.
   return b;
}

//------------------------------------------------------------------------------
//!
size_t
FileDevice::doWrite( const char* data, size_t n )
{
   size_t b = fwrite( data, sizeof(char), n, _file );
   if( b < n )
   {
      fprintf( stderr, "FileDevice::writeData() - Only wrote "FMT_SIZE_T" out of "FMT_SIZE_T" bytes\n", b, n );
   }
   return b;
}

//------------------------------------------------------------------------------
//!
bool
FileDevice::doFlush()
{
   int err = fflush( _file );
   return err == 0;
}
