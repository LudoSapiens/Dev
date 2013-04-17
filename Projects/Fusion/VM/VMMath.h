/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef FUSION_VMMATH_H
#define FUSION_VMMATH_H

#include <Fusion/StdDefs.h>
#include <Fusion/VM/VM.h>

NAMESPACE_BEGIN

/*==============================================================================
  CLASS VMCounter
==============================================================================*/
class VMCounter
{
public:

   /*----- methods -----*/

   VMCounter( double start = 0.0, double increment = 1.0 ): _cur( start ), _inc( increment ) {}

   inline double  current() const { return _cur; }
   inline void    current( double cur ) { _cur = cur; }

   inline double  increment() const { return _inc; }
   inline void    increment( double inc ) { _inc = inc; }

   inline void  set( double start, double increment ) { _cur = start; _inc = increment; }

   inline double  next() { double tmp = _cur; _cur += _inc; return tmp; }

   inline double  operator()() { return next(); }

protected:

   /*----- data members -----*/

   double  _cur;  //!< The current value.
   double  _inc;  //!< The increment to generate the next value.

private:
}; //class VMCounter


/*==============================================================================
  CLASS VMMath
==============================================================================*/

//! Provide math object (vec2, vec3, vec4, mat2, mat3, mat4) to VM.
//! Those objects only live in the VM side.
//! It also provide bit operation and other mathematical functions.
class VMMath
{

public:

   /*----- types and enumerations ----*/

   enum {
      VEC2,
      VEC3,
      VEC4,
      QUAT,
      MAT2,
      MAT3,
      MAT4,
      COUNTER,
      POINTER
   };

   /*----- static methods -----*/

   static void initialize();

   inline         static void push( VMState*, float );
   FUSION_DLL_API static void push( VMState*, const Vec2f& );
   FUSION_DLL_API static void push( VMState*, const Vec3f& );
   FUSION_DLL_API static void push( VMState*, const Vec4f& );
   FUSION_DLL_API static void push( VMState*, const Quatf& );
   FUSION_DLL_API static void push( VMState*, const Mat2f& );
   FUSION_DLL_API static void push( VMState*, const Mat3f& );
   FUSION_DLL_API static void push( VMState*, const Mat4f& );
   FUSION_DLL_API static void push( VMState*, const VMCounter& );
   inline         static void push( VMState*, void* );

   inline         static float toFloat( VMState*, int idx );
   FUSION_DLL_API static Vec2f toVec2( VMState*, int idx );
   FUSION_DLL_API static Vec3f toVec3( VMState*, int idx );
   FUSION_DLL_API static Vec4f toVec4( VMState*, int idx );
   FUSION_DLL_API static Quatf toQuat( VMState*, int idx );
   FUSION_DLL_API static Mat2f toMat2( VMState*, int idx );
   FUSION_DLL_API static Mat3f toMat3( VMState*, int idx );
   FUSION_DLL_API static Mat4f toMat4( VMState*, int idx );
   FUSION_DLL_API static VMCounter* toCounter( VMState*, int idx );
   inline         static void* toPointer( VMState*, int idx );

   FUSION_DLL_API static int  type( VMState* vm, int idx );

};

//------------------------------------------------------------------------------
//!
inline void
VMMath::push( VMState* vm, float f )
{
   VM::push( vm, f );
}

//------------------------------------------------------------------------------
//!
inline void
VMMath::push( VMState* vm, void* p )
{
   VM::push( vm, p );
}

//------------------------------------------------------------------------------
//!
inline float
VMMath::toFloat( VMState* vm, int idx )
{
   return VM::toFloat( vm, idx );
}

//-----------------------------------------------------------------------------
//!
inline void*
VMMath::toPointer( VMState* vm, int idx )
{
   return VM::toPtr( vm, idx );
}

NAMESPACE_END


#endif

