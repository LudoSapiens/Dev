/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef GFX_NULL_CONTEXT_H
#define GFX_NULL_CONTEXT_H

#include <Gfx/StdDefs.h>

#include <Gfx/Mgr/Context.h>

NAMESPACE_BEGIN

namespace Gfx
{

/*==============================================================================
  CLASS NullContext
==============================================================================*/
class NullContext:
   public Context
{
public:

   /*----- methods -----*/

   GFX_DLL_API NullContext();
   GFX_DLL_API virtual ~NullContext();

   GFX_DLL_API virtual bool vsync() const;
   GFX_DLL_API virtual void vsync( bool v );

protected:
   GFX_DLL_API virtual RCP<Manager>  createManager();

private:
}; //class NullContext

} //namepsace Gfx

NAMESPACE_END

#endif //GFX_NULL_CONTEXT_H
