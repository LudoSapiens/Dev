/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef BASE_TIME_H
#define BASE_TIME_H

#include <Base/StdDefs.h>

#include <Base/ADT/String.h>

NAMESPACE_BEGIN

/*==============================================================================
  CLASS Time
==============================================================================*/
class Time
{
public:

   /*----- static methods -----*/

   static BASE_DLL_API Time  now();

   /*----- methods -----*/

   Time() {}
   Time( double seconds ): _time(seconds) {}
   Time( int h, int m, int s = 0 ) { set(h, m, s); }
   Time( int h, int m, int s, int ms, int us = 0 ) { set(h, m, s, ms, us); }

   BASE_DLL_API Time&  set( int h, int m, int s = 0 );
   BASE_DLL_API Time&  set( int h, int m, int s, int ms, int us = 0 );
   BASE_DLL_API Time&  set( const String& time );

   BASE_DLL_API const Time&  get( int& h, int& m ) const;
   BASE_DLL_API const Time&  get( int& h, int& m, int& s ) const;
   BASE_DLL_API const Time&  get( int& h, int& m, int& s, int& ms, int& us ) const;

   Time&  add( double seconds ) { _time += seconds; return *this; }

   //BASE_DLL_API int  hour() const;
   //BASE_DLL_API int  minute() const;
   //BASE_DLL_API int  second() const;
   //BASE_DLL_API int  millisecond() const;
   //BASE_DLL_API int  microsecond() const;

   BASE_DLL_API double  asHours() const;
   BASE_DLL_API double  asMinutes() const;
   inline       double  asSeconds() const { return _time; }
   BASE_DLL_API double  asMilliseconds() const;
   BASE_DLL_API double  asMicroseconds() const;

   BASE_DLL_API String  toStr() const;

protected:

   /*----- data members -----*/

   double  _time; //!< Time in seconds.

}; //class Time


NAMESPACE_END

#endif //BASE_TIME_H
