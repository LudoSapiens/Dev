/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef BASE_MAP_H
#define BASE_MAP_H

#include <Base/StdDefs.h>

#include <map>

NAMESPACE_BEGIN

/*==============================================================================
  CLASS Map
==============================================================================*/

//! Map container class.

template< typename K, typename T >
class Map
   : public std::map<K,T>
{

public:

   /*----- types and enumerations ----*/

   typedef typename std::map<K,T>::const_iterator   ConstIterator;
   typedef typename std::map<K,T>::iterator         Iterator;
   typedef typename std::map<K,T>::reverse_iterator ReverseIterator;
   
   
   /*----- methods -----*/

   Map();
   Map( const Map& );

   bool has( const K& ) const;

};

//------------------------------------------------------------------------------
//!
template< typename K, typename T >
Map<K,T>::Map()
{}

//------------------------------------------------------------------------------
//!
template< typename K, typename T >
Map<K,T>::Map( const Map& m )
   : std::map<K,T>( m )
{}

//------------------------------------------------------------------------------
//!
template< typename K, typename T > bool
Map<K,T>::has( const K& key ) const
{
   return std::map<K,T>::find(key) != this->end();
}


NAMESPACE_END

#endif

