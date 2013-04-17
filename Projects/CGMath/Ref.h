/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef CGMATH_REF_H
#define CGMATH_REF_H

#include <CGMath/Quat.h>

NAMESPACE_BEGIN

/*==============================================================================
  CLASS Ref
==============================================================================*/

//! The referential is an axial system with a position in space.

template< typename T >
class Ref
{

public:

   /*----- static methods -----*/

   inline static Ref identity();
   inline static Ref lookAt( const Vec3<T>& pos, const Vec3<T>& at, const Vec3<T> up );

   /*----- methods -----*/

   template< typename S > Ref( const Ref<S>& ref ):
      _scale( ref.scale() ),
      _pos( ref.position() ),
      _orientation( ref.orientation() )
   {
   }

   Ref() {}
   Ref( const Quat<T>& orientation, const Vec3<T>& pos, T scale );
   Ref( const Quat<T>& orientation, const Vec3<T>& pos );
   explicit Ref( const Vec3<T>& pos );
   explicit Ref( const Quat<T>& orientation );
   explicit Ref( T scale );
   Ref( const Ref<T>& ref );
   ~Ref() {}

   T& scale();
   T scale() const;
   Vec3<T>& position();
   const Vec3<T>& position() const;
   Quat<T>& orientation();
   const Quat<T>& orientation() const;

   void scale( T );
   void position( const Vec3<T>& pos );
   void position( const T& x, const T& y, const T& z );
   void orientation( const Vec3<T>& x, const Vec3<T>& y, const Vec3<T>& z );
   void orientation( const Quat<T>& quat );

   Ref& rotate( const Vec3<T>& p, const Vec3<T>& axis, const T& angle );
   Ref& rotate( const Vec3<T>& p, const Quat<T>& quat );
   Ref& rotate( const Vec3<T>& axis, const T& angle );
   Ref& rotateLocal( const Vec3<T>& axis, const T& angle );
   Ref& rotateLocal( const Quat<T>& q );
   Ref& translate( const Vec3<T>& vec );
   Ref& translateLocal( const Vec3<T>& vec );

   Ref getInversed() const;
   Ref getRotated( const Vec3<T>& p, const Vec3<T>& axis, const T& angle ) const;
   Ref getRotated( const Vec3<T>& p, const Quat<T>& quat ) const;
   Ref getRotated( const Vec3<T>& axis, const T& angle ) const;
   Ref getRotatedLocal( const Vec3<T>& axis, const T& angle ) const;
   Ref getTranslated( const Vec3<T>& vec ) const;
   Ref getTranslatedLocal( const Vec3<T>& vec ) const;

   Ref slerp( const Ref& ref, T a ) const;
   Ref nlerp( const Ref& ref, T a ) const;

   Mat4<T> localToGlobal() const;
   Mat4<T> globalToLocal() const;
   inline Mat4<T> toMatrix() const;

   Ref  operator*( const Ref<T>& ref ) const;

   Ref& operator*=( const Ref<T>& ref );
   Ref& operator=( const Ref<T>& ref );

   bool operator==( const Ref<T>& rhs ) const;
   bool operator!=( const Ref<T>& rhs ) const;

private:

   /*----- data members -----*/

   T       _scale;
   Vec3<T> _pos;
   Quat<T> _orientation;
};

//------------------------------------------------------------------------------
//!
template< typename T > inline
Ref<T>
Ref<T>::identity()
{
   return Ref( Quat<T>::identity(), Vec3<T>::zero() );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline
Ref<T>
Ref<T>::lookAt( const Vec3<T>& pos, const Vec3<T>& at, const Vec3<T> up )
{
   // Calculate the true up vector.
   Vec3<T> z = ( pos - at ).normalize();
   Vec3<T> x = up.cross( z ).normalize();
   Vec3<T> y = z.cross( x ).normalize();

   return Ref<T>( Quat<T>::axes(x, y, z), pos );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline
Ref<T>::Ref( const Quat<T>& orientation, const Vec3<T>& pos, T scale )
   : _scale( scale ), _pos( pos ), _orientation( orientation )
{
}

//------------------------------------------------------------------------------
//!
template< typename T > inline
Ref<T>::Ref( const Quat<T>& orientation, const Vec3<T>& pos )
   : _scale( T(1) ), _pos( pos ), _orientation( orientation )
{
}

//------------------------------------------------------------------------------
//!
template< typename T > inline
Ref<T>::Ref( const Vec3<T>& pos )
   : _scale( T(1) ), _pos( pos ), _orientation( Quat<T>::identity() )
{
}

//------------------------------------------------------------------------------
//!
template< typename T > inline
Ref<T>::Ref( const Quat<T>& orientation ) :
   _scale( T(1) ), _pos( Vec3<T>::zero() ), _orientation( orientation )
{
}

//------------------------------------------------------------------------------
//!
template< typename T > inline
Ref<T>::Ref( T scale ) :
   _scale( scale ), _pos( Vec3<T>::zero() ), _orientation( Quat<T>::identity() )
{
}

//------------------------------------------------------------------------------
//!
template< typename T > inline
Ref<T>::Ref( const Ref<T>& ref )
   : _scale( ref._scale ), _pos( ref._pos ), _orientation( ref._orientation )
{
}

//------------------------------------------------------------------------------
//! 
template< typename T > inline T&
Ref<T>::scale()
{
   return _scale;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline T
Ref<T>::scale() const
{
   return _scale;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec3<T>&
Ref<T>::position()
{
   return _pos;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline const Vec3<T>&
Ref<T>::position() const
{
   return _pos;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Quat<T>&
Ref<T>::orientation()
{
   return _orientation;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline const Quat<T>&
Ref<T>::orientation() const
{
   return _orientation;
}

//------------------------------------------------------------------------------
//! 
template< typename T > inline void
Ref<T>::scale( T s )
{
   _scale = s;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline void
Ref<T>::position( const Vec3<T>& pos )
{
   _pos = pos;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline void
Ref<T>::position( const T& x, const T& y, const T& z )
{
   _pos(0) = x;
   _pos(1) = y;
   _pos(2) = z;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline void
Ref<T>::orientation( const Vec3<T>& x, const Vec3<T>& y, const Vec3<T>& z )
{
   _orientation = Quat<T>::axes( x, y, z );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline void
Ref<T>::orientation( const Quat<T>& orientation )
{
   _orientation = orientation;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Ref<T>&
Ref<T>::rotate( const Vec3<T>& p, const Vec3<T>& axis, const T& angle )
{
   return rotate( p, Quat<T>::axisAngle( axis, angle ) );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Ref<T>&
Ref<T>::rotate( const Vec3<T>& p, const Quat<T>& q )
{
   // rotate the point
   Vec3<T> pos = _pos - p;
   _pos = q.toMatrix() * pos;
   _pos += p;

   // rotate the quaternion
   _orientation = q * _orientation;

   return *this;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Ref<T>&
Ref<T>::rotate( const Vec3<T>& axis, const T& angle )
{
   // create the quaternion
   Quat<T> quat = Quat<T>::axisAngle( axis, angle );

   // rotate the quaternion
   _orientation = quat * _orientation;

   return *this;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Ref<T>&
Ref<T>::rotateLocal( const Vec3<T>& axis, const T& angle )
{
   // create the quaternion
   Quat<T> quat = Quat<T>::axisAngle( axis, angle );

   // rotate the quaternion
   _orientation = _orientation * quat;

   return *this;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Ref<T>&
Ref<T>::rotateLocal( const Quat<T>& q )
{
   // rotate the quaternion
   _orientation = _orientation * q;

   return *this;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Ref<T>&
Ref<T>::translate( const Vec3<T>& vec )
{
   _pos += vec;

   return *this;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Ref<T>&
Ref<T>::translateLocal( const Vec3<T>& vec )
{
   _pos += (_orientation * vec)*_scale;

   return *this;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Ref<T>
Ref<T>::getInversed() const
{
   Quat<T> invq = _orientation.getInversed();
   return Ref<T>( invq, invq * (-_pos / _scale), (T)1 / _scale );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Ref<T>
Ref<T>::getRotated( const Vec3<T>& p, const Vec3<T>& axis, const T& angle ) const
{
   return getRotated( p, Quat<T>::axisAngle( axis, angle ) );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Ref<T>
Ref<T>::getRotated( const Vec3<T>& p, const Quat<T>& q ) const
{
   // rotate the point
   Vec3<T> pos  = _pos - p;
   Vec3<T> pos2 = q.toMatrix() * pos;
   pos2 += p;

   // rotate the quaternion
   return Ref<T>( q*_orientation, pos2, _scale );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Ref<T>
Ref<T>::getRotated( const Vec3<T>& axis, const T& angle ) const
{
   // create the quaternion
   Quat<T> quat = Quat<T>::axisAngle( axis, angle );

   // rotate the quaternion
   return Ref<T>( quat * _orientation, _pos, _scale );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Ref<T>
Ref<T>::getRotatedLocal( const Vec3<T>& axis, const T& angle ) const
{
   // create the quaternion
   Quat<T> quat = Quat<T>::axisAngle( axis, angle );

   // rotate the quaternion
   return Ref<T>( _orientation * quat, _pos, _scale );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Ref<T>
Ref<T>::getTranslated( const Vec3<T>& vec ) const
{
   return Ref<T>( _orientation, _pos + vec, _scale );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Ref<T>
Ref<T>::getTranslatedLocal( const Vec3<T>& vec ) const
{
   return Ref<T>( _orientation, _pos + (_orientation * vec)*_scale, _scale );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Ref<T>
Ref<T>::slerp( const Ref& ref, T a ) const
{
   return Ref<T>(
      _orientation.slerp( ref._orientation, a ),
      (T(1)-a)*_pos   + a * ref._pos,
      (T(1)-a)*_scale + a * ref._scale
   );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Ref<T>
Ref<T>::nlerp( const Ref& ref, T a ) const
{
   return Ref<T>(
      _orientation.nlerp( ref._orientation, a ),
      (T(1)-a)*_pos   + a * ref._pos,
      (T(1)-a)*_scale + a * ref._scale
   );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Mat4<T>
Ref<T>::localToGlobal() const
{
   // calculate coefficients
   T x2 = (_orientation.x() + _orientation.x())*_scale;
   T y2 = (_orientation.y() + _orientation.y())*_scale;
   T z2 = (_orientation.z() + _orientation.z())*_scale;
   T xx = _orientation.x() * x2;
   T xy = _orientation.x() * y2;
   T xz = _orientation.x() * z2;
   T yy = _orientation.y() * y2;
   T yz = _orientation.y() * z2;
   T zz = _orientation.z() * z2;
   T wx = _orientation.w() * x2;
   T wy = _orientation.w() * y2;
   T wz = _orientation.w() * z2;

   return Mat4<T>(
      _scale - ( yy + zz ), xy - wz,             xz + wy,              _pos.x,
      xy + wz,             _scale - ( xx + zz ), yz - wx,              _pos.y,
      xz - wy,             yz + wx,              _scale - ( xx + yy ), _pos.z,
      (T)0,                (T)0,                 (T)0,                 (T)1
   );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Mat4<T>
Ref<T>::globalToLocal() const
{
   Quat<T> q = _orientation.getInversed(); // Should we used conjugate?

   // calculate coefficients
   T s  = (T)1 / _scale;
   T x2 = (q.x() + q.x())*s;
   T y2 = (q.y() + q.y())*s;
   T z2 = (q.z() + q.z())*s;
   T xx = q.x() * x2;
   T xy = q.x() * y2;
   T xz = q.x() * z2;
   T yy = q.y() * y2;
   T yz = q.y() * z2;
   T zz = q.z() * z2;
   T wx = q.w() * x2;
   T wy = q.w() * y2;
   T wz = q.w() * z2;

   return Mat4<T>(
      s  - ( yy + zz ), xy - wz,          xz + wy,          (T)0,
      xy + wz,          s  - ( xx + zz ), yz - wx,          (T)0,
      xz - wy,          yz + wx,          s  - ( xx + yy ), (T)0,
      (T)0,             (T)0,            (T)0,              (T)1
   ).translateBefore( -_pos );
}

//------------------------------------------------------------------------------
//! 
template< typename T > inline Mat4<T>
Ref<T>::toMatrix() const
{
   return localToGlobal();
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Ref<T>
Ref<T>::operator*( const Ref<T>& ref ) const
{
   return Ref<T>( _orientation * ref._orientation,
                  _pos + (_orientation * ref._pos) * _scale,
                  _scale * ref._scale );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Ref<T>&
Ref<T>::operator*=( const Ref<T>& ref )
{
   _orientation *= ref._orientation;
   _pos         += (_orientation * ref._pos) * _scale;
   _scale       *= ref._scale;

   return *this;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Ref<T>&
Ref<T>::operator=( const Ref<T>& ref )
{
   _orientation = ref._orientation;
   _pos         = ref._pos;
   _scale       = ref._scale;

   return *this;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline bool
Ref<T>::operator==( const Ref<T>& rhs ) const
{
   return
      _orientation == rhs._orientation &&
      _pos         == rhs._pos &&
      _scale       == rhs._scale;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline bool
Ref<T>::operator!=( const Ref<T>& rhs ) const
{
   return
      _orientation != rhs._orientation &&
      _pos         != rhs._pos &&
      _scale       != rhs._scale;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline TextStream&
operator<<( TextStream& stream, const Ref<T>& ref )
{
   return stream << "("
                 << ref.scale() << ","
                 << ref.position() << ","
                 << ref.orientation() << ")";
}

//------------------------------------------------------------------------------
//!
template< typename T > inline TextStream&
operator>>( TextStream& stream, Ref<T>& ref )
{
   char c;
   return stream >> c
                 >> ref.scale() >> c
                 >> ref.position() >> c
                 >> ref.orientation() >> c;
}

/*==============================================================================
  TYPEDEF
  ==============================================================================*/

typedef Ref< float >  Reff;
typedef Ref< double > Refd;

NAMESPACE_END

#endif
