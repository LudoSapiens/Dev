/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef BASE_PAIR_H
#define BASE_PAIR_H

#include <Base/StdDefs.h>

#include <utility>

NAMESPACE_BEGIN

/*==============================================================================
  CLASS Pair
==============================================================================*/

//! Pair class

template< typename T1, typename T2 >
class Pair
   : public std::pair<T1, T2>
{

public:

   /*----- methods -----*/

   Pair();
   Pair( const T1& t1, const T2& t2 );
   Pair( const Pair& );
};

//------------------------------------------------------------------------------
//!
template< typename T1, typename T2 >
Pair<T1, T2>::Pair()
{}

//------------------------------------------------------------------------------
//!
template< typename T1, typename T2 >
Pair<T1, T2>::Pair
( const T1& t1, const T2& t2 )
   : std::pair<T1, T2>( t1, t2 )
{}

//------------------------------------------------------------------------------
//!
template< typename T1, typename T2 >
Pair<T1, T2>::Pair
( const Pair& p )
   : std::pair<T1, T2>( p )
{}

NAMESPACE_END

#endif //BASE_PAIR_H
