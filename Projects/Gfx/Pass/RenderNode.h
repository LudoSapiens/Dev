/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef GFX_RENDER_NODE_H
#define GFX_RENDER_NODE_H

#include <Gfx/StdDefs.h>
#include <Gfx/Pass/Pass.h>

#include <Base/Util/RCObject.h>
#include <Base/Util/RCP.h>
#include <Base/ADT/Vector.h>


NAMESPACE_BEGIN

namespace Gfx
{


/*==============================================================================
  CLASS RenderNode
==============================================================================*/
class RenderNode:
   public RCObject
{
public:

   /*----- methods -----*/

   GFX_DLL_API RenderNode();
   GFX_DLL_API virtual ~RenderNode();

   // Requirements
   void  addRequirement( const RCP<RenderNode>& req ) { _requirements.pushBack(req); }
   uint  nbRequirements() const { return (uint)_requirements.size(); }
         RCP<RenderNode>&  getRequirement( const uint id )       { return _requirements[id]; }
   const RCP<RenderNode>&  getRequirement( const uint id ) const { return _requirements[id]; }

   // Passes
   void  addPass( const RCP<Pass>& p ) { _passes.pushBack(p); }
   uint  numPasses() const { return (uint)_passes.size(); }
         RCP<Pass>&  getPass( const uint id )       { return _passes[id]; }
   const RCP<Pass>&  getPass( const uint id ) const { return _passes[id]; }

   const RCP<Pass>&  current() const { return _passes.back(); }

   void  clear() { _requirements.clear(); _passes.clear(); }

protected:

   GFX_MAKE_MANAGERS_FRIENDS();

   /*----- data members -----*/

   Vector< RCP<RenderNode> >  _requirements;
   Vector< RCP<Pass> >        _passes;

private:
}; //class RenderNode


}  //namespace Gfx

NAMESPACE_END


#endif //GFX_RENDER_NODE_H
