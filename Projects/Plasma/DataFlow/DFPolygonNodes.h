/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_DFPOLYGON_NODES_H
#define PLASMA_DFPOLYGON_NODES_H

#include <Plasma/StdDefs.h>
#include <Plasma/DataFlow/DFNode.h>

#include <CGMath/Polygon.h>

#include <Base/Msg/Delegate.h>

NAMESPACE_BEGIN

typedef Polygonf DFPolygon;
class DFPolygonRenderable;

/*==============================================================================
   STATIC INITIALIZATION
==============================================================================*/

void initializePolygonNodes();
void terminatePolygonNodes();

/*==============================================================================
   CLASS DFPolygonOutput
==============================================================================*/

class DFPolygonOutput:
   public DFOutput
{
public:

   /*----- types -----*/

   typedef Delegate0< RCP<DFPolygon> > ProcessDelegate;

   /*----- methods -----*/

   PLASMA_DLL_API RCP<DFPolygon> getPolygon();
   PLASMA_DLL_API virtual Type type() const;

   void delegate( const ProcessDelegate& d ) { _delegate = d; }

protected:

   /*----- data members -----*/

   ProcessDelegate  _delegate;
};

/*==============================================================================
   CLASS DFPolygonInput
==============================================================================*/

class DFPolygonInput:
   public DFInput
{
public:

   /*----- methods -----*/

   DFPolygonInput( DFNode* n ): DFInput(n), _output(nullptr) {}

   PLASMA_DLL_API RCP<DFPolygon> getPolygon();
   PLASMA_DLL_API virtual Type type() const;
   PLASMA_DLL_API virtual bool isConnected() const;

protected:

   /*----- methods -----*/

   virtual void connect( DFOutput* );
   virtual void disconnect( DFOutput* );
   virtual void disconnect();

   /*----- data members -----*/

   DFPolygonOutput* _output;
};

/*==============================================================================
   CLASS DFPolygonMultiInput
==============================================================================*/

class DFPolygonMultiInput:
   public DFInput
{
public:

   /*----- methods -----*/

   DFPolygonMultiInput( DFNode* n ): DFInput(n) {}
   PLASMA_DLL_API RCP<DFPolygon> getPolygon( uint );

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

   Vector<DFPolygonOutput*> _outputs;
};

/*==============================================================================
   CLASS DFPolygonNode
==============================================================================*/

class DFPolygonNode:
   public DFNode
{
public:

   /*----- methods -----*/

   PLASMA_DLL_API DFPolygonNode();

   // Attributes.
   PLASMA_DLL_API virtual const ConstString& name() const;

   // Editor.
   PLASMA_DLL_API virtual RCP<DFNodeEditor>  edit();

   // Input/output.
   PLASMA_DLL_API virtual uint numInputs() const;

   PLASMA_DLL_API virtual DFOutput* output();
   PLASMA_DLL_API virtual DFInput* input( uint );

   // Dump.
   PLASMA_DLL_API virtual bool  dumpCustom( TextStream& os, StreamIndent& indent ) const;

   DFPolygon*  polygon() { return _polygon.ptr(); }

protected:

   /*----- methods -----*/

   RCP<DFPolygon> process();

   /*----- data members -----*/

   DFPolygonOutput   _output;
   RCP<DFPolygon>    _polygon;
   RCP<DFNodeEditor> _editor;
};

/*==============================================================================
   CLASS DFPolygonEditor
==============================================================================*/

class DFPolygonEditor:
   public DFNodeEditor
{
public:

   /*----- structures -----*/

   struct Pick
   {
      Pick(): _idx(-1)     {}
      void invalidate()    { _idx = -1; }
      bool isValid() const { return _idx >= 0; }

      inline bool operator==( const Pick& p ) const { return _idx == p._idx; }
      inline bool operator< ( const Pick& p ) const { return _idx  < p._idx; }

      int _idx;
   };

   /*----- types -----*/

   typedef DFSelection<Pick> Selection;

   /*----- methods -----*/

   DFPolygonEditor( DFPolygonNode* n );

   PLASMA_DLL_API virtual RCP<Manipulator> manipulator();

   DFPolygonRenderable* renderable() { return _renderable.ptr(); }
   DFPolygon* polygon()              { return _node->polygon(); }
   const DFPolygon* polygon()  const { return _node->polygon(); }

   PLASMA_DLL_API virtual RCP<DFNodeAttrList> attributes() const;
   PLASMA_DLL_API virtual RCP<DFNodeAttrStates> attributesStates() const;
   PLASMA_DLL_API virtual void updateAttributes( const DFNodeAttrStates& );

   // Selection.
   inline const Selection&  selection() const { return _selection; }
   inline void clearSelection()                 { _selection.clear(); updateSelection(); }
   inline void addSelection( const Pick& p )    { _selection.add( p ); updateSelection(); }
   inline void setSelection( const Pick& p )    { _selection.set( p ); updateSelection(); }
   inline void removeSelection( const Pick& p ) { _selection.remove( p ); updateSelection(); }
   inline bool toggleSelection( const Pick& p ) { bool b = _selection.toggle( p ); updateSelection(); return b; }


   // Editing API.
   void moveVertex( const Vec3f& p );
   void createVertex( const Vec3f& p );
   void removeVertex();

protected:

   void updateUI();
   void updateSelection();
   void updateAll();

   /*----- data members -----*/

   DFPolygonNode*            _node;
   RCP<DFPolygonRenderable>  _renderable;
   DFSelection<Pick>         _selection;
};


NAMESPACE_END

#endif
