/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_STIMULUS_H
#define PLASMA_STIMULUS_H

#include <Plasma/StdDefs.h>

#include <Fusion/VM/VM.h>

#include <Base/Util/RCObject.h>

NAMESPACE_BEGIN


//------------------------------------------------------------------------------
//! A utility template function used to create objects of a specific type.
//! Useful for registering a Stimulus
//!   Stimulus::registerStimulus( type, create<Stimulus, SpecificStimulus> );
template< typename RetType, typename SrcType >
inline RetType*  create()
{
   return new SrcType();
}

/*==============================================================================
  CLASS Stimulus
==============================================================================*/
class Stimulus:
   public RCObject
{
public:
   /*----- types -----*/

   typedef Stimulus* (*CtorFunc)();

   /*----- static methods -----*/

   static PLASMA_DLL_API void  initialize();
   static PLASMA_DLL_API void  terminate();

   static PLASMA_DLL_API bool  registerStimulus( const ConstString& type, CtorFunc ctorFunc );
   static PLASMA_DLL_API bool  unregisterStimulus( const ConstString& type );

   static Stimulus*  create( const ConstString& type );

   /*----- methods -----*/

   PLASMA_DLL_API Stimulus();

   PLASMA_DLL_API virtual const ConstString&  type() const = 0;

   PLASMA_DLL_API virtual void push( VMState* vm );
   PLASMA_DLL_API virtual bool to( VMState* vm, int idx );

protected:

   /*----- methods -----*/

   PLASMA_DLL_API virtual ~Stimulus();

private:
}; //class Stimulus


NAMESPACE_END

#endif //PLASMA_STIMULUS_H
