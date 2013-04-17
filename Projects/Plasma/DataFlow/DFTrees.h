/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_DFTREES_H
#define PLASMA_DFTREES_H

#include <Plasma/StdDefs.h>

#include <Plasma/DataFlow/DFStrokes.h>
#include <Plasma/DataFlow/DFStrokesNodes.h>

#include <CGMath/Random.h>

NAMESPACE_BEGIN

/*==============================================================================
   STATIC INITIALIZATION
==============================================================================*/

void initializeTreeNodes();
void terminateTreeNodes();

/*==============================================================================
   CLASS DFTreeNodeWP
==============================================================================*/
//!< A tree generation node implementing "Creation and Rendering of Realistic Trees"
//!< by Weber and Penn (1995).
class DFTreeNodeWP:
   public DFNode
{
public:

   /*----- types -----*/
   enum Shape
   {
      SHAPE_CONICAL,
      SHAPE_SPHERICAL,
      SHAPE_HEMISPHERICAL,
      SHAPE_CYLINDRICAL,
      SHAPE_TAPERED_CYLINDRICAL,
      SHAPE_FLAME,
      SHAPE_INVERSE_CONICAL,
      SHAPE_TEND_FLAME,
      SHAPE_ENVELOPE
   };

   /*----- methods -----*/

   PLASMA_DLL_API DFTreeNodeWP();

   // Attributes.
   PLASMA_DLL_API virtual const ConstString& name() const;

   // Input/output.
   PLASMA_DLL_API virtual DFOutput* output();

   PLASMA_DLL_API virtual RCP<DFNodeEditor> edit();

   PLASMA_DLL_API virtual bool dumpCustom( TextStream& os, StreamIndent& indent ) const;

   PLASMA_DLL_API void  init( VMState* vm, int idx );

   static const char*  toStr( Shape v );
   static Shape  toShape( const char* s );

protected:

   friend class DFTreeNodeWPEditor;

   /*----- types -----*/

   struct LevelParams
   {
      uint16_t _curveRes;   //!< Number of segments for this stem.
      float    _curve;      //!< The angle to curve a stem.
      float    _curveV;     //!< The variation magnitude for _curve over the whole stem (<0 means helix).
      float    _curveBack;  //!< The angle to curve a stem back (S-curve).
      float    _downAngle;  //!< The angle at which branches come out.
      float    _downAngleV; //!< The variation magnitude for _downAngle (<0 means different distribution).
      float    _length;     //!< Length of the stem.
      float    _lengthV;    //!< Length variation of the stem.
      float    _segSplits;  //!< The number of splits at every segment.
      float    _taper;      //!< The taper control (1.0f means cone, 0.0f means cylinder).
      LevelParams():
         _curveRes(5), _curve(0.0f), _curveV(0.0f), _curveBack(0.0f),
         _downAngle(60.0f), _downAngleV(0.0f),
         _length(1.0f), _lengthV(0.0f),
         _segSplits(0.0f),
         _taper(1.0f)
      {}
   };

   /*----- data members -----*/

   DFStrokesOutput  _output;

   RNG_WELL     _rng;    //!< Random number generator used.

   // General.
   uint32_t     _seed;       //!< The random seed used for the tree.
   Shape        _shape;      //!< The global shape of the tree.
   float        _scale;      //!< Base size for the tree.
   float        _scaleV;     //!< Base size variation magnitude.
   float        _ratio;      //!< Ratio of radius to height.
   float        _baseSplits; //!< The number of splits on the trunk.
   LevelParams  _[4];        //!< The various parameters for up to 4 levels.


   /*----- methods -----*/

   RCP<DFStrokes> process();

   void  makeStem( const Reff& start, float length, float radius, const LevelParams& params, DFStrokes& strokes );
   void  makeSegments(
      const Vec3i& loop,
      float l, float a, float da, float r, float dr,
      float& curSplit, float segSplit, float splitAngle,
      Reff& ref, DFStrokes& strokes, DFStrokes::Stroke& st
   );

   inline float  rs() { return float(_rng())*2.0f - 1.0f; } // return 0.0f

};


/*==============================================================================
   CLASS DFTreeNodeJPH
==============================================================================*/
class DFTreeNodeJPH:
   public DFNode
{
public:

   /*----- types -----*/

   /*----- methods -----*/

   PLASMA_DLL_API DFTreeNodeJPH();

   // Attributes.
   PLASMA_DLL_API virtual const ConstString&  name() const;

   // Input/output.
   PLASMA_DLL_API virtual DFOutput*  output();

   PLASMA_DLL_API virtual RCP<DFNodeEditor>  edit();

   PLASMA_DLL_API virtual bool  dumpCustom( TextStream& os, StreamIndent& indent ) const;

   PLASMA_DLL_API void  init( VMState* vm, int idx );

protected:

   friend class DFTreeNodeJPHEditor;

   /*----- types -----*/

   struct Ring
   {
      Reff  _ref;  //!< The position and orientation of the ring.
      float _rad;  //!< The radius of the ring.
      Ring() {}
      Ring( const Reff& ref, float rad ): _ref(ref), _rad(rad) {}
   };

   struct Stem
   {
      int               _level;
      float             _length;
      Vector<uint32_t>  _rings;     //!< The rings ids composing the stem.
      //Vector<uint32_t>  _children;  //!< The child stem ids.
   };

   struct LevelParams
   {
      float  _crook;   //!< Crookedness factor.
      float  _crookD;  //!< Random magnitude for the crookedness.
      float  _photo;   //!< The phototropism factor.
      float  _photoD;  //!< Random magnitude for the phototropism.
      float  _radius;  //!< The radius, proportional to the length.
      LevelParams():
         _crook(0.0f), _crookD(0.0f),
         _photo(0.0f), _photoD(0.0f),
         _radius(0.8f)
      {}
   };

   /*----- data members -----*/

   DFStrokesOutput  _output;

   RNG_WELL      _rng;    //!< Random number generator used.

   // General.
   uint32_t      _seed;     //!< The random seed used for the tree.
   uint16_t      _detail;   //!< The detail level (number of subdivisions).
   uint16_t      _levels;   //!< The number of recursive levels to generate.
   float         _height;   //!< The height of the tree.
   float         _heightD;  //!< The random height delta.
   float         _photoS;   //!< A scaling factor for the phototropism effect.
   Vec3f         _sun;      //!< The direction fo the phototropism effect.
   Vec3f         _sunN;     //!< The _sun variable, normalized.
   Vector<Ring>  _rings;    //!< All of the rings in the tree.
   Vector<Stem>  _stems;    //!< All of the stems composing the tree (0 is trunk).

   LevelParams   _params[4]; //!< Parameters specified at every level (trunk/branch/subbranch/twig).


   /*----- methods -----*/

   RCP<DFStrokes>  process();

   void  makeTree();
   void  makeBranches( Stem& );
   uint32_t  makeStem(
      int   level,
      Reff  start,
      float length,
      float radius,
      uint  detail
   );
   RCP<DFStrokes>  stemsToStrokes() const;

   uint32_t  newRing( const Reff& ref, float rad )
   {
      _rings.pushBack( Ring(ref, rad) );
      return uint32_t(_rings.size()) - 1;
   }

   void  set( const Ring& src, DFStrokes::Vertex& dst ) const
   {
      dst._ref   = src._ref;
      dst._flags = 0x01;
      //dst._creases = 0x0F;
      dst.setRadius( src._rad );
   }

   inline float  rs() { return float(_rng())*2.0f - 1.0f; } // return 0.0f
   inline float  rs( float val, float var ) { return var*rs() + val; }

};


NAMESPACE_END

#endif
