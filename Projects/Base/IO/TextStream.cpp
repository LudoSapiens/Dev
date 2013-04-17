/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Base/IO/TextStream.h>

#include <Base/IO/FileDevice.h>
#include <Base/IO/NullDevice.h>
#include <Base/IO/StringDevice.h>

/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN



UNNAMESPACE_END


NAMESPACE_BEGIN

//------------------------------------------------------------------------------
//!
TextStream  StdErr = TextStream( new FileDevice(stderr, FileDevice::MODE_WRITE | FileDevice::MODE_STRICT) );

//------------------------------------------------------------------------------
//!
TextStream  StdIn = TextStream( new FileDevice(stdin, FileDevice::MODE_READ | FileDevice::MODE_STRICT) );

//------------------------------------------------------------------------------
//!
TextStream  StdOut = TextStream( new FileDevice(stdout, FileDevice::MODE_WRITE | FileDevice::MODE_STRICT) );

//------------------------------------------------------------------------------
//!
TextStream  StdNull = TextStream( new NullDevice(FileDevice::MODE_READ_WRITE | FileDevice::MODE_STRICT) );

NAMESPACE_END


/*==============================================================================
  CLASS TextStream
==============================================================================*/

//------------------------------------------------------------------------------
//!
TextStream::LineIterator&
TextStream::LineIterator::operator++()
{
   //StdErr << nl
   //       << "Starting valid=" << isValid() << " bufPos=" << _bufPos << " bufSize=" << _bufSize << nl
   //       << " buf=" << String(_buf, _bufSize) << "=buf" << nl
   //       << " line=" << _line << "=line" << nl;

   _line.clear();
   if( _bufSize > 0 )
   {
      size_t s = _bufPos;
      while( true )
      {
         // Look for EOL character.
         while( _buf[_bufPos] != _delim )
         {
            ++_bufPos;
         }

         if( _bufPos < _bufSize )
         {
            //StdErr << "Valid line s=" << s << " _bufPos=" << _bufPos << " size=" << (_bufPos - s) << nl;
            // Did not hit the sentinel, so this is a valid delimiter.
            _line.append( _buf + s, _bufPos - s );
            ++_bufPos; // Prepare next line.
            break;
         }
         else
         {
            //StdErr << "Sentinel s=" << s << " _bufPos=" << _bufPos << " _bufSize=" << _bufSize << nl;
            // Hit the sentinel, append and keep going.
            _line.append( _buf + s, _bufPos - s );
            _bufSize = _ts->device()->read( _buf, _bufSize );
            _buf[_bufSize] = _delim; // Put sentinel.
            s = _bufPos = 0; // Continue from beginning.
            if( _bufSize == 0 )
            {
               // Done reading the file (but not invalid for this time).
               //StdErr << "early out" << nl;
               break;
            }
            //StdErr << "New Chunk " << _bufSize << nl
            //       << " buf=" << String(_buf, _bufSize) << "=buf" << nl;
         }
      }
   }
   else
   {
      // We're done.
      _ts = NULL;
   }

   //StdErr << "Ending valid=" << isValid() << " bufPos=" << _bufPos << " bufSize=" << _bufSize << nl
   //       << " buf=" << String(_buf, _bufSize) << "=buf" << nl
   //       << " line=" << _line << "=line" << nl;

   return *this;
}


/*==============================================================================
  CLASS TextStream
==============================================================================*/

//------------------------------------------------------------------------------
//!
TextStream::TextStream( IODevice* device ):
   _device( device )
{
}

//------------------------------------------------------------------------------
//!
TextStream::TextStream( String& str )
{
   _device = new StringDevice( str );
}

//------------------------------------------------------------------------------
//!
TextStream::~TextStream()
{
}

//------------------------------------------------------------------------------
//!
bool
TextStream::flush()
{
   if( _device.isValid() )
   {
      return _device->flush();
   }
   return true;
}

//------------------------------------------------------------------------------
//!
TextStream&
TextStream::operator>>( String& str )
{
   str.clear();
   char rtmp[128+1];
   char stmp[128+1];
   rtmp[128] = 0;
   while( true )
   {
      size_t p = _device->peek( rtmp, 128 );
      if( p > 0 )
      {
         rtmp[p] = '\0'; // Peek doesn't NULL-terminate the string.
         int s;
         if( sscanf( rtmp, "%128s%n", stmp, &s ) == 1 )
         {
            str += stmp;
            _device->seek( _device->pos() + s );
            if( s != 128 ) break;
         }
         else
         {
            // Must have been whitespaces, simply advance.
            _device->seek( _device->pos() + p );
            if( p != 128 ) break;
         }
      }
      else
      {
         // Force EOF so be raised.
         _device->read( rtmp, 1 );
         break;
      }
   }
   return *this;
}

////------------------------------------------------------------------------------
////!
//TextStream&
//TextStream::operator>>( char* str )
//{
//   // TODO.
//   return *this;
//}

//------------------------------------------------------------------------------
//!
TextStream&
TextStream::operator>>( char& val )
{
   _device->read( &val, 1 );
   return *this;
}

//------------------------------------------------------------------------------
//!
TextStream&
TextStream::operator>>( uchar& val )
{
   _device->read( (char*)&val, 1 );
   return *this;
}

//------------------------------------------------------------------------------
//!
TextStream&
TextStream::operator>>( short& /*val*/ )
{
   // TODO.
   return *this;
}

//------------------------------------------------------------------------------
//!
TextStream&
TextStream::operator>>( ushort& /*val*/ )
{
   // TODO.
   return *this;
}

//------------------------------------------------------------------------------
//!
TextStream&
TextStream::operator>>( int& /*val*/ )
{
   // TODO.
   return *this;
}

//------------------------------------------------------------------------------
//!
TextStream&
TextStream::operator>>( uint& /*val*/ )
{
   // TODO.
   return *this;
}

////------------------------------------------------------------------------------
////!
//TextStream&
//TextStream::operator>>( size_t& val )
//{
//   // TODO.
//   return *this;
//}

//------------------------------------------------------------------------------
//!
TextStream&
TextStream::operator>>( float& /*val*/ )
{
   // TODO.
   return *this;
}

//------------------------------------------------------------------------------
//!
TextStream&
TextStream::operator>>( double& /*val*/ )
{
   // TODO.
   return *this;
}

//------------------------------------------------------------------------------
//!
TextStream&
TextStream::operator>>( void*& /*ptr*/ )
{
   // TODO.
   return *this;
}
