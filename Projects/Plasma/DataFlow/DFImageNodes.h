/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_DFIMAGENODE_H
#define PLASMA_DFIMAGENODE_H

#include <Plasma/StdDefs.h>
#include <Plasma/DataFlow/DFNode.h>

#include <Fusion/Resource/Image.h>

#include <CGMath/AARect.h>

#include <Base/Msg/Delegate.h>

NAMESPACE_BEGIN

/*==============================================================================
   STATIC INITIALIZATION
==============================================================================*/

PLASMA_DLL_API void initializeImageNodes();
PLASMA_DLL_API void terminateImageNodes();

/*==============================================================================
   STRUCT DFImageParams
==============================================================================*/

class DFImageParams
{
public:

   /*----- methods -----*/

   DFImageParams(): _region(0.0f), _size(0) {}
   DFImageParams( const AARectf& r, const Vec2i& s ): _region(r), _size(s) {}
   DFImageParams( const Vec2f& pos, const Vec2f& size, const Vec2i& s ):
      _region( pos, size ), _size(s) {}

   inline const AARectf& region() const { return _region; }
   inline const Vec2i& size() const     { return _size; }

   PLASMA_DLL_API Mat3f  regionToBitmap() const;
   PLASMA_DLL_API Mat3f  bitmapToRegion() const;

   inline bool  operator==( const DFImageParams& p ) const { return _region == p._region && _size == p._size; }

   /*----- data members -----*/

   AARectf _region;
   Vec2i   _size;
};

/*==============================================================================
   CLASS DFImageOutput
==============================================================================*/

class DFImageOutput:
   public DFOutput
{
public:

   /*----- types -----*/

   typedef Delegate1< const DFImageParams&, RCP<Bitmap> > ProcessDelegate;

   /*----- methods -----*/

   PLASMA_DLL_API RCP<Bitmap> getImage( const DFImageParams& );
   PLASMA_DLL_API virtual Type type() const;

   void delegate( const ProcessDelegate& d ) { _delegate = d; }

protected:

   /*----- data members -----*/

   ProcessDelegate _delegate;
};

/*==============================================================================
   CLASS DFImageInput
==============================================================================*/

class DFImageInput:
   public DFInput
{
public:

   /*----- methods -----*/

   DFImageInput( DFNode* n ): DFInput(n), _output(nullptr) {}

   PLASMA_DLL_API RCP<Bitmap> getImage( const DFImageParams& );
   PLASMA_DLL_API virtual Type type() const;
   PLASMA_DLL_API virtual bool isConnected() const;

protected:

   /*----- methods -----*/

   virtual void connect( DFOutput* );
   virtual void disconnect( DFOutput* );
   virtual void disconnect();

   /*----- data members -----*/

   DFImageOutput* _output;
};

/*==============================================================================
   CLASS DFImageMultiInput
==============================================================================*/

class DFImageMultiInput:
   public DFInput
{
public:

   /*----- methods -----*/

   DFImageMultiInput( DFNode* n ): DFInput(n) {}
   PLASMA_DLL_API RCP<Bitmap> getImage( uint, const DFImageParams& );

   inline size_t size() const { return _outputs.size(); }

   PLASMA_DLL_API virtual Type type() const;
   PLASMA_DLL_API virtual bool isConnected() const;
   PLASMA_DLL_API virtual bool isMulti() const { return true; }

protected:

   /*----- methods -----*/

   virtual void connect( DFOutput* );
   virtual void disconnect( DFOutput* );
   virtual void disconnect();

   /*----- data members -----*/

   Vector<DFImageOutput*> _outputs;
};

/*==============================================================================
   CLASS DFOutputImageNode
==============================================================================*/

class DFOutputImageNode:
   public DFNode
{
public:

   /*----- methods -----*/

   PLASMA_DLL_API DFOutputImageNode();

   // Attributes.
   PLASMA_DLL_API virtual const ConstString& name() const;

   // Input/output.
   PLASMA_DLL_API virtual uint numInputs() const;

   PLASMA_DLL_API virtual DFOutput* output();
   PLASMA_DLL_API virtual DFInput* input( uint );

   DFImageParams& parameters()            { return _params; }
   const DFImageParams parameters() const { return _params; }

   // Editing.
   PLASMA_DLL_API virtual RCP<DFNodeEditor> edit();

   // Dump.
   PLASMA_DLL_API virtual bool  dumpCustom( TextStream& os, StreamIndent& indent ) const;

protected:

   /*----- methods -----*/

   RCP<Bitmap> process( const DFImageParams&  );

   /*----- data members -----*/

   DFImageOutput     _output;
   DFImageInput      _input;
   RCP<DFNodeEditor> _editor;
   DFImageParams     _params;
};

/*==============================================================================
   CLASS DFTransformImageNode
==============================================================================*/

class DFTransformImageNode:
   public DFNode
{
public:

   /*----- methods -----*/

   PLASMA_DLL_API DFTransformImageNode();

   // Attributes.
   PLASMA_DLL_API virtual const ConstString& name() const;

   // Input/output.
   PLASMA_DLL_API virtual uint numInputs() const;

   PLASMA_DLL_API virtual DFOutput* output();
   PLASMA_DLL_API virtual DFInput* input( uint );

   inline void scale( const Vec2f& v )   { _scale = v; }
   const Vec2f& scale() const            { return _scale; }

   inline void offset( const Vec2f& o )  { _offset = o; }
   const Vec2f& offset() const           { return _offset; }

   // Editing.
   PLASMA_DLL_API virtual RCP<DFNodeEditor> edit();

   // Dump.
   PLASMA_DLL_API virtual bool  dumpCustom( TextStream& os, StreamIndent& indent ) const;

protected:

   /*----- methods -----*/

   RCP<Bitmap> process( const DFImageParams&  );

   /*----- data members -----*/

   DFImageOutput     _output;
   DFImageInput      _input;
   RCP<DFNodeEditor> _editor;
   Vec2f             _scale;
   Vec2f             _offset;
};


/*==============================================================================
   CLASS DFBlendImageNode
==============================================================================*/

class DFBlendImageNode:
   public DFNode
{
public:

   /*----- types and enumerations -----*/

   enum BlendMode {
      BLEND_ADD,
      BLEND_MAX,
      BLEND_MIN,
      BLEND_NORMAL,
      BLEND_SUBTRACT
   };

   /*----- methods -----*/

   PLASMA_DLL_API DFBlendImageNode();

   // Attributes.
   PLASMA_DLL_API virtual const ConstString& name() const;

   // Input/output.
   PLASMA_DLL_API virtual uint numInputs() const;

   PLASMA_DLL_API virtual DFOutput* output();
   PLASMA_DLL_API virtual DFInput* input( uint );

   inline BlendMode mode() const   { return _mode; }
   inline void mode( BlendMode m ) { _mode = m; }

   // Editing.
   PLASMA_DLL_API virtual RCP<DFNodeEditor> edit();

   // Dump.
   PLASMA_DLL_API virtual bool  dumpCustom( TextStream& os, StreamIndent& indent ) const;

protected:

   /*----- methods -----*/

   RCP<Bitmap> process( const DFImageParams&  );

   /*----- data members -----*/

   DFImageOutput     _output;
   DFImageMultiInput _inputs;
   RCP<DFNodeEditor> _editor;
   BlendMode         _mode;
};

/*==============================================================================
   CLASS DFInvertImageNode
==============================================================================*/

class DFInvertImageNode:
   public DFNode
{
public:

   /*----- methods -----*/

   PLASMA_DLL_API DFInvertImageNode();

   // Attributes.
   PLASMA_DLL_API virtual const ConstString& name() const;

   // Input/output.
   PLASMA_DLL_API virtual uint numInputs() const;

   PLASMA_DLL_API virtual DFOutput* output();
   PLASMA_DLL_API virtual DFInput* input( uint );

protected:

   /*----- methods -----*/

   RCP<Bitmap> process( const DFImageParams& p );

   /*----- data members -----*/

   DFImageInput   _input;
   DFImageOutput  _output;
};

/*==============================================================================
   CLASS DFSwizzleNode
==============================================================================*/

class DFSwizzleNode:
   public DFNode
{
public:

   /*----- types -----*/

   enum Swizzle
   {
      ZERO,
      ONE,
      RED,
      GREEN,
      BLUE,
      ALPHA,
      LUMA,
      //GRAYSCALE = LUMA,
      CHROMA_U,
      CHROMA_V,
      // Y Co Cg
   };

   /*----- methods -----*/

   PLASMA_DLL_API DFSwizzleNode();

   // Attributes.
   PLASMA_DLL_API virtual const ConstString& name() const;

   // Input/output.
   PLASMA_DLL_API virtual uint numInputs() const;

   PLASMA_DLL_API virtual DFOutput* output();
   PLASMA_DLL_API virtual DFInput* input( uint );


   inline Swizzle  swizzle( uint idx ) const { return _swizzle[idx]; }

   inline void  swizzle( uint idx, Swizzle v )  { _swizzle[idx] = v; }

   inline void  swizzle( Swizzle v0, Swizzle v1, Swizzle v2, Swizzle v3 )
   { _swizzle[0] = v0; _swizzle[1] = v1; _swizzle[2] = v2; _swizzle[3] = v3; }

   // Editing.
   PLASMA_DLL_API virtual RCP<DFNodeEditor> edit();

   // Dump.
   PLASMA_DLL_API virtual bool  dumpCustom( TextStream& os, StreamIndent& indent ) const;

   static PLASMA_DLL_API void  getTransform( Swizzle s, Vec4f& scale, float& offset );

protected:

   /*----- methods -----*/

   RCP<Bitmap> process( const DFImageParams& );

   /*----- data members -----*/

   DFImageInput      _input;
   DFImageOutput     _output;
   RCP<DFNodeEditor> _editor;
   Swizzle           _swizzle[4];
};

//------------------------------------------------------------------------------
//!
PLASMA_DLL_API const char*  toStr( DFSwizzleNode::Swizzle v );

//------------------------------------------------------------------------------
//!
PLASMA_DLL_API DFSwizzleNode::Swizzle  toSwizzle( const char* str );


NAMESPACE_END

#endif
