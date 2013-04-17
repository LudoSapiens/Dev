/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_DFVIEWER_H
#define PLASMA_DFVIEWER_H

#include <Plasma/StdDefs.h>

#include <Plasma/DataFlow/DFGraph.h>

#include <Fusion/Widget/Widget.h>

#include <CGMath/AARect.h>

#include <Base/ADT/Set.h>
#include <Base/ADT/Pair.h>

NAMESPACE_BEGIN

class Image;
class Text;

/*==============================================================================
   CLASS DFViewer
==============================================================================*/

class DFViewer:
   public Widget
{
public:

   /*----- static methods -----*/

   static void initialize();

   /*----- methods -----*/

   PLASMA_DLL_API DFViewer();

   // VM.
   const char* meta() const;
   PLASMA_DLL_API void init( VMState* );
   PLASMA_DLL_API bool performGet( VMState* );
   PLASMA_DLL_API bool performSet( VMState* );

   PLASMA_DLL_API void graph( DFGraph* );
   inline DFGraph* graph() const { return _graph.ptr(); }

   PLASMA_DLL_API AARectf getVisibleArea() const;

   PLASMA_DLL_API Vec2f  screenToGraph( const Vec2f& p ) const;

protected:

   /*----- methods -----*/

   virtual ~DFViewer();

   virtual void onPointerPress( const Event& );
   virtual void onPointerRelease( const Event& );
   virtual void onPointerMove( const Event& );
   virtual void onPointerScroll( const Event& );
   virtual void onChar( const Event& );
   virtual void onPointerEnter( const Event& );
   virtual void onPointerLeave( const Event& );

   virtual void render( const RCP<Gfx::RenderNode>& );
   virtual void performSetGeometry();
   virtual void performSetPosition();
   virtual bool isAttribute( const char* ) const;

   void graphUpdated( DFGraph* );
   void updateGraphGeometry( bool recenter = false );
   void resizeLeftSelected( float );
   void resizeRightSelected( float );
   void updateCanvas();
   void deleteSelected();

   /*----- types and enumerations -----*/

   enum {
      MOVE_NOTHING,
      MOVE_LEFT,
      MOVE_RIGHT,
      MOVE_NODE,
   };

   typedef Pair<RCP<Text>, DFNode*> NodeText;

   /*----- data members -----*/

   RCP< DFGraph >         _graph;
   Set< DFNode* >         _selectedNodes;
   // Rendering data.
   RCP<Gfx::Geometry>     _geom;
   RCP<Gfx::Geometry>     _selectedGeom;
   RCP<Gfx::Geometry>     _decoGeom;
   RCP<Gfx::Geometry>     _selectedDecoGeom;
   RCP<Gfx::SamplerList>  _samplers;
   RCP<Gfx::SamplerList>  _decoSamplers;
   RCP<Gfx::ConstantList> _cl;
   RCP<Gfx::ConstantList> _selectedCl;
   RCP<Gfx::ConstantList> _decoCl;
   RCP<Image>             _img;
   RCP<Image>             _decoImg;
   Vector< NodeText >     _texts;
   Vector< NodeText >     _selectedTexts;
   Mat4f                  _mat;
   Mat4f                  _smat;
   Vec2f                  _canvasPos;
   Vec2f                  _selectedPos;
   int                    _moveMode;
   VMRef                  _onSelection;
};

NAMESPACE_END

#endif
