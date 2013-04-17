/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/Animation/AnimationRail.h>

#include <Base/Dbg/DebugStream.h>

USING_NAMESPACE

/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN

DBG_STREAM( os_ar, "AnimationRail" );

//------------------------------------------------------------------------------
//!
enum
{
   ATTRIB_ADD_ANCHOR,
   ATTRIB_ADD_ANCHORS,
   ATTRIB_GET,
   ATTRIB_GET_ANCHOR,
   ATTRIB_GET_ANCHORS,
   ATTRIB_NORMALIZE,
   ATTRIB_NUM_ANCHORS,
   ATTRIB_REMOVE_ALL_ANCHORS,
   ATTRIB_REMOVE_ANCHOR,
   ATTRIB_SET_ANCHOR,
   NUM_ATTRIBS
};

StringMap _attributes(
   "addAnchor",         ATTRIB_ADD_ANCHOR,
   "addAnchors",        ATTRIB_ADD_ANCHORS,
   "get",               ATTRIB_GET,
   "getAnchor",         ATTRIB_GET_ANCHOR,
   "getAnchors",        ATTRIB_GET_ANCHORS,
   "normalize",         ATTRIB_NORMALIZE,
   "numAnchors",        ATTRIB_NUM_ANCHORS,
   "removeAllAnchors",  ATTRIB_REMOVE_ALL_ANCHORS,
   "removeAnchor",      ATTRIB_REMOVE_ANCHOR,
   "setAnchor",         ATTRIB_SET_ANCHOR,
   ""
);

//------------------------------------------------------------------------------
//! rail:addAnchor( ref )
//! rail:addAnchor( ref, time )
int
addAnchorVM( VMState* vm )
{
   AnimationRail* rail = (AnimationRail*)VM::thisPtr( vm );
   CHECK( rail != NULL );

   Reff r = VM::toReff( vm, 1 );

   float t = -1.0f;
   uint top = VM::getTop( vm );
   if( top >= 2 )
   {
      t = (float)VM::toNumber( vm, 2 );
   }

   rail->addAnchor( r, t );

   return 0;
}

//------------------------------------------------------------------------------
//! rail:addAnchors( { {r1, t1}, {r2, t2}, {r3, t3} } )
int
addAnchorsVM( VMState* vm )
{
   AnimationRail* rail = (AnimationRail*)VM::thisPtr( vm );
   CHECK( rail != NULL );

   if( VM::isTable( vm, 1 ) )           //[T, ...]
   {
      uint n = VM::getTableSize( vm, 1 );
      rail->reserveAnchors( n );

      for( uint i = 1; i <= n; ++i )
      {
         VM::geti( vm, -1, i );         //[T, ..., anchor]
         if( VM::isTable( vm, -1 ) )
         {
            VM::geti( vm, -1, 1 );      //[T, ..., anchor, ref]
            Reff r = VM::toReff( vm, -1 );
            VM::pop( vm, 1 );           //[T, ..., anchor]

            VM::geti( vm, -1, 2 );      //[T, ..., anchor, t]
            float t = (float)VM::toNumber( vm, -1 );
            VM::pop( vm, 1 );           //[T, ..., anchor]

            rail->addAnchor( r, t );
         }
         else
         {
            StdErr << "ERROR - Anchor #" << i << " is not a table." << nl;
         }
         VM::pop( vm, 1 );              //[T, ...]
      }
   }
   else
   {
      StdErr << "ERROR - AnimationRails::addAnchors did not receive a table." << nl;
   }

   return 0;
}

//------------------------------------------------------------------------------
//! local ref = rail:get( 0.25 )
int
getVM( VMState* vm )
{
   AnimationRail* rail = (AnimationRail*)VM::thisPtr( vm );
   CHECK( rail != NULL );
   float t = (float)VM::toNumber( vm, 1 );
   VM::push( vm, rail->get(t) );
   return 1;
}

//------------------------------------------------------------------------------
//! local ref,time = rail:getAnchor( rail.numAnchors - 1 )
int
getAnchorVM( VMState* vm )
{
   AnimationRail* rail = (AnimationRail*)VM::thisPtr( vm );
   CHECK( rail != NULL );
   uint idx = VM::toUInt( vm, 1 ) - 1;
   if( idx < rail->numAnchors() )
   {
      const AnimationRail::Anchor& a = rail->anchor(idx);
      VM::newTable( vm );      // [..., t]
      VM::push( vm, a._ref );  // [..., t, ref]
      VM::seti( vm, -2, 1 );   // [..., t]  with t[1] = ref
      VM::push( vm, a._time ); // [..., t, time]
      VM::seti( vm, -2, 2 );   // [..., t]  with t[2] = time
   }
   else
   {
      // Return nil
      VM::push( vm );
   }
   return 1;
}

//------------------------------------------------------------------------------
//! local tbl = rail:getAnchors()
int
getAnchorsVM( VMState* vm )
{
   AnimationRail* rail = (AnimationRail*)VM::thisPtr( vm );
   CHECK( rail != NULL );
   VM::newTable( vm );           // [..., T]
   const uint n = rail->numAnchors();
   for( uint i = 0; i < n; ++i )
   {
      const AnimationRail::Anchor& a = rail->anchor(i);
      VM::newTable( vm );        // [..., T, t]
      VM::push( vm, a._ref );    // [..., T, t, ref]
      VM::seti( vm, -2, 1 );     // [..., T, t]  with t[1] = ref
      VM::push( vm, a._time );   // [..., T, t, time]
      VM::seti( vm, -2, 2 );     // [..., T, t]  with t[2] = time
      VM::seti( vm, -2, i+1 );   // [..., T]  with T[i+1] = t
   }
   return 1;
}

//------------------------------------------------------------------------------
//!
int
normalizeVM( VMState* vm )
{
   AnimationRail* rail = (AnimationRail*)VM::thisPtr( vm );
   CHECK( rail != NULL );
   rail->normalize();
   return 0;
}

//------------------------------------------------------------------------------
//!
int
removeAllAnchorsVM( VMState* vm )
{
   AnimationRail* rail = (AnimationRail*)VM::thisPtr( vm );
   CHECK( rail != NULL );
   rail->removeAllAnchors();
   return 0;
}

//------------------------------------------------------------------------------
//! rail.removeAnchor( 9 )
int
removeAnchorVM( VMState* vm )
{
   AnimationRail* rail = (AnimationRail*)VM::thisPtr( vm );
   CHECK( rail != NULL );
   CHECK( VM::getTop( vm ) == 1 );
   uint idx = VM::toUInt( vm, 1 ) - 1;
   if( idx < rail->numAnchors() )
   {
      rail->removeAnchor( idx );
   }
   return 0;
}

//------------------------------------------------------------------------------
//! rail.setAnchor( idx, ref )
//! rail.setAnchor( idx, ref, time )
int
setAnchorVM( VMState* vm )
{
   AnimationRail* rail = (AnimationRail*)VM::thisPtr( vm );
   CHECK( rail != NULL );

   uint idx = VM::toUInt( vm, 1 ) - 1;
   AnimationRail::Anchor& a = rail->anchor( idx );

   a._ref = VM::toReff( vm, 2 );

   if( VM::getTop( vm ) >= 3 )
   {
      a._time = (float)VM::toNumber( vm, 3 );
   }

   return 0;
}


//------------------------------------------------------------------------------
//! Converts an incoming [0, 1] such that the linear segment
//! becomes a smooth C2 curve.
inline float
interpolate( float t )
{
   //return (-2.0f*t + 3.0f)*t*t;
   return t;
}

UNNAMESPACE_END

//------------------------------------------------------------------------------
//!
AnimationRail::AnimationRail():
   _lastIndexUsed( 0 )
{
}

//------------------------------------------------------------------------------
//!
AnimationRail::~AnimationRail()
{
}

//------------------------------------------------------------------------------
//! Converts all of the negative anchor times into interpolated and extrapolated
//! values.
void
AnimationRail::normalize()
{
   DBG_BLOCK( os_ar, "AnimationRail::normalize()" );
   const uint n = numAnchors();

   if( n <= 1 )
   {
      return;
   }

   uint vidx = findNextValid( 0 );

   if( vidx == n )
   {
      if( n != 0 )
      {
         // Extrapolate all of the values.
         // Assume total time is 1.0f, and spread the frames accordingly.
         float n_1 = (float)(n-1);
         for( uint i = 0; i < n; ++i )
         {
            _anchors[i]._time = (float)i / n_1;
         }
      }
      else
      {
         // Special case; simply assign 0.
         _anchors[0]._time = 0.0f;
      }
   }
   else
   if( vidx != 0 )
   {
      // Extrapolate the first few values.
      // We simply spread from 0.0f to the first valid timestamp.
      float dur = _anchors[vidx]._time;
      for( uint i = 0; i < vidx; ++i )
      {
         _anchors[i]._time = dur * (float)i / (float)vidx;
      }
   }

   uint iidx = findNextInvalid( vidx + 1 );
   while( iidx < n )
   {
      CHECK( iidx < n );
      // Find whether it is a closed range (interpolate), or half-open (extrapolate on the right).
      uint rvidx = findNextValid( iidx + 1 );
      if( rvidx != n )
      {
         // Interpolate.
         const Anchor& lAnchor = _anchors[iidx-1];
         const Anchor& rAnchor = _anchors[rvidx];
         float delta = rAnchor._time - lAnchor._time;
         for( uint i = 0; i < vidx; ++i )
         {
            _anchors[i]._time = lAnchor._time + delta * (float)i / (float)vidx;
         }
         iidx = findNextInvalid( rvidx + 1 );
      }
      else
      {
         // Extrapolate, and finish.
         uint lastValidIdx = iidx - 1;
         float delta = (lastValidIdx != 0) ? _anchors[lastValidIdx]._time / (float)lastValidIdx : 1.0f;
         float n_1 = (float)(n-1);
         for( uint i = iidx; i < n; ++i )
         {
            _anchors[i]._time = delta * (float)i / n_1;
         }
         iidx = n;
      }
   }
}

//------------------------------------------------------------------------------
//! Retrieves an interpolated referential at time 't' in the animation.
Reff
AnimationRail::get( const float t )
{
   DBG_BLOCK( os_ar, "AnimationRail::get(" << t << ")" );
   Reff  ref;
   uint  idx = findIndex( t, _lastIndexUsed );
   DBG_MSG( os_ar, "idx=" << idx << "/" << numAnchors() );
   if( idx >= numAnchors() )
   {
      // Last anchor.
      ref = _anchors.back()._ref;
      DBG_MSG( os_ar, "Last anchor, returning: " << ref );
   }
   else
   {
      const Anchor& a0 = _anchors[idx];
      const Anchor& a1 = _anchors[idx+1];
      float delta      = a1._time - a0._time;
      float cur        = t        - a0._time;
      float f          = interpolate( cur / delta );
      ref = a0._ref.slerp( a1._ref, f );
      DBG_MSG( os_ar, "Interpolating " << "#" << idx << "(" << a0._time << ", " << a0._ref << ")" );
      DBG_MSG( os_ar, "         with " << "#" << idx+1 << "(" << a1._time << ", " << a1._ref << ")" );
      DBG_MSG( os_ar, " delta=" << delta << " cur=" << cur << " f=" << f << " --> " << ref );
   }
   _lastIndexUsed = idx;
   return ref;
}

//------------------------------------------------------------------------------
//! Returns the index of the anchor happening just before the specified time.
//! Can specify a starting index to optimize the algorithm.
//! The algorithm will start at idx, then check idx+1, and resort to a binary
//! search if those two fail.
uint
AnimationRail::findIndex( const float t, /*const*/ uint idx ) const
{
   CHECK( !_anchors.empty() );

   uint lastIdx = numAnchors();

   // Check current candidate.
   if( _anchors[idx]._time <= t )
   {
      // Can still be a candidate.                       i <= t
      if( idx == lastIdx || t < _anchors[idx+1]._time )
      {
         // The specified specified index is perfect.    i <= t < i+1
      }
      else
      {
         // Check the next.
         ++idx;
         if( _anchors[idx]._time <= t )
         {
            // Next can be a candidate.                  i+1 <= t
            if( idx == lastIdx || t < _anchors[idx+1]._time )
            {
               // The next index is fine.                i+1 <= t < i+2
               return idx;
            }
         }

         // Perform a binary search on the right.        i+1 <= t < n
         // (we could start at i+2, since we know i+1 <= t
         // and we also determined that i+2 is not bigger,
         // but it's better to send a valid left range here)
         return findIndex_BinarySearch( t, idx, lastIdx );
      }
   }

   // Perform a binary search on the left.
   return findIndex_BinarySearch( t, 0, idx );
}

//------------------------------------------------------------------------------
//! Performs a binary search to find the first index such that:
//!   _anchors[idx] <= t < _anchors[idx+1]
//! The incoming sidx and eidx must be valid indices.
uint
AnimationRail::findIndex_BinarySearch( const float t, /*const*/ uint sidx, /*const*/ uint eidx ) const
{
   while( sidx != eidx )
   {
      uint hidx = (sidx + eidx) >> 1;
      if( hidx == sidx )
      {
         // Avoid infinite loops.
         if( _anchors[eidx]._time <= t )
         {
            // Special case where orginal eidx was the left bound.
            sidx = eidx;
         }
         break;
      }
      else
      if( _anchors[hidx]._time <= t )
      {
         // New left bound.
         sidx = hidx;
      }
      else
      {
         // New right bound.
         eidx = hidx;
      }
   }
   return sidx;
}

//------------------------------------------------------------------------------
//!
void
AnimationRail::init( VMState* vm )
{
   DBG_BLOCK( os_ar, "AnimationRail::init" );

   if( VM::isTable( vm, -1 ) )
   {
      // Start iterating at index 0 (nil, pushed below).
      VM::push( vm );
      while( VM::next( vm, -2 ) )
      {
         // Let the performSet() routine handle the various assignments.
         performSet( vm );

         // Pop the value, but keep the key.
         VM::pop( vm, 1 );
      }

      // Creation-only parameters...
   }
}

//------------------------------------------------------------------------------
//!
bool
AnimationRail::performGet( VMState* vm )
{
   DBG_BLOCK( os_ar, "AnimationRail::performGet" );
   const char* str = VM::toCString( vm, -1 );
   switch( _attributes[str] )
   {
      case ATTRIB_ADD_ANCHOR:
      {
         VM::push( vm, this, addAnchorVM );
      }  return true;
      case ATTRIB_ADD_ANCHORS:
      {
         VM::push( vm, this, addAnchorsVM );
      }  return true;
      case ATTRIB_GET:
      {
         VM::push( vm, this, getVM );
      }  return true;
      case ATTRIB_GET_ANCHOR:
      {
         VM::push( vm, this, getAnchorVM );
      }  return true;
      case ATTRIB_GET_ANCHORS:
      {
         VM::push( vm, this, getAnchorsVM );
      }  return true;
      case ATTRIB_NORMALIZE:
      {
         VM::push( vm, this, normalizeVM );
      }  return true;
      case ATTRIB_NUM_ANCHORS:
      {
         VM::push( vm, numAnchors() );
      }  return true;
      case ATTRIB_REMOVE_ALL_ANCHORS:
      {
         VM::push( vm, this, removeAllAnchorsVM );
      }  return true;
      case ATTRIB_REMOVE_ANCHOR:
      {
         VM::push( vm, this, removeAnchorVM );
      }  return true;
      case ATTRIB_SET_ANCHOR:
      {
         VM::push( vm, this, setAnchorVM );
      }  return true;
      default:
      {
      }  break;
   }
   return false;
}

//------------------------------------------------------------------------------
//!
bool
AnimationRail::performSet( VMState* vm )
{
   DBG_BLOCK( os_ar, "AnimationRail::performSet" );
   const char* str = VM::toCString( vm, -2 );
   switch( _attributes[str] )
   {
      case ATTRIB_ADD_ANCHOR:
      case ATTRIB_ADD_ANCHORS:
      case ATTRIB_GET:
      case ATTRIB_GET_ANCHOR:
      case ATTRIB_GET_ANCHORS:
      case ATTRIB_NORMALIZE:
      case ATTRIB_NUM_ANCHORS:
      case ATTRIB_REMOVE_ALL_ANCHORS:
      case ATTRIB_REMOVE_ANCHOR:
      case ATTRIB_SET_ANCHOR:
         //Read-only
         return true;
      default:
         break;
   }
   return false;
}
