/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef BASE_VALUE_TRIGGER_H
#define BASE_VALUE_TRIGGER_H

#include <Base/StdDefs.h>

#include <Base/MT/Atomic.h>

NAMESPACE_BEGIN

/*==============================================================================
  CLASS ValueTrigger
==============================================================================*/
//! A structure similar to a semaphore, but with the reverse behavior: a wait
//! call will hold until the count becomes the desired value.
class ValueTrigger
{
public:

   /*----- methods -----*/

   BASE_DLL_API ValueTrigger( int32_t initialValue = 0, int32_t wantedValue = 0 );
   BASE_DLL_API ~ValueTrigger();


   inline int32_t  currentValue() const { return _current; }
   BASE_DLL_API void  currentValue( int32_t v );

   inline int32_t  wantedValue() const { return _wanted; }

   inline operator int32_t() { return _current; }

   BASE_DLL_API int32_t  operator++();
   BASE_DLL_API int32_t  operator--();

   BASE_DLL_API int32_t  operator+=( int32_t v );
   BASE_DLL_API int32_t  operator-=( int32_t v );

   BASE_DLL_API int32_t  operator&=( int32_t m );
   BASE_DLL_API int32_t  operator|=( int32_t m );
   BASE_DLL_API int32_t  operator^=( int32_t m );

   inline int32_t  operator=( int32_t v ) { currentValue(v); return v; }

   BASE_DLL_API void  wait();

protected:

   struct Impl;

   /*----- data members -----*/

   int32_t  _current;  //!< The current count value.
   int32_t  _wanted;   //!< The current count value.
   Impl*    _impl;     //!< The platform-specific private implementation.

   /*----- methods -----*/

   BASE_DLL_API void checkTrigger( int32_t oldV, int32_t newV );
   void  post();

private:
}; //class ValueTrigger


NAMESPACE_END

#endif //BASE_VALUE_TRIGGER_H
