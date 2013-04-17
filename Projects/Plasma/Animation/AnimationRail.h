/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_ANIMATION_RAIL_H
#define PLASMA_ANIMATION_RAIL_H

#include <Plasma/StdDefs.h>

#include <Fusion/VM/VMObject.h>

#include <CGMath/Ref.h>

#include <Base/ADT/StringMap.h>
#include <Base/ADT/Vector.h>
#include <Base/Util/RCObject.h>

NAMESPACE_BEGIN


/*==============================================================================
  CLASS AnimationRail
==============================================================================*/
class AnimationRail:
   public RCObject
{
public:

   struct Anchor
   {
      Anchor() {}
      Anchor( const Reff& r, const float t = -1.0f ): _ref(r), _time(t) {}
      Reff  _ref;
      float _time;
      bool operator==( const Anchor& a ) { return _ref == a._ref && _time == a._time; }
   };

   typedef Vector< Anchor >  AnchorContainer;

   /*----- methods -----*/

   PLASMA_DLL_API AnimationRail();
   PLASMA_DLL_API virtual ~AnimationRail();

   // Anchors.
   inline void  reserveAnchors( const uint s ) { _anchors.reserve( s ); }

   inline void  addAnchor( const Anchor& a ) { _anchors.pushBack( a ); }
   inline void  addAnchor( const Reff& r, const float f = -1.0f ) { addAnchor( Anchor(r, f) ); }

   inline void  removeAnchor( const Anchor& a ) { _anchors.remove( a ); }
   inline void  removeAnchor( const uint idx ) { _anchors.erase( _anchors.begin() + idx ); }
   inline void  removeAllAnchors() { _anchors.clear(); }

   inline uint  numAnchors() const { return uint(_anchors.size()); }

   inline       Anchor&  anchor( const uint idx )       { return _anchors[idx]; }
   inline const Anchor&  anchor( const uint idx ) const { return _anchors[idx]; }

   inline       AnchorContainer&  anchors()       { return _anchors; }
   inline const AnchorContainer&  anchors() const { return _anchors; }

   // Management.
   PLASMA_DLL_API void  normalize();

   inline uint  lastIndexUsed() const { return _lastIndexUsed; }
   inline void  lastIndexUsed( const uint idx ) { _lastIndexUsed = idx; }

   // Retrieves an interpolated referential at time 't' in the animation.
   PLASMA_DLL_API Reff  get( const float t );

   PLASMA_DLL_API uint  findIndex( const float t, const uint sidx ) const;
   PLASMA_DLL_API uint  findIndex_BinarySearch( const float t, const uint sidx, const uint eidx ) const;

   // VM.
   PLASMA_DLL_API void  init( VMState* vm );
   PLASMA_DLL_API bool  performGet( VMState* vm );
   PLASMA_DLL_API bool  performSet( VMState* vm );

protected:

   /*----- data members -----*/

   AnchorContainer  _anchors;        //!< All of the anchor points.
   uint             _lastIndexUsed;  //!< The last anchor index used (optimizes searches).

   /*----- methods -----*/

   //! Returns the index of next anchor (starting at idx) which doesn't include
   //! a negative time.
   //! Of none are found, numAnchors() is returned (which is an invalid index).
   uint  findNextValid( uint idx )
   {
      const uint n = numAnchors();
      while( idx < n && _anchors[idx]._time < 0.0f )
      {
         ++idx;
      }
      return idx;
   }

   //! Returns the index of next anchor (starting at idx) which include
   //! a negative time.
   //! Of none are found, numAnchors() is returned (which is an invalid index).
   uint  findNextInvalid( uint idx )
   {
      const uint n = numAnchors();
      while( idx < n && _anchors[idx]._time >= 0.0f )
      {
         ++idx;
      }
      return idx;
   }


}; //class AnimationRail


/*==============================================================================
  VM Section
  ==============================================================================*/

VMOBJECT_TRAITS( AnimationRail, rail )
typedef VMObject< AnimationRail > AnimationRailVM;


NAMESPACE_END

#endif //PLASMA_ANIMATION_RAIL_H
