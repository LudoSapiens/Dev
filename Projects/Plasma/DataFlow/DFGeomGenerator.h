/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_DFGEOMGENERATOR_H
#define PLASMA_DFGEOMGENERATOR_H

#include <Plasma/StdDefs.h>
#include <Plasma/DataFlow/DFGeomNodes.h>
#include <Plasma/DataFlow/DFBlocks.h>
#include <Plasma/DataFlow/DFPolygonNodes.h>

NAMESPACE_BEGIN

class DFGeometry;

/*==============================================================================
   STATIC INITIALIZATION
==============================================================================*/

void initializeGeomGenerator();
void terminateGeomGenerator();

/*==============================================================================
   CLASS DFBlocksNode
==============================================================================*/

class DFBlocksNode:
   public DFNode
{
public:

   /*----- methods -----*/

   PLASMA_DLL_API DFBlocksNode();

   // Attributes.
   PLASMA_DLL_API virtual const ConstString& name() const;

   // Input/output.
   PLASMA_DLL_API virtual uint numInputs() const;

   PLASMA_DLL_API virtual DFOutput* output();
   PLASMA_DLL_API virtual DFInput* input( uint );

   // Dump.
   PLASMA_DLL_API virtual bool  dumpCustom( TextStream& os, StreamIndent& indent ) const;

   // Blocks.
   DFBlocks* blocks() { return _blocks.ptr(); }

protected:

   /*----- methods -----*/

   RCP<DFGeometry> process();

   /*----- data members -----*/

   DFGeomOutput      _output;
   RCP<DFBlocks>     _blocks;
};

/*==============================================================================
   CLASS DFBoxNode
==============================================================================*/

class DFBoxNode:
   public DFNode
{
public:

   /*----- methods -----*/

   PLASMA_DLL_API DFBoxNode();

   // Attributes.
   PLASMA_DLL_API virtual const ConstString&  name() const;

   // Editor.
   PLASMA_DLL_API RCP<DFNodeEditor>  edit();

   // Input/output.
   PLASMA_DLL_API virtual DFOutput*  output();

   // Dump.
   PLASMA_DLL_API virtual bool  dumpCustom( TextStream& os, StreamIndent& indent ) const;
   PLASMA_DLL_API         void  init( VMState* vm, int idx );

   inline const Vec3f&  halfSize() const   { return _halfSize; }
   inline void  halfSize( const Vec3f& v ) { _halfSize = v;    }
   const Reff& referential() const         { return _referential; }
   void referential( const Reff& r )       { _referential = r; }
   void position( const Vec3f& p )         { _referential.position(p); }
   void orientation( const Quatf& q )      { _referential.orientation(q); }


protected:

   /*----- methods -----*/

   RCP<DFGeometry>  process();

   /*----- data members -----*/

   RCP<DFNodeEditor>  _editor;
   DFGeomOutput       _output;
   Vec3f              _halfSize;
   Reff               _referential;
};

/*==============================================================================
   CLASS DFBallNode
==============================================================================*/

class DFBallNode:
   public DFNode
{
public:

   /*----- methods -----*/

   PLASMA_DLL_API DFBallNode();

   // Attributes.
   PLASMA_DLL_API virtual const ConstString&  name() const;

   // Editor.
   PLASMA_DLL_API RCP<DFNodeEditor>  edit();

   // Input/output.
   PLASMA_DLL_API virtual DFOutput*  output();

   // Dump.
   PLASMA_DLL_API virtual bool  dumpCustom( TextStream& os, StreamIndent& indent ) const;
   PLASMA_DLL_API         void  init( VMState* vm, int idx );

   inline float  radius() const       { return _radius; }
   inline  void  radius( float v )    { _radius = v;    }

   const Reff& referential() const    { return _referential; }
   void referential( const Reff& r )  { _referential = r; }
   void position( const Vec3f& p )    { _referential.position(p); }
   void orientation( const Quatf& q ) { _referential.orientation(q); }


protected:

   /*----- methods -----*/

   RCP<DFGeometry>  process();

   /*----- data members -----*/

   RCP<DFNodeEditor>  _editor;
   DFGeomOutput       _output;
   float              _radius;
   Reff               _referential;
};

/*==============================================================================
   CLASS DFExtrudePolyNode
==============================================================================*/

class DFExtrudePolyNode:
   public DFNode
{
public:

   /*----- methods -----*/

   PLASMA_DLL_API DFExtrudePolyNode();

   // Attributes.
   PLASMA_DLL_API virtual const ConstString& name() const;

   // Input/output.
   PLASMA_DLL_API virtual uint numInputs() const;

   PLASMA_DLL_API virtual DFOutput* output();
   PLASMA_DLL_API virtual DFInput* input( uint );

   // Editing.
   PLASMA_DLL_API virtual RCP<DFNodeEditor> edit();

   // Dump.
   PLASMA_DLL_API virtual bool  dumpCustom( TextStream& os, StreamIndent& indent ) const;

   // Parameters.
   void height( float h ) { _height = h; }
   float height() const   { return _height; }

protected:

   /*----- methods -----*/

   RCP<DFGeometry> process();

   /*----- data members -----*/

   DFGeomOutput      _output;
   DFPolygonInput    _input;
   RCP<DFNodeEditor> _editor;
   float             _height;
};

NAMESPACE_END

#endif
