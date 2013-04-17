/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_DFGEOMNODE_H
#define PLASMA_DFGEOMNODE_H

#include <Plasma/StdDefs.h>
#include <Plasma/DataFlow/DFNode.h>

#include <Base/Msg/Delegate.h>

NAMESPACE_BEGIN

class DFGeometry;

/*==============================================================================
   STATIC INITIALIZATION
==============================================================================*/

void initializeGeomNodes();
void terminateGeomNodes();

/*==============================================================================
   CLASS DFGeomOutput
==============================================================================*/

class DFGeomOutput:
   public DFOutput
{
public:

   /*----- types -----*/

   typedef Delegate0< RCP<DFGeometry> > ProcessDelegate;

   /*----- methods -----*/

   PLASMA_DLL_API RCP<DFGeometry> getGeometry();
   PLASMA_DLL_API virtual Type type() const;

   void delegate( const ProcessDelegate& d ) { _delegate = d; }

protected:

   /*----- data members -----*/

   ProcessDelegate  _delegate;
};

/*==============================================================================
   CLASS DFGeomInput
==============================================================================*/

class DFGeomInput:
   public DFInput
{
public:

   /*----- methods -----*/

   DFGeomInput( DFNode* n ): DFInput(n), _output(nullptr) {}

   PLASMA_DLL_API RCP<DFGeometry> getGeometry();
   PLASMA_DLL_API virtual Type type() const;
   PLASMA_DLL_API virtual bool isConnected() const;

protected:

   /*----- methods -----*/

   virtual void connect( DFOutput* );
   virtual void disconnect( DFOutput* );
   virtual void disconnect();

   /*----- data members -----*/

   DFGeomOutput* _output;
};

/*==============================================================================
   CLASS DFGeomMultiInput
==============================================================================*/

class DFGeomMultiInput:
   public DFInput
{
public:

   /*----- methods -----*/

   DFGeomMultiInput( DFNode* n ): DFInput(n) {}
   PLASMA_DLL_API RCP<DFGeometry> getGeometry( uint );

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

   Vector<DFGeomOutput*> _outputs;
};

/*==============================================================================
   CLASS DFDifferenceNode
==============================================================================*/

class DFDifferenceNode:
   public DFNode
{
public:

   /*----- methods -----*/

   PLASMA_DLL_API DFDifferenceNode();

   // Attributes.
   PLASMA_DLL_API virtual const ConstString& name() const;

   // Input/output.
   PLASMA_DLL_API virtual uint numInputs() const;

   PLASMA_DLL_API virtual DFOutput* output();
   PLASMA_DLL_API virtual DFInput* input( uint );

protected:

   /*----- methods -----*/

   RCP<DFGeometry> process();

   /*----- data members -----*/

   DFGeomOutput      _output;
   DFGeomInput       _minuend;
   DFGeomMultiInput  _subtrahends;
};

/*==============================================================================
   CLASS DFIntersectionNode
==============================================================================*/

class DFIntersectionNode:
   public DFNode
{
public:

   /*----- methods -----*/

   PLASMA_DLL_API DFIntersectionNode();

   // Attributes.
   PLASMA_DLL_API virtual const ConstString& name() const;

   // Input/output.
   PLASMA_DLL_API virtual uint numInputs() const;

   PLASMA_DLL_API virtual DFOutput* output();
   PLASMA_DLL_API virtual DFInput* input( uint );

protected:

   /*----- methods -----*/

   RCP<DFGeometry> process();

   /*----- data members -----*/

   DFGeomOutput      _output;
   DFGeomMultiInput  _blocks;
};

/*==============================================================================
   CLASS DFUnionNode
==============================================================================*/

class DFUnionNode:
   public DFNode
{
public:

   /*----- methods -----*/

   PLASMA_DLL_API DFUnionNode();

   // Attributes.
   PLASMA_DLL_API virtual const ConstString& name() const;

   // Input/output.
   PLASMA_DLL_API virtual uint numInputs() const;

   PLASMA_DLL_API virtual DFOutput* output();
   PLASMA_DLL_API virtual DFInput* input( uint );

protected:

   /*----- methods -----*/

   RCP<DFGeometry> process();

   /*----- data members -----*/

   DFGeomOutput      _output;
   DFGeomMultiInput  _blocks;
};

/*==============================================================================
   CLASS DFTransformNode
==============================================================================*/

class DFTransformNode:
   public DFNode
{
public:

   /*----- methods -----*/

   PLASMA_DLL_API DFTransformNode();

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

   const Reff& referential() const    { return _referential; }
   void referential( const Reff& r )  { _referential = r; }
   void position( const Vec3f& p )    { _referential.position(p); }
   void orientation( const Quatf& q ) { _referential.orientation(q); }

protected:

   /*----- methods -----*/

   RCP<DFGeometry> process();

   /*----- data members -----*/

   DFGeomOutput      _output;
   DFGeomInput       _input;
   RCP<DFNodeEditor> _editor;
   Reff              _referential;
};

/*==============================================================================
   CLASS DFCloneNode
==============================================================================*/

class DFCloneNode:
   public DFNode
{
public:

   /*----- methods -----*/

   PLASMA_DLL_API DFCloneNode();

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

   void numClones( uint val )       { _num = val; }
   uint numClones() const           { return _num; }

   void offset( const Vec3f& val )  { _offset = val; }
   const Vec3f& offset() const      { return _offset; }

   void rotation( const Reff& r )   { _rotation = r; }
   void rotation( const Quatf& q )  { _rotation.orientation(q); }
   const Quatf& rotation() const    { return _rotation.orientation(); }
   void origin( const Vec3f& o )    { _rotation.position(o); }
   const Vec3f& origin() const      { return _rotation.position(); }

protected:

   /*----- methods -----*/

   RCP<DFGeometry> process();

   /*----- data members -----*/

   DFGeomOutput      _output;
   DFGeomInput       _input;
   RCP<DFNodeEditor> _editor;
   uint              _num;
   Vec3f             _offset;
   Reff              _rotation;
};

/*==============================================================================
   CLASS DFGeomImportNode
==============================================================================*/

class DFGeomImportNode:
   public DFNode
{
public:

   /*----- methods -----*/

   PLASMA_DLL_API DFGeomImportNode();

   // Attributes.
   PLASMA_DLL_API virtual const ConstString& name() const;

   PLASMA_DLL_API virtual bool isGraph() const;

   // Input/output.
   PLASMA_DLL_API virtual DFOutput* output();

   inline const String& geomID() const { return _geomID; }
   PLASMA_DLL_API void geomID( const String& );

   // Editing.
   PLASMA_DLL_API virtual RCP<DFNodeEditor> edit();

   // Dump.
   PLASMA_DLL_API virtual bool  dumpCustom( TextStream& os, StreamIndent& indent ) const;

protected:

   /*----- methods -----*/

   RCP<DFGeometry>  process();

   /*----- data members -----*/

   DFGeomOutput       _output;
   RCP<DFNodeEditor>  _editor;
   String             _geomID;
   RCP<DFGraph>       _geomGraph;
};

NAMESPACE_END

#endif
