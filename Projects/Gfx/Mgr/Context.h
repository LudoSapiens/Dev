/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef GFX_CONTEXT_H
#define GFX_CONTEXT_H

#include <Gfx/StdDefs.h>

#include <Base/ADT/String.h>
#include <Base/Util/RCObject.h>
#include <Base/Util/RCP.h>

NAMESPACE_BEGIN

namespace Gfx
{

class Manager;

/*==============================================================================
  CLASS Context
==============================================================================*/
class Context:
   public RCObject
{
public:

   /*----- methods -----*/
   static GFX_DLL_API RCP<Context>  getDefaultContext( const String& preferredAPI = String(), void* window = NULL );

   virtual GFX_DLL_API  bool vsync() const = 0;
   virtual GFX_DLL_API  void vsync( bool v ) = 0;

protected:

   friend class Manager;

   /*----- methods -----*/

   GFX_DLL_API Context();
   GFX_DLL_API virtual ~Context();

   GFX_DLL_API virtual RCP<Manager>  createManager() = 0;

private:
}; //class Context


}  //namespace Gfx

NAMESPACE_END


#endif //GFX_CONTEXT_H
