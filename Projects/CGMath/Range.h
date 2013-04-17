/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef CGMATH_RANGE_H
#define CGMATH_RANGE_H

#include <CGMath/StdDefs.h>

#include <Base/IO/TextStream.h>

NAMESPACE_BEGIN

/*==============================================================================
  CLASS Range
==============================================================================*/

//! Range class to keep minimum and maximum values of a series.

template< typename T >
class Range
{
public:

   /*----- static methods -----*/

   inline static Range<T> empty();

   /*----- methods -----*/

   inline Range();
   inline Range( T val );
   inline Range( T x0, T x1 );

   // Public methods
   inline void set( T val );
   inline void set( T x0, T x1 );

   inline void operator|=( T val );
   inline void operator|=( const Range<T>& r );
   inline void grow( T v );
   inline void shrink( T v );

   inline void translate( T v );

   inline T min() const;
   inline T max() const;

   inline T& min();
   inline T& max();
   inline Range& min( T v );
   inline Range& max( T v );

   inline T size() const;

   inline T center() const;

   inline bool isEmpty() const;

   inline bool isOverlapping( const Range<T>& r ) const;
   inline bool isInside( T v ) const;
   inline bool isInsideCO( T v ) const;
   inline bool isInsideOO( T v ) const;
   inline bool isInside( const Range<T>& r ) const;

private:

   /*----- data members -----*/

   T  _min;  //!< The smallest value encountered.
   T  _max;  //!< The largest value encountered.
};

//------------------------------------------------------------------------------
//!
template< typename T > inline Range<T>
Range<T>::empty()
{
   return Range<T>(
      CGConst<T>::highest(), CGConst<T>::lowest()
   );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline
Range<T>::Range() {}

//------------------------------------------------------------------------------
//!
template< typename T > inline
Range<T>::Range( T r )
{
   set( r );
}

//------------------------------------------------------------------------------
//!
template< typename T >  inline
Range<T>::Range( T x0, T x1 )
{
   set( x0, x1 );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline void
Range<T>::set( T r )
{
   _min = _max = r;
}

//------------------------------------------------------------------------------
//!
template< typename T >  inline void
Range<T>::set( T x0, T x1 )
{
   _min = x0;
   _max = x1;
}

//------------------------------------------------------------------------------
//! Add a point to the rectangle.
template< typename T > inline void
Range<T>::operator|=( T v )
{
   _min = CGM::min( _min, v );
   _max = CGM::max( _max, v );
}

//------------------------------------------------------------------------------
//! Add a rectangle to the current one.
template< typename T > inline void
Range<T>::operator|=( const Range<T>& rect )
{
   _min = CGM::min( _min, rect._min );
   _max = CGM::max( _max, rect._max );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline void
Range<T>::grow( T v )
{
   _min -= v;
   _max += v;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline void
Range<T>::shrink( T v )
{
   _min += v;
   _max -= v;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline void
Range<T>::translate( T v )
{
   _min += v;
   _max += v;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline T
Range<T>::min() const
{
   return _min;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline T
Range<T>::max() const
{
   return _max;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline T&
Range<T>::min()
{
   return _min;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline T&
Range<T>::max()
{
   return _max;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Range<T>&
Range<T>::min( T v )
{
   _min = v;
   return *this;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Range<T>&
Range<T>::max( T v )
{
   _max = v;
   return *this;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline T
Range<T>::size() const
{
   return _max - _min;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline T
Range<T>::center() const
{
   return (_min + _max) / (T)2;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline bool
Range<T>::isEmpty() const
{
   return _max <= _min;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline bool
Range<T>::isOverlapping( const Range<T>& r ) const
{
   if( _min > r._max || _max < r._min )
   {
      return false;
   }
   return true;
}

//------------------------------------------------------------------------------
//! Verifies if the specified value is inside the range.
//! It uses a closed interval in every dimension (i.e. min <= v < max).
template< typename T > inline bool
Range<T>::isInside( T v ) const
{
   return _min <= v && v <= _max;
}

//------------------------------------------------------------------------------
//! Verifies if the specified value is inside the range.
//! It uses a half-open interval in every dimension (i.e. min <= v < max).
template< typename T > inline bool
Range<T>::isInsideCO( T v ) const
{
   return _min <= v && v < _max;
}

//------------------------------------------------------------------------------
//! Verifies if the specified value is inside the range.
//! It uses an open interval in every dimension (i.e. min < v < max).
template< typename T > inline bool
Range<T>::isInsideOO( T v ) const
{
   return _min < v && v < _max;
}

//------------------------------------------------------------------------------
//! Verifies if the specified range is inside another.
//! It uses a closed interval in every dimension (i.e. min <= v <= max).
template< typename T > inline bool
Range<T>::isInside( const Range<T>& r ) const
{
   return _min <= r.min() && r.max() <= _max;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline TextStream&
operator<<
( TextStream& stream, const Range<T>& r )
{
   return stream << "[" << r.min() << "," << r.max << "]";
}


/*==============================================================================
  TYPEDEF
==============================================================================*/

typedef Range< int >    Rangei;
typedef Range< float >  Rangef;
typedef Range< double > Ranged;

NAMESPACE_END

#endif //CGMATH_RANGE_H
