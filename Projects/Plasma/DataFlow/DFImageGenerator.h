/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_DFIMAGEGENERATOR_H
#define PLASMA_DFIMAGEGENERATOR_H

#include <Plasma/StdDefs.h>
#include <Plasma/DataFlow/DFImageNodes.h>

NAMESPACE_BEGIN

/*==============================================================================
   Static INITIALIZATION
==============================================================================*/

PLASMA_DLL_API void initializeImageGenerators();
PLASMA_DLL_API void terminateImageGenerators();

/*==============================================================================
   CLASS DFImportImageNode
==============================================================================*/

class DFImportImageNode:
   public DFNode
{
public:

   /*----- methods -----*/

   PLASMA_DLL_API DFImportImageNode();

   // Attributes.
   PLASMA_DLL_API virtual const ConstString& name() const;

   // Input/output.
   PLASMA_DLL_API virtual uint numInputs() const;

   PLASMA_DLL_API virtual DFOutput* output();
   PLASMA_DLL_API virtual DFInput* input( uint );

   inline const String& imageID() const { return _imageID; }
   PLASMA_DLL_API void imageID( const String& );

   // Editing.
   PLASMA_DLL_API virtual RCP<DFNodeEditor> edit();

   // Dump.
   PLASMA_DLL_API virtual bool  dumpCustom( TextStream& os, StreamIndent& indent ) const;


protected:

   /*----- methods -----*/

   RCP<Bitmap> process( const DFImageParams&  );

   /*----- data members -----*/

   DFImageOutput      _output;
   RCP<DFNodeEditor>  _editor;
   RCP<Image>         _image;
   String             _imageID;
   RCP<Bitmap>        _cachedBitmap;
   DFImageParams      _cachedParams;
};

/*==============================================================================
   CLASS DFFillColorNode
==============================================================================*/

class DFFillColorNode:
   public DFNode
{
public:

   /*----- methods -----*/

   PLASMA_DLL_API DFFillColorNode();

   // Attributes.
   PLASMA_DLL_API virtual const ConstString& name() const;

   // Input/output.
   PLASMA_DLL_API virtual uint numInputs() const;

   PLASMA_DLL_API virtual DFOutput* output();
   PLASMA_DLL_API virtual DFInput* input( uint );

   inline const Vec4f& color() const   { return _color; }
   inline void color( const Vec4f& c ) { _color = c; }

   // Editing.
   PLASMA_DLL_API virtual RCP<DFNodeEditor> edit();

   // Dump.
   PLASMA_DLL_API virtual bool  dumpCustom( TextStream& os, StreamIndent& indent ) const;

protected:

   /*----- methods -----*/

   RCP<Bitmap> process( const DFImageParams& );

   /*----- data members -----*/

   DFImageOutput     _output;
   RCP<DFNodeEditor> _editor;
   Vec4f             _color;
};

/*==============================================================================
   CLASS DFDrawRectNode
==============================================================================*/

class DFDrawRectNode:
   public DFNode
{
public:

   /*----- methods -----*/

   PLASMA_DLL_API DFDrawRectNode();

   // Attributes.
   PLASMA_DLL_API virtual const ConstString& name() const;

   // Input/output.
   PLASMA_DLL_API virtual uint numInputs() const;

   PLASMA_DLL_API virtual DFOutput* output();
   PLASMA_DLL_API virtual DFInput* input( uint );

   inline const Vec4f& color() const    { return _color; }
   inline void color( const Vec4f& c )  { _color = c; }

   inline const Vec2f& center() const   { return _center; }
   inline void center( const Vec2f& v ) { _center = v; }

   inline const Vec2f& size() const     { return _size; }
   inline void size( const Vec2f& v )   { _size = v; }

   // Editing.
   PLASMA_DLL_API virtual RCP<DFNodeEditor> edit();

   // Dump.
   PLASMA_DLL_API virtual bool  dumpCustom( TextStream& os, StreamIndent& indent ) const;

protected:

   /*----- methods -----*/

   RCP<Bitmap> process( const DFImageParams& );

   /*----- data members -----*/

   DFImageOutput     _output;
   RCP<DFNodeEditor> _editor;
   Vec4f             _color;
   Vec2f             _center;
   Vec2f             _size;
};

/*==============================================================================
   CLASS DFDrawCircleNode
==============================================================================*/

class DFDrawCircleNode:
   public DFNode
{
public:

   /*----- methods -----*/

   PLASMA_DLL_API DFDrawCircleNode();

   // Attributes.
   PLASMA_DLL_API virtual const ConstString& name() const;

   // Input/output.
   PLASMA_DLL_API virtual uint numInputs() const;

   PLASMA_DLL_API virtual DFOutput* output();
   PLASMA_DLL_API virtual DFInput* input( uint );

   inline const Vec4f& color() const    { return _color; }
   inline void color( const Vec4f& c )  { _color = c; }

   inline const Vec2f& center() const   { return _center; }
   inline void center( const Vec2f& v ) { _center = v; }

   inline float radius() const          { return _radius; }
   inline void radius( float v )        { _radius = v; }

   // Editing.
   PLASMA_DLL_API virtual RCP<DFNodeEditor> edit();

   // Dump.
   PLASMA_DLL_API virtual bool  dumpCustom( TextStream& os, StreamIndent& indent ) const;

protected:

   /*----- methods -----*/

   RCP<Bitmap> process( const DFImageParams& );

   /*----- data members -----*/

   DFImageOutput     _output;
   RCP<DFNodeEditor> _editor;
   Vec4f             _color;
   Vec2f             _center;
   float             _radius;
};

/*==============================================================================
   CLASS DFNoiseImageNode
==============================================================================*/

class DFNoiseImageNode:
   public DFNode
{
public:

   /*----- types and enumerations -----*/

   enum Type
   {
      TYPE_CELL,
      TYPE_PERLIN,
      TYPE_PERLIN_FILTERED,
      TYPE_VORONOI,
      TYPE_FBM,
   };

   /*----- methods -----*/

   PLASMA_DLL_API DFNoiseImageNode();

   // Attributes.
   PLASMA_DLL_API virtual const ConstString& name() const;

   // Input/output.
   PLASMA_DLL_API virtual DFOutput* output();

   inline Type type() const   { return _type; }
   inline void type( Type v ) { _type = v; }

   inline float widthPerlinFiltered() const    { return _perFil._width; }
   inline void  widthPerlinFiltered( float v ) { _perFil._width = v;    }

   inline float widthFBM() const    { return _fbm._width; }
   inline void  widthFBM( float v ) { _fbm._width = v;    }

   inline float jitter() const    { return _vor._jitter; }
   inline void  jitter( float v ) { _vor._jitter = v;    }

   inline uint  octaves() const   { return _fbm._octaves; }
   inline void  octaves( uint v ) { _fbm._octaves = v;    }

   inline float lacunarity() const    { return _fbm._lacunarity; }
   inline void  lacunarity( float v ) { _fbm._lacunarity = v;    }

   inline float gain() const    { return _fbm._gain; }
   inline void  gain( float v ) { _fbm._gain = v;    }

   // Editing.
   PLASMA_DLL_API virtual RCP<DFNodeEditor> edit();

   // Dump.
   PLASMA_DLL_API virtual bool  dumpCustom( TextStream& os, StreamIndent& indent ) const;

protected:

   /*----- methods -----*/

   PLASMA_DLL_API void  setDefaults();

   RCP<Bitmap> process( const DFImageParams& p  );

   /*----- data types -----*/

   struct PerlinFitered
   {
      float  _width;
   };

   struct Voronoi
   {
      float  _jitter;
   };

   struct FBM
   {
      float  _width;
      uint   _octaves;
      float  _lacunarity;
      float  _gain;
   };


   /*----- data members -----*/

   DFImageOutput     _output;
   RCP<DFNodeEditor> _editor;
   Type              _type;
   // Cell: no arguments.
   // Perlin: no arguments.
   PerlinFitered  _perFil;
   Voronoi        _vor;
   FBM            _fbm;
};

NAMESPACE_END

#endif
