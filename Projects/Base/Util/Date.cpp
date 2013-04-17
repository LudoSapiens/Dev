/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Base/Util/Date.h>

#include <Base/IO/TextStream.h>

#include <cstdio>

USING_NAMESPACE

/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN

#if _MSC_VER
char* asctime_r( const tm* tm, char* buf )
{
   char* tmp = asctime( tm );
   memcpy( buf, tmp, 26*sizeof(char) );
   return tmp;
}

tm* localtime_r( const time_t* clock, struct tm *result )
{
   tm* tmp = localtime( clock );
   *result = *tmp;
   return tmp;
}

tm* gmtime_r( const time_t* clock, struct tm *result )
{
   tm* tmp = gmtime( clock );
   *result = *tmp;
   return tmp;
}

#endif

UNNAMESPACE_END


//------------------------------------------------------------------------------
//!
Date&
Date::set( time_t t, int nsec )
{
   _time = t;
   _nsec = nsec;
   // TODO: Support more timezones than localtime.
   //localtime_r( &_time, &_tm );
   return *this;
}

//------------------------------------------------------------------------------
//!
Date&
Date::set( int y, int m, int d, int hour, int min, int sec, int nsec )
{
   tm _tm;
   _tm.tm_year  = y - 1900;  // years since 1900.
   _tm.tm_mon   = m;         // months since January, range [0, 11].
   _tm.tm_mday  = d;         // day of the month, range [1-31].
   _tm.tm_hour  = hour;      // hours since midnight, [0-23].
   _tm.tm_min   = min;       // minutes after the hour, [0-59].
   _tm.tm_sec   = sec;       // seconds after the minute, range [0-61] (to accomodate for leap seconds).
   _tm.tm_isdst = -1;
   _tm.tm_wday  = 0;
   _tm.tm_yday  = 0;

   _time = mktime( &_tm );
   _nsec = nsec;

   // Reconvert _tm to clean inconsistencies.
   // TODO: Support more timezones than localtime.
   //localtime_r( &_time, &_tm );

   return *this;
}

//------------------------------------------------------------------------------
//!
Date&
Date::get( int& y, int& m, int& d )
{
   tm _tm;
   localtime_r( &_time, &_tm );

   y  = _tm.tm_year + 1900;
   m  = _tm.tm_mon;
   d  = _tm.tm_mday;

   return *this;
}

//------------------------------------------------------------------------------
//!
Date&
Date::get( int& y, int& m, int& d, int& wd )
{
   tm _tm;
   localtime_r( &_time, &_tm );

   y  = _tm.tm_year + 1900;
   m  = _tm.tm_mon;
   d  = _tm.tm_mday;
   wd = _tm.tm_wday;

   return *this;
}

//------------------------------------------------------------------------------
//!
Date&
Date::get( int& y, int& m, int& d, int& hour, int& min, int& sec )
{
   tm _tm;
   localtime_r( &_time, &_tm );

   y = _tm.tm_year + 1900;
   m = _tm.tm_mon;
   d = _tm.tm_mday;

   hour = _tm.tm_hour;
   min  = _tm.tm_min;
   sec  = _tm.tm_sec;

   return *this;
}

//------------------------------------------------------------------------------
//!
String
Date::toStr() const
{
   char tmp[26];
   // TODO: Support more timezones than localtime.
   tm _tm;
   localtime_r( &_time, &_tm );
   asctime_r( &_tm, tmp );
   // Need to remove trailing whitespaces that asctime_r puts in.
   return String(tmp).eatWhites();
}

//------------------------------------------------------------------------------
//!
String
Date::toISO8601() const
{
   char tmp[256];
   tm _tm;
   localtime_r( &_time, &_tm );
   strftime( tmp, sizeof(tmp), "%Y-%m-%dT%H:%M:%S", &_tm );
   return String(tmp).eatWhites();
}

//------------------------------------------------------------------------------
//!
const char*
Date::fromISO8601( const char* str )
{
#if PLAT_POSIX
   tm _tm;
   const char* tmp = strptime( str, "%Y-%m-%dT%H:%M:%S", &_tm );
   _time = mktime( &_tm );
   _nsec = 0;
   return tmp;
#else
   int yr, mo, dy;
   int hh, mm, ss;
   int pos;
   if( sscanf( str, "%d-%d-%dT%d:%d:%d%n", &yr, &mo, &dy, &hh, &mm, &ss, &pos ) == 6 )
   {
      set( yr, mo-1, dy, hh, mm, ss );
      return str + pos;
   }
   else
   {
      return str;
   }
#endif
}

//------------------------------------------------------------------------------
//!
Date
Date::now()
{
   return Date( time(NULL) );
}

//------------------------------------------------------------------------------
//!
double
Date::operator-( Date& rhs )
{
   // Earlier is rhs.
   return difftime( _time, rhs._time );
}
