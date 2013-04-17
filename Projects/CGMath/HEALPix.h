/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef CGMATH_HEALPIX_H
#define CGMATH_HEALPIX_H

#include <CGMath/StdDefs.h>

#include <CGMath/CGMath.h>
#include <CGMath/Vec3.h>

NAMESPACE_BEGIN

/*==============================================================================
   CLASS HEALPix
==============================================================================*/
//! 
class HEALPix
{

public: 

   /*----- static methods -----*/

   // Size.
   static inline int size( int los );
   static inline int faceSize( int los );
   static inline int faceLength( int los );

   // Conversion.
   static inline Vec3f pix2vec( int los, int face, int x, int y );

};

//------------------------------------------------------------------------------
//!
inline int
HEALPix::size( int los )
{
   return 12 * faceSize(los);
}

//------------------------------------------------------------------------------
//!
inline int
HEALPix::faceSize( int los )
{
   int fl = faceLength(los);
   return fl*fl;
}

//------------------------------------------------------------------------------
//!
inline int
HEALPix::faceLength( int los )
{
   return 1 << los;
}

//------------------------------------------------------------------------------
//!
inline Vec3f
HEALPix::pix2vec( int los, int face, int x, int y )
{
   static const int  jrll[] = { 2,2,2,2,3,3,3,3,4,4,4,4 };
   static const int  jpll[] = { 1,3,5,7,0,2,4,6,1,3,5,7 };
   
   // 2.0 * HEALPix::faceLength( los ) * _fact2[los]
   static const float fact1[] = {
      0.666666686534881590000,
      0.333333343267440800000,
      0.166666671633720400000,
      0.083333335816860199000,
      0.041666667908430099000,
      0.020833333954215050000,
      0.010416666977107525000,
      0.005208333488553762400,
      0.002604166744276881200,
      0.001302083372138440600,
      0.000651041686069220300,
      0.000325520843034610150,
      0.000162760421517305080,
      0.000081380210758652538,
      0.000040690105379326269,
      0.000020345052689663135
   };
   
   // 4.0 / HEALPix::size( los )
   static const float fact2[] = {
      0.33333334326744080000000000,
      0.08333333581686019900000000,
      0.02083333395421505000000000,
      0.00520833348855376240000000,
      0.00130208337213844060000000,
      0.00032552084303461015000000,
      0.00008138021075865253800000,
      0.00002034505268966313500000,
      0.00000508626317241578360000,
      0.00000127156579310394590000,
      0.00000031789144827598648000,
      0.00000007947286206899661900,
      0.00000001986821551724915500,
      0.00000000496705387931228870,
      0.00000000124176346982807220,
      0.00000000031044086745701804
   };
   
   int jr = ( jrll[face] << los ) - x - y - 1;
   
   const int nside = faceLength(los);
   int  nl4        = nside * 4;
   
   int nr;
   int kshift;
   float z;
   
   if( jr < nside )
   {
      nr     = jr;
      z      = 1 - nr * nr * fact2[los];
      kshift = 0;
   }
   else if( jr > 3 * nside )
   {
      nr     = nl4 - jr;
      z      = nr * nr * fact2[los] - 1;
      kshift = 0;
   }
   else
   {
      nr     = nside;
      z      = ( 2 * nside - jr ) * fact1[los];
      kshift = ( jr - nside ) & 1;
   }
   
   int jp = ( jpll[face] * nr + x - y + 1 + kshift ) / 2;
   if( jp > nl4 )
   {
      jp -= nl4;
   }
   if( jp < 1 ) 
   {
      jp += nl4;
   }
   
   float phi   = ( jp - ( kshift + 1 ) * 0.5f ) * ( CGConstf::pi_2() / nr );
   float scale = CGM::sqrt( 1.0f - z * z );
   
   return Vec3f( CGM::sin( phi ) * scale, z, CGM::cos( phi ) * scale );
}


NAMESPACE_END

#endif
