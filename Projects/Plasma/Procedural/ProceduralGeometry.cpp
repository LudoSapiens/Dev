/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/Procedural/ProceduralGeometry.h>
#include <Plasma/Procedural/ProceduralMaterial.h>
#include <Plasma/Geometry/MeshGeometry.h>
#include <Plasma/Physics/CollisionShapeGenerator.h>
#include <Plasma/Resource/ResManager.h>
#include <Plasma/Plasma.h>

#if MOTION_BULLET
#include <MotionBullet/Collision/BasicShapes.h>
#include <MotionBullet/Collision/CollisionGroup.h>
#else
#include <Motion/Collision/BasicShapes.h>
#include <Motion/Collision/CollisionGroup.h>
#endif

#include <Fusion/Resource/ResManager.h>
#include <Fusion/VM/VMRegistry.h>
#include <Fusion/Core/Core.h>

#include <Base/ADT/StringMap.h>
#include <Base/IO/FileSystem.h>

/*==============================================================================
  UNNAME NAMESPACE
==============================================================================*/

UNNAMESPACE_BEGIN

//------------------------------------------------------------------------------
//!
inline float triArea( const Vec2f& a, const Vec2f& b, const Vec2f& c )
{
   Vec2f u = b-a;
   Vec2f v = c-a;
   return u.y*v.x - u.x*v.y;
}

#if 0
//------------------------------------------------------------------------------
//!
inline bool ccw( const Vec2f& a, const Vec2f& b, const Vec2f& c, float& area )
{
   area = triArea( a, b, c );
   return area < 0.0f;
}
#endif

//------------------------------------------------------------------------------
//!
inline bool ccw( const Vec2f& a, const Vec2f& b, const Vec2f& c )
{
   return triArea( a, b, c ) < 0.0f;
}

/*==============================================================================
   Geometry functions.
==============================================================================*/

struct Vertex{
   Vec3f _pos;
   Vec3f _pos2;
   uint  _flag;
};

struct HEdge{
   ushort _vertex;
   ushort _next;
   ushort _prev;
   ushort _neighbor;
};

class Polytope{
public:
   Polytope(){}

   void build( const Boundary& b, uint f, int dv )
   {
      uint numVertices = b.numVertices(f);
      _vertices.resize(numVertices);
      _hedges.resize(numVertices);
      for( uint i = 0; i < numVertices; ++i )
      {
         _vertices[i]._pos    = b.vertex(f,i);
         _vertices[i]._pos2   = b.vertex( b.vertexID(f,i)+dv );
         _hedges[i]._vertex   = i;
         _hedges[i]._next     = (i+1)%numVertices;
         _hedges[i]._prev     = (i-1+numVertices)%numVertices;
         _hedges[i]._neighbor = 0xffff;
      }
      _plane = b.face(f).plane();
      _plane.direction().secAxes( _x, _y );
   }

   void quadrangulate( const Vec3f& x, const Vec3f& y )
   {
      // Compute witch vertices needs to be clipped and in what directions.
      for( uint i = 0; i < _hedges.size(); ++i )
      {
         HEdge* he0 = &_hedges[i];
         HEdge* he1 = &_hedges[he0->_next];
         HEdge* he2 = &_hedges[he1->_next];
         Vec3f v0   = _vertices[he0->_vertex]._pos;
         Vec3f v1   = _vertices[he1->_vertex]._pos;
         Vec3f v2   = _vertices[he2->_vertex]._pos;
         Vec3f e0   = normalize(v0-v1);
         Vec3f e1   = normalize(v2-v1);
         Vec3f t0   = cross( _plane.direction(), e0 );
         Vec3f t1   = cross( e1, _plane.direction() );

         uint flag = 0;

         // Verify which direction is contained inside.
         // Test  different condition depending on the convexity of the corner.
         float convexity = dot( cross( e1, e0 ), _plane.direction() );
         float t0x = dot( t0, x );
         float t1x = dot( t1, x );
         float t0y = dot( t0, y );
         float t1y = dot( t1, y );

         if( convexity > 0.0f )
         {
            flag |= (t0x<0)&&(t1x<0) ? 1 : 0;  // +x
            flag |= (t0x>0)&&(t1x>0) ? 2 : 0;  // -x
            flag |= (t0y<0)&&(t1y<0) ? 4 : 0;  // +y
            flag |= (t0y>0)&&(t1y>0) ? 8 : 0;  // -y
         }
         else
         {
            flag |= (t0x<0)||(t1x<0) ? 1 : 0;  // +x
            flag |= (t0x>0)||(t1x>0) ? 2 : 0;  // -x
            flag |= (t0y<0)||(t1y<0) ? 4 : 0;  // +y
            flag |= (t0y>0)||(t1y>0) ? 8 : 0;  // -y
         }

         // Remove direction near existant edges.
         if( flag > 0 )
         {
            const float cosError = 0.9f;
            float e0x = dot( e0, x );
            float e1x = dot( e1, x );
            if( e0x >  cosError ) flag &= 0xe;
            if( e1x >  cosError ) flag &= 0xe;
            if( e0x < -cosError ) flag &= 0xd;
            if( e1x < -cosError ) flag &= 0xd;
            float e0y = dot( e0, y );
            float e1y = dot( e1, y );
            if( e0y >  cosError ) flag &= 0xb;
            if( e1y >  cosError ) flag &= 0xb;
            if( e0y < -cosError ) flag &= 0x7;
            if( e1y < -cosError ) flag &= 0x7;
         }

         _vertices[he1->_vertex]._flag = flag;
      }

      // Loop on all exterior vertices and do some clipping when necessary.
      uint numVertices = uint(_vertices.size());
      for( uint i = 0; i < numVertices; ++i )
      {
         uint flag = _vertices[_hedges[i]._vertex]._flag;
         if( flag == 0 ) continue;
         if( flag & 1 ) clip( i,  x, 0xd );
         if( flag & 2 ) clip( i, -x, 0xe );
         if( flag & 4 ) clip( i,  y, 0x7 );
         if( flag & 8 ) clip( i, -y, 0xb );
      }
   }

   void clip( uint edge, const Vec3f& dir, uint mask )
   {
      float error = 0.0001f;
      Vec2f d0  = normalize(dir(_x,_y));
      Vec3f ori = _vertices[_hedges[edge]._vertex]._pos;

      // Find the correct edge so that dir and edge are in the same polygon.
      uint pedge = _hedges[edge]._prev;
      Vec2f e0   = normalize((_vertices[_hedges[_hedges[edge]._next]._vertex]._pos-ori)(_x,_y));
      for( uint nedge = _hedges[pedge]._neighbor; nedge != 0xffff;  )
      {
         // Check if dir is between edge and nedge.
         Vec2f e1 = normalize((_vertices[_hedges[pedge]._vertex]._pos-ori)(_x,_y));
         if( ccw( e0, d0, e1 ) ) break;

         // Next edge.
         edge  = nedge;
         pedge = _hedges[edge]._prev;
         nedge = _hedges[pedge]._neighbor;
         e0    = e1;
      }

      // Search in each edge for the nearest intersection.
      float t   = CGConstf:: infinity();
      float t2  = 0.0f;
      uint mine = 0;
      int ve    = -1;
      for( uint e = _hedges[edge]._next; e != edge; e = _hedges[e]._next )
      //for( uint e = 0; e < _hedges.size(); ++e )
      {
         HEdge* he0 = &_hedges[e];
         HEdge* he1 = &_hedges[he0->_next];
         Vec3f v0   = _vertices[he0->_vertex]._pos;
         Vec3f v1   = _vertices[he1->_vertex]._pos;

         // Compute edge intersection.
         Vec2f d1 = (v1-v0)(_x,_y);
         Vec2f d2 = (ori-v0)(_x,_y);

         float den  = (d1.y*d0.x)-(d1.x*d0.y);
         float num0 = (d1.x*d2.y)-(d1.y*d2.x);
         float num1 = (d0.x*d2.y)-(d0.y*d2.x);

         if( den <= 0.0f ) continue;

         // Edges are parallel.
         if( CGM::equal( den, 0.0f ) )
         {
            // Edges overlapping.
            if( CGM::equal( num0, 0.0f ) && CGM::equal( num1, 0.0f ))
            {
               // Test only for v0. v1 will be test on another edge.
               float t0 = -dot( d2, d0 );
               if( t0 < error ) continue;
               if( t0 < t )
               {
                  t    = t0;
                  mine = e;
                  ve   = 0;
               }
            }
         }
         else
         {
            float t0 = num0/den;
            float t1 = num1/den;

            // Is the intersection in a valid range on dir?
            if( t0 < error || t0 > t ) continue;

            // Testing vertices.
            Vec3f pos = CGM::linear2( v0, v1, t1 );
            if( equal( pos, v0, error ) )
            {
               t    = t0;
               mine = e;
               ve   = 0;
            }
            else if( equal( pos, v1, error ) )
            {
               t    = t0;
               mine = _hedges[e]._next;
               ve   = 0;
            }
            // Testing edge.
            else if( t1 > 0.0f && t1 < 1.0f )
            {
               t    = t0;
               t2   = t1;
               mine = e;
               ve   = -1;
            }
         }
      }

      // Insert new edge.
      // 1. Add a new vertex if necessary.
      uint next = 0xffff;
      if( ve < 0 )
      {
         HEdge* he0 = &_hedges[mine];
         HEdge* he1 = &_hedges[he0->_next];
         Vec3f v0   = _vertices[he0->_vertex]._pos;
         Vec3f v1   = _vertices[he0->_vertex]._pos2;
         Vec3f v2   = _vertices[he1->_vertex]._pos;
         Vec3f v3   = _vertices[he1->_vertex]._pos2;
         Vertex v;
         v._flag = 0;
         v._pos  = CGM::linear2( v0, v2, t2 );
         v._pos2 = CGM::linear2( v1, v3, t2 );
         _vertices.pushBack(v);
         // Update edges.
         HEdge he;
         he._vertex              = ushort(_vertices.size())-1;
         he._neighbor            = _hedges[mine]._neighbor;
         he._next                = _hedges[mine]._next;
         he._prev                = mine;
         _hedges[mine]._next     = ushort(_hedges.size());
         _hedges[he._next]._prev = ushort(_hedges.size());
         _hedges.pushBack(he);
         // add a neighbor edge.
         if( he._neighbor != 0xffff )
         {
            uint nei                = he._neighbor;
            _hedges[nei]._neighbor  = ushort(_hedges.size())-1;
            he._neighbor            = mine;
            he._next                = _hedges[nei]._next;
            he._prev                = nei;
            _hedges[mine]._neighbor = ushort(_hedges.size());
            _hedges[nei]._next      = ushort(_hedges.size());
            _hedges[he._next]._prev = ushort(_hedges.size());
            _hedges.pushBack(he);
            next = uint(_hedges.size())-1;
         }
         mine = _hedges[mine]._next;
      }

      // 2. Insert edge.
      HEdge he;
      he._vertex   = _hedges[edge]._vertex;
      he._neighbor = ushort(_hedges.size()) + 1;
      he._next     = mine;
      he._prev     = _hedges[edge]._prev;
      _hedges.pushBack(he);
      HEdge nhe;
      nhe._vertex   = _hedges[mine]._vertex;
      nhe._neighbor = ushort(_hedges.size())-1;
      nhe._next     = edge;
      nhe._prev     = _hedges[mine]._prev;
      _hedges.pushBack(nhe);
      _hedges[he._prev]._next  = nhe._neighbor;
      _hedges[mine]._prev      = nhe._neighbor;
      _hedges[edge]._prev      = he._neighbor;
      _hedges[nhe._prev]._next = he._neighbor;

      // 3. possibly remove flag of the destination vertex.
      _vertices[_hedges[mine]._vertex]._flag &= mask;

      // Continue clipping if not on exterior boundary.
      if( next != 0xffff ) clip( next, dir, mask );
   }

   void createBlocks(
      MetaBuilder* builder,
      const Mat4f& mat,
      MetaBlocks*  group,
      ushort       id,
      ushort       groupID,
      ushort       creases,
      uint         subdivisions
   )
   {
      // test polygons...
      Vector<ushort> vertices;
      Vector<ushort> sizes;

      // Create poylgons/quads.
      bool onlyQuads = true;
      for( uint i = 0; i < _hedges.size(); ++i )
      {
         if( _hedges[i]._vertex == 0xffff ) continue;
         // We have found a new poly, now construct it.
         uint e = i;
         uint size = 0;
         do
         {
            ++size;
            vertices.pushBack( _hedges[e]._vertex );
            _hedges[e]._vertex = 0xffff;
            e = _hedges[e]._next;
         } while( e != i );
         sizes.pushBack( size );
         if( size != 4 ) onlyQuads = false;
      }

      // Create blocs.
      Vec3f pos[8];
      uint p = 0;
      for( uint i = 0; i < sizes.size(); p += sizes[i], ++i )
      {
         if( onlyQuads )
         {
            MetaBlock* block = builder->createBlock();
            builder->add( group, block );
            builder->set( block, id, groupID, creases, subdivisions );
            pos[0] = mat * _vertices[vertices[p+0]]._pos2;
            pos[1] = mat * _vertices[vertices[p+1]]._pos2;
            pos[2] = mat * _vertices[vertices[p+3]]._pos2;
            pos[3] = mat * _vertices[vertices[p+2]]._pos2;
            pos[4] = mat * _vertices[vertices[p+0]]._pos;
            pos[5] = mat * _vertices[vertices[p+1]]._pos;
            pos[6] = mat * _vertices[vertices[p+3]]._pos;
            pos[7] = mat * _vertices[vertices[p+2]]._pos;
            builder->set( block, pos );
         }
         else
         {
            // compute center and edge vertices.
            Vec3f c1(0.0f);
            Vec3f c2(0.0f);
            for( uint v = 0; v < sizes[i]; ++v )
            {
               c1 += _vertices[vertices[p+v]]._pos;
               c2 += _vertices[vertices[p+v]]._pos2;
            }
            c1 /= (float)sizes[i];
            c2 /= (float)sizes[i];

            // create blocks.
            uint v0 = sizes[i]-2;
            uint v1 = sizes[i]-1;
            uint v2 = 0;
            for( ; v2 < sizes[i]; v0=v1, v1=v2++ )
            {
               MetaBlock* block = builder->createBlock();
               builder->add( group, block );
               builder->set( block, id, groupID, creases, subdivisions );
               Vertex* v0p = &_vertices[vertices[p+v0]];
               Vertex* v1p = &_vertices[vertices[p+v1]];
               Vertex* v2p = &_vertices[vertices[p+v2]];
               pos[0] = mat * ((v0p->_pos2+v1p->_pos2)*0.5f);
               pos[1] = mat * v1p->_pos2;
               pos[2] = mat * c2;
               pos[3] = mat * ((v1p->_pos2+v2p->_pos2)*0.5f);
               pos[4] = mat * ((v0p->_pos+v1p->_pos)*0.5f);
               pos[5] = mat * v1p->_pos;
               pos[6] = mat * c1;
               pos[7] = mat * ((v1p->_pos+v2p->_pos)*0.5f);
               builder->set( block, pos );
            }
         }
      }
   }

private:

   Planef         _plane;
   Vector<Vertex> _vertices;
   Vector<HEdge>  _hedges;
   int            _x;
   int            _y;
};


/*==============================================================================
   Procedural API/functions.
==============================================================================*/

// TODO:
// selections
// extrusions
// ...

//------------------------------------------------------------------------------
//!
inline GeometryContext* getContext( VMState* vm )
{
   return (GeometryContext*)VM::userData(vm);
}

//------------------------------------------------------------------------------
//!
void add( MetaBuilder* builder, const Vector<MetaNode*>& stack, MetaNode* node )
{
   if( stack.empty() ) return;

   MetaNode* last = stack.back();
   switch( last->type() )
   {
      case MetaNode::META_INPUT:
      case MetaNode::META_BLOCKS:
      case MetaNode::META_COMPOSITE_BLOCKS:
         break;
      case MetaNode::META_TRANSFORM:
      {
         MetaTransform* tr = (MetaTransform*)last;
         builder->set( tr, node );
      }  break;
      case MetaNode::META_COMPOSITE:
      case MetaNode::META_UNION:
      case MetaNode::META_INTERSECTION:
      case MetaNode::META_DIFFERENCE:
      {
         MetaOperation* op = (MetaOperation*)last;
         builder->add( op, node );
      }  break;
      default:
         StdErr << "Invalid MetaNode type of " << last->type() << " in add(), ignoring." << nl;
         break;
   }
}

//------------------------------------------------------------------------------
//!
void addShape( GeometryContext* context, CollisionShape* shape, const Reff& ref )
{
   context->_shapes.pushBack( GeometryContext::ShapeReferential(shape, ref) );
}

//------------------------------------------------------------------------------
//!
int executeVM( VMState* vm )
{
   int nargs = VM::getTop( vm ) - 1;

   // Read geometry id.
   String id = VM::toString( vm, 1 );

   // Find file name.
   const char* ext[] = { ".geom", 0 };
   String file = ResManager::idToPath( id, ext );
   if( file.empty() )
   {
      StdErr << "Geometry file '" << id << "' not found." << nl;
      return 0;
   }

   // Run geometry script.
   VM::doFile( vm, file, nargs, VM::MULTRET );

   // Compute number of returned arguments.
   int retarg = VM::getTop( vm ) - 1;

   // return component.
   return retarg;
}

//------------------------------------------------------------------------------
//!
int attractionVM( VMState* vm )
{
   GeometryContext* context = getContext(vm);

   // Not in a group?
   if( !context->_group )
   {
      StdErr << "Can't add attraction: not in a group.\n";
      return 0;
   }
   uint grp1 = VM::toUInt( vm, 1 );
   uint grp2 = VM::toUInt( vm, 2 );
   context->_builder->add( context->_group, grp1, grp2 );
   return 0;
}

//------------------------------------------------------------------------------
//!
int blockVM( VMState* vm )
{
   GeometryContext* context = getContext(vm);

   // Not in a group?
   if( !context->_group )
   {
      StdErr << "Can't create block: not in a group.\n";
      return 0;
   }

   MetaBuilder* builder = context->_builder;
   MetaBlock* block     = builder->createBlock();
   builder->add( context->_group, block );

   ushort id         = 0;
   ushort groupID    = 0;
   ushort creases    = 0;
   uint subdivisions = 0;
   VM::get( vm, -1, "id", id );
   if( id > 0 )  --id;
   VM::get( vm, -1, "g", groupID );
   VM::get( vm, -1, "c", creases );
   VM::get( vm, -1, "s", subdivisions );
   builder->set( block, id, groupID, creases, subdivisions );

   // Vertices.
   Vec3f pos[8];
   Mat4f m = context->_state._trf;
   switch( VM::getTableSize( vm, -1 ) )
   {
      case 0:
      {
         // No parameters: make a unit block.
         pos[0] = m * Vec3f( -1.0f, -1.0f, -1.0f );
         pos[1] = m * Vec3f(  1.0f, -1.0f, -1.0f );
         pos[2] = m * Vec3f( -1.0f,  1.0f, -1.0f );
         pos[3] = m * Vec3f(  1.0f,  1.0f, -1.0f );
         pos[4] = m * Vec3f( -1.0f, -1.0f,  1.0f );
         pos[5] = m * Vec3f(  1.0f, -1.0f,  1.0f );
         pos[6] = m * Vec3f( -1.0f,  1.0f,  1.0f );
         pos[7] = m * Vec3f(  1.0f,  1.0f,  1.0f );
      }  break;
      case 1:
      {
         VM::geti( vm, -1, 1 );
         Vec3f s;
         if( VM::isNumber( vm, -1 ) )
         {
            // Single scalar: radius (i.e. extent in all dimensions).
            s = Vec3f( CGM::abs( VM::toFloat( vm, -1 ) ) );
         }
         else
         {
            // Single vector: specifies extents.
            s = CGM::abs( VM::toVec3f( vm, -1 ) );
         }
         pos[0] = m * Vec3f( -s.x, -s.y, -s.z );
         pos[1] = m * Vec3f(  s.x, -s.y, -s.z );
         pos[2] = m * Vec3f( -s.x,  s.y, -s.z );
         pos[3] = m * Vec3f(  s.x,  s.y, -s.z );
         pos[4] = m * Vec3f( -s.x, -s.y,  s.z );
         pos[5] = m * Vec3f(  s.x, -s.y,  s.z );
         pos[6] = m * Vec3f( -s.x,  s.y,  s.z );
         pos[7] = m * Vec3f(  s.x,  s.y,  s.z );
         VM::pop( vm, 1 );
      }  break;
      case 2:
      {
         // Two vectors: min and max values in each axis.
         VM::geti( vm, -1, 1 );
         VM::geti( vm, -2, 2 );
         Vec3f a = VM::toVec3f( vm, -2 );
         Vec3f b = VM::toVec3f( vm, -1 );
         Vec3f min = CGM::min( a, b );
         Vec3f max = CGM::max( a, b );
         pos[0] = m * Vec3f( min.x, min.y, min.z );
         pos[1] = m * Vec3f( max.x, min.y, min.z );
         pos[2] = m * Vec3f( min.x, max.y, min.z );
         pos[3] = m * Vec3f( max.x, max.y, min.z );
         pos[4] = m * Vec3f( min.x, min.y, max.z );
         pos[5] = m * Vec3f( max.x, min.y, max.z );
         pos[6] = m * Vec3f( min.x, max.y, max.z );
         pos[7] = m * Vec3f( max.x, max.y, max.z );
         VM::pop( vm, 2 );
      }  break;
      case 5:
      {
         // Five vectors: a base quad, and an extrusion vector.
         for( int v = 1; v <= 4; ++v )
         {
            VM::geti( vm, -1, v );
            pos[v-1] = m * VM::toVec3f( vm, -1 );
            VM::pop( vm, 1 ); // Pop vertex.
         }
         VM::geti( vm, -1, 5 );
         Vec3f dir = m ^ VM::toVec3f( vm, -1 );
         pos[4] = pos[0] + dir;
         pos[5] = pos[1] + dir;
         pos[6] = pos[2] + dir;
         pos[7] = pos[3] + dir;
      }  break;
      case 8:
      {
         for( int v = 1; v <= 8; ++v )
         {
            VM::geti( vm, -1, v );
            pos[v-1] = m * VM::toVec3f( vm, -1 );
            VM::pop( vm, 1 );
         }
      }  break;
      default:
      {
         // Invalid format.
         StdErr << "Invalid block format; using degenerate block." << nl;
         pos[0] = m * Vec3f(0.0f);
         pos[1] = m * Vec3f(0.0f);
         pos[2] = m * Vec3f(0.0f);
         pos[3] = m * Vec3f(0.0f);
         pos[4] = m * Vec3f(0.0f);
         pos[5] = m * Vec3f(0.0f);
         pos[6] = m * Vec3f(0.0f);
         pos[7] = m * Vec3f(0.0f);
      }  break;
   }

   builder->set( block, pos );

   return 0;
}

//------------------------------------------------------------------------------
//!
int blocksVM( VMState* vm )
{
   GeometryContext* context = getContext(vm);

   // Not in a group?
   if( !context->_group )
   {
      StdErr << "Can't create block: not in a group.\n";
      return 0;
   }

   // Retrieve component to build blocks.
   Component* comp;
   if( VM::geti( vm, -1, 1 ) )
   {
      comp = (Component*)VM::toProxy( vm, -1 );
      VM::pop( vm, 1 );
   }
   else
   {
      StdErr << "No component given.\n";
      return 0;
   }

   // For now we only accept 3D prism component.
   Boundary* b = comp->boundary();
   if( !b || b->type() != Boundary::PRISM )
   {
      StdErr << "Component is not a prism.\n";
      return 0;
   }

   MetaBuilder* builder = context->_builder;

   // Reads parameters.
   // TODO: one group per wall...
   ushort id         = 0;
   ushort groupID    = 0;
   ushort creases    = 0xfff;
   uint subdivisions = 0;

   if( !VM::get( vm, -1, "g", groupID ) )
   {
      groupID = context->_grpID++;
      builder->add( context->_group, groupID, groupID );
   }

   VM::get( vm, -1, "id", id );
   if( id > 0 )  --id;
   //VM::get( vm, -1, "c", creases );
   //VM::get( vm, -1, "s", subdivisions );

   Vec3f pos[8];

   // Is the prism a block?
   if( (b->numFaces() == 6) && (b->numVertices() == 8) )
   {
      if( b->isConvex(1) )
      {
         MetaBlock* block = builder->createBlock();
         builder->add( context->_group, block );
         builder->set( block, id, groupID, creases, subdivisions );
         pos[0] = context->_state._trf * b->vertex(0);
         pos[1] = context->_state._trf * b->vertex(2);
         pos[2] = context->_state._trf * b->vertex(6);
         pos[3] = context->_state._trf * b->vertex(4);
         pos[4] = context->_state._trf * b->vertex(1);
         pos[5] = context->_state._trf * b->vertex(3);
         pos[6] = context->_state._trf * b->vertex(7);
         pos[7] = context->_state._trf * b->vertex(5);
         builder->set( block, pos );
         return 0;
      }
   }

   // General case.
   Polytope poly;
   poly.build( *b, 1, -1 );
   const Reff ref = comp->referential();
   poly.quadrangulate( ref.orientation().getAxisX(), ref.orientation().getAxisY() );
   poly.createBlocks( builder, context->_state._trf, context->_group, id, groupID, creases, subdivisions );

   return 0;
}

//------------------------------------------------------------------------------
//!
int blocksBeginVM( VMState* vm )
{
   GeometryContext* context = getContext(vm);

   // Already in group?
   if( context->_group )
   {
      StdErr << "Already in blocks group.\n";
      return 0;
   }

   MetaBlocks* node = context->_builder->createBlocks();
   add( context->_builder, context->_nodeStack, node );
   context->_builder->set(
      node,
      context->_state._mapping,
      context->_state._displacement
   );
   context->_group = node;
   context->pushNode( node );
   return 0;
}

//------------------------------------------------------------------------------
//!
int blocksEndVM( VMState* vm )
{
   GeometryContext* context = getContext(vm);

   if( !context->_group )
   {
      StdErr << "Blocks group can't be closed: not the last node on stack.\n";
      return 0;
   }
   context->_group = 0;
   context->popNode();
   return 0;
}

//------------------------------------------------------------------------------
//!
int compositeBlocksBeginVM( VMState* vm )
{
   GeometryContext* context = getContext(vm);

   // Already in group?
   if( context->_group )
   {
      StdErr << "Already in blocks group.\n";
      return 0;
   }

   int layer       = 0;
   Component* comp = 0;

   // Do we have a component to bind? And a layer?
   if( VM::getTop(vm) > 0 )
   {
      if( VM::isObject( vm, 1 ) )
      {
         comp = (Component*)VM::toProxy( vm, 1 );
      }
      else
      {
         layer = VM::toInt( vm, 1 );
      }
      if( VM::getTop(vm) > 1 )
      {
         layer = VM::toInt( vm, 2 );
      }
   }

   // Find the compositeBlocks for the requested layer.
   MetaCompositeBlocks* compBlocks = 0;
   if( !context->_nodeStack.empty() )
   {
      if( context->_nodeStack.back()->isOperation() )
      {
         MetaOperation* op = (MetaOperation*)context->_nodeStack.back();
         for( uint i = 0; i < op->numChildren(); ++i )
         {
            if( op->child(i)->type() == MetaNode::META_COMPOSITE_BLOCKS && op->child(i)->layer() == layer )
            {
               compBlocks = (MetaCompositeBlocks*)op->child(i);
               break;
            }
         }
      }
   }

   if( !compBlocks )
   {
      compBlocks =  context->_builder->createCompositeBlocks();
      add( context->_builder, context->_nodeStack, compBlocks );
      context->_builder->set( compBlocks, layer );
   }

   MetaBlocks* node = context->_builder->createBlocks();
   context->_builder->add( compBlocks, node );
   context->_compositor->bind( node, comp );
   context->_group = node;
   context->pushNode( node );
   return 0;
}

//------------------------------------------------------------------------------
//!
int compositeBlocksEndVM( VMState* vm )
{
   GeometryContext* context = getContext(vm);

   if( !context->_group )
   {
      StdErr << "Blocks group can't be closed: not the last node on stack.\n";
      return 0;
   }
   context->_group = 0;
   context->popNode();
   return 0;
}

//------------------------------------------------------------------------------
//!
int differenceBeginVM( VMState* vm )
{
   GeometryContext* context = getContext(vm);

   // In a group?
   if( context->_group )
   {
      StdErr << "Difference node can't be added to a group.\n";
      return 0;
   }

   MetaDifference* node = context->_builder->createDifference();
   add( context->_builder, context->_nodeStack, node );
   context->pushNode( node );
   return 0;
}

//------------------------------------------------------------------------------
//!
int differenceEndVM( VMState* vm )
{
   GeometryContext* context = getContext(vm);

   if( context->_nodeStack.empty() )
   {
      StdErr << "Difference can't be closed: nothing on the node stack.\n";
      return 0;
   }
   if( context->_nodeStack.back()->type() != MetaNode::META_DIFFERENCE )
   {
      StdErr << "Difference can't be closed: not the last node on stack.\n";
      return 0;
   }
   context->popNode();
   return 0;
}

//------------------------------------------------------------------------------
//!
int intersectionBeginVM( VMState* vm )
{
   GeometryContext* context = getContext(vm);

   // In a group?
   if( context->_group )
   {
      StdErr << "Intersection node can't be added to a group.\n";
      return 0;
   }

   MetaIntersection* node = context->_builder->createIntersection();
   add( context->_builder, context->_nodeStack, node );
   context->pushNode( node );
   return 0;
}

//------------------------------------------------------------------------------
//!
int intersectionEndVM( VMState* vm )
{
   GeometryContext* context = getContext(vm);

   if( context->_nodeStack.empty() )
   {
      StdErr << "Intersection can't be closed: nothing on the node stack.\n";
      return 0;
   }
   if( context->_nodeStack.back()->type() != MetaNode::META_INTERSECTION )
   {
      StdErr << "Intersection can't be closed: not the last node on stack.\n";
      return 0;
   }
   context->popNode();
   return 0;
}

//------------------------------------------------------------------------------
//!
int transformBeginVM( VMState* vm )
{
   GeometryContext* context = getContext(vm);
   // In a group?
   if( context->_group )
   {
      StdErr << "Transform node can't be added to a group.\n";
      return 0;
   }

   MetaTransform* node = context->_builder->createTransform();
   add( context->_builder, context->_nodeStack, node );
   context->pushNode( node );

   // Do we have transform parameters?
   if( VM::getTop(vm) > 0 )
   {
      Vec3f t( 0.0f );
      Quatf r = Quatf::identity();
      float s = 1.0f;
      VM::get( vm, -1, "t", t );
      VM::get( vm, -1, "r", r );
      VM::get( vm, -1, "s", s );
      Reff ref  = Reff( r, t, s );
      Mat4f mat = context->_state._trf * ref.toMatrix();
      context->_builder->set( node, mat );
      context->_state._ref = context->_state._ref * ref;
   }
   context->_state._trf = Mat4f::identity();

   return 0;
}

//------------------------------------------------------------------------------
//!
int transformEndVM( VMState* vm )
{
   GeometryContext* context = getContext(vm);

   if( context->_nodeStack.empty() )
   {
      StdErr << "Transform can't be closed: nothing on the node stack.\n";
      return 0;
   }
   if( context->_nodeStack.back()->type() != MetaNode::META_TRANSFORM )
   {
      StdErr << "Transform can't be closed: not the last node on stack.\n";
      return 0;
   }
   context->popNode();
   return 0;
}

//------------------------------------------------------------------------------
//!
int unionBeginVM( VMState* vm )
{
   GeometryContext* context = getContext(vm);

   // In a group?
   if( context->_group )
   {
      StdErr << "Union node can't be added to a group.\n";
      return 0;
   }

   MetaUnion* node = context->_builder->createUnion();
   add( context->_builder, context->_nodeStack, node );
   context->pushNode( node );
   return 0;
}

//------------------------------------------------------------------------------
//!
int unionEndVM( VMState* vm )
{
   GeometryContext* context = getContext(vm);

   if( context->_nodeStack.empty() )
   {
      StdErr << "Union can't be closed: nothing on the node stack.\n";
      return 0;
   }
   if( context->_nodeStack.back()->type() != MetaNode::META_UNION )
   {
      StdErr << "Union can't be closed: not the last node on stack.\n";
      return 0;
   }
   context->popNode();
   return 0;
}

//------------------------------------------------------------------------------
//!
int rotateVM( VMState* vm )
{
   GeometryContext* context = getContext(vm);

   Quatf q;
   if( VM::getTop( vm ) == 4 )
   {
      float r = VM::toFloat( vm, 1 );

      Vec3f a;
      a.x = VM::toFloat( vm, 2 );
      a.y = VM::toFloat( vm, 3 );
      a.z = VM::toFloat( vm, 4 );

      q = Quatf::axisAngle( a, CGM::cirToRad(r) );
   }
   else
   {
      q = VM::toQuatf( vm, 1 );
   }
   context->_state._trf = context->_state._trf * q.toMatrix();
   context->_state._ref.rotateLocal( q );
   return 0;
}

//------------------------------------------------------------------------------
//!
int translateVM( VMState* vm )
{
   Vec3f v;
   if( VM::type( vm, 1 ) == VM::NUMBER )
   {
      v.x = VM::toFloat( vm, 1 );
      v.y = VM::toFloat( vm, 2 );
      v.z = VM::toFloat( vm, 3 );
   }
   else
   {
      v = VM::toVec3f( vm, 1 );
   }

   GeometryContext* context = getContext(vm);
   context->_state._trf = context->_state._trf * Mat4f::translation( v );
   context->_state._ref.translateLocal( v );
   return 0;
}

//------------------------------------------------------------------------------
//!
int scaleVM( VMState* vm )
{
   float s = VM::toFloat( vm, 1 );

   GeometryContext* context = getContext(vm);
   context->_state._trf = context->_state._trf * Mat4f::scaling( s );
   context->_state._ref.scale( context->_state._ref.scale() * s );
   return 0;
}

//------------------------------------------------------------------------------
//!
int scopeBeginVM( VMState* vm )
{
   GeometryContext* context = getContext(vm);
   context->pushState();
   return 0;
}

//------------------------------------------------------------------------------
//!
int scopeEndVM( VMState* vm )
{
   GeometryContext* context = getContext(vm);
   context->popState();
   return 0;
}

//------------------------------------------------------------------------------
//!
int compositeBeginVM( VMState* vm )
{
   GeometryContext* context = getContext(vm);

   // In a group?
   if( context->_group )
   {
      StdErr << "Composite node can't be added to a group.\n";
      return 0;
   }

   // Composite node.
   MetaComposite* node = context->_builder->createComposite();
   add( context->_builder, context->_nodeStack, node );
   context->_composites.pushBack( Pair<MetaComposite*,bool>( node, false ) );
   context->pushNode( node );

   // Do we have a component to bind? And a layer?
   if( VM::getTop(vm) > 0 )
   {
      if( VM::isObject( vm, 1 ) )
      {
         Component* comp = (Component*)VM::toProxy( vm, 1 );
         context->_compositor->bind( node, comp );
         context->_compositor->scope( comp );
         context->_composites.back().second = true;
      }
      else
      {
         int layer = VM::toInt( vm, 1 );
         context->_builder->set( node, layer );
      }
      if( VM::getTop(vm) > 1 )
      {
         int layer = VM::toInt( vm, 2 );
         context->_builder->set( node, layer );
      }
   }

   return 0;
}

//------------------------------------------------------------------------------
//!
int compositeEndVM( VMState* vm )
{
   GeometryContext* context = getContext(vm);

   if( context->_nodeStack.empty() )
   {
      StdErr << "Composite can't be closed: nothing on the node stack.\n";
      return 0;
   }
   if( context->_nodeStack.back()->type() != MetaNode::META_COMPOSITE )
   {
      StdErr << "Composite can't be closed: not the last node on stack.\n";
      return 0;
   }

   if( context->_composites.back().second )
   {
      context->_compositor->unscope();
   }

   context->_composites.popBack();
   context->popNode();
   return 0;
}

//------------------------------------------------------------------------------
//!
int inputNodeVM( VMState* vm )
{
   GeometryContext* context = getContext(vm);

   // In a group?
   if( context->_group )
   {
      StdErr << "Input node can't be added to a group.\n";
      return 0;
   }
   // Do we have a parent composite node?
   if( context->_composites.empty() )
   {
      StdErr << "No parent composite node.\n";
   }

   MetaInput* node = context->_builder->createInput();
   add( context->_builder, context->_nodeStack, node );
   context->_builder->set( context->_composites.back().first, node );

   return 0;
}

//------------------------------------------------------------------------------
//!
int mappingVM( VMState* vm )
{
   GeometryContext* context = getContext(vm);

   // Read mapping function.
   if( VM::isFunction( vm, -1 ) )
   {
      // Create function.
      MetaFunction* func = context->_builder->createFunction();
      VM::getByteCode( vm, func->code() );
      context->_state._mapping = func;
      // Read parameters.
      int numParams = VM::getTop(vm)-1;
      for( int i = 0; i < numParams; ++i )
      {
         // Set key, value.
         VM::push( vm, i+1 );
         VM::pushValue( vm, i+1 );
         // Read.
         VM::toVariant( vm, func->parameters() );
      }
   }

   return 0;
}

//------------------------------------------------------------------------------
//!
int displacementVM( VMState* vm )
{
   GeometryContext* context = getContext(vm);

   // Read displacement function.
   if( VM::isFunction( vm, -1 ) )
   {
      // Create function.
      MetaFunction* func = context->_builder->createFunction();
      VM::getByteCode( vm, func->code() );
      context->_state._displacement = func;
      // Read parameters.
      int numParams = VM::getTop(vm)-1;
      for( int i = 0; i < numParams; ++i )
      {
         // Set key, value.
         VM::push( vm, i+1 );
         VM::pushValue( vm, i+1 );
         // Read.
         VM::toVariant( vm, func->parameters() );
      }
   }

   return 0;
}

//------------------------------------------------------------------------------
//!
StringMap _collision_strToType(
   "group"     , CollisionShape::GROUP     ,
   "sphere"    , CollisionShape::SPHERE    ,
   "box"       , CollisionShape::BOX       ,
   "cylinder"  , CollisionShape::CYLINDER  ,
   "cone"      , CollisionShape::CONE      ,
   "hull"      , CollisionShape::CONVEXHULL,
   "spheres"   , CollisionShape::SPHEREHULL,
   "trimesh"   , CollisionShape::TRIMESH   ,
   ""
);

//------------------------------------------------------------------------------
//!
int collisionVM( VMState* vm )
{
   GeometryContext* context = getContext(vm);

   const char* str = VM::toCString( vm, -1 );
   context->_autoCollisionShape = _collision_strToType[str];

   return 0;
}

//------------------------------------------------------------------------------
//!
int collisionBoxVM( VMState* vm )
{
   GeometryContext* context = getContext(vm);

   Vec3f size(1.0f);
   VM::get( vm, -1, "size", size );

   addShape( context, new BoxShape( size ), context->_state._ref );

   return 0;
}

//------------------------------------------------------------------------------
//!
int collisionConeVM( VMState* vm )
{
   GeometryContext* context = getContext(vm);

   float radius = 1.0f;
   float height = 1.0f;
   VM::get( vm, -1, "radius", radius );
   VM::get( vm, -1, "height", height );

   addShape( context, new ConeShape( radius, height ), context->_state._ref );

   return 0;
}

//------------------------------------------------------------------------------
//!
int collisionCylinderVM( VMState* vm )
{
   GeometryContext* context = getContext(vm);

   float radius = 1.0f;
   float height = 1.0f;
   VM::get( vm, -1, "radius", radius );
   VM::get( vm, -1, "height", height );

   addShape( context, new CylinderShape( radius, height ), context->_state._ref );

   return 0;
}

//------------------------------------------------------------------------------
//!
int collisionHullVM( VMState* vm )
{
   GeometryContext* context = getContext(vm);

   int size = VM::getTableSize( vm, -1 );

   RCP<ConvexHullShape> shape( new ConvexHullShape() );
   shape->reserveVertices( size );

   for( int i = 1; VM::geti( vm, -1, i ); ++i )
   {
      shape->addVertex( VM::toVec3f( vm, -1 ) );
      VM::pop( vm, 1 );
   }

   addShape( context, shape.ptr(), context->_state._ref );

   return 0;
}

//------------------------------------------------------------------------------
//!
int collisionSphereVM( VMState* vm )
{
   GeometryContext* context = getContext(vm);

   float radius = 1.0f;
   VM::get( vm, -1, "radius", radius );

   addShape( context, new SphereShape( radius ), context->_state._ref );

   return 0;
}

//------------------------------------------------------------------------------
//!
int collisionSpheresVM( VMState* vm )
{
   GeometryContext* context = getContext(vm);

   int size = VM::getTableSize( vm, -1 );

   RCP<SphereHullShape> shape( new SphereHullShape() );
   shape->reserveSpheres( size );

   for( int i = 1; VM::geti( vm, -1, i ); ++i )
   {
      Vec4f sphere = VM::toVec4f( vm, -1 );
      shape->addSphere( sphere );
      VM::pop( vm, 1 );
   }

   addShape( context, shape.ptr(), context->_state._ref );

   return 0;
}

//------------------------------------------------------------------------------
//!
int skeletonVM( VMState* vm )
{
   GeometryContext* context = getContext(vm);
   context->_skeletonRes    = ResManager::getSkeleton( VM::toString( vm, 1 ), context->task() );

   // Do we have a retarget table?
   if( VM::isTable( vm, 2 ) )
   {
      VM::push( vm );
      while( VM::next( vm, 2 ) )
      {
         context->_retarget[VM::toCString( vm, -2 )] = VM::toVec3f( vm, -1 );
         VM::pop( vm );
      }
   }

   return 0;
}

//------------------------------------------------------------------------------
//!
const VM::Reg funcs[] = {
   // Extern functions.
   { "execute",              executeVM              },
   // Geometries and transformations.
   { "attraction",           attractionVM           },
   { "block",                blockVM                },
   { "blocks",               blocksVM               },
   { "blocksBegin",          blocksBeginVM          },
   { "blocksEnd",            blocksEndVM            },
   { "differenceBegin",      differenceBeginVM      },
   { "differenceEnd",        differenceEndVM        },
   { "intersectionBegin",    intersectionBeginVM    },
   { "intersectionEnd",      intersectionEndVM      },
   { "rotate",               rotateVM               },
   { "scale",                scaleVM                },
   { "scopeBegin",           scopeBeginVM           },
   { "scopeEnd",             scopeEndVM             },
   { "transformBegin",       transformBeginVM       },
   { "transformEnd",         transformEndVM         },
   { "translate",            translateVM            },
   { "unionBegin",           unionBeginVM           },
   { "unionEnd",             unionEndVM             },
   // Composting.
   { "compositeBegin",       compositeBeginVM       },
   { "compositeEnd",         compositeEndVM         },
   { "compositeBlocksBegin", compositeBlocksBeginVM },
   { "compositeBlocksEnd",   compositeBlocksEndVM   },
   { "inputNode",            inputNodeVM            },
   // Surface.
   { "mapping",              mappingVM              },
   { "displacement",         displacementVM         },
   // Collision shapes.
   { "collision",            collisionVM            },
   { "collisionBox",         collisionBoxVM         },
   { "collisionCone",        collisionConeVM        },
   { "collisionCylinder",    collisionCylinderVM    },
   { "collisionHull",        collisionHullVM        },
   { "collisionSphere",      collisionSphereVM      },
   { "collisionSpheres",     collisionSpheresVM     },
   // Skeleton.
   { "skeleton",             skeletonVM             },
   { 0,0 }
};

//------------------------------------------------------------------------------
//!
void initVM( VMState* vm, uint /*mask*/ )
{
   VM::registerFunctions( vm, "_G", funcs );
}

UNNAMESPACE_END

NAMESPACE_BEGIN

/*==============================================================================
   CLASS GeometryContext
==============================================================================*/

//------------------------------------------------------------------------------
//!
GeometryContext::GeometryContext( Task* t ):
   ProceduralContext( GEOMETRY, t ),
   _compositor(0), _builder(0),
   _group(0),
   _grpID(1),
   _autoCollisionShape( StringMap::INVALID )
{
}


/*==============================================================================
   CLASS ProceduralGeometry
==============================================================================*/

//------------------------------------------------------------------------------
//!
void
ProceduralGeometry::initialize()
{
   VMRegistry::add( initVM, VM_CAT_GEOM );
}

//------------------------------------------------------------------------------
//!
ProceduralGeometry::ProceduralGeometry(
   Resource<Geometry>* res,
   const String&       path,
   bool                compiled
):
   _res( res ), _path( path ), _compiled( compiled )
{
   _gerror = Plasma::geometricError();
}

//------------------------------------------------------------------------------
//!
ProceduralGeometry::ProceduralGeometry(
   Resource<Geometry>*      res,
   const String&            path,
   const Table&             params,
   bool                     compiled
):
   _res( res ), _params( &params ), _path( path ), _compiled( compiled )
{
   _gerror = Plasma::geometricError();
}

//------------------------------------------------------------------------------
//!
void
ProceduralGeometry::execute()
{
   // Prepare to build geometry.
   RCP<MetaGeometry> metaGeom( new MetaGeometry() );
   MetaBuilder builder( metaGeom.ptr() );
   builder.setGeometricError( _gerror );
   builder.setDetailsError( 0.04f );

   // Compositor.
   RCP<Compositor> compositor( new Compositor() );

   // Create working context for this vm.
   GeometryContext context( this );
   context._compositor = compositor.ptr();
   context._builder    = &builder;

   // Open the vm.
   VMState* vm = VM::open( VM_CAT_GEOM | VM_CAT_MATH, true );

   // keep context pointer into the vm.
   VM::userData( vm, &context );

   // Push parameters.
   if( _params.isValid() )
   {
      VM::push( vm, *_params );
      // Execute script.
      VM::doFile( vm, _path, 1, 0 );
   }
   else
   {
      // Execute script.
      VM::doFile( vm, _path, 0 );
   }
   VM::close( vm );

   // Update composite nodes transform.
   compositor->updateTransform( builder );

   // Build MetaGeometry.
   builder.execute();

   // Compute final compiled geometry if necessary.
   RCP<Geometry> geom;
   if( _compiled || context._skeletonRes.isValid() )
   {
      // Convert MetaGeometry to mesh.
      geom = new MeshGeometry();
      metaGeom->createMesh( *geom->mesh() );

      // Create skeleton.
      if( context._skeletonRes.isValid() )
      {
         Skeleton* skel = waitForData( context._skeletonRes.ptr() );
         // Create the new skeleton.
         if( context._retarget.empty() )
         {
            geom->skeleton( skel );
         }
         else
         {
            // Retargeting.
            geom->skeleton( skel->retarget( context._retarget ).ptr() );
         }

         // Compute Weights for skinning.
         geom->computeBonesWeights();
      }
   }
   else
   {
      geom = metaGeom;
   }

   // Collision geometry.
   uint nShapes = uint(context._shapes.size());
   if( context._autoCollisionShape != StringMap::INVALID )
   {
      CollisionShapeGenerator gen;
      geom->collisionShape( gen.generate(context._autoCollisionShape, *geom) );
   }
   else
   if( nShapes > 0 )
   {
      if( nShapes == 1 && context._shapes.front().second == Reff::identity() )
      {
         geom->collisionShape( context._shapes.front().first.ptr() );
      }
      else
      {
         RCP<CollisionGroup> group = new CollisionGroup( nShapes );
         for( uint i = 0; i < nShapes; ++i )
         {
            const GeometryContext::ShapeReferential& cur = context._shapes[i];
            group->addShape( cur.second, cur.first.ptr() );
         }
         geom->collisionShape( group.ptr() );
      }
   }

   _res->data( geom.ptr() );
}

NAMESPACE_END
