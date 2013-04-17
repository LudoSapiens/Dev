/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/Animation/Skeleton.h>
#include <Plasma/Animation/SkeletalAnimation.h>
#include <Plasma/Geometry/MeshGeometry.h>
#include <Plasma/Geometry/SurfaceGeometry.h>

#include <Fusion/Core/Core.h>

#include <Base/ADT/Set.h>
#include <Base/Dbg/DebugStream.h>

#include <CGMath/CGMath.h>

/*==============================================================================
  UNNAME NAMESPACE
==============================================================================*/

UNNAMESPACE_BEGIN

DBG_STREAM( os_skl, "Skeleton" );

// Vertices used for the bone model.
Vec3f sVertexSet[] = {
   Vec3f(0,0,0),
   Vec3f(-0.15f,0.15f,0.2f),
   Vec3f(0.15f,0.15f,0.2f),
   Vec3f(0.15f,-0.15f,0.2f),
   Vec3f(-0.15f,-0.15f,0.2f),
   Vec3f(0,0,1)
};

// Faces used for bone model.
uint32_t sFaceSet[] = {
   0,1,2,
   0,2,3,
   0,3,4,
   0,4,1,
   1,5,2,
   2,5,3,
   3,5,4,
   4,5,1
};

//------------------------------------------------------------------------------
//! Creates a single mesh triangle.
//! Each vertex created is: Pos/Norm/Weights/Bones
void  createBoneTri( const Vec3f& a, const Vec3f& b, const Vec3f& c, float bone, float*& dstV, size_t dstVStride )
{
   Vec3f n = CGM::cross( b-a, c-a );
   n.normalize();
   Vec3f::as( dstV +  0 ) = a;
   Vec3f::as( dstV +  3 ) = n;
   Vec4f::as( dstV +  6 ) = Vec4f( 1.0f, 0.0f, 0.0f, 0.0f );
   Vec4f::as( dstV + 10 ) = Vec4f( bone, 0.0f, 0.0f, 0.0f );
   dstV += dstVStride;
   Vec3f::as( dstV +  0 ) = b;
   Vec3f::as( dstV +  3 ) = n;
   Vec4f::as( dstV +  6 ) = Vec4f( 1.0f, 0.0f, 0.0f, 0.0f );
   Vec4f::as( dstV + 10 ) = Vec4f( bone, 0.0f, 0.0f, 0.0f );
   dstV += dstVStride;
   Vec3f::as( dstV +  0 ) = c;
   Vec3f::as( dstV +  3 ) = n;
   Vec4f::as( dstV +  6 ) = Vec4f( 1.0f, 0.0f, 0.0f, 0.0f );
   Vec4f::as( dstV + 10 ) = Vec4f( bone, 0.0f, 0.0f, 0.0f );
   dstV += dstVStride;
}


//------------------------------------------------------------------------------
//! Creates a bone mesh made of 2 square-based pyramids placed back to back.
//! The first pyramid starts at A, and touches B1 to B4, whereas
//! the second pyramid touches B1 to B4 and ends at C.
void  createBoneMesh( uint32_t boneID, const Mat4f& mat, float* dstV, size_t dstVStride, uint32_t* dstI, uint32_t nV )
{
   size_t s = 3 + 3 + 4 + 4;
   CHECK( dstVStride == s );
   float bone = float(boneID);

   Vec3f A  = mat * Vec3f( 0.00f, 0.00f, 0.0f );
   Vec3f B1 = mat * Vec3f(-0.15f, 0.15f, 0.2f );
   Vec3f B2 = mat * Vec3f( 0.15f, 0.15f, 0.2f );
   Vec3f B3 = mat * Vec3f( 0.15f,-0.15f, 0.2f );
   Vec3f B4 = mat * Vec3f(-0.15f,-0.15f, 0.2f );
   Vec3f C  = mat * Vec3f( 0.00f, 0.00f, 1.0f );

   // First pyramid.
   createBoneTri( A, B1, B2, bone, dstV, s );
   createBoneTri( A, B2, B3, bone, dstV, s );
   createBoneTri( A, B3, B4, bone, dstV, s );
   createBoneTri( A, B4, B1, bone, dstV, s );
   *dstI++ = nV++; *dstI++ = nV++; *dstI++ = nV++;
   *dstI++ = nV++; *dstI++ = nV++; *dstI++ = nV++;
   *dstI++ = nV++; *dstI++ = nV++; *dstI++ = nV++;
   *dstI++ = nV++; *dstI++ = nV++; *dstI++ = nV++;

   // Second pyramid.
   createBoneTri( C, B2, B1, bone, dstV, s );
   createBoneTri( C, B3, B2, bone, dstV, s );
   createBoneTri( C, B4, B3, bone, dstV, s );
   createBoneTri( C, B1, B4, bone, dstV, s );
   *dstI++ = nV++; *dstI++ = nV++; *dstI++ = nV++;
   *dstI++ = nV++; *dstI++ = nV++; *dstI++ = nV++;
   *dstI++ = nV++; *dstI++ = nV++; *dstI++ = nV++;
   *dstI++ = nV++; *dstI++ = nV++; *dstI++ = nV++;
}

UNNAMESPACE_END


NAMESPACE_BEGIN

/*==============================================================================
  CLASS Skeleton::Instance
==============================================================================*/

//------------------------------------------------------------------------------
//!
Skeleton::Instance::Instance()
{
}

//------------------------------------------------------------------------------
//!
void
Skeleton::Instance::skeleton( Skeleton* val )
{
   _skeleton = val;
   _offset   = Reff::identity();

   if( _skeleton.isNull() )
   {
      _localRefs.clear();
      _transforms.clear();
      return;
   }

   // Resize the arrays to the bone hierarchy size.
   uint size = _skeleton->numBones();
   _localRefs.resize( size );
   _globalRefs.resize( size );
   _transforms.resize( size );

   // Create a constant buffer.
   // FIXME: for now we only store the ptr.
   _constants = Core::gfx()->createConstants( 8 );
   _constants->addConstantArray(
      "boneMatrices",
      Gfx::CONST_MAT4,
      size,
      0,
      Gfx::VERTEX_SHADER,
      _transforms.data()
   );

   // Reset to the reference position.
   for( uint i = 0; i < _skeleton->numBones(); ++i )
   {
      _localRefs[i] = _skeleton->bone(i).localReferential();
   }

   // Update the palette of matrices.
   updateGlobalTransforms();
}

//------------------------------------------------------------------------------
//!
void
Skeleton::Instance::updateGlobalTransforms()
{
   const Vector<Node>& hierarchy = _skeleton->hierarchy();
   const BoneContainer& bones    = _skeleton->bones();

   for( uint i = 0; i < _globalRefs.size(); ++i )
   {
      int b = hierarchy[i]._boneID;
      int p = bones[b].parent();

      _globalRefs[b] = p != Skeleton::ROOT ? _globalRefs[p] * _localRefs[b] : _offset*_localRefs[b];
      _transforms[b] = (_globalRefs[b] * bones[b].globalToLocalReferential()).toMatrix();
   }
}


//------------------------------------------------------------------------------
//!
void
Skeleton::Instance::updateGlobalTransforms( const Vector<bool>& dirtyMatrices )
{
   const Vector<Node>& hierarchy = _skeleton->hierarchy();
   const BoneContainer& bones    = _skeleton->bones();

   for( uint i = 0; i < _globalRefs.size(); ++i )
   {
      int b = hierarchy[i]._boneID;
      int p = bones[b].parent();

      if( dirtyMatrices[b] )
      {
         _globalRefs[b] = p != Skeleton::ROOT ? _globalRefs[p] * _localRefs[b] : _offset*_localRefs[b];
         _transforms[b] = (_globalRefs[b] * bones[b].globalToLocalReferential()).toMatrix();
      }
   }
}

//------------------------------------------------------------------------------
//!
uint
Skeleton::Instance::setPose( SkeletalAnimation* anim, float time )
{
   SkeletalPose* p0;
   SkeletalPose* p1;
   float t;

   uint pose = anim->getPoses( time, p0, p1, t );

   const SkeletalPose::BoneContainer& b0 = p0->bones();
   const SkeletalPose::BoneContainer& b1 = p1->bones();

   _offset = p0->referential().slerp( p1->referential(), t );

   for( uint i = 0; i < _localRefs.size(); ++i )
   {
      _localRefs[i].orientation( b0[i].slerp( b1[i], t ) );
   }

   updateGlobalTransforms();

   return pose;
}

//------------------------------------------------------------------------------
//!
void
Skeleton::Instance::setPose( const Vector<Reff>& pose )
{
   for( uint i = 0; i < _localRefs.size(); ++i )
   {
      _localRefs[i] = pose[i];
   }

   updateGlobalTransforms();
}


/*==============================================================================
  CLASS Skeleton
==============================================================================*/

//------------------------------------------------------------------------------
//!
Skeleton::Skeleton()
{
}

//------------------------------------------------------------------------------
//!
Skeleton::Skeleton( const Skeleton& skel ) :
   _bones( skel._bones ), _hierarchy( skel._hierarchy ), _limbs( skel._limbs )
{
}

//------------------------------------------------------------------------------
//!
Skeleton::~Skeleton()
{}

//------------------------------------------------------------------------------
//!
void
Skeleton::computeDerivedData()
{
   // Compute hierarchy.

   // 1. Compute a temporary tree like structure of the bones hierarchy.
   Vector<int> roots;
   Vector< Vector<int> > children( numBones() );
   for( uint i = 0; i < numBones(); ++i )
   {
      if( bone(i).parent() == Skeleton::ROOT )
      {
         roots.pushBack(i);
      }
      else
      {
         children[bone(i).parent()].pushBack(i);
      }
   }
   // 2. Traverse hierarchy in depth first order to construct the final
   // hierarchy structure.
   Vector<uint> traversal( numBones(), 0 );
   Vector<uint> boneNode( numBones() , 0 );
   _hierarchy.resize( numBones() );
   uint nodeID = 0;
   for( uint i = 0; i < roots.size(); ++i )
   {
      int boneID = roots[i];
      while( boneID != Skeleton::ROOT )
      {
         uint nextChild = traversal[boneID];
         // Create hierarchy info, if visited for the first time.
         if( nextChild == 0 )
         {
            _hierarchy[nodeID]._boneID      = boneID;
            _hierarchy[nodeID]._numChildren = uint(children[boneID].size());
            boneNode[boneID]                = nodeID;
            ++nodeID;
         }
         // Do we have a child to traverse?
         if( nextChild < children[boneID].size() )
         {
            ++traversal[boneID];
            boneID = children[boneID][nextChild];
         }
         else
         {
            // Going to parent.
            _hierarchy[boneNode[boneID]]._skipNodeID = nodeID;
            boneID = bone(boneID).parent();
         }
      }
   }

   // Compute limbs data.
   // 1. Compute limbs node IDs, endEffector and reach.
   for( uint i = 0; i < numLimbs(); ++i )
   {
      // Find limb node ID.
      limb(i)._nodeID = boneNode[limb(i).boneID()];
      // Find endEffectorID.
      limb(i)._endEffectorID = _hierarchy[limb(i).nodeID()+1].boneID();
      // Compute reach.
      limb(i)._reachRadius   =
         bone( limb(i).boneID() ).length() +
         bone( limb(i).endEffectorID() ).length();

   }
   // 2. Compute the limb ID of each bone.
   // 2.1. Set default limb to BODY.
   for( uint i = 0; i < numBones(); ++i )
   {
      bone(i).limb( BODY );
   }
   // 2.2. Traverse limbs and associate bones to them.
   for( uint i = 0; i < numLimbs(); ++i )
   {
      for( DepthFirstIterator it = depthFirst( limb(i).nodeID() ); it(); ++it )
      {
         bone( _hierarchy[*it].boneID() ).limb(i);
      }
   }

   // Compute the inverse global transform of all bones.
   Vector<Reff> globalRefs( numBones() );

   for( uint i = 0; i < numBones(); ++i )
   {
      int b = _hierarchy[i]._boneID;
      int p = bone(b).parent();
      globalRefs[b] = p != Skeleton::ROOT ? globalRefs[p] * bone(b).localReferential() :
         bone(b).localReferential();
   }

   // inverse.
   for( uint i = 0; i < numBones(); ++i )
   {
      bone(i).globalToLocalReferential( globalRefs[i].getInversed() );
   }
}

//------------------------------------------------------------------------------
//! Creates a simplistic geometry to visualize the skeleton (as in debugging).
RCP<MeshGeometry>
Skeleton::createMesh()
{
   int attribs[] = {
      MeshGeometry::POSITION,
      MeshGeometry::NORMAL,
      MeshGeometry::GENERIC_4, // Weigths
      MeshGeometry::GENERIC_4, // Bones
      0
   };
   uint32_t nV = 8*3;
   uint32_t nI = 8*3;
   uint32_t nB = numBones();
   RCP<MeshGeometry> mesh = new MeshGeometry( MeshGeometry::TRIANGLES, attribs, nV*nB, nI*nB );
   mesh->skeleton( this );

   float*    curV = mesh->vertices();
   uint32_t* curI = mesh->indices();
   for( uint32_t bi = 0; bi < nB; ++bi, curV += nV*mesh->vertexStride(), curI += nI )
   {
      // Need the global referential
      Mat4f boneMatrix = bone(bi).globalToLocalReferential().globalToLocal();

      Vec3f tempVec( bone(bi).endPoint() );
      Quatf tempQuat = Quatf::twoVecs( Vec3f(0.0f,0.0f,1.0f), tempVec );
      boneMatrix *= tempQuat.toMatrix();

      float length = tempVec.length();
      boneMatrix *= Mat4f(
         length, 0, 0, 0,
         0, length, 0, 0,
         0, 0, length, 0,
         0, 0, 0, 1
      );

      createBoneMesh( bi, boneMatrix, curV, mesh->vertexStride(), curI, nI*bi );
   }
   mesh->addPatch( 0, nI*nB );

   return mesh;
}

//------------------------------------------------------------------------------
//! Creates a simplistic geometry to visualize the skeleton (as in debugging).
RCP<SurfaceGeometry>
Skeleton::createSurface()
{
   //computeEndpoints();

   RCP<SurfaceGeometry> result( new SurfaceGeometry() );

   uint nbVertices = sizeof(sVertexSet)/sizeof(sVertexSet[0]);
   uint nbWedges   = sizeof(sFaceSet)/sizeof(sFaceSet[0]);
   uint nbFaces    = nbWedges/3;

   result->reserveVertices( numBones() * nbVertices );
   result->reserveWeights( numBones() * nbVertices );

   uint b, vertexBase;
   for ( b = 0, vertexBase = 0; b < numBones(); ++b, vertexBase += nbVertices )
   {
      // Need the global referential
      Mat4f boneMatrix = bone(b).globalToLocalReferential().globalToLocal();

      Vec3f tempVec( bone(b).endPoint() );
      Quatf tempQuat = Quatf::twoVecs( Vec3f(0,0,1), tempVec );
      boneMatrix *= tempQuat.toMatrix();

      float length = tempVec.length();
      boneMatrix *= Mat4f(
         length, 0, 0, 0,
         0, length, 0, 0,
         0, 0, length, 0,
         0, 0, 0, 1
      );

      // Add vertices
      for ( uint i=0; i < nbVertices; ++i )
      {
         result->addVertex( boneMatrix * sVertexSet[i] );
         result->addWeights( 1, Vec4f((float)b,0.0f,0.0f,0.0f), Vec4f(1.0f, 0.0f, 0.0f, 0.0f) );
      }

      // Create 1 patch per bone
      // different materials could come in handy for bone selection...
      RCP<SurfaceGeometry::Patch> currPatch = result->addPatch();
      currPatch->reserveFaces( nbFaces );

      for ( uint i=0; i < nbFaces; ++i )
      {
         currPatch->addFace(
            vertexBase + sFaceSet[i*3],
            vertexBase + sFaceSet[i*3+1],
            vertexBase + sFaceSet[i*3+2],
            0, 0, 0
         );
      }
   }

   // Mappings
   result->addMapping( Vec2f(0.0f) );
   result->computeDerivedData();

   return result;

}

//------------------------------------------------------------------------------
//!
void
Skeleton::computeEndpoints()
{
   //return;
   Set<uint> leaves;  // The leaf bones only (starts with all bones, then remove parents).
   for( uint i = 0; i < (uint)_bones.size(); ++i )
   {
      leaves.add( i );
   }

   // Assign endpoints of non-leaf bones.
   BoneContainer::Iterator cur = _bones.begin();
   BoneContainer::Iterator end = _bones.end();
   for( ; cur != end; ++cur )
   {
      if( (*cur).parent() != Skeleton::ROOT )
      {
         leaves.remove( (*cur).parent() );
         _bones[(*cur).parent()].endPoint( (*cur).position() );
      }
   }

   // Handle leaves differently by trying to repeat the last bone:
   //    parentEnd + (parentEnd - parentStart)
   //  = 2*parentEnd - parentStart
   // but rotated by the orientation matrix.
   // FIXME: Sometimes, it's too big, so it's scaled by half for now.
   Set<uint>::Iterator curLeaf = leaves.begin();
   Set<uint>::Iterator endLeaf = leaves.end();
   for( ; curLeaf != endLeaf; ++curLeaf )
   {
      Bone& curBone = _bones[(*curLeaf)];
      const Bone& parentBone = _bones[curBone.parent()];
      Vec3f parentStart;
      if( parentBone.parent() != Skeleton::ROOT )
      {
         // Just grab the endpoint of its parent.
         parentStart = _bones[parentBone.parent()].position();
      }
      else
      {
         // FIXME: This code is probably WRONG!!!
         // Compute bone's origin is world space using a matrix.
         Mat4f mat = parentBone.localReferential().globalToLocal();
         parentStart = mat * parentBone.position();
      }
      Vec3f boneDir = parentBone.endPoint()*2 - parentStart;
      curBone.endPoint( boneDir*0.5 );
   }
}

//------------------------------------------------------------------------------
//!
RCP<Skeleton>
Skeleton::retarget( Map<ConstString,Vec3f>& bones ) const
{
   RCP<Skeleton> skeleton( new Skeleton(*this) );
   skeleton->retargetEq( bones );
   return skeleton;
}

//------------------------------------------------------------------------------
//!
void
Skeleton::retargetEq( Map<ConstString,Vec3f>& bones )
{
   ConstString rootStr( "root" );
   Vec3f root;
   if( bones.has( rootStr ) )
   {
      root = bones[rootStr];
   }
   else
   {
      root = Vec3f::zero();
   }

   Vector<Reff> globalRefs( numBones() );
   Vector<bool> done( numBones(), false );
   Vector<float> scales( numBones(), 1.0f );
   Vector<int> link;

   // Fix constraints.
   for( uint i = 0; i < numBones(); ++i )
   {
      int b = _hierarchy[i]._boneID;
      int p = bone(b).parent();

      // Compute bone position and globalRefs.
      if( p != Skeleton::ROOT )
      {
         bone(b).position( bone(p).endPoint() );
         globalRefs[b] = globalRefs[p] * bone(b).localReferential();
      }
      else
      {
         bone(b).position(root);
         globalRefs[b] = bone(b).localReferential();
      }

      // Constraint to resolve?
      if( bones.has( bone(b).name() ) )
      {
         // Compute bone link.
         int startID = b;
         link.clear();
         link.pushBack( b );

         while( (p = bone(startID).parent()) != Skeleton::ROOT && !done[p]  )
         {
            startID = p;
            link.pushBack( p );
         }

         Vec3f startPos = globalRefs[startID].position();
         Vec3f curPos   = globalRefs[b].toMatrix() * bone(b).endPoint();
         Vec3f fixPos   = bones[bone(b).name()];

         Vec3f len1 = curPos - startPos;
         Vec3f len2 = fixPos - startPos;

         // Rotate.
         Quatf rot       = Quatf::twoVecs( len1, len2 );
         p               = bone(startID).parent();
         Quatf parentInv = p != Skeleton::ROOT ? globalRefs[p].orientation().getInversed() : Quatf::identity();
         Quatf orient    = parentInv * rot * globalRefs[startID].orientation();
         bone(startID).orientation( orient );

         // Scale.
         float scale = len2.length() / len1.length();
         for( int l = int(link.size())-1; l >= 0; --l )
         {
            b         = link[l];
            done[b]   = true;
            scales[b] = scale;
            p         = bone(b).parent();
            bone(b).endPoint( bone(b).endPoint() * scale );

            if( p != Skeleton::ROOT )
            {
               bone(b).position( bone(p).endPoint() );
               globalRefs[b] = globalRefs[p] * bone(b).localReferential();
            }
            else
            {
               bone(b).position(root);
               globalRefs[b] = bone(b).localReferential();
            }
         }
      }
   }

   // Fix extremeties.
   for( uint i = 0; i < numBones(); ++i )
   {
      int b = _hierarchy[i]._boneID;
      int p = bone(b).parent();

      // Compute bone position and endpoint.
      if( !done[b] )
      {
         if( p != Skeleton::ROOT )
         {
            bone(b).position( bone(p).endPoint() );
            bone(b).endPoint( bone(b).endPoint() * scales[p] );
            scales[b] = scales[p];
         }
         else
         {
            bone(b).position(root);
            globalRefs[b] = bone(b).localReferential();
         }
      }
   }

   computeDerivedData();
}

//------------------------------------------------------------------------------
//!
void
Skeleton::dump() const
{
   StdErr << "#bones: " << numBones() << nl;
   for( uint i = 0; i < numBones(); ++i )
   {
      StdErr << _bones[i].name().cstr()
             << " pos: " << _bones[i].position()
             << " ori: " << _bones[i].orientation()
             << " end: " << _bones[i].endPoint()
             << " par: " << _bones[i].parent() << nl;
   }
}

NAMESPACE_END
