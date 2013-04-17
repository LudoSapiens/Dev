/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef BASE_PACKED_XYZ_H
#define BASE_PACKED_XYZ_H

#include <Base/StdDefs.h>

NAMESPACE_BEGIN

/*==============================================================================
  CLASS X11Y11Z10
==============================================================================*/
class X11Y11Z10
{
public:

   /*----- methods -----*/

   X11Y11Z10(){}
   X11Y11Z10( uint32_t x, uint32_t y, uint32_t z ){ set(x, y, z); }
   //X11Y11Z10( int32_t x, int32_t y, int32_t z ){ set(x, y, z); }

   // Unsigned routines.
   inline uint32_t  x() const { return (_bits         ) & MASK_X; }
   inline uint32_t  y() const { return (_bits>>SHIFT_Y) & MASK_Y; }
   inline uint32_t  z() const { return (_bits>>SHIFT_Z)         ; }

   inline void  x( uint32_t v ) { _bits &= ~(MASK_X         ); _bits |= ((v&MASK_X)         ); }
   inline void  y( uint32_t v ) { _bits &= ~(MASK_Y<<SHIFT_Y); _bits |= ((v&MASK_Y)<<SHIFT_Y); }
   inline void  z( uint32_t v ) { _bits &= ~(MASK_Z<<SHIFT_Z); _bits |= ((v&MASK_Z)<<SHIFT_Z); }

   inline void xy( uint32_t vx, uint32_t vy ) { x(vx); y(vy); }
   inline void xz( uint32_t vx, uint32_t vz ) { x(vx); z(vz); }
   inline void yz( uint32_t vy, uint32_t vz ) { y(vy); z(vz); }

   inline void set( uint32_t vx, uint32_t vy, uint32_t vz ) { x(vx); y(vy); z(vz); }

   // Signed routines.
   inline int32_t  xs() const { return int32_t(_bits<<(32-SIZE_X        ))>>(32-SIZE_X); }
   inline int32_t  ys() const { return int32_t(_bits<<(32-SIZE_Y-SHIFT_Y))>>(32-SIZE_Y); }
   inline int32_t  zs() const { return int32_t(_bits<<(32-SIZE_Z-SHIFT_Z))>>(32-SIZE_Z); }

   //inline void  x( int32_t v ) { _bits &= ~(MASK_X         ); _bits |= ((v&MASK_X)         ); }
   //inline void  y( int32_t v ) { _bits &= ~(MASK_Y<<SHIFT_Y); _bits |= ((v&MASK_Y)<<SHIFT_Y); }
   //inline void  z( int32_t v ) { _bits &= ~(MASK_Z<<SHIFT_Z); _bits |= ((v&MASK_Z)<<SHIFT_Z); }

   //inline void set( int32_t vx, int32_t vy, int32_t vz ) { x(vx); y(vy); z(vz); }

   inline uint32_t  bits() const { return _bits; }

protected:

   enum
   {
      SIZE_X = 11,
      SIZE_Y = 11,
      SIZE_Z = 10,
      SHIFT_X = 0,
      SHIFT_Y = SIZE_X,
      SHIFT_Z = SIZE_X + SIZE_Y,
      MASK_X = (1<<SIZE_X)-1,
      MASK_Y = (1<<SIZE_Y)-1,
      MASK_Z = (1<<SIZE_Z)-1,
   };

   /*----- data members -----*/

   uint32_t  _bits;

private:
}; //class X11Y11Z10


NAMESPACE_END

#endif //BASE_PACKED_XYZ_H
