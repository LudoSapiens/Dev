/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_RENDERER_H
#define PLASMA_RENDERER_H

#include <Plasma/StdDefs.h>
#include <Plasma/Geometry/MaterialSet.h>

#include <Gfx/Pass/RenderNode.h>

#include <CGMath/Vec2.h>

#include <Base/Util/RCObject.h>
#include <Base/Util/RCP.h>

NAMESPACE_BEGIN

class Viewport;
class World;
class Entity;
class Geometry;

/*==============================================================================
  CLASS Renderer
==============================================================================*/

//!

class Renderer
   : public RCObject
{

public:

   /*----- methods -----*/

   PLASMA_DLL_API Renderer();

   PLASMA_DLL_API void size( const Vec2i& size );
   inline const Vec2i& size() const { return _size; }

   PLASMA_DLL_API void beginFrame();
   PLASMA_DLL_API void endFrame();

   PLASMA_DLL_API virtual void render( const RCP<Gfx::RenderNode>&, World*, Viewport* );


   // Use only when the rendering is done in a current (not is own) buffer.
   bool clearDepth() const   { return _clearDepth; }
   void clearDepth( bool v ) { _clearDepth = v; }

protected:

   /*----- enumerations -----*/

   enum
   {
     STANDARD           = 0x0,
     CUSTOM             = 0x1,
     GHOST              = 0x2,
     REFLECTIVE         = 0x3,
     HAS_SKIN           = 0x8,
     STANDARD_SKINNED   = STANDARD   | HAS_SKIN,
     CUSTOM_SKINNED     = CUSTOM     | HAS_SKIN,
     GHOST_SKINNED      = GHOST      | HAS_SKIN,
     REFLECTIVE_SKINNED = REFLECTIVE | HAS_SKIN
   };

   /*----- classes -----*/

   struct Chunk
   {
      Chunk( int type, Entity* e, const Mat4f* t, Geometry* g, Material* m, int id ):
         _type(type), _patchId(id), _entity(e), _trf(t), _geom(g), _mat(m) {}

      int          _type;
      int          _patchId;
      Entity*      _entity;
      const Mat4f* _trf;
      Geometry*    _geom;
      Material*    _mat;
   };

   /*----- methods -----*/

   ~Renderer();
   virtual void performResize();
   virtual void performBeginFrame();
   virtual void performEndFrame();

   void classifyEntities( World* );

   Gfx::Pass* getPass();
   Gfx::RenderNode* getRenderNode();
   Gfx::SamplerList* getSamplerList();
   Gfx::ConstantList* getConstantList();

   /*----- members -----*/

   bool                             _clearDepth;
   Vec2i                            _size;
   uint                             _numPasses;
   Vector< RCP<Gfx::Pass> >         _passes;
   uint                             _numRenderNodes;
   Vector< RCP<Gfx::RenderNode> >   _renderNodes;
   uint                             _numSamplerLists;
   Vector< RCP<Gfx::SamplerList> >  _samplerLists;
   uint                             _numConstantLists;
   Vector< RCP<Gfx::ConstantList> > _constantLists;

   Vector< Chunk >                  _reflections;
   Vector< Chunk >                  _ghosts;
   Vector< Chunk >                  _visibles;
   Vector< Chunk >                  _particles;

   RCP<MaterialSet>                _defMatSet;
};


NAMESPACE_END

#endif
