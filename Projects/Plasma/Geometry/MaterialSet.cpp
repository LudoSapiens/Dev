/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/Geometry/MaterialSet.h>

NAMESPACE_BEGIN

/*==============================================================================
   CLASS MaterialMap
==============================================================================*/

//------------------------------------------------------------------------------
//!
void
MaterialMap::print( TextStream& os ) const
{
   os << "{";
   const char* sep = "";
   for( MapWithDefault<uint, uint>::ConstIterator cur = _map.begin();
        cur != _map.end();
        ++cur )
   {
      os << sep << "[" << cur->first << "]=" << cur->second;
      sep = ", ";
   }
   os << "}";
}

//------------------------------------------------------------------------------
//!
void
MaterialMap::toMap( VMState* vm, int idx, MaterialMap& map )
{
   if( VM::isTable( vm, idx ) )
   {
      VM::push( vm );
      while( VM::next( vm, idx ) )
      {
         if( VM::isNumber( vm, -2 ) )
         {
            uint newIdx = VM::toUInt( vm, -2 ) - 1;
            uint oldIdx = VM::toUInt( vm, -1 ) - 1;
            map.set( newIdx, oldIdx );
         }
         else
         {
            String key = VM::toString( vm, -2 );
            if( key == "default" )
            {
               uint defIdx = VM::toUInt( vm, -1 ) - 1;
               map.setDefault( defIdx );
            }
            else
            {
               StdErr << "ERROR - MaterialMap::toMap() - Invalid key '" << key << "'." << nl;
            }
         }
         VM::pop( vm );
      }
   }
   else
   {
      StdErr << "ERROR - MaterialMap needs a table." << nl;
   }
}

/*==============================================================================
   CLASS MaterialSet
==============================================================================*/
   
//------------------------------------------------------------------------------
//! 
MaterialSet::MaterialSet()
{
}

//------------------------------------------------------------------------------
//! 
MaterialSet::~MaterialSet()
{
}

//------------------------------------------------------------------------------
//!
void 
MaterialSet::add( Material* mat )
{
   _materials.pushBack( mat );
}

//------------------------------------------------------------------------------
//! 
void
MaterialSet::remove( Material* mat )
{
   _materials.remove( mat );
}

//------------------------------------------------------------------------------
//! 
void
MaterialSet::clear()
{
   _materials.clear();
}

//------------------------------------------------------------------------------
//!
void
MaterialSet::remap( const MaterialMap& map )
{
   Vector< RCP<Material> > newMats;

   // Assuming we have 10 materials:
   //   0 1 2 3 4 5 6 7 8 9
   if( map.getDefault() == MaterialMap::NO_DEFAULT )
   {
      // Assume we have { [2]=3, [5]=2 }.
      // We need to yield:
      //   0 1 3 3 4 2 6 7 8 9
      uint n = numMaterials();
      for( uint i = 0; i < n; ++i )
      {
         MaterialMap::ConstIterator it = map.find( i );
         uint oldIdx = (it == map.end() ? i : it->second);
         newMats.pushBack( material(oldIdx) );
      }
   }
   else
   {
      // Assume we have { [2]=3, [5]=2, default=8 }.
      // We need to yield:
      //   8 8 3 8 8 2 8_8 8 8 (last 3 are superfluous).
      Material* defMat = material( map.getDefault() );
      for( MaterialMap::ConstIterator cur = map.begin();
           cur != map.end();
           ++cur )
      {
         uint newIdx = cur->first;
         uint oldIdx = cur->second;
         newMats.grow( newIdx+1, defMat );
         newMats[newIdx] = material(oldIdx);
      }
      if( newMats.back() != defMat )  newMats.pushBack( defMat );
   }

   _materials.swap( newMats );
}

NAMESPACE_END
