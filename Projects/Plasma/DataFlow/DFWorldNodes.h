/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_DFWORLDNODE_H
#define PLASMA_DFWORLDNODE_H

#include <Plasma/StdDefs.h>
#include <Plasma/DataFlow/DFNode.h>
#include <Plasma/DataFlow/DFWorld.h>
#include <Plasma/DataFlow/DFGeomNodes.h>
#include <Plasma/World/Probe.h>

#include <Fusion/VM/VMFmt.h>

#include <Base/Msg/Delegate.h>

NAMESPACE_BEGIN

/*==============================================================================
   STATIC INITIALIZATION
==============================================================================*/

PLASMA_DLL_API void initializeWorldNodes();
PLASMA_DLL_API void terminateWorldNodes();

/*==============================================================================
   CLASS DFWorldOutput
==============================================================================*/

class DFWorldOutput:
   public DFOutput
{
public:

   /*----- types -----*/

   typedef Delegate0< RCP<DFWorld> > ProcessDelegate;

   /*----- methods -----*/

   PLASMA_DLL_API RCP<DFWorld> getWorld();
   PLASMA_DLL_API virtual Type type() const;

   void delegate( const ProcessDelegate& d ) { _delegate = d; }

protected:

   /*----- data members -----*/

   ProcessDelegate _delegate;
};

/*==============================================================================
   CLASS DFWorldInput
==============================================================================*/

class DFWorldInput:
   public DFInput
{
public:

   /*----- methods -----*/

   DFWorldInput( DFNode* n ): DFInput(n), _output(nullptr) {}

   PLASMA_DLL_API RCP<DFWorld> getWorld();
   PLASMA_DLL_API virtual Type type() const;
   PLASMA_DLL_API virtual bool isConnected() const;

protected:

   /*----- methods -----*/

   virtual void connect( DFOutput* );
   virtual void disconnect( DFOutput* );
   virtual void disconnect();

   /*----- data members -----*/

   DFWorldOutput* _output;
};

/*==============================================================================
   CLASS DFWorldMultiInput
==============================================================================*/

class DFWorldMultiInput:
   public DFInput
{
public:

   /*----- methods -----*/

   DFWorldMultiInput( DFNode* n ): DFInput(n) {}
   PLASMA_DLL_API RCP<DFWorld> getWorld( uint );

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

   Vector<DFWorldOutput*> _outputs;
};

/*==============================================================================
   CLASS DFMergeWorldNode
==============================================================================*/

class DFMergeWorldNode:
   public DFNode
{
public:

   /*----- methods -----*/

   PLASMA_DLL_API DFMergeWorldNode();

   // Attributes.
   PLASMA_DLL_API virtual const ConstString& name() const;

   // Input/output.
   PLASMA_DLL_API virtual uint numInputs() const;

   PLASMA_DLL_API virtual DFOutput* output();
   PLASMA_DLL_API virtual DFInput* input( uint );

protected:

   /*----- methods -----*/

   RCP<DFWorld> process();

   /*----- data members -----*/

   DFWorldOutput      _output;
   DFWorldMultiInput  _worlds;
};

/*==============================================================================
   CLASS DFEntityNode
==============================================================================*/

class DFEntityNode:
   public DFNode
{
public:

   /*----- methods -----*/

   PLASMA_DLL_API DFEntityNode();

   // Attributes.
   PLASMA_DLL_API virtual const ConstString& name() const;

   // Input/output.
   PLASMA_DLL_API virtual uint numInputs() const;

   PLASMA_DLL_API virtual DFOutput* output();
   PLASMA_DLL_API virtual DFInput* input( uint );

   DFEntity* entity()             { return _world->entities()[0].ptr(); }
   const DFEntity* entity() const { return _world->entities()[0].ptr(); }

   // Editing.
   PLASMA_DLL_API virtual RCP<DFNodeEditor> edit();

   // Dump.
   PLASMA_DLL_API virtual bool  dumpCustom( TextStream& os, StreamIndent& indent ) const;


protected:

   /*----- methods -----*/

   RCP<DFWorld> process();

   /*----- data members -----*/

   DFWorldOutput     _output;
   DFGeomInput       _geometry;
   RCP<DFNodeEditor> _editor;
   RCP<DFWorld>      _world;
};

/*==============================================================================
   CLASS DFCameraNode
==============================================================================*/

class DFCameraNode:
   public DFNode
{
public:

   /*----- methods -----*/

   PLASMA_DLL_API DFCameraNode();

   // Attributes.
   PLASMA_DLL_API virtual const ConstString& name() const;

   // Input/output.
   PLASMA_DLL_API virtual uint numInputs() const;

   PLASMA_DLL_API virtual DFOutput* output();
   PLASMA_DLL_API virtual DFInput* input( uint );

   DFCamera* camera()             { return (DFCamera*)_world->entities()[0].ptr(); }
   const DFCamera* camera() const { return (const DFCamera*)_world->entities()[0].ptr(); }

   // Editing.
   PLASMA_DLL_API virtual RCP<DFNodeEditor> edit();

   // Dump.
   PLASMA_DLL_API virtual bool  dumpCustom( TextStream& os, StreamIndent& indent ) const;

protected:

   /*----- methods -----*/

   RCP<DFWorld> process();

   /*----- data members -----*/

   DFWorldOutput     _output;
   DFGeomInput       _geometry;
   RCP<DFNodeEditor> _editor;
   RCP<DFWorld>      _world;
};

/*==============================================================================
   CLASS DFLightNode
==============================================================================*/

class DFLightNode:
   public DFNode
{
public:

   /*----- methods -----*/

   PLASMA_DLL_API DFLightNode();

   // Attributes.
   PLASMA_DLL_API virtual const ConstString& name() const;

   // Input/output.
   PLASMA_DLL_API virtual uint numInputs() const;

   PLASMA_DLL_API virtual DFOutput* output();
   PLASMA_DLL_API virtual DFInput* input( uint );

   DFLight* light()             { return (DFLight*)_world->entities()[0].ptr(); }
   const DFLight* light() const { return (const DFLight*)_world->entities()[0].ptr(); }

   // Editing.
   PLASMA_DLL_API virtual RCP<DFNodeEditor> edit();

   // Dump.
   PLASMA_DLL_API virtual bool  dumpCustom( TextStream& os, StreamIndent& indent ) const;

protected:

   /*----- methods -----*/

   RCP<DFWorld> process();

   /*----- data members -----*/

   DFWorldOutput     _output;
   DFGeomInput       _geometry;
   RCP<DFNodeEditor> _editor;
   RCP<DFWorld>      _world;
};

/*==============================================================================
   CLASS DFProbeNode
==============================================================================*/

class DFProbeNode:
   public DFNode
{
public:

   /*----- methods -----*/

   PLASMA_DLL_API DFProbeNode();

   // Attributes.
   PLASMA_DLL_API virtual const ConstString&  name() const;

   // Input/output.
   PLASMA_DLL_API virtual DFOutput* output();

   // Editor.
   PLASMA_DLL_API RCP<DFNodeEditor> edit();

   // Dump.
   PLASMA_DLL_API virtual bool dumpCustom( TextStream& os, StreamIndent& indent ) const;

   PLASMA_DLL_API void  init( VMState* vm, int idx );

         DFProbe&  probe()       { return *_world->probes()[0]; }
   const DFProbe&  probe() const { return *_world->probes()[0]; }

protected:

   /*----- methods -----*/

   RCP<DFWorld>  process();

   /*----- data members -----*/

   RCP<DFNodeEditor>  _editor;
   DFWorldOutput      _output;
   RCP<DFWorld>       _world;
};


NAMESPACE_END

#endif
