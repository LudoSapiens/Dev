/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/Animation/SkeletalAnimation.h>

#include <Base/Dbg/DebugStream.h>

/*==============================================================================
  UNNAME NAMESPACE
==============================================================================*/

UNNAMESPACE_BEGIN

DBG_STREAM( os_sa, "SkeletalAnimation" );

const char* _anim_str = "skeletalAnimation";

UNNAMESPACE_END


NAMESPACE_BEGIN

/*==============================================================================
  CLASS SkeletalPose
  ==============================================================================*/

//------------------------------------------------------------------------------
//!
SkeletalPose::SkeletalPose( const Reff& ref, uint size ):
   _referential( ref ), _bonesOrient(size)
{}

//------------------------------------------------------------------------------
//!
SkeletalPose::~SkeletalPose()
{}

//------------------------------------------------------------------------------
//!
RCP<SkeletalPose> 
SkeletalPose::clone() const
{
   RCP<SkeletalPose> pose( new SkeletalPose() );
   pose->_referential = _referential;
   pose->_bonesOrient = _bonesOrient;
   return pose;
}

//-----------------------------------------------------------------------------
//!
void
SkeletalPose::print() const
{
   StdErr << "SkelPose{" << nl
          << "  ref=" << referential() << nl
          << "  " << numBones() << "bones:" << nl;
   for( uint i = 0; i < numBones(); ++i )
   {
      StdErr << "  [" << i << "]: " << _bonesOrient[i] << nl;
   }
   StdErr << "}" << nl;
}

/*==============================================================================
  CLASS SkeletalAnimation
==============================================================================*/

//------------------------------------------------------------------------------
//!
SkeletalAnimation::SkeletalAnimation() : 
   _flags( 0x0 ), // ABSOLUTE, NON-CYCLIC
   _rate( 24.0f ),
   _duration( 0 )
{}

//------------------------------------------------------------------------------
//!
SkeletalAnimation::~SkeletalAnimation()
{}
   
//------------------------------------------------------------------------------
//!
RCP<SkeletalAnimation> 
SkeletalAnimation::clone() const
{
   RCP<SkeletalAnimation> anim( new SkeletalAnimation() );
   anim->_flags    = _flags;
   anim->_rate     = _rate;
   anim->_duration = _duration;
   anim->_velocity = _velocity;
   anim->_offset   = _offset;
   anim->_skeleton = _skeleton;
   
   anim->_poses.resize( numPoses() );
   for( uint i = 0; i < numPoses(); ++i )
   {
      anim->_poses[i] = pose(i)->clone();
   }
   return anim;
}

//------------------------------------------------------------------------------
//!
RCP<SkeletalAnimation> 
SkeletalAnimation::clone( uint startPose, uint endPose ) const
{
   RCP<SkeletalAnimation> anim( new SkeletalAnimation() );
   anim->_flags    = _flags;
   anim->_rate     = _rate;
   anim->_duration = _duration;
   anim->_velocity = _velocity;
   anim->_offset   = _offset;
   anim->_skeleton = _skeleton;

   if( startPose <= endPose )
   {
      uint lastPose = numPoses() - 1;
      if( startPose <= lastPose )
      {
         if( endPose > lastPose )
         {
            endPose = lastPose;
         }
         anim->reservePoses( endPose - startPose + 1 );
         for( uint i = startPose; i <= endPose; ++i )
         {
            anim->addPose( pose(i)->clone().ptr() );
         }
      }
      // else starting after the end of the animation
   }
   // else invalid range specified

   return anim;
}

//------------------------------------------------------------------------------
//!
uint
SkeletalAnimation::getPoses( float time, SkeletalPose*& p0, SkeletalPose*& p1, float& t )
{
   float p = time * _rate;
   int sp  = int(p);
   t       = p - sp;
   sp      = sp % (_poses.size()-1);

   CHECK( sp >= 0 );
   CHECK( sp   < (int)numPoses() );
   CHECK( sp+1 < (int)numPoses() );
   p0 = _poses[sp].ptr();
   p1 = _poses[(sp+1)].ptr();

   return (uint)sp;
}

//------------------------------------------------------------------------------
//!
uint
SkeletalAnimation::getPosesClamped( float time, SkeletalPose*& p0, SkeletalPose*& p1, float& t )
{
   float p = time * _rate;
   uint sp = uint(p);
   t       = p - sp;

   uint n = uint(_poses.size());
   if( sp >= n-1 )
   {
      sp = n-2;
      t  = 1.0f;
   }

   CHECK( sp   < numPoses() );
   CHECK( sp+1 < numPoses() );
   p0 = _poses[sp].ptr();
   p1 = _poses[(sp+1)].ptr();

   return sp;
}

//------------------------------------------------------------------------------
//!
void
SkeletalAnimation::setPose( uint pose, const Vector<Quatf>& bones, const Reff& root )
{
   if( pose >= numPoses() )
   {
      _poses.resize( pose+1 );
      updateDuration();
   }
   
   _poses[pose]->referential( root );
   _poses[pose]->bones() = bones;
}

//------------------------------------------------------------------------------
//!
void
SkeletalAnimation::setPose( uint pose, const Vector<Reff>& bones, const Reff& root )
{
   if( pose >= numPoses() )
   {
      _poses.resize( pose+1 );
      updateDuration();
   }
   
   _poses[pose]->referential( root );
   _poses[pose]->bones().resize( bones.size() );
   
   for( uint i = 0; i < bones.size(); ++i )
   {
      _poses[pose]->bones()[i] = bones[i].orientation();
   }
}

//------------------------------------------------------------------------------
//!
void
SkeletalAnimation::makeRelative()
{
   if( type() == ABSOLUTE )
   {
      type( RELATIVE );
      
      // Compute velocity.
      _velocity   = ( pose( numPoses()-1 )->position() - pose(0)->position() ) / _duration;
      _velocity.y = 0.0f;
    
      // Compute offset.
      _offset = Vec3f(0.0f);
      for( uint p = 0; p < numPoses(); ++p )
      {
         _offset += pose(p)->position() - _velocity*(p/_rate);
      }
      _offset /= (float)numPoses();
      _offset.y = 0.0f;
      
      // Transform all absolute offsets to relative.
      for( uint p = 0; p < numPoses(); ++p )
      {
         pose(p)->position( pose(p)->position() - _velocity*(p/_rate) - _offset );
      }
   }
}

//------------------------------------------------------------------------------
//!
void
SkeletalAnimation::makeAbsolute()
{
   if( type() == RELATIVE )
   {
      type( ABSOLUTE );
      
      // Transform all relative offsets to absolute.
      for( uint p = 0; p < numPoses(); ++p )
      {
         pose(p)->position( pose(p)->position() + _velocity*(p/_rate) + _offset );
      }
   }
}

//-----------------------------------------------------------------------------
//! Fixes some corner cases.
void
SkeletalAnimation::prepare()
{
   if( numPoses() == 1 )
   {
      addPose( pose(0) );
   }
}

//-----------------------------------------------------------------------------
//!
void
SkeletalAnimation::print() const
{
   const char* typeToStr[] = {
      "ABSOLUTE",
      "RELATIVE",
      "<unknown>"
   };
   uint typeIdx = type();
   if( typeIdx > 2 )  typeIdx = 2;
   StdErr << "SkelAnim{" << nl;
   StdErr << "  type=" << typeToStr[typeIdx] << nl;
   StdErr << "  rate=" << rate() << nl;
   StdErr << "  duration=" << duration() << nl;
   StdErr << "  velocity=" << velocity() << nl;
   StdErr << "  offset=" << offset() << nl;
   StdErr << "  " << numPoses() << " poses" << nl;
   StdErr << "}" << nl;
}

//------------------------------------------------------------------------------
//! 
const char*
SkeletalAnimation::meta() const
{
   return _anim_str;
}

NAMESPACE_END
