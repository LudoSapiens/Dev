/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef CGMATH_CGCONST_H
#define CGMATH_CGCONST_H

#include <CGMath/StdDefs.h>

#include <Base/Util/Bits.h>

#include <limits>

NAMESPACE_BEGIN

/*==============================================================================
  CLASS CGConst
==============================================================================*/
//! The Pi value was generated using the 'bc' command-line tool:
//!   scale=50; 4*a(1)
template< typename T >
class CGConst
{
public:

   // Limits.
   static inline T min();
   static inline T max();
   static inline T infinity();
   static inline T lowest();
   static inline T highest();
   static inline T epsilon();
   static inline T epsilon( const T& v );

   // Constants.
   static inline T pi();
   static inline T pi2();
   static inline T pi_2();
   static inline T pi_4();
   static inline T one_pi();
   static inline T one_pi2();
   static inline T one_pi4();

   static inline T e();

   static inline T phi();
   static inline T one_phi();

   static inline T sqrt2();

   // Special values.
   static inline T NaN();

}; //class CGConst

//------------------------------------------------------------------------------
//! Returns the smallest representable number.
//! For fixed-point numbers, it is a large negative number (or 0 if unsigned).
//! For floating-point numbers, it is the smallest non-zero positive value.
template< typename T > inline T
CGConst<T>::min()
{
   return std::numeric_limits<T>::min();
}

//------------------------------------------------------------------------------
//! Returns the largest representable number, excluding infinity.
template< typename T > inline T
CGConst<T>::max()
{
   return std::numeric_limits<T>::max();
}

//------------------------------------------------------------------------------
//! Returns a representation of infinity, or the closest representable value.
template< typename T > inline T
CGConst<T>::infinity()
{
   return std::numeric_limits<T>::max();
}

//------------------------------------------------------------------------------
//! Returns a representation of infinity, or the closest representable value.
template<> inline float
CGConst<float>::infinity()
{
   return std::numeric_limits<float>::infinity();
}

//------------------------------------------------------------------------------
//! Returns a representation of infinity, or the closest representable value.
template<> inline double
CGConst<double>::infinity()
{
   return std::numeric_limits<double>::infinity();
}

//------------------------------------------------------------------------------
//! Returns the largest representable number, excluding infinity.
template< typename T > inline T
CGConst<T>::lowest()
{
   return std::numeric_limits<T>::min();
}

//------------------------------------------------------------------------------
//! Returns the largest representable number, excluding infinity.
template<> inline float
CGConst<float>::lowest()
{
   return -std::numeric_limits<float>::infinity();
}

//------------------------------------------------------------------------------
//! Returns the largest representable number, excluding infinity.
template<> inline double
CGConst<double>::lowest()
{
   return -std::numeric_limits<double>::infinity();
}

//------------------------------------------------------------------------------
//! Returns a representation of infinity, or the closest representable value.
template< typename T > inline T
CGConst<T>::highest()
{
   return std::numeric_limits<T>::max();
}

//------------------------------------------------------------------------------
//! Returns the largest representable number, including infinity if available.
template<> inline float
CGConst<float>::highest()
{
   return std::numeric_limits<float>::infinity();
}

//------------------------------------------------------------------------------
//! Returns the largest representable number, including infinity if available.
template<> inline double
CGConst<double>::highest()
{
   return std::numeric_limits<double>::infinity();
}

//------------------------------------------------------------------------------
//! Returns a value corresponding to a ULP (unit of least precision, or the LSB
//! of the mantissa of a floating-point number).
template< typename T > inline T
CGConst<T>::epsilon()
{
   return std::numeric_limits<T>::epsilon();
}

//------------------------------------------------------------------------------
//! Returns a value corresponding to a ULP (unit of least precision, or the LSB
//! of the mantissa of a floating-point number).
template< typename T > inline T
CGConst<T>::epsilon( const T& v )
{
   return v * epsilon();
}

#if _MSC_VER >= 1600
#pragma warning(push)
#pragma warning(disable:4305)
#endif
//------------------------------------------------------------------------------
//! Returns the value of Pi.
template< typename T > inline T
CGConst<T>::pi()
{
   return T(3.1415926535897932384626433832795);
}

//------------------------------------------------------------------------------
//! Returns the value of Pi*2.
template< typename T > inline T
CGConst<T>::pi2()
{
   return T(6.2831853071795864769252867665590);
}

//------------------------------------------------------------------------------
//! Returns the value of Pi/2.
template< typename T > inline T
CGConst<T>::pi_2()
{
   return T(1.5707963267948966192313216916398);
}

//------------------------------------------------------------------------------
//! Returns the value of Pi/4.
template< typename T > inline T
CGConst<T>::pi_4()
{
   return T(0.7853981633974483096156608458199);
}

//------------------------------------------------------------------------------
//! Returns the value of 1/Pi.
template< typename T > inline T
CGConst<T>::one_pi()
{
   return T(0.3183098861837906715377675267450);
}

//------------------------------------------------------------------------------
//! Returns the value of 1/(2*Pi).
template< typename T > inline T
CGConst<T>::one_pi2()
{
   return T(0.1591549430918953357688837633725);
}

//------------------------------------------------------------------------------
//! Returns the value of 1/(4*Pi).
template< typename T > inline T
CGConst<T>::one_pi4()
{
   return T(1.2732395447351626861510701069801);
}

//------------------------------------------------------------------------------
//! Returns the value of e (the Euler's number).
template< typename T > inline T
CGConst<T>::e()
{
   return T(2.7182818284590452353602874713527);
}

//------------------------------------------------------------------------------
//! Returns the golden ratio, i.e. (1+sqrt(5))/2.
template< typename T > inline T
CGConst<T>::phi()
{
   return T(1.6180339887498948482045868343656);
}

//------------------------------------------------------------------------------
//! Returns the inverse of the golden ratio, i.e. 2/(1+sqrt(5)).
template< typename T > inline T
CGConst<T>::one_phi()
{
   return T(0.6180339887498948482045868343656);
}

//------------------------------------------------------------------------------
//! Returns sqrt(2).
template< typename T > inline T
CGConst<T>::sqrt2()
{
   return T(1.4142135623730950488016887242097);
}

#if _MSC_VER >= 1600
#pragma warning(pop)
#endif

//------------------------------------------------------------------------------
//! Returns a NaN bit pattern.
//! This is a quiet NaN, in order to avoid hardward interrupts.
template<> inline float
CGConst<float>::NaN()
{
   const uint tmp = 0x7FFFFFFF;
   return toFloat(tmp);
}

//------------------------------------------------------------------------------
//! Returns a NaN bit pattern.
//! This is a quiet NaN, in order to avoid hardward interrupts.
template<> inline double
CGConst<double>::NaN()
{
   return (double)CGConst<float>::NaN();
}

/*==============================================================================
  TYPEDEF
==============================================================================*/

typedef CGConst< int >    CGConsti;
typedef CGConst< uint >   CGConstu;
typedef CGConst< float >  CGConstf;
typedef CGConst< double > CGConstd;


NAMESPACE_END

#endif //CGMATH_CGCONST_H
