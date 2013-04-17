/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_SKELETALANIMATION_H
#define PLASMA_SKELETALANIMATION_H

#include <Plasma/StdDefs.h>
#include <Plasma/Animation/Skeleton.h>

#include <Fusion/VM/VM.h>

#include <CGMath/Ref.h>

#include <Base/Util/Bits.h>
#include <Base/Util/RCObject.h>
#include <Base/Util/RCP.h>
#include <Base/ADT/Vector.h>

NAMESPACE_BEGIN


class SkeletalPose;

#ifdef ABSOLUTE
#undef ABSOLUTE
#endif

#ifdef RELATIVE
#undef RELATIVE
#endif

/*==============================================================================
  CLASS SkeletalAnimation
==============================================================================*/

//! A loopable sequence should finish with the same pose as the first one.

class SkeletalAnimation:
   public RCObject,
   public VMProxy
{

public:

   /*----- types and enumerations ----*/

   enum {
      ABSOLUTE,
      RELATIVE
   };
   
   typedef Vector< RCP<SkeletalPose> >  PoseContainer;

   /*----- methods -----*/

   PLASMA_DLL_API SkeletalAnimation();

   PLASMA_DLL_API RCP<SkeletalAnimation> clone() const;
   PLASMA_DLL_API RCP<SkeletalAnimation> clone( uint startPose, uint endPos ) const;

   // Skeleton.
   inline Skeleton* skeleton() const   { return _skeleton.ptr(); }
   inline void skeleton( Skeleton* s ) { _skeleton = s; }

   // Poses.
   inline void reservePoses( uint );
   inline SkeletalPose* addPose( const Reff& ref = Reff::identity() );
   inline SkeletalPose* addPose( SkeletalPose* );
   inline void removePose( uint );
   inline SkeletalPose* pose( uint ) const;

   PLASMA_DLL_API uint getPoses( float time, SkeletalPose*& p0, SkeletalPose*& p1, float& t );
   PLASMA_DLL_API uint getPosesClamped( float time, SkeletalPose*& p0, SkeletalPose*& p1, float& t );
   PLASMA_DLL_API void setPose( uint, const Vector<Quatf>&, const Reff& );
   PLASMA_DLL_API void setPose( uint, const Vector<Reff>&, const Reff& );

   inline uint numPoses() const           { return (uint)_poses.size(); }
   
   // Time.
   inline void rate( float r )            { _rate = r; }
   inline float rate() const              { return _rate; }
   inline float duration() const          { return _duration; }

   // Cyclic.
   inline bool cyclic() const             { return getbit( _flags, 0 ); }
   inline void cyclic( bool v )           { _flags = setbit( _flags, 0, v ); }

   // Type.
   inline uint type() const               { return getbits( _flags, 1, 1 ); }
   inline void type( uint t )             { _flags = setbits( _flags, 1, 1, t ); }
   PLASMA_DLL_API void makeRelative();
   PLASMA_DLL_API void makeAbsolute();
   
   inline const Vec3f& velocity() const   { return _velocity; }
   inline void velocity( const Vec3f& v ) { _velocity = v; }
   inline const Vec3f& offset() const     { return _offset; }
   inline void offset( const Vec3f& o )   { _offset = o; }

   // Utility.
   PLASMA_DLL_API void prepare();

   PLASMA_DLL_API void print() const;

   // VM.
   virtual const char* meta() const;

protected:

   /*----- methods -----*/

   PLASMA_DLL_API virtual ~SkeletalAnimation();

   inline void updateDuration() { _duration = (_poses.size()-1) / _rate; }

private:

   /*----- data members -----*/

   //! NAME          SIZE   LOCATION         DESCRIPTION
   //! cyclic        (1)   _flags[ 0: 0]     Set to 1 when an animation is cyclic.
   //! relative      (1)   _flags[ 1: 1]     Set to 1 when an animation is relative (absolute otherwise).

   uint32_t      _flags;
   PoseContainer _poses;
   float         _rate;
   float         _duration;
   Vec3f         _velocity;
   Vec3f         _offset;
   RCP<Skeleton> _skeleton;
};

/*==============================================================================
  CLASS SkeletalPose
==============================================================================*/

//!

class SkeletalPose
   : public RCObject
{

public:

   /*----- types and enumerations ----*/

   typedef Vector< Quatf >  BoneContainer;

   /*----- methods -----*/

   SkeletalPose( const Reff& ref = Reff::identity(), uint size = 0 );

   PLASMA_DLL_API RCP<SkeletalPose> clone() const;

   inline void referential( const Reff& ref )      { _referential = ref; }
   inline const Reff& referential() const          { return _referential; }

   inline void position( const Vec3f& pos )        { _referential.position( pos ); }
   inline const Vec3f& position() const            { return _referential.position(); }  
   inline void orientation( const Quatf& orient )  { _referential.orientation( orient ); }
   inline const Quatf& orientation() const         { return _referential.orientation(); }

   inline void reserveBones( uint qty )            { _bonesOrient.reserve( qty ); }
   inline void addBone( const Quatf& orient )      { _bonesOrient.pushBack( orient ); }
   inline const BoneContainer& bones() const       { return _bonesOrient; }
   inline BoneContainer& bones()                   { return _bonesOrient; }
   inline uint numBones() const                    { return (uint)_bonesOrient.size(); }

   PLASMA_DLL_API void print() const;

protected:

   /*----- methods -----*/

   virtual ~SkeletalPose();
   
private:

   /*----- data members -----*/

   Reff          _referential;
   BoneContainer _bonesOrient;
};

//------------------------------------------------------------------------------
//!
inline void
SkeletalAnimation::reservePoses( uint qty )
{
   _poses.reserve( qty );
}

//------------------------------------------------------------------------------
//!
inline SkeletalPose*
SkeletalAnimation::addPose( const Reff& ref )
{
   _poses.pushBack( new SkeletalPose(ref) );
   updateDuration();
   return _poses.back().ptr();
}

//------------------------------------------------------------------------------
//!
inline SkeletalPose*
SkeletalAnimation::addPose( SkeletalPose* pose )
{
   _poses.pushBack( pose );
   updateDuration();
   return _poses.back().ptr();
}

//------------------------------------------------------------------------------
//!
inline void
SkeletalAnimation::removePose( const uint p )
{
   _poses.erase( _poses.begin() + p );
}

//------------------------------------------------------------------------------
//!
inline SkeletalPose*
SkeletalAnimation::pose( const uint p ) const
{
   return _poses[p].ptr();
}


NAMESPACE_END

#endif
