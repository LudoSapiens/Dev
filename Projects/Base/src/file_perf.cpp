/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Base/IO/MMapDevice.h>
#include <Base/IO/FileDevice.h>
#include <Base/IO/TextStream.h>
#include <Base/Msg/Delegate.h>
#include <Base/Util/Timer.h>

#include <cmath>

USING_NAMESPACE

using namespace std;

String bytesToSize( size_t bytes )
{
   const char* sizes[] = { "bytes", "KiB", "MiB", "GiB", "TiB", "PiB", "EiB", "ZiB", "YiB" };
   if( bytes == 0 )  return String("0 byte");
   if( bytes == 1 )  return String("1 byte");

   double d = floor( log( double(bytes) ) / log( 1024.0 ) );
   int i = int( d );
   double r = bytes / pow( 1024.0, d );
   if( i == 0 )  return String().format("%g %s", r, sizes[i]);
   return String().format("%g %s", r, sizes[i]);
}

int _chunkSize = (1 << 16);

void  testFileDevice( const FS::Entry& e )
{
   StdErr << "FILE: " << e.path().string() << " chunk=" << _chunkSize << nl;
   int h[256];
   memset( h, 0, sizeof(h) );
   char* chunk = new char[_chunkSize];
   memset( chunk, 0, _chunkSize );

   Timer timer;
   RCP<FileDevice> dev = new FileDevice( e.path(), IODevice::MODE_READ );
   if( dev->ok() )
   {
      while( !dev->eof() )
      {
         size_t r = dev->read( chunk, _chunkSize );
         for( size_t i = 0; i < r; ++i )
         {
            ++h[uint8_t(chunk[i])];
         }
      }
   }
   double t = timer.elapsed();

   delete [] chunk;

   size_t n = 0;
   for( int y = 0; y < 16; ++y )
   {
      for( int x = 0; x < 16; ++x )
      {
         int i = y*16 + x;
         n += h[i];
         StdErr << String().format(" [%03d]: %-5d", i, h[i]);
      }
      StdErr << nl;
   }
   StdErr << t << " s." << nl;
   StdErr << "Total bytes read: " << n << " expected: " << e.size() << nl;
   StdErr << "File Throughput: " << bytesToSize(n/t) << "/s" << nl;
}

void  testMMapDevice( const FS::Entry& e )
{
   StdErr << "MMAP: " << e.path().string() << nl;
   int h[256];
   memset( h, 0, sizeof(h) );

   Timer timer;
   RCP<MMapDevice> dev = new MMapDevice( e, IODevice::MODE_READ );
   if( dev->ok() )
   {
      const char* str = dev->bytes();
      size_t n = e.size();
      for( size_t i = 0; i < n; ++i )
      {
         ++h[uint8_t(str[i])];
      }
   }
   double t = timer.elapsed();

   size_t n = 0;
   for( int y = 0; y < 16; ++y )
   {
      for( int x = 0; x < 16; ++x )
      {
         int i = y*16 + x;
         n += h[i];
         StdErr << String().format(" [%03d]: %-5d", i, h[i]);
      }
      StdErr << nl;
   }
   StdErr << t << " s." << nl;
   StdErr << "Total bytes read: " << n << " expected: " << e.size() << nl;
   StdErr << "MMap Throughput: " << bytesToSize(n/t) << "/s" << nl;
}

int main( int argc, char* argv[] )
{
   Delegate1<const FS::Entry&> delegate;
   delegate = &testFileDevice;
   for( int i = 1; i < argc; ++i )
   {
      if( argv[i][0] == '-' )
      {
         // Options.
         switch( argv[i][1] )
         {
            case 'c':
               ++i;
               _chunkSize = 1 << atoi( argv[i] );
               break;
            case 'f':
               delegate = &testFileDevice;
               break;
            case 'm':
               delegate = &testMMapDevice;
               break;
            default:
               StdErr << "Ignoring: " << argv[i] << nl;
               break;
         }
      }
      else
      {
         delegate( FS::Entry(argv[i]) );
      }
   }
}
