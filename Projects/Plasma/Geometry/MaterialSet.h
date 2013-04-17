/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_MATERIALSET_H
#define PLASMA_MATERIALSET_H

#include <Plasma/StdDefs.h>
#include <Plasma/Geometry/Material.h>

#include <Fusion/VM/VM.h>

#include <CGMath/Math.h>

#include <Base/ADT/Vector.h>
#include <Base/ADT/MapWithDefault.h>
#include <Base/Util/RCP.h>

NAMESPACE_BEGIN

/*==============================================================================
  CLASS MaterialMap
==============================================================================*/
class MaterialMap:
   public RCObject
{
public:

   /*----- types -----*/

   enum {
      NO_DEFAULT = (uint)-1
   };

   typedef MapWithDefault<uint, uint>::Iterator       Iterator;
   typedef MapWithDefault<uint, uint>::ConstIterator  ConstIterator;

   /*----- methods -----*/

   inline MaterialMap(): _map( NO_DEFAULT ) {}

   inline void  setDefault( uint v ) { _map.setDefault(v); }
   inline uint  getDefault() const   { return _map.getDefault(); }

   inline void  set( uint oldIdx, uint newIdx ) { _map.set(oldIdx, newIdx); }

   inline uint  get( uint oldIdx ) const { return _map.get(oldIdx); }

   inline uint operator[]( uint oldIdx ) const { return _map[oldIdx]; }

   inline      Iterator begin()       { return _map.begin(); }
   inline ConstIterator begin() const { return _map.begin(); }
   inline      Iterator end  ()       { return _map.end();   }
   inline ConstIterator end  () const { return _map.end();   }
   inline      Iterator find( uint oldIdx )       { return _map.find(oldIdx); }
   inline ConstIterator find( uint oldIdx ) const { return _map.find(oldIdx); }


   void  print( TextStream& os = StdErr ) const;

   static PLASMA_DLL_API void  toMap( VMState* vm, int idx, MaterialMap& map );

protected:
   MapWithDefault<uint, uint>  _map;

}; //class MaterialMap


/*==============================================================================
   CLASS MaterialSet
==============================================================================*/

class MaterialSet:
   public RCObject
{
public:

   /*----- methods -----*/

   MaterialSet();

   // Material setting.
   void add( Material* );
   void remove( Material* );
   void clear();

   //
   uint numMaterials() const          { return uint(_materials.size()); }
   Material* material( uint i ) const { return _materials[ CGM::min( i, uint(_materials.size()-1) ) ].ptr(); }

   void  remap( const MaterialMap& map );

protected:

   /*----- methods -----*/

   virtual ~MaterialSet();

   /*----- data members -----*/

   Vector< RCP<Material> > _materials;
};

NAMESPACE_END

#endif
