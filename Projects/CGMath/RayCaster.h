/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef CGMATH_RAY_CASTER_H
#define CGMATH_RAY_CASTER_H

#include <CGMath/StdDefs.h>

#include <CGMath/CGConst.h>
#include <CGMath/Ray.h>

#include <Base/Dbg/Defs.h>

NAMESPACE_BEGIN

template< typename T >
inline String  toStr( const Vec3<T>& v )
{
   return String("(") + toStr(v.x) + ", " + toStr(v.y) + ", " + toStr(v.z) + ")";
}


namespace RayCaster
{

/*==============================================================================
  CLASS Hit
==============================================================================*/
template< typename T >
class Hit
{
public:

   /*----- methods -----*/

   Hit():
      _pos( (T)0 ),
      _bary( (T)0 ),
      _t( CGConst<T>::infinity() ),
      _tMin( (T)0 ),
      _backFacing( false )
   {}

   /*----- data members -----*/

   Vec3<T>  _pos;
   Vec3<T>  _bary;
   T        _t;
   T        _tMin;
   bool     _backFacing;

}; //class Hit

typedef Hit<float>   Hitf;
typedef Hit<double>  Hitd;

//------------------------------------------------------------------------------
//! Ray-triange intersection.
//! This code uses the algorithm from the "3D Rasterization" tech report:
//!   http://www.vis.uni-stuttgart.de/~dachsbcn/download/3dr_techreport.pdf
//! as well as "Incremental and Hierarchical Hilbert Order Edge Equation Polygon Rasterization":
//!   http://portal.acm.org/citation.cfm?id=383528
//! Notation:
//!   o: The ray's origin.
//!   p: A point on the ray indicating the start of the ray (ray.origin() + ray.direction()*ray.t()).
//!   p0-p1-p2: The triangle's vertices.
//! Notes:
//!  - For edge cases, we decide using X axis first (favoring triangle on the right).
//!    When the edge is horizontal, we use Y axis (favoring triangle above).
//!  - For corner cases, we keep only the triangle which contains +X axis (favoring triangle above).
//!  - We might need to swap some edge ordering if (a x b) * c != (b x c) * a in some corner cases.
//!    In such cases, we'll likely need to flip the edge's endpoints and use -Vi instead.
template< typename T, typename Prec >
bool  hit( const Ray<T>& ray, const Vec3<T>& dxf, const Vec3<T>& dyf, const Vec3<T>& p0f, const Vec3<T>& p1f, const Vec3<T>& p2f, Hit<T>& h )
{
   Vec3<Prec> p0( p0f );
   Vec3<Prec> p1( p1f );
   Vec3<Prec> p2( p2f );
   Vec3<Prec> dx( dxf );
   Vec3<Prec> dy( dyf );

   Vec3<Prec> e0 = p2 - p1;
   Vec3<Prec> e1 = p0 - p2;
   Vec3<Prec> e2 = p1 - p0;

   Vec3<Prec> o  = ray.origin();
   Vec3<Prec> o2 = o * 2.0;
   Vec3<Prec> n0 = CGM::cross( e0, p1+p2-o2 ); // Error in Fig.10?  Normals point outward, but should point inward.
   Vec3<Prec> n1 = CGM::cross( e1, p0+p2-o2 );
   Vec3<Prec> n2 = CGM::cross( e2, p1+p0-o2 );

   Vec3<Prec> d = ray.direction();
   Prec V       = CGM::dot( n0, p0-o );
   Prec V0      = CGM::dot( n0, d );
   Prec V1      = CGM::dot( n1, d );
   Prec V2      = CGM::dot( n2, d );
   Prec V012    = V0 + V1 + V2;

   // Some signness bitfields.
   // They are each 2 bits: bits[1:0] = { eqz, ltz }.
   uint8_t sb_V, sb_V0, sb_V1, sb_V2;
   sb_V  = (V  == 0.0) ? 0x1 : 0x0; sb_V  <<= 1; sb_V  |= (V  <  0.0) ? 0x1 : 0x0;
   sb_V0 = (V0 == 0.0) ? 0x1 : 0x0; sb_V0 <<= 1; sb_V0 |= (V0 <  0.0) ? 0x1 : 0x0;
   sb_V1 = (V1 == 0.0) ? 0x1 : 0x0; sb_V1 <<= 1; sb_V1 |= (V1 <  0.0) ? 0x1 : 0x0;
   sb_V2 = (V2 == 0.0) ? 0x1 : 0x0; sb_V2 <<= 1; sb_V2 |= (V2 <  0.0) ? 0x1 : 0x0;

   bool        backFacing;
   Vec3<Prec>  pos;
   Vec3<Prec>  bary;
   Prec        t;

   switch( sb_V )
   {
      //=============================================================================
      // Front facing
      //=============================================================================
      case 0x0: // V > 0
      {
         backFacing = false;

         // If any volume is of the other signness (Vi < 0), we're outside.
         if( ((sb_V0 | sb_V1 | sb_V2) & 0x1) != 0x0 )  return false;

         // Gather all of the 'eqz' bits in one field.
         uint8_t zb = (sb_V2 >> 1);
         zb <<= 1;
         zb |= (sb_V1 >> 1);
         zb <<= 1;
         zb |= (sb_V0 >> 1);

         switch( zb )
         {
            case 0:  // V0 != 0, V1 != 0, V2 != 0.
            {
               // Interior case.
               // Compute I,J,K.
               bary(0) = V0 / V012;
               bary(1) = V1 / V012;
               bary(2) = V2 / V012;
               pos = p0*bary(0) + p1*bary(1) + p2*bary(2);
            }  break;
            case 1:  // V0 == 0, V1 != 0, V2 != 0.
            {
               // Single edge case.
               Prec vix = CGM::dot( n0, dx );
               if( vix <  0.0 )  return false;
               else
               if( vix == 0.0 )
               {
                  // Use the vertical component of the perpendicular direction.
                  Prec viy = CGM::dot( n0, dy );
                  if( viy < 0.0 )  return false;
                  CHECK( viy != 0.0 );  // Should never happen (another V* should be zero).
                  // Compute I,J,K (below).
               }
               //else
               //( vix >  T(0) )
                  // Compute I,J,K (below).

               bary(0) = 0.0;
               bary(1) = V1 / V012;
               bary(2) = V2 / V012;
               pos = p1*bary(1) + p2*bary(2);
            }  break;
            case 2:  // V0 != 0, V1 == 0, V2 != 0.
            {
               // Single edge case.
               Prec vix = CGM::dot( n1, dx );
               if( vix <  0.0 )  return false;
               else
               if( vix == 0.0 )
               {
                  // Use the vertical component of the perpendicular direction.
                  Prec viy = CGM::dot( n1, dy );
                  if( viy < 0.0 )  return false;
                  CHECK( viy != 0.0 );  // Should never happen (another V* should be zero).
                  // Compute I,J,K (below).
               }
               //else
               //( vix >  T(0) )
                  // Compute I,J,K (below).

               bary(0) = V0 / V012;
               bary(1) = 0.0;
               bary(2) = V2 / V012;
               pos = p0*bary(0) + p2*bary(2);
            }  break;
            case 3:  // V0 == 0, V1 == 0, V2 != 0.
            {
               // Corner case.
               Prec vix = CGM::dot( n0, dx );
               Prec vjx = CGM::dot( n1, dx );
               if( vix <= 0.0 || vjx < 0.0 )  return false;

               bary(0) = 0.0;
               bary(1) = 0.0;
               bary(2) = 1.0;
               pos = p2;
            }  break;
            case 4:  // V0 != 0, V1 != 0, V2 == 0.
            {
               // Single edge case.
               Prec vix = CGM::dot( n2, dx );
               if( vix <  0.0 )  return false;
               else
               if( vix == 0.0 )
               {
                  // Use the vertical component of the perpendicular direction.
                  Prec viy = CGM::dot( n2, dy );
                  if( viy < 0.0 )  return false;
                  CHECK( viy != 0.0 );  // Should never happen (another V* should be zero).
                  // Compute I,J,K (below).
               }
               //else
               //( vix >  T(0) )
                  // Compute I,J,K (below).

               bary(0) = V0 / V012;
               bary(1) = V1 / V012;
               bary(2) = 0.0;
               pos = p0*bary(0) + p1*bary(1);
            }  break;
            case 5:  // V0 == 0, V1 != 0, V2 == 0.
            {
               // Corner case.
               Prec vix = CGM::dot( n2, dx );
               Prec vjx = CGM::dot( n0, dx );
               if( vix <= 0.0 || vjx < 0.0 )  return false;

               bary(0) = 0.0;
               bary(1) = 1.0;
               bary(2) = 0.0;
               pos = p1;
            }  break;
            case 6:  // V0 != 0, V1 == 0, V2 == 0.
            {
               // Corner case.
               Prec vix = CGM::dot( n1, dx );
               Prec vjx = CGM::dot( n2, dx );
               if( vix <= 0.0 || vjx < 0.0 )  return false;

               bary(0) = 1.0;
               bary(1) = 0.0;
               bary(2) = 0.0;
               pos = p0;
            }  break;
            case 7:  // V0 == 0, V1 == 0, V2 == 0.
            {
               // Degenerate triangle case.
               return false;
            }
            default:
            {
               // Impossible.
               CHECK( false );
               return false;
            }
         } // switch( zb )
      }  break;

      //=============================================================================
      // Back facing
      //=============================================================================
      case 0x1: // V < 0
      {
         backFacing = true;

         // If any volume is of the other signness (Vi > 0), we're outside.
         if( ( ((sb_V0>>1|sb_V0) & (sb_V1>>1|sb_V1) & (sb_V2>>1|sb_V2)) & 0x1 ) == 0x0 )  return false;

         // Gather all of the 'eqz' bits in one field.
         uint8_t zb = (sb_V2 >> 1);
         zb <<= 1;
         zb |= (sb_V1 >> 1);
         zb <<= 1;
         zb |= (sb_V0 >> 1);

         switch( zb )
         {
            case 0:  // V0 != 0, V1 != 0, V2 != 0.
            {
               // Interior case.
               // Compute I,J,K (below).
               bary(0) = V0 / V012;
               bary(1) = V1 / V012;
               bary(2) = V2 / V012;
               pos = p0*bary(0) + p1*bary(1) + p2*bary(2);
            }  break;
            case 1:  // V0 == 0, V1 != 0, V2 != 0.
            {
               // Single edge case.
               Prec vix = -CGM::dot( n0, dx );
               if( vix <  0.0 )  return false;
               else
               if( vix == 0.0 )
               {
                  // Use the vertical component of the perpendicular direction.
                  Prec viy = -CGM::dot( n0, dy );
                  if( viy < 0.0 )  return false;
                  CHECK( viy != 0.0 );  // Should never happen (another V* should be zero).
                  // Compute I,J,K (below).
               }
               //else
               //( vix >  T(0) )
                  // Compute I,J,K (below).

               bary(0) = 0.0;
               bary(1) = V1 / V012;
               bary(2) = V2 / V012;
               pos = p1*bary(1) + p2*bary(2);
            }  break;
            case 2:  // V0 != 0, V1 == 0, V2 != 0.
            {
               // Single edge case.
               Prec vix = -CGM::dot( n1, dx );
               if( vix <  0.0 )  return false;
               else
               if( vix == 0.0 )
               {
                  // Use the vertical component of the perpendicular direction.
                  Prec viy = -CGM::dot( n1, dy );
                  if( viy < 0.0 )  return false;
                  CHECK( viy != 0.0 );  // Should never happen (another V* should be zero).
                  // Compute I,J,K (below).
               }
               //else
               //( vix >  T(0) )
                  // Compute I,J,K (below).

               bary(0) = V0 / V012;
               bary(1) = 0.0;
               bary(2) = V2 / V012;
               pos = p0*bary(0) + p2*bary(2);
            }  break;
            case 3:  // V0 == 0, V1 == 0, V2 != 0.
            {
               // Corner case.
               Prec vix = -CGM::dot( n1, dx );
               Prec vjx = -CGM::dot( n0, dx );
               if( vix <= 0.0 || vjx < 0.0 )  return false;

               bary(0) = 0.0;
               bary(1) = 0.0;
               bary(2) = 1.0;
               pos = p2;
            }  break;
            case 4:  // V0 != 0, V1 != 0, V2 == 0.
            {
               // Single edge case.
               Prec vix = -CGM::dot( n2, dx );
               if( vix <  0.0 )  return false;
               else
               if( vix == 0.0 )
               {
                  // Use the vertical component of the perpendicular direction.
                  Prec viy = -CGM::dot( n2, dy );
                  if( viy < 0.0 )  return false;
                  CHECK( viy != 0.0 );  // Should never happen (another V* should be zero).
                  // Compute I,J,K (below).
               }
               //else
               //( vix >  T(0) )
                  // Compute I,J,K (below).

               bary(0) = V0 / V012;
               bary(1) = V1 / V012;
               bary(2) = 0.0;
               pos = p0*bary(0) + p1*bary(1);
            }  break;
            case 5:  // V0 == 0, V1 != 0, V2 == 0.
            {
               // Corner case.
               Prec vix = -CGM::dot( n0, dx );
               Prec vjx = -CGM::dot( n2, dx );
               if( vix <= 0.0 || vjx < 0.0 )  return false;

               bary(0) = 0.0;
               bary(1) = 1.0;
               bary(2) = 0.0;
               pos = p1;
            }  break;
            case 6:  // V0 != 0, V1 == 0, V2 == 0.
            {
               // Corner case.
               Prec vix = -CGM::dot( n2, dx );
               Prec vjx = -CGM::dot( n1, dx );
               if( vix <= 0.0 || vjx < 0.0 )  return false;

               bary(0) = 1.0;
               bary(1) = 0.0;
               bary(2) = 0.0;
               pos = p0;
            }  break;
            case 7:  // V0 == 0, V1 == 0, V2 == 0.
            {
               // Degenerate triangle case.
               return false;
            }
            default:
            {
               // Impossible.
               CHECK( false );
               return false;
            }
         } // switch( zb )
      }  break;

      //=============================================================================
      // Degenerate case (ray's origin lying on triangle).
      //=============================================================================
      case 0x2: // V == 0
      {
         // FIXME: We need to dissociate two cases: lying on triangle and zero projected area.
         return false;

         // Degenerate case where the ray's origin lies directly on the triangle.
         // Just compensate by throwing a ray from further back, and adjust the resulting 't' parameter.
         Ray<T> newRay( o-d, d );
         if( hit( newRay, p0f, p1f, p2f, h ) )
         {
            h._t = 0.0f;
            return true;
         }
         else
         {
            return false;
         }
      }  break;

      default:
      {
         // Invalid result.
         CHECK( false );
         return false;
      }
   } // switch( sb_V )

   t = V / V012;
   if( t < h._tMin || t > h._t ) return false;

   // Assign values.
   h._t          = T(t);
   h._bary       = bary;
   h._backFacing = backFacing;
   h._pos        = pos;

   return true;
}

template< typename T >
inline bool  hitf( const Ray<T>& ray, const Vec3<T>& dxf, const Vec3<T>& dyf, const Vec3<T>& p0f, const Vec3<T>& p1f, const Vec3<T>& p2f, Hit<T>& h )
{
   return hit<T,float>( ray, dxf, dyf, p0f, p1f, p2f, h );
}

template< typename T >
inline bool  hitd( const Ray<T>& ray, const Vec3<T>& dxf, const Vec3<T>& dyf, const Vec3<T>& p0f, const Vec3<T>& p1f, const Vec3<T>& p2f, Hit<T>& h )
{
   return hit<T,double>( ray, dxf, dyf, p0f, p1f, p2f, h );
}

template< typename T >
inline bool  hit( const Ray<T>& ray, const Vec3<T>& dxf, const Vec3<T>& dyf, const Vec3<T>& p0f, const Vec3<T>& p1f, const Vec3<T>& p2f, Hit<T>& h )
{
   return hit<T,double>( ray, dxf, dyf, p0f, p1f, p2f, h );
}

template< typename T >
bool  hitf( const Ray<T>& ray, const Vec3<T>& p0, const Vec3<T>& p1, const Vec3<T>& p2, Hit<T>& h )
{
   Vec3<T> dx = Vec3<T>::perpendicular( ray.direction() );
   Vec3<T> dy = CGM::cross( dx, ray.direction() );
   return hitf( ray, dx, dy, p0, p1, p2, h );
}

template< typename T >
bool  hitd( const Ray<T>& ray, const Vec3<T>& p0, const Vec3<T>& p1, const Vec3<T>& p2, Hit<T>& h )
{
   Vec3<T> dx = Vec3<T>::perpendicular( ray.direction() );
   Vec3<T> dy = CGM::cross( dx, ray.direction() );
   return hitd( ray, dx, dy, p0, p1, p2, h );
}

template< typename T >
bool  hit( const Ray<T>& ray, const Vec3<T>& p0, const Vec3<T>& p1, const Vec3<T>& p2, Hit<T>& h )
{
   Vec3<T> dx = Vec3<T>::perpendicular( ray.direction() );
   Vec3<T> dy = CGM::cross( dx, ray.direction() );
   return hit( ray, dx, dy, p0, p1, p2, h );
}

} //namespace RayCaster


NAMESPACE_END

#endif //CGMATH_RAY_CASTER_H
