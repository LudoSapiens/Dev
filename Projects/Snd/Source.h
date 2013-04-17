/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef SND_SOURCE_H
#define SND_SOURCE_H

#include <Snd/StdDefs.h>
#include <Snd/Manager.h>
#include <Snd/Types.h>

#include <Base/Util/Bits.h>
#include <Base/Util/RCObject.h>
#include <Base/Util/RCP.h>


NAMESPACE_BEGIN

namespace Snd
{

class Sound;

/*==============================================================================
  CLASS Source
==============================================================================*/
class Source:
   public RCObject
{
public:

   /*----- methods -----*/

   // State
   float  gain() const { return _gain; }
   void   gain( const float gain ) { _gain = gain; }

   float  pitch() const { return _pitch; }
   void   pitch( const float pitch ) { _pitch = pitch; }

         Vec3f&  position()       { return _position; }
   const Vec3f&  position() const { return _position; }
   void  position( const Vec3f& p ) { _position = p; }

         Vec3f&  velocity()       { return _velocity; }
   const Vec3f&  velocity() const { return _velocity; }
   void  velocity( const Vec3f& v ) { _velocity = v; }

   bool looping() const { return getbits(_params, 0, 1) != 0; }
   void looping( bool v ) { _params = setbits(_params, 0, 1, v?1:0); }

   bool relative() const { return getbits(_params, 1, 1) != 0; }
   void relative( bool v ) { _params = setbits(_params, 1, 1, v?1:0); }

   bool autoDelete() const { return getbits(_params, 2, 1) != 0; }
   void autoDelete( bool v ) { _params = setbits(_params, 2, 1, v?1:0); }

   void  bind( const Sound* sound ) { _mgr->bind(this, sound); }

   SND_DLL_API void  setDefaults();
   SND_DLL_API void  setBGM();


   // Commands
   inline bool  update() { return _mgr->update(this); }

   inline bool  play()   { return _mgr->play(this);   }
   inline bool  pause()  { return _mgr->pause(this);  }
   inline bool  stop()   { return _mgr->stop(this);   }
   inline bool  rewind() { return _mgr->rewind(this); }

   // State queries
   inline bool  playing() const { return _mgr->playing(this); }
   inline bool  stopped() const { return _mgr->stopped(this); }

   inline bool  valid()   const { return _id != INVALID_ID;   }

protected:

   /*----- data members -----*/
   Manager*  _mgr;       //!< A pointer to the manager.
   uint      _id;        //!< A unique identifier.
   float     _gain;      //!< The gain, or volume (1.0f meaning no adjustment).
   float     _pitch;     //!< Pitch multiplier (from (0, +inf)).
   Vec3f     _position;  //!< The location of the listener
   Vec3f     _velocity;  //!< The speed of the listener
   uchar     _params;    //!< A series of bits packed together.
   //! looping    (1)  _params[ 0: 0]    //!< Set when source should loop.
   //! relative   (1)  _params[ 1: 1]    //!< Set when source is specified relative to the listener (e.g. BGM).
   //! autoDelete (1)  _params[ 2: 2]    //!< Specifies that the source should delete itself once it is done playing.
   RCP<const Sound>  _sound;  //!< The associated sound.

   /*----- methods -----*/

   SND_MAKE_MANAGERS_FRIENDS();

   // Only managers can create this object
   Source( Manager* mgr );
   virtual ~Source();

   inline uint  id() const { return _id; }
   inline void  id( uint v ) { _id = v; }

   void  terminate();

private:
}; //class Source


}  //namespace Snd

NAMESPACE_END


#endif //SND_SOURCE_H
