/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Base/Dbg/UnitTest.h>

#include <CGMath/Vec3.h>

#include <MotionBullet/Collision/BasicShapes.h>
#include <MotionBullet/Collision/CollisionInfo.h>
#include <MotionBullet/Attractor/GravitationalAttractor.h>
#include <MotionBullet/World/MotionWorld.h>

USING_NAMESPACE

const float sFloatThreshold = 1e-5f;
//const float sFloatThreshold = 0;
const Vec3f sVec3fThreshold(sFloatThreshold);

//------------------------------------------------------------------------------
//!
inline bool
areEqual
( const float f1, const float f2, const float threshold = sFloatThreshold )
{
   return fabsf(f1 - f2) <= threshold;
}

//------------------------------------------------------------------------------
//!
inline bool
areEqual
( const Vec3f& v1, const Vec3f& v2, const Vec3f& threshold = sVec3fThreshold )
{
   return areEqual(v1(0), v2(0), threshold(0)) &&
          areEqual(v1(1), v2(1), threshold(1)) &&
          areEqual(v1(2), v2(2), threshold(2));
}

#if 0

//------------------------------------------------------------------------------
//!
void test_collision_sphere_sphere( Test::Result& res )
{
   bool col;
   CollisionInfo  colInfo;
   const CollisionInfo::Contact* contact = NULL;

   Reff refA = Reff::identity();
   Reff refB = Reff::identity();

   //Make the same example as the UnderstandingODE_K.odg document.
   RCP<CollisionShape> a( new SphereShape(2) );
   RCP<CollisionShape> b( new SphereShape(5) );

   CollisionGroup grpA, grpB;
   grpA.addShape( a );
   grpB.addShape( b );

   //Start by keeping A at the origin for now
   a->globalReferential( refA );
   refB.position( Vec3f(7.0001f, 0, 0) );
   b->globalReferential( refB );
   col = true; //FIXME CollisionShape::collide(a, b, colInfo);
   TEST_ADD( res, !col );
   TEST_ADD( res, colInfo.numContacts() == 0 );

   refB.position( Vec3f(7, 0, 0) );
   b->globalReferential( refB );
   col = false; //FIXME CollisionShape::collide(a, b, colInfo);
   TEST_ADD( res, col );
   TEST_ADD( res, colInfo.numContacts() == 1 );
   contact = &(colInfo.contact(0));
   TEST_ADD( res, areEqual(contact->worldPositionA(), Vec3f(+2.0, 0, 0)) );
   TEST_ADD( res, areEqual(contact->worldPositionB(), Vec3f(+2.0, 0, 0)) );
   TEST_ADD( res, areEqual(contact->worldNormal(), Vec3f(-1, 0, 0)) );
   TEST_ADD( res, areEqual(contact->depth(), 0) );

   colInfo.clearContacts();
   //Just check this functionality once
   TEST_ADD( res, colInfo.numContacts() == 0 );

   refB.position( Vec3f(6, 0, 0) );
   b->globalReferential( refB );
   col = false; //FIXME CollisionShape::collide(a, b, colInfo);
   TEST_ADD( res, col );
   TEST_ADD( res, colInfo.numContacts() == 1 );
   contact = &(colInfo.contact(0));
   TEST_ADD( res, areEqual(contact->worldPositionA(), Vec3f(+2.0, 0, 0)) );
   TEST_ADD( res, areEqual(contact->worldPositionB(), Vec3f(+1.0, 0, 0)) );
   TEST_ADD( res, areEqual(contact->worldNormal(), Vec3f(-1, 0, 0)) );
   TEST_ADD( res, areEqual(contact->depth(), 1) );

   colInfo.clearContacts();

   refB.position( Vec3f(5, 0, 0) );
   b->globalReferential( refB );
   col = false; //FIXME CollisionShape::collide(a, b, colInfo);
   TEST_ADD( res, col );
   TEST_ADD( res, colInfo.numContacts() == 1 );
   contact = &(colInfo.contact(0));
   TEST_ADD( res, areEqual(contact->worldPositionA(), Vec3f(+2.0, 0, 0)) );
   TEST_ADD( res, areEqual(contact->worldPositionB(), Vec3f(+0.0, 0, 0)) );
   TEST_ADD( res, areEqual(contact->worldNormal(), Vec3f(-1, 0, 0)) );
   TEST_ADD( res, areEqual(contact->depth(), 2) );

   colInfo.clearContacts();

   refB.position( Vec3f(4, 0, 0) );
   b->globalReferential( refB );
   col = false; //FIXME CollisionShape::collide(a, b, colInfo);
   TEST_ADD( res, col );
   TEST_ADD( res, colInfo.numContacts() == 1 );
   contact = &(colInfo.contact(0));
   TEST_ADD( res, areEqual(contact->worldPositionA(), Vec3f(+2.0, 0, 0)) );
   TEST_ADD( res, areEqual(contact->worldPositionB(), Vec3f(-1.0, 0, 0)) );
   TEST_ADD( res, areEqual(contact->worldNormal(), Vec3f(-1, 0, 0)) );
   TEST_ADD( res, areEqual(contact->depth(), 3) );

   colInfo.clearContacts();

   refB.position( Vec3f(3, 0, 0) );
   b->globalReferential( refB );
   col = false; //FIXME CollisionShape::collide(a, b, colInfo);
   TEST_ADD( res, col );
   TEST_ADD( res, colInfo.numContacts() == 1 );
   contact = &(colInfo.contact(0));
   TEST_ADD( res, areEqual(contact->worldPositionA(), Vec3f(+2.0, 0, 0)) );
   TEST_ADD( res, areEqual(contact->worldPositionB(), Vec3f(-2.0, 0, 0)) );
   TEST_ADD( res, areEqual(contact->worldNormal(), Vec3f(-1, 0, 0)) );
   TEST_ADD( res, areEqual(contact->depth(), 4) );

   colInfo.clearContacts();

   refB.position( Vec3f(2, 0, 0) );
   b->globalReferential( refB );
   col = false; //FIXME CollisionShape::collide(a, b, colInfo);
   TEST_ADD( res, col );
   TEST_ADD( res, colInfo.numContacts() == 1 );
   contact = &(colInfo.contact(0));
   TEST_ADD( res, areEqual(contact->worldPositionA(), Vec3f(+2.0, 0, 0)) );
   TEST_ADD( res, areEqual(contact->worldPositionB(), Vec3f(-3.0, 0, 0)) );
   TEST_ADD( res, areEqual(contact->worldNormal(), Vec3f(-1, 0, 0)) );
   TEST_ADD( res, areEqual(contact->depth(), 5) );

   colInfo.clearContacts();

   refB.position( Vec3f(1, 0, 0) );
   b->globalReferential( refB );
   col = false; //FIXME CollisionShape::collide(a, b, colInfo);
   TEST_ADD( res, col );
   TEST_ADD( res, colInfo.numContacts() == 1 );
   contact = &(colInfo.contact(0));
   TEST_ADD( res, areEqual(contact->worldPositionA(), Vec3f(+2.0, 0, 0)) );
   TEST_ADD( res, areEqual(contact->worldPositionB(), Vec3f(-4.0, 0, 0)) );
   TEST_ADD( res, areEqual(contact->worldNormal(), Vec3f(-1, 0, 0)) );
   TEST_ADD( res, areEqual(contact->depth(), 6) );

   colInfo.clearContacts();

   //Special case (sharing centers)
   refB.position( Vec3f(0, 0, 0) );
   b->globalReferential( refB );
   col = false; //FIXME CollisionShape::collide(a, b, colInfo);
   TEST_ADD( res, col );
   TEST_ADD( res, colInfo.numContacts() == 1 );
   contact = &(colInfo.contact(0));
   TEST_ADD( res, areEqual(contact->worldPositionA(), Vec3f(0.0f, -2.0f, 0.0f)) );
   TEST_ADD( res, areEqual(contact->worldPositionB(), Vec3f(0.0f, 5.0f, 0.0f)) );
   TEST_ADD( res, areEqual(contact->worldNormal(), Vec3f(0.0f, 1.0f, 0.0f)) );
   TEST_ADD( res, areEqual(contact->depth(), 7) );

   colInfo.clearContacts();

   //Start going out over the left side
   refB.position( Vec3f(-1, 0, 0) );
   b->globalReferential( refB );
   col = false; //FIXME CollisionShape::collide(a, b, colInfo);
   TEST_ADD( res, col );
   TEST_ADD( res, colInfo.numContacts() == 1 );
   contact = &(colInfo.contact(0));
   TEST_ADD( res, areEqual(contact->worldPositionA(), Vec3f(-2.0, 0, 0)) );
   TEST_ADD( res, areEqual(contact->worldPositionB(), Vec3f(+4.0, 0, 0)) );
   TEST_ADD( res, areEqual(contact->worldNormal(), Vec3f(1, 0, 0)) );
   TEST_ADD( res, areEqual(contact->depth(), 6) );

   colInfo.clearContacts();

   refB.position( Vec3f(-2, 0, 0) );
   b->globalReferential( refB );
   col = false; //FIXME CollisionShape::collide(a, b, colInfo);
   TEST_ADD( res, col );
   TEST_ADD( res, colInfo.numContacts() == 1 );
   contact = &(colInfo.contact(0));
   TEST_ADD( res, areEqual(contact->worldPositionA(), Vec3f(-2.0, 0, 0)) );
   TEST_ADD( res, areEqual(contact->worldPositionB(), Vec3f(+3.0, 0, 0)) );
   TEST_ADD( res, areEqual(contact->worldNormal(), Vec3f(1, 0, 0)) );
   TEST_ADD( res, areEqual(contact->depth(), 5) );

   colInfo.clearContacts();

   refB.position( Vec3f(-3, 0, 0) );
   b->globalReferential( refB );
   col = false; //FIXME CollisionShape::collide(a, b, colInfo);
   TEST_ADD( res, col );
   TEST_ADD( res, colInfo.numContacts() == 1 );
   contact = &(colInfo.contact(0));
   TEST_ADD( res, areEqual(contact->worldPositionA(), Vec3f(-2.0, 0, 0)) );
   TEST_ADD( res, areEqual(contact->worldPositionB(), Vec3f(+2.0, 0, 0)) );
   TEST_ADD( res, areEqual(contact->worldNormal(), Vec3f(1, 0, 0)) );
   TEST_ADD( res, areEqual(contact->depth(), 4) );

   colInfo.clearContacts();

   refB.position( Vec3f(-4, 0, 0) );
   b->globalReferential( refB );
   col = false; //FIXME CollisionShape::collide(a, b, colInfo);
   TEST_ADD( res, col );
   TEST_ADD( res, colInfo.numContacts() == 1 );
   contact = &(colInfo.contact(0));
   TEST_ADD( res, areEqual(contact->worldPositionA(), Vec3f(-2.0, 0, 0)) );
   TEST_ADD( res, areEqual(contact->worldPositionB(), Vec3f(+1.0, 0, 0)) );
   TEST_ADD( res, areEqual(contact->worldNormal(), Vec3f(1, 0, 0)) );
   TEST_ADD( res, areEqual(contact->depth(), 3) );

   colInfo.clearContacts();

   refB.position( Vec3f(-5, 0, 0) );
   b->globalReferential( refB );
   col = false; //FIXME CollisionShape::collide(a, b, colInfo);
   TEST_ADD( res, col );
   TEST_ADD( res, colInfo.numContacts() == 1 );
   contact = &(colInfo.contact(0));
   TEST_ADD( res, areEqual(contact->worldPositionA(), Vec3f(-2.0, 0, 0)) );
   TEST_ADD( res, areEqual(contact->worldPositionB(), Vec3f(+0.0, 0, 0)) );
   TEST_ADD( res, areEqual(contact->worldNormal(), Vec3f(1, 0, 0)) );
   TEST_ADD( res, areEqual(contact->depth(), 2) );

   colInfo.clearContacts();

   refB.position( Vec3f(-6, 0, 0) );
   b->globalReferential( refB );
   col = false; //FIXME CollisionShape::collide(a, b, colInfo);
   TEST_ADD( res, col );
   TEST_ADD( res, colInfo.numContacts() == 1 );
   contact = &(colInfo.contact(0));
   TEST_ADD( res, areEqual(contact->worldPositionA(), Vec3f(-2.0, 0, 0)) );
   TEST_ADD( res, areEqual(contact->worldPositionB(), Vec3f(-1.0, 0, 0)) );
   TEST_ADD( res, areEqual(contact->worldNormal(), Vec3f(1, 0, 0)) );
   TEST_ADD( res, areEqual(contact->depth(), 1) );

   colInfo.clearContacts();

   refB.position( Vec3f(-7, 0, 0) );
   b->globalReferential( refB );
   col = false; //FIXME CollisionShape::collide(a, b, colInfo);
   TEST_ADD( res, col );
   TEST_ADD( res, colInfo.numContacts() == 1 );
   contact = &(colInfo.contact(0));
   TEST_ADD( res, areEqual(contact->worldPositionA(), Vec3f(-2.0, 0, 0)) );
   TEST_ADD( res, areEqual(contact->worldPositionB(), Vec3f(-2.0, 0, 0)) );
   TEST_ADD( res, areEqual(contact->worldNormal(), Vec3f(1, 0, 0)) );
   TEST_ADD( res, areEqual(contact->depth(), 0) );

   colInfo.clearContacts();

   refB.position( Vec3f(-7.0001f, 0, 0) );
   b->globalReferential( refB );
   col = true; //FIXME CollisionShape::collide(a, b, colInfo);
   TEST_ADD( res, !col );
   TEST_ADD( res, colInfo.numContacts() == 0 );

   /**
   printf("a: pos=(%g, %g, %g)   b: pos=(%g, %g, %g)\n",
          a->globalReferential().position()(0), a->globalReferential().position()(1), a->globalReferential().position()(2),
          b->globalReferential().position()(0), b->globalReferential().position()(1), b->globalReferential().position()(2));
   printf("contact: posA=(%g, %g, %g)  posB=(%g, %g, %g)  norm=(%g, %g, %g)  depth=%g\n",
          contact->worldPositionA()(0), contact->worldPositionA()(1), contact->worldPositionA()(2),
          contact->worldworldPositionB()(0), contact->worldworldPositionB()(1), contact->worldPositionB()(2),
          contact->worldNormal()(0), contact->worldNormal()(1), contact->worldNormal()(2),
          contact->depth());
   **/
}

//------------------------------------------------------------------------------
//!
void test_collision_sphere_box( Test::Result& res )
{
   bool col;
   CollisionInfo  colInfo;
   const CollisionInfo::Contact* contact = NULL;

   Reff refA = Reff::identity();
   Reff refB = Reff::identity();

   RCP<CollisionShape> a( new SphereShape(2) );
   RCP<CollisionShape> b( new BoxShape( 10, 5, 5 ) );

   CollisionGroup grpA, grpB;
   grpA.addShape( a );
   grpB.addShape( b );

   //Start by keeping A at the origin for now
   a->globalReferential( refA );
   refB.position( Vec3f(7.0001f, 0, 0) );
   b->globalReferential( refB );
   col = true; //FIXME CollisionShape::collide(a, b, colInfo);
   TEST_ADD( res, !col );
   TEST_ADD( res, colInfo.numContacts() == 0 );

   refB.position( Vec3f(7, 0, 0) );
   b->globalReferential( refB );
   col = false; //FIXME CollisionShape::collide(a, b, colInfo);
   TEST_ADD( res, col );
   TEST_ADD( res, colInfo.numContacts() == 1 );
   contact = &(colInfo.contact(0));
   TEST_ADD( res, areEqual(contact->worldPositionA(), Vec3f(+2.0, 0, 0)) );
   TEST_ADD( res, areEqual(contact->worldPositionB(), Vec3f(+2.0, 0, 0)) );
   TEST_ADD( res, areEqual(contact->worldNormal(), Vec3f(-1, 0, 0)) );
   TEST_ADD( res, areEqual(contact->depth(), 0) );

   colInfo.clearContacts();
   //Just check this functionality once
   TEST_ADD( res, colInfo.numContacts() == 0 );


   refB.position( Vec3f(6, 0, 0) );
   b->globalReferential( refB );
   col = false; //FIXME CollisionShape::collide(a, b, colInfo);
   TEST_ADD( res, col );
   TEST_ADD( res, colInfo.numContacts() == 1 );
   contact = &(colInfo.contact(0));
   TEST_ADD( res, areEqual(contact->worldPositionA(), Vec3f(+2.0, 0, 0)) );
   TEST_ADD( res, areEqual(contact->worldPositionB(), Vec3f(+1.0, 0, 0)) );
   TEST_ADD( res, areEqual(contact->worldNormal(), Vec3f(-1, 0, 0)) );
   TEST_ADD( res, areEqual(contact->depth(), 1) );

   colInfo.clearContacts();

   refB.position( Vec3f(5, 0, 0) );
   b->globalReferential( refB );
   col = false; //FIXME CollisionShape::collide(a, b, colInfo);
   TEST_ADD( res, col );
   TEST_ADD( res, colInfo.numContacts() == 1 );
   contact = &(colInfo.contact(0));
   TEST_ADD( res, areEqual(contact->worldPositionA(), Vec3f(+2.0, 0, 0)) );
   TEST_ADD( res, areEqual(contact->worldPositionB(), Vec3f(+0.0, 0, 0)) );
   TEST_ADD( res, areEqual(contact->worldNormal(), Vec3f(-1, 0, 0)) );
   TEST_ADD( res, areEqual(contact->depth(), 2) );

   colInfo.clearContacts();
}

//------------------------------------------------------------------------------
//!
void test_collision_shape_support( Test::Result& res )
{
   RCP<CollisionShape> shape;
   Reff ref = Reff::identity();
   Vec3f dir(1, 0, 0);
   Vec3f pos;

   //Compare on the +X axis with a sphere of radius=3
   shape = new SphereShape(3);
   CollisionGroup group;
   group.addShape( shape );
   shape->globalReferential(ref);
   pos = shape->getFarthestPointAlong(dir);
   TEST_ADD( res, areEqual(pos, Vec3f(3, 0, 0)) );
   //Move sphere 1 unit in the X direction
   ref.position(1, 0, 0);
   shape->globalReferential(ref);
   pos = shape->getFarthestPointAlong(dir);
   TEST_ADD( res, areEqual(pos, Vec3f(4, 0, 0)) );
   //Flip the direction (should find opposite end of the sphere)
   dir = -dir;
   pos = shape->getFarthestPointAlong(dir);
   TEST_ADD( res, areEqual(pos, Vec3f(-2, 0, 0)) );
   //Make sure that the Y and Z component don't affect the X calculation
   ref.position(0, 4, 5);
   shape->globalReferential(ref);
   pos = shape->getFarthestPointAlong(dir);
   TEST_ADD( res, areEqual(pos, Vec3f(-3, 4, 5)) );
   //Make sure that a rotation doesn't affect the calculation
   ref.orientation( Quatf(0, 1, 3, 0.127f) );
   shape->globalReferential(ref);
   pos = shape->getFarthestPointAlong(dir);
   TEST_ADD( res, areEqual(pos, Vec3f(-3, 4, 5)) );
   //Calculate the diameter of the sphere
   ref.position(11, 23, -42);  //arbitrary position
   shape->globalReferential(ref);
   dir = Vec3f(-83, 17, 127); //arbitrary direction
   pos = shape->getFarthestPointAlong(dir) - shape->getFarthestPointAlong(-dir);
   TEST_ADD( res, areEqual(pos.length(), 2*3, 1e-5f) );

   //Reset everything
   ref = Reff::identity();

   //Compare on the +X axis with a cube of size (3, 4, 5)
   shape = new BoxShape(3, 4, 5);
   group.clearShapes();
   group.addShape( shape );
   ref.position(3, -10, -5);
   shape->globalReferential(ref);
   dir = Vec3f(1, 1, 1);
   pos = shape->getFarthestPointAlong(dir);
   printf("Pos %g %g %g\n", pos.x, pos.y, pos.z);
   TEST_ADD( res, areEqual(pos, Vec3f(4.5, -8, -2.5), Vec3f(0.2f)) );
   dir = -dir;
   pos = shape->getFarthestPointAlong(dir);
   printf("Pos %g %g %g\n", pos.x, pos.y, pos.z);
   TEST_ADD( res, areEqual(pos, Vec3f(1.5, -12, -7.5), Vec3f(0.2f)) );

   /**
   printf("shape: pos=(%g, %g, %g)\n",
          shape->globalReferential().position()(0), shape->globalReferential().position()(1), shape->globalReferential().position()(2));
   printf("dir=(%g, %g, %g)  pos=(%g, %g, %g)\n",
          dir(0), dir(1), dir(2),
          pos(0), pos(1), pos(2));
   **/
}

//------------------------------------------------------------------------------
//!
void test_collision_GJK_sphere_sphere( Test::Result& res )
{
   bool col;
   CollisionInfo  colInfo;
   const CollisionInfo::Contact* contact = NULL;

   Reff refA = Reff::identity();
   Reff refB = Reff::identity();

   //Make the same example as the UnderstandingODE_K.odg document.
   RCP<CollisionShape> a( new SphereShape(2) );
   RCP<CollisionShape> b( new SphereShape(5) );

   CollisionGroup grpA, grpB;
   grpA.addShape( a );
   grpB.addShape( b );

   //Start by keeping A at the origin for now
   a->globalReferential( refA );
   refB.position( Vec3f(7.01f, 0, 0) );
   b->globalReferential( refB );
   col = true; //FIXME CollisionShape::collideGJK(a, b, colInfo);
   TEST_ADD( res, !col );
   TEST_ADD( res, colInfo.numContacts() == 0 );

   //colInfo.clearContacts();

   refB.position( Vec3f(7, 0, 0) );
   b->globalReferential( refB );
   col = false; //FIXME CollisionShape::collideGJK(a, b, colInfo);
   TEST_ADD( res, col );
   TEST_ADD( res, colInfo.numContacts() == 1 );
   contact = &(colInfo.contact(0));
   TEST_ADD( res, areEqual(contact->worldPositionA(), Vec3f(+2.0, 0, 0)) );
   TEST_ADD( res, areEqual(contact->worldPositionB(), Vec3f(+2.0, 0, 0)) );
   TEST_ADD( res, areEqual(contact->worldNormal(), Vec3f(-1, 0, 0), 0.2f) );
   TEST_ADD( res, areEqual(contact->depth(), 0) );

   colInfo.clearContacts();

   refB.position( Vec3f(6, 0, 0) );
   b->globalReferential( refB );
   col = false; //FIXME CollisionShape::collideGJK(a, b, colInfo);
   TEST_ADD( res, col );
   TEST_ADD( res, colInfo.numContacts() == 1 );
   contact = &(colInfo.contact(0));
   TEST_ADD( res, areEqual(contact->worldPositionA(), Vec3f(+2.0, 0, 0)) );
   TEST_ADD( res, areEqual(contact->worldPositionB(), Vec3f(+1.0, 0, 0)) );
   TEST_ADD( res, areEqual(contact->worldNormal(), Vec3f(-1, 0, 0)) );
   TEST_ADD( res, areEqual(contact->depth(), 1) );

   colInfo.clearContacts();

   refB.position( Vec3f(5, 0, 0) );
   b->globalReferential( refB );
   col = false; //FIXME CollisionShape::collideGJK(a, b, colInfo);
   TEST_ADD( res, col );
   TEST_ADD( res, colInfo.numContacts() == 1 );
   contact = &(colInfo.contact(0));
   TEST_ADD( res, areEqual(contact->worldPositionA(), Vec3f(+2.0, 0, 0)) );
   TEST_ADD( res, areEqual(contact->worldPositionB(), Vec3f(+0.0, 0, 0)) );
   TEST_ADD( res, areEqual(contact->worldNormal(), Vec3f(-1, 0, 0)) );
   TEST_ADD( res, areEqual(contact->depth(), 2) );

   colInfo.clearContacts();

   refB.position( Vec3f(4, 0, 0) );
   b->globalReferential( refB );
   col = false; //FIXME CollisionShape::collideGJK(a, b, colInfo);
   TEST_ADD( res, col );
   TEST_ADD( res, colInfo.numContacts() == 1 );
   contact = &(colInfo.contact(0));
   TEST_ADD( res, areEqual(contact->worldPositionA(), Vec3f(+2.0, 0, 0)) );
   TEST_ADD( res, areEqual(contact->worldPositionB(), Vec3f(-1.0, 0, 0)) );
   TEST_ADD( res, areEqual(contact->worldNormal(), Vec3f(-1, 0, 0)) );
   TEST_ADD( res, areEqual(contact->depth(), 3) );

   colInfo.clearContacts();

   refB.position( Vec3f(3, 0, 0) );
   b->globalReferential( refB );
   col = false; //FIXME CollisionShape::collideGJK(a, b, colInfo);
   TEST_ADD( res, col );
   TEST_ADD( res, colInfo.numContacts() == 1 );
   contact = &(colInfo.contact(0));
   TEST_ADD( res, areEqual(contact->worldPositionA(), Vec3f(+2.0, 0, 0)) );
   TEST_ADD( res, areEqual(contact->worldPositionB(), Vec3f(-2.0, 0, 0)) );
   TEST_ADD( res, areEqual(contact->worldNormal(), Vec3f(-1, 0, 0)) );
   TEST_ADD( res, areEqual(contact->depth(), 4) );

   colInfo.clearContacts();

   refB.position( Vec3f(2, 0, 0) );
   b->globalReferential( refB );
   col = false; //FIXME CollisionShape::collideGJK(a, b, colInfo);
   TEST_ADD( res, col );
   TEST_ADD( res, colInfo.numContacts() == 1 );
   contact = &(colInfo.contact(0));
   TEST_ADD( res, areEqual(contact->worldPositionA(), Vec3f(+2.0, 0, 0)) );
   TEST_ADD( res, areEqual(contact->worldPositionB(), Vec3f(-3.0, 0, 0)) );
   TEST_ADD( res, areEqual(contact->worldNormal(), Vec3f(-1, 0, 0)) );
   TEST_ADD( res, areEqual(contact->depth(), 5) );

   colInfo.clearContacts();

   refB.position( Vec3f(1, 0, 0) );
   b->globalReferential( refB );
   col = false; //FIXME CollisionShape::collideGJK(a, b, colInfo);
   TEST_ADD( res, col );
   TEST_ADD( res, colInfo.numContacts() == 1 );
   contact = &(colInfo.contact(0));
   TEST_ADD( res, areEqual(contact->worldPositionA(), Vec3f(+2.0, 0, 0)) );
   TEST_ADD( res, areEqual(contact->worldPositionB(), Vec3f(-4.0, 0, 0)) );
   TEST_ADD( res, areEqual(contact->worldNormal(), Vec3f(-1, 0, 0)) );
   TEST_ADD( res, areEqual(contact->depth(), 6) );

   colInfo.clearContacts();

   //Special case (sharing centers)
   refB.position( Vec3f(0, 0, 0) );
   b->globalReferential( refB );
   col = false; //FIXME CollisionShape::collideGJK(a, b, colInfo);
   TEST_ADD( res, col );
   TEST_ADD( res, colInfo.numContacts() == 1 );
   contact = &(colInfo.contact(0));
   //TEST_ADD( res, areEqual(contact->worldPositionA(), Vec3f(-2.0, 0, 0)) );
   //TEST_ADD( res, areEqual(contact->worldPositionB(), Vec3f(+5.0, 0, 0)) );
   //TEST_ADD( res, areEqual(contact->worldNormal(), Vec3f(1, 0, 0)) );
   TEST_ADD( res, areEqual(contact->depth(), 7) );

   colInfo.clearContacts();

   //Start going out over the left side
   refB.position( Vec3f(-1, 0, 0) );
   b->globalReferential( refB );
   col = false; //FIXME CollisionShape::collideGJK(a, b, colInfo);
   TEST_ADD( res, col );
   TEST_ADD( res, colInfo.numContacts() == 1 );
   contact = &(colInfo.contact(0));
   TEST_ADD( res, areEqual(contact->worldPositionA(), Vec3f(-2.0, 0, 0)) );
   TEST_ADD( res, areEqual(contact->worldPositionB(), Vec3f(+4.0, 0, 0)) );
   TEST_ADD( res, areEqual(contact->worldNormal(), Vec3f(1, 0, 0)) );
   TEST_ADD( res, areEqual(contact->depth(), 6) );

   colInfo.clearContacts();

   refB.position( Vec3f(-2, 0, 0) );
   b->globalReferential( refB );
   col = false; //FIXME CollisionShape::collideGJK(a, b, colInfo);
   TEST_ADD( res, col );
   TEST_ADD( res, colInfo.numContacts() == 1 );
   contact = &(colInfo.contact(0));
   TEST_ADD( res, areEqual(contact->worldPositionA(), Vec3f(-2.0, 0, 0)) );
   TEST_ADD( res, areEqual(contact->worldPositionB(), Vec3f(+3.0, 0, 0)) );
   TEST_ADD( res, areEqual(contact->worldNormal(), Vec3f(1, 0, 0)) );
   TEST_ADD( res, areEqual(contact->depth(), 5) );

   colInfo.clearContacts();

   refB.position( Vec3f(-3, 0, 0) );
   b->globalReferential( refB );
   col = false; //FIXME CollisionShape::collideGJK(a, b, colInfo);
   TEST_ADD( res, col );
   TEST_ADD( res, colInfo.numContacts() == 1 );
   contact = &(colInfo.contact(0));
   TEST_ADD( res, areEqual(contact->worldPositionA(), Vec3f(-2.0, 0, 0)) );
   TEST_ADD( res, areEqual(contact->worldPositionB(), Vec3f(+2.0, 0, 0)) );
   TEST_ADD( res, areEqual(contact->worldNormal(), Vec3f(1, 0, 0)) );
   TEST_ADD( res, areEqual(contact->depth(), 4) );

   colInfo.clearContacts();

   refB.position( Vec3f(-4, 0, 0) );
   b->globalReferential( refB );
   col = false; //FIXME CollisionShape::collideGJK(a, b, colInfo);
   TEST_ADD( res, col );
   TEST_ADD( res, colInfo.numContacts() == 1 );
   contact = &(colInfo.contact(0));
   TEST_ADD( res, areEqual(contact->worldPositionA(), Vec3f(-2.0, 0, 0)) );
   TEST_ADD( res, areEqual(contact->worldPositionB(), Vec3f(+1.0, 0, 0)) );
   TEST_ADD( res, areEqual(contact->worldNormal(), Vec3f(1, 0, 0)) );
   TEST_ADD( res, areEqual(contact->depth(), 3) );

   colInfo.clearContacts();

   refB.position( Vec3f(-5, 0, 0) );
   b->globalReferential( refB );
   col = false; //FIXME CollisionShape::collideGJK(a, b, colInfo);
   TEST_ADD( res, col );
   TEST_ADD( res, colInfo.numContacts() == 1 );
   contact = &(colInfo.contact(0));
   TEST_ADD( res, areEqual(contact->worldPositionA(), Vec3f(-2.0, 0, 0)) );
   TEST_ADD( res, areEqual(contact->worldPositionB(), Vec3f(+0.0, 0, 0)) );
   TEST_ADD( res, areEqual(contact->worldNormal(), Vec3f(1, 0, 0)) );
   TEST_ADD( res, areEqual(contact->depth(), 2) );

   colInfo.clearContacts();

   refB.position( Vec3f(-6, 0, 0) );
   b->globalReferential( refB );
   col = false; //FIXME CollisionShape::collideGJK(a, b, colInfo);
   TEST_ADD( res, col );
   TEST_ADD( res, colInfo.numContacts() == 1 );
   contact = &(colInfo.contact(0));
   TEST_ADD( res, areEqual(contact->worldPositionA(), Vec3f(-2.0, 0, 0)) );
   TEST_ADD( res, areEqual(contact->worldPositionB(), Vec3f(-1.0, 0, 0)) );
   TEST_ADD( res, areEqual(contact->worldNormal(), Vec3f(1, 0, 0)) );
   TEST_ADD( res, areEqual(contact->depth(), 1) );

   colInfo.clearContacts();

   refB.position( Vec3f(-7, 0, 0) );
   b->globalReferential( refB );
   col = false; //FIXME CollisionShape::collideGJK(a, b, colInfo);
   TEST_ADD( res, col );
   TEST_ADD( res, colInfo.numContacts() == 1 );
   contact = &(colInfo.contact(0));
   TEST_ADD( res, areEqual(contact->worldPositionA(), Vec3f(-2.0, 0, 0)) );
   TEST_ADD( res, areEqual(contact->worldPositionB(), Vec3f(-2.0, 0, 0)) );
   TEST_ADD( res, areEqual(contact->worldNormal(), Vec3f(1, 0, 0)) );
   TEST_ADD( res, areEqual(contact->depth(), 0) );

   colInfo.clearContacts();

   refB.position( Vec3f(-7.01f, 0, 0) );
   b->globalReferential( refB );
   col = true; //FIXME CollisionShape::collideGJK(a, b, colInfo);
   TEST_ADD( res, !col );
   TEST_ADD( res, colInfo.numContacts() == 0 );

   /**
   printf("a: pos=(%g, %g, %g)   b: pos=(%g, %g, %g)\n",
          a->globalReferential().position()(0), a->globalReferential().position()(1), a->globalReferential().position()(2),
          b->globalReferential().position()(0), b->globalReferential().position()(1), b->globalReferential().position()(2));
   printf("contact: posA=(%g, %g, %g)  posB=(%g, %g, %g)  norm=(%g, %g, %g)  depth=%g\n",
          contact->worldPositionA()(0), contact->worldPositionA()(1), contact->worldPositionA()(2),
          contact->worldPositionB()(0), contact->worldPositionB()(1), contact->worldPositionB()(2),
          contact->worldNormal()(0), contact->worldNormal()(1), contact->worldNormal()(2),
          contact->depth());
   **/
}

//------------------------------------------------------------------------------
//!
void test_collision_GJK_sphere_box( Test::Result& res )
{
   bool col;
   CollisionInfo  colInfo;
   const CollisionInfo::Contact* contact = NULL;

   Reff refA = Reff::identity();
   Reff refB = Reff::identity();

   RCP<CollisionShape> a( new SphereShape(2) );
   RCP<CollisionShape> b( new BoxShape( 10, 4, 4 ) );

   CollisionGroup grpA, grpB;
   grpA.addShape( a );
   grpB.addShape( b );

   a->globalReferential( refA );
   b->globalReferential( refB );

   //Start by keeping A at the origin for now
   refB.position( Vec3f(7.01f, 0, 0) );
   b->globalReferential( refB );
   col = true; //FIXME CollisionShape::collideGJK(a, b, colInfo);
   TEST_ADD( res, !col );
   TEST_ADD( res, colInfo.numContacts() == 0 );

   refB.position( Vec3f(7, 0, 0) );
   b->globalReferential( refB );
   col = false; //FIXME CollisionShape::collideGJK(a, b, colInfo);
   TEST_ADD( res, col );
   TEST_ADD( res, colInfo.numContacts() == 1 );
   contact = &(colInfo.contact(0));
   TEST_ADD( res, areEqual(contact->worldPositionA(), Vec3f(+2.0, 0, 0)) );
   TEST_ADD( res, areEqual(contact->worldPositionB(), Vec3f(+2.0, 0, 0)) );
   TEST_ADD( res, areEqual(contact->worldNormal(), Vec3f(-1, 0, 0)) );
   TEST_ADD( res, areEqual( contact->depth(), 0) );

   colInfo.clearContacts();
   //Just check this functionality once
   TEST_ADD( res, colInfo.numContacts() == 0 );

   refB.position( Vec3f(6, 0.5, 0) );
   b->globalReferential( refB );
   col = false; //FIXME CollisionShape::collideGJK(a, b, colInfo);
   TEST_ADD( res, col );
   TEST_ADD( res, colInfo.numContacts() == 1 );
   contact = &(colInfo.contact(0));
   TEST_ADD( res, areEqual(contact->worldPositionA(), Vec3f(+2.0, 0, 0)) );
   TEST_ADD( res, areEqual(contact->worldPositionB(), Vec3f(+1.0, 0, 0)) );
   TEST_ADD( res, areEqual(contact->worldNormal(), Vec3f(-1, 0, 0)) );
   TEST_ADD( res, areEqual(contact->depth(), 1) );

   colInfo.clearContacts();

   refB.position( Vec3f(5, 0, 0) );
   b->globalReferential( refB );
   col = false; //FIXME CollisionShape::collideGJK(a, b, colInfo);
   TEST_ADD( res, col );
   TEST_ADD( res, colInfo.numContacts() == 1 );
   contact = &(colInfo.contact(0));
   TEST_ADD( res, areEqual(contact->worldPositionA(), Vec3f(+2.0, 0, 0)) );
   TEST_ADD( res, areEqual(contact->worldPositionB(), Vec3f(+0.0, 0, 0)) );
   TEST_ADD( res, areEqual(contact->worldNormal(), Vec3f(-1, 0, 0)) );
   TEST_ADD( res, areEqual(contact->depth(), 2) );

   colInfo.clearContacts();

   // Verify when origins overlap
   refA.position( Vec3f(12, 13, -17) );
   refB = refA;
   a->globalReferential( refA );
   b->globalReferential( refB );
   col = false; //FIXME CollisionShape::collideGJK(a, b, colInfo);
   TEST_ADD( res, col );
   // Contact point and direction could be multiple values,
   // but the shortest depth is 2+2=4
   contact = &(colInfo.contact(0));
   TEST_ADD( res, areEqual(contact->depth(), 4) );

   colInfo.clearContacts();

   // ... even with an arbitrary orientation
   refA.orientation( Quatf::axisAngle( Vec3f(0, 1, 3).normalize(), 0.127f ) );
   refB.orientation( Quatf::axisAngle( Vec3f(12, 13, -17).normalize(), -0.831f ) );
   a->globalReferential( refA );
   b->globalReferential( refB );
   col = false; //FIXME CollisionShape::collideGJK(a, b, colInfo);
   TEST_ADD( res, col );
   contact = &(colInfo.contact(0));
   //FIXME: TEST_ADD( res, areEqual(contact->depth(), 4) );

   colInfo.clearContacts();

   // Bring back both objects to the origin
   refA = Reff::identity();
   refB = Reff::identity();
   a->globalReferential( refA );
   b->globalReferential( refB );
   col = false; //FIXME CollisionShape::collideGJK(a, b, colInfo);
   TEST_ADD( res, col );
   contact = &(colInfo.contact(0));
   TEST_ADD( res, areEqual(contact->depth(), 4) );

   colInfo.clearContacts();

   // Prepare the cases below
   Vec3f boxCorner(5, 2, 2);
   float boxCornerDistance = boxCorner.length();
   float sphDistance = 2;
   float distance;

   /*        ___
    *       /   \
    *       \___/
    *  ____.
    *      |
    *      |
    */
   distance = (boxCornerDistance+sphDistance) * 1.1f;
   refA.position( boxCorner.getRescaled(distance) );
   a->globalReferential( refA );
   col = true; //FIXME CollisionShape::collideGJK(a, b, colInfo);
   TEST_ADD( res, !col );

   colInfo.clearContacts();

   /*       ___
    *      /   \
    *  ____\___/
    *      |
    *      |
    */
   distance = (boxCornerDistance+sphDistance) * 0.99f;
   refA.position( boxCorner.getRescaled(distance) );
   a->globalReferential( refA );
   col = false; //FIXME CollisionShape::collideGJK(a, b, colInfo);
   TEST_ADD( res, col );
   /**
   contact = &(colInfo.contact(0));
   TEST_ADD( res, areEqual(contact->worldPositionA(), boxCorner) );
   TEST_ADD( res, areEqual(contact->worldPositionB(), boxCorner) );
   TEST_ADD( res, areEqual(contact->worldNormal(), -boxCorner.getNormalized()) );
   TEST_ADD( res, areEqual(contact->depth(), 0) );
   printf("a: pos=(%g, %g, %g)   b: pos=(%g, %g, %g)\n",
          a->globalReferential().position()(0), a->globalReferential().position()(1), a->globalReferential().position()(2),
          b->globalReferential().position()(0), b->globalReferential().position()(1), b->globalReferential().position()(2));
   printf("contact: posA=(%g, %g, %g)  posB=(%g, %g, %g)  norm=(%g, %g, %g)  depth=%g\n",
          contact->worldPositionA()(0), contact->worldPositionA()(1), contact->worldPositionA()(2),
          contact->worldPositionB()(0), contact->worldPositionB()(1), contact->worldPositionB()(2),
          contact->worldNormal()(0), contact->worldNormal()(1), contact->worldNormal()(2),
          contact->depth());
   **/

   colInfo.clearContacts();

   /*      ___
    *  ___/.  \
    *     \|__/
    *      |
    */
   distance = (boxCornerDistance+sphDistance) * 0.99f;
   refA.position( boxCorner.getRescaled(distance) );
   a->globalReferential( refA );
   col = false; //FIXME CollisionShape::collideGJK(a, b, colInfo);
   TEST_ADD( res, col );

   colInfo.clearContacts();


   /*         ___
    *  ____. /   \
    *      | \___/
    *      |
    */
   refA.position( boxCorner + Vec3f(sphDistance+0.01f, 0, 0) );
   a->globalReferential( refA );
   col = true; //FIXME CollisionShape::collideGJK(a, b, colInfo);
   TEST_ADD( res, !col );

   colInfo.clearContacts();

   /*       ___
    *  ____/   \
    *      \___/
    *      |
    */
   refA.position( boxCorner + Vec3f(sphDistance+0.00f, 0, 0) );
   a->globalReferential( refA );
   col = false; //FIXME CollisionShape::collideGJK(a, b, colInfo);
   TEST_ADD( res, col );

   colInfo.clearContacts();

   /*      ___
    *  ___/.  \
    *     \|__/
    *      |
    */
    refA.position( boxCorner + Vec3f(sphDistance-0.01f, 0, 0) );
    a->globalReferential( refA );
    col = false; //FIXME CollisionShape::collideGJK(a, b, colInfo);
    TEST_ADD( res, col );

    colInfo.clearContacts();

   /*     ___
    *    /   \
    *    \___/
    *  ____.
    *      |
    *      |
    */
   refA.position( boxCorner + Vec3f(0, sphDistance+0.01f, 0) );
   a->globalReferential( refA );
   col = true; //FIXME CollisionShape::collideGJK(a, b, colInfo);
   TEST_ADD( res, !col );

   colInfo.clearContacts();

   /*     ___
    *    /   \
    *  __\_._/
    *      |
    *      |
    */
   refA.position( boxCorner + Vec3f(0, sphDistance+0.00f, 0) );
   a->globalReferential( refA );
   col = false; //FIXME CollisionShape::collideGJK(a, b, colInfo);
   TEST_ADD( res, col );

   colInfo.clearContacts();

   /*     ___
    *  __/_. \
    *    \_|_/
    *      |
    */
   refA.position( boxCorner + Vec3f(0, sphDistance-0.01f, 0) );
   a->globalReferential( refA );
   col = false; //FIXME CollisionShape::collideGJK(a, b, colInfo);
   TEST_ADD( res, col );

   colInfo.clearContacts();

   // Above Z
   refA.position( boxCorner + Vec3f(0, 0, sphDistance+0.01f) );
   a->globalReferential( refA );
   col = true; //FIXME CollisionShape::collideGJK(a, b, colInfo);
   TEST_ADD( res, !col );

   colInfo.clearContacts();

   // On Z
   refA.position( boxCorner + Vec3f(0, 0, sphDistance+0.00f) );
   a->globalReferential( refA );
   col = false; //FIXME CollisionShape::collideGJK(a, b, colInfo);
   TEST_ADD( res, col );

   colInfo.clearContacts();

   // Below Z
   refA.position( boxCorner + Vec3f(0, 0, sphDistance-0.01f) );
   a->globalReferential( refA );
   col = false; //FIXME CollisionShape::collideGJK(a, b, colInfo);
   TEST_ADD( res, col );
}


//------------------------------------------------------------------------------
//!
void test_collision_GJK_box_box( Test::Result& res )
{
   bool col;
   CollisionInfo  colInfo;
   //const CollisionInfo::Contact* contact = NULL;

   Reff refA = Reff::identity();
   Reff refB = Reff::identity();

   RCP<CollisionShape> a( new BoxShape( 10,  4, 14 ) );
   RCP<CollisionShape> b( new BoxShape(  2,  2,  2 ) );

   CollisionGroup grpA, grpB;
   grpA.addShape( a );
   grpB.addShape( b );

   a->globalReferential( refA );
   b->globalReferential( refB );

   // Some temporaries for the cases below
   Vec3f cornerA(5, 2, 7);
   Vec3f cornerB(1, 1, 1);
   Vec3f cornerApB = cornerA + cornerB;
   float distance = cornerApB.length();
   Vec3f pos;

   //Start by keeping A at the origin for now
   refB.position( Vec3f(6+0.01f, 0, 0) );
   b->globalReferential( refB );
   col = true; //FIXME CollisionShape::collideGJK(a, b, colInfo);
   TEST_ADD( res, !col );

   refB.position( Vec3f(6, 0, 0) );
   b->globalReferential( refB );
   col = false; //FIXME CollisionShape::collideGJK(a, b, colInfo);
   TEST_ADD( res, col );

   colInfo.clearContacts();

   refB.position( Vec3f(5.9999f, 0, 0) );
   b->globalReferential( refB );
   col = false; //FIXME CollisionShape::collideGJK(a, b, colInfo);
   TEST_ADD( res, col );

   colInfo.clearContacts();

   refB.position( Vec3f(5, 0, 0) );
   b->globalReferential( refB );
   col = false; //FIXME CollisionShape::collideGJK(a, b, colInfo);
   TEST_ADD( res, col );

   colInfo.clearContacts();

   refB.position( Vec3f(0, 0, 0) );
   b->globalReferential( refB );
   col = false; //FIXME CollisionShape::collideGJK(a, b, colInfo);
   TEST_ADD( res, col );

   colInfo.clearContacts();

   refB.position( Vec3f(-5, 0, 0) );
   b->globalReferential( refB );
   col = false; //FIXME CollisionShape::collideGJK(a, b, colInfo);
   TEST_ADD( res, col );

   colInfo.clearContacts();

   refB.position( Vec3f(-5.9999f, 0, 0) );
   b->globalReferential( refB );
   col = false; //FIXME CollisionShape::collideGJK(a, b, colInfo);
   TEST_ADD( res, col );

   colInfo.clearContacts();

   refB.position( Vec3f(-6, 0, 0) );
   b->globalReferential( refB );
   col = false; //FIXME CollisionShape::collideGJK(a, b, colInfo);
   TEST_ADD( res, col );

   colInfo.clearContacts();

   refB.position( Vec3f(-6.01f, 0, 0) );
   b->globalReferential( refB );
   col = true; //FIXME CollisionShape::collideGJK(a, b, colInfo);
   TEST_ADD( res, !col );

   colInfo.clearContacts();

   /*        |  b
    *        .___
    *  ____.
    *      |
    *  a   |
    */
   pos = cornerApB.getRescaled(distance*1.001f);
   refB.position( pos );
   b->globalReferential( refB );
   col = true; //FIXME CollisionShape::collideGJK(a, b, colInfo);
   TEST_ADD( res, !col );

   colInfo.clearContacts();

   /*      |  b
    *  ____.___
    *      |
    *  a   |
    */
   pos = cornerApB;
   refB.position( pos );
   b->globalReferential( refB );
   col = true; //FIXME CollisionShape::collideGJK(a, b, colInfo);
   TEST_ADD( res, !col );

   colInfo.clearContacts();

   /*    |  b
    *  __|_.
    *    ._|__
    *  a   |
    */
   pos = cornerApB * 0.99f;
   refB.position( pos );
   b->globalReferential( refB );
   col = false; //FIXME CollisionShape::collideGJK(a, b, colInfo);
   TEST_ADD( res, col );

   colInfo.clearContacts();

   /*         ____
    *  ____. |  b
    *      | |____
    *  a   |
    */
   pos = cornerA + Vec3f( cornerB(0) + 0.01f, 0, 0 );
   refB.position( pos );
   b->globalReferential( refB );
   col = true; //FIXME CollisionShape::collideGJK(a, b, colInfo);
   TEST_ADD( res, !col );

   colInfo.clearContacts();

   /*       ____
    *  ____|  b
    *      |____
    *  a   |
    */
   pos = cornerA + Vec3f( cornerB(0), 0, 0 );
   refB.position( pos );
   b->globalReferential( refB );
   col = false; //FIXME CollisionShape::collideGJK(a, b, colInfo);
   TEST_ADD( res, col );

   colInfo.clearContacts();

   /*     _____
    *  __|_. b
    *    |_|___
    *  a   |
    */
   pos = cornerA + Vec3f( cornerB(0) - 0.0001f, 0, 0 );
   refB.position( pos );
   b->globalReferential( refB );
   col = false; //FIXME CollisionShape::collideGJK(a, b, colInfo);
   TEST_ADD( res, col );

   colInfo.clearContacts();

   /*    |b  |
    *    |___|
    *  ____.
    *      |
    *  a   |
    */
   pos = cornerA + Vec3f( 0, cornerB(1) + 0.01f, 0 );
   refB.position( pos );
   b->globalReferential( refB );
   col = true; //FIXME CollisionShape::collideGJK(a, b, colInfo);
   TEST_ADD( res, !col );

   colInfo.clearContacts();

   /*    |b  |
    *    |   |
    *  __|_._|
    *      |
    *  a   |
    */
   pos = cornerA + Vec3f( 0, cornerB(1), 0 );
   refB.position( pos );
   b->globalReferential( refB );
   col = false; //FIXME CollisionShape::collideGJK(a, b, colInfo);
   TEST_ADD( res, col );

   colInfo.clearContacts();

   /*    |b  |
    *  __|_. |
    *    |_|_|
    *  a   |
    */
   pos = cornerA + Vec3f( 0, cornerB(1) - 0.0001f, 0 );
   refB.position( pos );
   b->globalReferential( refB );
   col = false; //FIXME CollisionShape::collideGJK(a, b, colInfo);
   TEST_ADD( res, col );

   colInfo.clearContacts();

   // Over Z
   pos = cornerA + Vec3f( 0, 0, cornerB(1) + 0.01f );
   refB.position( pos );
   b->globalReferential( refB );
   col = true; //FIXME CollisionShape::collideGJK(a, b, colInfo);
   TEST_ADD( res, !col );

   colInfo.clearContacts();

   // On Z
   pos = cornerA + Vec3f( 0, 0, cornerB(1) );
   refB.position( pos );
   b->globalReferential( refB );
   col = false; //FIXME CollisionShape::collideGJK(a, b, colInfo);
   TEST_ADD( res, col );

   colInfo.clearContacts();

   // Under Z
   pos = cornerA + Vec3f( 0, 0, cornerB(1) - 0.0001f );
   refB.position( pos );
   b->globalReferential( refB );
   col = false; //FIXME CollisionShape::collideGJK(a, b, colInfo);
   TEST_ADD( res, col );

   colInfo.clearContacts();

   // Verify when origins overlap
   refA.position( Vec3f(12, 13, -17) );
   refB = refA;
   col = false; //FIXME CollisionShape::collideGJK(a, b, colInfo);
   TEST_ADD( res, col );

   colInfo.clearContacts();

   // ... even with an arbitrary orientation
   refA.orientation( Quatf(0, 1, 3, 0.127f) );
   refB.orientation( Quatf(12, 13, -17, -0.831f) );
   col = false; //FIXME CollisionShape::collideGJK(a, b, colInfo);
   TEST_ADD( res, col );

   colInfo.clearContacts();

   // Bring back both objects to the origin
   refA = Reff::identity();
   refB = Reff::identity();
   a->globalReferential( refA );
   b->globalReferential( refB );
   col = false; //FIXME CollisionShape::collideGJK(a, b, colInfo);
   TEST_ADD( res, col );

   colInfo.clearContacts();

   // Rotate B (small square box) 45deg
   Quatf q = Quatf::axisAngle( Vec3f(0, 0, 1), CGM::degToRad(45.0f) );
   distance = CGM::sqrt(1.0f + 1.0f);  //distance from center to origin
   refB.orientation( q );
   b->globalReferential( refB );
   col = false; //FIXME CollisionShape::collideGJK(a, b, colInfo);
   TEST_ADD( res, col );

   colInfo.clearContacts();

   // Move up until while it still touches
   pos = Vec3f(0, cornerA(1) + distance - 0.01f, 0);
   refB.position( pos );
   b->globalReferential( refB );
   col = false; //FIXME CollisionShape::collideGJK(a, b, colInfo);
   TEST_ADD( res, col );

   colInfo.clearContacts();

   // Move up until it barely touches
   pos = Vec3f(0, cornerA(1) + distance, 0);
   refB.position( pos );
   b->globalReferential( refB );
   col = true; //FIXME CollisionShape::collideGJK(a, b, colInfo);
   TEST_ADD( res, !col );

   colInfo.clearContacts();

   // Move up until it barely no longer touches
   pos = Vec3f(0, cornerA(1) + distance + 0.01f, 0);
   refB.position( pos );
   b->globalReferential( refB );
   col = true; //FIXME CollisionShape::collideGJK(a, b, colInfo);
   TEST_ADD( res, !col );

   colInfo.clearContacts();

   // Move rotated cube on the left side
   // Move it to the end until it barely touches (edge-edge)
   pos = Vec3f(-cornerA(0), cornerA(1) + distance*0.99f, 0);
   refB.position( pos );
   b->globalReferential( refB );
   col = false; //FIXME CollisionShape::collideGJK(a, b, colInfo);
   TEST_ADD( res, col );

   colInfo.clearContacts();

   // Move it to the end until it no longer touches (edge-edge)
   pos = Vec3f(-cornerA(0) - distance - 0.0001f, cornerA(1) + distance, 0);
   refB.position( pos );
   b->globalReferential( refB );
   col = true; //FIXME CollisionShape::collideGJK(a, b, colInfo);
   TEST_ADD( res, !col );

   colInfo.clearContacts();

   // Move it to the (face-edge) case
   pos = Vec3f(-cornerA(0) - 0.5f * distance, cornerA(1) + 0.49f*distance, 0);
   refB.position( pos );
   b->globalReferential( refB );
   col = false; //FIXME CollisionShape::collideGJK(a, b, colInfo);
   TEST_ADD( res, col );

   colInfo.clearContacts();

   // Move it so that it barely doesn't touch
   pos(0) -= 0.01f;
   refB.position( pos );
   b->globalReferential( refB );
   col = true; //FIXME CollisionShape::collideGJK(a, b, colInfo);
   TEST_ADD( res, !col );
}

//------------------------------------------------------------------------------
//!
void test_gravitational_attractor( Test::Result& res )
{
   Vec3f sum;
   RCP<MotionWorld>  world( new MotionWorld() );
   RCP<GravitationalAttractor> grav( new GravitationalAttractor() );

   RCP<RigidBody> a = world->createRigidBody();
   RCP<RigidBody> b = world->createRigidBody();
   RCP<RigidBody> c = world->createRigidBody();
   a->mass(1e6f);
   b->mass(1e6f);
   c->mass(1e6f);

   Vector< RCP<RigidBody> >  bodies;
   bodies.pushBack(a);
   bodies.pushBack(b);
   bodies.pushBack(c);

   a->position( Vec3f(0, 0, 0) );
   b->position( Vec3f(1, 0, 0) );
   c->position( Vec3f(1, 1, 0) );

   grav->addForce( bodies );

   TEST_ADD( res, a->totalForce()(0) > 0.0f );
   TEST_ADD( res, b->totalForce()(0) < 0.0f );
   TEST_ADD( res, c->totalForce()(0) < 0.0f );
   TEST_ADD( res, a->totalForce()(0) > -b->totalForce()(0) );
   TEST_ADD( res, b->totalForce()(0) <  c->totalForce()(0) );

   TEST_ADD( res, a->totalForce()(1) > 0.0f );
   TEST_ADD( res, b->totalForce()(1) > 0.0f );
   TEST_ADD( res, c->totalForce()(1) < 0.0f );
   TEST_ADD( res, a->totalForce()(1) <  b->totalForce()(1) );
   TEST_ADD( res, b->totalForce()(1) < -c->totalForce()(1) );

   TEST_ADD( res, areEqual(a->totalForce()(2),  0.0f) );
   TEST_ADD( res, areEqual(b->totalForce()(2),  0.0f) );
   TEST_ADD( res, areEqual(c->totalForce()(2),  0.0f) );

   TEST_ADD( res, areEqual(a->totalForce()(0), -c->totalForce()(1)) );
   TEST_ADD( res, areEqual(a->totalForce()(1), -c->totalForce()(0)) );
   TEST_ADD( res, areEqual(b->totalForce()(0), -b->totalForce()(1)) );

   sum = a->totalForce() + b->totalForce() + c->totalForce();
   TEST_ADD( res, areEqual(sum, Vec3f::zero()) );

   // Reset forces.
   a->addForce( -a->totalForce() );
   b->addForce( -b->totalForce() );
   c->addForce( -c->totalForce() );

   c->position( Vec3f(0.5f, CGM::sqrt(1.0f-(0.5f*0.5f)), 0.0f) );
   grav->addForce( bodies );

   TEST_ADD( res, a->totalForce()(0) > 0.0f );
   TEST_ADD( res, b->totalForce()(0) < 0.0f );
   TEST_ADD( res, areEqual(c->totalForce()(0),  0.0f) );
   TEST_ADD( res, areEqual(a->totalForce()(0), -b->totalForce()(0)) );

   TEST_ADD( res, a->totalForce()(1) > 0.0f );
   TEST_ADD( res, b->totalForce()(1) > 0.0f );
   TEST_ADD( res, c->totalForce()(1) < 0.0f );
   TEST_ADD( res, areEqual(a->totalForce()(1),  b->totalForce()(1)) );

   TEST_ADD( res, areEqual(a->totalForce()(2),  0.0f) );
   TEST_ADD( res, areEqual(b->totalForce()(2),  0.0f) );
   TEST_ADD( res, areEqual(c->totalForce()(2),  0.0f) );

   sum = a->totalForce() + b->totalForce() + c->totalForce();
   TEST_ADD( res, areEqual(sum, Vec3f::zero()) );

   grav->addForce( bodies );

   // Reset forces.
   a->addForce( -a->totalForce() );
   b->addForce( -b->totalForce() );
   c->addForce( -c->totalForce() );
}

//------------------------------------------------------------------------------
//!
void test_debug( Test::Result& res )
{
   bool col;
   CollisionInfo  colInfo;
   const CollisionInfo::Contact* contact = NULL;

   Reff refA = Reff::identity();
   Reff refB = Reff::identity();

   //Make the same example as the UnderstandingODE_K.odg document.
   //RCP<CollisionShape> a( new SphereShape(2) );
   //RCP<CollisionShape> b( new SphereShape(5) );
   
   RCP<CollisionShape> a( new SphereShape(2) );
   RCP<CollisionShape> b( new BoxShape( 10, 4, 4 ) );

   CollisionGroup grpA, grpB;
   grpA.addShape( a );
   grpB.addShape( b );

   // Prepare the cases below
   Vec3f boxCorner(5, 2, 2);
   float boxCornerDistance = boxCorner.length();
   float sphDistance = 2;
   float distance;

   /*       ___
    *      /   \
    *  ____\___/
    *      |
    *      |
    */
   distance = (boxCornerDistance+sphDistance) * 0.999f;
   refA.position( boxCorner.getRescaled(distance) );
   a->globalReferential( refA );
   col = false; //FIXME CollisionShape::collideGJK(a, b, colInfo);
   TEST_ADD( res, col );
   
   contact = &(colInfo.contact(0));
   TEST_ADD( res, areEqual(contact->worldPositionA(), boxCorner) );
   TEST_ADD( res, areEqual(contact->worldPositionB(), boxCorner) );
   TEST_ADD( res, areEqual(contact->worldNormal(), -boxCorner.getNormalized()) );
   TEST_ADD( res, areEqual(contact->depth(), 0) );
   
   /*
   
   //Special case (sharing centers)
   refB.position( Vec3f(7.0, 0, 0) );
   b->globalReferential( refB );
   col = false; //FIXME CollisionShape::collideGJK(a, b, colInfo);
   TEST_ADD( res, col );
   TEST_ADD( res, colInfo.numContacts() == 1 );
   contact = &(colInfo.contact(0));
   //TEST_ADD( res, areEqual(contact->worldPositionA(), Vec3f(-2.0, 0, 0)) );
   //TEST_ADD( res, areEqual(contact->worldPositionB(), Vec3f(+5.0, 0, 0)) );
   //TEST_ADD( res, areEqual(contact->worldNormal(), Vec3f(1, 0, 0)) );
   TEST_ADD( res, areEqual(contact->depth(), 0) );
   printf("DEPTH: %g\n", contact->depth());
   */
}

//------------------------------------------------------------------------------
//!
void init_tests()
{
   Test::Collection& std = Test::standard();
   std.add( new Test::Function( "sph_sph"    , "Sphere-Sphere"           , test_collision_sphere_sphere     ) );
   std.add( new Test::Function( "sph_box"    , "Sphere-Box"              , test_collision_sphere_box        ) );
   std.add( new Test::Function( "supp"       , "Support function"        , test_collision_shape_support     ) );
   std.add( new Test::Function( "gjk_sph_sph", "GJK Sphere-Sphere"       , test_collision_GJK_sphere_sphere ) );
   std.add( new Test::Function( "gjk_sph_box", "GJK Sphere-Box"          , test_collision_GJK_sphere_box    ) );
   std.add( new Test::Function( "gjk_box_box", "GJK Box-Box"             , test_collision_GJK_box_box       ) );
   std.add( new Test::Function( "grav"       , "Gravitational attractors", test_gravitational_attractor     ) );
   std.add( new Test::Function( "dbg"        , "DEBUG"                   , test_debug                       ) );
}
#endif
//------------------------------------------------------------------------------
//!
int
main( int argc, char* argv[] )
{
   //init_tests();
   return Test::main( argc, argv );
}
