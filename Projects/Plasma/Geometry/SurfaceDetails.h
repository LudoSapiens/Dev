/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_SURFACEDETAILS_H
#define PLASMA_SURFACEDETAILS_H

#include <Plasma/StdDefs.h>

#include <Fusion/VM/VM.h>
#include <Fusion/Resource/Bitmap.h>

#include <Gfx/Tex/Sampler.h>

#include <CGMath/AARect.h>
#include <CGMath/Vec2.h>

#include <Base/Util/RCObject.h>
#include <Base/Util/RCP.h>

NAMESPACE_BEGIN

class ParametricPatch;

/*==============================================================================
   CLASS SurfaceDetails
==============================================================================*/

class SurfaceDetails:
   public RCObject
{
public:

   /*----- structures -----*/ 

   struct Patch
   {
      Vec2i   _size;
      Vec2i   _offset;
      uint    _neighbors[4];
      ushort  _flipped;
      ushort  _edges;
   };

   struct Context{
      int   _id;
      int   _fid;
      Vec2f _uv;
      Vec3f _ipos;
      Vec3f _inor;
      Vec3f _opos;
      Vec3f _ocol;
   };

   /*----- static methods -----*/

   inline static int crease( const Patch& p, uint edge )         { return getbits( p._edges, 8+edge, 1 ); }
   inline static uint neighborPatch( const Patch& p, uint edge ) { return p._neighbors[edge]; }
   inline static uint neighborEdge( const Patch& p, uint edge )  { return getbits( p._edges, edge*2, 2 ); }
   inline static Vec2i patchOffset( const Patch& p, uint edge );

   /*----- methods -----*/

   SurfaceDetails();

   // Allocation.
   void clear();
   void reservePatches( uint num );

   // Creation.
   uint addPatch();
   uint addPatch( const Vec2i& size );

   Patch& patch( uint i )             { return _patches[i]; }
   const Patch& patch( uint i ) const { return _patches[i]; }

   // Atlas.
   void makeMinimalAtlas();
   void createAtlas();
   void beginAtlas();
   void endAtlas();
   void evaluatePatch( uint i, const ParametricPatch&, uint id, uint fid );
   void flipNormals( uint i );

   // Program.
   void programName( const String& );

   // Rendering texture.
   inline const RCP<Gfx::Sampler>& getColorSampler();
   inline const RCP<Gfx::Sampler>& getNormalSampler();

   // Mapping...
   void  mapping( uint i, Vec2f& uv, Vec2f& du, Vec2f& dv ) const;
   Vec3f position( uint p, const Vec2f& uv ) const;
   void  parameters( uint p, const Vec2f& uv, Vec3f& pos, Vec3f& normal ) const;

   int flatness( uint i, const Vec2f& uv0, const Vec2f& uv1, int edge, float error ) const;

protected:

   /*----- methods -----*/

   virtual ~SurfaceDetails();

   void fixBorders();
   void computeSamplers();
   void computeNormals();

private:


   /*----- data members -----*/

   Vec2f             _size;
   Context           _context;
   Vector<Patch>     _patches;
   String            _progName;
   VMState*          _vm;
   RCP<Bitmap>       _color;
   RCP<Bitmap>       _normal;
   RCP<Bitmap>       _displacement;
   RCP<Gfx::Sampler> _colorSampler;
   RCP<Gfx::Sampler> _normalSampler;
};

//------------------------------------------------------------------------------
//! 
inline const RCP<Gfx::Sampler>&
SurfaceDetails::getColorSampler()
{
   if( _colorSampler.isNull() ) computeSamplers();
   return _colorSampler;
}

//------------------------------------------------------------------------------
//! 
inline const RCP<Gfx::Sampler>&
SurfaceDetails::getNormalSampler()
{
   if( _normalSampler.isNull() ) computeSamplers();
   return _normalSampler;
}

//------------------------------------------------------------------------------
//! 
inline Vec2i 
SurfaceDetails::patchOffset( const Patch& p, uint edge )
{
   Vec2i off( p._offset );
   switch( edge + 4*p._flipped )
   {
      case 0:                                break;
      case 1: off.x += p._size.x-1;          break;
      case 2: off   += p._size - Vec2i(1);   break;
      case 3: off.y += p._size.y-1;          break;
      case 4: off.x += p._size.x-1;          break;
      case 5: off   += p._size - Vec2i(1);   break;
      case 6: off.y += p._size.y-1;          break;
      case 7:                                break;
   }
   return off;
}

NAMESPACE_END

#endif
