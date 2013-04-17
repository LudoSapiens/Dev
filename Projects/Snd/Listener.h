/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef SND_LISTENER_H
#define SND_LISTENER_H

#include <Snd/StdDefs.h>
#include <Snd/Manager.h>
#include <Snd/Types.h>

#include <Base/Util/RCObject.h>


NAMESPACE_BEGIN

namespace Snd
{

/*==============================================================================
  CLASS Listener
==============================================================================*/
class Listener:
   public RCObject
{
public:

   /*----- methods -----*/
   float  gain() const { return _gain; }
   void   gain( const float g ) { _gain = g; }

   //float  pitch() const { return _pitch; }
   //void   pitch( const float p ) { _pitch = p; }

         Vec3f&  position()       { return _position; }
   const Vec3f&  position() const { return _position; }
   void  position( const Vec3f& p ) { _position = p; }

         Vec3f&  velocity()       { return _velocity; }
   const Vec3f&  velocity() const { return _velocity; }
   void  velocity( const Vec3f& v ) { _velocity = v; }

         Vec3f&  pointOfInterest()       { return _poi; }
   const Vec3f&  pointOfInterest() const { return _poi; }
   void  pointOfInterest( const Vec3f& poi ) { _poi = poi; }

         Vec3f&  upVector()       { return _up; }
   const Vec3f&  upVector() const { return _up; }
   void  upVector( const Vec3f& up ) { _up = up; }

   inline bool  update() { return _mgr->update(this); }

   SND_DLL_API void  setDefaults();

protected:

   /*----- data members -----*/
   Manager*  _mgr;       //!< A pointer to the manager.
   uint      _id;        //!< A unique identifier.
   float     _gain;      //!< The gain, or volume (positive).
   //float     _pitch;     //!< The pitch, or speed (from 0 to +INF).
   Vec3f     _position;  //!< The location of the listener
   Vec3f     _velocity;  //!< The speed of the listener
   Vec3f     _poi;       //!< Point of interest (for orientation, needs to be followed by _up)
   Vec3f     _up;        //!< The 'up' vector (for orientation, needs to be preceeded by _poi)

   /*----- methods -----*/

   SND_MAKE_MANAGERS_FRIENDS();

   // Only managers can create this object
   Listener( Manager* mgr );
   virtual ~Listener();

   inline uint  id() const { return _id; }
   inline void  id( uint v ) { _id = v; }

   void  terminate();

private:
}; //class Listener


}  //namespace Snd

NAMESPACE_END


#endif //SND_LISTENER_H
