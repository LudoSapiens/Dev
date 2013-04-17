/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/Geometry/ParametricPatch.h>

/*==============================================================================
  UNNAME NAMESPACE
==============================================================================*/

UNNAMESPACE_BEGIN


UNNAMESPACE_END

NAMESPACE_BEGIN

/*==============================================================================
   CLASS ParametricPatch
==============================================================================*/

#define APPROX4

//------------------------------------------------------------------------------
//! 
void 
ParametricPatch::init( MetaSurface::Patch& p )
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
      int n                  = 0;
      int cc                 = c;
      int crnum              = 0;
      MetaSurface::Patch* cp = &p;
      do {
         //creases[n] = MetaSurface::crease( *cp, cc ) == 0 ? 2.0f/3.0f : 0.0f;
         if( MetaSurface::crease( *cp, cc ) == 0 )
         {
            creases[n] = 2.0f/3.0f;
         }
         else
         {
            creasesID[crnum++] = n;
            creases[n] = 0.0f;
         }
         vring[n]   = *cp->_controlPts[(cc+1)%4];
         vdiag[n++] = *cp->_controlPts[(cc+2)%4];
         int ne     = (cc+3)%4;
         cc         = MetaSurface::neighborEdge( *cp, ne );
         cp         = MetaSurface::neighborPatch( *cp, ne );
      } while( cp != &p );

      val[c]   = n;
      Vec3f& v = *p._controlPts[c];

#ifdef APPROX4
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
#endif

#ifdef APPROX3
      // Face and corner vertices.
      vlim[c] = Vec3f(0.0f);
      for( int i = 0; i < n; ++i )
      {
         // Face vertex.
         f[c][i] = v*(4.0f/9.0f) + (vring[i]+vring[(i+1)%n])*(2.0f/9.0f) + vdiag[i]*(1.0f/9.0f);
            
         // Corner vertex.
         vlim[c] += v*n + vring[i]*4.0f + vdiag[i];
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
         t0[c] = (v*(2*n) + vring[0]*4.0f + (vring[1] + vring[n-1])*2.0f + vdiag[0] + vdiag[n-1])/float(2*n+10);
      }
      if( creases[1] == 0.0f )
      {
         t1[c] = (v*2.0f + vring[1])/3.0f;
      }
      else
      {
         t1[c] = (v*(2*n) + vring[1]*4.0f + (vring[2] + vring[0])*2.0f + vdiag[0] + vdiag[1])/float(2*n+10);
      }

      cn[c] = CGM::cos( CGConstf::pi2()/float(n) );
      sn[c] = CGM::sin( CGConstf::pi2()/float(n) );
#endif

#ifdef APPROX2
      // Face and corner vertices.
      vlim[c] = Vec3f(0.0f);
      for( int i = 0; i < n; ++i )
      {
         // Face vertex.
         // FIXME: when creases, should be computed differently.
         f[c][i] = (v*n + (vring[i]+vring[(i+1)%n])*2.0f + vdiag[i])/float(n+5);

         // Corner vertex.
         vlim[c] += f[c][i];
      }
      if( crnum < 2 )
      {
         vlim[c] = vlim[c] / float(n);
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
#endif

#ifdef APPROX1
      // Compute limite vertex and face vertex.
      vlim[c] = Vec3f(0.0f);
      for( int i = 0; i < n; ++i )
      {
         // Face vertex.
         // Change equation when adding crease.
         //f[c][i] = v*(4.0f/9.0f) + (vring[i]+vring[(i+1)%n])*(2.0f/9.0f) + vdiag[i]*(1.0f/9.0f);

         float cr0 = creases[i];
         float cr1 = creases[(i+1)%n];
         f[c][i] = v*((1.0f-cr0)*(1.0f-cr1)) +
                      (v+vring[i])*((1.0f-cr0)*cr1*0.5f) +
                      (v+vring[(i+1)%n])*(cr0*(1.0f-cr1)*0.5f) +
                      (v+vring[i]+vring[(i+1)%n]+vdiag[i])*(cr0*cr1*0.25f);

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
#endif
   }

   // Compute per patch parameters.

   // Do we have a regular patch?
   //if( (val[0] == 4) && (val[1] == 4) && (val[2] == 4) && (val[3] == 4) )
   if( true )
   {
      _regular = true;
      // Cubic patch.
      _b[0]  = vlim[0]; _b[1]  = t0[0];   _b[2]  = t1[1];   _b[3]  = vlim[1];
      _b[4]  = t1[0];   _b[5]  = f[0][0]; _b[6]  = f[1][0]; _b[7]  = t0[1];
      _b[8]  = t0[3];   _b[9]  = f[3][0]; _b[10] = f[2][0]; _b[11] = t1[2];
      _b[12] = vlim[3]; _b[13] = t1[3];   _b[14] = t0[2];   _b[15] = vlim[2];
   }
   else
   {
      _regular = false;
      // P4-Patch.
      // 300, 210 and 120.
      for( uint c = 0; c < 4; ++c )
      {
         _b[c*6+0] = vlim[c];      // 300
         _b[c*6+1] = t0[c];        // 210
         _b[c*6+2] = t1[(c+1)%4];  // 120
      }

      // 211, 121 and 004.
      _b[24] = Vec3f(0.0f);
      for( uint c = 0; c < 4; ++c )
      {
         uint c1    = (c+1)%4;
         float sinv = 1.0f/(sn[c]+sn[c1]);

         Vec3f b310 = _b[c*6+1]*0.75f + _b[c*6]*0.25f;
         Vec3f b130 = _b[c*6+2]*0.75f + _b[c1*6]*0.25f;
         // 211
         _b[c*6+3] = b310 + (_b[c*6+2]-_b[c*6+1])*((1.0f+cn[c])*0.25f)   +
                            (_b[c*6+1]-_b[c*6+0])*((1.0f-cn[c1])*0.125f) +
                            (f[c][0]-f[c][val[c]-1])*(sinv*0.375f);
         // 121
         _b[c*6+4] = b130 + (_b[c*6+1]-_b[c*6+2])*((1.0f+cn[c1])*0.25f) +
                            (_b[c*6+2]-_b[c1*6])*((1.0f-cn[c])*0.125f)  +
                            (f[c1][0]-f[c1][1])*(sinv*0.375f);
         // 004
         _b[24] += _b[c*6] + (t0[c]+t1[c])*3.0f + f[c][0]*9.0f;
      }
      _b[24] /= 64.0f;

      // b112.
      for( uint c = 0; c < 4; ++c )
      {
         uint c1 = (c+1)%4;
         uint c2 = (c+2)%4;
         uint c3 = (c+3)%4;
         _b[c*6+5] = _b[24] + (_b[c*6+3]+_b[c*6+4]-_b[c1*6+4]-_b[c3*6+3])*0.1875f +
                              (_b[c1*6+3]+_b[c3*6+4]-_b[c2*6+3]-_b[c2*6+4])*0.0625f;
      }
   }
}

//------------------------------------------------------------------------------
//! 
void 
ParametricPatch::parameters( const Vec2f& uv, Vec3f& pos, Vec3f& normal ) const
{
   if( _regular )
   {
      evalCubic( uv, pos, normal );
   }
   else
   {
      // Find triangular patch to evaluate and convert uv.
      Vec3f uvw;
      uint p;
      if( uv.y <= uv.x )
      {
         if( uv.x+uv.y <= 1.0f )
         {
            uvw = Vec3f( 1.0f-uv.x-uv.y, uv.x-uv.y, 2.0f*uv.y );
            p   = 0;
         }
         else
         {
            uvw = Vec3f( uv.x-uv.y, uv.x+uv.y-1.0f, 2.0f*(1.0f-uv.x) );
            p   = 1;
         }
      }
      else
      {
         if( uv.x+uv.y <= 1.0f )
         {
            uvw = Vec3f( uv.y-uv.x, 1.0f-uv.y-uv.x, 2.0f*uv.x );
            p   = 3;
         }
         else
         {
            uvw = Vec3f( uv.x+uv.y-1.0f, uv.y-uv.x, 2.0f*(1.0f-uv.y) );
            p   = 2;
         }
      }
      evalTriangle( p, uvw, pos, normal );
   }
}

//------------------------------------------------------------------------------
//! 
const Vec3f& 
ParametricPatch::position( uint i ) const
{
   static int reg[4]  = {0, 3, 15, 12 };
   return _regular ? _b[reg[i]] : _b[i*6];
}

//------------------------------------------------------------------------------
//! 
void 
ParametricPatch::evalCubic( const Vec2f& uv, Vec3f& pos, Vec3f& normal ) const
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
void 
ParametricPatch::evalTriangle( uint p, const Vec3f& uvw, Vec3f& pos, Vec3f& normal ) const
{
   // Compute patch control points.
   uint o  = p*6;
   uint op = ((p+3)%4)*6;
   uint on = ((p+1)%4)*6;

   Vec3f p130 = _b[op+2]*0.75f + _b[o]*0.25f;
   Vec3f n310 = _b[on]*0.25f   + _b[on+1]*0.75f;

   Vec3f pt[15];
   pt[0]  = _b[o];                         // 400
   pt[1]  = _b[o]*0.25f   + _b[o+1]*0.75;  // 310
   pt[2]  = _b[o+1]*0.50f + _b[o+2]*0.5f;  // 220
   pt[3]  = _b[o+2]*0.75f + _b[on]*0.25f;  // 130
   pt[4]  = _b[on];                        // 040

   pt[5]  = (pt[1]+p130)*0.5f;             // 301
   pt[6]  = _b[o+3];                       // 211
   pt[7]  = _b[o+4];                       // 121
   pt[8]  = (pt[3]+n310)*0.5f;             // 031

   pt[9]  = (pt[6]+_b[op+4])*0.5f;         // 202
   pt[10] = _b[o+5];                       // 112
   pt[11] = (pt[7]+_b[on+3])*0.5f;         // 022

   pt[12] = (pt[10]+_b[op+5])*0.5f;        // 103 
   pt[13] = (pt[10]+_b[on+5])*0.5f;        // 013

   pt[14] = _b[24];                        // 004


   // Tianyun's deCasteljau.
   float u = uvw.x;
   float v = uvw.y;
   float w = uvw.z;

   Vec3f pt0[10];
   pt0[0] = pt[0]*u  + pt[1]*v  + pt[5]*w;
   pt0[1] = pt[1]*u  + pt[2]*v  + pt[6]*w;
   pt0[2] = pt[2]*u  + pt[3]*v  + pt[7]*w;
   pt0[3] = pt[3]*u  + pt[4]*v  + pt[8]*w;
   pt0[4] = pt[5]*u  + pt[6]*v  + pt[9]*w;
   pt0[5] = pt[6]*u  + pt[7]*v  + pt[10]*w;
   pt0[6] = pt[7]*u  + pt[8]*v  + pt[11]*w;
   pt0[7] = pt[9]*u  + pt[10]*v + pt[12]*w;
   pt0[8] = pt[10]*u + pt[11]*v + pt[13]*w;
   pt0[9] = pt[12]*u + pt[13]*v + pt[14]*w;

   Vec3f pt1[6];
   pt1[0] = pt0[0]*u + pt0[1]*v + pt0[4]*w;
   pt1[1] = pt0[1]*u + pt0[2]*v + pt0[5]*w;
   pt1[2] = pt0[2]*u + pt0[3]*v + pt0[6]*w;
   pt1[3] = pt0[4]*u + pt0[5]*v + pt0[7]*w;
   pt1[4] = pt0[5]*u + pt0[6]*v + pt0[8]*w;
   pt1[5] = pt0[7]*u + pt0[8]*v + pt0[9]*w;

   Vec3f pt2[3];
   pt2[0] = pt1[0]*u + pt1[1]*v + pt1[3]*w;
   pt2[1] = pt1[1]*u + pt1[2]*v + pt1[4]*w;
   pt2[2] = pt1[3]*u + pt1[4]*v + pt1[5]*w;

   Vec3f su = (pt2[0]-pt2[2]);
   Vec3f sv = (pt2[1]-pt2[2]);
   pos      = pt2[0]*u + pt2[1]*v + pt2[2]*w;
   normal   = su.cross( sv ).getNormalized();
}

//------------------------------------------------------------------------------
//! 
int
ParametricPatch::flatness( const Vec2f& uv0, const Vec2f& uv1, int /*edge*/, float /*error*/ )
{
   if( (uv1-uv0).x < 1.0f ) return 0;
   return (1|4|16)|(2|8|32);
}

//------------------------------------------------------------------------------
//! 
void
ParametricPatch::print() const
{
   StdErr << "parametric patch:\n";
   for( int i = 0; i < 16; ++i )
   {
      StdErr << i << ": " << _b[i] << nl;
   }
}

NAMESPACE_END
