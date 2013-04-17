/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef MOTION_COLLISION_INFO_H
#define MOTION_COLLISION_INFO_H

#include <Motion/StdDefs.h>
#include <Motion/World/RigidBody.h>

#include <Base/ADT/Vector.h>
#include <Base/IO/TextStream.h>
#include <Base/Util/RCP.h>

#include <CGMath/Vec3.h>
#include <CGMath/Mat3.h>

NAMESPACE_BEGIN


/*==============================================================================
  CLASS CollisionInfo
==============================================================================*/
//! A class containing all of the collision information
class CollisionInfo
{
public:

   /*==============================================================================
     CLASS Contact
   ==============================================================================*/
   //! A class containing information on where collisions occured
   //! On creation, positions and normal are given in world space.
   class Contact
   {
   public:

      /*----- methods -----*/

      Contact() { }
      
      Contact( 
         const Vec3f& worldA, 
         const Vec3f& worldB, 
         const Vec3f& worldN, 
         const Vec3f& localA, 
         const Vec3f& localB 
      ) :
         _p( 0.0f ),
         _fp( 0.0f ),
         _localPosA( localA ), _localPosB( localB ),
         _worldPosA( worldA ), _worldPosB( worldB ), 
         _worldN( worldN ) 
      {
         _depth = ( _worldPosB - _worldPosA ).dot( _worldN );
      }

      // Local space attributes.
      const Vec3f&  localPositionA() const { return _localPosA; }
      void  localPositionA( const Vec3f& p ) { _localPosA = p; }

      const Vec3f&  localPositionB() const { return _localPosB; }
      void  localPositionB( const Vec3f& p ) { _localPosB = p; }
      
      // World space attributes.
      const Vec3f&  worldPositionA() const { return _worldPosA; }
      const Vec3f&  worldPositionB() const { return _worldPosB; }

            Vec3f&  worldNormal()       { return _worldN; }
      const Vec3f&  worldNormal() const { return _worldN; }
      void  worldNormal( const Vec3f& n ) { _worldN = n; }

      // Get the penetration depth.
      float  depth() const { return _depth; }
      
      // Update the world position of the contact points.
      void updateWorldPosition( const Mat4f& matA, const Mat4f& matB ) 
      {
         _worldPosA = matA * _localPosA;
         _worldPosB = matB * _localPosB;
         _depth     = ( _worldPosB - _worldPosA ).dot( _worldN );
      }
      
      /*----- data members -----*/
      
      float   _p;
      float   _fp;
      float   _restitution;
      float   _nRelVel;
      float   _normK;
      Mat3f   _k;

   protected:

      /*----- data members -----*/

      Vec3f  _localPosA; //! Local position of the contact point on object A
      Vec3f  _localPosB; //! Local position of the contact point on object B
      Vec3f  _worldPosA; //! World position of the contact point on object A
      Vec3f  _worldPosB; //! World position of the contact point on object B
      Vec3f  _worldN;    //! Normal at the contact point (relative to object A)
      float  _depth;
   }; //class Contact


   /*----- methods -----*/

   CollisionInfo() : _numContacts(0) { }

   //------------------------------------------------------------------------------
   //! Adds a contact point to this collision.
   void  addContact( const Contact& contact ) 
   { 
      if( _numContacts < 4 )
      {
         _contacts[_numContacts++] = contact;
      }
      else
      {
         // Find which contact to replace.
         
         // FIXME: For now we use Bullet algo.
         
         int maxDepthId = -1;
         float maxDepth = contact.depth();
         for( int i = 0; i < 4; ++i )
         {
            if( _contacts[i].depth() > maxDepth )
            {
               maxDepth = _contacts[i].depth();
               maxDepthId = i;
            }
         }
#if 0         
         Vec4f s( 0.0f );
         
         if( maxDepthId != 0 )
         {
            Vec3f e0 = contact.localPositionA() - _contacts[1].localPositionA();
            Vec3f e1 = _contacts[3].localPositionA() -_contacts[2].localPositionA();
            s(0) = e0.cross( e1 ).sqrLength();
         }
         
         if( maxDepthId != 1 )
         {
            Vec3f e0 = contact.localPositionA() - _contacts[0].localPositionA();
            Vec3f e1 = _contacts[3].localPositionA() -_contacts[2].localPositionA();
            s(1) = e0.cross( e1 ).sqrLength();
         }
         
         if( maxDepthId != 2 )
         {
            Vec3f e0 = contact.localPositionA() - _contacts[0].localPositionA();
            Vec3f e1 = _contacts[3].localPositionA() -_contacts[1].localPositionA();
            s(2) = e0.cross( e1 ).sqrLength();
         }
         
         if( maxDepthId != 3 )
         {
            Vec3f e0 = contact.localPositionA() - _contacts[0].localPositionA();
            Vec3f e1 = _contacts[2].localPositionA() -_contacts[1].localPositionA();
            s(3) = e0.cross( e1 ).sqrLength();
         }
         
         int id = 0;
         float maxArea = s(0);
         
         for( int i = 1; i < 4; ++i )
         {
            if( s(i) > maxArea )
            {
               maxArea = s(i);
               id = i;
            }
         }
         
         // Replace contact.
         _contacts[id] = contact;
#else
         float edges[10];
         edges[0] = ( _contacts[0].localPositionA() - _contacts[1].localPositionA() ).length();
         edges[1] = ( _contacts[0].localPositionA() - _contacts[2].localPositionA() ).length();
         edges[2] = ( _contacts[0].localPositionA() - _contacts[3].localPositionA() ).length();
         edges[3] = ( _contacts[0].localPositionA() - contact.localPositionA() ).length();
         edges[4] = ( _contacts[1].localPositionA() - _contacts[2].localPositionA() ).length();
         edges[5] = ( _contacts[1].localPositionA() - _contacts[3].localPositionA() ).length();
         edges[6] = ( _contacts[1].localPositionA() - contact.localPositionA() ).length();
         edges[7] = ( _contacts[2].localPositionA() - _contacts[3].localPositionA() ).length();
         edges[8] = ( _contacts[2].localPositionA() - contact.localPositionA() ).length();
         edges[9] = ( _contacts[3].localPositionA() - contact.localPositionA() ).length();
         
         
         float sum[5] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
         
         if( maxDepthId != 0 )
         {
            sum[0] = edges[4] + edges[5] + edges[6] + edges[7] + edges[8] + edges[9];
         }
         if( maxDepthId != 1 )
         {
            sum[1] = edges[1] + edges[2] + edges[3] + edges[7] + edges[8] + edges[9];
         }
         if( maxDepthId != 2 )
         {
            sum[2] = edges[0] + edges[2] + edges[3] + edges[5] + edges[6] + edges[9];
         }
         if( maxDepthId != 3 )
         {
            sum[3] = edges[0] + edges[1] + edges[3] + edges[4] + edges[6] + edges[8];
         }
         if( maxDepthId != -1 )
         {
            sum[4] = edges[0] + edges[1] + edges[2] + edges[4] + edges[5] + edges[7];
         }
         
         int id = 0;
         float maxEdges = sum[0];
         
         for( int i = 1; i < 4; ++i )
         {
            if( sum[i] > maxEdges )
            {
               maxEdges = sum[i];
               id = i;
            }
         }
         
         // Reject new point.
         if( sum[4] > maxEdges )
         {
            return;
         }
         _contacts[id] = contact;
#endif         
         
      }
   }

   //------------------------------------------------------------------------------
   //! Returns the number of contact in this collision (0 means no collision).
   uint  numContacts() const { return _numContacts; }

   //------------------------------------------------------------------------------
   //! Retrieves the specified contact.
   const Contact&  contact( const uint idx ) const { return _contacts[idx]; }
   Contact&  contact( const uint idx ) { return _contacts[idx]; }

   //------------------------------------------------------------------------------
   //! Removes all of the contacts.
   void  clearContacts() { _numContacts = 0; }
   
   // Separting axis caching.
   void  separatingAxis( const Vec3f& axis ) { _sepAxis = axis; }
   const Vec3f& separatingAxis() const { return _sepAxis; }
   
   // 
   void updatePositions( const Mat4f& matA, const Mat4f& matB )
   {
      for( uint i = 0; i < _numContacts; ++i )
      {
         _contacts[i].updateWorldPosition( matA, matB );
      }
   }
   
   void removeContact( uint i )
   {
      _contacts[i] = _contacts[--_numContacts];
   }
   
   void removeInvalids()
   {
      const float threshold = -0.01f;
      for( uint i = 0; i < _numContacts; ++i )
      {
         // Too far apart?
         if( _contacts[i].depth() < threshold )
         {
            //printf( "removing too far: %f\n", _contacts[i].depth() );
            removeContact(i);
         }
         else
         {
            //printf( "a(%f, %f, %f) b(%f, %f, %f)\n",
            //   _contacts[i].worldPositionA().x,
            //   _contacts[i].worldPositionA().y,
            //   _contacts[i].worldPositionA().z,
            //   _contacts[i].worldPositionB().x,
            //   _contacts[i].worldPositionB().y,
            //   _contacts[i].worldPositionB().z
            //);
            Vec3f proj = _contacts[i].worldPositionA() + 
               _contacts[i].worldNormal() * _contacts[i].depth();
            Vec3f tanB = _contacts[i].worldPositionB() - proj;
            
            // Tangent too big?
            if( tanB.sqrLength() > threshold*threshold )
            {
               //printf( "tanB to far: %f\n", tanB.length() );
               removeContact(i);
            }
         }
      }
   }

protected:

   /*----- data members -----*/

   Vec3f      _sepAxis;
   uint       _numContacts;
   Contact    _contacts[4];
   
}; //class CollisionInfo

//------------------------------------------------------------------------------
//!
inline TextStream&
operator<<
( TextStream& stream, const CollisionInfo::Contact& contact )
{
   return stream << "(" << contact.worldPositionA() << "," 
                 << contact.worldNormal() << "," << contact.depth() << ")";
}

//------------------------------------------------------------------------------
//!
inline TextStream&
operator<<
( TextStream& stream, const CollisionInfo& colInfo )
{
   stream << "ColInfo[";
   for( uint i = 0; i < colInfo.numContacts(); ++i )
   {
      stream << " " << colInfo.contact(i);
   }
   return stream << " ]";
}

/*==============================================================================
  CLASS CollisionPair
==============================================================================*/
//! A class containing all of the collision information between two bodies.
class CollisionPair
{
public:

   /*----- methods -----*/

   CollisionPair( const RCP<RigidBody>& bodyA, const RCP<RigidBody>& bodyB ) :
      _info(0)
   {
      if( bodyA.ptr() < bodyB.ptr() )
      {
         _bodyA = bodyA.ptr();
         _bodyB = bodyB.ptr();
      }
      else
      {
         _bodyA = bodyB.ptr();
         _bodyB = bodyA.ptr();
      }
   }
   
   ~CollisionPair()
   {
      delete _info;
   }
   
   CollisionInfo* allocateInfo()
   {
      if( !_info )
      {
         _info = new CollisionInfo;
         // Compute separating axis.
         Vec3f dir = _bodyB->simPosition() - _bodyA->simPosition();
         if( CGM::equal( dir.sqrLength(), 0.0f ) )
         {
            // Degenerate case; use +X for now.
            dir = Vec3f(1.0f, 0.0f, 0.0f);
         }
         _info->separatingAxis( dir );
      }
      return _info;
   }
      
   void frame( uint f ) { _frame = f; }
   uint frame() const { return _frame; }
      
      
   // Accessors.
   RigidBody* bodyA() { return _bodyA; }
   const RigidBody* bodyA() const { return _bodyA; }

   RigidBody* bodyB() { return _bodyB; }
   const RigidBody* bodyB() const { return _bodyB; }
         
   CollisionInfo* info() const { return _info; }
      
   // Operators.
   bool operator<( const CollisionPair& p ) const
   {
      if( _bodyA < p._bodyA )
      {
         return true;
      }
      if( _bodyA == p._bodyA && _bodyB < p._bodyB )
      {
         return true;
      }
      return false;
   }
   
   bool operator==( const CollisionPair& p ) const
   {
      return ( _bodyA == p._bodyA && _bodyB == p._bodyB );
   }
   bool operator!=( const CollisionPair& p ) const
   {
      return ( _bodyA != p._bodyA || _bodyB != p._bodyB );
   }

private:
   
   /*----- data members -----*/

   RigidBody*         _bodyA;
   RigidBody*         _bodyB;
   CollisionInfo*     _info;
   uint               _frame;
}; 

NAMESPACE_END

#endif //MOTION_COLLISION_INFO_H
