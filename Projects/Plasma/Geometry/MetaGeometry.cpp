/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/Geometry/MetaGeometry.h>
#include <Plasma/Geometry/MeshGeometry.h>
#include <Plasma/Intersector.h>

#include <Fusion/Core/Core.h>

#include <CGMath/RayCaster.h>

#include <Base/Dbg/DebugStream.h>

#include <algorithm>

//#define BLOCKS
//#define CONNECTION
//#define SUBFACE_MAPPING

/*==============================================================================
   UNNAME NAMESPACE
==============================================================================*/

UNNAMESPACE_BEGIN

DBG_STREAM( os_b, "MetaGeometry" );

const float _serror = 0.0001f;

//------------------------------------------------------------------------------
//! 
const uint faceVertex[6][4] =
{
   {0,4,6,2},
   {5,1,3,7},
   {0,1,5,4},
   {6,7,3,2},
   {1,0,2,3},
   {4,5,7,6}
};

const uint faceEdge[6][4] =
{
   {8,5,11,4},
   {9,7,10,6},
   {0,9,3,8},
   {2,10,1,11},
   {0,4,1,7},
   {3,6,2,5}
};

const Vec2f faceUV[4] = 
{ 
   Vec2f(0.0f, 0.0f), Vec2f(1.0f,0.0f), Vec2f(1.0f,1.0f), Vec2f(0.0f,1.0f) 
};

const float subFracs[5][4] =
{
   { 1.0f, 1.0f, 1.0f, 1.0f },
   { 1.0f, 1.0f, 1.0f, 1.0f },
   { 0.5f, 1.0f, 1.0f, 1.0f },
   { 1.0f/3.0f, 2.0f/3.0f, 1.0f, 1.0f },
   { 0.25f, 0.5f, 0.75f, 1.0f },
};

#if 0
//------------------------------------------------------------------------------
//! 
void computeNormals( Vector<float>& vertices, Vector<uint>& indices )
{
   // Clear normals.
   for( uint i = 0; i < vertices.size(); i+=8 )
   {
      vertices[i+5] = 0.0f;
      vertices[i+6] = 0.0f;
      vertices[i+7] = 0.0f;
   }

   // Add face normals.
   for( uint f = 0; f < indices.size(); f+=3 )
   {
      uint i0 = indices[f];
      uint i1 = indices[f+1];
      uint i2 = indices[f+2];

      Vec3f p0( vertices[i0*8], vertices[i0*8+1], vertices[i0*8+2] );
      Vec3f p1( vertices[i1*8], vertices[i1*8+1], vertices[i1*8+2] );
      Vec3f p2( vertices[i2*8], vertices[i2*8+1], vertices[i2*8+2] );

      Vec3f n = cross( p2-p1, p0-p1 );

      vertices[i0*8+5] += n.x;
      vertices[i0*8+6] += n.y;
      vertices[i0*8+7] += n.z;

      vertices[i1*8+5] += n.x;
      vertices[i1*8+6] += n.y;
      vertices[i1*8+7] += n.z;

      vertices[i2*8+5] += n.x;
      vertices[i2*8+6] += n.y;
      vertices[i2*8+7] += n.z;
   }

   // Normalization.
   for( uint i = 0; i < vertices.size(); i+=8 )
   {
      Vec3f n( vertices[i+5], vertices[i+6], vertices[i+7] );
      n.normalize();
      vertices[i+5] = n.x;
      vertices[i+6] = n.y;
      vertices[i+7] = n.z;
   }
}
#endif

//------------------------------------------------------------------------------
//! 
bool materialSort( MetaSurface::Patch* a, MetaSurface::Patch* b )
{
   return (a->_id & 0x1fffffff) < (b->_id & 0x1fffffff);
}

//------------------------------------------------------------------------------
//! 
inline Vec3f
direction( const Vec3f& p0, const Vec3f& p1, const Vec3f& p2, const Vec3f& p3 )
{
   Vec3f tanu = (p1-p0)*0.5f + (p2-p3)*0.5f;
   Vec3f tanv = (p3-p0)*0.5f + (p2-p1)*0.5f;
   return normalize( tanu.cross( tanv ) );
}

//------------------------------------------------------------------------------
//! 
bool 
intersect( 
   const Vec3f& pos, 
   const Vec3f& dir, 
   const Vec3f& p0, 
   const Vec3f& p1, 
   const Vec3f& p2, 
   const Vec3f& p3, 
   Vec3f&       ipos 
)
{
   Intersector::Hit hit;
   hit._tmin = -0.1f;
   Rayf ray( pos, dir );
   if( Intersector::trace( p0, p1, p2, ray, hit ) )
   {
      ipos = hit._pos;
      return true;
   }
   if( Intersector::trace( p0, p2, p3, ray, hit ) )
   {
      ipos = hit._pos;
      return true;
   }
   return false;
}

//------------------------------------------------------------------------------
//! 
void
link( MetaBlock::Face* fa, MetaBlock::Face* fb, const Vec3f& ca, const Vec3f& cb )
{
   float dist = (ca-cb).sqrLength();

   if( dist > (fa->force() + fb->force()) ) return;

   // Testing existant link on face a.
   if( fa->_link && ( dist > (ca-fa->_link->center()).sqrLength() ) ) return;

   // Testing existant link on face b.
   if( fb->_link && ( dist > (cb-fb->_link->center()).sqrLength() ) ) return;

   // Establish link.
   fa->_link = fb;
   fb->_link = fa;

   // Compute the orientation between faces.
   dist = CGConstf::infinity();

   for( uint ca = 0; ca < 4; ++ca )
   {
      Vec3f a = fa->_corners[ca].position();

      for( uint cb = 0; cb < 4; ++cb )
      {
         float d = (fb->_corners[cb].position()-a).sqrLength();
         if( d < dist )
         {
            dist            = d;
            fa->_linkCorner = (cb+ca)%4;
         }
      }
   }

   fb->_linkCorner = fa->_linkCorner;
}

//------------------------------------------------------------------------------
//! 
inline void
neighbor( MetaBlock::HEdge& e0, MetaBlock::HEdge& e1 )
{
   e0._neighbor = &e1;
   e1._neighbor = &e0;
}


//------------------------------------------------------------------------------
//! 
void 
blocksOverlap( void* /*data*/, void* objA, void* objB )
{
   MetaBlock* a = (MetaBlock*)objA;
   MetaBlock* b = (MetaBlock*)objB;

   // Check if blocks can attract each other.
   if( a->group() != b->group() ) return;
   if( !a->group()->areAttracted( a->groupID(), b->groupID() ) ) return;

   Vec3f ca = a->center();
   Vec3f cb = b->center();

   // Compute sides directions.
   Vec3f dirA[6];
   Vec3f dirB[6];
   for( uint s = 0; s < 6; ++s )
   {
      dirA[s] = direction( 
         a->position( faceVertex[s][0] ), 
         a->position( faceVertex[s][1] ), 
         a->position( faceVertex[s][2] ), 
         a->position( faceVertex[s][3] )
      );
      dirB[s] = direction( 
         b->position( faceVertex[s][0] ), 
         b->position( faceVertex[s][1] ), 
         b->position( faceVertex[s][2] ), 
         b->position( faceVertex[s][3] )
      );
   }

   // Check for links.
   for( uint i = 0; i < 6; ++i )
   {
      int subA = a->subdivision(i);

      // Try connecting side a to side or face b.
      if( subA == 0 )
      {
         // Compute side A center.
         Vec3f csa = a->side(i).center();

         // Test each side of b.
         for( uint j = 0; j < 6; ++j )
         {
            // Not seeing each other?
            if( dot( dirA[i], dirB[j] ) >= 0.0f ) continue;

            Vec3f b0 = b->position( faceVertex[j][0] );
            Vec3f b1 = b->position( faceVertex[j][1] );
            Vec3f b2 = b->position( faceVertex[j][2] );
            Vec3f b3 = b->position( faceVertex[j][3] );

            int subB = b->subdivision(j);

            // Try connecting side to side.
            if( subB == 0 )
            {
               Vec3f csb = b->side(j).center();
               Planef planA( dirA[i], (ca+csa)*0.5f );
               Planef planB( dirB[j], (cb+csb)*0.5f );
               if( planA.distance( csb ) < 0.0f ) continue;
               if( planB.distance( csa ) < 0.0f ) continue;
               Vec3f ipos;
               Vec3f a0 = a->position( faceVertex[i][0] );
               Vec3f a1 = a->position( faceVertex[i][1] );
               Vec3f a2 = a->position( faceVertex[i][2] );
               Vec3f a3 = a->position( faceVertex[i][3] );
               if( !intersect( csa, dirA[i], b0, b1, b2, b3, ipos ) ) continue;
               if( !intersect( csb, dirB[j], a0, a1, a2, a3, ipos ) ) continue;
               link( &a->side(i), &b->side(j), csa, csb );
            }
            // Try connecting side to face.
            else
            {
               // Can current sides of a and b links?
               Vec3f ipos;
               if( intersect( csa, dirA[i], b0, b1, b2, b3, ipos ) )
               {
                  // Find the nearest face.
                  MetaBlock::Face* f  = &b->side(j);
                  MetaBlock::Face* fb = 0;
                  float dist          = CGConstf::infinity();
                  Vec3f csb;
                  while( f )
                  {
                     Vec3f cc = f->center();
                     float d  = (cc-ipos).sqrLength();
                     if( d < dist )
                     {
                        dist = d;
                        fb   = f;
                        csb  = cc;
                     }
                     f = f->_next;
                  }
                  link( &a->side(i), fb, csa, csb );
               }
            }
         }
      }
      // Try connecting face to side.
      else
      {
         Vec3f a0 = a->position( faceVertex[i][0] );
         Vec3f a1 = a->position( faceVertex[i][1] );
         Vec3f a2 = a->position( faceVertex[i][2] );
         Vec3f a3 = a->position( faceVertex[i][3] );

         // Test each side of b.
         for( uint j = 0; j < 6; ++j )
         {
            // Can't connect side to side.
            int subB = b->subdivision(j);
            if( subB != 0 ) continue;
            // Not seeing each other?
            if( dot( dirA[i], dirB[j] ) >= 0.0f ) continue;

            // Computer side center.
            Vec3f csb = b->side(j).center();

            // Can current sides of a and b links?
            Vec3f ipos;
            if( intersect( csb, dirB[j], a0, a1, a2, a3, ipos ) )
            {
               // Find the nearest face.
               MetaBlock::Face* f  = &a->side(i);
               MetaBlock::Face* fa = 0;
               float dist          = CGConstf::infinity();
               Vec3f csa;
               while( f )
               {
                  Vec3f cc = f->center();
                  float d = (cc-ipos).sqrLength();
                  if( d < dist )
                  {
                     dist = d;
                     fa   = f;
                     csa  = cc;
                  }
                  f = f->_next;
               }
               link( &b->side(j), fa, csb, csa );
            }
         }
      }
   }
}

//------------------------------------------------------------------------------
//! 
inline MetaBlock* block( MetaSurface::Patch* p )
{
   MetaBlock::HEdge* he = (MetaBlock::HEdge*)p->_data;
   return he->_face->_block;
}

//------------------------------------------------------------------------------
//! 
void
patchesOverlap( void* data, void* objA, void* objB )
{
   MetaSurface::Patch* pA = (MetaSurface::Patch*)objA;
   MetaSurface::Patch* pB = (MetaSurface::Patch*)objB;
   MetaBlock* blockA      = block( pA );
   MetaBlock* blockB      = block( pB );

   // Two merged blocks can't do CSG.
   if( blockA == blockB ) return;
   if( blockA->group() == blockB->group() )
   {
      if( blockA->group()->areAttracted( blockA->groupID(), blockB->groupID() ) ) return;
   }
   MetaSurface* surface = (MetaSurface*)data;
   surface->trim( *pA, *pB );
}

//------------------------------------------------------------------------------
//! 
struct ClassifyCSG
{
   AABBoxf          _region;
   int              _count;
   int              _axis;
   Rayf             _ray;
   Intersector::Hit _hit2;
   RayCaster::Hitf  _hit;
};

//------------------------------------------------------------------------------
//! 
void
subpatchesCSG( void* /*data*/, void* objA, void* objB )
{
   DBG_BLOCK( os_b, "subpatchesCSG" );
   MetaSurface::Subpatch* sp = (MetaSurface::Subpatch*)objA;
   ClassifyCSG* cl           = (ClassifyCSG*)objB;
   MetaBlocks* group         = block( sp->_patch )->group();

   // FIXME: move the detection into MetaSurface.
   // Detect which "big triangle" we need to triangulate.
   const Vec3f& a  = sp->_corners[0]->_pos;
   const Vec3f& b  = sp->_corners[1]->_pos;
   const Vec3f& c  = sp->_corners[2]->_pos;
   const Vec3f& d  = sp->_corners[3]->_pos;

   Vec3f ab = b - a;
   Vec3f ac = c - a;
   Vec3f ad = d - a;
   Vec3f n0 = cross(ab,ac);
   Vec3f n1 = cross(ac,ad);

   // triangles (0,1,2),(0,2,3).
   if( dot(n0,n1) > 0.0f )
   {
      // Triangulate/intersect triangle (0,1,2).
      MetaSurface::Vertex* v1 = sp->_corners[1];
      MetaSurface::Vertex* v0 = v1->_next[3];
      MetaSurface::Vertex* v2 = v1;
      for( ; v2 != sp->_corners[2]; v1 = v2 )
      {
         v2 = v2->_next[2];
         cl->_hit._t = CGConstf::infinity();
         if( RayCaster::hit( cl->_ray, v0->_pos, v1->_pos, v2->_pos, cl->_hit ) )
         {
            group->addIntersection( cl->_hit._t-_serror, cl->_hit._backFacing );
         }
      }
      v0 = sp->_corners[0];
      for( v1 = v0->_next[1]; v1 != sp->_corners[1]; v0 = v1, v1 = v1->_next[1] )
      {
         cl->_hit._t = CGConstf::infinity();
         if( RayCaster::hit( cl->_ray, v0->_pos, v1->_pos, v2->_pos, cl->_hit ) )
         {
            group->addIntersection( cl->_hit._t-_serror, cl->_hit._backFacing );
         }
      }
      // Triangulate/intersect triangle (0,2,3).
      v1 = sp->_corners[3];
      v0 = v1->_next[1];
      v2 = v1;
      for( ; v2 != sp->_corners[0]; v1 = v2 )
      {
         v2 = v2->_next[0];
         cl->_hit._t = CGConstf::infinity();
         if( RayCaster::hit( cl->_ray, v0->_pos, v1->_pos, v2->_pos, cl->_hit ) )
         {
            group->addIntersection( cl->_hit._t-_serror, cl->_hit._backFacing );
         }
      }
      v0 = sp->_corners[2];
      for( v1 = v0->_next[3]; v1 != sp->_corners[3]; v0 = v1, v1 = v1->_next[3] )
      {
         cl->_hit._t = CGConstf::infinity();
         if( RayCaster::hit( cl->_ray, v0->_pos, v1->_pos, v2->_pos, cl->_hit ) )
         {
            group->addIntersection( cl->_hit._t-_serror, cl->_hit._backFacing );
         }
      }
   }
   // triangles (0,1,3),(1,2,3).
   else
   {
      // Triangulate/intersect triangle (0,1,3).
      MetaSurface::Vertex* v1 = sp->_corners[0];
      MetaSurface::Vertex* v0 = v1->_next[2];
      MetaSurface::Vertex* v2 = v1;
      for( ; v2 != sp->_corners[1]; v1 = v2 )
      {
         v2 = v2->_next[1];
         cl->_hit._t = CGConstf::infinity();
         if( RayCaster::hit( cl->_ray, v0->_pos, v1->_pos, v2->_pos, cl->_hit ) )
         {
            group->addIntersection( cl->_hit._t-_serror, cl->_hit._backFacing );
         }
      }
      v0 = sp->_corners[3];
      for( v1 = v0->_next[0]; v1 != sp->_corners[0]; v0 = v1, v1 = v1->_next[0] )
      {
         cl->_hit._t = CGConstf::infinity();
         if( RayCaster::hit( cl->_ray, v0->_pos, v1->_pos, v2->_pos, cl->_hit ) )
         {
            group->addIntersection( cl->_hit._t-_serror, cl->_hit._backFacing );
         }
      }
      // Triangulate/intersect triangle (1,2,3).
      v1 = sp->_corners[2];
      v0 = v1->_next[0];
      v2 = v1;
      for( ; v2 != sp->_corners[3]; v1 = v2 )
      {
         v2 = v2->_next[3];
         cl->_hit._t = CGConstf::infinity();
         if( RayCaster::hit( cl->_ray, v0->_pos, v1->_pos, v2->_pos, cl->_hit ) )
         {
            group->addIntersection( cl->_hit._t-_serror, cl->_hit._backFacing );
         }
      }
      v0 = sp->_corners[1];
      for( v1 = v0->_next[2]; v1 != sp->_corners[2]; v0 = v1, v1 = v1->_next[2] )
      {
         cl->_hit._t = CGConstf::infinity();
         if( RayCaster::hit( cl->_ray, v0->_pos, v1->_pos, v2->_pos, cl->_hit ) )
         {
            group->addIntersection( cl->_hit._t-_serror, cl->_hit._backFacing );
         }
      }
   }
}

//------------------------------------------------------------------------------
//! 
void
patchesCSG( void* data, void* objA, void* objB )
{
   DBG_BLOCK( os_b, "patchesCSG" );
   MetaSurface::Patch* p = (MetaSurface::Patch*)objA;
   ClassifyCSG* cl       = (ClassifyCSG*)objB;

   // Do we have to intersect the patch?
   MetaBlocks* group = block( p )->group();

   if( group->countID() != cl->_count ) return;

   if( p->_tree )
   {
      p->_tree->findCollisions( data, objB, cl->_region, subpatchesCSG );
   }
   else
   {
      subpatchesCSG( data, &p->_subpatch, objB );
   }
}

UNNAMESPACE_END


NAMESPACE_BEGIN

/*==============================================================================
   CLASS MetaGeometry
==============================================================================*/

//------------------------------------------------------------------------------
//! 
MetaGeometry::MetaGeometry():
   Geometry( METAGEOMETRY ),
   _geomNeedUpdate(false),
   _count(0),
   _gerror(0.025f),
   _derror(0.04f),
   _bPool(32),
   _hePool(64),
   _fPool(64),
   _vPool(64),
   _grid(0)
{
   // FIXME: find better value for pool.
   _grid    = new HGrid();
   _surface = new MetaSurface();
}

//------------------------------------------------------------------------------
//! 
MetaGeometry::~MetaGeometry()
{
   delete _grid;
}

//------------------------------------------------------------------------------
//!
void
MetaGeometry::update()
{
   // 1. Update tree structure and compute group object's transform.
   updateCSGTree();

   // 2. Establish links between blocks.
   updateLinks();

   // 3. Create control mesh (MetaSurface::Patch).
   updateControlMesh();

   // 4. Create subdivision mesh (MetaSurface::Subpatch).
   _surface->subdivide( _gerror, _derror );
   
   // 5. CSG
#ifndef BLOCKS
   updateCSG();
#endif
   // 6. Create rendering geometry (flagging only).
   _geomNeedUpdate = true;
}

//------------------------------------------------------------------------------
//! 
void
MetaGeometry::updateCSGTree()
{
   // Update tree structure by connecting layers in composite nodes.
   for( uint c = 0; c < _composites.size(); ++c )
   {
      _composites[c]->connectComposites();
   }

   // Update blocks.
   _groups.clear();
   for( uint n = 0; n < _nodes.size(); ++n )
   {
      MetaNode* node = _nodes[n].ptr();
      if( node->type() == MetaNode::META_BLOCKS && 
         ( !node->parent() || node->parent()->type() != MetaNode::META_COMPOSITE_BLOCKS ) )
      {
         _groups.pushBack( (MetaBlocks*)node );
      }
      else if( node->type() == MetaNode::META_COMPOSITE_BLOCKS )
      {
         MetaCompositeBlocks* cb = (MetaCompositeBlocks*)node;
         _groups.pushBack( cb );
         cb->update();
      }
   }

   // Update transforms.
   for( uint n = 0; n < _nodes.size(); ++n )
   {
      MetaNode* node = _nodes[n].ptr();
      if( !node->parent() )
      {
         updateTransforms( node, Mat4f::identity() );
         updateBoundingBoxes( node );
         //node->print();
      }
   }
}

//------------------------------------------------------------------------------
//! 
void 
MetaGeometry::updateTransforms( MetaNode* node, const Mat4f& mat )
{
   if( !node ) return;

   switch( node->type() )
   {
      case MetaNode::META_COMPOSITE:
      {
         MetaComposite* comp = (MetaComposite*)node;
         for( uint c = 0; c < comp->numChildren(); ++c )
         {
            updateTransforms( comp->child(c), comp->transform() );
         }
      }  break;
      case MetaNode::META_COMPOSITE_BLOCKS:
      {
         MetaCompositeBlocks* comp = (MetaCompositeBlocks*)node;
         for( uint c = 0; c < comp->numChildren(); ++c )
         {
            updateTransforms( comp->child(c), Mat4f::identity() );
         }
      }  break;
      case MetaNode::META_INPUT: break;
      case MetaNode::META_BLOCKS:
      {
         MetaBlocks* group = (MetaBlocks*)node;
         Mat4f m = mat * group->transform();
         for( uint b = 0; b < group->numBlocks(); ++b )
         {
            MetaBlock* block = group->block(b);
            for( uint c = 0; c < 8; ++c ) 
            {
               block->position(c) = m * block->localPosition(c);
            }
         }
      }  break;
      case MetaNode::META_TRANSFORM:
      {
         MetaTransform* transform = (MetaTransform*)node;
         Mat4f m = mat * transform->transform();
         updateTransforms( transform->child(), m );
      }  break;
      case MetaNode::META_UNION:
      case MetaNode::META_INTERSECTION:
      case MetaNode::META_DIFFERENCE:
      {
         MetaOperation* operation = (MetaOperation*)node;
         for( uint c = 0; c < operation->numChildren(); ++c )
         {
            updateTransforms( operation->child(c), mat );
         }
      }  break;
      default:
      {
         CHECK( false );
      }  break;
   }
}

//------------------------------------------------------------------------------
//! 
void 
MetaGeometry::updateBoundingBoxes( MetaNode* node )
{
   switch( node->type() )
   {
      case MetaNode::META_COMPOSITE:
      {
         MetaComposite* comp = (MetaComposite*)node;
         if( !comp->mainChild() ) return;
         updateBoundingBoxes( comp->mainChild() );
         comp->boundingBox( comp->mainChild()->boundingBox() );
      }  break;
      case MetaNode::META_INPUT:
      {
         MetaInput* input = (MetaInput*)node;
         if( !input->child() ) return;
         updateBoundingBoxes( input->child() );
         input->boundingBox( input->child()->boundingBox() );
      }  break;
      case MetaNode::META_BLOCKS:
      case MetaNode::META_COMPOSITE_BLOCKS:
      {
         MetaBlocks* group = (MetaBlocks*)node;
         AABBoxf gBox     = AABBoxf::empty();
         for( uint b = 0; b < group->numBlocks(); ++b )
         {
            AABBoxf bBox     = AABBoxf::empty();
            MetaBlock* block = group->block(b);
            for( uint c = 0; c < 8; ++c ) 
            {
               bBox |= block->position(c);
            }
            bBox.grow( bBox.size() * 1.0f );
            gBox |= bBox;
         }
         group->boundingBox( gBox );
      }  break;
      case MetaNode::META_TRANSFORM:
      {
         MetaTransform* transform = (MetaTransform*)node;
         if( !transform->child() ) return;
         updateBoundingBoxes( transform->child() );
         transform->boundingBox( transform->child()->boundingBox());
      }  break;
      case MetaNode::META_UNION:
      case MetaNode::META_INTERSECTION:
      case MetaNode::META_DIFFERENCE:
      {
         MetaOperation* operation = (MetaOperation*)node;
         AABBoxf nBox = AABBoxf::empty();
         for( uint c = 0; c < operation->numChildren(); ++c )
         {
            updateBoundingBoxes( operation->child(c) );
            nBox |= operation->child(c)->boundingBox();
         }
         operation->boundingBox( nBox );
      }  break;
      default:
      {
         CHECK( false );
      }  break;
   }
}

//------------------------------------------------------------------------------
//! 
void 
MetaGeometry::updateLinks()
{
   // 1 Add all blocks to grid.
   for( uint g = 0; g < _groups.size(); ++g )
   {
      MetaBlocks* group = _groups[g];
      for( uint b = 0; b < group->numBlocks(); ++b )
      {
         MetaBlock* block = group->block(b);

         // Build block internal structure.
         updateTopology( block );

         // Compute BB.
         AABBoxf box = AABBoxf::empty();
         for( uint c = 0; c < 8; ++c ) box |= block->position(c);
         box.grow( box.size()*0.25f ); // how much????

         // Add to grid.
         _grid->add( block, box );
      }
   }

#if !defined( BLOCKS ) || defined( CONNECTION )
   // 2 Establish all links.
   _grid->findAllCollisions( this, blocksOverlap );
#endif

   // 3 Clean all non-bidirectionnal links.
   for( uint g = 0; g < _groups.size(); ++g )
   {
      MetaBlocks* group = _groups[g];
      for( uint b = 0; b < group->numBlocks(); ++b )
      {
         MetaBlock* block = group->block(b);
         for( uint s = 0; s < 6; ++s )
         {
            for( MetaBlock::Face* f = &block->side(s); f; f = f->_next )
            {
               if( f->_link && (f != f->_link->_link ) ) f->_link = 0;
            }
         }
      }
   }

   // 4 TODO: Clean non legal links.
}

//------------------------------------------------------------------------------
//! 
void
MetaGeometry::updateControlMesh()
{
   Vector<MetaBlock::Vertex*> vertices;

   for( uint g = 0; g < _groups.size(); ++g )
   {
      MetaBlocks* group = _groups[g];
     
      // 1. Connect all edges and vertices.

      // 1.1 Connect edges.
      for( uint b = 0; b < group->numBlocks(); ++b )
      {
         MetaBlock* block = group->block(b);
         for( uint s = 0; s < 6; ++s )
         {
            for( MetaBlock::Face* f = &block->side(s); f; f = f->_next )
            {
               if( f->_link ) connect( f );
            }
         }
      }

      // 1.2 Connect neighbors faces.
      for( uint b = 0; b < group->numBlocks(); ++b )
      {
         MetaBlock* block = group->block(b);
         for( uint s = 0; s < 6; ++s )
         {
            for( MetaBlock::Face* f = &block->side(s); f; f = f->_next )
            {
               // Skip interior faces.
               if( f->_link ) continue;
               
               MetaBlock::HEdge* he = &f->_corners[0];
               do
               {
                  if( !he->_link )
                  {
                     MetaBlock::HEdge* ne = he->_neighbor;
                     for( ; ne->_link; ne = ne->_link->_neighbor );
                     he->_link = ne;
                     ne->_link = he;
                  }
                  he = he->_next;
               } while( he != &f->_corners[0] );
            }
         }
      }

      // 1.3 Create joint vertices.
      for( uint b = 0; b < group->numBlocks(); ++b )
      {
         MetaBlock* block = group->block(b);
         for( uint s = 0; s < 6; ++s )
         {
            for( MetaBlock::Face* f = &block->side(s); f; f = f->_next )
            {
               // Skip interior faces.
               if( f->_link ) continue;

               MetaBlock::HEdge* he = &f->_corners[0];
               do
               {
                  // Compute joint vertex if not already done.
                  if( !he->_vertex->_jointVertex )
                  {
                     // Accumulate all vertices.
                     Vec3f v( he->position() );
                     vertices.clear();
                     vertices.pushBack( he->_vertex );
                     for( MetaBlock::HEdge* ne = he->_link->_next; ne != he; ne = ne->_link->_next )
                     {
                        if( vertices.find( ne->_vertex ) == vertices.end() )
                        {
                           vertices.pushBack( ne->_vertex );
                           v+= ne->position();
                        }
                     }
                     // Joined them.
                     if( vertices.size() == 1 )
                     {
                        vertices[0]->_jointVertex = vertices[0];
                     }
                     else
                     {
                        MetaBlock::Vertex* vertex = _vPool.alloc();
                        vertex->_position         = v / (float)vertices.size();
                        vertex->_jointVertex      = 0;
                        for( uint i = 0; i < vertices.size(); ++i )
                        {
                           vertices[i]->_jointVertex = vertex;
                        }
                     }
                  }

                  he = he->_next;
               } while( he != &f->_corners[0] );
            }
         }
      }

      // 2. Build control mesh.
      // 2.1 Create patches and centers.
      for( uint b = 0; b < group->numBlocks(); ++b )
      {
         MetaBlock* block = group->block(b);
         for( uint s = 0; s < 6; ++s )
         {
            for( MetaBlock::Face* f = &block->side(s); f; f = f->_next )
            {
               // Skip interior faces.
               if( f->_link ) continue;

               Vec3f* center = _surface->createPoint();
               uint count    = 0;
               int  henum    = 0;

               MetaBlock::HEdge* he = &f->_corners[0];
               do
               {
                  ++count;
                  *center                   += he->jointPosition();
                  he->_patch                 = _surface->createPatch();
                  he->_patch->_mapping       = group->mapping();
                  he->_patch->_displacement  = group->displacement();
                  he->_patch->_id            = block->id() | (s <<29);
                  //he->_patch->_id            = b | (s <<29);
                  he->_patch->_data          = he;
                  he->_patch->_controlPts[0] = center;
#ifndef SUBFACE_MAPPING
                  he->_patch->_uv[0]         = faceUV[(henum+2)%4];
                  he->_patch->_uv[1]         = faceUV[(henum+3)%4];
                  he->_patch->_uv[2]         = faceUV[(henum+4)%4];
                  he->_patch->_uv[3]         = faceUV[(henum+5)%4];
#else
                  he->_patch->_uv[0]         = Vec2f( 0.5f, 0.5f );
                  he->_patch->_uv[1]         = Vec2f( 1.0f, 0.5f );
                  he->_patch->_uv[2]         = Vec2f( 1.0f, 1.0f );
                  he->_patch->_uv[3]         = Vec2f( 0.5f, 1.0f );
#endif
                  he                         = he->_next;
                  // Find the half-edge number in subface.
                  if( henum < 3 && he == &f->_corners[henum+1] ) ++henum;
               } while( he != &f->_corners[0] );
               *center /= float(count);
            }
         }
      }

      // 2.2 Add edge and corners vertices.
      for( uint b = 0; b < group->numBlocks(); ++b )
      {
         MetaBlock* block = group->block(b);
         for( uint s = 0; s < 6; ++s )
         {
            for( MetaBlock::Face* f = &block->side(s); f; f = f->_next )
            {
               // Skip interior faces.
               if( f->_link ) continue;

               MetaBlock::HEdge* he = &f->_corners[0];
               do
               {
                  // Setting neighbors.
                  int crease = he->_flags | he->_link->_flags;
                  _surface->neighbors( he->_patch, 2, he->_link->_next->_patch, 1, crease );
                  _surface->neighbors( he->_patch, 3, he->_next->_patch, 0, 0 );

                  // Edge vertex.
                  if( !he->_patch->_controlPts[3] )
                  {
                     Vec3f* v = _surface->createPoint();

                     // Smooth or crease?
                     if( crease )
                     {
                        *v += he->jointPosition();
                        *v += he->_next->jointPosition();
                        *v *= 0.5f;
                     }
                     else
                     {
                        *v += *he->_patch->_controlPts[0];
                        *v += *he->_link->_patch->_controlPts[0];
                        *v += he->jointPosition();
                        *v += he->_next->jointPosition();
                        *v *= 0.25f;
                     }

                     he->_patch->_controlPts[3]               = v;
                     he->_next->_patch->_controlPts[1]        = v;
                     he->_link->_patch->_controlPts[3]        = v;
                     he->_link->_next->_patch->_controlPts[1] = v;
                  }

                  // Corner vertex.
                  if( !he->_patch->_controlPts[2] )
                  {
                     Vec3f vc(0.0f);
                     uint crCount = 0;
                     if( crease )
                     {
                        ++crCount;
                        vc += he->_next->jointPosition();
                     }

                     Vec3f* v = _surface->createPoint();
                     *v += *he->_patch->_controlPts[0];
                     *v += he->_next->jointPosition();
                     he->_patch->_controlPts[2] = v;
                     int count = 1;   
                     for( MetaBlock::HEdge* ne = he->_link->_next; ne != he; ne = ne->_link->_next )
                     {
                        ne->_patch->_controlPts[2] = v;
                        *v += *ne->_patch->_controlPts[0];
                        *v += ne->_next->jointPosition();
                        ++count;
                        if( ne->_flags | ne->_link->_flags )
                        {
                           ++crCount;
                           vc += ne->_next->jointPosition();
                        }
                     }
                     if( crCount <= 1 )
                     {
                        *v /= float(count*count);
                        *v += he->jointPosition()*((float)(count-2)/(float)count);
                     }
                     else if( crCount == 2 )
                     {
                        *v = (vc + he->jointPosition()*6.0f)*0.125;
                     }
                     else
                     {
                        *v = he->jointPosition();
                     }
                  }

                  he = he->_next;
               } while( he != &f->_corners[0] );
            }
         }
      }
   }
}


//------------------------------------------------------------------------------
//! 
void
MetaGeometry::updateTopology( MetaBlock* b )
{
   // 1. Create default block with topology.
   for( uint i = 0; i < 8; ++i )
   {
      b->vertex(i)._jointVertex = 0;
   }
   b->vertex(0)._t = 0;
   b->vertex(1)._t = 1;
   b->vertex(2)._t = 1;
   b->vertex(3)._t = 2;
   b->vertex(4)._t = 1;
   b->vertex(5)._t = 2;
   b->vertex(6)._t = 2;
   b->vertex(7)._t = 3;

   for( uint s = 0; s < 6; ++s )
   {
      MetaBlock::Face* f = &b->side(s);
      f->_next           = 0;
      f->_link           = 0;
      f->_linkCorner     = 0;
      f->_block          = b;
 
      for( uint c = 0; c < 4; ++c )
      {
         f->_corners[c]._face     = f;
         f->_corners[c]._patch    = 0;
         f->_corners[c]._link     = 0;
         f->_corners[c]._vertex   = &b->vertex( faceVertex[s][c] );
         f->_corners[c]._next     = &f->_corners[(c+1)%4];
#ifdef BLOCKS
         f->_corners[c]._flags    = 1;
#else
         f->_corners[c]._flags    = b->crease( faceEdge[s][c] );
#endif
      }
   }
   neighbor( b->side(0)._corners[0], b->side(2)._corners[3] );
   neighbor( b->side(0)._corners[1], b->side(5)._corners[3] );
   neighbor( b->side(0)._corners[2], b->side(3)._corners[3] );
   neighbor( b->side(0)._corners[3], b->side(4)._corners[1] );
   neighbor( b->side(1)._corners[0], b->side(2)._corners[1] );
   neighbor( b->side(1)._corners[1], b->side(4)._corners[3] );
   neighbor( b->side(1)._corners[2], b->side(3)._corners[1] );
   neighbor( b->side(1)._corners[3], b->side(5)._corners[1] );
   neighbor( b->side(2)._corners[0], b->side(4)._corners[0] );
   neighbor( b->side(2)._corners[2], b->side(5)._corners[0] );
   neighbor( b->side(3)._corners[0], b->side(5)._corners[2] );
   neighbor( b->side(3)._corners[2], b->side(4)._corners[2] );

   // 2. For each face.
   for( uint s = 0; s < 6; ++s )
   {
      // 2.1 Subdivide side edges.
      int sub = b->subdivision(s);

      // Nothing to subdivide.
      if( sub == 0 ) continue;

      split( &b->side(s), (sub&3)+1, (sub>>2)+1 );
   }
}

//------------------------------------------------------------------------------
//! 
void
MetaGeometry::split( MetaBlock::HEdge* e0, float frac )
{
   MetaBlock::HEdge* ne0 = e0->_neighbor;

   // New Vertex.
   MetaBlock::Vertex* v  = _vPool.alloc();
   v->_jointVertex       = 0;
   v->_position          = (ne0->position()-e0->position())*frac + e0->position();
   v->_t                 = (ne0->t()-e0->t())*frac + e0->t();

   // New edges.
   MetaBlock::HEdge* e1  = _hePool.alloc();
   MetaBlock::HEdge* ne1 = _hePool.alloc();

   e1->_patch     = 0;
   e1->_face      = e0->_face;
   e1->_link      = e0->_link;
   e1->_neighbor  = ne0;
   e1->_next      = e0->_next;
   e1->_vertex    = v;
   e1->_flags     = e0->_flags;

   ne1->_patch    = 0;
   ne1->_face     = ne0->_face;
   ne1->_link     = ne0->_link;
   ne1->_neighbor = e0;
   ne1->_next     = ne0->_next;
   ne1->_vertex   = v;
   ne1->_flags    = ne0->_flags;

   // Update old edges.
   e0->_neighbor  = ne1;
   e0->_next      = e1;

   ne0->_neighbor = e1;
   ne0->_next     = ne1;

   // Recursive split.
   if( ne0->_link )
   {
      split( ne0->_link, frac );
      ne0->_link = ne0->_link->_next;
   }

}

//------------------------------------------------------------------------------
//! 
void 
MetaGeometry::split( 
   MetaBlock::HEdge*  startEdge, 
   MetaBlock::HEdge*  endEdge, 
   const float*       fracs,
   MetaBlock::HEdge** edges
)
{
   const float eps = CGConstf::epsilon(32.0f);

   // Find split direction.
   float dt  = endEdge->t() - startEdge->t();
   bool dir  = dt < 0.0f ? false : true;

   MetaBlock::HEdge* curEdge  = startEdge;
   MetaBlock::HEdge* prevEdge = 0;
   // Split edge(s).
   while( *fracs < 1.0f )
   {
      // Compute next uv to  insert.
      float t = startEdge->t() + dt*(*fracs);

      // Find edge to split.
      if( dir )
      {
         while( curEdge->_next->t() <= t+eps ) 
         {
            prevEdge = curEdge;
            curEdge  = curEdge->_next;
         }
      }
      else
      {
         while( curEdge->_next->t() >= t-eps ) 
         {
            prevEdge = curEdge;
            curEdge  = curEdge->_next;
         }
      }

      // Split edge if needed.
      if( !CGM::equal( curEdge->t(), t, eps ) )
      {
         float frac = (t-curEdge->t()) / (curEdge->_next->t()-curEdge->t());
         split( curEdge, frac );
         // We can't  split the same edge twice.
         prevEdge = curEdge;
         curEdge  = curEdge->_next;
      }

      // Save previous edge.
      if( edges )
      {
         *edges = prevEdge;
         ++edges;
      }

      // Next uv fraction.
      ++fracs;
   }

   // Find last edge.
   if( edges )
   {
      while( curEdge->_next != endEdge ) curEdge = curEdge->_next;
      *edges = curEdge;
   }
}

//------------------------------------------------------------------------------
//! 
void
MetaGeometry::split( MetaBlock::Face* f, int sx, int sy )
{
   MetaBlock::Face* faces[16];
   MetaBlock::HEdge* se0[4];
   MetaBlock::HEdge* se1[4];
   MetaBlock::HEdge* se2[4];
   MetaBlock::HEdge* se3[4];

   // 1 Subdivide side edges.
   
   // Subdivide in x.
   split( &f->_corners[0], &f->_corners[1], subFracs[sx], se0 );
   split( &f->_corners[2], &f->_corners[3], subFracs[sx], se2 );

   // Subdivide in y.
   split( &f->_corners[1], &f->_corners[2], subFracs[sy], se1 );
   split( &f->_corners[3], &f->_corners[0], subFracs[sy], se3 );

   // 2 Create face(s).
   faces[0] = f;
   for( int i = 1; i < sx*sy; ++i )
   {
      faces[i]                     = _fPool.alloc();
      faces[i]->_next              = 0;
      faces[i]->_link              = 0;
      faces[i]->_linkCorner        = 0;
      faces[i]->_block             = f->_block;
      faces[i]->_corners[0]._flags = 0;
      faces[i]->_corners[1]._flags = 0;
      faces[i]->_corners[2]._flags = 0;
      faces[i]->_corners[3]._flags = 0;
      faces[i-1]->_next            = faces[i];
   }

   // 3 Set corners + prev.
   // Corner 2.
   MetaBlock::HEdge* he     = &faces[sx*sy-1]->_corners[2];
   *he                      = f->_corners[2];
   se1[sy-1]->_next         = he;
   he->_neighbor->_neighbor = he;
   if( se2[0] == &f->_corners[2] ) se2[0] = he;
   // Corner 1.
   if( sx > 1 )
   {
      he                       = &faces[sx-1]->_corners[1];
      *he                      = f->_corners[1];
      se0[sx-1]->_next         = he;
      he->_neighbor->_neighbor = he;
      if( se1[0] == &f->_corners[1] ) se1[0] = he;
   }
   // Corner 3.
   if( sy > 1 )
   {
      he                       = &faces[sx*sy-sx]->_corners[3];
      *he                      = f->_corners[3];
      se2[sx-1]->_next         = he;
      he->_neighbor->_neighbor = he;
      if( se3[0] == &f->_corners[3] ) se3[0] = he;
   }

   // 4 Update interior edges and create interior vertices.
   const Vec3f& c0 = f->_corners[0].position();
   const Vec3f& c1 = f->_corners[1].position();
   const Vec3f& c2 = f->_corners[2].position();
   const Vec3f& c3 = f->_corners[3].position();

   Vec2f dt( f->_corners[1].t()-f->_corners[0].t(), f->_corners[3].t()-f->_corners[0].t() );

   for( int y = 1; y < sy; ++y )
   {
      for( int x = 1; x < sx; ++x )
      {
         MetaBlock::Face* f0       = faces[(y-1)*sx+x-1];
         MetaBlock::Face* f1       = faces[(y-1)*sx+x];
         MetaBlock::Face* f2       = faces[y*sx+x];
         MetaBlock::Face* f3       = faces[y*sx+x-1];

         Vec2f uv                  = Vec2f( subFracs[sx][x-1], subFracs[sy][y-1] );
         MetaBlock::Vertex* v      = _vPool.alloc();
         v->_jointVertex           = 0;
         v->_position              = CGM::bilinear( c0, c1-c0, c3, c2-c3, uv.x, uv.y );
         v->_t                     = dt.dot( uv );

         f0->_corners[2]._vertex   = v;
         f0->_corners[2]._link     = 0;
         f0->_corners[2]._neighbor = &f3->_corners[0];
         f0->_corners[2]._next     = &f0->_corners[3];

         f1->_corners[3]._vertex   = v;
         f1->_corners[3]._link     = 0;
         f1->_corners[3]._neighbor = &f0->_corners[1];
         f1->_corners[3]._next     = &f1->_corners[0];

         f2->_corners[0]._vertex   = v;
         f2->_corners[0]._link     = 0;
         f2->_corners[0]._neighbor = &f1->_corners[2];
         f2->_corners[0]._next     = &f2->_corners[1];

         f3->_corners[1]._vertex   = v;
         f3->_corners[1]._link     = 0;
         f3->_corners[1]._neighbor = &f2->_corners[3];
         f3->_corners[1]._next     = &f3->_corners[2];
      }
   }
   
   // 5 Update side edges (except corners).
   MetaBlock::Face** fp   = 0;
   MetaBlock::HEdge** hep = 0;
   int len   = 0;
   int fpinc = 0;
   for( int i = 0; i < 4; ++i )
   {
      switch( i )
      {
         case 0: 
            fp    = &faces[sx-1]; 
            fpinc = -1;
            hep   = &se0[sx-2];
            len   = sx;
            break;
         case 1: 
            fp    = &faces[sy*sx-1];
            fpinc = -sx;
            hep   = &se1[sy-2];
            len   = sy;
            break;
         case 2: 
            fp    = &faces[(sy-1)*sx];
            fpinc = 1;
            hep   = &se2[sx-2];
            len   = sx;
            break;
         case 3: 
            fp    = &faces[0];
            fpinc = sx;
            hep   = &se3[sy-2];
            len   = sy;
            break;
      }

      for( int j = 1; j < len; ++j )
      {
         MetaBlock::Face* f1  = *fp; fp += fpinc;
         MetaBlock::Face* f0  = *fp;
         MetaBlock::HEdge* be = *hep--;

         f1->_corners[i] = *be->_next;
         _hePool.free( be->_next );
         be->_next = &f0->_corners[(1+i)%4];
         f1->_corners[i]._neighbor->_neighbor = &f1->_corners[i];

         f0->_corners[(1+i)%4]._link     = 0;
         f0->_corners[(1+i)%4]._vertex   = f1->_corners[i]._vertex;
         f0->_corners[(1+i)%4]._neighbor = &f1->_corners[(3+i)%4];
         f0->_corners[(1+i)%4]._next     = &f0->_corners[(2+i)%4];
      }
   }
  
   // 6. Update faces pointers.
   for( int i = 0; i < sx*sy; ++i )
   {
      MetaBlock::Face* cf  = faces[i];
      MetaBlock::HEdge* he = &cf->_corners[0];
      do
      {
         he->_face = cf;
         he = he->_next;
      } while( he != &cf->_corners[0] );
   }

   // Change flag.
   if( sx > 1 ) f->_corners[1]._flags = 0;
   if( sy > 1 ) f->_corners[2]._flags = 0;
}

//------------------------------------------------------------------------------
//!
void
MetaGeometry::connect( MetaBlock::Face* f )
{
   // Testing if face is already connected.
   if( f->_corners[0]._link != 0 ) return;

   Vector<float> fracs0;
   Vector<float> fracs1;
   Vector<float> fracs;
   fracs0.reserve(4);
   fracs1.reserve(4);
   fracs.reserve(4);

   Vector<MetaBlock::HEdge*> edges;

   MetaBlock::HEdge* e;
   
   for( int c = 0; c < 4; ++c )
   {
      // Find subdivision in current edge.
      fracs0.clear();
      MetaBlock::HEdge* se0 = &f->_corners[c];
      MetaBlock::HEdge* ee0 = &f->_corners[(c+1)%4];
      for( e = se0->_next; e != ee0; e = e->_next )
      {
         fracs0.pushBack( (e->t()-se0->t()) / (ee0->t()-se0->t()) );
      }

      // Find subdivision in opposite edge.
      fracs1.clear();
      edges.clear();
      MetaBlock::HEdge* se1 = &f->_link->_corners[(f->_linkCorner-c+3)%4];
      MetaBlock::HEdge* ee1 = &f->_link->_corners[(f->_linkCorner-c+4)%4];
      edges.pushBack( se1 );
      for( e = se1->_next; e != ee1; e = e->_next )
      {
         edges.pushBack( e );
         fracs1.pushBack( (e->t()-se1->t()) / (ee1->t()-se1->t()) );
      }

      // Apply subdivision on current edge.
      if( !fracs1.empty() )
      {
         fracs.clear();
         for( int i = int(fracs1.size())-1; i >= 0; --i )
         {
            fracs.pushBack( 1.0f - fracs1[i] );
         }
         fracs.pushBack( 1.0f );
         split( se0, ee0, fracs.data(), 0 );
      }

      // Apply subdivision on opposite edge.
      if( !fracs0.empty() )
      {
         fracs.clear();
         for( int i = int(fracs0.size())-1; i >= 0; --i )
         {
            fracs.pushBack( 1.0f - fracs0[i] );
         }
         fracs.pushBack( 1.0f );
         split( se1, ee1, fracs.data(), 0 );
         // Recompute edge list.
         edges.clear();
         edges.pushBack( se1 );
         for( e = se1->_next; e != ee1; e = e->_next )
         {
            edges.pushBack( e );
         }
      }

      // Connect edges.
      Vector<MetaBlock::HEdge*>::ReverseIterator it = edges.rbegin();
      for( e = se0; e != ee0; e = e->_next, ++it )
      {
         e->_link     = *it;
         (*it)->_link = e;
      }
   }
}

//------------------------------------------------------------------------------
//! 
void
MetaGeometry::updateCSG()
{
   DBG_BLOCK( os_b, "MetaGeometry::updateCSG" );
   int flipped;
   Vec3f pos;
   Vec3f dir;
   // Find all intersecting patches and apply trimming.
   _surface->hgrid().findAllCollisions( _surface.ptr(), patchesOverlap );

   // Classify each patch/subpatches as on/off surface and inverted or not.
   for( MetaSurface::PatchIter pIt = _surface->patches(); pIt.isValid(); ++pIt )
   {
      MetaBlock* b = block( pIt );

      if( !MetaSurface::isTrimmed( *pIt ) )
      {
         DBG_MSG( os_b, "non-trimmed patch" );
         // Classify patch: all subpatches have the same classification.
         _surface->computePoint( pIt->_subpatch, pos, dir );
         int hidden = classifyCSG( pos, dir, b, flipped );
         MetaSurface::hide( *pIt, hidden );
         MetaSurface::flip( *pIt, flipped );
         DBG_MSG( os_b, "  pos: " << pos << " dir: " << dir << " hide: " << hidden << " flipped: " << flipped );
         // TODO: propagate with fix stack size.
      }
      else
      {
         // Classify subpatches.
         for( MetaSurface::SubpatchIter sIt = _surface->subpatches(*pIt); sIt.isValid(); ++sIt )
         {
            bool dbg = false;
            //bool dbg = (pIt->_detailsID == 85) && (sIt->_corners[0]->_uv == Vec2f(0.125f,0.5f));
            if( dbg ) _surface->dump( *sIt );
            //if( !dbg ) continue;

            if( !MetaSurface::isTrimmed( *sIt ) )
            {
               DBG_MSG( os_b, "non-trimmed subpath" );
               if( dbg ) StdErr << "  Not trimmed\n";
               _surface->computePoint( *sIt, pos, dir );
               //MetaSurface::hide( *sIt, classifyCSG( pos, dir, b, flipped ) );
               bool hidden = classifyCSG( pos, dir, b, flipped ) != 0;
               MetaSurface::hide( *sIt, hidden );
               if( flipped ) MetaSurface::flip( *pIt, 1 );
               DBG_MSG( os_b, "  pos: " << pos << " dir: " << dir << " hide: " << hidden << " flipped: " << flipped );
            }
            else
            {
               DBG_MSG( os_b, "trimmed subpatch " << pIt->_detailsID );
               if( dbg ) StdErr << "  Trimmed\n";
               // Classify loops.
               MetaSurface::Trimming* trim = _surface->trimming( *sIt );
               dir = trim->_plane0.direction();
               if( dbg ) _surface->dump( *sIt, *trim );
               for( MetaSurface::Loop* loop = trim->_loops; loop; loop = loop->_next )
               {
                  _surface->computePoint( *sIt, *trim, *loop, pos );
                  //MetaSurface::hide( *loop, classifyCSG( pos, dir, b, flipped ) );
                  int hidden = classifyCSG( pos, dir, b, flipped );
                  MetaSurface::hide( *loop, hidden );
                  if( flipped ) MetaSurface::flip( *pIt, 1 );
                  if( dbg ) StdErr << "  point: " << pos << " flip: " << flipped << " hide: " << hidden << "\n";
                  DBG_MSG( os_b, "  pos: " << pos << " dir: " << dir << " hide: " << hidden << " flipped: " << flipped );
               }
            }
         }
      }
      // Reverse normals on surface details if necessary.
      // FIXME: should be done when computing the normals.
      //if( MetaSurface::isFlipped( *pIt ) ) details()->flipNormals( pIt->_detailsID );
   }
}

//------------------------------------------------------------------------------
//! 
void
MetaGeometry::computeRenderableGeometry()
{
   // Nothing to do.
   if( _surface->numPatches() == 0 ) return;

   Vector<float> vertices;
   Vector<uint> indices;

   // Triangulate all patches.
   // 1. Regroup and sort patches by materials.
   Vector<MetaSurface::Patch*> patches;
   patches.reserve( _surface->numPatches() );
   for( MetaSurface::PatchIter pIt = _surface->patches(); pIt.isValid(); ++pIt )
   {
      patches.pushBack( pIt );
   }
   std::sort( patches.begin(), patches.end(), materialSort );

   // 2. Triangulate and create patch info for mesh.
   Geometry::clearPatches();
   uint id   = (patches[0]->_id & 0x1fffffff);
   uint sidx = 0;
   for( uint i = 0; i < patches.size(); ++i )
   {
      uint cid = (patches[i]->_id & 0x1fffffff);
      if( cid != id )
      {
         Geometry::addPatch( sidx, uint(indices.size())-sidx, id );
         sidx = uint(indices.size());
         id   = cid;
      }
      _surface->triangulate( *patches[i], vertices, indices, 0, true );
   }
   Geometry::addPatch( sidx, uint(indices.size())-sidx, id );

   // Transfer to Gfx.
   _rgeom = Core::gfx()->createGeometry( Gfx::PRIM_TRIANGLES );
   RCP<Gfx::VertexBuffer> vbuffer = Core::gfx()->createBuffer( 
      Gfx::BUFFER_FLAGS_NONE, vertices.dataSize(), vertices.data()
   );
   vbuffer->addAttribute( Gfx::ATTRIB_TYPE_POSITION,  Gfx::ATTRIB_FMT_32F_32F_32F, 0 );
   vbuffer->addAttribute( Gfx::ATTRIB_TYPE_TEXCOORD0, Gfx::ATTRIB_FMT_32F_32F, 12 );
   vbuffer->addAttribute( Gfx::ATTRIB_TYPE_NORMAL,    Gfx::ATTRIB_FMT_32F_32F_32F, 20 );
   _rgeom->addBuffer( vbuffer );

   if( vertices.size() < (1<<8) )
   {
      // Recompact into 8b indices.
      Vector<uint8_t> indices8( indices.size() );
      for( size_t i = 0; i < indices.size(); ++i )
      {
         indices8[i] = indices[i];
      }
      _rgeom->indexBuffer(
         Core::gfx()->createBuffer(
            Gfx::INDEX_FMT_8, Gfx::BUFFER_FLAGS_NONE, indices8.dataSize(), indices8.data()
         )
      );
   }
   else
   if( vertices.size() < (1<<16) )
   {
      // Recompact into 16b indices.
      Vector<uint16_t> indices16( indices.size() );
      for( size_t i = 0; i < indices.size(); ++i )
      {
         indices16[i] = indices[i];
      }
      _rgeom->indexBuffer(
         Core::gfx()->createBuffer(
            Gfx::INDEX_FMT_16, Gfx::BUFFER_FLAGS_NONE, indices16.dataSize(), indices16.data()
         )
      );
   }
   else
   {
      _rgeom->indexBuffer(
         Core::gfx()->createBuffer(
            Gfx::INDEX_FMT_32, Gfx::BUFFER_FLAGS_NONE, indices.dataSize(), indices.data()
         )
      );
   }

   StdErr << "#v: " << vertices.size()/8 << " #t: " << indices.size()/3 << "\n";
   _geomNeedUpdate = false;
}

//------------------------------------------------------------------------------
//! The resulting vertex buffer is composed of:
//!   X Y Z U V (NX NY NZ)
void
MetaGeometry::triangulate(
   Vector<float>& vertices, 
   Vector<uint>&  indices,
   Vector<uint>*  faceInfos,
   bool           normals
) const
{
   // Triangulate all patches.
   // 1. Regroup and sort patches by materials.
   Vector<MetaSurface::Patch*> patches;
   patches.reserve( _surface->numPatches() );
   for( MetaSurface::PatchIter pIt = _surface->patches(); pIt.isValid(); ++pIt )
   {
      patches.pushBack( pIt );
   }
   std::sort( patches.begin(), patches.end(), materialSort );

   // 2. Triangulate and create patch info for mesh.
   for( uint i = 0; i < patches.size(); ++i )
   {
      _surface->triangulate( *patches[i], vertices, indices, faceInfos, normals );
   }
}

//------------------------------------------------------------------------------
//! 
void 
MetaGeometry::createMesh( MeshGeometry& mesh, Vector<uint>* faceInfos ) const
{
   // Nothing to do.
   if( _surface->numPatches() == 0 ) return;

   Vector<float> vertices;
   Vector<uint> indices;

   // Triangulate all patches.
   // 1. Regroup and sort patches by materials.
   Vector<MetaSurface::Patch*> patches;
   patches.reserve( _surface->numPatches() );
   for( MetaSurface::PatchIter pIt = _surface->patches(); pIt.isValid(); ++pIt )
   {
      patches.pushBack( pIt );
   }
   std::sort( patches.begin(), patches.end(), materialSort );

   // 2. Triangulate and create patch info for mesh.
   uint id   = (patches[0]->_id & 0x1fffffff);
   uint sidx = 0;
   for( uint i = 0; i < patches.size(); ++i )
   {
      uint cid = (patches[i]->_id & 0x1fffffff);
      if( cid != id )
      {
         mesh.addPatch( sidx, uint(indices.size())-sidx, id );
         sidx = uint(indices.size());
         id   = cid;
      }
      _surface->triangulate( *patches[i], vertices, indices, faceInfos, true );
   }
   mesh.addPatch( sidx, uint(indices.size())-sidx, id );

   // Copy in mesh.
   uint numVertices = uint(vertices.size())/8;
   uint numIndices  = uint(indices.size());

   const int attribs[] = {
      MeshGeometry::POSITION,
      MeshGeometry::MAPPING,
      MeshGeometry::NORMAL,
      0
   };

   //computeNormals( vertices, indices );

   mesh.allocateIndices( numIndices );
   mesh.copyIndices( indices.data() );
   mesh.setAttributes( attribs );
   mesh.allocateVertices( numVertices );
   mesh.copyAttributes( vertices.data(), 8, 8, 0 );
   mesh.updateProperties();
}

//------------------------------------------------------------------------------
//! 
int
MetaGeometry::classifyCSG( const Vec3f& pos, const Vec3f& dir, MetaBlock* block, int& flipped )
{
   DBG_BLOCK( os_b, "MetaGeometry::classifyCSG" );
   // 1. compute BB and reset node.
   ++_count;
   AABBoxf box = AABBoxf::empty();
   for( uint g = 0; g < _groups.size(); ++g )
   {
      MetaBlocks* group = _groups[g];
      if( group->boundingBox().isInside( pos ) )
      {
         // Add node bounding box to region of interest.
         group->resetCount( _count );
         box |= group->boundingBox();
      }
   }

   // 2. Throw rays.
   // Compute main axis.
   ClassifyCSG data;
   data._count = _count;
   data._axis  = dir.maxComponent();
   data._ray   = Rayf( pos, Vec3f(0.0f) );

   // Compute CSG region.
   data._region.set( pos );
   if( dir(data._axis) < 0.0f )
   {
      data._region.min(data._axis)      = box.min(data._axis)-_serror;
      data._region.max(data._axis)     += _serror;
      data._ray.direction()(data._axis) = -1.0f;
      data._ray.origin()(data._axis)   += _serror;
   }
   else
   {
      data._region.max(data._axis)      = box.max(data._axis)+_serror;
      data._region.min(data._axis)     -= _serror;
      data._ray.direction()(data._axis) = 1.0f;
      data._ray.origin()(data._axis)   -= _serror;
   }
   DBG_MSG( os_b, "ray: " << data._ray << " region: " << data._region );
   DBG_MSG( os_b, "ori: " << toStr(data._ray.origin()) << toStr(data._ray.direction()) );

   // Classify each group by throwing a ray.
   _surface->hgrid().findCollisions( this, &data, data._region, patchesCSG );

   // 3. Evaluate surface.
   return evaluateCSG( block, pos, flipped );
}

//------------------------------------------------------------------------------
//! 
int
MetaGeometry::evaluateCSG( MetaBlock* block, const Vec3f& pos, int& flipped )
{
   DBG_BLOCK( os_b, "MetaGeometry::evaluateCSG" );
   flipped = 0;
   // Find group.
   MetaBlocks* group = block->group();

   // evalute until root.
   MetaBlocks* bnode;
   MetaNode* prevNode = group;
   MetaNode* node     = group->parent();
   int state          = group->currentState(_count);

   if( state != BIN ) return 1;

   while( node )
   {
      DBG_MSG( os_b, "node: " << node->type() );
      switch( node->type() )
      {
         case MetaNode::META_COMPOSITE:
            break;
         case MetaNode::META_INPUT:
            break;
         case MetaNode::META_TRANSFORM:
            break;
         case MetaNode::META_UNION:
         { 
            MetaOperation* nodeOperation = (MetaOperation*)node;
            for( uint i = 0; i < nodeOperation->numChildren(); ++i )
            {
               MetaNode* child = nodeOperation->child(i);
               // Skip node of current group.
               if( child == prevNode ) continue;
               int curState = evaluateCSG( child, pos, bnode );
               if( curState == OUT ) continue;
               if( (curState == IN) || (curState != state) || (group > bnode) )
               {
                  // Patch is inside, NOT ON BOUNDARY.
                  return 1;
               }
            }
         }  break;
         case MetaNode::META_INTERSECTION:
         {
            MetaOperation* nodeOperation = (MetaOperation*)node;
            for( uint i = 0; i < nodeOperation->numChildren(); ++i )
            {
               MetaNode* child = nodeOperation->child(i);
               // Skip node of current group.
               if( child == prevNode ) continue;
               int curState = evaluateCSG( child, pos, bnode );
               if( curState == IN ) continue;
               if( curState == OUT || (curState != state) || (group > bnode) )
               {
                  // Patch is outside, NOT ON BOUNDARY.
                  return 1;
               }
            }
         }  break;
         case MetaNode::META_DIFFERENCE:
         {
            MetaOperation* nodeOperation = (MetaOperation*)node;
            // Evaluate first member.
            if( nodeOperation->child(0) != prevNode )
            {
               int curState = evaluateCSG( nodeOperation->child(0), pos, bnode );
               // Flip state
               state = (state == BOUT) ? BIN : BOUT;

               if( curState != IN )
               {
                  if( curState == OUT || (curState != state) || (group > bnode) )
                  {
                     // Patch is outside, NOT ON BOUNDARY.
                     return 1;
                  } 
               }
            }
            // Evaluate subtracting members.
            for( uint i = 1; i < nodeOperation->numChildren(); ++i )
            {
               MetaNode* child = nodeOperation->child(i);
               // Skip node of current group.
               if( child == prevNode ) continue;
               int curState = evaluateCSG( child, pos, bnode );
               if( curState == OUT ) continue;
               if( curState == IN || (curState == state) || (group > bnode) )
               {
                  // Patch is outside, NOT ON BOUNDARY.
                  return 1;
               }
            }
         }  break;
         default: break;
      }
      prevNode = node;
      node     = node->parent();
   }

   // Subpatch is on boundary.
   flipped = state == BIN ? 0 : 1;
   return 0;
}

//------------------------------------------------------------------------------
//!
int
MetaGeometry::evaluateCSG( MetaNode* node, const Vec3f& pos, MetaBlocks*& boundaryNode )
{
   MetaBlocks* bnode;

   // Point not contained in bounding box: OUTSIDE.
   if( !node->boundingBox().isInside( pos ) ) return OUT;

   switch( node->type() )
   {
      case MetaNode::META_COMPOSITE:
      {
         // Propagate query.
         MetaComposite* nodeComposite = (MetaComposite*)node;
         return evaluateCSG( nodeComposite->mainChild(), pos, boundaryNode );
      }  break;
      case MetaNode::META_INPUT:
      {
         // Propagate query.
         MetaInput* nodeInput = (MetaInput*)node;
         return evaluateCSG( nodeInput->child(), pos, boundaryNode );
      }  break;
      case MetaNode::META_BLOCKS:
      case MetaNode::META_COMPOSITE_BLOCKS:
      {
         boundaryNode = (MetaBlocks*)node;
         return boundaryNode->currentState( _count );
      }  break;
      case MetaNode::META_TRANSFORM:
      {
         // Propagate query.
         MetaTransform* nodeTransform = (MetaTransform*)node;
         return evaluateCSG( nodeTransform->child(), pos, boundaryNode );
      }  break;
      case MetaNode::META_UNION:
      { 
         int state = OUT;
         MetaOperation* nodeOperation = (MetaOperation*)node;
         for( uint i = 0; i < nodeOperation->numChildren(); ++i )
         {
            int curState = evaluateCSG( nodeOperation->child(i), pos, bnode );
            
            if( curState == IN ) return IN;
            if( curState == OUT ) continue;
            // Handling of boundary cases.
            if( state == OUT )
            {
               boundaryNode = bnode;
               state        = curState;
            }
            else
            {
               if( state != curState ) return IN;
               // Keep boundary based on pointer priority.
               boundaryNode = CGM::min( bnode, boundaryNode );
            }
         }
         return state;
      }  break;
      case MetaNode::META_INTERSECTION:
      { 
         int state = OUT;
         MetaOperation* nodeOperation = (MetaOperation*)node;
         for( uint i = 0; i < nodeOperation->numChildren(); ++i )
         {
            int curState = evaluateCSG( nodeOperation->child(i), pos, bnode );
            
            if( curState == OUT ) return OUT;
            if( curState == IN ) continue;
            // Handling of boundary cases.
            if( state == OUT )
            {
               boundaryNode = bnode;
               state        = curState;
            }
            else
            {
               if( state != curState ) return OUT;
               // Keep boundary based on pointer priority.
               boundaryNode = CGM::min( bnode, boundaryNode );
            }
         }
         return state;
      }  break;
      case MetaNode::META_DIFFERENCE:
      {
         MetaOperation* nodeOperation = (MetaOperation*)node;
         uint numChildren = nodeOperation->numChildren();

         // No children then: OUTSIDE.
         if( numChildren == 0 ) return OUT;

         // Evalute first member.
         int state = evaluateCSG( nodeOperation->child(0), pos, boundaryNode );

         // Not contained in the first child then OUTSIDE.
         if( state == OUT ) return OUT;

         // Evaluate subtracting members.
         for( uint i = 1; i < numChildren; ++i )
         {
            int curState = evaluateCSG( nodeOperation->child(i), pos, bnode );
            
            if( curState == IN ) return OUT;
            if( curState == OUT ) continue;
            // Flip state.
            curState = (curState == BOUT) ? BIN : BOUT;
            // Handling of boundary cases.
            if( state == IN )
            {
               boundaryNode = bnode;
               state        = curState;
            }
            else
            {
               if( state != curState ) return OUT;
               // Keep boundary based on pointer priority.
               boundaryNode = CGM::min( bnode, boundaryNode ); 
            }
         }
         return state;
      }  break;
      default: break; 
   }
   return OUT;
}

/*==============================================================================
   CLASS MetaBuilder
==============================================================================*/

//------------------------------------------------------------------------------
//! 
MetaBuilder::MetaBuilder( MetaGeometry* geom )
{
   _geom = geom;
}

//------------------------------------------------------------------------------
//! 
MetaBuilder::~MetaBuilder()
{   
}

//------------------------------------------------------------------------------
//! 
MetaBlock*
MetaBuilder::createBlock()
{
   MetaBlock* block     = _geom->_bPool.alloc();
   block->_groupID      = 0;
   block->_id           = 0;
   block->_creases      = 0; // No creases.
   block->_subdivisions = 0; // All 1x1.
   return block;
}

//------------------------------------------------------------------------------
//! 
MetaBlocks* 
MetaBuilder::createBlocks()
{
   MetaBlocks* node = new MetaBlocks();
   _geom->_nodes.pushBack( node );
   return node;
}

//------------------------------------------------------------------------------
//! 
MetaCompositeBlocks* 
MetaBuilder::createCompositeBlocks()
{
   MetaCompositeBlocks* node = new MetaCompositeBlocks();
   _geom->_nodes.pushBack( node );
   return node;
}

//------------------------------------------------------------------------------
//! 
MetaDifference* 
MetaBuilder::createDifference()
{
   MetaDifference* node = new MetaDifference();
   _geom->_nodes.pushBack( node );
   return node;
}

//------------------------------------------------------------------------------
//! 
MetaIntersection* 
MetaBuilder::createIntersection()
{
   MetaIntersection* node = new MetaIntersection();
   _geom->_nodes.pushBack( node );
   return node;
}

//------------------------------------------------------------------------------
//! 
MetaTransform* 
MetaBuilder::createTransform()
{
   MetaTransform* node = new MetaTransform();
   _geom->_nodes.pushBack( node );
   return node;
}
   
//------------------------------------------------------------------------------
//! 
MetaUnion* 
MetaBuilder::createUnion()
{
   MetaUnion* node = new MetaUnion();
   _geom->_nodes.pushBack( node );
   return node;
}

//------------------------------------------------------------------------------
//! 
MetaComposite*
MetaBuilder::createComposite()
{
   MetaComposite* node = new MetaComposite();
   _geom->_nodes.pushBack( node );
   _geom->_composites.pushBack( node );
   return node;
}

//------------------------------------------------------------------------------
//! 
MetaInput*
MetaBuilder::createInput()
{
   MetaInput* node = new MetaInput();
   _geom->_nodes.pushBack( node );
   return node;
}

//------------------------------------------------------------------------------
//! 
MetaFunction*
MetaBuilder::createFunction()
{
   MetaFunction* func = new MetaFunction();
   _geom->_functions.pushBack( func );
   return func;
}

//------------------------------------------------------------------------------
//! 
void 
MetaBuilder::add( MetaBlocks* group, MetaBlock* block )
{
   group->add( block );
   block->_group = group;
}

//------------------------------------------------------------------------------
//! 
void
MetaBuilder::add( MetaBlocks* group, uint grp1, uint grp2 )
{
   group->add( grp1, grp2 );
}

//------------------------------------------------------------------------------
//! 
void 
MetaBuilder::add( MetaOperation* op, MetaNode* node )
{
   op->add( node );
}

//------------------------------------------------------------------------------
//! 
void
MetaBuilder::add( MetaCompositeBlocks* comp, MetaBlocks* blocks )
{
   comp->add( blocks );
}

//------------------------------------------------------------------------------
//! 
void 
MetaBuilder::set( MetaTransform* tr, MetaNode* node )
{
   tr->child( node );
}

//------------------------------------------------------------------------------
//! 
void
MetaBuilder::set( MetaNode* node, int layer )
{
   node->layer( layer );
}

//------------------------------------------------------------------------------
//! 
void 
MetaBuilder::set( MetaBlocks* blocks, const Mat4f& mat )
{
   blocks->transform( mat );
}

//------------------------------------------------------------------------------
//! 
void
MetaBuilder::set( MetaBlocks* blocks, MetaFunction* mapping, MetaFunction* displ )
{
   blocks->mapping( mapping );
   blocks->displacement( displ );
}

//------------------------------------------------------------------------------
//! 
void 
MetaBuilder::set( MetaTransform* tr, const Mat4f& mat )
{
   tr->transform( mat );
}

//------------------------------------------------------------------------------
//! 
void 
MetaBuilder::set( MetaComposite* comp, const Mat4f& mat )
{
   comp->transform( mat );
}

//------------------------------------------------------------------------------
//! 
void
MetaBuilder::set( MetaComposite* comp, MetaInput* input )
{
   comp->input( input );
}

//------------------------------------------------------------------------------
//! 
void 
MetaBuilder::set( MetaBlock* block, uint id, ushort grp, ushort creases, uint sub )
{
   block->_subdivisions = sub;
   block->_groupID      = grp;
   block->_id           = id;
   block->_creases      = creases;
}

//------------------------------------------------------------------------------
//! 
void
MetaBuilder::set( MetaBlock* block, const Vec3f pos[8] )
{
   for( uint v = 0; v < 8; ++v )
   {
      block->_localPos[v] = pos[v];
   }
}

//------------------------------------------------------------------------------
//! 
void 
MetaBuilder::setGeometricError( float error )
{
   _geom->_gerror = error;
}

//------------------------------------------------------------------------------
//! 
void 
MetaBuilder::setDetailsError( float error )
{
   _geom->_derror = error;
}

//------------------------------------------------------------------------------
//! 
void 
MetaBuilder::execute()
{
   _geom->update();
}
 
NAMESPACE_END




