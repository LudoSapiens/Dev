/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef UNION_H
#define UNION_H

#include <Base/StdDefs.h>

NAMESPACE_BEGIN

union U8_t
{
   char      c;
   int8_t    i8;
   uint8_t   u8;
};

union U16_t
{
   char      c[2];
   int8_t    i8[2];
   uint8_t   u8[2];

   short     s;
   int16_t   i16;
   uint16_t  u16;
};

union U32_t
{
   char      c[4];
   int8_t    i8[4];
   uint8_t   u8[4];

   short     s[2];
   int16_t   i16[2];
   uint16_t  u16[2];

   int       i;
   float     f;
   int32_t   i32;
   uint32_t  u32;
   float     f32;
};

union U64_t
{
   char      c[8];
   int8_t    i8[8];
   uint8_t   u8[8];

   short     s[4];
   int16_t   i16[4];
   uint16_t  u16[4];

   int       i[2];
   float     f[2];
   int32_t   i32[2];
   uint32_t  u32[2];
   float     f32[2];

   double    d;
   double    f64;
   int64_t   i64;
   uint64_t  u64;

   void*     p; // Guaranteed to exist in both 32b and 64b architectures.
};

union U128_t
{
   char      c[16];
   int8_t    i8[16];
   uint8_t   u8[16];

   short     s[8];
   int16_t   i16[8];
   uint16_t  u16[8];

   int       i[4];
   float     f[4];
   int32_t   i32[4];
   uint32_t  u32[4];
   float     f32[4];

   double    d[2];
   double    f64[2];
   int64_t   i64[2];
   uint64_t  u64[2];
};

union U256_t
{
   char      c[32];
   int8_t    i8[32];
   uint8_t   u8[32];

   short     s[16];
   int16_t   i16[16];
   uint16_t  u16[16];

   int       i[8];
   float     f[8];
   int32_t   i32[8];
   uint32_t  u32[8];
   float     f32[8];

   double    d[4];
   double    f64[4];
   int64_t   i64[4];
   uint64_t  u64[4];
};

NAMESPACE_END

#endif //UNION_H
