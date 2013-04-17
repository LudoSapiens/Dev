/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Base/IO/GZippedFileDevice.h>

#include <Base/Dbg/Defs.h>

#if defined(_MSC_VER)
//#define NULL_GZIP 1  // Temp.
#endif

#if NULL_GZIP
#define Z_NO_FLUSH 1
int gzclose( void* )
{
   return 0;
}
int gzeof( void* )
{
   return 1;
}
const char* gzerror( void*, int* )
{
   return "NULL_GZIP_ERROR";
}
int gzflush( void*, int )
{
   return 0;
}
void* gzopen( const char*, const char* )
{
   return NULL;
}
size_t gzread( void*, char*, size_t )
{
   return 0;
}
size_t gzseek( void*, size_t, int )
{
   return 0;
}
size_t gztell( void* )
{
   return 0;
}
size_t gzwrite( void*, const char*, size_t )
{
   return 0;
}
#else
#include <zlib.h>
#endif

USING_NAMESPACE

/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN

//------------------------------------------------------------------------------
//! Converts the Mode into the required string used in C's fopen() routine.
//! Returns a String in order to be thread-safe.
inline const char* convert( GZippedFileDevice::Mode mode )
{
   static const char* str[] = {
             //    NEWFILE
             //   /   WRITE
             //   |  /   READ
             //   |  |  /
      "",    //   0  0  0  (UNKNOWN)
      "rb",  //   0  0  1
      "wb9", //   0  1  0  (Keep previous content; user flag will prevent reads)
      "rb",  //   0  1  1  (Unsupported, favors read)
      "",    //   1  0  0  (INVALID)
      "rb",  //   1  0  1  (Ignore NEWFILE bit)
      "wb9", //   1  1  0
      "w+"   //   1  1  1  (Unsupported)
   };

   GZippedFileDevice::Mode mask = GZippedFileDevice::MODE_READ_WRITE | GZippedFileDevice::MODE_NEWFILE;
   return str[ mode & mask ];
}

UNNAMESPACE_END

//------------------------------------------------------------------------------
//!
GZippedFileDevice::GZippedFileDevice( const char* filename, Mode mode ):
   _file( NULL ),
   _skipClose( false )
{
   //fprintf( stderr, "Zlib: %s\n", zlibVersion() );
   open( filename, mode );
}

//------------------------------------------------------------------------------
//!
GZippedFileDevice::~GZippedFileDevice()
{
   close();
}

//------------------------------------------------------------------------------
//!
bool
GZippedFileDevice::open( const char* filename, Mode mode )
{
   close();

   setMode( mode );
   _file = gzopen( filename, convert(mode) );
   if( _file != NULL )
   {
      setState( STATE_OK );
      if( mode & GZippedFileDevice::MODE_MOVE_TO_END )
      {
         // FIXME: Seek to the end.
         fprintf( stderr, "GZippedFileDevice::open() - MOVE_TO_END flag not yet honored.\n" );
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
void
GZippedFileDevice::close()
{
   if( _file && !_skipClose )
   {
      if( gzclose( _file ) != 0 )
      {
         int err;
         gzerror( _file, &err );
         fprintf( stderr, "GZippedFileDevice::close - Error: %d.\n", err );
         CHECK( false );
      }
   }
   _file = NULL;
   setState( STATE_BAD );
}

//------------------------------------------------------------------------------
//!
bool
GZippedFileDevice::doSeek( size_t pos )
{
   if( !isStrict() )
   {
      int err = gzseek( _file, z_off_t(pos), SEEK_SET );
      if( gzeof(_file) )  addState( STATE_EOF );
      return err == 0;
   }
   return false;
}

//------------------------------------------------------------------------------
//!
size_t
GZippedFileDevice::doPos() const
{
   return gztell( _file );
}

//------------------------------------------------------------------------------
//!
size_t
GZippedFileDevice::doRead( char* data, size_t n )
{
   size_t b = gzread( _file, data, unsigned(n) );
   if( gzeof(_file) )  addState( STATE_EOF );
   return b;
}

//------------------------------------------------------------------------------
//!
size_t
GZippedFileDevice::doPeek( char* data, size_t n )
{
   size_t p = pos();
   size_t b = gzread( _file, data, unsigned(n) );
   seek( p );  // Bring back position to where it was.
   return b;
}

//------------------------------------------------------------------------------
//!
size_t
GZippedFileDevice::doWrite( const char* data, size_t n )
{
   size_t b = gzwrite( _file, data, unsigned(n) );
   if( b < n )
   {
      fprintf( stderr, "GZippedFileDevice::writeData() - Only wrote "FMT_SIZE_T" out of "FMT_SIZE_T" bytes\n", b, n );
   }
   return b;
}

//------------------------------------------------------------------------------
//!
bool
GZippedFileDevice::doFlush()
{
   int err = gzflush( _file, Z_NO_FLUSH );
   return err == 0;
}

//------------------------------------------------------------------------------
//!
bool
GZippedFileDevice::eof() const
{
   return gzeof(_file) != 0;
}

//------------------------------------------------------------------------------
//!
void
GZippedFileDevice::skip( size_t n )
{
   gzseek(_file, z_off_t(n), SEEK_CUR);
}
