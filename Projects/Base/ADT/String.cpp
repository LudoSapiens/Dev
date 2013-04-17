/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Base/ADT/String.h>
#include <Base/Dbg/Defs.h>
#include <Base/Util/Bits.h>
#include <Base/Util/Compiler.h>
#include <Base/Util/CPU.h>

#include <cstdarg>
#include <cstdio>

#if defined(__CYGWIN__) && (__GNUC_VERSION__ == 40503)
// Should be in cstdarg, but it apparently isn't.
extern "C" int vsnprintf(char *str, size_t size, const char *format, va_list ap);
#endif

/*==============================================================================
  UNNAME NAMESPACE  
  ==============================================================================*/

UNNAMESPACE_BEGIN

// BUFSIZE corresponds to a relatively small size big enough for most cases.
const int BUFSIZE = 128;

UNNAMESPACE_END


NAMESPACE_BEGIN

/*==============================================================================
  CLASS String
==============================================================================*/

const String::SizeType String::npos = std::basic_string<char>::npos;

const String String::ws = String(" \n\r\t\v\f");

String::String( const char* str, SizeType n, UTF encoding )
{
   char   tmp[4];
   size_t tmpC;
   const char* cur = str;
   const char* end = str + n;
   char32_t c;
   while( cur < end )
   {
      cur = nextUTF( cur, encoding, c );
      tmpC = toUTF8( c, tmp );
      append( tmp, tmpC );
   }
}

////------------------------------------------------------------------------------
////!
//String::String( uchar v )
//{
//   *this += (char)v;
//   //format( "%hu", v );
//}
//
////------------------------------------------------------------------------------
////!
//String::String( short v )
//{
//   format( "%hd", v );
//}
//
////------------------------------------------------------------------------------
////!
//String::String( ushort v )
//{
//   format( "%hu", v );
//}
//
#if BASE_INT_NOT_INT32_T
//------------------------------------------------------------------------------
//!
String::String( int v )
{
   format( "%d", v );
}

//------------------------------------------------------------------------------
//!
String::String( uint v )
{
   format( "%u", v );
}
#endif

//------------------------------------------------------------------------------
//!
String::String( float v )
{
   format( "%g", v );
}

//------------------------------------------------------------------------------
//!
String::String( double v )
{
   format( "%g", v );
}

//------------------------------------------------------------------------------
//!
String::String( const void* ptr )
{
   format( "%p", ptr );
}

//------------------------------------------------------------------------------
//!
String::String( int8_t v )
{
   format( "%d", v );
}

//------------------------------------------------------------------------------
//!
String::String( uint8_t v )
{
   format( "%u", v );
}

//------------------------------------------------------------------------------
//!
String::String( int16_t v )
{
   format( "%hd", v );
}

//------------------------------------------------------------------------------
//!
String::String( uint16_t v )
{
   format( "%hu", v );
}

//------------------------------------------------------------------------------
//!
String::String( int32_t v )
{
   format( "%d", v );
}

//------------------------------------------------------------------------------
//!
String::String( uint32_t v )
{
   format( "%u", v );
}

//------------------------------------------------------------------------------
//!
String::String( int64_t v )
{
   format( FMT_INT64_T, v );
}

//------------------------------------------------------------------------------
//!
String::String( uint64_t v )
{
   format( FMT_UINT64_T, v );
}

#if defined(__APPLE__)
//------------------------------------------------------------------------------
//!
String::String( size_t v )
{
   format( FMT_SIZE_T, v );
}
#  if CPU_SIZE != 32
//------------------------------------------------------------------------------
//!
String::String( ptrdiff_t v )
{
   format( FMT_PTRDIFF_T, v );
}
#  endif
#endif // defined(__APPLE__)

//------------------------------------------------------------------------------
//!
void
String::split( const String& seps, Vector<String>& parts, const bool keepEmptyStrings ) const
{
   if( !seps.empty() )
   {
      if( keepEmptyStrings )
      {
         String::SizeType s = 0;
         String::SizeType e = 0;
         while( e < size() )
         {
            e = findFirstOf(seps, e+1);
            parts.pushBack( String(*this, s, e-s) );
            s = e + 1;
         }
      }
      else
      {
         String::SizeType s = findFirstNotOf(seps);
         String::SizeType e = findFirstOf(seps, s+1);
         while( s != String::npos )
         {
            parts.pushBack( String(*this, s, e-s) );
            s = findFirstNotOf(seps, e);
            e = findFirstOf(seps, s+1);
         }
      }
   }
   else
   {
      // Special case splitting all characters in separate strings.
      for( ConstIterator cur = begin(); cur != end(); ++cur )
      {
         parts.pushBack( String(*cur) );
      }
   }
}

//------------------------------------------------------------------------------
//!
String
String::operator*
( const uint f )
{
   String tmp;
   tmp.reserve(size()*f);
   for( uint i = 0; i < f; ++i )
   {
      tmp += *this;
   }
   return tmp;
}

//------------------------------------------------------------------------------
//!
String&
String::operator*=
( const uint f )
{
   if( f == 0 )
   {
      clear();
   }
   else
   {
      size_t s = size();
      reserve( s*f );
      for( uint i = 0; i < f; ++i )
      {
         append( cstr(), s );
      }
   }
   return (*this);
}

//------------------------------------------------------------------------------
//!
String&
String::format
( const char* fmt, ... )
{
   va_list argp;
   va_start( argp, fmt );

   char tmp[BUFSIZE];
   int req_size = vsnprintf( tmp, BUFSIZE, fmt, argp );
   if( req_size < BUFSIZE )
   {
      *this = tmp;
   }
   else
   {
      // We need more characters.
      ++req_size;  // Add one for the '\0'.
      char* tmp2 = new char[req_size];
      int req_size2 = vsnprintf( tmp2, req_size, fmt, argp );
      CHECK( req_size2 <= req_size );
      *this = tmp2;
      delete [] tmp2;
   }

   return *this;
}

//------------------------------------------------------------------------------
//!
String toStr( const float f )
{
   uint32_t s, e, m;
   toBits( f, s, e, m );
   return String().format("%g 0x%08X (s=%01X, e=0x%02X(%d), m=0x%06X)", f, toBits(f), s, e, (int)e-127, m);
}

NAMESPACE_END
