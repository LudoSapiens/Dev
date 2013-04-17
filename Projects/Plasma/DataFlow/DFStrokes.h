/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_DFSTROKES_H
#define PLASMA_DFSTROKES_H

#include <Plasma/StdDefs.h>

#include <Plasma/DataFlow/DFBlocks.h>

NAMESPACE_BEGIN

// Forward declaration,
class DFGeometry;

// Stroke (set of vertices connected by Segments)
// Segment (connects two vertices + sub level per segment and per face + edges creases)
// Vertex (position,orientation, 4 2d vertices, sub per face and creases)
// Link (connected a vertices to a segment of a vertex)

// Compare memory requirement for many blocks...


/*==============================================================================
   CLASS DFStrokes
==============================================================================*/

class DFStrokes:
   public RCObject
{
public:

   /*----- structures -----*/

   struct Vertex
   {
      Reff     _ref;
      Vec3f    _corners[4];
      uint16_t _subdivisions;
      uint8_t  _creases;
      uint8_t  _flags;

      inline void  setRadius( float r );
      inline void  setRadius( float r1, float r2 );
   };

   struct Segment
   {
      uint16_t _subdivisions;
      uint8_t  _creases;
      uint8_t  _partition;
   };

   struct Link
   {
      uint32_t _v;
      uint32_t _stroke;
      uint32_t _segment;
   };

   struct Stroke
   {
      Vector<Vertex>  _vertices;
      Vector<Segment> _segments;
      uint numVertices() const { return uint(_vertices.size()); }
      uint numSegments() const { return uint(_segments.size()); }
   };

   /*----- methods -----*/

   PLASMA_DLL_API DFStrokes();

   // Creation.
   PLASMA_DLL_API uint addStroke();
   PLASMA_DLL_API uint addVertex( Stroke& );
   PLASMA_DLL_API uint insertVertex( Stroke&, uint id );
   PLASMA_DLL_API uint addSegment( Stroke& );
   PLASMA_DLL_API uint insertSegment( Stroke&, uint id );
   PLASMA_DLL_API uint addLink();

   PLASMA_DLL_API void removeStroke( uint id );
   PLASMA_DLL_API void removeVertex( Stroke&, uint id );

   // Attributes.
   uint numStrokes() const                                   { return uint(_strokes.size()); }
   uint numLinks() const                                     { return uint(_links.size());   }

   Stroke& stroke( uint id )                                 { return _strokes[id]; }
   const Stroke& stroke( uint id ) const                     { return _strokes[id]; }
   Vector<Stroke>& strokes()                                 { return _strokes; }
   const Vector<Stroke>& strokes() const                     { return _strokes; }

   Vertex& vertex( Stroke& s, uint id )                      { return s._vertices[id]; }
   const Vertex& vertex( const Stroke& s, uint id ) const    { return s._vertices[id]; }
   Vector<Vertex>& vertices( Stroke& s )                     { return s._vertices; }
   const Vector<Vertex>& vertices( const Stroke& s ) const   { return s._vertices; }

   Segment& segment( Stroke& s, uint id )                    { return s._segments[id]; }
   const Segment& segment( const Stroke& s, uint id ) const  { return s._segments[id]; }
   Vector<Segment>& segments( Stroke& s )                    { return s._segments; }
   const Vector<Segment>& segments( const Stroke& s ) const  { return s._segments; }

   Link& link( uint id )                                     { return _links[id]; }
   const Link& link( uint id ) const                         { return _links[id]; }

   PLASMA_DLL_API Reff referential( const Stroke& s, uint vid ) const;

   // Update.
   PLASMA_DLL_API void invalidate();

   // Surface.
   PLASMA_DLL_API DFGeometry* geometry();

          PLASMA_DLL_API void  print( TextStream& os = StdErr ) const;
   static PLASMA_DLL_API void  print( const Stroke& s, TextStream& os = StdErr );
   static PLASMA_DLL_API void  print( const Vertex& v, TextStream& os = StdErr );
   static PLASMA_DLL_API void  print( const Segment& s, TextStream& os = StdErr );
   static PLASMA_DLL_API void  print( const Link& l, TextStream& os = StdErr );

protected:

   /*----- members -----*/

   void initVertex( Vertex& );
   void initSegment( Segment& );
   void update();

   /*----- members -----*/

   bool             _needUpdate;
   Vector<Stroke>   _strokes;
   Vector<Link>     _links;
   RCP<DFBlocks>    _blocks;
};

//------------------------------------------------------------------------------
//!
inline void
DFStrokes::Vertex::setRadius( float r )
{
   _corners[0] = Vec3f(-r, 0.0f, -r);
   _corners[1] = Vec3f(-r, 0.0f,  r);
   _corners[2] = Vec3f( r, 0.0f,  r);
   _corners[3] = Vec3f( r, 0.0f, -r);
}

//------------------------------------------------------------------------------
//!
inline void
DFStrokes::Vertex::setRadius( float r1, float r2 )
{
   _corners[0] = Vec3f(-r1, 0.0f, -r2);
   _corners[1] = Vec3f(-r1, 0.0f,  r2);
   _corners[2] = Vec3f( r1, 0.0f,  r2);
   _corners[3] = Vec3f( r1, 0.0f, -r2);
}


NAMESPACE_END

#endif
