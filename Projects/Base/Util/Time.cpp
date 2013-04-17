/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Base/Util/Time.h>

#include <Base/Dbg/Defs.h>
#include <Base/IO/TextStream.h>

#include <cmath>
#include <cstdlib>

USING_NAMESPACE

/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN

//static double sMilliSecondFactor = 1.0 / 1000.0;
static double sMicroSecondFactor = 1.0 / 1000000.0;

UNNAMESPACE_END

//------------------------------------------------------------------------------
//!
Time&
Time::set( int h, int m, int s )
{
   int tmp;
   tmp  = h;
   tmp *= 60;  // ... minutes in an hour.
   tmp += m;
   tmp *= 60;  // ... seconds in a minute.
   tmp += s;
   _time = tmp;
   return *this;
}

//------------------------------------------------------------------------------
//!
Time&
Time::set( int h, int m, int s, int ms, int us )
{
   int tmp;
   // Splitting code into 2 parts in order to:
   // - avoid overflowing an 'int'
   // while
   // - reducing the number of int -> double conversions.
   tmp  = ms;
   tmp *= 1000;  // ... microsecond in a millisecond.
   tmp += us;
   _time = tmp * sMicroSecondFactor;
   tmp  = h;
   tmp *= 60;    // ... minutes in an hour.
   tmp += m;
   tmp *= 60;    // ... seconds in a minute.
   tmp += s;
   _time += tmp;
   return *this;
}

//------------------------------------------------------------------------------
//!
Time&
Time::set( const String& str )
{
   int h  = 0;
   int m  = 0;
   int s  = 0;
   int ms = 0;
   int us = 0;
   // Eventually accept more generic formats?
   switch( sscanf(str.cstr(), "%d:%d:%d:%d:%d", &h, &m, &s, &ms, &us ) )
   {
      case 0:
         StdErr << "Could not parse time out of '" << str << "'" << nl;
         break;
      case 1:
      case 2:
      case 3:
         set( h, m, s );
         break;
      case 4:
      case 5:
         set( h, m, s, ms, us );
         break;
      default:
         StdErr << "Error in sscanf" << nl;
         CHECK( false );
         break;
   }
   return *this;
}

//------------------------------------------------------------------------------
//!
const Time&
Time::get( int& h, int& m ) const
{
   div_t  tmpDiv;

   tmpDiv = div( (int)_time, 3600 );
   h = tmpDiv.quot;
   m = tmpDiv.rem / 60;

   return *this;
}

//------------------------------------------------------------------------------
//!
const Time&
Time::get( int& h, int& m, int& s ) const
{
   div_t  tmpDiv;

   tmpDiv = div( (int)_time, 3600 );
   h = tmpDiv.quot;
   m = tmpDiv.rem;
   tmpDiv = div( m, 60 );
   m = tmpDiv.quot;
   s = tmpDiv.rem;

   return *this;
}

//------------------------------------------------------------------------------
//!
const Time&
Time::get( int& h, int& m, int& s, int& ms, int& us ) const
{
   double h_m_s, ms_us;
   ms_us = modf( _time, &h_m_s );
   ms_us *= 1000000.0;
   ms_us += 0.5;

   div_t  tmpDiv;

   tmpDiv = div( (int)h_m_s, 3600 );
   h = tmpDiv.quot;
   m = tmpDiv.rem;
   tmpDiv = div( m, 60 );
   m = tmpDiv.quot;
   s = tmpDiv.rem;

   tmpDiv = div( (int)ms_us, 1000 );
   ms = tmpDiv.quot;
   us = tmpDiv.rem;

   return *this;
}

//------------------------------------------------------------------------------
//!
double
Time::asHours() const
{
   return _time / 3600.0;
}

//------------------------------------------------------------------------------
//!
double
Time::asMinutes() const
{
   return _time / 60.0;
}

//------------------------------------------------------------------------------
//!
//double
//Time::asSeconds() const
//{
//   return _time;
//}

//------------------------------------------------------------------------------
//!
double
Time::asMilliseconds() const
{
   return _time * 1000.0;
}

//------------------------------------------------------------------------------
//!
double
Time::asMicroseconds() const
{
   return _time * 1000000.0;
}

//------------------------------------------------------------------------------
//!
String
Time::toStr() const
{
   int h, m, s, ms, us;
   get( h, m, s, ms, us );
   return String().format("%02d:%02d:%02d.%03d.%03d", h, m, s, ms, us);
}
