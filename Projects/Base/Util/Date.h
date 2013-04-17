/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef BASE_DATE_H
#define BASE_DATE_H

#include <Base/StdDefs.h>

#include <Base/ADT/String.h>

#include <ctime>

NAMESPACE_BEGIN

/*==============================================================================
  CLASS Date
==============================================================================*/
class Date
{
public:

   /*----- types and enums -----*/
   enum Month
   {
      MONTH_JAN,
      MONTH_FEB,
      MONTH_MAR,
      MONTH_APR,
      MONTH_MAY,
      MONTH_JUN,
      MONTH_JUL,
      MONTH_AUG,
      MONTH_SEP,
      MONTH_OCT,
      MONTH_NOV,
      MONTH_DEC
   };

   enum Weekday
   {
      WEEKDAY_SUN,
      WEEKDAY_MON,
      WEEKDAY_TUE,
      WEEKDAY_WED,
      WEEKDAY_THU,
      WEEKDAY_FRI,
      WEEKDAY_SAT
   };

   /*----- methods -----*/

   Date() {}
   Date( int y, int m, int d, int hour = 0, int min = 0, int sec = 0, int nsec = 0 ) { set(y, m, d, hour, min, sec, nsec); }
   Date( time_t t, int nsec = 0 ) { set(t, nsec); }

   inline operator time_t() const { return _time; }

   inline int   nsec() const { return _nsec; }
   inline void  nsec( int n ) { _nsec = n; }

   BASE_DLL_API Date&  set( int y, int m, int d, int hour = 0, int min = 0, int sec = 0, int nsec = 0 );
   BASE_DLL_API Date&  set( time_t t, int nsec = 0 );

   BASE_DLL_API Date&  get( int& y, int& m, int& d );
   BASE_DLL_API Date&  get( int& y, int& m, int& d, int& wd );
   BASE_DLL_API Date&  get( int& y, int& m, int& d, int& hour, int& min, int& sec );


   BASE_DLL_API String  toStr() const;
   BASE_DLL_API String  toISO8601() const;
   BASE_DLL_API const char* fromISO8601( const char* str );

   static BASE_DLL_API Date  now();

   BASE_DLL_API double  operator-( Date& rhs );

protected:

   /*----- data members -----*/

   time_t    _time;  //!< The number of seconds since Epoch.
   int       _nsec;  //!< The number of nanoseconds to add to _time.
   //tm        _tm;    //!< Should always be in sync with _time.

}; //class Date


NAMESPACE_END

#endif //BASE_DATE_H
