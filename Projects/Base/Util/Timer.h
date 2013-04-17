/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef BASE_TIMER_H
#define BASE_TIMER_H

#include <Base/StdDefs.h>

#if defined(_WIN32)
#include <Base/Util/windows.h>
#else
#include <sys/time.h>
#include <cmath>
#endif

NAMESPACE_BEGIN

/*==============================================================================
  CLASS Timer
==============================================================================*/

//!

class Timer
{

public:

   /*----- methods -----*/

   inline Timer();
   inline ~Timer();

   inline double restart();
   inline double elapsed() const;

protected:

   /*----- data members -----*/

#if defined(_WIN32)
   LARGE_INTEGER _frequency;
   LARGE_INTEGER _start;
#else
   timeval _start;
#endif
};


/*==============================================================================
  CLASS StopTimer
==============================================================================*/

//!

class StopTimer:
   public Timer
{

public:

   /*----- methods -----*/

   inline StopTimer();

   inline double restart();
   inline double elapsed() const;
   inline double pause();
   inline double resume();

   inline bool paused() const;

private:

   /*----- data members -----*/

#if defined(_WIN32)
   enum
   {
      INVALID_QUAD_PART = (LONGLONG)-1
   };
   LARGE_INTEGER _pause;
#else
   enum
   {
      INVALID_USEC = 0xFFFFF // Smallest bitmask that can represent 1000000.
   };
   timeval _pause;
#endif
};


#if defined(_WIN32)
//------------------------------------------------------------------------------
//!
inline
Timer::Timer()
{
   QueryPerformanceFrequency( &_frequency );
   QueryPerformanceCounter( &_start );
}

//------------------------------------------------------------------------------
//!
inline
Timer::~Timer()
{}

//------------------------------------------------------------------------------
//!
inline double
Timer::restart()
{
   LARGE_INTEGER prev = _start;
   QueryPerformanceCounter( &_start );
   return double( _start.QuadPart - prev.QuadPart ) /
          double( _frequency.QuadPart );
}

//------------------------------------------------------------------------------
//!
inline double
Timer::elapsed() const
{
   LARGE_INTEGER time;
   QueryPerformanceCounter( &time );
   return double( time.QuadPart - _start.QuadPart ) /
          double( _frequency.QuadPart );
}

//------------------------------------------------------------------------------
//!
inline
StopTimer::StopTimer()
{
   _pause.QuadPart = INVALID_QUAD_PART; // Not paused.
}

//------------------------------------------------------------------------------
//!
inline double
StopTimer::restart()
{
   _pause.QuadPart = INVALID_QUAD_PART; // Unpause.
   return Timer::restart();
}

//------------------------------------------------------------------------------
//!
inline double
StopTimer::elapsed() const
{
   if( !paused() )
   {
      return Timer::elapsed();
   }
   else
   {
      return double( _pause.QuadPart - _start.QuadPart ) /
             double( _frequency.QuadPart );
   }
}

//------------------------------------------------------------------------------
//!
inline double
StopTimer::pause()
{
   QueryPerformanceCounter( &_pause );
   return double( _pause.QuadPart - _start.QuadPart ) /
          double( _frequency.QuadPart );
}

//------------------------------------------------------------------------------
//!
inline double
StopTimer::resume()
{
   if( paused() )
   {
      LARGE_INTEGER time;
      QueryPerformanceCounter( &time );
      // Tweak _start in order to skip the delta time that was paused.
      _start.QuadPart += (time.QuadPart - _pause.QuadPart);
      _pause.QuadPart = INVALID_QUAD_PART;
      return double( time.QuadPart - _start.QuadPart ) /
             double( _frequency.QuadPart );
    }
    else
    {
       return elapsed();
    }
}

//------------------------------------------------------------------------------
//!
inline bool
StopTimer::paused() const
{
   return _pause.QuadPart != INVALID_QUAD_PART;
}

#else

//------------------------------------------------------------------------------
//!
inline
Timer::Timer()
{
   gettimeofday( &_start, 0 );
}

//------------------------------------------------------------------------------
//!
inline
Timer::~Timer()
{}

//------------------------------------------------------------------------------
//!
inline double
Timer::restart()
{
   timeval prev = _start;
   gettimeofday( &_start, 0 );
   return double( _start.tv_sec  - prev.tv_sec  ) +
          double( _start.tv_usec - prev.tv_usec ) / 1000000.0;
}

//------------------------------------------------------------------------------
//!
inline double
Timer::elapsed() const
{
   timeval time;
   gettimeofday( &time, 0 );
   return double( time.tv_sec  - _start.tv_sec  ) +
          double( time.tv_usec - _start.tv_usec ) / 1000000.0;
}

//------------------------------------------------------------------------------
//!
inline
StopTimer::StopTimer()
{
   _pause.tv_usec = INVALID_USEC; // Not paused.
}

//------------------------------------------------------------------------------
//!
inline double
StopTimer::restart()
{
   _pause.tv_usec = INVALID_USEC; // Unpause.
   return Timer::restart();
}

//------------------------------------------------------------------------------
//!
inline double
StopTimer::elapsed() const
{
   if( !paused() )
   {
      return Timer::elapsed();
   }
   else
   {
      return double( _pause.tv_sec  - _start.tv_sec  ) +
             double( _pause.tv_usec - _start.tv_usec ) / 1000000.0;
   }
}

//------------------------------------------------------------------------------
//!
inline double
StopTimer::pause()
{
   gettimeofday( &_pause, 0 );
   return double( _pause.tv_sec  - _start.tv_sec  ) +
          double( _pause.tv_usec - _start.tv_usec ) / 1000000.0;
}

//------------------------------------------------------------------------------
//!
inline double
StopTimer::resume()
{
   if( paused() )
   {
      timeval time;
      gettimeofday( &time, 0 );
      // Tweak _start in order to skip the delta time that was paused.
      double delta = double( time.tv_sec  - _pause.tv_sec  ) +
                     double( time.tv_usec - _pause.tv_usec ) / 1000000.0;
      double iPart,fPart;
      fPart = modf( delta, &iPart );
      _start.tv_sec  += (int)iPart;
      _start.tv_usec += (int)(fPart * 1000000.0);
      _pause.tv_usec = INVALID_USEC;
      return double( time.tv_sec  - _start.tv_sec  ) +
             double( time.tv_usec - _start.tv_usec ) / 1000000.0;
    }
    else
    {
       return elapsed();
    }
}

//------------------------------------------------------------------------------
//!
inline bool
StopTimer::paused() const
{
   return _pause.tv_usec != INVALID_USEC;
}

#endif


NAMESPACE_END

#endif //BASE_TIMER_H
