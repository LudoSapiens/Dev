/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_PUPPETEER_H
#define PLASMA_PUPPETEER_H

#include <Plasma/StdDefs.h>

#include <Plasma/Animation/AnimationGraph.h>
#include <Plasma/Animation/SkeletalAnimation.h>
#include <Plasma/Animation/Skeleton.h>

#include <Fusion/Resource/ResourceTask.h>
#include <Fusion/VM/VM.h>

#include <CGMath/Vec3.h>
#include <CGMath/Sphere.h>

#include <Base/ADT/Vector.h>
#include <Base/Util/RCP.h>

NAMESPACE_BEGIN


/*==============================================================================
  CLASS Puppeteer
==============================================================================*/
class Puppeteer
{
public:

   /*----- data types -----*/

   /*==============================================================================
     CLASS PositionalConstraint
   ==============================================================================*/
   class PositionalConstraint
   {
   public:
      PositionalConstraint()
      { }

      PositionalConstraint( const Vec3f& position, float weight, int id ):
         _position( position ), _weight( weight ), _id( id ) { }

            Vec3f& position()                 { return _position; }
      const Vec3f& position() const           { return _position; }
      void  position( const Vec3f& position ) { _position = position; }

      float weight() const                    { return _weight; }
      void  weight( float weight )            { _weight = weight; }

      int id() const                          { return _id; }
      void id( int newID )                    { _id = newID; }

      const Spheref& sphere() const           { return _sphere; }
      const Vec3f& currentPos() const         { return _currentPos; }
      const Vec3f& endPos() const             { return _endEffectorPos; }

   protected:

      /*----- data members -----*/

      Vec3f   _position; // The position (in world space) of the end effector.
      float   _weight;   // An importance value (1.0 means pos is absolutely respected, 0.0 means we don't care).
      int     _id;       // A unique ID (could be the bone ID, or use a remapping table from one to the other).

      /*----- methods -----*/

   private:
      friend class Puppeteer;

      mutable Vec3f   _currentPos;
      mutable Vec3f   _endEffectorPos;
      mutable Spheref _sphere;
   }; //class PositionalConstraint


   /*----- methods -----*/

   //------------------------------------------------------------------------------
   //! Moves the animation into the specified referential.
   PLASMA_DLL_API static RCP<SkeletalAnimation> applyReferential(
      SkeletalAnimation* srcAnim,
      const Reff&        ref
   );

   //------------------------------------------------------------------------------
   //! Blend between two animation. Going from the starting animation to the ending
   //! animation.
   PLASMA_DLL_API static RCP<SkeletalAnimation> blend(
      SkeletalAnimation* startAnim,
      SkeletalAnimation* endAnim
   );

   //------------------------------------------------------------------------------
   //! Blends between 2 animations at a specific frame.
   //! This is done by interpolating 2 frames of animA, then 2 frames of animB,
   //! then blending the 2 resulting frames together according to the factor.
   //! @param animA:   The first animation.
   //! @param timeA:   A time hint for the first animation.
   //! @param animB:   The second animation.
   //! @param timeB:   A time hint for the second animation.
   //! @param factor:  A blend factor to merge animA with animB.
   //! @param dstPose: A pre-allocated pose into which to store the result.
   PLASMA_DLL_API static void blendFrame(
      SkeletalAnimation* animA,
      float              timeA,
      SkeletalAnimation* animB,
      float              timeB,
      float              factor,
      SkeletalPose&      dstPose
   );

   //------------------------------------------------------------------------------
   //! Blend between two poses (pose0 and pose1), and stores the result in a
   //! preallocated pose of the right size (dstPose).
   PLASMA_DLL_API static void blendPoses(
      SkeletalPose*  pose0,
      SkeletalPose*  pose1,
      float          factor,
      SkeletalPose&  dstPose
   );

   //------------------------------------------------------------------------------
   //! Blend between 4 poses (pose0 and pose1), and stores the result in a
   //! preallocated pose of the right size (dstPose).
   //! It performs trilinear interpolation:
   //!   lerp( lerp(p0, p1, f01), lerp(p2, p3, f23), f01_23 )
   PLASMA_DLL_API static void blendPoses(
      SkeletalPose*  p0,
      SkeletalPose*  p1,
      SkeletalPose*  p2,
      SkeletalPose*  p3,
      float          f01,
      float          f23,
      float          f01_23,
      SkeletalPose&  dstPose
   );

   //------------------------------------------------------------------------------
   //! Append anim2 at the end of anim1 (no blending whatsoever).
   PLASMA_DLL_API static RCP<SkeletalAnimation> concatenate(
      SkeletalAnimation* anim1,
      SkeletalAnimation* anim2
   );

   //------------------------------------------------------------------------------
   //! Retrieve a portion of anim between the specified start and end positions.
   PLASMA_DLL_API static RCP<SkeletalAnimation> cut(
      SkeletalAnimation* anim,
      uint               startPose,
      uint               endPose
   );

   //------------------------------------------------------------------------------
   //! Create a cycling animation from a non cycling animation.
   PLASMA_DLL_API static RCP<SkeletalAnimation> cycle(
      SkeletalAnimation* anim,
      float              fraction
   );

   //------------------------------------------------------------------------------
   //! Removes the specified axis from the rotation of the root.
   PLASMA_DLL_API static RCP<SkeletalAnimation> dropRotation(
      SkeletalAnimation* anim,
      const Vec3f&       axis
   );

   //------------------------------------------------------------------------------
   //! Resample the animation sequence in order to reach the specific rate.
   //! It changes the duration in order to meet the specified rate exactly,
   //! unless forceDuration is set to false, in which case it will recalculate
   //! a rate in order to meet the original duration instead.
   PLASMA_DLL_API static RCP<SkeletalAnimation> resample(
      SkeletalAnimation* anim,
      float              rate,
      bool               forceDuration = false
   );

   //------------------------------------------------------------------------------
   //!
   PLASMA_DLL_API static RCP<SkeletalAnimation> retarget(
      SkeletalAnimation* srcAnim,
      Skeleton*          dstSkel
   );

   //------------------------------------------------------------------------------
   //! Retargets all of an animation from one skeleton to another one.
   //! Should be called at load time (or preprocess).
   PLASMA_DLL_API static RCP<SkeletalAnimation> retarget(
      Skeleton*          srcSkel,
      Skeleton*          dstSkel,
      SkeletalAnimation* srcAnim,
      bool               constraints,
      const Vec3f&       userScale = Vec3f( 1.0f )
   );

   //------------------------------------------------------------------------------
   //! Reverses the animation.
   PLASMA_DLL_API static RCP<SkeletalAnimation> reverse(
      SkeletalAnimation* anim
   );

   //------------------------------------------------------------------------------
   //! Create a transition animation from one animation (startAnim) to another one
   //! (endAnim) lasting for a specified duration.
   PLASMA_DLL_API static RCP<SkeletalAnimation> transition(
      SkeletalAnimation* startAnim,
      SkeletalAnimation* endAnim,
      float              duration
   );

   //------------------------------------------------------------------------------
   //! Stores a transition frame (in dstPose) from one animation (startAnim) to another one
   //! (endAnim) using the specified factor.
   PLASMA_DLL_API static void transitionFrame(
      SkeletalAnimation* startAnim,
      SkeletalAnimation* endAnim,
      float              factor,
      SkeletalPose&      dstPose
   );

   //------------------------------------------------------------------------------
   //! Fixes a specific animation frame.
   //! @param skelInst:  The skeleton instance (has a pointer to the skeleton itself).
   //! @param pose:  The pose data to set into skelInst.
   PLASMA_DLL_API static void setPose(
      Skeleton::Instance& skelInst,
      const Vector<Reff>& pose
   );

   //------------------------------------------------------------------------------
   //! Fixes a specific animation frame.
   //! @param skelInst:  The skeleton instance (has a pointer to the skeleton itself).
   //! @param pose:  The pose to set into skelInst.
   PLASMA_DLL_API static void setPose(
      Skeleton::Instance&  skelInst,
      SkeletalPose*        pose
   );

   //------------------------------------------------------------------------------
   //! Applies a specific animation to a skeletal instance.
   //! This is typically done by interpolating between the 2 closest frames.
   //! @param skelInst:  The skeleton instance (has a pointer to the skeleton itself).
   //! @param anim:  The animation (already retargetted to the skel) we adapt based on
   //!               the constraints.
   //! @param poseID:  The actual pose index from anim to use.
   PLASMA_DLL_API static void setPose(
      Skeleton::Instance& skelInst,
      SkeletalAnimation*  anim,
      const uint          poseID
   );

   //------------------------------------------------------------------------------
   //! Applies a specific animation to a skeletal instance.
   //! This is typically done by interpolating between the 2 closest frames.
   //! @param skelInst:  The skeleton instance (has a pointer to the skeleton itself).
   //! @param anim:  The animation (already retargetted to the skel) we adapt based on
   //!               the constraints.
   //! @param time:  A time hint to determine the current pose(s) to use.
   //! @return:  The frame ID being used (the first if blending between 2).
   PLASMA_DLL_API static uint setPose(
      Skeleton::Instance&           skelInst,
      SkeletalAnimation*            anim,
      float                         time
   );

   //------------------------------------------------------------------------------
   //! Adapts an animation at runtime, and computes the required transforms to
   //! apply to the skeleton.
   //! @param skelInst:  The skeleton instance (has a pointer to the skeleton itself).
   //! @param anim:  The animation (already retargetted to the skel) we adapt based on
   //!               the constraints.
   //! @param constraints:  The constraints we have to respect (end effectors with importance).
   //! @param time:  A time hint to determine the current pose(s) to use.
   //! @return:  The frame ID being used (the first if blending between 2).
   PLASMA_DLL_API static uint setPose(
      Skeleton::Instance&                   skelInst,
      SkeletalAnimation*                    anim,
      const Vector< PositionalConstraint >& constraints,
      const float                           time
   );

   //------------------------------------------------------------------------------
   //! Adapts an animation at runtime, and computes the required transforms to
   //! apply to the skeleton.
   //! @param skelInst:  The skeleton instance (has a pointer to the skeleton itself).
   //! @param constraints:  The constraints we have to respect (end effectors with importance).
   PLASMA_DLL_API static void resolveConstraints(
      Skeleton::Instance&                   skelInst,
      const Vector< PositionalConstraint >& constraints
   );

protected:

   /*----- methods -----*/

   Puppeteer();
   virtual ~Puppeteer();

private:
}; //class Puppeteer

/*==============================================================================
   CLASS AnimationRetargetingTask
==============================================================================*/

class AnimationRetargetingTask:
   public Task
{
public:

   /*----- methods -----*/

   AnimationRetargetingTask( Resource<SkeletalAnimation>* res, SkeletalAnimation* anim, Skeleton* skel ):
      _res( res ), _anim( anim ), _skel( skel ) {}

   virtual void execute()
   {
      RCP<SkeletalAnimation> anim = Puppeteer::retarget( _anim.ptr(), _skel.ptr() );
      _res->data( anim.ptr() );
   }

private:

   /*----- data members -----*/

   RCP< Resource<SkeletalAnimation> > _res;
   RCP<SkeletalAnimation>             _anim;
   RCP<Skeleton>                      _skel;
};

/*==============================================================================
   CLASS GraphRetargetingTask
==============================================================================*/

class GraphRetargetingTask:
   public ResourceTask
{
public:

   /*----- methods -----*/

   GraphRetargetingTask( Resource<AnimationGraph>* res, AnimationGraph* graph, Skeleton* skel ):
      _res( res ), _graph( graph ), _skel( skel ) {}

   virtual void execute();

private:

   /*----- data members -----*/

   RCP< Resource<AnimationGraph> > _res;
   RCP<AnimationGraph>             _graph;
   RCP<Skeleton>                   _skel;
};

NAMESPACE_END


#endif //PLASMA_PUPPETEER_H
