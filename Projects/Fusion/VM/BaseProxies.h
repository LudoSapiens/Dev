/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef FUSION_BASE_PROXIES_H
#define FUSION_BASE_PROXIES_H

#include <Fusion/StdDefs.h>

#include <Fusion/VM/VMObject.h>

#include <Base/Util/RCObject.h>
#include <Base/Util/RCP.h>

NAMESPACE_BEGIN


namespace BaseProxies
{

   FUSION_DLL_API void initialize( VMState* );

} // GfxProxies

namespace FS
{
   class Entry;
}

/*==============================================================================
  CLASS BaseFSEntry
==============================================================================*/
class BaseFSEntry:
   public RCObject
{

public:

   /*----- methods -----*/

   FUSION_DLL_API BaseFSEntry();

   // VM.
   FUSION_DLL_API void  init( VMState* );
   FUSION_DLL_API bool  performGet( VMState* );
   FUSION_DLL_API bool  performSet( VMState* );

         FS::Entry*  get()       { return _entry; }
   const FS::Entry*  get() const { return _entry; }

protected: 

   /*----- methods -----*/

   virtual ~BaseFSEntry();

   FS::Entry*  _entry;

}; //BaseFSEntry

//------------------------------------------------------------------------------
//! VM Section
VMOBJECT_TRAITS( BaseFSEntry, Entry )
typedef VMObject< BaseFSEntry >  BaseFSEntryVM;


NAMESPACE_END

#endif //FUSION_BASE_PROXIES_H
