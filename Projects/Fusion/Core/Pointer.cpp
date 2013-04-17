/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Fusion/Core/Pointer.h>

#include <Fusion/Core/Core.h>
#include <Fusion/Core/Event.h>

#include <Base/ADT/DEQueue.h>

/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN


/*==============================================================================
  CLASS DataIntegrator
==============================================================================*/
class DataIntegrator
{
public:

   /*----- methods -----*/

   DataIntegrator(): _dir(0.0f) {}

   void  add( float val, double curTime, double timeWindow );

   void  clear();
   void  clean( double curTime, double timeWindow );

   float  computeDerivative( double curTime, double timeWindow );

protected:

   /*----- data types -----*/

   struct Sample
   {
      float   _val;
      double  _time;
      Sample() {}
      Sample( float val, double time ):
         _val( val ), _time( time ) {}
   };

   /*----- data members -----*/

   DEQueue<Sample>  _samples;
   float            _dir;

   /*----- methods -----*/

private:
}; //class DataIntegrator

//------------------------------------------------------------------------------
//!
void
DataIntegrator::add( float val, double curTime, double timeWindow )
{
   //StdErr << "add(" << val << "," << curTime << "," << timeWindow << ")" << nl;
   // Compute direction.
   float dir = _samples.empty() ? 0.0f : val-_samples.back()._val;

   //StdErr << "dir=" << dir << " _dir=" << _dir << nl;

   // Clear history when we change direction.
   if( _dir*dir < 0.0f )  clear();
   // Else keep memory to a minimum.
   else                   clean( curTime, timeWindow );

   // Register new direction.
   _dir = (dir != 0.0f) ? dir : _dir;

   // Store the new sample.
   _samples.pushBack( Sample(val, curTime) );

   //StdErr << "add() ends with " << _samples.size() << " samples and deriv=" << computeDerivative( curTime, timeWindow ) << nl;
}

//------------------------------------------------------------------------------
//!
void
DataIntegrator::clear()
{
   //StdErr << "CLEAR" << nl;
   _samples.clear();
   _dir = 0.0f;
}

//------------------------------------------------------------------------------
//!
void
DataIntegrator::clean( double curTime, double timeWindow )
{
   //StdErr << "CLEAN " << _samples.size();
   double oldestValidTime = curTime - timeWindow;
   while( !_samples.empty() && (_samples.front()._time < oldestValidTime) )
   {
      _samples.popFront();
   }
   //StdErr << " --> " << _samples.size() << nl;
}

//------------------------------------------------------------------------------
//!
float
DataIntegrator::computeDerivative( double curTime, double timeWindow )
{
   // Need to clean, as maybe no event happened for a long time.
   clean( curTime, timeWindow );
   //StdErr << nl;
   //StdErr << "compDeriv(" << curTime << "," << timeWindow << ")" << nl;

   uint n = uint(_samples.size());
#if 0
   for( uint i = 1; i < n; ++i )
   {
      const Sample& s = _samples.peek(i);
      StdErr << "Sample " << i << ": " << s._val << " @ " << s._time << nl;
   }
#endif
   if( n < 2 )  return 0.0f;

   float deltaTime = float(curTime - _samples.front()._time);

   //StdErr << " deltaTime=" << deltaTime;

   // Ignore very quick taps.
   if( CGM::equal( deltaTime, 0.0f, 1.0f/1024.0f ) )  return 0.0f;

   float vMin = _samples.front()._val;
   float vMax = vMin;

   for( uint i = 1; i < n; ++i )
   {
      float val = _samples.peek(i)._val;
      vMin = CGM::min( vMin, val );
      vMax = CGM::max( vMax, val );
   }

   //StdErr << " min=" << vMin << " max=" << vMax;
   //StdErr << " res=" << (float)(vMax - vMin) * CGM::sign(_dir) / deltaTime << "=";

   // Return the average variation over the time.
   return (vMax - vMin) * CGM::sign(_dir) / deltaTime;
}


UNNAMESPACE_END

/*==============================================================================
  CLASS Pointer::PrivateData
==============================================================================*/
class Pointer::PrivateData:
   public RCObject
{
public:

   /*----- methods -----*/

   PrivateData( double timeWindow = 0.3 );

   inline void  clear();
   inline void  curEvent( double curTime, const Event& event );
   inline Vec2f computeDerivatives( double curTime );

   /*----- members -----*/

   Event   _curEvent;     //!< The lastest event registered for this pointer.
   Event   _lastEvent;    //!< The previous event registered for this pointer.
   Event   _lastPress;    //!< The previous press event registered for this pointer.
   Event   _lastRelease;  //!< The previous release event registered for this pointer.

   float           _fartestDistSq;  //!< The largest distance from the last press event, squared.
   double          _timeWindow;  //!< Time window size for the data integrators.
   DataIntegrator  _integX;  //!< Data integration for X.
   DataIntegrator  _integY;  //!< Data integration for Y.

}; //class Pointer::PrivateData

//------------------------------------------------------------------------------
//!
Pointer::PrivateData::PrivateData( double timeWindow ):
   _timeWindow( timeWindow )
{
}

//------------------------------------------------------------------------------
//!
inline void
Pointer::PrivateData::clear()
{
   _curEvent      = Event::Invalid();
   _lastEvent     = Event::Invalid();
   _lastPress     = Event::Invalid();
   _lastRelease   = Event::Invalid();
   _fartestDistSq = 0.0f;
}

//------------------------------------------------------------------------------
//!
inline void
Pointer::PrivateData::curEvent( double curTime, const Event& event )
{
   _lastEvent = _curEvent;
   _curEvent  = event;
   switch( event.type() )
   {
      case Event::POINTER_PRESS:
         _lastPress = event;
         _fartestDistSq = 0.0f;
         _integX.clear();
         _integY.clear();
         break;
      case Event::POINTER_RELEASE:
         _lastRelease = event;
         break;
      case Event::POINTER_MOVE:
         _fartestDistSq = CGM::max( _fartestDistSq, CGM::sqrLength( event.position() - _lastPress.position() ) );
         break;
      default:
         break;
   }
   _integX.add( event.position().x, curTime, _timeWindow );
   _integY.add( event.position().y, curTime, _timeWindow );
}

//------------------------------------------------------------------------------
//!
inline Vec2f
Pointer::PrivateData::computeDerivatives( double curTime )
{
   return Vec2f(
      _integX.computeDerivative( curTime, _timeWindow ),
      _integY.computeDerivative( curTime, _timeWindow )
   );
}


/*==============================================================================
  CLASS Pointer
==============================================================================*/


//------------------------------------------------------------------------------
//!
Pointer::Pointer():
   _fields( 0x00 )
{
   _private = new PrivateData(); // Use a static memory pool?
}

//------------------------------------------------------------------------------
//!
Pointer::Pointer( const Pointer& p )
{
   *this = p;
}

//------------------------------------------------------------------------------
//!
Pointer::~Pointer()
{
}

//------------------------------------------------------------------------------
//!
Pointer&
Pointer::operator=( const Pointer& p )
{
   _id     = p._id;
   _fields = p._fields;
   _state  = p._state;
   _icon   = p._icon;

   _grabbingWidget  = p._grabbingWidget;
   _hoveringWidget  = p._hoveringWidget;
   _followingWidget = p._followingWidget;
   _private         = p._private;

   return *this;
}

//------------------------------------------------------------------------------
//!
void
Pointer::init( const uint id, const bool p )
{
   _id     = id;
   _fields = 0x00;
   persistent( p );
   clear();
}

//------------------------------------------------------------------------------
//!
void
Pointer::clear()
{
   _state           = 0x0;
   _icon            = DEFAULT;
   _grabbingWidget  = NULL;
   _hoveringWidget  = NULL;
   _followingWidget = NULL;
   _private->clear();
}

//------------------------------------------------------------------------------
//!
const Vec2f&
Pointer::position() const
{
   return curEvent().position();
}

//------------------------------------------------------------------------------
//!
const Vec2f&
Pointer::lastPosition() const
{
   return lastEvent().position();
}

//------------------------------------------------------------------------------
//!
const Vec2f&
Pointer::pressPosition() const
{
   return lastPressEvent().position();
}

//------------------------------------------------------------------------------
//!
const Event&
Pointer::curEvent() const
{
   return _private->_curEvent;
}

//------------------------------------------------------------------------------
//!
const Event&
Pointer::lastEvent() const
{
   return _private->_lastEvent;
}

//------------------------------------------------------------------------------
//!
const Event&
Pointer::lastPressEvent() const
{
   return _private->_lastPress;
}

//------------------------------------------------------------------------------
//!
const Event&
Pointer::lastReleaseEvent() const
{
   return _private->_lastRelease;
}

//------------------------------------------------------------------------------
//!
Vec2f
Pointer::getSpeed() const
{
   return _private->computeDerivatives( Core::lastTime() );
}

//------------------------------------------------------------------------------
//! Returns whether or not all of the move events stayed within a certain
//! distance (inclusive) of the original press event.
bool
Pointer::withinPress( float radius ) const
{
   return _private->_fartestDistSq <= (radius*radius);
}

//------------------------------------------------------------------------------
//! Returns whether or not we are within a certain delay of the last press event.
bool
Pointer::withinDelay( double delay ) const
{
   return (_private->_curEvent.timestamp() - _private->_lastPress.timestamp()) <= delay;
}

//------------------------------------------------------------------------------
//!
void
Pointer::curEvent( double time, const Event& event )
{
   _private->curEvent( time, event );
}
