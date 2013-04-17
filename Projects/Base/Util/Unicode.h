/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef BASE_UNICODE_H
#define BASE_UNICODE_H

#include <Base/StdDefs.h>

#include <Base/Util/Compiler.h>

//#include <string>

NAMESPACE_BEGIN

#if COMPILER_CXX11 || (COMPILER == COMPILER_MSVC)
// C++11 already has char16_t and char32_t built in.

#else

// Potentially define char16_t and char32_t, if it doesn't already exist.
#if !defined(char16_t) && !defined(_CHAR16T)
typedef uint16_t  char16_t;
#endif

#if !defined(char32_t) && !defined(_CHAR32T)
typedef uint32_t  char32_t;
#endif

#endif // C++0x


#define INVALID_CODEPOINT  char32_t(-1)

//-----------------------------------------------------------------------------
//!
inline bool  invalid( char32_t code )
{
   return code > 0x10FFFF;
}


enum UTF
{
   UTF_UNKNOWN,
   UTF8,
   UTF16,
   UTF16BE,
   UTF16LE,
   UTF32,
   UTF32BE,
   UTF32LE
};

//-----------------------------------------------------------------------------
//!
inline bool  errorUTF32( char32_t code )
{
   return ((code >> 31) & 0x01) == 0x01;
}

//-----------------------------------------------------------------------------
//!
BASE_DLL_API const char* readBOM( const char* cur, UTF& utf );

//-----------------------------------------------------------------------------
//!
inline bool  isUTF8Continuation( char c )
{
   return (c & 0xC0) == 0x80;
}

//-----------------------------------------------------------------------------
//!
BASE_DLL_API const char* nextUTF8( const char* cur, char32_t& code );

//-----------------------------------------------------------------------------
//!
BASE_DLL_API const char* nextUTF8( const char* cur );

//-----------------------------------------------------------------------------
//!
BASE_DLL_API const char* skipUTF8( const char* cur, size_t numCodepoints );

//-----------------------------------------------------------------------------
//!
BASE_DLL_API const char16_t* nextUTF16( const char16_t* cur, char32_t& code );

//-----------------------------------------------------------------------------
//!
BASE_DLL_API const char* nextUTF16BE( const char* cur, char32_t& code );

//-----------------------------------------------------------------------------
//!
BASE_DLL_API const char* nextUTF16LE( const char* cur, char32_t& code );

//-----------------------------------------------------------------------------
//!
BASE_DLL_API const char* nextUTF32BE( const char* cur, char32_t& code );

//-----------------------------------------------------------------------------
//!
BASE_DLL_API const char* nextUTF32LE( const char* cur, char32_t& code );

//-----------------------------------------------------------------------------
//!
BASE_DLL_API char32_t toUTF32( const char* src );

//-----------------------------------------------------------------------------
//!
BASE_DLL_API char32_t toUTF32( const char16_t* src );

//-----------------------------------------------------------------------------
//!
BASE_DLL_API size_t toUTF8( char32_t src, char* dst );

//-----------------------------------------------------------------------------
//!
inline size_t toUTF8( const char32_t* src, char* dst )
{
   return toUTF8( src[0], dst );
}

//-----------------------------------------------------------------------------
//!
BASE_DLL_API size_t toUTF8( const char16_t* src, char* dst );

//-----------------------------------------------------------------------------
//!
BASE_DLL_API size_t toUTF16( char32_t src, char16_t* dst );

//-----------------------------------------------------------------------------
//!
inline size_t toUTF16( const char32_t* src, char16_t* dst )
{
   return toUTF16( src[0], dst );
}

//-----------------------------------------------------------------------------
//!
BASE_DLL_API size_t toUTF16( const char* src, char16_t* dst );

//-----------------------------------------------------------------------------
//!
BASE_DLL_API size_t getUTF8Length( const char* str, const size_t strLen );

//-----------------------------------------------------------------------------
//!
BASE_DLL_API size_t getUTF8Size( const char32_t code );

//-----------------------------------------------------------------------------
//!
BASE_DLL_API size_t getUTF8Size( const char* str );

//-----------------------------------------------------------------------------
//!
BASE_DLL_API size_t getUTF8Size( const char16_t* str, const size_t strLen );

//-----------------------------------------------------------------------------
//!
BASE_DLL_API size_t getUTF8Size( const char32_t* str, const size_t strLen );

//-----------------------------------------------------------------------------
//!
BASE_DLL_API size_t getUTF16Size( const char32_t code );

//-----------------------------------------------------------------------------
//!
BASE_DLL_API size_t getUTF16Size( const char* str, const size_t strLen );

//-----------------------------------------------------------------------------
//!
BASE_DLL_API size_t getUTF16Size( const char32_t* str, const size_t strLen );

//-----------------------------------------------------------------------------
//!
BASE_DLL_API size_t getUTF16Length( const char16_t* str, const size_t strLen );

//-----------------------------------------------------------------------------
//!
BASE_DLL_API size_t getUTF16BELength( const char* str, const size_t strLen );

//-----------------------------------------------------------------------------
//!
BASE_DLL_API size_t getUTF16LELength( const char* str, const size_t strLen );

//-----------------------------------------------------------------------------
//!
inline const char* nextUTF( const char* str, const UTF utf, char32_t& code )
{
   switch( utf )
   {
      case UTF_UNKNOWN: return nextUTF8( str, code );
      case UTF8       : return nextUTF8( str, code );
      case UTF16      : return nextUTF16LE( str, code );
      case UTF16BE    : return nextUTF16BE( str, code );
      case UTF16LE    : return nextUTF16LE( str, code );
      case UTF32      : return nextUTF32LE( str, code );
      case UTF32BE    : return nextUTF32BE( str, code );
      case UTF32LE    : return nextUTF32LE( str, code );
      default         : return nextUTF8( str, code );
   }
}

NAMESPACE_END

#endif //BASE_UNICODE_H
