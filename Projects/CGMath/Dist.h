/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef CGMATH_DIST_H
#define CGMATH_DIST_H

#include <CGMath/StdDefs.h>

#include <CGMath/Vec3.h>

NAMESPACE_BEGIN


/*==============================================================================
  NAMESPACE CGM
==============================================================================*/

//! This file contains a series of distance calculation routines.

namespace CGM
{


//------------------------------------------------------------------------------
//!
CGMATH_DLL_API
void
triangle_point_closestPoint
( const Vec3f& triA, const Vec3f& triB, const Vec3f& triC,
  const Vec3f& pt,
  Vec3f& closestPt );

//-----------------------------------------------------------------------------
//!
CGMATH_DLL_API float  grayscaleDistance( float srcVal, const Vec2f& srcGrd );

//-----------------------------------------------------------------------------
//!
CGMATH_DLL_API float  grayscaleDistance( const Vec2i& srcIdx, float srcVal, const Vec2i& dstIdx, float dstVal, const Vec2f& dstGrd );

} // namespace CGM

NAMESPACE_END

#endif //CGMATH_DIST_H
