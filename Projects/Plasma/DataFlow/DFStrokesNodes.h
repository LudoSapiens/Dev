/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_DFSTROKES_NODES_H
#define PLASMA_DFSTROKES_NODES_H

#include <Plasma/StdDefs.h>
#include <Plasma/DataFlow/DFNode.h>
#include <Plasma/DataFlow/DFBlocks.h>
#include <Plasma/DataFlow/DFGeomNodes.h>

#include <CGMath/Random.h>

#include <Base/Msg/Delegate.h>

NAMESPACE_BEGIN

class DFStrokes;

/*==============================================================================
   STATIC INITIALIZATION
==============================================================================*/

void initializeStrokesNodes();
void terminateStrokesNodes();

/*==============================================================================
   CLASS DFStrokesOutput
==============================================================================*/

class DFStrokesOutput:
   public DFOutput
{
public:

   /*----- types -----*/

   typedef Delegate0< RCP<DFStrokes> > ProcessDelegate;

   /*----- methods -----*/

   PLASMA_DLL_API RCP<DFStrokes> getStrokes();
   PLASMA_DLL_API virtual Type type() const;

   void delegate( const ProcessDelegate& d ) { _delegate = d; }

protected:

   /*----- data members -----*/

   ProcessDelegate  _delegate;
};

/*==============================================================================
   CLASS DFStrokesInput
==============================================================================*/

class DFStrokesInput:
   public DFInput
{
public:

   /*----- methods -----*/

   DFStrokesInput( DFNode* n ): DFInput(n), _output(nullptr) {}

   PLASMA_DLL_API RCP<DFStrokes> getStrokes();
   PLASMA_DLL_API virtual Type type() const;
   PLASMA_DLL_API virtual bool isConnected() const;

protected:

   /*----- methods -----*/

   virtual void connect( DFOutput* );
   virtual void disconnect( DFOutput* );
   virtual void disconnect();

   /*----- data members -----*/

   DFStrokesOutput* _output;
};

/*==============================================================================
   CLASS DFStrokesMultiInput
==============================================================================*/

class DFStrokesMultiInput:
   public DFInput
{
public:

   /*----- methods -----*/

   DFStrokesMultiInput( DFNode* n ): DFInput(n) {}
   PLASMA_DLL_API RCP<DFStrokes> getStrokes( uint );

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

   Vector<DFStrokesOutput*> _outputs;
};


/*==============================================================================
   CLASS DFStrokesToGeometryNode
==============================================================================*/

class DFStrokesToGeometryNode:
   public DFNode
{
public:

   /*----- methods -----*/

   PLASMA_DLL_API DFStrokesToGeometryNode();

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

   DFStrokesInput _input;
   DFGeomOutput   _output;
};

/*==============================================================================
   CLASS DFStrokesDebugNode
==============================================================================*/

class DFStrokesDebugNode:
   public DFNode
{
public:

   /*----- methods -----*/

   PLASMA_DLL_API DFStrokesDebugNode();

   // Attributes.
   PLASMA_DLL_API virtual const ConstString& name() const;

   // Editor.
   PLASMA_DLL_API virtual RCP<DFNodeEditor>  edit();

   // Input/output.
   PLASMA_DLL_API virtual uint numInputs() const;

   PLASMA_DLL_API virtual DFOutput* output();
   PLASMA_DLL_API virtual DFInput* input( uint );

protected:

   /*----- methods -----*/

   RCP<DFStrokes> process();

   /*----- data members -----*/

   DFStrokesInput    _input;
   DFStrokesOutput   _output;
   RCP<DFNodeEditor> _editor;
};


NAMESPACE_END

#endif
