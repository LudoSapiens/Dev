/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef FUSION_EVENT_PROFILE_VIEWER_H
#define FUSION_EVENT_PROFILE_VIEWER_H

#include <Fusion/StdDefs.h>

#include <Fusion/Core/EventProfiler.h>
#include <Fusion/Widget/Widget.h>

#include <CGMath/AARect.h>

#include <Base/ADT/Set.h>
#include <Base/ADT/Pair.h>

NAMESPACE_BEGIN

class Image;
class Text;
class TQuad;

/*==============================================================================
   CLASS EventProfileViewer
==============================================================================*/

class EventProfileViewer:
   public Widget
{
public:

   /*----- types -----*/

   enum Type
   {
      TYPE_CORE,
      TYPE_LOOP,
      TYPE_RENDER,
      TYPE_DISPLAY,
      TYPE_WORLD,
      TYPE_ANIMATORS,
      TYPE_BRAINS,
      TYPE_COMMANDS,
      TYPE_ACTIONS,
      TYPE_PHYSICS,
      //PE_ACTIONS,
      NUM_TYPES,
   };

   /*----- static methods -----*/

   static void initialize();

   /*----- methods -----*/

   FUSION_DLL_API EventProfileViewer();

   // VM.
   const char* meta() const;
   FUSION_DLL_API void init( VMState* );
   FUSION_DLL_API bool performGet( VMState* );
   FUSION_DLL_API bool performSet( VMState* );

   FUSION_DLL_API bool load( const char* filename );

protected:

   /*----- methods -----*/

   virtual ~EventProfileViewer();

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

   void updateGeometry( bool recenterAfter = false );
   void updateCanvas();

   void  makeBox( Type type, const Vec2f& yRange, double from, double to );
   void  recomputeBoundingBox();
   void  recenter();

   /*----- data members -----*/

   EventProfiler          _loadedProfiler;
   EventProfiler*         _profiler;

   // Rendering data.
   RCP<Image>             _img;
   RCP<Gfx::SamplerList>  _sampler;
   RCP<Gfx::Geometry>     _geometry[NUM_TYPES];
   RCP<Gfx::ConstantList> _constants[NUM_TYPES];
   Vec4f                  _colors[NUM_TYPES];
   Vec2f                  _yRange[NUM_TYPES];
   Mat4f                  _mat;
   Vec2f                  _center;
   Vec2f                  _zoom;
   AARectf                _dataRange;
   // Legend.
   RCP<Text>              _legendTexts[NUM_TYPES];
   RCP<TQuad>             _legendQuads[NUM_TYPES];
   bool                   _legendVisible;
};

NAMESPACE_END

#endif
