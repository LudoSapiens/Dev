/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/DataFlow/DFBlocks.h>
#include <Plasma/Intersector.h>

#include <CGMath/Plane.h>

#define SUBFACE_MAPPING 1

/*==============================================================================
   UNNAME NAMESPACE
==============================================================================*/

UNNAMESPACE_BEGIN

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
inline void
neighbor( DFBlock::HEdge& e0, DFBlock::HEdge& e1 )
{
   e0._neighbor = &e1;
   e1._neighbor = &e0;
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
void
link( DFBlock::Face* fa, DFBlock::Face* fb, const Vec3f& ca, const Vec3f& cb )
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
void 
blocksOverlap( void* /*data*/, void* objA, void* objB )
{
   DFBlock* a = (DFBlock*)objA;
   DFBlock* b = (DFBlock*)objB;

   // Check if blocks can attract each other.
   //if( !a->group()->areAttracted( a->groupID(), b->groupID() ) ) return;

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
      int subA   = a->subdivision(i);
      uint attrA = a->attraction(i);

      // Try connecting side a to side or face b.
      if( subA == 0 )
      {
         // Compute side A center.
         Vec3f csa = a->side(i).center();

         // Test each side of b.
         for( uint j = 0; j < 6; ++j )
         {
            uint attrB = b->attraction(j);

            // Face attractions?
            if( (attrA | attrB) != 2 ) continue;

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
                  DFBlock::Face* f  = &b->side(j);
                  DFBlock::Face* fb = 0;
                  float dist        = CGConstf::infinity();
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
            uint attrB = b->attraction(j);

            // Face attractions?
            if( (attrA | attrB) != 2 ) continue;

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
               DFBlock::Face* f  = &a->side(i);
               DFBlock::Face* fa = 0;
               float dist        = CGConstf::infinity();
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


UNNAMESPACE_END

NAMESPACE_BEGIN

/*==============================================================================
   CLASS DFBlocks
==============================================================================*/

//------------------------------------------------------------------------------
//! 
DFBlocks::DFBlocks(): 
   _needUpdate(true), 
   _count(0), 
   _gerror(0.0125f),
   _bPool(32),
   _hePool(64),
   _fPool(64),
   _vPool(64),
   _grid(0)
{
   // FIXME: find better value for pool.
   _grid = new HGrid();
}

//------------------------------------------------------------------------------
//! 
DFBlocks::~DFBlocks()
{
   delete _grid;
}

//------------------------------------------------------------------------------
//! 
DFBlock* DFBlocks::createBlock()
{
   DFBlock* block     = _bPool.alloc();
   block->_groupID      = 0;
   block->_id           = 0;
   block->_creases      = 0;     // No creases.
   block->_subdivisions = 0;     // All 1x1.
   block->_attractions  = 0xAAA; // All connections enabled.

   _blocks.pushBack( block );

   return block;
}

//------------------------------------------------------------------------------
//! 
void DFBlocks::set( DFBlock* block, uint id, ushort grp, ushort creases, uint sub, uint attractions )
{
   block->_subdivisions = sub;
   block->_groupID      = grp;
   block->_id           = id;
   block->_creases      = creases;
   block->_attractions  = attractions;
}

//------------------------------------------------------------------------------
//! 
void DFBlocks::set( DFBlock* block, const Vec3f pos[8] )
{
   for( uint v = 0; v < 8; ++v )
   {
      block->position(v) = pos[v];
   }
}

//------------------------------------------------------------------------------
//! 
DFGeometry* DFBlocks::geometry()
{
   if( _surface.isNull() ) _surface = new DFGeometry();

   update();

   return _surface.ptr();
}

//------------------------------------------------------------------------------
//! 
void DFBlocks::update()
{
   if( !_needUpdate ) return;

   updateLinks();
   updateControlMesh();
   _surface->subdivide( _gerror );

   _needUpdate = false;
}

//------------------------------------------------------------------------------
//! 
void DFBlocks::updateLinks()
{
   // 1 Add all blocks to grid.
   for( uint b = 0; b < _blocks.size(); ++b )
   {
      DFBlock* block = _blocks[b];

      // Build block internal structure.
      updateTopology( block );

      // Compute BB.
      AABBoxf box = AABBoxf::empty();
      for( uint c = 0; c < 8; ++c ) box |= block->position(c);
      box.grow( box.size()*0.25f ); // how much????

      // Add to grid.
      _grid->add( block, box );
   }

   // 2 Establish all links.
   _grid->findAllCollisions( this, blocksOverlap );

   // 3 Clean all non-bidirectionnal links.
   for( uint b = 0; b < _blocks.size(); ++b )
   {
      DFBlock* block = _blocks[b];
      for( uint s = 0; s < 6; ++s )
      {
         for( DFBlock::Face* f = &block->side(s); f; f = f->_next )
         {
            if( f->_link && (f != f->_link->_link ) ) f->_link = 0;
         }
      }
   }

   // 4 TODO: Clean non legal links.
}

//------------------------------------------------------------------------------
//! 
void DFBlocks::updateControlMesh()
{
   Vector<DFBlock::Vertex*> vertices;

   // 1. Connect all edges and vertices.

   // 1.1 Connect edges.
   for( uint b = 0; b < _blocks.size(); ++b )
   {
      DFBlock* block = _blocks[b];
      for( uint s = 0; s < 6; ++s )
      {
         for( DFBlock::Face* f = &block->side(s); f; f = f->_next )
         {
            if( f->_link ) connect( f );
         }
      }
   }

   // 1.2 Connect neighbors faces.
   for( uint b = 0; b < _blocks.size(); ++b )
   {
      DFBlock* block = _blocks[b];
      for( uint s = 0; s < 6; ++s )
      {
         for( DFBlock::Face* f = &block->side(s); f; f = f->_next )
         {
            // Skip interior faces.
            if( f->_link ) continue;

            DFBlock::HEdge* he = &f->_corners[0];
            do
            {
               if( !he->_link )
               {
                  DFBlock::HEdge* ne = he->_neighbor;
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
   for( uint b = 0; b <_blocks.size(); ++b )
   {
      DFBlock* block = _blocks[b];
      for( uint s = 0; s < 6; ++s )
      {
         for( DFBlock::Face* f = &block->side(s); f; f = f->_next )
         {
            // Skip interior faces.
            if( f->_link ) continue;

            DFBlock::HEdge* he = &f->_corners[0];
            do
            {
               // Compute joint vertex if not already done.
               if( !he->_vertex->_jointVertex )
               {
                  // Accumulate all vertices.
                  Vec3f v( he->position() );
                  vertices.clear();
                  vertices.pushBack( he->_vertex );
                  for( DFBlock::HEdge* ne = he->_link->_next; ne != he; ne = ne->_link->_next )
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
                     DFBlock::Vertex* vertex = _vPool.alloc();
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
   for( uint b = 0; b < _blocks.size(); ++b )
   {
      DFBlock* block = _blocks[b];
      for( uint s = 0; s < 6; ++s )
      {
         for( DFBlock::Face* f = &block->side(s); f; f = f->_next )
         {
            // Skip interior faces.
            if( f->_link ) continue;

            uint centerID = _surface->addControlPoint();
            Vec3f& center = _surface->controlPoint( centerID );
            uint count    = 0;
            int  henum    = 0;

            // Compute UVs.
            Vector<Vec2f> uvs;
            uvs.reserve(4);

            DFBlock::HEdge* he = &f->_corners[0];
            do
            {
               float t0  = f->_corners[henum]._vertex->_t;
               float t1  = f->_corners[(henum+1)%4]._vertex->_t;
               float t   = he->_vertex->_t;
               Vec2f uv0 = faceUV[henum];
               Vec2f uv1 = faceUV[(henum+1)%4];

               Vec2f uv  = CGM::linear2( uv0, uv1, (t-t0)/(t1-t0) );
               uvs.pushBack( uv );
               he = he->_next;
               if( henum < 3 && he == &f->_corners[henum+1] ) ++henum;
            } while( he != &f->_corners[0] );

            henum = 0;
            he    = &f->_corners[0];
            do
            {
               center              += he->jointPosition();

               he->_patch           = _surface->addPatch();
               DFGeometry::Patch& p = _surface->patch( he->_patch );

               p._id                = block->id() | (s <<29);
               //p._id              = b | (s <<29);
               p._controlPts[0]     = centerID;
#ifndef SUBFACE_MAPPING
               p._uv[0]             = faceUV[(henum+2)%4];
               p._uv[1]             = faceUV[(henum+3)%4];
               p._uv[2]             = faceUV[(henum+4)%4];
               p._uv[3]             = faceUV[(henum+5)%4];
#else
               p._uv[0] = Vec2f(0.0f, 0.0f);
               p._uv[1] = Vec2f(1.0f, 0.0f);
               p._uv[2] = Vec2f(1.0f, 1.0f);
               p._uv[3] = Vec2f(0.0f, 1.0f);
               //p._uv[0]             = Vec2f( 0.5f, 0.5f );
               //p._uv[2]             = uvs[count];
               //p._uv[1]             = (p._uv[2] + uvs[((count+uvs.size())-1)%uvs.size()])*0.5f;
               //p._uv[3]             = (p._uv[2] + uvs[(count+1)%uvs.size()])*0.5f;
#endif
               he                   = he->_next;
                ++count;
               // Find the half-edge number in subface.
               if( henum < 3 && he == &f->_corners[henum+1] ) ++henum;
            } while( he != &f->_corners[0] );
            center /= float(count);
         }
      }
   }

   // 2.2 Add edge and corners vertices.
   for( uint b = 0; b < _blocks.size(); ++b )
   {
      DFBlock* block = _blocks[b];
      for( uint s = 0; s < 6; ++s )
      {
         for( DFBlock::Face* f = &block->side(s); f; f = f->_next )
         {
            // Skip interior faces.
            if( f->_link ) continue;

            DFBlock::HEdge* he = &f->_corners[0];
            do
            {
               // Setting neighbors.
               int crease = he->_flags | he->_link->_flags;
               _surface->neighbors( he->_patch, 2, he->_link->_next->_patch, 1, crease );
               _surface->neighbors( he->_patch, 3, he->_next->_patch, 0, 0 );

               DFGeometry::Patch& p = _surface->patch( he->_patch );

               // Edge vertex.
               if( p._controlPts[3] == DFGeometry::NULL_ID32 )
               {
                  uint vID = _surface->addControlPoint();
                  Vec3f& v = _surface->controlPoint( vID );

                  // Smooth or crease?
                  if( crease )
                  {
                     v += he->jointPosition();
                     v += he->_next->jointPosition();
                     v *= 0.5f;
                  }
                  else
                  {
                     v += _surface->controlPoint( p._controlPts[0] );
                     v += _surface->controlPoint( _surface->patch( he->_link->_patch )._controlPts[0] );
                     v += he->jointPosition();
                     v += he->_next->jointPosition();
                     v *= 0.25f;
                  }

                  p._controlPts[3]                                           = vID;
                  _surface->patch( he->_next->_patch )._controlPts[1]        = vID;
                  _surface->patch( he->_link->_patch )._controlPts[3]        = vID;
                  _surface->patch( he->_link->_next->_patch )._controlPts[1] = vID;
               }

               // Corner vertex.
               if( p._controlPts[2] == DFGeometry::NULL_ID32 )
               {
                  Vec3f vc(0.0f);
                  uint crCount = 0;
                  if( crease )
                  {
                     ++crCount;
                     vc += he->_next->jointPosition();
                  }

                  uint vID = _surface->addControlPoint();
                  Vec3f& v = _surface->controlPoint( vID );

                  v += _surface->controlPoint( p._controlPts[0] );
                  v += he->_next->jointPosition();
                  p._controlPts[2] = vID;
                  int count = 1;
                  for( DFBlock::HEdge* ne = he->_link->_next; ne != he; ne = ne->_link->_next )
                  {
                     DFGeometry::Patch& np = _surface->patch( ne->_patch );
                     np._controlPts[2] = vID;
                     v += _surface->controlPoint( np._controlPts[0] );
                     v += ne->_next->jointPosition();
                     ++count;
                     if( ne->_flags | ne->_link->_flags )
                     {
                        ++crCount;
                        vc += ne->_next->jointPosition();
                     }
                  }
                  if( crCount <= 1 )
                  {
                     v /= float(count*count);
                     v += he->jointPosition()*((float)(count-2)/(float)count);
                  }
                  else if( crCount == 2 )
                  {
                     v = (vc + he->jointPosition()*6.0f)*0.125;
                  }
                  else
                  {
                     v = he->jointPosition();
                  }
               }

               he = he->_next;
            } while( he != &f->_corners[0] );
         }
      }
   }
}

//------------------------------------------------------------------------------
//! 
void
DFBlocks::updateTopology( DFBlock* b )
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
      DFBlock::Face* f = &b->side(s);
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
DFBlocks::split( DFBlock::HEdge* e0, float frac )
{
   DFBlock::HEdge* ne0 = e0->_neighbor;

   // New Vertex.
   DFBlock::Vertex* v  = _vPool.alloc();
   v->_jointVertex     = 0;
   v->_position        = (ne0->position()-e0->position())*frac + e0->position();
   v->_t               = (ne0->t()-e0->t())*frac + e0->t();

   // New edges.
   DFBlock::HEdge* e1  = _hePool.alloc();
   DFBlock::HEdge* ne1 = _hePool.alloc();

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
DFBlocks::split( 
   DFBlock::HEdge*  startEdge, 
   DFBlock::HEdge*  endEdge, 
   const float*     fracs,
   DFBlock::HEdge** edges
)
{
   const float eps = CGConstf::epsilon(32.0f);

   // Find split direction.
   float dt  = endEdge->t() - startEdge->t();
   bool dir  = dt < 0.0f ? false : true;

   DFBlock::HEdge* curEdge  = startEdge;
   DFBlock::HEdge* prevEdge = 0;
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
DFBlocks::split( DFBlock::Face* f, int sx, int sy )
{
   DFBlock::Face* faces[16];
   DFBlock::HEdge* se0[4];
   DFBlock::HEdge* se1[4];
   DFBlock::HEdge* se2[4];
   DFBlock::HEdge* se3[4];

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
   DFBlock::HEdge* he       = &faces[sx*sy-1]->_corners[2];
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
         DFBlock::Face* f0         = faces[(y-1)*sx+x-1];
         DFBlock::Face* f1         = faces[(y-1)*sx+x];
         DFBlock::Face* f2         = faces[y*sx+x];
         DFBlock::Face* f3         = faces[y*sx+x-1];

         Vec2f uv                  = Vec2f( subFracs[sx][x-1], subFracs[sy][y-1] );
         DFBlock::Vertex* v        = _vPool.alloc();
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
   DFBlock::Face** fp   = 0;
   DFBlock::HEdge** hep = 0;
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
         DFBlock::Face* f1  = *fp; fp += fpinc;
         DFBlock::Face* f0  = *fp;
         DFBlock::HEdge* be = *hep--;

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
      DFBlock::Face* cf  = faces[i];
      DFBlock::HEdge* he = &cf->_corners[0];
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
DFBlocks::connect( DFBlock::Face* f )
{
   // Testing if face is already connected.
   if( f->_corners[0]._link != 0 ) return;

   Vector<float> fracs0;
   Vector<float> fracs1;
   Vector<float> fracs;
   fracs0.reserve(4);
   fracs1.reserve(4);
   fracs.reserve(4);

   Vector<DFBlock::HEdge*> edges;

   DFBlock::HEdge* e;

   for( int c = 0; c < 4; ++c )
   {
      // Find subdivision in current edge.
      fracs0.clear();
      DFBlock::HEdge* se0 = &f->_corners[c];
      DFBlock::HEdge* ee0 = &f->_corners[(c+1)%4];
      for( e = se0->_next; e != ee0; e = e->_next )
      {
         fracs0.pushBack( (e->t()-se0->t()) / (ee0->t()-se0->t()) );
      }

      // Find subdivision in opposite edge.
      fracs1.clear();
      edges.clear();
      DFBlock::HEdge* se1 = &f->_link->_corners[(f->_linkCorner-c+3)%4];
      DFBlock::HEdge* ee1 = &f->_link->_corners[(f->_linkCorner-c+4)%4];
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
      Vector<DFBlock::HEdge*>::ReverseIterator it = edges.rbegin();
      for( e = se0; e != ee0; e = e->_next, ++it )
      {
         e->_link     = *it;
         (*it)->_link = e;
      }
   }
}

NAMESPACE_END
