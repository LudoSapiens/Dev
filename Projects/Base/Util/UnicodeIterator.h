/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef BASE_UNICODE_ITERATOR_H
#define BASE_UNICODE_ITERATOR_H

#include <Base/StdDefs.h>

#include <Base/Util/Unicode.h>

NAMESPACE_BEGIN

/*==============================================================================
  CLASS UTF8Iterator
==============================================================================*/
class UTF8Iterator
{
public:

   /*----- methods -----*/

   inline UTF8Iterator( const char* str );

   inline bool  operator()() const;

   BASE_DLL_API char32_t  operator*() const;

   inline void  next();
   inline void  prev();

   inline UTF8Iterator&  operator++();
   inline UTF8Iterator&  operator--();

   inline UTF8Iterator&  operator+=( int nCodes );
   inline UTF8Iterator&  operator-=( int nCodes );

   inline UTF8Iterator  operator+( int nCodes ) const;
   inline UTF8Iterator  operator-( int nCodes ) const;

   inline bool  operator< ( const UTF8Iterator& it ) const { return _cur <  it._cur; }
   inline bool  operator<=( const UTF8Iterator& it ) const { return _cur <= it._cur; }
   inline bool  operator==( const UTF8Iterator& it ) const { return _cur == it._cur; }
   inline bool  operator!=( const UTF8Iterator& it ) const { return _cur != it._cur; }
   inline bool  operator> ( const UTF8Iterator& it ) const { return _cur >  it._cur; }
   inline bool  operator>=( const UTF8Iterator& it ) const { return _cur >= it._cur; }

   inline bool  operator< ( const char* str ) const { return _cur <  str; }
   inline bool  operator<=( const char* str ) const { return _cur <= str; }
   inline bool  operator==( const char* str ) const { return _cur == str; }
   inline bool  operator!=( const char* str ) const { return _cur != str; }
   inline bool  operator> ( const char* str ) const { return _cur >  str; }
   inline bool  operator>=( const char* str ) const { return _cur >= str; }

   inline int  operator-( const UTF8Iterator& it ) const;

   inline const char*  cur() const;
   inline void  cur( const char* str );

   inline UTF8Iterator&  operator=( const char* str );

protected:

   /*----- data members -----*/

   const char* _cur;

   /*----- methods -----*/

   static inline void next( const char*& str );

private:
}; //class UTF8Iterator

//-----------------------------------------------------------------------------
//!
inline
UTF8Iterator::UTF8Iterator( const char* str ):
   _cur( str )
{
}

//-----------------------------------------------------------------------------
//!
inline bool
UTF8Iterator::operator()() const
{
   return (*_cur != '\0');
}

//-----------------------------------------------------------------------------
//!
void
UTF8Iterator::next()
{
   ++_cur;
   while( isUTF8Continuation(*_cur) )
   {
      ++_cur;
   }
}

//-----------------------------------------------------------------------------
//!
void
UTF8Iterator::prev()
{
   --_cur;
   while( isUTF8Continuation(*_cur) )
   {
      --_cur;
   }
}

//-----------------------------------------------------------------------------
//!
inline void
UTF8Iterator::next( const char*& str )
{
   ++str;
   while( isUTF8Continuation(*str) )
   {
      ++str;
   }
}

//-----------------------------------------------------------------------------
//!
UTF8Iterator&
UTF8Iterator::operator++()
{
   next();
   return *this;
}

//-----------------------------------------------------------------------------
//!
UTF8Iterator&
UTF8Iterator::operator--()
{
   prev();
   return *this;
}

//-----------------------------------------------------------------------------
//!
UTF8Iterator&
UTF8Iterator::operator+=( int nCodes )
{
   if( nCodes >= 0 )
   {
      while( nCodes-- )
      {
         next();
      }
   }
   else
   {
      while( nCodes++ )
      {
         prev();
      }
   }
   return *this;
}

//-----------------------------------------------------------------------------
//!
UTF8Iterator&
UTF8Iterator::operator-=( int nCodes )
{
   if( nCodes >= 0 )
   {
      while( nCodes-- )
      {
         prev();
      }
   }
   else
   {
      while( nCodes++ )
      {
         next();
      }
   }
   return *this;
}

//-----------------------------------------------------------------------------
//!
UTF8Iterator
UTF8Iterator::operator+( int nCodes ) const
{
   UTF8Iterator tmp( *this );
   tmp += nCodes;
   return tmp;
}

//-----------------------------------------------------------------------------
//!
UTF8Iterator
UTF8Iterator::operator-( int nCodes ) const
{
   UTF8Iterator tmp( *this );
   tmp -= nCodes;
   return tmp;
}

//-----------------------------------------------------------------------------
//!
int
UTF8Iterator::operator-( const UTF8Iterator& it ) const
{
   int n = 0;
   if( it <= (*this) )
   {
      const char* s = it._cur;
      const char* e = _cur;
      while( s < e )
      {
         next( s );
         ++n;
      }
   }
   else
   {
      const char* s = _cur;
      const char* e = it._cur;
      while( s < e )
      {
         next( s );
         --n;
      }
   }
   return n;
}

//-----------------------------------------------------------------------------
//!
inline const char*
UTF8Iterator::cur() const
{
   return _cur;
}

//-----------------------------------------------------------------------------
//!
inline void
UTF8Iterator::cur( const char* str )
{
   _cur = str;
}

//-----------------------------------------------------------------------------
//!
inline UTF8Iterator&
UTF8Iterator::operator=( const char* str )
{
   _cur = str;
   return *this;
}

NAMESPACE_END

#endif //BASE_UNICODE_ITERATOR_H
