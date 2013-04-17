/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_DFSTOKES_TOOL_H
#define PLASMA_DFSTOKES_TOOL_H

#include <Plasma/StdDefs.h>

#include <Plasma/DataFlow/DFBlocks.h>
#include <Plasma/DataFlow/DFStrokes.h>
#include <Plasma/DataFlow/DFStrokesNodes.h>
#include <Plasma/Manipulator/Manipulator.h>

NAMESPACE_BEGIN

// Forward declarations.
class DFStrokesEditor;
class DFStrokesRenderable;


/*==============================================================================
   Static INITIALIZATION
==============================================================================*/

PLASMA_DLL_API void initializeStrokes();
PLASMA_DLL_API void terminateStrokes();


/*==============================================================================
   CLASS DFStrokesNode
==============================================================================*/

class DFStrokesNode:
   public DFNode
{
public:

   /*----- methods -----*/

   PLASMA_DLL_API DFStrokesNode();

   // Attributes.
   PLASMA_DLL_API virtual const ConstString& name() const;

   // Input/output.
   PLASMA_DLL_API virtual uint numInputs() const;

   PLASMA_DLL_API virtual DFOutput* output();
   PLASMA_DLL_API virtual DFInput* input( uint );

   // Editor.
   PLASMA_DLL_API virtual RCP<DFNodeEditor>  edit();

   // Dump.
   PLASMA_DLL_API virtual bool  dumpCustom( TextStream& os, StreamIndent& indent ) const;

   // Strokes.
   DFStrokes*  strokes() { return _strokes.ptr(); }

protected:

   /*----- methods -----*/

   RCP<DFStrokes> process();

   /*----- data members -----*/

   DFStrokesOutput       _output;
   RCP<DFStrokes>        _strokes;
   RCP<DFStrokesEditor>  _editor;
};


/*==============================================================================
  CLASS DFStrokesEditor
==============================================================================*/
class DFStrokesEditor:
   public DFNodeEditor
{
public:

   /*----- enumerations -----*/

   enum Mode
   {
      MODE_STROKES,
      MODE_RINGS,
      MODE_EDGES,
   };

   enum PickType
   {
      PICK_INVALID,
      // Elements:
      //PICK_STROKE,
      //PICK_STROKE_RING,
      //PICK_STROKE_SEGMENT,
      // Vertices:
      PICK_STROKE_VERTEX,
      PICK_STROKE_RING_VERTEX,
      //PICK_STROKE_SEGMENT_VERTEX,
      // Edges:
      //PICK_STROKE_EDGE,
      PICK_STROKE_RING_EDGE,
      PICK_STROKE_SEGMENT_EDGE,
      // Faces:
      PICK_STROKE_FACE,
      PICK_STROKE_RING_FACE
   };

   /*----- types -----*/

   /*==============================================================================
     CLASS Pick
   ==============================================================================*/
   class Pick
   {
   public:

      /*----- methods -----*/

      Pick() { setInvalid(); }

      inline Pick&  setInvalid();
      inline Pick&  setStrokeVertex( uint32_t si, uint32_t vi );
      inline Pick&  setStrokeRingVertex( uint32_t si, uint32_t ri, uint16_t vi );
      inline Pick&  setStrokeRingEdge( uint32_t si, uint32_t ri, uint16_t ei );
      inline Pick&  setStrokeSegmentEdge( uint32_t si, uint32_t gi, uint16_t ei );
      inline Pick&  setStrokeFace( uint32_t si, uint32_t gi, uint16_t ei );
      inline Pick&  setStrokeRingFace( uint32_t si, uint32_t ri );

      PickType  type() const         { return (PickType)_type; }
      uint32_t  strokeIdx() const    { return _idxStk; }
      uint32_t  primaryIdx() const   { return _idxPri; }
      uint16_t  secondaryIdx() const { return _idxSec; }

      uint32_t  geomIdx() const      { return _idxGeom; }
      void  geomIdx( uint32_t idx )  { _idxGeom = idx; }

      bool isValid() const           { return _type != PICK_INVALID; }

      inline bool  operator==( const Pick& p ) const;
      inline bool  operator< ( const Pick& p ) const;

      PLASMA_DLL_API void  print( TextStream& os = StdErr ) const;

   protected:

      /*----- data members -----*/

      uint32_t _idxGeom;  //!< An index used for quick rendering updates.
      uint32_t _idxStk;   //!< The index of the stroke containing this element.
      uint32_t _idxPri;   //!< A primary offset (a stroke's vertex, edge, ring, or segment, range [0,n), depending on the size).
      uint16_t _idxSec;   //!< A secondary offset (ring vertex [0-3], ring edge [0-3], segment face [0-3]).
      uint16_t _type;     //!< One of the PickTypes.
   };

   typedef DFSelection<Pick> Selection;


   /*----- methods -----*/

   PLASMA_DLL_API DFStrokesEditor( DFStrokesNode* n );
   PLASMA_DLL_API virtual ~DFStrokesEditor();

   PLASMA_DLL_API virtual RCP<Manipulator>  manipulator();
   PLASMA_DLL_API virtual RCP<DFNodeAttrList>  attributes() const;
   PLASMA_DLL_API virtual RCP<DFNodeAttrStates>  attributesStates() const;
   PLASMA_DLL_API virtual void updateAttributes( const DFNodeAttrStates& );

   inline      DFStrokesNode*  node() const { return _node; }
   inline       DFStrokes*  strokes()       { return _node->strokes(); }
   inline const DFStrokes*  strokes() const { return _node->strokes(); }

   inline Mode  mode() const { return _mode; }
   PLASMA_DLL_API void  mode( Mode m );

   // Selection.
   inline const Selection&  selection() const   { return _selection; }
   inline void clearSelection()                 { _selection.clear(); updateSelection(); }
   inline void addSelection( const Pick& p )    { _selection.add( p ); updateSelection(); }
   inline void setSelection( const Pick& p )    { _selection.set( p ); updateSelection(); }
   inline void removeSelection( const Pick& p ) { _selection.remove( p ); updateSelection(); }
   inline bool toggleSelection( const Pick& p ) { bool b = _selection.toggle( p ); updateSelection(); return b; }

   // Editing API.
   void changeFace( const Pick& );
   void changeCrease( const Pick& );
   void moveVertex( const Vec3f& p );
   void moveCorner( const Pick&, const Vec3f& p );
   void createVertex( const Vec3f& p );
   void moveRing( const Reff& r );
   void createStroke( const Vec3f& p );
   void scaleRing( float sf );
   void removeVertex();

protected:

   /*----- methods -----*/

   void updateUI();
   void updateSelection();
   void updateAll();

   /*----- data members -----*/

   DFStrokesNode*            _node;
   RCP<DFStrokesRenderable>  _renderable;
   Mode                      _mode;
   DFSelection<Pick>         _selection;

private:
}; //class DFStrokesEditor

//------------------------------------------------------------------------------
//!
DFStrokesEditor::Pick&
DFStrokesEditor::Pick::setInvalid()
{
   _type   = PICK_INVALID;
   _idxStk = 0;
   _idxPri = 0;
   _idxSec = 0;
   return *this;
}

//------------------------------------------------------------------------------
//!
DFStrokesEditor::Pick&
DFStrokesEditor::Pick::setStrokeVertex( uint32_t si, uint32_t vi )
{
   _type   = PICK_STROKE_VERTEX;
   _idxStk = si;
   _idxPri = vi;
   _idxSec =  0;
   return *this;
}

//------------------------------------------------------------------------------
//!
DFStrokesEditor::Pick&
DFStrokesEditor::Pick::setStrokeRingVertex( uint32_t si, uint32_t ri, uint16_t vi )
{
   _type   = PICK_STROKE_RING_VERTEX;
   _idxStk = si;
   _idxPri = ri;
   _idxSec = vi;
   return *this;
}

//------------------------------------------------------------------------------
//!
DFStrokesEditor::Pick&
DFStrokesEditor::Pick::setStrokeRingEdge( uint32_t si, uint32_t ri, uint16_t ei )
{
   _type   = PICK_STROKE_RING_EDGE;
   _idxStk = si;
   _idxPri = ri;
   _idxSec = ei;
   return *this;
}

//------------------------------------------------------------------------------
//!
DFStrokesEditor::Pick&
DFStrokesEditor::Pick::setStrokeSegmentEdge( uint32_t si, uint32_t gi, uint16_t ei )
{
   _type   = PICK_STROKE_SEGMENT_EDGE;
   _idxStk = si;
   _idxPri = gi;
   _idxSec = ei;
   return *this;
}

//------------------------------------------------------------------------------
//!
DFStrokesEditor::Pick&
DFStrokesEditor::Pick::setStrokeFace( uint32_t si, uint32_t gi, uint16_t ei )
{
   _type   = PICK_STROKE_FACE;
   _idxStk = si;
   _idxPri = gi;
   _idxSec = ei;
   return *this;
}

//------------------------------------------------------------------------------
//!
DFStrokesEditor::Pick&
DFStrokesEditor::Pick::setStrokeRingFace( uint32_t si, uint32_t ri )
{
   _type   = PICK_STROKE_RING_FACE;
   _idxStk = si;
   _idxPri = ri;
   _idxSec = 0;
   return *this;
}

//------------------------------------------------------------------------------
//!
inline bool
DFStrokesEditor::Pick::operator==( const Pick& p ) const
{
   if( _type == p._type )
   {
      if( _type != PICK_INVALID )
      {
         // Same type, check the rest (except for _geomOffset).
         return _idxStk == p._idxStk &&
                _idxPri == p._idxPri &&
                _idxSec == p._idxSec;
      }
      else
      {
         // Both are invalid.
         return true;
      }
   }
   else
   {
      // Different types.
      return false;
   }
}

//------------------------------------------------------------------------------
//!
inline bool
DFStrokesEditor::Pick::operator<( const Pick& p ) const
{
   return _type < p._type ||
          ((_type   == p._type  ) && _idxStk < p._idxStk) ||
          ((_idxStk == p._idxStk) && _idxPri < p._idxPri) ||
          ((_idxPri == p._idxPri) && _idxSec < p._idxSec);
}


NAMESPACE_END

#endif //PLASMA_DFSTOKES_TOOL_H
