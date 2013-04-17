/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_MATERIAL_H
#define PLASMA_MATERIAL_H

#include <Plasma/StdDefs.h>

#include <Fusion/Resource/Image.h>
#include <Fusion/Resource/ResManager.h>

#include <Gfx/Tex/Sampler.h>
#include <Gfx/Prog/Constants.h>

#include <CGMath/Variant.h>

#include <Base/ADT/Vector.h>
#include <Base/Util/RCObject.h>

NAMESPACE_BEGIN

/*==============================================================================
   CLASS Material
==============================================================================*/

class Material:
   public RCObject
{
public:

   /*----- static methods -----*/

   static void  initialize();
   static void  terminate();

   /*----- types and enumerations -----*/

   enum {
      BASE,
      REFLECTIVE_PLANAR,
      CUSTOM
   };

   /*----- static methods -----*/

   static Material* white();

   /*----- methods -----*/

   inline int type() const { return _type; }

   inline const RCP<Gfx::SamplerList>& samplers();
   inline const RCP<Gfx::ConstantList>& constants();

   virtual void updateSamplers() = 0;

protected:

   /*----- methods -----*/

   Material( int type );
   ~Material();
   virtual void createSamplers() = 0;
   virtual void createConstants() = 0;

   /*----- data members -----*/

   int                    _type;
   RCP<Gfx::SamplerList>  _samplers;
   RCP<Gfx::ConstantList> _constants;
};

//------------------------------------------------------------------------------
//!
inline const RCP<Gfx::SamplerList>&
Material::samplers()
{
   if( _samplers.isNull() ) createSamplers();
   return _samplers;
}

//------------------------------------------------------------------------------
//!
inline const RCP<Gfx::ConstantList>&
Material::constants()
{
   if( _constants.isNull() ) createConstants();
   return _constants;
}

/*==============================================================================
   CLASS BaseMaterial
==============================================================================*/

class BaseMaterial:
   public Material
{
public:

   /*----- structures -----*/

   struct Layer
   {
      Layer(): _clamp(0) {}
      Layer( int clamp, Image* img ): _clamp(clamp), _color( img ) {}
      Layer( Image* img ): _clamp(0), _color( img ) {}

      int        _clamp;
      RCP<Image> _color;
   };

   /*----- methods -----*/

   BaseMaterial();

   // Creation.
   void addLayer( const Layer& );
   void removeLayer( uint i );
   void removeAllLayers();

   void color( const Vec4f& );
   void brdf( float );

   // Accessors.
   inline         uint numLayers() const       { return uint(_layers.size()); }
   inline       Layer& layer( uint i )         { return _layers[i]; }
   inline const Layer& layer( uint i ) const   { return _layers[i]; }

   inline const Vec4f& color() const           { return _color; }
   inline float brdf() const                   { return _brdfID; }

   virtual void updateSamplers();

protected:

   /*----- methods -----*/

   ~BaseMaterial();
   void createSamplers();
   void createConstants();

   /*----- data members -----*/

   Vec4f                  _color;
   float                  _brdfID;
   RCP<Image>             _brdf;
   Vector<Layer>          _layers;
};

/*==============================================================================
   CLASS ReflectivePlanarMaterial
==============================================================================*/

class ReflectivePlanarMaterial:
   public BaseMaterial
{
public:

   /*----- methods -----*/

   ReflectivePlanarMaterial();

   void position( const Vec3f& pos );
   void normal( const Vec3f& n );
   inline const Vec3f& position() const { return _pos; }
   inline const Vec3f& normal() const   { return _normal; }

protected:

   /*----- data members -----*/

   Vec3f _pos;
   Vec3f _normal;
};

/*==============================================================================
   CLASS CustomMaterial
==============================================================================*/

class CustomMaterial:
   public Material
{
public:

   /*----- methods -----*/

   CustomMaterial();

   inline void  programName( const String& name )    { _progName = name; }
   inline const String&  programName() const         { return _progName; }

   inline void  program( Gfx::Program* prog )        { _prog = prog; }
   inline const RCP<Gfx::Program>&  program() const  { return _prog; }

   inline       Table&  variants()                   { return *_variants; }
   inline const Table&  variants() const             { return *_variants; }

   uint  addImage( Image* image )                    { _images.pushBack(image); return uint(_images.size()) - 1; }
   inline Image* image( uint idx ) const             { return _images[idx].ptr(); }

   void  print( TextStream& os = StdErr ) const;

   virtual void updateSamplers();

   void setConstant( const ConstString& name, const void* value );

protected:

   /*----- methods -----*/

   void guaranteeProgram();
   void createSamplers();
   void createConstants();

   String                _progName;
   RCP<Gfx::Program>     _prog;
   RCP<Table>            _variants;
   Vector< RCP<Image> >  _images;
};

NAMESPACE_END

#endif
