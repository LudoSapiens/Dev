/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_SKELETON_H
#define PLASMA_SKELETON_H

#include <Plasma/StdDefs.h>

#include <CGMath/Mat4.h>
#include <CGMath/Ref.h>

#include <Gfx/Prog/Constants.h>

#include <Base/ADT/Vector.h>
#include <Base/ADT/Map.h>

NAMESPACE_BEGIN

class MeshGeometry;
class SkeletalAnimation;
class SurfaceGeometry;

/*==============================================================================
  CLASS Skeleton
==============================================================================*/

//! Define a default canonical posture used by a skeleton instance.

class Skeleton
   : public RCObject
{

public:

   /*----- classes -----*/

   class Bone;
   class Instance;

   struct Node
   {
      int boneID() const       { return _boneID; }
      uint skipNodeID() const  { return _skipNodeID; }
      bool isLeaf() const      { return _numChildren == 0; }

      int  _boneID;
      uint _numChildren;
      uint _skipNodeID;
   };

   struct Limb
   {
      Limb( int boneID, uint nodeID ) :
         _boneID( boneID ), _nodeID( nodeID )
      {}

      int boneID() const         { return _boneID; }
      uint nodeID() const        { return _nodeID; }
      int endEffectorID() const  { return _endEffectorID; }
      float reachRadius() const  { return _reachRadius; }

      int   _boneID;
      uint  _nodeID;
      int   _endEffectorID;
      float _reachRadius;
   };

   class DepthFirstIterator
   {
   public:

      DepthFirstIterator( uint node, uint end ) :
         _nodeID( node ), _endID( end )
      {}

      bool operator()() const           { return _nodeID < _endID; }
      uint operator*() const            { return _nodeID; }
      DepthFirstIterator& operator++()  { ++_nodeID; return *this; }

    private:

      uint _nodeID;
      uint _endID;
   };

   /*----- types and enumerations ----*/

   typedef Vector< Bone >  BoneContainer;
   typedef Vector< Mat4f > MatrixContainer;

   enum
   {
      ROOT = -1,
      BODY = (uint)~0
   };

   enum
   {
      RX = 1,
      RY = 2,
      RZ = 4
   };

   /*----- methods -----*/

   PLASMA_DLL_API Skeleton();
   PLASMA_DLL_API Skeleton( const Skeleton& );

   // Bones.
   inline void reserveBones( uint );
   inline void addBone( const ConstString& name, const Reff& ref, int parentID, const Vec3f& endPoint = Vec3f(0,0,0) );

   inline const BoneContainer& bones() const;
   inline BoneContainer& bones();
   inline const Bone& bone( uint ) const;
   inline Bone& bone( uint );
   inline uint numBones() const;

   // Limbs.
   inline void reserveLimbs( uint );
   inline void addLimb( int startBone );

   inline const Limb& limb( uint ) const;
   inline Limb& limb( uint );
   inline const Limb& limbFromBone( uint ) const;
   inline Limb& limbFromBone( uint );
   inline uint numLimbs() const;

   // Bones ordering.
   inline const Vector<Node>& hierarchy() const;
   inline const Node& node( uint ) const;
   inline Node& node( uint );
   inline DepthFirstIterator depthFirst( uint nodeID ) const;

   // ...
   PLASMA_DLL_API void computeDerivedData();
   PLASMA_DLL_API RCP<MeshGeometry> createMesh();
   PLASMA_DLL_API RCP<SurfaceGeometry> createSurface();
   PLASMA_DLL_API void computeEndpoints();

   // Change the skeleton to follow constraints.
   PLASMA_DLL_API RCP<Skeleton> retarget( Map<ConstString,Vec3f>& ) const;
   PLASMA_DLL_API void retargetEq( Map<ConstString,Vec3f>& );

   // Debug.
   PLASMA_DLL_API void dump() const;

protected:

   /*----- methods -----*/

   PLASMA_DLL_API virtual ~Skeleton();

private:


   /*----- data members -----*/

   BoneContainer   _bones;
   Vector<Node>    _hierarchy;
   Vector<Limb>    _limbs;
};

/*==============================================================================
  CLASS Skeleton::Bone
==============================================================================*/

//! Define a bone structure with a referential, an end point and some
//! constraints. A bone is used in a skeleton to define a canonical posture.
//! Referentials defining a current pose for bones are defined in
//! Skeleton::Instance.
class Skeleton::Bone
{

public:

   /*----- methods -----*/

   Bone( const Reff& ref, int parentID, const Vec3f& endPoint = Vec3f(0,0,0) )
      : _localRef( ref ), _parentID( parentID ), _limbID(0), _dof(0), _endPoint( endPoint )
   {}

   Bone( const ConstString& name, const Reff& ref, int parentID, const Vec3f& endPoint = Vec3f(0,0,0) )
      : _name( name ), _localRef( ref ), _parentID( parentID ), _endPoint( endPoint )
   {}

   inline const ConstString& name() const                  { return _name; }

   inline const Reff& localReferential() const             { return _localRef; }
   inline void localReferential( const Reff& ref )         { _localRef = ref; }

   inline const Vec3f& position() const                    { return _localRef.position(); }
   inline void position( const Vec3f& pos )                { _localRef.position( pos ); }
   inline const Quatf& orientation() const                 { return _localRef.orientation(); }
   inline void orientation( const Quatf& orient )          { _localRef.orientation( orient ); }

   inline int parent() const                               { return _parentID; }

   inline void endPoint( const Vec3f& pt )                 { _endPoint = pt; }
   inline const Vec3f& endPoint() const                    { return _endPoint; }

   inline float length() const                             { return _endPoint.length(); }

   inline void globalToLocalReferential( const Reff& ref ) { _globalToLocalRef = ref; }
   inline const Reff& globalToLocalReferential() const     { return _globalToLocalRef; }

   inline uint limb() const                                { return _limbID; }
   inline void limb( uint l )                              { _limbID = l; }

   inline uint dof() const                                 { return _dof; }
   inline void dof( uint val )                             { _dof = val; }

private:

   /*----- data members -----*/

   ConstString _name;      //!< A unique name describing the bone.
   Reff        _localRef;  //!< The referential for the bone, specifying it's local space.
   int         _parentID;  //!< The index of the parent bone (Skeleton::ROOT if the bone is a root bone).
   uint        _limbID;    //!< The index of the associated limb.
   uint        _dof;       //!< Degrees of freedrom.
   Vec3f       _endPoint;  //!< The end position of the bone, in its local space.
   Reff        _globalToLocalRef; //!< A cached inverse transform used for performance optimizations.
};

/*==============================================================================
  CLASS Skeleton::Instance
==============================================================================*/

//! Define a current pose for a skeleton.
//! A pose is made up of a vector of referentials, one for each bone and a offset
//! representing the current offset in the animation cycle. The global transform
//! takes into account the canonical global to local transform of a bone, the
//! list of local transforms from this bone to the root, and the root itself.

class Skeleton::Instance
{

public:

   /*----- methods -----*/

   Instance();

   ~Instance() {}

   void skeleton( Skeleton* );
   inline Skeleton* skeleton() const;

   inline const Reff& offset() const;
   inline Reff& offset();
   inline void offset( const Reff& );

   inline const Vector<Reff>& globalReferentials() const;
   inline const Reff& globalReferential( uint index ) const;
   inline const Vec3f& globalPosition( uint index ) const;

   inline const MatrixContainer& transforms() const;

   inline const RCP<Gfx::ConstantBuffer>& constants() const;

   uint setPose( SkeletalAnimation* anim, float time );
   void setPose( const Vector<Reff>& pose );
   inline const Vector<Reff>& pose() const;

   inline void localReferential( uint index, const Reff& ref );
   inline void localReferential( uint index, const Reff& ref, const Vector<bool>& dirtyMatrices );
   inline void localReferentialNoUpdate( uint index, const Reff& ref );
   inline const Reff& localReferential( uint index ) const;

   inline void localOrientation( uint index, const Quatf& );
   inline void localOrientationNoUpdate( uint index, const Quatf& );
   inline const Quatf& localOrientation( uint index ) const;

   void updateGlobalTransforms();
   void updateGlobalTransforms( const Vector<bool>& dirtyMatrices );

private:

   friend class Puppeteer;

   /*----- data members -----*/

   Reff                     _offset;
   RCP<Skeleton>            _skeleton;
   Vector<Reff>             _localRefs;
   Vector<Reff>             _globalRefs;
   MatrixContainer          _transforms; // Should probably change to quaternion + vec3.
   RCP<Gfx::ConstantBuffer> _constants;
};

//------------------------------------------------------------------------------
//!
inline void
Skeleton::reserveBones( uint qty )
{
   _bones.reserve( qty );
}

//------------------------------------------------------------------------------
//!
inline void
Skeleton::addBone( const ConstString& name, const Reff& ref, int parentID, const Vec3f& endPoint )
{
   _bones.pushBack( Bone( name, ref, parentID, endPoint ) );
}

//------------------------------------------------------------------------------
//!
inline const Skeleton::BoneContainer&
Skeleton::bones() const
{
   return _bones;
}

//------------------------------------------------------------------------------
//!
inline Skeleton::BoneContainer&
Skeleton::bones()
{
   return _bones;
}

//------------------------------------------------------------------------------
//!
inline const Skeleton::Bone&
Skeleton::bone( uint i ) const
{
   return _bones[i];
}

//------------------------------------------------------------------------------
//!
inline Skeleton::Bone&
Skeleton::bone( uint i )
{
   return _bones[i];
}

//------------------------------------------------------------------------------
//!
inline uint
Skeleton::numBones() const
{
   return (uint)_bones.size();
}

//------------------------------------------------------------------------------
//!
inline void
Skeleton::reserveLimbs( uint qty )
{
   _limbs.reserve( qty );
}

//------------------------------------------------------------------------------
//!
inline void
Skeleton::addLimb( int startBone )
{
   _limbs.pushBack( Limb( startBone, 0 ) );
}

//------------------------------------------------------------------------------
//!
inline const Skeleton::Limb&
Skeleton::limb( uint i ) const
{
   return _limbs[i];
}

//------------------------------------------------------------------------------
//!
inline Skeleton::Limb&
Skeleton::limb( uint i )
{
   return _limbs[i];
}

//------------------------------------------------------------------------------
//!
inline const Skeleton::Limb&
Skeleton::limbFromBone( uint i ) const
{
   return _limbs[bone(i).limb()];
}

//------------------------------------------------------------------------------
//!
inline Skeleton::Limb&
Skeleton::limbFromBone( uint i )
{
   return _limbs[bone(i).limb()];
}

//------------------------------------------------------------------------------
//!
inline uint
Skeleton::numLimbs() const
{
   return (uint)_limbs.size();
}

//------------------------------------------------------------------------------
//!
inline const Vector<Skeleton::Node>&
Skeleton::hierarchy() const
{
   return _hierarchy;
}

//------------------------------------------------------------------------------
//!
inline const Skeleton::Node&
Skeleton::node( uint i ) const
{
   return _hierarchy[i];
}

//------------------------------------------------------------------------------
//!
inline Skeleton::Node&
Skeleton::node( uint i )
{
   return _hierarchy[i];
}

//------------------------------------------------------------------------------
//!
inline Skeleton::DepthFirstIterator
Skeleton::depthFirst( uint nodeID ) const
{
   return DepthFirstIterator( nodeID, _hierarchy[nodeID].skipNodeID() );
}

//------------------------------------------------------------------------------
//!
inline const Reff&
Skeleton::Instance::offset() const
{
   return _offset;
}

//------------------------------------------------------------------------------
//!
inline Reff&
Skeleton::Instance::offset()
{
   return _offset;
}

//------------------------------------------------------------------------------
//!
inline void
Skeleton::Instance::offset( const Reff& offset )
{
   _offset = offset;
}

//------------------------------------------------------------------------------
//!
inline Skeleton*
Skeleton::Instance::skeleton() const
{
   return _skeleton.ptr();
}

//------------------------------------------------------------------------------
//!
inline const Vector<Reff>&
Skeleton::Instance::globalReferentials() const
{
   return _globalRefs;
}

//------------------------------------------------------------------------------
//!
inline const Reff&
Skeleton::Instance::globalReferential( uint index ) const
{
   return _globalRefs[index];
}

//------------------------------------------------------------------------------
//!
inline const Vec3f&
Skeleton::Instance::globalPosition( uint index ) const
{
   return _globalRefs[index].position();
}

//------------------------------------------------------------------------------
//!
inline const Skeleton::MatrixContainer&
Skeleton::Instance::transforms() const
{
   return _transforms;
}

//------------------------------------------------------------------------------
//!
inline const RCP<Gfx::ConstantBuffer>&
Skeleton::Instance::constants() const
{
   return _constants;
}

//------------------------------------------------------------------------------
//!
inline const Vector<Reff>&
Skeleton::Instance::pose() const
{
   return _localRefs;
}

//------------------------------------------------------------------------------
//!
inline void
Skeleton::Instance::localReferential( uint index, const Reff& ref )
{
   _localRefs[index] = ref;
   updateGlobalTransforms();
}

//------------------------------------------------------------------------------
//!
inline void
Skeleton::Instance::localReferential
( uint index, const Reff& ref, const Vector<bool>& dirtyMatrices )
{
   _localRefs[index] = ref;
   updateGlobalTransforms( dirtyMatrices );
}

//------------------------------------------------------------------------------
//!
inline void
Skeleton::Instance::localReferentialNoUpdate( uint index, const Reff& ref )
{
   _localRefs[index] = ref;
}

//------------------------------------------------------------------------------
//!
inline const Reff&
Skeleton::Instance::localReferential( uint index ) const
{
   return _localRefs[index];
}

//------------------------------------------------------------------------------
//!
inline void
Skeleton::Instance::localOrientation( uint index, const Quatf& orient )
{
   _localRefs[index].orientation( orient );
   updateGlobalTransforms();
}

//------------------------------------------------------------------------------
//!
inline void
Skeleton::Instance::localOrientationNoUpdate( uint index, const Quatf& orient )
{
   _localRefs[index].orientation( orient );
}

//------------------------------------------------------------------------------
//!
inline const Quatf&
Skeleton::Instance::localOrientation( uint index ) const
{
   return _localRefs[index].orientation();
}

NAMESPACE_END

#endif

