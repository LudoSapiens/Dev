/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/Animation/Puppeteer.h>
#include <Plasma/Animation/AnimationGraph.h>
#include <Plasma/Resource/ResManager.h>

#include <CGMath/AABBox.h>
#include <CGMath/CGConst.h>
#include <CGMath/Plane.h>

#include <Base/ADT/Pair.h>
#include <Base/ADT/StringMap.h>
#include <Base/Dbg/DebugStream.h>


USING_NAMESPACE

/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN

DBG_STREAM( os_pup, "Puppeteer" );

//------------------------------------------------------------------------------
//!
float
computeFloorHeight( Skeleton* skel, SkeletalAnimation* anim )
{

   Skeleton::Instance skelInst;
   skelInst.skeleton( skel );

   uint n = anim->numPoses();
   if( n == 0 )  return 0;

   float floor = CGConstf::infinity();
   for( uint p = 0; p < n; ++p )
   {
      Puppeteer::setPose( skelInst, anim, p );
      for( uint i = 0; i < skel->numBones(); ++i )
      {
         Vec3f pos = skelInst.globalPosition(i);
         if( pos.y < floor ) floor = pos.y;
      }
   }

   return floor;
}

//------------------------------------------------------------------------------
//!
void
getRootHeightWidth(
   Skeleton* skel,
   Vec3f&    root,
   float&    height,
   float&    width
)
{
   DBG_MSG( os_pup, "getRootHeightWidth" );
   // For now, the root is the position of the first bone having no parent.
   CHECK( skel->bone(0).parent() == Skeleton::ROOT );
   root = skel->bone(0).position();

   AABBox<float> lowerBodyAABB(root);
   uint numLimbs = skel->numLimbs();
   height = 0.0f;
   float totalLimbs = 0.0f;

   for( uint l = 0; l < numLimbs; ++l )
   {
      const Skeleton::Limb& limb = skel->limb(l);
      const Skeleton::Bone& bone = skel->bone( limb.boneID() );

      Mat4f mat      = bone.globalToLocalReferential().globalToLocal();
      Vec3f startPos = mat * Vec3f::zero();
      Vec3f midPos   = mat * bone.endPoint();

      // The mid bone position is below the root.
      if( midPos.y < root.y )
      {
         height += limb.reachRadius();
         lowerBodyAABB |= startPos;
         ++totalLimbs;
      }
   }

   height /= totalLimbs;
   width  = lowerBodyAABB.size(0);
   if( width == 0.0f )
   {
      width = lowerBodyAABB.size(2);
   }
}

//------------------------------------------------------------------------------
//!
void
adjustRoot( const Vector<Puppeteer::PositionalConstraint>& pc, Vec3f& root, Vec3f& dRoot )
{
   // Test if root is contained into limbs spheres.
   Vec3f newRoot;

   bool in = true;
   for( uint i = 0; i < pc.size(); ++i )
   {
      in &= pc[i].sphere().isInside( root );
   }
   // Are we inside all constraints range?
   if( in ) return;

   // 1 sphere.
   if( pc.size() == 1 )
   {
      newRoot = pc[0].sphere().project( root );
   }
   // 2 spheres not intersecting.
   else if( !pc[0].sphere().isOverlapping( pc[1].sphere() ) )
   {
      // Return the projection on the most important one.
      uint s  = pc[0].weight() > pc[1].weight() ? 0 : 1;
      newRoot = pc[s].sphere().project( root );
   }
   else
   {
      // 2 spheres intersecting.
      // Test nearest point on spheres.
      Vec3f projPt[2];
      projPt[0] = pc[0].sphere().project( root );
      projPt[1] = pc[1].sphere().project( root );

      int minPt  = -1;
      float dist = CGConstf::infinity();
      if( pc[1].sphere().isInside( projPt[0] ) )
      {
         minPt = 0;
         dist  = (projPt[0]-root).sqrLength();
      }

      if( pc[0].sphere().isInside( projPt[1] ) )
      {
         float cdist = (projPt[1]-root).sqrLength();
         if( cdist < dist ) minPt = 1;
      }

      // Test circle.
      if( minPt < 0 )
      {
         const Spheref& s0 = pc[0].sphere();
         const Spheref& s1 = pc[1].sphere();

         // Compute center of intersection circle.
         Vec3f dir = s1.center()-s0.center();
         float d2  = dir.sqrLength();
         float t  = (d2 - s1.radius()*s1.radius() + s0.radius()*s0.radius())/(2.0f*d2);
         Vec3f center = s0.center() + dir*t;

         // Compute radius of intersection circle.
         float r = CGM::sqrt( (s0.radius()*s0.radius())-t*t*d2 );

         // Project root on the circle plane.
         Planef plane( dir, center );
         plane.normalize();
         Vec3f pRoot = plane.closest( root );

         // Nearest point on circle.
         newRoot = Spheref( center, r ).project( pRoot );
      }
      else
      {
         newRoot = projPt[minPt];
      }
   }

   // Compute new root and displacement.
   dRoot  += newRoot - root;
   root    = newRoot;
}

//------------------------------------------------------------------------------
//!
void
adjustLimb(
   Skeleton::Instance&                    skelInst,
   const Puppeteer::PositionalConstraint& constraint,
   const Vec3f&                           dRoot
)
{
   Skeleton* skeleton            = skelInst.skeleton();
   int cID                       = constraint.id();
   const Skeleton::Limb& limb    = skeleton->limbFromBone( cID );
   int sID                       = limb.boneID();
   int eID                       = limb.endEffectorID();

   // 1. Compute mid angle.
   float b1Len    = skeleton->bone( sID ).length();
   float b2Len    = skeleton->bone( eID ).length();

   // Current angle.
   Vec3f cdir      = constraint.currentPos()-skelInst.globalPosition(sID);
   float clenSqr   = cdir.sqrLength();
   float ccosAngle = (b1Len*b1Len + b2Len*b2Len - clenSqr) / (2*b1Len*b2Len);
   ccosAngle       = CGM::clamp( ccosAngle, -1.0f, 1.0f );
   float cangle    = CGM::acos( ccosAngle );

   // New angle.
   Vec3f dir      = constraint.endPos()-(skelInst.globalPosition(sID)+dRoot);
   float lenSqr   = dir.sqrLength();
   float cosAngle = (b1Len*b1Len + b2Len*b2Len - lenSqr) / (2*b1Len*b2Len);
   cosAngle       = CGM::clamp( cosAngle, -1.0f, 1.0f );
   float angle    = CGM::acos( cosAngle );

   float diffAngle = cangle-angle;

   Quatf orient;
   switch( skeleton->bone( eID ).dof() )
   {
      case Skeleton::RX: orient = Quatf::eulerX( diffAngle ); break;
      case Skeleton::RY: orient = Quatf::eulerY( diffAngle ); break;
      case Skeleton::RZ: orient = Quatf::eulerZ( diffAngle ); break;
      default: orient = Quatf::eulerX( diffAngle );
   }

   orient = orient * skelInst.localOrientation( eID );

   // 2. Compute start angle.
   // 2.1 target position in local space of first limb bone.
   Vec3f target = skelInst.globalReferential(sID).globalToLocal() * (constraint.endPos()-dRoot);

   // 2.2 Compute new current end effector position in limb space.
   Vec3f localPos = skelInst.localReferential(eID).position();
   Vec3f curPos = Reff( orient, localPos ).toMatrix() * skeleton->bone(eID).endPoint();

   // 2.3 Compute rotation quaternion.
   Quatf rot = Quatf::twoVecs( curPos, target );
   rot = rot * skelInst.localOrientation(sID);

   // 3. Blend with slerp/nlerp.
   //skelInst.localOrientationNoUpdate( sID, skelInst.localOrientation(sID).slerp( rot, constraint.weight() ) );
   //skelInst.localOrientationNoUpdate( eID, skelInst.localOrientation(eID).slerp( orient, constraint.weight() ) );
   skelInst.localOrientationNoUpdate( sID, skelInst.localOrientation(sID).nlerp( rot, constraint.weight() ) );
   skelInst.localOrientationNoUpdate( eID, skelInst.localOrientation(eID).nlerp( orient, constraint.weight() ) );
}

/*==============================================================================
  CLASS Contact
==============================================================================*/
class Contact
{
public:

   /*----- methods -----*/

   Contact() { /* leave uninitialized */ }

   Contact( const uint start, const uint end, const Vec3f& startPos, const Vec3f& endPos ):
      _start( start  ), _end( end ),
      _startPosition( startPos ), _endPosition( endPos )
   { }

   uint  start() const                          { return _start; }
   void  start( const uint start )              { _start = start; }

   uint  end() const                            { return _end; }
   void  end( const uint end )                  { _end = end; }

   uint  duration() const                       { return _end - _start+1; }

   const Vec3f&  startPosition() const          { return _startPosition; }
   void  startPosition( const Vec3f& position ) { _startPosition = position; }

   const Vec3f&  endPosition() const            { return _endPosition; }
   void  endPosition( const Vec3f& position )   { _endPosition = position; }

   Vec3f&  target()                             { return _target; }
   const Vec3f&  target() const                 { return _target; }
   void  target( const Vec3f& t )               { _target = t; }

protected:

   /*----- data members -----*/

   uint   _start;          //!< The pose index where the contact starts.
   uint   _end;            //!< The pose index where the contact ends.
   Vec3f  _startPosition;  //!< The position where the contact starts in the source animation.
   Vec3f  _endPosition;    //!< The position where the contact ends in the source animation.
   Vec3f  _target;         //!< The position where the contact should happen in the destination animation.

}; //class Contact


/*==============================================================================
  CLASS ContactList
==============================================================================*/
class ContactList
{
public:

   typedef Vector<Contact>  ContactContainer;

   /*----- methods -----*/

   ContactList( const int boneID = Skeleton::ROOT, const float influenceRadius = 0.0f ):
      _boneID(boneID), _radius(influenceRadius) { }

   int  boneID() const                             { return _boneID; }
   void  boneID( const int b )                     { _boneID = b; }

   float  influenceRadius() const                  { return _radius; }
   void   influenceRadius( const float r )         { _radius = r; }

   void  add( const Contact& contact )             { _contacts.pushBack(contact); }

   uint  numContacts() const                       { return (uint)_contacts.size(); }

   bool  empty() const                             { return _contacts.empty(); }

         Contact&  contact( const uint idx )       { return _contacts[idx]; }
   const Contact&  contact( const uint idx ) const { return _contacts[idx]; }

         ContactContainer&  contacts()             { return _contacts; }
   const ContactContainer&  contacts() const       { return _contacts; }


protected:

   /*----- data members -----*/

   int               _boneID;    //!< The bone ID unto which all of the contacts refer to.
   float             _radius;    //!< The maximum radius inside which the bone can affect the contact importance.
   ContactContainer  _contacts;  //!< The various contacts.

}; //class ContactList


//------------------------------------------------------------------------------
//!
void
findContactCandidates(
   Skeleton*               skel,
   SkeletalAnimation*      anim,
   const float             maxVelocityDetectionThreshold,
   const float             maxPositionExtensionThreshold,
   const uint              minNumFramesThreshold,
   const Vector<int>&      boneCandidates,
   Vector< ContactList >&  contactLists
)
{
   DBG_BLOCK(
      os_pup,
      "findContactCandidates("
      << skel
      << ", "
      << anim
      << ", "
      << maxVelocityDetectionThreshold
      << ", "
      << maxPositionExtensionThreshold
      << ", "
      << minNumFramesThreshold
      << ")"
   );

   const uint nBones    = uint(boneCandidates.size());
   const uint nPoses    = anim->numPoses();
   const float maxVelDetSq = maxVelocityDetectionThreshold * maxVelocityDetectionThreshold;
   const float maxPosExtSq = maxPositionExtensionThreshold * maxPositionExtensionThreshold;
   CHECK(nPoses >= 2);

   Skeleton::Instance skelInst;
   skelInst.skeleton( skel );

   Vector<Contact>  activeContacts( nBones );
   Vector<Vec3f>    prevPosition(nBones);
   Vector<Vec3f>    prevVelocity(nBones);
   Vector<AABBoxf>  noiseBoxes(nBones);

   // Initialize contact lists.
   contactLists.resize(nBones);  // Initialized to Skeleton::ROOT (inactive state)

   // Initialize positions.
   Puppeteer::setPose( skelInst, anim, (uint)0 );
   for( uint i = 0; i < nBones; ++i )
   {
      int b           = boneCandidates[i];
      prevPosition[i] = skelInst.globalReferential(b).toMatrix() * skel->bone(b).endPoint();
   }

   // Initialize velocities.
   Puppeteer::setPose( skelInst, anim, (uint)1 );
   for( uint i = 0; i < nBones; ++i )
   {
      int b           = boneCandidates[i];
      Vec3f position  = skelInst.globalReferential(b).toMatrix() * skel->bone(b).endPoint();
      prevVelocity[i] = position - prevPosition[i];
      prevPosition[i] = position;
   }

   // Loop over all of the frames to find contact candidates.
   for( uint p = 2; p < nPoses; ++p )
   {
      Puppeteer::setPose( skelInst, anim, p );

      for( uint i = 0; i < nBones; ++i )
      {
         int b                    = boneCandidates[i];
         ContactList& contactList = contactLists[i];
         Contact& contact         = activeContacts[i];
         AABBoxf& noiseBox        = noiseBoxes[i];

         Vec3f position = skelInst.globalReferential(b).toMatrix() * skel->bone(b).endPoint();
         Vec3f velocity = position - prevPosition[i];

         if( contactList.boneID() == Skeleton::ROOT )
         {
            // Contact is inactive; check if we can activate it.
            if( velocity.sqrLength() <= maxVelDetSq )
            {
               contactList.boneID( b );
               contact.start( p - 1 );
               contact.startPosition( prevPosition[i] );
               contact.target( position + prevPosition[i] );
               noiseBox.set( position );
            }
         }
         else
         {
            // Contact is active; try to continue it.
            if( (position - noiseBox.center()).sqrLength() <= maxPosExtSq )
            {
               // Noise did it... keep growing the noise box.
               contact.target() += position;
               noiseBox |= position;
            }
            else
            {
               // No longer matches, so register the range, and deactivate.
               contact.end( p - 1 );
               if( contact.duration() >= minNumFramesThreshold )
               {
                  contact.endPosition( prevPosition[i] );
                  contact.target() *= 1.0f / contact.duration();
                  contactLists[i].add( contact );
               }
               contactList.boneID( Skeleton::ROOT );
            }
         }

         // Update values
         prevVelocity[i] = velocity;
         prevPosition[i] = position;
      }
   }

   // Terminate outstanding active contacts.
   for( uint i = 0; i < nBones; ++i )
   {
      ContactList& contactList = contactLists[i];
      Contact& contact         = activeContacts[i];

      if( contactList.boneID() != Skeleton::ROOT )
      {
         // Active, so register the range, and deactivate.
         contact.end( nPoses - 1 );
         if( contact.duration() >= minNumFramesThreshold )
         {
            contact.endPosition( prevPosition[i] );
            contact.target() *= 1.0f / contact.duration();
            contactLists[i].add( contact );
         }
      }

      // Make sure that the boneIDs are all set back to their proper values.
      contactList.boneID( boneCandidates[i] );
   }
}


/*==============================================================================
  CLASS ConstraintConverter
==============================================================================*/
class ConstraintConverter
{
public:

   /*----- methods -----*/

   ConstraintConverter( const Skeleton::Instance& skelInst, const Vector<ContactList>& contactLists ):
      _skelInst(skelInst), _contactLists(contactLists) { /*_constraints.reserve(_contactLists.size());*/ }

   const Skeleton::Instance&  skeletonInstance() const                 { return _skelInst; }

   const Vector<ContactList>&  contactLists() const                    { return _contactLists; }

   const Vector<Puppeteer::PositionalConstraint>&  constraints() const { return _constraints; }

   const Vector<Puppeteer::PositionalConstraint>&  computePoseConstraints( const uint poseID );

protected:

   /*----- data members -----*/

   const Skeleton::Instance&                _skelInst;      //!< Skeleton instance to compute the weights from.
   const Vector<ContactList>&               _contactLists;  //!< Contact infos.
   Vector<Puppeteer::PositionalConstraint>  _constraints;   //!< The positional contraints.

   /*----- methods -----*/
   float  computeWeight( const Vec3f& actualPosition, const Vec3f& desiredPosition, const float influenceRadius ) const;

   Vec3f  getBonePosition( const int boneID ) const
   {
      // Slight optimization would be to use the start of the next bone (if it exists).
      return _skelInst.globalReferential(boneID).toMatrix() * _skelInst.skeleton()->bone(boneID).endPoint();
   }

private:
}; //class ConstraintConverter

//------------------------------------------------------------------------------
//!
const Vector<Puppeteer::PositionalConstraint>&
ConstraintConverter::computePoseConstraints( const uint poseID )
{
   _constraints.clear();
   for( uint i = 0; i < _contactLists.size(); ++i )
   {
      const ContactList&  contactList = _contactLists[i];
      if( contactList.empty() )  continue;

      const Contact* first = NULL;
      const Contact* next  = NULL;

      // 1. Find first and last contacts.
      ContactList::ContactContainer::ConstIterator curC = contactList.contacts().begin();
      ContactList::ContactContainer::ConstIterator endC = contactList.contacts().end();
      for( ; curC != endC; ++curC )
      {
         if( (*curC).start() <= poseID )
         {
            // The first is the latest one which happens before the specified poseID.
            // We keep assigning until we go beyond that point.
            first = &(*curC);
         }
         else
         {
            // The next is the first one which happens after the specified poseID.
            // We stop as soon as we find such a case.
            next = &(*curC);
            break;
         }
      }

#define ADD_CONSTRAINT(p, w) \
   if( w > 0.0f ) \
   { \
      _constraints.pushBack( \
         Puppeteer::PositionalConstraint( \
            p, w, contactList.boneID() \
         ) \
      ); \
   }

      // 2. Handle the various cases.
      if( first != NULL )
      {
         if( poseID <= first->end() )
         {
            // The contact is currently active, so use weight of 1.
            ADD_CONSTRAINT( first->target(), 1.0f );
            continue;
         }

         Vec3f bonePos = getBonePosition( contactList.boneID() );
         float firstW  = computeWeight( bonePos, first->endPosition(), contactList.influenceRadius() );

         if( next != NULL )
         {
            // Sitting between 2 candidates (pick the best one).
            float nextW = computeWeight( bonePos, next->startPosition(), contactList.influenceRadius() );
            if( firstW > nextW )
            {
               ADD_CONSTRAINT( first->target(), firstW );
            }
            else
            {
               ADD_CONSTRAINT( next->target(), nextW );
            }
         }
         else
         {
            // End of the animation (only a first).
            ADD_CONSTRAINT( first->target(), firstW );
         }
      }
      else
      {
         // Beginning of the animation (only a last).
         // (guaranteed since we checked for emptiness at the beginning).
         Vec3f bonePos = getBonePosition( contactList.boneID() );
         float nextW   = computeWeight( bonePos, next->startPosition(), contactList.influenceRadius() );
         ADD_CONSTRAINT( next->target(), nextW );
      }
      // else, none are valid...
   }

#undef ADD_CONSTRAINT

   return _constraints;
}

//------------------------------------------------------------------------------
//!
float
ConstraintConverter::computeWeight(
   const Vec3f& actualPosition,
   const Vec3f& desiredPosition,
   float        influenceRadius
) const
{
   float d = (desiredPosition-actualPosition).length() / influenceRadius;
   if( d < 1.0f )
   {
      // 2d^3 - 3d^2 + 1 = (2d - 3)d^2 + 1
      return (2*d - 3)*d*d + 1;
   }
   else
   {
      return 0.0f;
   }
}


UNNAMESPACE_END


/*==============================================================================
  CLASS Puppeteer
==============================================================================*/

//------------------------------------------------------------------------------
//!
Puppeteer::Puppeteer()
{
}

//------------------------------------------------------------------------------
//!
Puppeteer::~Puppeteer()
{
}

//------------------------------------------------------------------------------
//! Moves the animation into the specified referential.
RCP<SkeletalAnimation>
Puppeteer::applyReferential(
   SkeletalAnimation* animation,
   const Reff&        ref
)
{
   DBG_BLOCK( os_pup, "Puppeteer::applyReferential(" << animation << ", " << ref << ")" );
   RCP<SkeletalAnimation> anim = animation->clone();
   //anim->makeRelative();
   anim->makeAbsolute();
   uint numPoses = anim->numPoses();
   for( uint p = 0; p < numPoses; ++p )
   {
      SkeletalPose* pose = anim->pose( p );
      pose->referential( ref * pose->referential() );
   }
   //anim->makeRelative();
   //anim->velocity( anim->velocity() )
   return anim;
}

//------------------------------------------------------------------------------
//!
RCP<SkeletalAnimation>
Puppeteer::blend(
   SkeletalAnimation* startAnim,
   SkeletalAnimation* endAnim
)
{
   DBG_BLOCK( os_pup, "Puppeteer::blend(" << startAnim << ", " << endAnim << ")" );
   RCP<SkeletalAnimation> anim = new SkeletalAnimation();
   anim->skeleton( startAnim->skeleton() );

   startAnim->makeRelative();
   endAnim->makeRelative();

   // Compute duration, rate and number of poses.
   float duration = (startAnim->duration() + endAnim->duration()) * 0.5f;
   float rate     = CGM::max( startAnim->rate(), endAnim->rate() );
   uint numPoses  = uint(rate*duration) + 1;

   anim->reservePoses( numPoses );
   anim->rate( rate );
   anim->velocity( (startAnim->velocity() + endAnim->velocity())*0.5f );
   anim->offset( (startAnim->offset() + endAnim->offset())*0.5f );

   // Compute poses.
   SkeletalPose* sp0;
   SkeletalPose* sp1;
   SkeletalPose* ep0;
   SkeletalPose* ep1;

   for( uint p = 0; p < numPoses; ++p )
   {
      float t = float(p) / float(numPoses-1);
      float st;
      float et;

      startAnim->getPosesClamped( startAnim->duration() * t, sp0, sp1, st );
      endAnim->getPosesClamped( endAnim->duration() * t, ep0, ep1, et );

      const SkeletalPose::BoneContainer& sb0 = sp0->bones();
      const SkeletalPose::BoneContainer& sb1 = sp1->bones();
      const SkeletalPose::BoneContainer& eb0 = ep0->bones();
      const SkeletalPose::BoneContainer& eb1 = ep1->bones();

      //Reff sref = sp0->referential().slerp( sp1->referential(), st );
      //Reff eref = ep0->referential().slerp( ep1->referential(), et );
      //Reff ref  = sref.slerp( eref, t );
      Reff sref = sp0->referential().nlerp( sp1->referential(), st );
      Reff eref = ep0->referential().nlerp( ep1->referential(), et );
      Reff ref  = sref.nlerp( eref, t );

      SkeletalPose* pose = anim->addPose( ref );
      pose->reserveBones( uint(sb0.size()) );
      for( uint i = 0; i < sb0.size(); ++i )
      {
         //Quatf sorient = sb0[i].slerp( sb1[i], st );
         //Quatf eorient = eb0[i].slerp( eb1[i], et );
         //Quatf orient  = sorient.slerp( eorient, t );
         Quatf sorient = sb0[i].nlerp( sb1[i], st );
         Quatf eorient = eb0[i].nlerp( eb1[i], et );
         Quatf orient  = sorient.nlerp( eorient, t );
         pose->addBone( orient );
      }
   }
   return anim;
}

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
void
Puppeteer::blendFrame(
   SkeletalAnimation* animA,
   float              timeA,
   SkeletalAnimation* animB,
   float              timeB,
   float              factor,
   SkeletalPose&      dstPose
)
{
   DBG_BLOCK( os_pup, "Puppeteer::blendFrame(" << animA << ", " << timeA << ", " << animB << ", " << timeB << ", " << factor << ")" );

   animA->makeRelative();
   animB->makeRelative();

   // Compute poses.
   SkeletalPose* pA0;
   SkeletalPose* pA1;
   float tA;
   SkeletalPose* pB0;
   SkeletalPose* pB1;
   float tB;

   animA->getPoses( timeA, pA0, pA1, tA );
   animB->getPoses( timeB, pB0, pB1, tB );

   DBG_MSG( os_pup, "A: " << timeA << " --> " << tA );
   DBG_MSG( os_pup, "B: " << timeB << " --> " << tB );

   const SkeletalPose::BoneContainer& bA0 = pA0->bones();
   const SkeletalPose::BoneContainer& bA1 = pA1->bones();
   const SkeletalPose::BoneContainer& bB0 = pB0->bones();
   const SkeletalPose::BoneContainer& bB1 = pB1->bones();

   //Reff refA = pA0->referential().slerp( pA1->referential(), tA );
   //Reff refB = pB0->referential().slerp( pB1->referential(), tB );
   //Reff ref  = refA.slerp( refB, factor );
   Reff refA = pA0->referential().nlerp( pA1->referential(), tA );
   Reff refB = pB0->referential().nlerp( pB1->referential(), tB );
   Reff ref  = refA.nlerp( refB, factor );

   dstPose.referential( ref );
   SkeletalPose::BoneContainer& dstBones = dstPose.bones();

   CHECK( bA0.size() == bB0.size() );
   CHECK( bA0.size() == dstBones.size() );
   const uint nBones = uint(dstBones.size());
   for( uint i = 0; i < nBones; ++i )
   {
      //Quatf sorient = bA0[i].slerp( bA1[i], tA );
      //Quatf eorient = bB0[i].slerp( bB1[i], tB );
      //Quatf orient  = sorient.slerp( eorient, factor );
      Quatf sorient = bA0[i].nlerp( bA1[i], tA );
      Quatf eorient = bB0[i].nlerp( bB1[i], tB );
      Quatf orient  = sorient.nlerp( eorient, factor );
      dstBones[i] = orient;
   }
}

//------------------------------------------------------------------------------
//! Blend between two poses.
void
Puppeteer::blendPoses(
   SkeletalPose*  pose0,
   SkeletalPose*  pose1,
   float          factor,
   SkeletalPose&  dstPose
)
{
   CHECK( pose0 );
   CHECK( pose1 );

   //Reff ref = pose0->referential().slerp( pose1->referential(), factor );
   Reff ref = pose0->referential().nlerp( pose1->referential(), factor );

   dstPose.referential( ref );

   const SkeletalPose::BoneContainer& bones0   = pose0->bones();
   const SkeletalPose::BoneContainer& bones1   = pose1->bones();
         SkeletalPose::BoneContainer& dstBones = dstPose.bones();
   CHECK( bones0.size() == bones1.size() );
   CHECK( bones0.size() == dstBones.size() );
   const uint nBones = uint(dstBones.size());
   for( uint i = 0; i < nBones; ++i )
   {
      //Quatf orient = bones0[i].slerp( bones1[i], factor );
      Quatf orient = bones0[i].nlerp( bones1[i], factor );
      dstBones[i] = orient;
   }
}

//------------------------------------------------------------------------------
//! Blend between 4 poses (pose0 and pose1), and stores the result in a
//! preallocated pose of the right size (dstPose).
//! It performs trilinear interpolation:
//!   lerp( lerp(p0, p1, f01), lerp(p2, p3, f23), f01_23 )
void
Puppeteer::blendPoses(
   SkeletalPose*  p0,
   SkeletalPose*  p1,
   SkeletalPose*  p2,
   SkeletalPose*  p3,
   float          f01,
   float          f23,
   float          f01_23,
   SkeletalPose&  dstPose
)
{
   CHECK( p0 );
   CHECK( p1 );
   CHECK( p2 );
   CHECK( p3 );
   CHECK( dstPose.bones().size() == p0->bones().size() );
   CHECK( p0->bones().size() == p1->bones().size() );
   CHECK( p1->bones().size() == p2->bones().size() );
   CHECK( p2->bones().size() == p3->bones().size() );

   //Reff r0 = p0->referential().slerp( p1->referential(), f01 );
   //Reff r1 = p2->referential().slerp( p3->referential(), f23 );
   //dstPose.referential( r0.slerp( r1, blend ) );
   Reff r0 = p0->referential().nlerp( p1->referential(), f01 );
   Reff r1 = p2->referential().nlerp( p3->referential(), f23 );
   dstPose.referential( r0.nlerp( r1, f01_23 ) );

   for( uint i = 0; i < dstPose.numBones(); ++i )
   {
      //Quatf q0 = p0->bones()[i].slerp( p1->bones()[i], f01 );
      //Quatf q1 = p2->bones()[i].slerp( p2->bones()[i], f23 );
      //dstPose.bones()[i] = q0.slerp( q1, f01_23 );
      Quatf q0 = p0->bones()[i].nlerp( p1->bones()[i], f01 );
      Quatf q1 = p2->bones()[i].nlerp( p2->bones()[i], f23 );
      dstPose.bones()[i] = q0.nlerp( q1, f01_23 );
   }
}

//------------------------------------------------------------------------------
//! Append anim2 at the end of anim1 (no blending whatsoever).
//! If rates differ, the rate of anim1 is used for the final animation.
RCP<SkeletalAnimation>
Puppeteer::concatenate(
   SkeletalAnimation* anim1,
   SkeletalAnimation* anim2
)
{
   DBG_BLOCK( os_pup, "Puppeteer::concatenate(" << anim1 << ", " << anim2 << ")" );
   anim1->makeRelative();
   anim2->makeRelative();
   RCP<SkeletalAnimation> anim = anim1->clone();
   if( anim2->numPoses() > 0 )
   {
      RCP<SkeletalAnimation> animToAppend;
      if( anim1->rate() == anim2->rate() )
      {
         animToAppend = anim2;
      }
      else
      {
         animToAppend = resample( anim2, anim1->rate() );
      }
      // Remove the first animation's last frame (a copy of frame 0).
      uint np1 = anim->numPoses() - 1;
      anim->removePose( np1 );
      // Add the second animation's poses.
      uint np2 = animToAppend->numPoses();
      anim->reservePoses( np1 + np2 );
      for( uint p = 0; p < np2; ++p )
      {
         anim->addPose( animToAppend->pose(p)->clone().ptr() );
      }
   }
   return anim;
}

//------------------------------------------------------------------------------
//! Retrieve a portion of anim between the specified start and end positions.
RCP<SkeletalAnimation>
Puppeteer::cut(
   SkeletalAnimation* srcAnim,
   uint               startPose,
   uint               endPose
)
{
   DBG_BLOCK( os_pup, "Puppeteer::cut(" << srcAnim << ", " << startPose << ", " << endPose << ")" );
   RCP<SkeletalAnimation> dstAnim = srcAnim->clone( startPose, endPose );
   return dstAnim;
}

//------------------------------------------------------------------------------
//!
RCP<SkeletalAnimation>
Puppeteer::cycle( SkeletalAnimation* animation, float fraction )
{
   DBG_BLOCK( os_pup, "Puppeteer::cycle(" << animation << ", " << fraction << ")" );
   RCP<SkeletalAnimation> anim = animation->clone();
   anim->makeRelative();

   fraction      = CGM::clamp( fraction, 0.0f, 1.0f );
   uint offset   = uint((anim->numPoses()-1)*fraction);
   uint numPoses = anim->numPoses()-offset;

   // Compute delta transformations.
   SkeletalPose* sp = anim->pose(0);
   SkeletalPose* ep = anim->pose( anim->numPoses()-1 );
   Reff dref = sp->referential() * ep->referential().getInversed();

   SkeletalPose::BoneContainer& b0 = sp->bones();
   SkeletalPose::BoneContainer& b1 = ep->bones();

   Vector<Quatf> drot( b0.size() );

   for( uint i = 0; i < b0.size(); ++i )
   {
      drot[i] = b0[i] * b1[i].getInversed();
   }

   // Compute poses.
   for( uint p = 0; p < numPoses; ++p )
   {
      SkeletalPose* p0 = anim->pose( offset+p );
      SkeletalPose::BoneContainer& b0 = p0->bones();

      float t = float(p) / float(numPoses-1);

      //Reff ref = p0->referential().slerp( dref*p0->referential(), t );
      Reff ref = p0->referential().nlerp( dref*p0->referential(), t );
      p0->referential( ref );
      for( uint i = 0; i < b0.size(); ++i )
      {
         //b0[i] = b0[i].slerp( drot[i]*b0[i], t );
         b0[i] = b0[i].nlerp( drot[i]*b0[i], t );
      }
   }

   anim->cyclic( true );  // We are now cyclic.

   return anim;
}

//------------------------------------------------------------------------------
//! Removes the specified axis from the rotation of the root.
RCP<SkeletalAnimation>
Puppeteer::dropRotation(
   SkeletalAnimation* animation,
   const Vec3f&       axis
)
{
   DBG_BLOCK( os_pup, "Puppeteer::dropRotation(" << animation << ", " << axis << ")" );
   RCP<SkeletalAnimation> anim = animation->clone();
   Vec3f pAxis = Vec3f::perpendicular( axis );
   const uint numPoses = anim->numPoses();
   StdOut << pAxis << nl;
   for( uint p = 0; p < numPoses; ++p )
   {
      SkeletalPose* pose = anim->pose( p );
      const Quatf& q     = pose->orientation();
      Vec3f src          = q * pAxis;
      Vec3f dst          = src - src.projectOnto( axis );
      Quatf r            = Quatf::twoVecs( src, dst );
      pose->orientation( r );
      StdOut << p << ": " << q << "," << src << "," << dst << "," << r << nl;
      //pose->orientation( Quatf::identity() );
   }
   return anim;
}

//------------------------------------------------------------------------------
//! Resample the animation sequence in order to reach the specific rate.
//! It does not change the duration, which can sometimes become inconsistent.
RCP<SkeletalAnimation>
Puppeteer::resample(
   SkeletalAnimation* srcAnim,
   float              rate,
   bool               forceDuration
)
{
   DBG_BLOCK( os_pup, "Puppeteer::resample(" << srcAnim << ", " << rate << ", " << forceDuration << ")" );
   RCP<SkeletalAnimation> dstAnim = new SkeletalAnimation();
   dstAnim->cyclic( srcAnim->cyclic() );
   dstAnim->skeleton( srcAnim->skeleton() );
   srcAnim->makeRelative();

   uint n = uint( rate * srcAnim->duration() ); // not putting the +1, and instead looping until <=n

   if( forceDuration )
   {
      // Tweak rate to meet duration.
      rate = float(n) / srcAnim->duration();
   }

   dstAnim->reservePoses( n + 1 );
   dstAnim->rate( rate );
   dstAnim->velocity( srcAnim->velocity() );
   dstAnim->offset( srcAnim->offset() );

   // Compute poses.
   float timeFactor = srcAnim->duration() / float(n);
   for( uint p = 0; p <= n; ++p )
   {
      float animTime = float(p) * timeFactor;

      SkeletalPose*  srcPose0;
      SkeletalPose*  srcPose1;
      float  srcPoseFactor;

      srcAnim->getPoses( animTime, srcPose0, srcPose1, srcPoseFactor );
      if( srcPoseFactor != 0.0f )
      {
         // Interpolate between the 2 poses.
         // 1. Create new pose with interpolated referential.
         //Reff ref = srcPose0->referential().slerp( srcPose1->referential(), srcPoseFactor );
         Reff ref = srcPose0->referential().nlerp( srcPose1->referential(), srcPoseFactor );
         SkeletalPose* pose = dstAnim->addPose( ref );
         // 2. Add every interpolated bone.
         uint nBones = uint(srcPose0->bones().size());
         pose->reserveBones( nBones );
         for( uint i = 0; i < nBones; ++i )
         {
            //pose->addBone( srcPose0->bones()[i].slerp( srcPose1->bones()[i], srcPoseFactor ) );
            pose->addBone( srcPose0->bones()[i].nlerp( srcPose1->bones()[i], srcPoseFactor ) );
         }
      }
      else
      {
         // No interpolation necessary, just copy.
         dstAnim->addPose( srcPose0->clone().ptr() );
      }
   }

   CHECK( !forceDuration || CGM::equal(srcAnim->duration(), dstAnim->duration()) );

   return dstAnim;
}

//------------------------------------------------------------------------------
//!
RCP<SkeletalAnimation>
Puppeteer::retarget(
   SkeletalAnimation* srcAnim,
   Skeleton*          dstSkel
)
{
   return retarget( srcAnim->skeleton(), dstSkel, srcAnim, false );
}

//------------------------------------------------------------------------------
//!
RCP<SkeletalAnimation>
Puppeteer::retarget(
   Skeleton*          srcSkel,
   Skeleton*          dstSkel,
   SkeletalAnimation* srcAnim,
   bool               constraints,
   const Vec3f&       userScale
)
{
   DBG_BLOCK( os_pup, "Puppeteer::retarget(" << srcSkel << ", " << dstSkel << ", " << srcAnim << ")" );
   if( (srcSkel == 0) || (dstSkel == 0) || (srcAnim == 0) )
   {
      // Safeguarding.
      return NULL;
   }

   // Compute scaling factors.
   Vec3f srcRoot;
   Vec3f dstRoot;
   float srcH, srcW;
   float dstH, dstW;
   getRootHeightWidth( srcSkel, srcRoot, srcH, srcW );
   getRootHeightWidth( dstSkel, dstRoot, dstH, dstW );

   float rootScaleY = dstH / srcH;
   float stepScale  = dstW / srcW;


   // Detect step constraints. (might need extra parameters to help this stage)
   // 1. Thresholds
   Vector< ContactList >  contactLists;
   float maxVelDetThreshold   = 0.1f * srcH / srcAnim->rate();
   float maxPosExtThreshold   = 0.05f * srcH;
   uint minNumFramesThreshold = 5;

   // 2. Bones candidates.
   Vector<int> boneCandidates;
   if( constraints )
   {
      for( uint i = 0; i < srcSkel->numLimbs(); ++i )
      {
         uint nID = srcSkel->limb(i).nodeID();
         Skeleton::DepthFirstIterator it = srcSkel->depthFirst( nID );
         for( uint j = 0; j < 3 && it(); ++j, ++it )
         {
            nID = *it;
         }
         boneCandidates.pushBack( srcSkel->node(nID).boneID() );
      }
   }


   // 3. Contacts.
   findContactCandidates(
      srcSkel, srcAnim,
      maxVelDetThreshold,
      maxPosExtThreshold,
      minNumFramesThreshold,
      boneCandidates,
      contactLists
   );

   // Set a default (constant) influence radius for all contact candidates.
   float influenceRadius = srcH * 0.3f;
   for( uint cl = 0; cl < contactLists.size(); ++cl )
   {
      ContactList& contactList = contactLists[cl];
      contactList.influenceRadius( influenceRadius );
   }

   // Retarget animation.
   // 1. Clone animation.
   RCP<SkeletalAnimation> dstAnim = srcAnim->clone();
   dstAnim->skeleton( dstSkel );

   // 2. Rescale root based on lower body height and readjust bone angle
   // to take into account the difference in the canonical pose.

   // Compute transformation from the src pose to the dst pose.
   Vector<Quatf> convert(srcSkel->numBones());
   for( uint b = 0; b < srcSkel->numBones(); ++b )
   {
      convert[b] = dstSkel->bone(b).orientation() * srcSkel->bone(b).orientation().getInversed();
   }


   Vec3f floorHeight( 0.0f, computeFloorHeight( srcSkel, srcAnim ), 0.0f );
   Vec3f scale = userScale * Vec3f( stepScale, rootScaleY, rootScaleY );
   for( uint p = 0; p < dstAnim->numPoses(); ++p )
   {
      // Adjust root.
      SkeletalPose& pose = *dstAnim->pose(p);
      Mat4f mat      = pose.orientation().toMatrix();
      Vec3f goalRoot = ( pose.position() + mat*srcRoot - floorHeight )*scale;
      Vec3f curRoot  = mat * dstRoot;
      pose.position( goalRoot-curRoot );

      // Adjust bones angles.
      for( uint b = 0; b < pose.bones().size(); ++b )
      {
         pose.bones()[b] = convert[b]*pose.bones()[b];
      }
   }

   // 3. Compute targets constraints.
   for( uint cl = 0; cl < contactLists.size(); ++cl )
   {
      ContactList& contactList = contactLists[cl];
      for( uint c = 0; c < contactList.numContacts(); ++c )
      {
         Contact& contact = contactList.contact(c);
         contact.target( (contact.target() - floorHeight)*scale );
      }
   }

   // 4. Retarget each frame of animation by imposing constraints on limbs
   // positions.
   Skeleton::Instance srcInst;
   srcInst.skeleton( srcSkel );
   Skeleton::Instance dstInst;
   dstInst.skeleton( dstSkel );
   ConstraintConverter  cc( srcInst, contactLists );
   for( uint p = 0; p < dstAnim->numPoses(); ++p )
   {
      setPose( srcInst, srcAnim, p ); // The ConstraintConverter needs the proper positions.
      setPose( dstInst, dstAnim.ptr(), p );
      resolveConstraints( dstInst, cc.computePoseConstraints(p) );
      dstAnim->setPose( p, dstInst.pose(), dstInst.offset() );
   }

   // 5. Scale animation speed based on relative scales.
   dstAnim->velocity( dstAnim->velocity() * scale );

   return dstAnim;
}

//------------------------------------------------------------------------------
//! Reverses the animation.
RCP<SkeletalAnimation>
Puppeteer::reverse( SkeletalAnimation* srcAnim )
{
   DBG_BLOCK( os_pup, "Puppeteer::reverse(" << srcAnim << ")" );
   const uint numPoses = srcAnim->numPoses();
   RCP<SkeletalAnimation> dstAnim = new SkeletalAnimation();
   dstAnim->skeleton( srcAnim->skeleton() );
   dstAnim->rate( srcAnim->rate() );
   dstAnim->reservePoses( numPoses );
   for( uint p = 0; p < numPoses; ++p )
   {
      SkeletalPose* srcPose = srcAnim->pose( numPoses - 1 - p );
      dstAnim->addPose( srcPose->clone().ptr() );
   }
   return dstAnim;
}

//------------------------------------------------------------------------------
//!
RCP<SkeletalAnimation>
Puppeteer::transition(
   SkeletalAnimation* startAnim,
   SkeletalAnimation* endAnim,
   float              duration
)
{
   DBG_BLOCK( os_pup, "Puppeteer::transition(" << startAnim << ", " << endAnim << ", " << duration << ")" );
   RCP<SkeletalAnimation> anim = new SkeletalAnimation();
   anim->skeleton( startAnim->skeleton() );

   startAnim->makeRelative();
   endAnim->makeRelative();

   float rate    = CGM::max( startAnim->rate(), endAnim->rate() );
   uint numPoses = uint(rate*duration) + 1;

   anim->reservePoses( numPoses );
   anim->rate( rate );
   anim->velocity( Vec3f(0.0f) );
   anim->offset( (startAnim->offset() + endAnim->offset())*0.5f );


   // Compute poses.
   SkeletalPose* p0 = startAnim->pose( startAnim->numPoses()-1 );
   SkeletalPose* p1 = endAnim->pose(0);

   const SkeletalPose::BoneContainer& b0 = p0->bones();
   const SkeletalPose::BoneContainer& b1 = p1->bones();

   for( uint p = 0; p < numPoses; ++p )
   {
      float t = float(p) / float(numPoses-1);

      //Reff ref = p0->referential().slerp( p1->referential(), t );
      Reff ref = p0->referential().nlerp( p1->referential(), t );
      SkeletalPose* pose = anim->addPose( ref );
      pose->reserveBones( uint(b0.size()) );
      for( uint i = 0; i < b0.size(); ++i )
      {
         //pose->addBone( b0[i].slerp( b1[i], t ) );
         pose->addBone( b0[i].nlerp( b1[i], t ) );
      }
   }

   return anim;
}

//------------------------------------------------------------------------------
//! Create a transition frame from one animation (startAnim) to another one
//! (endAnim) using the specified factor.
void
Puppeteer::transitionFrame(
   SkeletalAnimation* startAnim,
   SkeletalAnimation* endAnim,
   float              factor,
   SkeletalPose&      dstPose
)
{
   DBG_BLOCK( os_pup, "Puppeteer::transitionFrame(" << startAnim << ", " << endAnim << ", " << factor << ")" );

   startAnim->makeRelative();
   endAnim->makeRelative();

   // Compute poses.
   SkeletalPose* p0 = startAnim->pose( startAnim->numPoses()-1 );
   SkeletalPose* p1 = endAnim->pose(0);

   const SkeletalPose::BoneContainer& b0 = p0->bones();
   const SkeletalPose::BoneContainer& b1 = p1->bones();
         SkeletalPose::BoneContainer& db = dstPose.bones();

   //Reff ref = p0->referential().slerp( p1->referential(), factor );
   Reff ref = p0->referential().nlerp( p1->referential(), factor );
   dstPose.referential( ref );
   CHECK( b0.size() == b1.size() );
   CHECK( b0.size() == db.size() );
   const uint numBones = (uint)db.size();
   for( uint i = 0; i < numBones; ++i )
   {
      //db[i] = b0[i].slerp( b1[i], factor );
      db[i] = b0[i].nlerp( b1[i], factor );
   }
}

//------------------------------------------------------------------------------
//!
void
Puppeteer::setPose(
   Skeleton::Instance& skelInst,
   const Vector<Reff>& pose
)
{
   for( uint i = 0; i < skelInst._localRefs.size(); ++i )
   {
      skelInst._localRefs[i] = pose[i];
   }

   skelInst.updateGlobalTransforms();
}

//------------------------------------------------------------------------------
//!
void
Puppeteer::setPose(
   Skeleton::Instance&  skelInst,
   SkeletalPose*        pose
)
{
   const SkeletalPose::BoneContainer& bones = pose->bones();
   skelInst.offset( pose->referential() );
   for( uint i = 0; i < skelInst._localRefs.size(); ++i )
   {
      // NOTE: Do we need to set position back to zero?
      skelInst._localRefs[i].orientation( bones[i] );
   }

   skelInst.updateGlobalTransforms();
}

//------------------------------------------------------------------------------
//!
void
Puppeteer::setPose(
   Skeleton::Instance&  skelInst,
   SkeletalAnimation*   anim,
   const uint           poseID
)
{
   const SkeletalPose::BoneContainer& bones = anim->pose(poseID)->bones();
   CHECK( skelInst._localRefs.size() == bones.size() );

   CHECK( poseID < anim->numPoses() );
   skelInst.offset( anim->pose(poseID)->referential() );

   for( uint i = 0; i < skelInst._localRefs.size(); ++i )
   {
      skelInst._localRefs[i].orientation( bones[i] );
   }

   skelInst.updateGlobalTransforms();
}

//------------------------------------------------------------------------------
//!
uint
Puppeteer::setPose(
   Skeleton::Instance&  skelInst,
   SkeletalAnimation*   anim,
   float                time
)
{
   SkeletalPose* p0;
   SkeletalPose* p1;
   float t;

   uint pose = anim->getPoses( time, p0, p1, t );

   const SkeletalPose::BoneContainer& b0 = p0->bones();
   const SkeletalPose::BoneContainer& b1 = p1->bones();

   //skelInst.offset( p0->referential().slerp( p1->referential(), t ) );
   skelInst.offset( p0->referential().nlerp( p1->referential(), t ) );

   for( uint i = 0; i < skelInst._localRefs.size(); ++i )
   {
      //skelInst._localRefs[i].orientation( b0[i].slerp( b1[i], t ) );
      skelInst._localRefs[i].orientation( b0[i].nlerp( b1[i], t ) );
   }

   skelInst.updateGlobalTransforms();

   return pose;
}

//------------------------------------------------------------------------------
//!
uint
Puppeteer::setPose(
   Skeleton::Instance&                   /*skelInst*/,
   SkeletalAnimation*                    /*anim*/,
   const Vector< PositionalConstraint >& /*constraints*/,
   const float                           /*time*/
)
{
   return 0;
}

//------------------------------------------------------------------------------
//!
void
Puppeteer::resolveConstraints(
   Skeleton::Instance&                   skelInst,
   const Vector< PositionalConstraint >& constraints
)
{
   if( constraints.empty() ) return;

   Skeleton* skeleton = skelInst.skeleton();

   // Compute global root position.
   Vec3f root = skelInst.globalPosition(0);

   // 1. Transform constraints into end effector constraints.
   for( uint i = 0; i < constraints.size(); ++i )
   {
      int cID              = constraints[i].id();
      Skeleton::Limb& limb = skeleton->limbFromBone( cID );
      int eID              = limb.endEffectorID();

      Vec3f epos = skelInst.globalReferential( eID ).toMatrix() * skeleton->bone( eID ).endPoint();
      constraints[i]._currentPos     = epos;
      constraints[i]._endEffectorPos = constraints[i]._position;
      if( cID != eID )
      {
         Vec3f cpos = skelInst.globalReferential( cID ).toMatrix() * skeleton->bone( cID ).endPoint();
         constraints[i]._endEffectorPos -= cpos-epos;
      }

      Vec3f delta = root - skelInst.globalPosition( limb.boneID() );
      constraints[i]._sphere.center( constraints[i]._endEffectorPos + delta );
      constraints[i]._sphere.radius( limb.reachRadius()/constraints[i].weight() );
   }

   // 2. Compute an approximate root position.
   Vec3f dRoot(0.0f);
   float weights   = 0.0f;
   float maxWeight = 0.0f;
   for( uint i = 0; i < constraints.size(); ++i )
   {
      dRoot   += constraints[i].weight() * (constraints[i]._endEffectorPos-constraints[i]._currentPos);
      weights += constraints[i].weight();
      if( constraints[i].weight() > maxWeight )
      {
         maxWeight = constraints[i].weight();
      }
   }
   dRoot *= (maxWeight/weights)*0.8f;
   root  += dRoot;


   // 3. Adjust root position to satisfy all constraints.
   adjustRoot( constraints, root, dRoot );

   // 4. Adjust limbs positions.
   for( uint i = 0; i < constraints.size(); ++i )
   {
      adjustLimb( skelInst, constraints[i], dRoot );
   }

   // 5. Adjust skeleton instance.
   skelInst.offset().position() += dRoot;
   skelInst.updateGlobalTransforms();

   // TODO: Ajust end of limbs...
}

/*==============================================================================
   CLASS GraphRetargetingTask
==============================================================================*/

void
GraphRetargetingTask::execute()
{
   // 1. Clone the graph.
   RCP<AnimationGraph> graph = _graph->clone();

   // 2. Retarget each animation in each node.

   // Fetch all animations contained in the graph.
   Vector< SkeletalAnimation* > anims;
   _graph->getAnimations( anims );

   // Retarget all animation.
   Vector< RCP< Resource<SkeletalAnimation> > > animsRes (anims.size());
   for( uint i = 0; i < anims.size(); ++i )
   {
      animsRes[i] = ResManager::retarget( anims[i], _skel.ptr(), this );
   }
   waitForAll();

   // Replace all animations.
   Map< SkeletalAnimation*, RCP<SkeletalAnimation> > ranims;
   for( uint i = 0; i < anims.size(); ++i )
   {
      ranims[anims[i]] = waitForData( animsRes[i].ptr() );
   }
   graph->replaceAnimations( ranims );

   _res->data( graph.ptr() );
}





