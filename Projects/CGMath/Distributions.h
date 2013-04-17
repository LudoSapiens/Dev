/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef CGMATH_DISTRIBUTIONS_H
#define CGMATH_DISTRIBUTIONS_H

#include <CGMath/StdDefs.h>

#include <CGMath/CGMath.h>
#include <CGMath/Vec3.h>

NAMESPACE_BEGIN

/*==============================================================================
  CLASS Uniform01
==============================================================================*/
//!< A distribution which is just a passthrough.
template< typename T=double >
class Uniform01
{
public:

   /*----- types -----*/
   typedef T  InputType;
   typedef T  ReturnType;

   /*----- methods -----*/

   Uniform01() { }

   Uniform01( const Uniform01& u ) { *this = u; }

   //---------------------------------------------------------------------------
   //! Remaps a value into the valid range.
   ReturnType operator()( const InputType& v ) const
   {
      return v;
   }

   //---------------------------------------------------------------------------
   //! Remaps the outcome of the generator into the valid range.
   template< class Generator >
   ReturnType operator()( Generator& gen ) const
   {
      return this->operator()( (InputType)gen() );
   }

protected:

private:
}; //class Uniform01


/*==============================================================================
  CLASS UniformReal
==============================================================================*/
//!< A distribution which remaps the linear input into a different range.
template< typename T=double >
class UniformReal
{
public:

   /*----- types -----*/
   typedef T  InputType;
   typedef T  ReturnType;

   /*----- methods -----*/

   UniformReal( const InputType& min = InputType(0), const InputType& max = InputType(1) )
   { set(min, max); }

   UniformReal( const UniformReal& u )
   { *this = u; }

   //--------------------------------------------------------------------------
   //! Sets the range with a minimum and a maximum value.
   void  set( const InputType& min, const InputType& max )
   {
      _scale = max - min;
      _bias = min;
   }

   //--------------------------------------------------------------------------
   //! Sets the range with a source range mapping into a destination range.
   void  set( const InputType&  srcMin, const InputType&  srcMax,
              const ReturnType& dstMin, const ReturnType& dstMax )
   {
      _scale = (dstMax - dstMin)/(srcMax - srcMin);
      _bias  = (dstMin - srcMin);
   }

   //---------------------------------------------------------------------------
   //! Remaps a value into the valid range.
   ReturnType operator()( const InputType& v ) const
   {
      return v*_scale + _bias;
   }

   //---------------------------------------------------------------------------
   //! Remaps the outcome of the generator into the valid range.
   template< class Generator >
   ReturnType operator()( Generator& gen ) const
   {
      return this->operator()( (InputType)gen() );
   }

protected:

   /*----- data members -----*/

   T  _scale;  //!< The scaling factor.
   T  _bias;   //!< The bias value.

private:
}; //class UniformReal


/*==============================================================================
  CLASS UniformAvgVar
==============================================================================*/
//!< A distribution which remaps a uniform distribution from [0, 1] into a
//!< another uniform distribution centered around an average, and varying
//!< to a certain range (which we refer to as variance).
//!< In summary, we need to convert a value from [0, 1] into [-1, 1], multiply
//!< that value by that variance, and then add the average.
//!< We therefore optimize:
//!<   (2*x - 1)*var + avg
//!< into:
//!<   (2*var)*x + (avg-var)
template< typename T=double >
class UniformAvgVar
{
public:

   /*----- types -----*/
   typedef T  InputType;
   typedef T  ReturnType;

   /*----- methods -----*/

   UniformAvgVar( const InputType& avg = InputType(0), const InputType& var = InputType(1) )
   { set(avg, var); }

   UniformAvgVar( const UniformAvgVar& u )
   { *this = u; }

   void  set( const InputType& avg, const InputType& var )
   {
      _scale = var * (InputType)2;
      _bias  = avg - var;
   }

   InputType  scale() const { return _scale; }
   InputType  bias()  const { return _bias;  }
   InputType  variance() const { return _scale / (InputType)2; }
   InputType  average() const { return _bias + variance(); }

   void  scale( const InputType& v ) { _scale = v; }
   void  bias ( const InputType& v ) { _bias  = v; }
   void  variance( const InputType& v ) { set(average(), v); }
   void  average( const InputType& v ) { set(v, variance()); }


   //---------------------------------------------------------------------------
   //! Remaps a value into the valid range.
   //! We simply remap to [-1, 1] range, and multiply by the variance.
   ReturnType operator()( const InputType& v ) const
   {
      return v*_scale + _bias;
   }

   //---------------------------------------------------------------------------
   //! Remaps the outcome of the generator into the valid range.
   template< class Generator >
   ReturnType operator()( Generator& gen ) const
   {
      return this->operator()( (InputType)gen() );
   }

protected:

   /*----- data members -----*/

   T  _scale;  //!< A value equivalent to 2*var.
   T  _bias;   //!< A value equivalent to (avg-var).

private:
}; //class UniformAvgVar


/*==============================================================================
  CLASS UniformSphere
==============================================================================*/
//!< A distribution which is uniform over a sphere.
//!< Note: This is not as simple as randomizing each dimension, and renormalizing,
//!<       as the corners of the cube will bias the values (boost is bad).
template< typename T=double >
class UniformSphere
{
public:

   /*----- types -----*/
   typedef Vec3<T>  InputType;
   typedef Vec3<T>  ReturnType;

   /*----- methods -----*/

   UniformSphere() { }

   UniformSphere( const UniformSphere& u ) { *this = u; }

   //---------------------------------------------------------------------------
   //! Remaps the outcome of the generator into the valid range.
   template< class Generator >
   ReturnType operator()( Generator& gen ) const
   {
      Vec3<T> d;
      T l;
      do
      {
         d(0) = (T)2 * (T)gen() - (T)1;
         d(1) = (T)2 * (T)gen() - (T)1;
         d(2) = (T)2 * (T)gen() - (T)1;
         l = d.sqrLength();
      } while ( l > T(1) || l == T(0) );
      l = (T)1 / CGM::sqrt( l );
      return d*l;
   }

   //---------------------------------------------------------------------------
   //! Generates a vector in the hemisphere centered around the specified direction.
   template< class Generator >
   ReturnType operator()( Generator& gen, const Vec3<T>& dir ) const
   {
      Vec3<T> d = this->operator()( gen );
      if( d.dot( dir ) < T(0) )
      {
         // Looking backwards, just flip in front.
         return -d;
      }
      else
      {
         // Already in hemisphere.
         return d;
      }
   }

protected:

private:
}; //class UniformSphere


/*==============================================================================
  CLASS UniformHemisphere
==============================================================================*/
//!< A distribution which is uniform over an hemisphere.
//!< Note: This is not as simple as randomizing each dimension, and renormalizing,
//!<       as the corners of the cube will bias the values (boost is bad).
template< typename T=double >
class UniformHemisphere:
   public UniformSphere<T>
{
public:

   UniformHemisphere( const T& x, const T& y, const T& z ): _dir( x, y, z ) { }

   UniformHemisphere( const Vec3<T>& dir ): _dir( dir ) { }

   UniformHemisphere( const UniformHemisphere& u ) { *this = u; }

   const Vec3<T>&  direction() const { return _dir; }

   void  direction( const Vec3<T>& dir ) { _dir = dir; }

   //---------------------------------------------------------------------------
   //! Remaps the outcome of the generator into the valid range.
   template< class Generator >
   typename UniformSphere<T>::ReturnType operator()( Generator& gen ) const
   {
      return UniformSphere<T>::operator()( gen, _dir );
   }

protected:
   Vec3<T>  _dir;  //!< The direction.

private:
}; //class UniformHemisphere


/*==============================================================================
  CLASS UniformSegment
==============================================================================*/
//!< A distribution which is uniform over a line segment in space.
template< typename T=double >
class UniformSegment
{
public:

   /*----- types -----*/
   typedef T        InputType;
   typedef Vec3<T>  ReturnType;

   /*----- methods -----*/

   UniformSegment( const Vec3<T>& ptA, const Vec3<T>& ptB ) { set(ptA, ptB); }

   UniformSegment( const UniformSegment& u ) { *this = u; }

   void  set( const Vec3<T>& ptA, const Vec3<T>& ptB )
   {
      _scale = ptB - ptA;
      _bias  = ptA;
   }

   const Vec3<T>&  scale() const { return _scale; }
   const Vec3<T>&  bias()  const { return _bias;  }

   void  scale( const Vec3<T>& v ) { _scale = v; }
   void  bias ( const Vec3<T>& v ) { _bias  = v; }

   const Vec3<T>&  endpointA() const { return _bias; }
         Vec3<T>   endpointB() const { return _scale + _bias; }

   //---------------------------------------------------------------------------
   //! Remaps a value into the valid range.
   ReturnType operator()( const InputType& v ) const
   {
      return v*_scale + _bias;
   }

   //---------------------------------------------------------------------------
   //! Remaps the outcome of the generator into the valid range.
   template< class Generator >
   ReturnType operator()( Generator& gen ) const
   {
      return this->operator()( (InputType)gen() );
   }

protected:
   Vec3<T>  _scale;
   Vec3<T>  _bias;

private:
}; //class UniformSegment


/*==============================================================================
  CLASS UniformBox
==============================================================================*/
//!< A distribution which is uniform inside an axis-aligned box in space.
template< typename T=double >
class UniformBox
{
public:

   /*----- types -----*/
   typedef Vec3<T>  InputType;
   typedef Vec3<T>  ReturnType;

   /*----- methods -----*/

   UniformBox( const Vec3<T>& minPt, const Vec3<T>& maxPt ) { set(minPt, maxPt); }

   UniformBox( const UniformBox& u ) { *this = u; }

   void  set( const Vec3<T>& minPt, const Vec3<T> maxPt )
   {
      _scale = maxPt - minPt;
      _bias  = minPt;
   }

   const Vec3<T>&  scale() const { return _scale; }
   const Vec3<T>&  bias()  const { return _bias;  }

   void  scale( const Vec3<T>& v ) { _scale = v; }
   void  bias ( const Vec3<T>& v ) { _bias = v;  }

   //---------------------------------------------------------------------------
   //! Remaps a value into the valid range.
   ReturnType operator()( InputType& v ) const
   {
      return v*_scale + _bias;
   }

   ReturnType operator()( const InputType& v ) const
   {
      return v*_scale + _bias;
   }

   //---------------------------------------------------------------------------
   //! Remaps a value into the valid range.
   ReturnType operator()( const T& v0, const T& v1, const T& v2 ) const
   {
      return operator()( InputType(v0, v1, v2) );
   }

   //---------------------------------------------------------------------------
   //! Remaps the outcome of the generator into the valid range.
   template< class Generator >
   ReturnType operator()( Generator& gen ) const
   {
      return this->operator()( InputType((T)gen(), (T)gen(), (T)gen()) );
   }

protected:
   Vec3<T>  _scale;
   Vec3<T>  _bias;

}; //class UniformBox


// Typedefs
typedef UniformReal<float>  UniformFloat;

NAMESPACE_END

#endif //CGMATH_DISTRIBUTIONS_H
