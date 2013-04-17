/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef FUSION_LAYER_H
#define FUSION_LAYER_H

#include <Fusion/StdDefs.h>
#include <Fusion/Widget/WidgetContainer.h>

NAMESPACE_BEGIN


/*==============================================================================
  CLASS Layer
==============================================================================*/

//!
//! A layer renders its contents into an offscreen texture using a separate Node,
//! and then blits it back into the current node.

class Layer
   : public WidgetContainer
{

public:
   
   /*----- static methods -----*/

   static void initialize();

   /*----- methods -----*/

   FUSION_DLL_API Layer();

   virtual Vec2f screenToLayer( const Vec2f& ) const;
   virtual Vec2f layerToScreen( const Vec2f& ) const;

   virtual Widget* getWidgetAt( const Vec2f& pos );

   // VM.
   const char* meta() const;
   FUSION_DLL_API void init( VMState* vm );
   FUSION_DLL_API bool performGet( VMState* );
   FUSION_DLL_API bool performSet( VMState* );

protected:

   /*----- methods -----*/

   virtual ~Layer();

   virtual void render( const RCP<Gfx::RenderNode>& );
   virtual void performSetGeometry();
   virtual void performSetPosition();

   virtual bool isAttribute( const char* ) const;

   void  updateBuffers();

   /*----- data members -----*/

   RCP< Gfx::Texture >        _cb;
   RCP< Gfx::Texture >        _db;

   RCP< Gfx::Framebuffer >    _fb;

   Gfx::TextureState          _ts;
   RCP< Gfx::SamplerList >    _sl;
   RCP< Gfx::Geometry >       _geom;

   Vec4f                      _color;
   RCP< Gfx::ConstantBuffer > _constants;
   RCP< Gfx::ConstantList >   _cl;

   Mat4f                      _layerContentsTransform;
   Mat4f                      _layerTransform;

   RCP<Gfx::RenderNode>       _rtt_rn;
   RCP<Gfx::Pass>             _rtt_pass;

private:

   /*----- data members -----*/
};

NAMESPACE_END

#endif //FUSION_LAYER_H
