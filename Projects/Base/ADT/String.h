/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef BASE_STRING_H
#define BASE_STRING_H

#include <Base/StdDefs.h>

#include <Base/ADT/Vector.h>
#include <Base/Util/Compiler.h>
#include <Base/Util/CPU.h>
#include <Base/Util/Unicode.h>

#include <string>
#include <cctype>

// Formats for size_t, ptrdiff_t, void*, and {u}int64 types.
//
// Notes:
// 1) Apple uses:
//       size_t == long unsigned int,
//       ptrdiff_t == long int for 64b (same as above), but int for 32b (inconsistent)
// 2) We previously used long longs for 64b platforms, but longs might be sufficient
//    and more consistent across platforms.

#if COMPILER_CXX11

#define FMT_SIZE_T         "%zu"
#define FMT_HEX_SIZE_T     "%zx"
#define FMT_PTRDIFF_T      "%td"
#define FMT_HEX_PTRDIFF_T  "%tx"

#else

#define FMT_SIZE_T         "%lu"
#define FMT_HEX_SIZE_T     "%lx"

#if defined(__APPLE__) && (CPU_SIZE == 32)
#define FMT_PTRDIFF_T      "%d"
#define FMT_HEX_PTRDIFF_T  "%x"
#else
#define FMT_PTRDIFF_T      "%ld"
#define FMT_HEX_PTRDIFF_T  "%lx"
#endif

#endif //COMPILER_CXX11

#define FMT_INT64_T        "%lld"
#define FMT_UINT64_T       "%llu"
#define FMT_HEX_INT64_T    "%llx"
#define FMT_HEX_UINT64_T   "%llx"

#if CPU_SIZE > 32
#define FMT_VOID_PTR       "%016lu"
#define FMT_HEX_VOID_PTR   "%016lx"
#else
#define FMT_VOID_PTR       "%08u"
#define FMT_HEX_VOID_PTR   "%08x"
#endif

NAMESPACE_BEGIN

/*==============================================================================
  CLASS String
==============================================================================*/

//!

class String
   : private std::basic_string<char>
{

public:

   /*----- types and enumerations ----*/

   typedef int (*CharClassFunc)( int );

   typedef std::basic_string<char>::const_iterator ConstIterator;
   typedef std::basic_string<char>::iterator       Iterator;
   typedef std::basic_string<char>::size_type      SizeType;

   /*----- static members -----*/

   static BASE_DLL_API const SizeType npos;
   static BASE_DLL_API const String   ws;

   /*----- methods -----*/

            BASE_DLL_API String();
            BASE_DLL_API String( const String& str, SizeType pos = 0, SizeType n = npos );
            BASE_DLL_API String( const char* str );
            BASE_DLL_API String( const char* str, SizeType n );
            BASE_DLL_API String( const char* str, SizeType n, UTF encoding );
   //explicit BASE_DLL_API String( const char* fmt, ... );
   explicit BASE_DLL_API String( char c );
   explicit BASE_DLL_API String( char c, uint nCopies );
   //explicit BASE_DLL_API String( uchar v );
   //explicit BASE_DLL_API String( short v );
   //explicit BASE_DLL_API String( ushort v );
#if BASE_INT_NOT_INT32_T
   explicit BASE_DLL_API String( int v );
   explicit BASE_DLL_API String( uint v );
#endif
   explicit BASE_DLL_API String( float v );
   explicit BASE_DLL_API String( double v );
   explicit BASE_DLL_API String( int8_t v );
   explicit BASE_DLL_API String( uint8_t v );
   explicit BASE_DLL_API String( int16_t v );
   explicit BASE_DLL_API String( uint16_t v );
   explicit BASE_DLL_API String( int32_t v );
   explicit BASE_DLL_API String( uint32_t v );
   explicit BASE_DLL_API String( int64_t v );
   explicit BASE_DLL_API String( uint64_t v );
#if defined(__APPLE__)
   explicit BASE_DLL_API String( size_t v );
#  if CPU_SIZE != 32
   explicit BASE_DLL_API String( ptrdiff_t v );
#  endif
#endif
   explicit BASE_DLL_API String( const void* ptr );

   inline Iterator begin();
   inline Iterator end();
   inline ConstIterator begin() const;
   inline ConstIterator end() const;

   inline void reserve( SizeType );
   inline void resize( SizeType );
   inline bool empty() const;
   inline SizeType size() const;
   inline SizeType length() const;
   inline const char* cstr() const;
   inline void clear();
   inline String upper() const;
   inline String lower() const;
   inline String eatLeadingWhites() const;
   inline String eatTrailingWhites() const;
   inline String eatWhites() const;
   inline int compare( const String& str ) const;
   inline int compare( SizeType startPos, const String& str, SizeType strStartPos, SizeType n ) const;
   inline int compare( const char* str ) const;
   inline bool startsWith( const String& str ) const;
   inline bool endsWith( const String& str ) const;
   inline String& erase( SizeType pos );
   inline String& erase( SizeType pos, SizeType n );
   inline String& insert( SizeType pos, const String& str );
   inline String& insert( SizeType pos, const char* str );
   inline String& insert( SizeType pos, const char* str, SizeType n );
   inline String& insert( SizeType pos, char c );
   inline String sub( SizeType pos, SizeType n = npos ) const;

   inline SizeType find( char c, SizeType pos = 0 ) const;
   inline SizeType find( const char* s, SizeType pos = 0 ) const;
   inline SizeType find( const char* s, SizeType pos, SizeType n ) const;
   inline SizeType find( const String& str, SizeType pos = 0 ) const;
   inline SizeType rfind( char c, SizeType pos = npos ) const;
   inline SizeType rfind( const char* s, SizeType pos = npos ) const;
   inline SizeType rfind( const char* s, SizeType pos, SizeType n ) const;
   inline SizeType rfind( const String& str, SizeType pos = npos ) const;

   inline SizeType findFirstOf( CharClassFunc func, SizeType pos = 0 ) const;
   inline SizeType findFirstNotOf( CharClassFunc func, SizeType pos = 0 ) const;

   inline SizeType findFirstWhite( SizeType pos = 0 ) const;
   inline SizeType findFirstNonWhite( SizeType pos = 0 ) const;

   inline SizeType findFirstDigit( SizeType pos = 0 ) const;
   inline SizeType findFirstNonDigit( SizeType pos = 0 ) const;

   inline SizeType findFirstOf( const String& charSet, SizeType pos = 0 ) const;
   inline SizeType findFirstNotOf( const String& charSet, SizeType pos = 0 ) const;

   inline SizeType findLastOf( const String& charSet, SizeType pos = npos ) const;
   inline SizeType findLastNotOf( const String& charSet, SizeType pos = npos ) const;

   inline String getExt() const;

   inline String& replace( SizeType pos, SizeType n, const String& str );

   BASE_DLL_API void split( const String& seps, Vector<String>& parts, const bool keepEmptyStrings = false ) const;

   inline double toDouble() const;
   inline float  toFloat() const;
   inline int    toInt() const;

   inline char& operator[]( SizeType n );
   inline const char& operator[]( SizeType n ) const;

   inline String& operator=( const String& );
   inline String& operator=( const char* );
   inline String& operator=( char );

   inline String& operator+=( const String& );
   inline String& operator+=( const char* );
   inline String& operator+=( char );
   inline String& append( const char* str, size_t n );

   inline bool operator==( const String& ) const;
   inline bool operator==( const char* ) const;

   BASE_DLL_API String  operator*( const uint f );
   BASE_DLL_API String& operator*=( const uint f );

   BASE_DLL_API String& format( const char* fmt, ... );
private:

   /*----- types and enumerations ----*/

   typedef std::basic_string<char> BaseClass;

};

//------------------------------------------------------------------------------
//!
BASE_DLL_API String toStr( const float f );

//------------------------------------------------------------------------------
//!
inline String::String()
{
   // FIXME: This is a temporary fix until we rewrite completely the String
   // class. gcc is now using static data for the null string and this create a
   // really hard to fix bug.
   reserve( 5 );
}

//------------------------------------------------------------------------------
//!
inline String::String( const String& str, SizeType pos, SizeType n )
   : BaseClass( str, pos, n )
{}

//------------------------------------------------------------------------------
//!
inline String::String( const char* str )
{
   this->operator=(str);
}

//------------------------------------------------------------------------------
//!
inline String::String( const char* str, SizeType n ):
   BaseClass( str, n )
{}

//------------------------------------------------------------------------------
//!
inline String::String( char ch )
{
   this->operator=(ch);
}

//------------------------------------------------------------------------------
//!
inline String::String( char c, uint nCopies ):
   BaseClass( nCopies,  c )
{
}

//------------------------------------------------------------------------------
//!
inline String::Iterator
String::begin()
{
   return BaseClass::begin();
}

//------------------------------------------------------------------------------
//!
inline String::Iterator
String::end()
{
   return BaseClass::end();
}

//------------------------------------------------------------------------------
//!
inline String::ConstIterator
String::begin() const
{
   return BaseClass::begin();
}

//------------------------------------------------------------------------------
//!
inline String::ConstIterator
String::end() const
{
   return BaseClass::end();
}

//------------------------------------------------------------------------------
//!
inline void
String::reserve( String::SizeType size )
{
   return BaseClass::reserve( size );
}

//------------------------------------------------------------------------------
//!
inline void
String::resize( String::SizeType size )
{
   BaseClass::resize( size );
}

//------------------------------------------------------------------------------
//!
inline bool
String::empty() const
{
   return BaseClass::empty();
}

//------------------------------------------------------------------------------
//!
inline String::SizeType
String::size() const
{
   return BaseClass::size();
}

//------------------------------------------------------------------------------
//!
inline String::SizeType
String::length() const
{
   return getUTF8Length( cstr(), size() );
}

//------------------------------------------------------------------------------
//!
inline const char*
String::cstr() const
{
   return BaseClass::c_str();
}

//------------------------------------------------------------------------------
//!
inline void
String::clear()
{
   BaseClass::clear();
}

//------------------------------------------------------------------------------
//!
inline String
String::upper() const
{
   String str( *this );

   for ( Iterator it = str.begin(); it != str.end(); ++ it )
   {
      (*it) = toupper( *it );
   }
   return str;
}

//------------------------------------------------------------------------------
//!
inline String
String::lower() const
{
   String str( *this );

   for ( Iterator it = str.begin(); it != str.end(); ++ it )
   {
      (*it) = tolower( *it );
   }
   return str;
}

//------------------------------------------------------------------------------
//!
inline String
String::eatLeadingWhites() const
{
   ConstIterator it;

   for ( it = begin(); it != end(); ++ it )
   {
      if( isprint(*it) && !isspace(*it) )
         break;
   }

   if( it == end() )
      return String();
   else
      return sub( it-begin(), end()-it );
}

//------------------------------------------------------------------------------
//!
inline String
String::eatTrailingWhites() const
{
   ConstIterator it;

   for ( it = end(); it != begin(); --it )
   {
      if( isprint(*(it-1)) && !isspace(*(it-1)) )
         break;
   }

   if( it == begin() )
      return String();
   else
      return sub( 0, it- begin() );
}

//------------------------------------------------------------------------------
//!
inline String
String::eatWhites() const
{
   ConstIterator firstNonWhite;
   for( firstNonWhite = begin(); firstNonWhite != end(); ++firstNonWhite )
   {
      if( isprint(*firstNonWhite) && !isspace(*firstNonWhite) )
         break;
   }

   ConstIterator lastNonWhite;
   for( lastNonWhite = end(); lastNonWhite != begin(); --lastNonWhite )
   {
      if( isprint(*(lastNonWhite-1)) && !isspace(*(lastNonWhite-1)) )
         break;
   }

   return sub( firstNonWhite-begin(), lastNonWhite-firstNonWhite );
}


//------------------------------------------------------------------------------
//!
inline int
String::compare( const String& str ) const
{
   return BaseClass::compare( str );
}

//------------------------------------------------------------------------------
//!
inline int
String::compare( SizeType startPos, const String& str, SizeType strStartPos, SizeType n ) const
{
   return BaseClass::compare( startPos, n, str, strStartPos, n );
}

//------------------------------------------------------------------------------
//!
inline int
String::compare( const char* str ) const
{
   return BaseClass::compare( str );
}

//------------------------------------------------------------------------------
//!
inline bool
String::startsWith( const String& str ) const
{
   return compare( 0, str, 0, str.size() ) == 0;
}

//------------------------------------------------------------------------------
//!
inline bool
String::endsWith( const String& str ) const
{
   SizeType n = str.size();
   SizeType s = size();
   if( s > n )  s -= n; // Clamp to avoid underflows
   else         s = 0;
   return compare( s, str, 0, n ) == 0;
}

//------------------------------------------------------------------------------
//!
inline String&
String::erase( SizeType pos )
{
   BaseClass::erase( pos, 1 );
   return *this;
}

//------------------------------------------------------------------------------
//!
inline String&
String::erase( SizeType pos, SizeType n )
{
   BaseClass::erase( pos, n );
   return *this;
}

//------------------------------------------------------------------------------
//!
inline String&
String::insert( SizeType pos, const String& str )
{
   BaseClass::insert( pos, str );
   return *this;
}

//------------------------------------------------------------------------------
//!
inline String&
String::insert( SizeType pos, const char* str )
{
   BaseClass::insert( pos, str );
   return *this;
}

//------------------------------------------------------------------------------
//!
inline String&
String::insert( SizeType pos, const char* str, SizeType n )
{
   BaseClass::insert( pos, str, n );
   return *this;
}

//------------------------------------------------------------------------------
//!
inline String&
String::insert( SizeType pos, char c )
{
   BaseClass::insert( pos, 1, c );
   return *this;
}

//------------------------------------------------------------------------------
//!
inline String
String::sub( SizeType pos, SizeType n ) const
{
   return String( *this, pos, n );
}

//------------------------------------------------------------------------------
//!
inline String::SizeType
String::find( char c, SizeType pos ) const
{
   return BaseClass::find( c, pos );
}

//------------------------------------------------------------------------------
//!
inline String::SizeType
String::find( const char* s, SizeType pos ) const
{
   return BaseClass::find( s, pos );
}

//------------------------------------------------------------------------------
//!
inline String::SizeType
String::find( const char* s, SizeType pos, SizeType n ) const
{
   return BaseClass::find( s, pos, n );
}

//------------------------------------------------------------------------------
//!
inline String::SizeType
String::find( const String& str, SizeType pos ) const
{
   return BaseClass::find( str, pos );
}

//------------------------------------------------------------------------------
//!
inline String::SizeType
String::rfind( char c, SizeType pos ) const
{
   return BaseClass::rfind( c, pos );
}

//------------------------------------------------------------------------------
//!
inline String::SizeType
String::rfind( const char* s, SizeType pos ) const
{
   return BaseClass::rfind( s, pos );
}

//------------------------------------------------------------------------------
//!
inline String::SizeType
String::rfind( const char* s, SizeType pos, SizeType n ) const
{
   return BaseClass::rfind( s, pos, n );
}

//------------------------------------------------------------------------------
//!
inline String::SizeType
String::rfind( const String& str, SizeType pos ) const
{
   return BaseClass::rfind( str, pos );
}

//------------------------------------------------------------------------------
//!
inline String::SizeType
String::findFirstOf( CharClassFunc func, SizeType pos ) const
{
   if( pos >= size() )
   {
	   return BaseClass::npos;
   }

   ConstIterator it;

   for ( it = begin()+pos; it < end(); ++ it )
   {
      if( func(*it) )
         break;
   }

   if( it >= end() )
   {
      return BaseClass::npos;
   }

   return it - begin();
}

//------------------------------------------------------------------------------
//!
inline String::SizeType
String::findFirstNotOf( CharClassFunc func, SizeType pos ) const
{
   if( pos >= size() )
   {
	   return BaseClass::npos;
   }

   ConstIterator it;

   for ( it = begin()+pos; it < end(); ++ it )
   {
      if( !func(*it) )
         break;
   }

   if( it >= end() )
   {
      return BaseClass::npos;
   }

   return it - begin();
}

//------------------------------------------------------------------------------
//!
inline String::SizeType
String::findFirstWhite( SizeType pos ) const
{
   return findFirstOf( isspace, pos );
}

//------------------------------------------------------------------------------
//!
inline String::SizeType
String::findFirstNonWhite( SizeType pos ) const
{
   return findFirstNotOf( isspace, pos );
}

//------------------------------------------------------------------------------
//!
inline String::SizeType
String::findFirstDigit( SizeType pos ) const
{
   return findFirstOf( isdigit, pos );
}

//------------------------------------------------------------------------------
//!
inline String::SizeType
String::findFirstNonDigit( SizeType pos ) const
{
   return findFirstNotOf( isdigit, pos );
}

//------------------------------------------------------------------------------
//!
inline String::SizeType
String::findFirstOf( const String& charSet, SizeType pos ) const
{
   if( pos >= size() )
   {
	   return BaseClass::npos;
   }

   ConstIterator it;

   for ( it = begin()+pos; it < end(); ++ it )
   {
      if( charSet.find(*it) != BaseClass::npos )
         break;
   }

   if( it >= end() )
   {
      return BaseClass::npos;
   }

   return it - begin();
}

//------------------------------------------------------------------------------
//!
inline String::SizeType
String::findFirstNotOf( const String& charSet, SizeType pos ) const
{
   if( pos >= size() )
   {
	   return BaseClass::npos;
   }

   ConstIterator it;

   for ( it = begin()+pos; it < end(); ++ it )
   {
      if( charSet.find(*it) == BaseClass::npos )
         break;
   }

   if( it >= end() )
   {
      return BaseClass::npos;
   }

   return it - begin();
}

//------------------------------------------------------------------------------
//!
inline String::SizeType
String::findLastOf( const String& charSet, SizeType pos ) const
{
   if( pos >= size() )
   {
      pos = size();
   }
   else
   {
      ++pos;
   }

   while( pos != 0 )
   {
      --pos;
      if( charSet.find((*this)[pos]) != BaseClass::npos )
      {
         return pos;
      }
   }

   return BaseClass::npos;
}

//------------------------------------------------------------------------------
//!
inline String::SizeType
String::findLastNotOf( const String& charSet, SizeType pos ) const
{
   if( pos >= size() )
   {
      pos = size();
   }
   else
   {
      ++pos;
   }

   while( pos != 0 )
   {
      --pos;
      if( charSet.find((*this)[pos]) == BaseClass::npos )
      {
         return pos;
      }
   }

   return BaseClass::npos;
}

//------------------------------------------------------------------------------
//!
inline String
String::getExt() const
{
   String::SizeType extStart = rfind('.');
   if( extStart != npos )
   {
      return sub(extStart + 1, npos);
   }
   else
   {
      return String();
   }
}

//------------------------------------------------------------------------------
//!
inline String&
String::replace( SizeType pos, SizeType n, const String& str )
{
   BaseClass::replace( pos, n, str );
   return *this;
}

//------------------------------------------------------------------------------
//!
inline double
String::toDouble() const
{
   return atof( cstr() );
}

//------------------------------------------------------------------------------
//!
inline float
String::toFloat() const
{
   return (float)toDouble();
}

//------------------------------------------------------------------------------
//!
inline int
String::toInt() const
{
   return atoi( cstr() );
}

//------------------------------------------------------------------------------
//!
inline char&
String::operator[]( SizeType n )
{
   return BaseClass::operator[]( n );
}

//------------------------------------------------------------------------------
//!
inline const char&
String::operator[]( SizeType n ) const
{
   return BaseClass::operator[]( n );
}

//------------------------------------------------------------------------------
//!
inline String&
String::operator=( const String& str )
{
   BaseClass::operator=( str );
   return *this;
}

//------------------------------------------------------------------------------
//!
inline String&
String::operator=( const char* str )
{
   if( str != NULL )
   {
      BaseClass::operator=( str );
   }
   else
   {
      clear();
   }
   return *this;
}

//------------------------------------------------------------------------------
//!
inline String&
String::operator=( char ch )
{
   BaseClass::operator=( ch );
   return *this;
}

//------------------------------------------------------------------------------
//!
inline String&
String::operator+=( const String& str )
{
   BaseClass::operator+=( str );
   return *this;
}

//------------------------------------------------------------------------------
//!
inline String&
String::operator+=( const char* str )
{
   BaseClass::operator+=( str );
   return *this;
}

//------------------------------------------------------------------------------
//!
inline String&
String::operator+=( char ch )
{
   BaseClass::operator+=( ch );
   return *this;
}

//------------------------------------------------------------------------------
//!
inline String&
String::append( const char* str, size_t n )
{
   BaseClass::append( str, n );
   return *this;
}

//------------------------------------------------------------------------------
//!
inline bool
String::operator==( const String& str ) const
{
   return *( static_cast<const BaseClass*>( this ) ) ==
          *( static_cast<const BaseClass*>( &str ) );
}

//------------------------------------------------------------------------------
//!
inline bool
String::operator==( const char* str ) const
{
   return *( static_cast<const BaseClass*>( this ) ) == str;
}

/*==============================================================================
  GLOBAL FUNCTIONS
==============================================================================*/

//------------------------------------------------------------------------------
//!
inline String
operator+( const String& lhs, const String& rhs )
{
   String str( lhs );
   str += rhs;
   return str;
}

//------------------------------------------------------------------------------
//!
inline String
operator+( const String& lhs, const char* rhs )
{
   String str( lhs );
   str += rhs;
   return str;
}

//------------------------------------------------------------------------------
//!
inline String
operator+( const String& lhs, char rhs )
{
   String str( lhs );
   str += rhs;
   return str;
}

//------------------------------------------------------------------------------
//!
inline bool
operator<( const String& lhs, const String& rhs )
{
   return lhs.compare( rhs ) < 0;
}

//------------------------------------------------------------------------------
//!
inline bool
operator<( const String& lhs, const char* rhs )
{
   return lhs.compare( rhs ) < 0;
}

//------------------------------------------------------------------------------
//!
inline bool
operator<( const char* lhs, const String& rhs )
{
   return rhs.compare( lhs ) > 0;
}

//------------------------------------------------------------------------------
//!
inline bool
operator<=( const String& lhs, const String& rhs )
{
   return lhs.compare( rhs ) <= 0;
}

//------------------------------------------------------------------------------
//!
inline bool
operator<=( const String& lhs, const char* rhs )
{
   return lhs.compare( rhs ) <= 0;
}

//------------------------------------------------------------------------------
//!
inline bool
operator<=( const char* lhs, const String& rhs )
{
   return rhs.compare( lhs ) >= 0;
}

//------------------------------------------------------------------------------
//!
inline bool
operator!=( const String& lhs, const String& rhs )
{
   return lhs.compare( rhs ) != 0;
}

//------------------------------------------------------------------------------
//!
inline bool
operator!=( const String& lhs, const char* rhs )
{
   return lhs.compare( rhs ) != 0;
}

//------------------------------------------------------------------------------
//!
inline bool
operator!=( const char* lhs, const String& rhs )
{
   return rhs.compare( lhs ) != 0;
}

//------------------------------------------------------------------------------
//!
inline String  toHex8( uint8_t v )
{
   return String().format( "0x%02x", v );
}

//------------------------------------------------------------------------------
//!
inline String  toHex16( uint16_t v )
{
   return String().format( "0x%04x", v );
}

//------------------------------------------------------------------------------
//!
inline String  toHex32( uint32_t v )
{
   return String().format( "0x%08x", v );
}

//------------------------------------------------------------------------------
//!
inline String  toHex64( uint64_t v )
{
   return String().format( "0x%08x_%08x", (uint32_t)((v>>32)&0xFFFFFFFF), (uint32_t)(v&0xFFFFFFFF) );
}

//------------------------------------------------------------------------------
//!
inline String  toHex( uint8_t v )
{
   return toHex8(v);
}

//------------------------------------------------------------------------------
//!
inline String  toHex( uint16_t v )
{
   return toHex16(v);
}

//------------------------------------------------------------------------------
//!
inline String  toHex( uint32_t v )
{
   return toHex32(v);
}

//------------------------------------------------------------------------------
//!
inline String  toHex( uint64_t v )
{
   return toHex64(v);
}

//-----------------------------------------------------------------------------
//!
inline String toStr( char32_t v )
{
   char tmp[5] = { 0, 0, 0, 0, 0 };
   toUTF8( v, tmp );
   return String( tmp );
}

NAMESPACE_END

#endif //BASE_STRING_H
