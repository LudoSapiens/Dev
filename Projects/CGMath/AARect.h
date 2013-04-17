/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef CGMATH_AARECT_H
#define CGMATH_AARECT_H

#include <CGMath/StdDefs.h>

#include <CGMath/Vec2.h>

#include <Base/IO/TextStream.h>

NAMESPACE_BEGIN

/*==============================================================================
  CLASS AARect
==============================================================================*/

//! Rectangle class.

template< typename T >
class AARect
{
public:

   /*----- static methods -----*/

   inline static AARect<T> empty();

   /*----- methods -----*/

   inline AARect();
   inline AARect( T r );
   inline AARect( const Vec2<T>& size );
   inline AARect( const Vec2<T>& pos, const Vec2<T>& size );
   inline AARect( T x0, T x1, T y0, T y1 );

   // Public methods
   inline void set( T r );
   inline void set( const Vec2<T>& size );
   inline void set( const Vec2<T>& pos, const Vec2<T>& size );
   inline void set( T x0, T x1, T y0, T y1 );

   inline void operator|=( const Vec2<T>& pt );
   inline void operator|=( const AARect<T>& rect );
   inline void grow( T v );
   inline void grow( const Vec2<T>& v );
   inline void shrink( T v );
   inline void shrink( const Vec2<T>& v );

   inline void scaleAndBias( const Vec2<T>& s, const Vec2<T>& t );
   inline void scale( const Vec2<T>& v );
   inline void translate( const Vec2<T>& v );

   inline const Vec2<T>& slabX() const;
   inline const Vec2<T>& slabY() const;
   inline const Vec2<T>& slab( uint axis ) const;

   inline Vec2<T>& slabX();
   inline Vec2<T>& slabY();
   inline Vec2<T>& slab( uint axis );

   inline T min( uint axis ) const;
   inline T max( uint axis ) const;

   inline T& min( uint axis );
   inline T& max( uint axis );

   inline Vec2<T> size() const;
   inline T size( uint axis ) const;
   inline T maxSize() const;

   inline T width() const;
   inline T height() const;
   inline T area() const;

   inline Vec2<T> position() const;
   inline Vec2<T> corner( int x, int y ) const;
   inline Vec2<T> corner( int corner ) const;

   inline T left() const;
   inline T right() const;
   inline T bottom() const;
   inline T top() const;
   inline AARect& left( T v );
   inline AARect& right( T v );
   inline AARect& bottom( T v );
   inline AARect& top( T v );

   inline Vec2<T> center() const;
   inline T center( uint axis ) const;

   inline bool isEmpty() const;

   inline bool isOverlapping( const AARect<T>& rect ) const;
   inline bool isInside( const Vec2<T>& pt ) const;
   inline bool isInsideCO( const Vec2<T>& pt ) const;
   inline bool isInsideOO( const Vec2<T>& pt ) const;
   inline bool isInside( const AARect<T>& rect ) const;

   inline uint longestSide() const;

   inline bool operator==( const AARect<T>& rect ) const;
   inline bool operator!=( const AARect<T>& rect ) const;

private:

   /*----- data members -----*/

   Vec2<T> _s[2];
};

//------------------------------------------------------------------------------
//!
template< typename T > inline AARect<T>
AARect<T>::empty()
{
   return AARect<T>(
      CGConst<T>::highest(), CGConst<T>::lowest(),
      CGConst<T>::highest(), CGConst<T>::lowest()
   );
}

//------------------------------------------------------------------------------
//!highest
template< typename T > inline
AARect<T>::AARect() {}

//------------------------------------------------------------------------------
//!
template< typename T > inline
AARect<T>::AARect( T r )
{
   set(r);
}

//------------------------------------------------------------------------------
//!
template< typename T > inline
AARect<T>::AARect( const Vec2<T>& size )
{
   set(size);
}

//------------------------------------------------------------------------------
//!
template< typename T > inline
AARect<T>::AARect( const Vec2<T>& pos, const Vec2<T>& size )
{
   set(pos, size);
}

//------------------------------------------------------------------------------
//!
template< typename T >  inline
AARect<T>::AARect( T x0, T x1, T y0, T y1 )
{
   set(x0, x1, y0, y1);
}

//------------------------------------------------------------------------------
//!
template< typename T > inline void
AARect<T>::set( T r )
{
   _s[0](0) = -r;
   _s[0](1) =  r;
   _s[1](0) = -r;
   _s[1](1) =  r;
}

//------------------------------------------------------------------------------
//! Specifies a rectangle of the specified size positioned at the origin.
template< typename T > inline void
AARect<T>::set( const Vec2<T>& size )
{
   _s[0](0) = 0;
   _s[0](1) = size.x;
   _s[1](0) = 0;
   _s[1](1) = size.y;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline void
AARect<T>::set( const Vec2<T>& pos, const Vec2<T>& size )
{
   _s[0](0) = pos.x;
   _s[0](1) = pos.x + size.x;
   _s[1](0) = pos.y;
   _s[1](1) = pos.y + size.y;
}

//------------------------------------------------------------------------------
//!
template< typename T >  inline void
AARect<T>::set( T x0, T x1, T y0, T y1 )
{
   _s[0](0) = x0;
   _s[0](1) = x1;
   _s[1](0) = y0;
   _s[1](1) = y1;
}

//------------------------------------------------------------------------------
//! Add a point to the rectangle.
template< typename T > inline void
AARect<T>::operator|=( const Vec2<T>& point )
{
#if 1
   _s[0](0) = CGM::min( _s[0](0), point(0) );
   _s[0](1) = CGM::max( _s[0](1), point(0) );

   _s[1](0) = CGM::min( _s[1](0), point(1) );
   _s[1](1) = CGM::max( _s[1](1), point(1) );
#else
   if( point(0) < _s[0](0) )
   {
      _s[0](0) = point(0);
   }
   if( point(0) > _s[0](1) )
   {
      _s[0](1) = point(0);
   }

   if( point(1) < _s[1](0) )
   {
      _s[1](0) = point(1);
   }
   if( point(1) > _s[1](1) )
   {
      _s[1](1) = point(1);
   }
#endif
}

//------------------------------------------------------------------------------
//! Add a rectangle to the current one.
template< typename T > inline void
AARect<T>::operator|=( const AARect<T>& rect )
{
#if 1
   _s[0](0) = CGM::min( _s[0](0), rect._s[0](0) );
   _s[0](1) = CGM::max( _s[0](1), rect._s[0](1) );

   _s[1](0) = CGM::min( _s[1](0), rect._s[1](0) );
   _s[1](1) = CGM::max( _s[1](1), rect._s[1](1) );
#else
   if( rect._s[0](0) < _s[0](0) )
   {
      _s[0](0) = rect._s[0](0);
   }
   if( rect._s[0](1) > _s[0](1) )
   {
      _s[0](1) = rect._s[0](1);
   }

   if( rect._s[1](0) < _s[1](0) )
   {
      _s[1](0) = rect._s[1](0);
   }
   if( rect._s[1](1) > _s[1](1) )
   {
      _s[1](1) = rect._s[1](1);
   }
#endif
}

//------------------------------------------------------------------------------
//!
template< typename T > inline void
AARect<T>::grow( T v )
{
   _s[0](0) -= v;
   _s[1](0) -= v;
   _s[0](1) += v;
   _s[1](1) += v;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline void
AARect<T>::grow( const Vec2<T>& v )
{
   _s[0](0) -= v.x;
   _s[1](0) -= v.y;
   _s[0](1) += v.x;
   _s[1](1) += v.y;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline void
AARect<T>::shrink( T v )
{
   _s[0](0) += v;
   _s[1](0) += v;
   _s[0](1) -= v;
   _s[1](1) -= v;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline void
AARect<T>::shrink( const Vec2<T>& v )
{
   _s[0](0) += v.x;
   _s[1](0) += v.y;
   _s[0](1) -= v.x;
   _s[1](1) -= v.y;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline void
AARect<T>::scale( const Vec2<T>& v )
{
   _s[0](0) *= v.x;
   _s[1](0) *= v.y;
   _s[0](1) *= v.x;
   _s[1](1) *= v.y;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline void
AARect<T>::translate( const Vec2<T>& v )
{
   _s[0](0) += v.x;
   _s[1](0) += v.y;
   _s[0](1) += v.x;
   _s[1](1) += v.y;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline void
AARect<T>::scaleAndBias( const Vec2<T>& s, const Vec2<T>& t )
{
   _s[0](0) = _s[0](0)*s.x + t.x;
   _s[1](0) = _s[1](0)*s.y + t.y;
   _s[0](1) = _s[0](1)*s.x + t.x;
   _s[1](1) = _s[1](1)*s.y + t.y;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline const Vec2<T>&
AARect<T>::slabX() const
{
   return _s[0];
}

//------------------------------------------------------------------------------
//!
template< typename T > inline const Vec2<T>&
AARect<T>::slabY() const
{
   return _s[1];
}

//------------------------------------------------------------------------------
//!
template< typename T > inline const Vec2<T>&
AARect<T>::slab( uint axis ) const
{
   return _s[axis];
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec2<T>&
AARect<T>::slabX()
{
   return _s[0];
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec2<T>&
AARect<T>::slabY()
{
   return _s[1];
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec2<T>&
AARect<T>::slab( uint axis )
{
   return _s[axis];
}

//------------------------------------------------------------------------------
//!
template< typename T > inline T
AARect<T>::min( uint axis ) const
{
   return _s[axis](0);
}

//------------------------------------------------------------------------------
//!
template< typename T > inline T
AARect<T>::max( uint axis ) const
{
   return _s[axis](1);
}

//------------------------------------------------------------------------------
//!
template< typename T > inline T&
AARect<T>::min( uint axis )
{
   return _s[axis](0);
}

//------------------------------------------------------------------------------
//!
template< typename T > inline T&
AARect<T>::max( uint axis )
{
   return _s[axis](1);
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec2<T>
AARect<T>::size() const
{
   return Vec2<T>( size(0), size(1) );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline T
AARect<T>::size( uint axis ) const
{
   return _s[axis](1) - _s[axis](0);
}

//------------------------------------------------------------------------------
//!
template< typename T > inline T
AARect<T>::maxSize() const
{
   return CGM::max( size(0), size(1) );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline T
AARect<T>::width() const
{
   return size(0);
}

//------------------------------------------------------------------------------
//!
template< typename T > inline T
AARect<T>::height() const
{
   return size(1);
}

//------------------------------------------------------------------------------
//!
template< typename T > inline T
AARect<T>::area() const
{
   return width() * height();
}

//------------------------------------------------------------------------------
//! Return the asked corner
template< typename T > inline Vec2<T>
AARect<T>::position() const
{
   return Vec2<T>( _s[0](0), _s[1](0) );
}

//------------------------------------------------------------------------------
//! Return the asked corner
template< typename T > inline Vec2<T>
AARect<T>::corner( int x, int y ) const
{
   return Vec2<T>( _s[0](x), _s[1](y) );
}

//------------------------------------------------------------------------------
//! Return the asked corner
template< typename T > inline Vec2<T>
AARect<T>::corner( int corner ) const
{
   return Vec2<T>(
      _s[0]( corner & 1 ),
      _s[1]( (corner>>1) & 1)
   );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline T
AARect<T>::left() const
{
   return _s[0](0);
}

//------------------------------------------------------------------------------
//!
template< typename T > inline T
AARect<T>::right() const
{
   return _s[0](1);
}

//------------------------------------------------------------------------------
//!
template< typename T > inline T
AARect<T>::bottom() const
{
   return _s[1](0);
}

//------------------------------------------------------------------------------
//!
template< typename T > inline T
AARect<T>::top() const
{
   return _s[1](1);
}

//------------------------------------------------------------------------------
//!
template< typename T > inline AARect<T>&
AARect<T>::left( T v )
{
   _s[0](0) = v;
   return *this;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline AARect<T>&
AARect<T>::right( T v )
{
   _s[0](1) = v;
   return *this;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline AARect<T>&
AARect<T>::bottom( T v )
{
   _s[1](0) = v;
   return *this;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline AARect<T>&
AARect<T>::top( T v )
{
   _s[1](1) = v;
   return *this;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec2<T>
AARect<T>::center() const
{
   return Vec2<T>(
      ( _s[0](0) + _s[0](1) ) / (T)2,
      ( _s[1](0) + _s[1](1) ) / (T)2
   );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline T
AARect<T>::center( uint axis ) const
{
   return ( _s[axis](0) + _s[axis](1) ) / (T)2;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline bool
AARect<T>::isEmpty() const
{
   return (_s[0](1) < _s[0](0)) || (_s[1](1) < _s[1](0));
}

//------------------------------------------------------------------------------
//!
template< typename T > inline bool
AARect<T>::isOverlapping( const AARect<T>& rect ) const
{
   if( _s[0](0) > rect._s[0](1) || _s[0](1) < rect._s[0](0) )
   {
      return false;
   }
   if( _s[1](0) > rect._s[1](1) || _s[1](1) < rect._s[1](0) )
   {
      return false;
   }
   return true;
}

//------------------------------------------------------------------------------
//! Verifies if the specified point is inside the rectangle.
//! It uses a closed interval in every dimension (i.e. min <= v < max).
template< typename T > inline bool
AARect<T>::isInside( const Vec2<T>& v ) const
{
   return _s[0](0) <= v(0) && v(0) <= _s[0](1) &&
          _s[1](0) <= v(1) && v(1) <= _s[1](1);
}

//------------------------------------------------------------------------------
//! Verifies if the specified point is inside the rectangle.
//! It uses a half-open interval in every dimension (i.e. min <= v < max).
template< typename T > inline bool
AARect<T>::isInsideCO( const Vec2<T>& v ) const
{
   return _s[0](0) <= v(0) && v(0) < _s[0](1) &&
          _s[1](0) <= v(1) && v(1) < _s[1](1);
}

//------------------------------------------------------------------------------
//! Verifies if the specified point is inside the rectangle.
//! It uses an open interval in every dimension (i.e. min < v < max).
template< typename T > inline bool
AARect<T>::isInsideOO( const Vec2<T>& v ) const
{
   return _s[0](0) < v(0) && v(0) < _s[0](1) &&
          _s[1](0) < v(1) && v(1) < _s[1](1);
}

//------------------------------------------------------------------------------
//! Verifies if the specified rectangle (rect) is inside this rectangle.
//! It uses a closed interval in every dimension (i.e. min <= v <= max).
template< typename T > inline bool
AARect<T>::isInside( const AARect<T>& rect ) const
{
   return _s[0](0) <= rect.min(0) && rect.max(0) <= _s[0](1) &&
          _s[1](0) <= rect.min(1) && rect.max(1) <= _s[1](1);
}

//------------------------------------------------------------------------------
//!
template< typename T > inline uint
AARect<T>::longestSide() const
{
   T x = _s[0](1) - _s[0](0);
   T y = _s[1](1) - _s[1](0);

   return (y > x) ? 1 : 0;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline bool
AARect<T>::operator==( const AARect<T>& rect ) const
{
   return _s[0] == rect._s[0] && _s[1] == rect._s[1];
}

//------------------------------------------------------------------------------
//!
template< typename T > inline bool
AARect<T>::operator!=( const AARect<T>& rect ) const
{
   return _s[0] != rect._s[0] || _s[1] != rect._s[1];
}

////------------------------------------------------------------------------------
////!
//template< typename T > inline TextStream&
//operator<<
//( TextStream& stream, const AARect<T>& box )
//{
//   return stream << box.corner(0) << " - " << box.corner(7);
//}

//------------------------------------------------------------------------------
//!
template< typename T > inline TextStream&
operator<<
( TextStream& stream, const AARect<T>& rect )
{
   return stream << "[" << rect.width() << "x" << rect.height() << "]@" << rect.position();
}


/*==============================================================================
  TYPEDEF
==============================================================================*/

typedef AARect< int >    AARecti;
typedef AARect< float >  AARectf;
typedef AARect< double > AARectd;

NAMESPACE_END

#endif //CGMATH_AARECT_H
