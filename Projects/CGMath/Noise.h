/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef CGMATH_NOISE_H
#define CGMATH_NOISE_H

#include <CGMath/StdDefs.h>

#include <CGMath/Vec3.h>
#include <CGMath/Geom.h>
#include <CGMath/Math.h>

NAMESPACE_BEGIN

namespace CGM
{

/*----- constants -----*/


/*----- functions -----*/

//------------------------------------------------------------------------------
//! Converts a cell index into a constant noise value in [0, 1) range.
CGMATH_DLL_API float  cellNoise1( int x, int y, int z );

//------------------------------------------------------------------------------
//! Converts a 3D coordinate into noise value for every unit-length cell.
//! The returned noise value is in [0, 1) range.
inline float  cellNoise1( const Vec3f& p )
{
   return cellNoise1( CGM::floori(p.x), CGM::floori(p.y), CGM::floori(p.z) );
}

//------------------------------------------------------------------------------
//! Converts a cell index into a vector of constant noise values in [0, 1) range.
CGMATH_DLL_API Vec2f  cellNoise2( int x, int y, int z );

//------------------------------------------------------------------------------
//! Converts a 3D coordinate into noise value for every unit-length cell.
//! The returned noise values are in [0, 1) range.
inline Vec2f  cellNoise2( const Vec3f& p )
{
   return cellNoise2( CGM::floori(p.x), CGM::floori(p.y), CGM::floori(p.z) );
}

//------------------------------------------------------------------------------
//! Converts a cell index into a vector of constant noise values in [0, 1) range.
CGMATH_DLL_API Vec3f  cellNoise3( int x, int y, int z );

//------------------------------------------------------------------------------
//! Converts a 3D coordinate into noise value for every unit-length cell.
//! The returned noise values are in [0, 1) range.
inline Vec3f  cellNoise3( const Vec3f& p )
{
   return cellNoise3( CGM::floori(p.x), CGM::floori(p.y), CGM::floori(p.z) );
}

//------------------------------------------------------------------------------
//! Evaluates improved Perlin noise, returning a value in [-1, 1] range.
CGMATH_DLL_API float  perlinNoise1( const Vec3f& p );

//------------------------------------------------------------------------------
//! Evaluates improved Perlin noise, returning a vector of values in [-1, 1] range.
CGMATH_DLL_API Vec2f  perlinNoise2( const Vec3f& p );

//------------------------------------------------------------------------------
//! Evaluates improved Perlin noise, returning a vector of values in [-1, 1] range.
CGMATH_DLL_API Vec3f  perlinNoise3( const Vec3f& p );

//------------------------------------------------------------------------------
//! Anti-aliased version of Perlin noise, returned in [0, 1] range.
inline float  filteredPerlinNoise1( const Vec3f& p, const float width )
{
   return perlinNoise1(p) * (1.0f - smoothStep(0.2f, 0.75f, width));
}

//------------------------------------------------------------------------------
//! Anti-aliased version of Perlin noise, returned in [0, 1] range.
inline Vec2f  filteredPerlinNoise2( const Vec3f& p, const float width )
{
   return perlinNoise2(p) * (1.0f - smoothStep(0.2f, 0.75f, width));
}

//------------------------------------------------------------------------------
//! Anti-aliased version of Perlin noise, returned in [0, 1] range.
inline Vec3f  filteredPerlinNoise3( const Vec3f& p, const float width )
{
   return perlinNoise3(p) * (1.0f - smoothStep(0.2f, 0.75f, width));
}

//------------------------------------------------------------------------------
//! Voronoi cell noise, 1-feature version, like RenderMan.
//! The jitter parameter should be <=0.5f.
//! The function returns the distance to the closest point (stored in p1).
CGMATH_DLL_API float  voronoiNoise1( const Vec3f& p, const float jitter, Vec3f& p1 );

//------------------------------------------------------------------------------
//! Voronoi cell noise, 2-feature version, like RenderMan.
//! The jitter parameter should be <=0.5f.
//! The function returns the 2 closest distances (x being the closest) to the
//! two closest points (stored in p1 and p2).
CGMATH_DLL_API Vec2f  voronoiNoise2( const Vec3f& p, const float jitter, Vec3f& p1, Vec3f& p2 );


//------------------------------------------------------------------------------
//! Evaluates a fractional Brownian motion noise.
//!  @param p            The position
//!  @param width        Approximate space between pixels.
//!  @param octaves      The number of octaves to loop.
//!  @param lacunarity   Frequency spacing between octaves.
//!  @param gain         Scaling factor between octaves.
CGMATH_DLL_API float  fBmNoise1(
   const Vec3f& p,
   const float  width      = 0.0f,
   const uint   octaves    = 4,
   const float  lacunarity = 2.0f,
   const float  gain       = 0.5f
);

} // namespace CGM

NAMESPACE_END

#endif //CGMATH_NOISE_H
