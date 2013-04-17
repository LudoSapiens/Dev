/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Base/ADT/StringMap.h>

#include <algorithm>
#include <cstdarg>
#include <cstring>

NAMESPACE_BEGIN

/*==============================================================================
  CLASS StringMap
==============================================================================*/

//------------------------------------------------------------------------------
//!
struct StringMap::Element 
{
   const char* _str;
   int         _value;
};

//------------------------------------------------------------------------------
//!
struct StringMap::Cmp
{
   bool operator()( const Element& e1, const Element& e2 ) const
   {
      return strcmp( e1._str, e2._str ) < 0;
   }
};

//------------------------------------------------------------------------------
//!
StringMap::StringMap
( const char* str, ... )
   : _array( 0 )
{
   if( str == 0 || str[0] == 0 )
   {
      return;
   }

   // Count number of elements
   va_list vars;
   va_start( vars, str );

   const char* cstr = str;
   int nbStr        = 0;

   va_arg( vars, int );
   while( 1 )
   {
      ++nbStr;
      cstr = va_arg( vars, const char* );
      if( cstr == 0 || cstr[0] == 0 )
      {
         break;
      }
      va_arg( vars, int );
   }
   va_end( vars );

   // Create array.
   _array = new Element[ nbStr ];
   _size  = nbStr;

   // Fill the array.
   cstr = str;
   va_start( vars, str );
   int cvalue = va_arg( vars, int );
   while( 1 )
   {
      // Add element.
      _array[ --nbStr ]._str = cstr;
      _array[ nbStr ]._value = cvalue;

      // Get next element.
      cstr = va_arg( vars, const char* );
      if( cstr == 0 || cstr[0] == 0 )
      {
         break;
      }
      cvalue = va_arg( vars, int );
   }
   va_end( vars );

   // Sort the array.
   std::sort( _array, _array + _size, Cmp() );
}

//------------------------------------------------------------------------------
//!
StringMap::~StringMap()
{
   delete[] _array;
}

//------------------------------------------------------------------------------
//!
int
StringMap::operator[]
( const char* str )
{
   // Binary search.
   int start  = 0;
   int end    = _size;

   while( 1 )
   {
      int middle = start + ( ( end - start ) >> 1 );
      
      if( middle == start )
      {
         if( strcmp( str, _array[ middle ]._str ) == 0 )
         {
            return _array[ middle ]._value;
         }
         return INVALID;
      }

      if( strcmp( str, _array[ middle ]._str ) < 0 )
      {
         end = middle;
      }
      else
      {
         start = middle;
      }
   }   
   return INVALID;
}

//------------------------------------------------------------------------------
//!
const char*
StringMap::operator[]
( int value )
{
   // Linear search.
   int i;

   for( i=0; i<_size; ++i )
   {
      if( _array[i]._value == value )
      {
         return _array[i]._str;
      }
   }

   return NULL;
}

//------------------------------------------------------------------------------
//!
int
StringMap::findInPrefix
( const char* str )
{
   // Binary search.
   int start  = 0;
   int end    = _size;

   while( 1 )
   {
      int middle = start + ( ( end - start ) >> 1 );
      
      if( middle == start )
      {
         if( strncmp( str, _array[ middle ]._str, strlen(_array[ middle ]._str) ) == 0 )
         {
            return _array[ middle ]._value;
         }
         return INVALID;
      }

      if( strcmp( str, _array[ middle ]._str ) < 0 )
      {
         end = middle;
      }
      else
      {
         start = middle;
      }
   }   
   return INVALID;
}

NAMESPACE_END
