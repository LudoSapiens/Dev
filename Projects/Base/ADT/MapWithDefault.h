/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef BASE_MAP_WITH_DEFAULT_H
#define BASE_MAP_WITH_DEFAULT_H

#include <Base/StdDefs.h>

#include <Base/ADT/Map.h>

NAMESPACE_BEGIN

/*==============================================================================
  CLASS MapWithDefault
==============================================================================*/
template< typename K, typename T >
class MapWithDefault:
   public Map<K,T>
{
public:

   /*----- methods -----*/

   MapWithDefault( const T& v = T() ): _default( v ) {}

   void      setDefault( const T& v ) { _default = v;    }
   const T&  getDefault() const       { return _default; }

   void  set( const K& k, const T& v ) { Map<K,T>::operator[](k) = v; }

   const T&  get( const K& k ) const
   {
      typename Map<K,T>::ConstIterator it = Map<K,T>::find(k);
      if( it != Map<K,T>::end() )  return (*it).second;
      else                         return _default;
   }

   const T& operator[]( const K& k ) const { return get(k); }

protected:

   /*----- data members -----*/

   T  _default;

   /*----- methods -----*/

   /* methods... */

private:
}; //class MapWithDefault


NAMESPACE_END

#endif //BASE_MAP_WITH_DEFAULT_H
