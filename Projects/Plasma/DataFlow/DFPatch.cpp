/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/DataFlow/DFPatch.h>

/*==============================================================================
  UNNAME NAMESPACE
==============================================================================*/

UNNAMESPACE_BEGIN


UNNAMESPACE_END

NAMESPACE_BEGIN

/*==============================================================================
   CLASS DFPatch
==============================================================================*/

//------------------------------------------------------------------------------
//! 
void 
DFPatch::init( const DFGeometry& geom, const DFGeometry::Patch& p )
{
   const uint MAX_VALENCE = 64;
   Vec3f f[4][MAX_VALENCE];
   Vec3f vring[MAX_VALENCE];
   Vec3f vdiag[MAX_VALENCE];
   float creases[MAX_VALENCE];
   int creasesID[MAX_VALENCE];
   Vec3f vlim[4];
   Vec3f t0[4];
   Vec3f t1[4];
   float cn[4];
   float sn[4];
   int val[4];

   // Compute per vertex parameters.
   for( uint c = 0; c < 4; ++c )
   {
      // Retrieve ring vertices and compute valence.
      int n                       = 0;
      int cc                      = c;
      int crnum                   = 0;
      const DFGeometry::Patch* cp = &p;
      do {
         if( cp->crease( cc ) == 0 )
         {
            creases[n] = 2.0f/3.0f;
         }
         else
         {
            creasesID[crnum++] = n;
            creases[n] = 0.0f;
         }
         vring[n]   = geom.controlPoint( cp->_controlPts[(cc+1)%4] );
         vdiag[n++] = geom.controlPoint( cp->_controlPts[(cc+2)%4] );
         int ne     = (cc+3)%4;
         cc         = cp->neighborEdge( ne );
         cp         = &geom.patch( cp->neighborPatch( ne ) );
      } while( cp != &p );

      val[c]         = n;
      const Vec3f& v = geom.controlPoint( p._controlPts[c] );

      if( crnum == 0 )
      {
         // Compute limite vertex and face vertex.
         vlim[c] = Vec3f(0.0f);
         for( int i = 0; i < n; ++i )
         {
            // Face vertex.
            f[c][i] = v*(4.0f/9.0f) + (vring[i]+vring[(i+1)%n])*(2.0f/9.0f) + vdiag[i]*(1.0f/9.0f);

            // Corner vertex.
            vlim[c] += f[c][i];
         }
         vlim[c] = ( vlim[c]*9.0f/(float)n + v*(float(n)-4.0f) )*(1.0f/(float(n)+5.0f));
            
         // Compute t0 and t1.
         Vec3f e0(0.0f);
         Vec3f e1(0.0f);
         for( int i = 0; i < n; ++i )
         {
            float angle = CGConstf::pi2()*(float)i/float(n); 
            Vec3f e     = (f[c][i]+f[c][(i-1+n)%n])*0.5f;
            e0         += e * CGM::cos(angle);
            e1         += e * CGM::sin(angle);
         }
         cn[c]        = CGM::cos( CGConstf::pi2()/float(n) );
         sn[c]        = CGM::sin( CGConstf::pi2()/float(n) );
         float lambda = n == 4 ? 0.5f : ( cn[c] + 5.0f + CGM::sqrt((cn[c]+9.0f)*(cn[c]+1.0f)) ) / 16.0f;
         float k      = 1.0f/(lambda*float(n));
         e0          *= k;
         e1          *= k;
         t0[c]        = vlim[c] + e0;
         t1[c]        = vlim[c] + e0*cn[c] + e1*sn[c];
      }
      else
      {
         // Face and corner vertices.
         vlim[c] = Vec3f(0.0f);
         for( int i = 0; i < n; ++i )
         {
            // Face vertex.
            f[c][i] = v*(4.0f/9.0f) + (vring[i]+vring[(i+1)%n])*(2.0f/9.0f) + vdiag[i]*(1.0f/9.0f);
            
            // Corner vertex.
            vlim[c] += v*float(n) + vring[i]*4.0f + vdiag[i];
         }
         if( crnum < 2 )
         {
            vlim[c] = vlim[c] / float(n*n + 5*n);
         }
         else if( crnum == 2 )
         {
            vlim[c] = (v*4.0f + vring[creasesID[0]] + vring[creasesID[1]])/6.0f;
         }
         else
         {
            vlim[c] = v;
         }

         // Edge vertices.
         if( creases[0] == 0.0f )
         {
            t0[c] = (v*2.0f + vring[0])/3.0f;
         }
         else
         {
            t0[c] = (f[c][0] + f[c][n-1])*0.5f;
         }
         if( creases[1] == 0.0f )
         {
            t1[c] = (v*2.0f + vring[1])/3.0f;
         }
         else
         {
            t1[c] = (f[c][0] + f[c][1])*0.5f;
         }

         cn[c] = CGM::cos( CGConstf::pi2()/float(n) );
         sn[c] = CGM::sin( CGConstf::pi2()/float(n) );
      }
   }

   // Compute per patch parameters.
   _b[0]  = vlim[0]; _b[1]  = t0[0];   _b[2]  = t1[1];   _b[3]  = vlim[1];
   _b[4]  = t1[0];   _b[5]  = f[0][0]; _b[6]  = f[1][0]; _b[7]  = t0[1];
   _b[8]  = t0[3];   _b[9]  = f[3][0]; _b[10] = f[2][0]; _b[11] = t1[2];
   _b[12] = vlim[3]; _b[13] = t1[3];   _b[14] = t0[2];   _b[15] = vlim[2];
}

//------------------------------------------------------------------------------
//! 
void 
DFPatch::parameters( const Vec2f& uv, Vec3f& pos, Vec3f& normal ) const
{
   evalCubic( uv, pos, normal );
}

//------------------------------------------------------------------------------
//! 
const Vec3f& 
DFPatch::position( uint i ) const
{
   static int reg[4]  = {0, 3, 15, 12 };
   return _b[reg[i]];
}

//------------------------------------------------------------------------------
//! 
void 
DFPatch::evalCubic( const Vec2f& uv, Vec3f& pos, Vec3f& normal ) const
{
   float um1 = (1.0f-uv.x);
   float um2 = um1*um1;
   float um3 = um2*um1;

   float u1 = uv.x;
   float u2 = u1*u1;
   float u3 = u2*u1;

   float vm1 = (1.0f-uv.y);
   float vm2 = vm1*vm1;
   float vm3 = vm2*vm1;

   float v1 = uv.y;
   float v2 = v1*v1;
   float v3 = v2*v1;

   float uc[4];
   uc[0] = um3;
   uc[1] = um2*u1*3.0f;
   uc[2] = um1*u2*3.0f;
   uc[3] = u3;

   float vc[4];
   vc[0] = vm3;
   vc[1] = vm2*v1*3.0f;
   vc[2] = vm1*v2*3.0f;
   vc[3] = v3;

   float ud[4];
   ud[0] = um2*-3.0f;
   ud[1] = um1*(3.0f-9.0f*u1);
   ud[2] = (6.0f-9.0f*u1)*u1;
   ud[3] = u2*3.0f;

   float vd[4];
   vd[0] = vm2*-3.0f;
   vd[1] = vm1*(3.0f-9.0f*v1);
   vd[2] = (6.0f-9.0f*v1)*v1;
   vd[3] = v2*3.0f;

   Vec3f p(0.0f);
   Vec3f su(0.0f);
   Vec3f sv(0.0f);

   const Vec3f* b = _b;
   for( uint j = 0; j < 4; ++j )
   {
      Vec3f cp(0.0f);
      Vec3f csu(0.0f);
      for( uint i = 0; i < 4; ++i, ++b )
      {
         cp  += (*b)*(uc[i]);
         csu += (*b)*(ud[i]);
      }
      p  += cp*vc[j];
      su += csu*vc[j];
      sv += cp*vd[j];
   }

   pos = p;

   // Normal computation.
   Vec3f dir = su.cross( sv );
   float len = dir.length();

   // FIXME: should have a better more general solution.
   if( len < 1e-5 )
   {
      normal = (_b[10]-_b[15]).cross( su ).getNormalized();
   }
   else
   {
      normal = dir / len;
   }
}

//------------------------------------------------------------------------------
//! 
int
DFPatch::flatness( const Vec2f& uv0, const Vec2f& uv1, int /*edge*/, float /*error*/ )
{
   if( (uv1-uv0).x < 1.0f ) return 0;
   return (1|4|16)|(2|8|32);
}

//------------------------------------------------------------------------------
//! 
void
DFPatch::print() const
{
   StdErr << "parametric patch:\n";
   for( int i = 0; i < 16; ++i )
   {
      StdErr << i << ": " << _b[i] << nl;
   }
}

NAMESPACE_END
