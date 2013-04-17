/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef BASE_CONSTSTRING_H
#define BASE_CONSTSTRING_H

#include <Base/StdDefs.h>

#include <Base/MT/Atomic.h>

NAMESPACE_BEGIN

/*==============================================================================
   CLASS RCString
==============================================================================*/

class RCString
{
public:

   /*----- static members -----*/

   BASE_DLL_API static RCString* create( const char* );

   /*----- members -----*/

   RCString(): _size(0), _count(1) { _str[0] = 0; }

   void addReference()        { ++_count; }
   void removeReference()     { if( (_size != 0) && (--_count == 0) ) remove(); }
   int  count() const         { return _count; }

   const char* cstr() const   { return _str; }
   uint size() const          { return _size; }

   void* operator new( size_t, int );
   void operator delete( void* );

private:

   /*----- members -----*/

   RCString( const char* str, int size );

   BASE_DLL_API void remove();

   /*----- data members -----*/

   int         _size;
   AtomicInt32 _count;
   char        _str[1];
};

/*==============================================================================
   CLASS ConstString
==============================================================================*/

class ConstString
{
public:

   /*----- static methods -----*/

   BASE_DLL_API static void  printAll();

   /*----- methods -----*/

   ConstString()                                     { _string = RCString::create(0); }
   ConstString( const char* str )                    { _string = RCString::create( str ); }
   ConstString( const ConstString& );

   ~ConstString()                                    { _string->removeReference(); }

   bool isNull() const                               { return size() == 0; }
   const char* cstr() const                          { return _string->cstr(); }
   uint size() const                                 { return _string->size(); }

   ConstString& operator=( const ConstString& str );
   bool operator==( const ConstString& str ) const   { return _string == str._string; }
   bool operator!=( const ConstString& str ) const   { return _string != str._string; }
   bool operator<( const ConstString& str ) const    { return _string < str._string; }
   char operator[]( uint i ) const                   { return _string->cstr()[i]; }

   // To use extremely caustiously.
   ConstString( RCString* str )                      { _string = str; _string->addReference(); }
   RCString* rcstring() const                        { return _string; }

private:

   /*----- data members -----*/

   RCString* _string;
};

//------------------------------------------------------------------------------
//! 
inline ConstString::ConstString( const ConstString& str )
{
   _string = str._string;
   _string->addReference();
}

//------------------------------------------------------------------------------
//! 
inline ConstString& 
ConstString::operator=( const ConstString& str )
{
   if( str._string != _string )
   {
      _string->removeReference();
      str._string->addReference();
      _string = str._string;
   }
   return *this;
}

NAMESPACE_END

#endif
