/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef BASE_ENUM_H
#define BASE_ENUM_H

#include <Base/StdDefs.h>

NAMESPACE_BEGIN

//------------------------------------------------------------------------------
//! This file contains a few macros to automatically generate utility routines
//! for some enumerations.


#define GEN_ENUM_BITWISE_OPS2( Type, SubType ) \
   inline Type operator~( const Type& v ) \
   { \
      return Type( ~SubType(v) ); \
   } \
   inline Type operator&( const Type& lhs, const Type& rhs ) \
   { \
      return Type( SubType(lhs) & SubType(rhs) ); \
   } \
   inline Type operator|( const Type& lhs, const Type& rhs ) \
   { \
      return Type( SubType(lhs) | SubType(rhs) ); \
   } \
   inline Type operator^( const Type& lhs, const Type& rhs ) \
   { \
      return Type( SubType(lhs) ^ SubType(rhs) ); \
   } \
   inline Type& operator&=( Type& lhs, const Type& rhs ) \
   { \
      return lhs = Type( SubType(lhs) & SubType(rhs) ); \
   } \
   inline Type& operator|=( Type& lhs, const Type& rhs ) \
   { \
      return lhs = Type( SubType(lhs) | SubType(rhs) ); \
   } \
   inline Type& operator^=( Type& lhs, const Type& rhs ) \
   { \
      return lhs = Type( SubType(lhs) ^ SubType(rhs) ); \
   } \
   inline Type& operator<<=( Type& v, SubType sh ) \
   { \
      return v = Type( SubType(v) << sh ); \
   } \
   inline Type& operator>>=( Type& v, SubType sh ) \
   { \
      return v = Type( SubType(v) >> sh ); \
   }

#define GEN_ENUM_BITWISE_OPS( Type )  GEN_ENUM_BITWISE_OPS2( Type, int )


NAMESPACE_END

#endif //BASE_ENUM_H
