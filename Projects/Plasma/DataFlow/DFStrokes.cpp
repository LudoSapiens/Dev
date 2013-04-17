/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/DataFlow/DFStrokes.h>

#include <Plasma/DataFlow/DFGeometry.h>

/*==============================================================================
   UNNAME NAMESPACE
==============================================================================*/

UNNAMESPACE_BEGIN
UNNAMESPACE_END

NAMESPACE_BEGIN

/*==============================================================================
   CLASS DFStrokes
==============================================================================*/

//------------------------------------------------------------------------------
//!
DFStrokes::DFStrokes():
   _needUpdate(true)
{
   _blocks = new DFBlocks();
}

//------------------------------------------------------------------------------
//!
DFGeometry* DFStrokes::geometry()
{
   update();
   return _blocks->geometry();
}

//------------------------------------------------------------------------------
//!
uint DFStrokes::addStroke()
{
   _strokes.pushBack( Stroke() );
   return uint(_strokes.size())-1;
}

//------------------------------------------------------------------------------
//!
uint DFStrokes::addVertex( Stroke& s )
{
   s._vertices.pushBack( Vertex() );
   initVertex( s._vertices.back() );
   return uint(s._vertices.size())-1;
}

//------------------------------------------------------------------------------
//!
uint DFStrokes::insertVertex( Stroke& s, uint id )
{
   s._vertices.insert( s._vertices.begin()+id, Vertex() );
   initVertex( s._vertices[id] );
   return id;
}

//------------------------------------------------------------------------------
//!
uint DFStrokes::addSegment( Stroke& st )
{
   st._segments.pushBack( Segment() );
   initSegment( st._segments.back() );
   return uint(st._segments.size())-1;
}

//------------------------------------------------------------------------------
//!
uint DFStrokes::insertSegment( Stroke& st, uint id )
{
   st._segments.insert( st._segments.begin()+id, Segment() );
   initSegment( st._segments[id] );
   return id;
}

//------------------------------------------------------------------------------
//!
uint DFStrokes::addLink()
{
   _links.pushBack( Link() );
   Link& l = _links.back();

   l._v       = 0;
   l._stroke  = 0;
   l._segment = 0;

   return uint(_links.size())-1;
}

//------------------------------------------------------------------------------
//!
void DFStrokes::removeStroke( uint id )
{
   if( id >= _strokes.size() ) return;
   _strokes.erase( _strokes.begin() + id );
}

//------------------------------------------------------------------------------
//!
void DFStrokes::removeVertex( Stroke& s, uint id )
{
   // Remove vertex.
   if( id >= s._vertices.size() ) return;
   s._vertices.erase( s._vertices.begin() + id );

   // Remove segment.
   uint sid = id == 0 ? 0 : id-1;
   if( sid >= s._segments.size() ) return;
   s._segments.erase( s._segments.begin() + sid );
}

//------------------------------------------------------------------------------
//!
Reff DFStrokes::referential( const Stroke& s, uint vid ) const
{
   const Vertex& vertex = s._vertices[vid];

   if( vertex._flags & 0x01 )  return vertex._ref;

   uint numSeg = uint(s._segments.size());
   if( numSeg == 0 ) return s._vertices[0]._ref;

   uint32_t v0ID = vid < 1 ? 0 : vid-1;
   uint32_t v1ID = vid < numSeg ? vid+1 : numSeg;

   Vec3f tan = s._vertices[v1ID]._ref.position() - s._vertices[v0ID]._ref.position();
   Reff ref  = vertex._ref;

   Quatf qt = Quatf::twoVecs( ref.orientation().getAxisY(), tan );
   ref.orientation( qt * ref.orientation() );

   return ref;
}

//------------------------------------------------------------------------------
//!
void DFStrokes::invalidate()
{
   _needUpdate = true;
   _blocks = new DFBlocks();
}

//------------------------------------------------------------------------------
//!
void DFStrokes::initVertex( Vertex& v )
{
   v._ref          = Reff::identity();
   v._corners[0]   = Vec3f(-0.5f, 0.0f, -0.5f);
   v._corners[1]   = Vec3f(-0.5f, 0.0f,  0.5f);
   v._corners[2]   = Vec3f( 0.5f, 0.0f,  0.5f);
   v._corners[3]   = Vec3f( 0.5f, 0.0f, -0.5f);
   v._subdivisions = 0;
   v._creases      = 0;
   v._flags        = 0;
}

//------------------------------------------------------------------------------
//!
void DFStrokes::initSegment( Segment& s )
{
   s._subdivisions = 0;
   s._creases      = 0;
   s._partition    = 0;
}

//------------------------------------------------------------------------------
//!
void DFStrokes::update()
{
   if( !_needUpdate ) return;

   // TODO: creases, subdivisions.

   // Create blocks for each strokes.
   for( size_t sID = 0; sID < _strokes.size(); ++sID )
   {
      Stroke& s = _strokes[sID];
      Reff r0   = referential( s, 0 );
      for( size_t i = 0; i < s._segments.size(); ++i )
      {
         Segment& seg = s._segments[i];
         Vertex& v0   = s._vertices[i];
         Vertex& v1   = s._vertices[i+1];
         Reff r1      = referential( s, uint(i)+1 );
         Mat4f m0     = r0.toMatrix();

         ushort creases   = 0;
         uint subd        = 0;
         uint id          = 0;
         uint attractions = 0xA0;

         // Face subdivision.
         subd |= (seg._subdivisions&0xf);
         subd |= (seg._subdivisions&0xf0)<<16;
         subd |= (seg._subdivisions&0xf00)>>4;
         subd |= (seg._subdivisions&0xf000)<<4;

         // Vertex creases.
         if( v0._creases & 1 ) creases |= 1<<8;
         if( v0._creases & 2 ) creases |= 1<<3;
         if( v0._creases & 4 ) creases |= 1<<9;
         if( v0._creases & 8 ) creases |= 1<<0;

         // Lateral creases.
         creases |= seg._creases << 4;

         Vec3f corners[8];
         // First quad loop.
         corners[0] = m0 * v0._corners[0];
         corners[1] = m0 * v0._corners[3];
         corners[4] = m0 * v0._corners[1];
         corners[5] = m0 * v0._corners[2];

         // Intermediate loops.
         for( uint p = 0; p < seg._partition; ++p )
         {
            float t  = float(p+1)/float(seg._partition+1);
            Mat4f m1 = r0.slerp( r1, t ).toMatrix();
            corners[2] = m1 * CGM::linear2( v0._corners[0], v1._corners[0], t );
            corners[3] = m1 * CGM::linear2( v0._corners[3], v1._corners[3], t );
            corners[6] = m1 * CGM::linear2( v0._corners[1], v1._corners[1], t );
            corners[7] = m1 * CGM::linear2( v0._corners[2], v1._corners[2], t );

            // Bottom face subdivision.
            if( i == 0 && p == 0 ) subd |= (v0._subdivisions&0xf)<<8;
            else subd &= 0xfffff0ff;

            DFBlock* b = _blocks->createBlock();
            _blocks->set( b, id, ushort(sID), creases, subd, attractions );
            _blocks->set( b, corners );

            corners[0] = corners[2];
            corners[1] = corners[3];
            corners[4] = corners[6];
            corners[5] = corners[7];
         }

         // Last quad loop forming the last block.
         Mat4f m1 = r1.toMatrix();
         corners[2] = m1 * v1._corners[0];
         corners[3] = m1 * v1._corners[3];
         corners[6] = m1 * v1._corners[1];
         corners[7] = m1 * v1._corners[2];

         // Top vertex creases.
         if( v1._creases & 1 ) creases |= 1<<11;
         if( v1._creases & 2 ) creases |= 1<<2;
         if( v1._creases & 4 ) creases |= 1<<10;
         if( v1._creases & 8 ) creases |= 1<<1;

         // Bottom face subdivision.
         if( i == 0 && seg._partition == 0 ) subd |= (v0._subdivisions&0xf)<<8;
         // Top face subdivision.
         if( i == s._segments.size()-1 ) subd |= (v1._subdivisions&0xf)<<12;

         DFBlock* b = _blocks->createBlock();
         _blocks->set( b, id, ushort(sID), creases, subd, attractions );
         _blocks->set( b, corners );

         r0 = r1;
      }
   }

   // Links.

   _needUpdate = false;
}

//------------------------------------------------------------------------------
//!
void
DFStrokes::print( TextStream& os ) const
{
   uint n = numStrokes();
   os << n << " stroke" << (n>0?"s:":"") << nl;
   for( uint i = 0; i < n; ++i )
   {
      os << nl;
      os << "stroke #" << i << ":" << nl;
      print( _strokes[i], os );
      os << nl;
   }

   os << nl;
   n = numLinks();
   os << n << " link" << (n>0?"s:":"") << nl;
   for( uint i = 0; i < n; ++i )
   {
      os << "link #" << i << ":\t";
      print( _links[i], os );
      os << nl;
   }
}

//------------------------------------------------------------------------------
//!
void
DFStrokes::print( const Stroke& s, TextStream& os )
{
   uint n = s.numVertices();
   os << n << " vert" << (n>0?"ices:":"ex") << nl;
   for( uint i = 0; i < n; ++i )
   {
      os << "[" << i << "]:\t";
      print( s._vertices[i], os );
      os << nl;
   }

   n = s.numSegments();
   os << n << " segment" << (n>0?"s:":"") << nl;
   for( uint i = 0; i < n; ++i )
   {
      os << "[" << i << "]:\t";
      print( s._segments[i], os );
      os << nl;
   }
}

//------------------------------------------------------------------------------
//!
void
DFStrokes::print( const Vertex& v, TextStream& os )
{
   os << "r=" << v._ref
      << " "
      << "0:" << v._corners[0]
      << " "
      << "1:" << v._corners[1]
      << " "
      << "2:" << v._corners[2]
      << " "
      << "3:" << v._corners[3]
      << " "
      << "s=" << v._subdivisions
      << " "
      << "c=" << v._creases
      << " "
      << "f=" << v._flags;
}

//------------------------------------------------------------------------------
//!
void
DFStrokes::print( const Segment& s, TextStream& os )
{
   os << "s=" << s._subdivisions
      << " "
      << "c=" << s._creases
      << " "
      << "p=" << s._partition;
}

//------------------------------------------------------------------------------
//!
void
DFStrokes::print( const Link& l, TextStream& os )
{
   os << "v=" << l._v
      << " "
      << "st=" << l._stroke
      << " "
      << "sg=" << l._segment;
}



NAMESPACE_END
