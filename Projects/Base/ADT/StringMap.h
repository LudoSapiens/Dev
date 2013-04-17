/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef BASE_STRINGMAP_H
#define BASE_STRINGMAP_H

#include <Base/StdDefs.h>

NAMESPACE_BEGIN

/*==============================================================================
  CLASS StringMap
==============================================================================*/

//! Static map for key string and int value.

class BASE_DLL_API StringMap
{

public:

   /*----- types and enumerations ----*/

   enum { INVALID = -1 };

   /*----- methods -----*/

   StringMap( const char* startStr, ... );
   ~StringMap();

   int operator[]( const char* str );
   const char* operator[]( int value );

   int findInPrefix( const char* str );
   
private:

   /*----- classes -----*/

   struct Element;
   struct Cmp;

   /*----- data members -----*/

   int      _size;
   Element* _array;
};

NAMESPACE_END

#endif


