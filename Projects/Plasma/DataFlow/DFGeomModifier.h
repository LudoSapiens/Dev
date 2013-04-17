/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_DFGEOMMODIFIER_H
#define PLASMA_DFGEOMMODIFIER_H

#include <Plasma/StdDefs.h>
#include <Plasma/DataFlow/DFGeomNodes.h>

NAMESPACE_BEGIN

class DFGeometry;

/*==============================================================================
   STATIC INITIALIZATION
==============================================================================*/

void initializeGeomModifier();
void terminateGeomModifier();

/*==============================================================================
   CLASS DFMappingNode
==============================================================================*/

class DFMappingNode:
   public DFNode
{
public:

   /*----- methods -----*/

   PLASMA_DLL_API DFMappingNode();

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

   float scale() const          { return _scale; }
   void scale( float v )        { _scale = v; }
   const Vec3f& offset() const  { return _offset; }
   void offset( const Vec3f& v) { _offset = v; }

protected:

   /*----- methods -----*/

   RCP<DFGeometry> process();

   /*----- data members -----*/

   DFGeomOutput      _output;
   DFGeomInput       _input;
   RCP<DFNodeEditor> _editor;
   float             _scale;
   Vec3f             _offset;
};

/*==============================================================================
   CLASS DFMaterialIDNode
==============================================================================*/

class DFMaterialIDNode:
   public DFNode
{
public:

   /*----- methods -----*/

   PLASMA_DLL_API DFMaterialIDNode();

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

   uint32_t materialID() const       { return _matID; }
   void materialID( uint32_t v )     { _matID = v;    }

protected:

   /*----- methods -----*/

   RCP<DFGeometry> process();

   /*----- data members -----*/

   DFGeomOutput      _output;
   DFGeomInput       _input;
   RCP<DFNodeEditor> _editor;
   uint32_t          _matID;
};

NAMESPACE_END

#endif
