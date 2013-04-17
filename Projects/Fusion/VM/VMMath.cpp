/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Fusion/VM/VMMath.h>

#include <Fusion/Resource/Image.h>
#include <Fusion/Resource/BitmapManipulator.h>
#include <Fusion/Resource/ResManager.h>
#include <Fusion/VM/VMRegistry.h>

#include <CGMath/Distributions.h>
#include <CGMath/Math.h>
#include <CGMath/Noise.h>
#include <CGMath/Random.h>

#include <Base/ADT/StringMap.h>
#include <Base/Dbg/DebugStream.h>

extern "C" {
#include <lauxlib.h>
#include <lualib.h>
}

// LUA uses only double precision numbers. Bit operations on high order bits
// can cause loss in lower order bit, corrupting the bitfield.
// The standard library can be compiled in two modes:
//   - Checked        all size checks are done by the library
//   - Unchecked      user must ensure that no overflow occurs
// In Unchecked mode, when compiled in debug, the library will perform size
// checks and issue warnings whenever size overflow occurs. Warnings are issued
// in the trace group VMMath.

//#define MATH_CHECKED
#define MATH_UNCHECKED             // default

//------------------------------------------------------------------------------
//!
#if ( !defined(MATH_CHECKED) && !defined(MATH_UNCHECKED) )
#define MATH_UNCHECKED
#endif

#if ( defined(MATH_CHECKED) || defined(_DEBUG) )
#define MATH_CHECK_SIZE
#endif

#if ( defined(MATH_UNCHECKED) && defined(_DEBUG) )
#define MATH_SIZE_WARNINGS
#endif

//------------------------------------------------------------------------------
//! A utility macro to shorten error checking in some switch..case statements.
#define IF_ERR( cond, msg ) \
   if( cond ) \
   { \
      StdErr << msg << nl; \
      return 0; \
   }


/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/

UNNAMESPACE_BEGIN

DBG_STREAM( os_vm, "VMMath" );

enum
{
   ATTRIB_CONJUGATE,
   ATTRIB_CURRENT,
   ATTRIB_INCREMENT,
   ATTRIB_INVERSE,
   ATTRIB_NEGATE,
   ATTRIB_NORMALIZE,
   ATTRIB_PICK,
   ATTRIB_RANDOM,
   ATTRIB_SEED,
   ATTRIB_SET,
   ATTRIB_TO_AXIS_ANGLE,
   ATTRIB_TO_AXIS_CIR,
   ATTRIB_TO_AXIS_COS,
   ATTRIB_TO_MATRIX,
   ATTRIB_TO_MATRIX3
};

StringMap _counter_attr(
   "current",    ATTRIB_CURRENT,
   "increment",  ATTRIB_INCREMENT,
   "set",        ATTRIB_SET,
   ""
);

StringMap _rng_attr(
   "pick",    ATTRIB_PICK,
   "random",  ATTRIB_RANDOM,
   "seed",    ATTRIB_SEED,
   ""
);
StringMap _quat_attr(
   "conjugate",   ATTRIB_CONJUGATE,
   "inverse",     ATTRIB_INVERSE,
   "negate",      ATTRIB_NEGATE,
   "normalize",   ATTRIB_NORMALIZE,
   "toAxisAngle", ATTRIB_TO_AXIS_ANGLE,
   "toAxisCir",   ATTRIB_TO_AXIS_CIR,
   "toAxisCos",   ATTRIB_TO_AXIS_COS,
   "toMatrix",    ATTRIB_TO_MATRIX,
   "toMatrix3",   ATTRIB_TO_MATRIX3,
   ""
);

//------------------------------------------------------------------------------
//! Converts a character such that xyzw map to indices 0123 (and >3 otherwise).
inline uint xyzwToIdx( char c )
{
   uint tmp = (c - 'w');
   return ((tmp + 3) & 0x03) + (tmp & ~0x03);
}

//------------------------------------------------------------------------------
//! Converts a digit character into its value (>9 if it's not a digit).
inline uint digitToIdx( char c )
{
   return c - '0';
}

//------------------------------------------------------------------------------
//!
inline const char* typeToStr( int type )
{
   switch( type )
   {
      case VMMath::VEC2: return "vec2";
      case VMMath::VEC3: return "vec3";
      case VMMath::VEC4: return "vec4";
      case VMMath::QUAT: return "quat";
      case VMMath::MAT2: return "mat2";
      case VMMath::MAT3: return "mat3";
      case VMMath::MAT4: return "mat4";
      default          : return "<invalid>";
   }
}

// IEEE standard says 53, but we stay on the safe side. ;)
const int MATH_BITSIZE    = 50;

const int64_t MATH_BITMASK = ((int64_t)1 << MATH_BITSIZE)- 1;
const int64_t MATH_MSB     = (int64_t)1 << (MATH_BITSIZE - 1);

#ifdef MATH_CHECK_SIZE

//------------------------------------------------------------------------------
//! Check and possibly issue warnings
inline int64_t
checkSize( double bitField )
{
   if( (int64_t)bitField != bitField ||
       bitField < ~MATH_BITMASK || bitField > MATH_BITMASK )
   {
#ifdef MATH_SIZE_WARNINGS
      DBG_MSG( os_vm, "WARNING! VMMath, bitfield overflow! Precision loss!" );
#endif
      int64_t sign = (int64_t)((bitField < 0)?-1:1);
      return (((int64_t)bitField * sign) & MATH_BITMASK) * sign;
   }

   return (int64_t)bitField;
}

//------------------------------------------------------------------------------
//!
inline int64_t
checkSize( int64_t bitField )
{
   if( (int64_t)bitField != bitField ||
       bitField < ~MATH_BITMASK || bitField > MATH_BITMASK )
   {
#ifdef MATH_SIZE_WARNINGS
      DBG_MSG( os_vm, "WARNING! VMMath, bitfield overflow! Precision loss!" );
#endif
      int64_t sign = (int64_t)((bitField & MATH_MSB)?-1:1);
      return ((bitField * sign) & MATH_BITMASK) * sign;
   }

   return bitField;
}

#else

//------------------------------------------------------------------------------
//!
inline int64_t
checkSize( double bitField )
{
   return (int64_t)bitField;
}

//------------------------------------------------------------------------------
//!
inline int64_t
checkSize( int64_t bitField )
{
   return bitField;
}

#endif  //MATH_CHECK_SIZE

//------------------------------------------------------------------------------
//!
int
bnot( VMState* vm )
{
   CHECK( VM::getTop( vm ) == 1 );
   int64_t bitField = checkSize( VM::toNumber( vm, 1 ) );
   VM::push( vm, (double)~bitField );
   return 1;
}

//------------------------------------------------------------------------------
//!
int
band( VMState* vm )
{
   int nParams = VM::getTop( vm );
   CHECK( nParams > 0 );

   int64_t bitField = checkSize( VM::toNumber( vm, nParams ) );
   --nParams;

   while( nParams > 0 )
   {
      bitField &= checkSize( VM::toNumber( vm, nParams ) );
      --nParams;
   }

   VM::push( vm, (double)bitField );
   return 1;
}

//------------------------------------------------------------------------------
//!
int
bor( VMState* vm )
{
   int nParams = VM::getTop( vm );
   CHECK( nParams > 0 );

   int64_t bitField = checkSize( VM::toNumber( vm, nParams ) );
   --nParams;

   while( nParams > 0 )
   {
      bitField |= checkSize( VM::toNumber( vm, nParams ) );
      --nParams;
   }

   VM::push( vm, (double)bitField );
   return 1;
}

//------------------------------------------------------------------------------
//!
int
bxor( VMState* vm )
{
   int nParams = VM::getTop( vm );
   CHECK( nParams > 0 );

   int64_t bitField = checkSize( VM::toNumber( vm, nParams ) );
   --nParams;

   while( nParams > 0 )
   {
      bitField ^= checkSize( VM::toNumber( vm, nParams ) );
      --nParams;
   }

   VM::push( vm, (double)bitField );
   return 1;
}

//------------------------------------------------------------------------------
//!
int
lshift( VMState* vm )
{
   CHECK( VM::getTop( vm ) == 2 );
   int64_t bitField1 = checkSize( VM::toNumber( vm, 1 ) );
   int64_t dim       = checkSize( VM::toNumber( vm, 1 ) );
   CHECK( dim > 0 );

   VM::push( vm, (double)checkSize( bitField1 << dim ) );
   return 1;
}

//------------------------------------------------------------------------------
//!
int
rshift( VMState* vm )
{
   CHECK( VM::getTop( vm ) == 2 );
   int64_t bitField1 = checkSize( VM::toNumber( vm, 1 ) );
   int64_t dim       = checkSize( VM::toNumber( vm, 1 ) );
   CHECK( dim > 0 );

   VM::push( vm, (double)(bitField1 >> dim) );
   return 1;
}

//------------------------------------------------------------------------------
//!
int
arshift( VMState* vm )
{
   CHECK( VM::getTop( vm ) == 2 );
   int64_t bitField1 = checkSize( VM::toNumber( vm, 1 ) );
   int64_t dim       = checkSize( VM::toNumber( vm, 1 ) );
   CHECK( dim > 0 );

   if( bitField1 & MATH_MSB )
   {
      VM::push( vm, (double)~( (~bitField1) >> dim ) );
   }
   else
   {
      VM::push( vm, (double)(bitField1 >> dim) );
   }
   return 1;
}

//------------------------------------------------------------------------------
//!
int
div( VMState* vm )
{
   CHECK( VM::getTop( vm ) == 2 );
   int64_t numerator   = checkSize( VM::toNumber( vm, 1 ) );
   int64_t denominator = checkSize( VM::toNumber( vm, 2 ) );

   VM::push( vm, (double)(numerator / denominator) );
   return 1;
}

//------------------------------------------------------------------------------
//!
int
mod( VMState* vm )
{
   CHECK( VM::getTop( vm ) == 2 );
   int64_t numerator   = checkSize( VM::toNumber( vm, 1 ) );
   int64_t denominator = checkSize( VM::toNumber( vm, 2 ) );

   VM::push( vm, (double)(numerator % denominator) );
   return 1;
}

//------------------------------------------------------------------------------
//!
const VM::Reg bit_funcs[] = {
   { "bnot",    bnot     },
   { "band",    band     },
   { "bor",     bor      },
   { "bxor",    bxor     },
   { "lshift",  lshift   },
   { "rshift",  rshift   },
   { "arshift", arshift  },
   { "div",     div      },
   { "mod",     mod      },
   { 0, 0 }
};

/*==============================================================================
   STRUCT VMType
==============================================================================*/

template< class T >
struct VMType
{
   int _type;
   T   _val;
};

typedef VMType<Vec2f> VMVec2;
typedef VMType<Vec3f> VMVec3;
typedef VMType<Vec4f> VMVec4;
typedef VMType<Quatf> VMQuat;
typedef VMType<Mat2f> VMMat2;
typedef VMType<Mat3f> VMMat3;
typedef VMType<Mat4f> VMMat4;

struct VMRType
{
   int   _type;
   float _val[4];
};

//------------------------------------------------------------------------------
//!
template< class T > inline void
create( VMState* vm, uint type, const T& v, const char* meta )
{
   VMType<T>* ud = (VMType<T>*)lua_newuserdata( vm, sizeof(VMType<T>) );
   ud->_type = type;
   ud->_val  = v;
   luaL_getmetatable( vm, meta );
   lua_setmetatable( vm, -2 );
}

//------------------------------------------------------------------------------
//!
int
equal( VMState* vm )
{
   VMRType* op1 = (VMRType*)lua_touserdata( vm, 1 );
   VMRType* op2 = (VMRType*)lua_touserdata( vm, 2 );
   IF_ERR( op1->_type != op2->_type, "Type mismatch on dot(" << typeToStr(op1->_type) << "," << typeToStr(op2->_type) << ")." );
   if( VM::getTop( vm ) <= 2 )
   {
      switch( op1->_type )
      {
         case VMMath::VEC2: VM::push( vm, CGM::equal( ((VMVec2*)op1)->_val, ((VMVec2*)op2)->_val ) ); return 1;
         case VMMath::VEC3: VM::push( vm, CGM::equal( ((VMVec3*)op1)->_val, ((VMVec3*)op2)->_val ) ); return 1;
         case VMMath::VEC4: VM::push( vm, CGM::equal( ((VMVec4*)op1)->_val, ((VMVec4*)op2)->_val ) ); return 1;
         //case VMMath::QUAT: VM::push( vm, CGM::equal( ((VMQuat*)op1)->_val, ((VMQuat*)op2)->_val ) ); return 1;
         //case VMMath::MAT2: VM::push( vm, CGM::equal( ((VMMat2*)op1)->_val, ((VMMat2*)op2)->_val ) ); return 1;
         //case VMMath::MAT3: VM::push( vm, CGM::equal( ((VMMat3*)op1)->_val, ((VMMat3*)op2)->_val ) ); return 1;
         //case VMMath::MAT4: VM::push( vm, CGM::equal( ((VMMat4*)op1)->_val, ((VMMat4*)op2)->_val ) ); return 1;
         default:;
      }
   }
   else
   {
      float err = VM::toFloat( vm, 3 );
      switch( op1->_type )
      {
         case VMMath::VEC2: VM::push( vm, CGM::equal( ((VMVec2*)op1)->_val, ((VMVec2*)op2)->_val, err ) ); return 1;
         case VMMath::VEC3: VM::push( vm, CGM::equal( ((VMVec3*)op1)->_val, ((VMVec3*)op2)->_val, err ) ); return 1;
         case VMMath::VEC4: VM::push( vm, CGM::equal( ((VMVec4*)op1)->_val, ((VMVec4*)op2)->_val, err ) ); return 1;
         //case VMMath::QUAT: VM::push( vm, CGM::equal( ((VMQuat*)op1)->_val, ((VMQuat*)op2)->_val, err ) ); return 1;
         //case VMMath::MAT2: VM::push( vm, CGM::equal( ((VMMat2*)op1)->_val, ((VMMat2*)op2)->_val, err ) ); return 1;
         //case VMMath::MAT3: VM::push( vm, CGM::equal( ((VMMat3*)op1)->_val, ((VMMat3*)op2)->_val, err ) ); return 1;
         //case VMMath::MAT4: VM::push( vm, CGM::equal( ((VMMat4*)op1)->_val, ((VMMat4*)op2)->_val, err ) ); return 1;
         default:;
      }
   }
   return 0;
}

//------------------------------------------------------------------------------
//!
int
vec2( VMState* vm )
{
   uint numParams = VM::getTop(vm);

   float v[8];
   int w = 0;
   v[0] = 0.0f;
   v[1] = 0.0f;
   for( uint i = 1; i <= numParams && w < 2; ++i )
   {
      VMRType* vec = (VMRType*)lua_touserdata( vm, i );

      if( !vec )
      {
         v[w++] = VM::toFloat(vm,i);
         if( w == 1 && numParams == 1 ) v[1] = v[0];
      }
      else
      {
         switch( vec->_type )
         {
            case VMMath::VEC2:
            {
               v[w++] = vec->_val[0];
               v[w++] = vec->_val[1];
            } break;
            case VMMath::VEC3:
            {
               v[w++] = vec->_val[0];
               v[w++] = vec->_val[1];
               v[w++] = vec->_val[3];
            } break;
            case VMMath::VEC4:
            {
               v[w++] = vec->_val[0];
               v[w++] = vec->_val[1];
               v[w++] = vec->_val[2];
               v[w++] = vec->_val[3];
            } break;
         }
      }
   }

   VMMath::push( vm, Vec2f::as(v) );
   return 1;
}

//------------------------------------------------------------------------------
//!
int
vec3( VMState* vm )
{
   uint numParams = VM::getTop(vm);

   float v[8];
   int w = 0;
   v[0] = 0.0f;
   v[1] = 0.0f;
   v[2] = 0.0f;
   for( uint i = 1; i <= numParams && w < 3; ++i )
   {
      VMRType* vec = (VMRType*)lua_touserdata( vm, i );

      if( !vec )
      {
         v[w++] = VM::toFloat(vm,i);
         if( w == 1 && numParams == 1 ) v[2] = v[1] = v[0];
      }
      else
      {
         switch( vec->_type )
         {
            case VMMath::VEC2:
            {
               v[w++] = vec->_val[0];
               v[w++] = vec->_val[1];
            } break;
            case VMMath::VEC3:
            {
               v[w++] = vec->_val[0];
               v[w++] = vec->_val[1];
               v[w++] = vec->_val[3];
            } break;
            case VMMath::VEC4:
            {
               v[w++] = vec->_val[0];
               v[w++] = vec->_val[1];
               v[w++] = vec->_val[2];
               v[w++] = vec->_val[3];
            } break;
         }
      }
   }

   VMMath::push( vm, Vec3f::as(v) );
   return 1;
}

//------------------------------------------------------------------------------
//!
int
vec4( VMState* vm )
{
   uint numParams = VM::getTop(vm);

   float v[8];
   int w = 0;
   v[0] = 0.0f;
   v[1] = 0.0f;
   v[2] = 0.0f;
   v[3] = 0.0f;
   for( uint i = 1; i <= numParams && w < 4; ++i )
   {
      VMRType* vec = (VMRType*)lua_touserdata( vm, i );

      if( !vec )
      {
         v[w++] = VM::toFloat(vm,i);
         if( w == 1 && numParams == 1 ) v[3] = v[2] = v[1] = v[0];
      }
      else
      {
         switch( vec->_type )
         {
            case VMMath::VEC2:
            {
               v[w++] = vec->_val[0];
               v[w++] = vec->_val[1];
            } break;
            case VMMath::VEC3:
            {
               v[w++] = vec->_val[0];
               v[w++] = vec->_val[1];
               v[w++] = vec->_val[2];
            } break;
            case VMMath::VEC4:
            {
               v[w++] = vec->_val[0];
               v[w++] = vec->_val[1];
               v[w++] = vec->_val[2];
               v[w++] = vec->_val[3];
            } break;
         }
      }
   }

   VMMath::push( vm, Vec4f::as(v) );
   return 1;
}

//------------------------------------------------------------------------------
//!
int
quat( VMState* vm )
{
   Quatf v;

   uint numParams = VM::getTop(vm);
   switch( numParams )
   {
      case 0:
      {
         v = Quatf::identity();
      }  break;
      //case 1:
      //{
      //   // TODO: Copy constructor?
      //   v = Quatf::identity();
      //}  break;
      case 2:
      {
         if( VM::type(vm,1) == VM::NUMBER )
         {
            // Angle/Axis.
            v = Quatf::axisAngle( VM::toVec3f(vm,2), CGM::cirToRad(VM::toFloat(vm,1)) );
         }
         else
         if( VM::type(vm,2) == VM::NUMBER )
         {
            // Axis/Angle.
            v = Quatf::axisAngle( VM::toVec3f(vm,1), CGM::cirToRad(VM::toFloat(vm,2)) );
         }
         else
         {
            // Two vecs.
            v = Quatf::twoVecs( VM::toVec3f(vm,1), VM::toVec3f(vm,2) );
         }
      }  break;
      case 3:
      {
         // 3 axes.
         v = Quatf::axes( VM::toVec3f(vm,1), VM::toVec3f(vm,2), VM::toVec3f(vm,3) );
      }  break;
      case 4:
      {
         v = Quatf( VM::toFloat(vm,1), VM::toFloat(vm,2), VM::toFloat(vm,3), VM::toFloat(vm,4) );
      }  break;
      default:
      {
         StdErr << "ERROR - quat() received an invalid number of parameters (" << numParams << ")" << nl;
         v = Quatf::identity();
      }  break;
   }

   VMMath::push( vm, v );
   return 1;
}

//------------------------------------------------------------------------------
//!
int
mat2( VMState* vm )
{
   Mat2f m;

   uint numParams = VM::getTop(vm);
   switch( numParams )
   {
      case 0: m = Mat2f::identity(); break;
      case 1:
      {
         float v = VM::toFloat(vm,1);
         m = Mat2f( v, 0.0f, 0.0f, v );
      } break;
      case 2:
      {
         Vec2f r0 = VMMath::toVec2(vm,1);
         Vec2f r1 = VMMath::toVec2(vm,2);
         m = Mat2f( r0.x, r0.y, r1.x, r1.y );
      } break;
      case 4:
         m = Mat2f( VM::toFloat(vm,1), VM::toFloat(vm,2),
                    VM::toFloat(vm,3), VM::toFloat(vm,4) );
         break;
      default:;
   }

   VMMath::push( vm, m );
   return 1;
}

//------------------------------------------------------------------------------
//!
int
mat3( VMState* vm )
{
   Mat3f m;

   uint numParams = VM::getTop(vm);
   switch( numParams )
   {
      case 0: m = Mat3f::identity(); break;
      case 1:
      {
         float v = VM::toFloat(vm,1);
         m = Mat3f( v, 0.0f, 0.0f, 0.0f, v, 0.0f, 0.0f, 0.0f, v );
      } break;
      case 3:
      {
         Vec3f r0 = VMMath::toVec3(vm,1);
         Vec3f r1 = VMMath::toVec3(vm,2);
         Vec3f r2 = VMMath::toVec3(vm,3);
         m = Mat3f( r0.x, r0.y, r0.z, r1.x, r1.y, r1.z, r2.x, r2.y, r2.z );
      } break;
      case 9:
         m = Mat3f( VM::toFloat(vm,1), VM::toFloat(vm,2), VM::toFloat(vm,3),
                    VM::toFloat(vm,4), VM::toFloat(vm,5), VM::toFloat(vm,6),
                    VM::toFloat(vm,7), VM::toFloat(vm,8), VM::toFloat(vm,9) );
         break;
      default:;
   }

   VMMath::push( vm, m );
   return 1;
}

//------------------------------------------------------------------------------
//!
int
mat4( VMState* vm )
{
   Mat4f m;

   uint numParams = VM::getTop(vm);
   switch( numParams )
   {
      case 0: m = Mat4f::identity(); break;
      case 1:
      {
         float v = VM::toFloat(vm,1);
         m = Mat4f( v, 0.0f, 0.0f, 0.0f,
                    0.0f, v, 0.0f, 0.0f,
                    0.0f, 0.0f, v, 0.0f,
                    0.0f, 0.0f, 0.0f, v );
      } break;
      case 4:
      {
         Vec4f r0 = VMMath::toVec4(vm,1);
         Vec4f r1 = VMMath::toVec4(vm,2);
         Vec4f r2 = VMMath::toVec4(vm,3);
         Vec4f r3 = VMMath::toVec4(vm,4);
         m = Mat4f( r0.x, r0.y, r0.z, r0.w,
                    r1.x, r1.y, r1.z, r1.w,
                    r2.x, r2.y, r2.z, r2.w,
                    r3.x, r3.y, r3.z, r3.w );
      } break;
      case 16:
         m = Mat4f( VM::toFloat(vm, 1), VM::toFloat(vm, 2), VM::toFloat(vm, 3), VM::toFloat(vm, 4),
                    VM::toFloat(vm, 5), VM::toFloat(vm, 6), VM::toFloat(vm, 7), VM::toFloat(vm, 8),
                    VM::toFloat(vm, 9), VM::toFloat(vm,10), VM::toFloat(vm,11), VM::toFloat(vm,12),
                    VM::toFloat(vm,13), VM::toFloat(vm,14), VM::toFloat(vm,15), VM::toFloat(vm,16) );
         break;
      default:;
   }

   VMMath::push( vm, m );
   return 1;
}


// Defines a 1-parameter function calling "CGM::funcName()" with either a float.
#define DEFINE_F( funcName ) \
   int funcName( VMState* vm ) \
   { \
      VM::push( vm, CGM::funcName( VM::toFloat( vm, 1 ) ) ); \
      return 1; \
   }

// Defines a 1-parameter function calling "CGM::funcName()" with a vec{2,3,4}.
#define DEFINE_V( funcName ) \
   int funcName( VMState* vm ) \
   { \
      VMRType* vec = (VMRType*)lua_touserdata( vm, 1 ); \
      if( !vec ) \
      { \
         StdErr << "ERROR - " #funcName " called on non-vector type." << nl; \
         return 0; \
      } \
      switch( vec->_type ) \
      { \
         case VMMath::VEC2: VMMath::push( vm, CGM::funcName(((VMVec2*)vec)->_val) ); return 1; \
         case VMMath::VEC3: VMMath::push( vm, CGM::funcName(((VMVec3*)vec)->_val) ); return 1; \
         case VMMath::VEC4: VMMath::push( vm, CGM::funcName(((VMVec4*)vec)->_val) ); return 1; \
         default:; \
      } \
      return 0; \
   }

// Defines a 1-parameter function calling "CGM::funcName()" with either a float or a vec{2,3,4}.
#define DEFINE_FV( funcName ) \
   int funcName( VMState* vm ) \
   { \
      VMRType* vec = (VMRType*)lua_touserdata( vm, 1 ); \
      if( !vec ) \
      { \
         VM::push( vm, CGM::funcName( VM::toFloat( vm, 1 ) ) ); \
         return 1; \
      } \
      switch( vec->_type ) \
      { \
         case VMMath::VEC2: VMMath::push( vm, CGM::funcName(((VMVec2*)vec)->_val) ); return 1; \
         case VMMath::VEC3: VMMath::push( vm, CGM::funcName(((VMVec3*)vec)->_val) ); return 1; \
         case VMMath::VEC4: VMMath::push( vm, CGM::funcName(((VMVec4*)vec)->_val) ); return 1; \
         default:; \
      } \
      return 0; \
   }

// Defines a 2-parameter function calling "CGM::funcName( op1, op2 )" with 2 vec{2,3,4}.
#define DEFINE_V_V( funcName ) \
   int funcName( VMState* vm ) \
   { \
      CHECK( VM::getTop( vm ) == 2 ); \
      VMRType* op1 = (VMRType*)lua_touserdata( vm, 1 ); \
      VMRType* op2 = (VMRType*)lua_touserdata( vm, 2 ); \
      IF_ERR( op1->_type != op2->_type, "Type mismatch on " #funcName "(" << typeToStr(op1->_type) << "," << typeToStr(op2->_type) << ")." ); \
      switch( op1->_type ) \
      { \
         case VMMath::VEC2: VM::push( vm, CGM::funcName( ((VMVec2*)op1)->_val, ((VMVec2*)op2)->_val ) ); return 1; \
         case VMMath::VEC3: VM::push( vm, CGM::funcName( ((VMVec3*)op1)->_val, ((VMVec3*)op2)->_val ) ); return 1; \
         case VMMath::VEC4: VM::push( vm, CGM::funcName( ((VMVec4*)op1)->_val, ((VMVec4*)op2)->_val ) ); return 1; \
         default:; \
      } \
      return 0; \
   }

// Defines a 2-parameter function calling "CGM::funcName( op1, op2 )" with either floats or vec{2,3,4}.
#define DEFINE_FV_FV( funcName ) \
   int funcName( VMState* vm ) \
   { \
      VMRType* op1 = (VMRType*)lua_touserdata( vm, 1 ); \
      if( !op1 ) \
      { \
         VM::push( vm, CGM::funcName( VM::toFloat( vm, 1 ), VM::toFloat( vm, 2 ) ) ); \
         return 1; \
      } \
      VMRType* op2 = (VMRType*)lua_touserdata( vm, 2 ); \
      IF_ERR( op1->_type != op2->_type, "Type mismatch on " #funcName "(" << typeToStr(op1->_type) << "," << typeToStr(op2->_type) << ")." ); \
      switch( op1->_type ) \
      { \
         case VMMath::VEC2: VMMath::push( vm, CGM::funcName( ((VMVec2*)op1)->_val, ((VMVec2*)op2)->_val ) ); return 1; \
         case VMMath::VEC3: VMMath::push( vm, CGM::funcName( ((VMVec3*)op1)->_val, ((VMVec3*)op2)->_val ) ); return 1; \
         case VMMath::VEC4: VMMath::push( vm, CGM::funcName( ((VMVec4*)op1)->_val, ((VMVec4*)op2)->_val ) ); return 1; \
         default:; \
      } \
      return 0; \
   }

//------------------------------------------------------------------------------
//!
DEFINE_V_V( dot )

//------------------------------------------------------------------------------
//!
int cross( VMState* vm )
{
   CHECK( VM::getTop( vm ) == 2 );
   VMRType* op1 = (VMRType*)lua_touserdata( vm, 1 );
   VMRType* op2 = (VMRType*)lua_touserdata( vm, 2 );
   IF_ERR( op1->_type != op2->_type, "Type mismatch on cross(" << typeToStr(op1->_type) << "," << typeToStr(op2->_type) << ")." );

   switch( op1->_type )
   {
      case VMMath::VEC2: VM::push( vm, CGM::pseudoCross( ((VMVec2*)op1)->_val, ((VMVec2*)op2)->_val ) ); return 1;
      case VMMath::VEC3: VMMath::push( vm, CGM::cross( ((VMVec3*)op1)->_val, ((VMVec3*)op2)->_val ) ); return 1;
      case VMMath::VEC4: VMMath::push( vm, CGM::cross( ((VMVec4*)op1)->_val, ((VMVec4*)op2)->_val ) ); return 1;
      default:;
   }

   return 0;
}

DEFINE_FV( abs )
DEFINE_FV_FV( min )
DEFINE_FV_FV( max )

//-----------------------------------------------------------------------------
//!
int clamp( VMState* vm )
{
   VMRType* op1 = (VMRType*)lua_touserdata( vm, 1 );
   if( !op1 )
   {
      VM::push( vm, CGM::clamp( VM::toFloat( vm, 1 ), VM::toFloat( vm, 2 ), VM::toFloat( vm, 3 ) ) );
      return 1;
   }
   VMRType* op2 = (VMRType*)lua_touserdata( vm, 2 );
   if( !op2 )
   {
      float minVal = VM::toFloat( vm, 2 );
      float maxVal = VM::toFloat( vm, 2 );
      switch( op1->_type )
      {
         case VMMath::VEC2: VMMath::push( vm, CGM::clamp( ((VMVec2*)op1)->_val, minVal, maxVal ) ); return 1;
         case VMMath::VEC3: VMMath::push( vm, CGM::clamp( ((VMVec3*)op1)->_val, minVal, maxVal ) ); return 1;
         case VMMath::VEC4: VMMath::push( vm, CGM::clamp( ((VMVec4*)op1)->_val, minVal, maxVal ) ); return 1;
         default:
            IF_ERR( true, "Invalid call to clamp(" << typeToStr(op1->_type) << ", float, float)." );
            return 0;
      }
      return 1;
   }
   VMRType* op3 = (VMRType*)lua_touserdata( vm, 3 );
   IF_ERR( (op1->_type+op2->_type+op3->_type) != 3*op1->_type, "Invalid call to clamp(" << typeToStr(op1->_type) << ", " << typeToStr(op2->_type) << ", " << typeToStr(op3->_type) << ")." );
   switch( op1->_type )
   {
      case VMMath::VEC2: VMMath::push( vm, CGM::clamp( ((VMVec2*)op1)->_val, ((VMVec2*)op2)->_val, ((VMVec2*)op3)->_val ) ); return 1;
      case VMMath::VEC3: VMMath::push( vm, CGM::clamp( ((VMVec3*)op1)->_val, ((VMVec3*)op2)->_val, ((VMVec3*)op3)->_val ) ); return 1;
      case VMMath::VEC4: VMMath::push( vm, CGM::clamp( ((VMVec4*)op1)->_val, ((VMVec4*)op2)->_val, ((VMVec4*)op3)->_val ) ); return 1;
      default:
         IF_ERR( true, "Invalid call to clamp(" << typeToStr(op1->_type) << ", " << typeToStr(op2->_type) << ", " << typeToStr(op3->_type) << ")." );
         return 0;
   }
   return 0;
}

//------------------------------------------------------------------------------
//!
int mix( VMState* vm )
{
   VMRType* op1 = (VMRType*)lua_touserdata( vm, 1 );
   VMRType* op2 = (VMRType*)lua_touserdata( vm, 2 );
   float t      = VM::toFloat( vm, 3 );

   if( !op1 )
   {
      VM::push( vm, VM::toFloat( vm, 1 )*(1.0f-t) + VM::toFloat( vm, 2 )*t );
      return 1;
   }

   IF_ERR( op1->_type != op2->_type, "Type mismatch on mix(" << typeToStr(op1->_type) << "," << typeToStr(op2->_type) << ")." );

   switch( op1->_type )
   {
      case VMMath::VEC2: VMMath::push( vm, ((VMVec2*)op1)->_val*(1.0f-t) + ((VMVec2*)op2)->_val*t ); return 1;
      case VMMath::VEC3: VMMath::push( vm, ((VMVec3*)op1)->_val*(1.0f-t) + ((VMVec3*)op2)->_val*t ); return 1;
      case VMMath::VEC4: VMMath::push( vm, ((VMVec4*)op1)->_val*(1.0f-t) + ((VMVec4*)op2)->_val*t ); return 1;
      default:;
   }

   return 0;
}

DEFINE_FV( floor )
DEFINE_FV( ceil  )
DEFINE_FV( round )
DEFINE_FV( fract )

DEFINE_V( length )

//------------------------------------------------------------------------------
//!
int maxLength( VMState* vm )
{
   VMRType* vec = (VMRType*)lua_touserdata( vm, 1 );
   float    val = (float)lua_tonumber( vm, 2 );
   switch( vec->_type )
   {
      case VMMath::VEC2:
         VM::push( vm, ((VMVec2*)vec)->_val.maxLength(val) );
         return 1;
      case VMMath::VEC3:
         VM::push( vm, ((VMVec3*)vec)->_val.maxLength(val) );
         return 1;
      case VMMath::VEC4:
         VM::push( vm, ((VMVec4*)vec)->_val.maxLength(val) );
      default:;
   }
   return 0;
}

DEFINE_F( sqrt )

//------------------------------------------------------------------------------
//!
int normalize( VMState* vm )
{
   VMRType* vec = (VMRType*)lua_touserdata( vm, 1 );

   switch( vec->_type )
   {
      case VMMath::VEC2: VMMath::push( vm, ((VMVec2*)vec)->_val.getNormalized() ); return 1;
      case VMMath::VEC3: VMMath::push( vm, ((VMVec3*)vec)->_val.getNormalized() ); return 1;
      case VMMath::VEC4: VMMath::push( vm, ((VMVec4*)vec)->_val.getNormalized() ); return 1;
      case VMMath::QUAT: VMMath::push( vm, ((VMQuat*)vec)->_val.getNormalized() ); return 1;
      default:;
   }

   return 0;
}

//------------------------------------------------------------------------------
//!
int inverse( VMState* vm )
{
   VMRType* vec = (VMRType*)lua_touserdata( vm, 1 );

   if( vec != NULL )
   {
      switch( vec->_type )
      {
         case VMMath::QUAT: VMMath::push( vm, ((VMQuat*)vec)->_val.getInversed() ); return 1;
         case VMMath::MAT2: VMMath::push( vm, ((VMMat2*)vec)->_val.getInversed() ); return 1;
         case VMMath::MAT3: VMMath::push( vm, ((VMMat3*)vec)->_val.getInversed() ); return 1;
         case VMMath::MAT4: VMMath::push( vm, ((VMMat4*)vec)->_val.getInversed() ); return 1;
         default:;
      }
   }

   return 0;
}

//------------------------------------------------------------------------------
//!
int perlin1( VMState* vm )
{
   VMRType* vec = (VMRType*)lua_touserdata( vm, 1 );
   switch( vec->_type )
   {
      case VMMath::VEC2:
         VM::push( vm, CGM::perlinNoise1( Vec3f(((VMVec2*)vec)->_val) ) );
         return 1;
      case VMMath::VEC3:
         VM::push( vm, CGM::perlinNoise1( ((VMVec3*)vec)->_val ) );
         return 1;
      case VMMath::VEC4:
         return 0;
      default:;
   }
   return 0;
}

//------------------------------------------------------------------------------
//!
int cellnoise1( VMState* vm )
{
   VMRType* vec = (VMRType*)lua_touserdata( vm, 1 );
   switch( vec->_type )
   {
      case VMMath::VEC2:
         VM::push( vm, CGM::cellNoise1( Vec3f(((VMVec2*)vec)->_val) ) );
         return 1;
      case VMMath::VEC3:
         VM::push( vm, CGM::cellNoise1( ((VMVec3*)vec)->_val ) );
         return 1;
      case VMMath::VEC4:
         return 0;
      default:;
   }
   return 0;
}

//------------------------------------------------------------------------------
//!
int filteredPerlin1( VMState* vm )
{
   VMRType* vec = (VMRType*)lua_touserdata( vm, 1 );
   float p      = VM::toFloat(vm, 2);
   switch( vec->_type )
   {
      case VMMath::VEC2:
         VM::push( vm, CGM::filteredPerlinNoise1( Vec3f(((VMVec2*)vec)->_val), p ) );
         return 1;
      case VMMath::VEC3:
         VM::push( vm, CGM::filteredPerlinNoise1( ((VMVec3*)vec)->_val, p ) );
         return 1;
      case VMMath::VEC4:
         return 0;
      default:;
   }
   return 0;
}

//------------------------------------------------------------------------------
//!
#define DECL_NOISE_FUNC( cFunc, cppFunc ) \
   int cFunc( VMState* vm ) \
   { \
      VMRType* vec = (VMRType*)lua_touserdata( vm, 1 ); \
      switch( vec->_type ) \
      { \
         case VMMath::VEC2: \
         { \
            const Vec2f& v = ((VMVec2*)vec)->_val; \
            VMMath::push( vm, CGM::cppFunc( Vec3f(v) ) ); \
            return 1; \
         } \
         case VMMath::VEC3: \
         { \
            const Vec3f& v = ((VMVec3*)vec)->_val; \
            VMMath::push( vm, CGM::cppFunc( v ) ); \
            return 1; \
         } \
         case VMMath::VEC4: \
         { \
            return 0; \
         } \
         default:; \
      } \
      return 0; \
   }

#define DECL_NOISE_FUNC_P1( cFunc, cppFunc, pExpr ) \
   int cFunc( VMState* vm ) \
   { \
      VMRType* vec = (VMRType*)lua_touserdata( vm, 1 ); \
      pExpr; \
      switch( vec->_type ) \
      { \
         case VMMath::VEC2: \
         { \
            const Vec2f& v = ((VMVec2*)vec)->_val; \
            VMMath::push( vm, CGM::cppFunc( Vec3f(v), p ) ); \
            return 1; \
         } \
         case VMMath::VEC3: \
         { \
            const Vec3f& v = ((VMVec3*)vec)->_val; \
            VMMath::push( vm, CGM::cppFunc( v, p ) ); \
            return 1; \
         } \
         case VMMath::VEC4: \
         { \
            return 0; \
         } \
         default:; \
      } \
      return 0; \
   }

DECL_NOISE_FUNC( perlin2, perlinNoise2 )
DECL_NOISE_FUNC( perlin3, perlinNoise3 )
DECL_NOISE_FUNC( cellnoise2, cellNoise2 )
DECL_NOISE_FUNC( cellnoise3, cellNoise3 )
DECL_NOISE_FUNC_P1( filteredPerlin2, filteredPerlinNoise2, float p = VM::toFloat(vm, 2) )
DECL_NOISE_FUNC_P1( filteredPerlin3, filteredPerlinNoise3, float p = VM::toFloat(vm, 2) )

//------------------------------------------------------------------------------
//!
int tex( VMState* vm )
{
   String id = VM::toString( vm, 1 );
   VMRType* coord = (VMRType*)lua_touserdata( vm, 2 );
   const char* param = VM::toCString( vm, 3 );
   // TODO: optional third argument, a table with named parameters (filter, lodbias, etc.).

   RCP<Image> img = data( ResManager::getImage( id ) );
   if( img.isNull() )  return 0;

   Bitmap* tex = img->bitmap();

   CHECK( coord->_type == VMMath::VEC2 );

   if( param == NULL || param[0] == 'b' )
   {
      // "bilinear" or "box".
      VMMath::push( vm, BitmapManipulator::bilinear(*tex, ((VMVec2*)coord)->_val) );
   }
   else
   {
      // "point" or "nearest".
      VMMath::push( vm, BitmapManipulator::nearest(*tex, ((VMVec2*)coord)->_val) );
   }

   return 1;
}

//------------------------------------------------------------------------------
//! Maps the CGMath::smoothStep( minVal, maxVal, val ) routine.
//! Accepts the following formats:
//!   smoothStep( f, f, f )
//!   smoothStep( f, f, v )
//!   smoothStep( v, v, v )
int smoothStep( VMState* vm )
{
   if( VM::isObject( vm, 1 ) )
   {
      // smoothStep( v, v, v )
      VMRType* minVal = (VMRType*)lua_touserdata( vm, 1 );
      IF_ERR( !VM::isObject( vm, 2 ), "Type mismatch on smoothStep(): minVal is a " << typeToStr(minVal->_type) << " but maxVal isn't even an object." );

      VMRType* maxVal = (VMRType*)lua_touserdata( vm, 2 );
      IF_ERR( minVal->_type != maxVal->_type, "Type mismatch on smoothStep(): minVal is " << typeToStr(minVal->_type) << " while maxVal is " << typeToStr(maxVal->_type) << "." );

      IF_ERR( !VM::isObject( vm, 3 ), "Type mismatch on smoothStep(): minVal and maxVal are " << typeToStr(minVal->_type) << " while val isn't even an object." );

      VMRType* val = (VMRType*)lua_touserdata( vm, 3 );
      IF_ERR( minVal->_type != val->_type, "Type mismatch on smoothStep(): minVal and maxVal are " << typeToStr(minVal->_type) << " while val is " << typeToStr(val->_type) << "." );
      switch( minVal->_type )
      {
         case VMMath::VEC2: VMMath::push( vm, CGM::smoothStep( ((VMVec2*)minVal)->_val, ((VMVec2*)maxVal)->_val, ((VMVec2*)val)->_val ) ); return 1;
         case VMMath::VEC3: VMMath::push( vm, CGM::smoothStep( ((VMVec3*)minVal)->_val, ((VMVec3*)maxVal)->_val, ((VMVec3*)val)->_val ) ); return 1;
         case VMMath::VEC4: VMMath::push( vm, CGM::smoothStep( ((VMVec4*)minVal)->_val, ((VMVec4*)maxVal)->_val, ((VMVec4*)val)->_val ) ); return 1;
      }
   }
   else
   {
      float minVal = (float)lua_tonumber( vm, 1 );
      float maxVal = (float)lua_tonumber( vm, 2 );
      if( VM::isObject( vm, 3 ) )
      {
         // smoothStep( f, f, v )
         VMRType* val = (VMRType*)lua_touserdata( vm, 3 );
         switch( val->_type )
         {
            case VMMath::VEC2: VMMath::push( vm, CGM::smoothStep( minVal, maxVal, ((VMVec2*)val)->_val ) ); return 1;
            case VMMath::VEC3: VMMath::push( vm, CGM::smoothStep( minVal, maxVal, ((VMVec3*)val)->_val ) ); return 1;
            case VMMath::VEC4: VMMath::push( vm, CGM::smoothStep( minVal, maxVal, ((VMVec4*)val)->_val ) ); return 1;
         }
      }
      else
      {
         // smoothStep( f, f, f )
         float val = (float)lua_tonumber( vm, 3 );
         VM::push( vm, CGM::smoothStep( minVal, maxVal, val ) );
         return 1;
      }
   }

   return 0;
}

//------------------------------------------------------------------------------
//!
int counter( VMState* vm )
{
   switch( VM::getTop(vm) )
   {
      case 0:
         VMMath::push( vm, VMCounter() );
         return 1;
      case 1:
         VMMath::push( vm, VMCounter( VM::toNumber(vm, 1) ) );
         return 1;
      case 2:
         VMMath::push( vm, VMCounter( VM::toNumber(vm, 1), VM::toNumber(vm, 2) ) );
         return 1;
   }
   return 0;
}

//------------------------------------------------------------------------------
//! Forward-declaration.
int rng_create( VMState* vm );

//------------------------------------------------------------------------------
//!
int sinVM( VMState* vm )
{
   VMRType* vec = (VMRType*)lua_touserdata( vm, 1 );

   if( !vec )
   {
      VM::push( vm, CGM::sin( CGM::cirToRad(VM::toFloat( vm, 1 )) ) );
      return 1;
   }

   switch( vec->_type )
   {
      case VMMath::VEC2:
      {
         Vec2f c = CGM::cirToRad( ((VMVec2*)vec)->_val );
         VMMath::push( vm, Vec2f( CGM::sin(c.x), CGM::sin(c.y) ) );
         return 1;
      }
      case VMMath::VEC3:
      {
         Vec3f c = CGM::cirToRad( ((VMVec3*)vec)->_val );
         VMMath::push( vm, Vec3f( CGM::sin(c.x), CGM::sin(c.y), CGM::sin(c.z) ) );
         return 1;
      }
      case VMMath::VEC4:
      {
         Vec4f c = CGM::cirToRad( ((VMVec4*)vec)->_val );
         VMMath::push( vm, Vec4f( CGM::sin(c.x), CGM::sin(c.y), CGM::sin(c.z), CGM::sin(c.w) ) );
         return 1;
      }
      default:;
   }
   return 0;
}

//------------------------------------------------------------------------------
//!
int cosVM( VMState* vm )
{
   VMRType* vec = (VMRType*)lua_touserdata( vm, 1 );

   if( !vec )
   {
      VM::push( vm, CGM::cos( CGM::cirToRad(VM::toFloat( vm, 1 )) ) );
      return 1;
   }

   switch( vec->_type )
   {
      case VMMath::VEC2:
      {
         Vec2f c = CGM::cirToRad( ((VMVec2*)vec)->_val );
         VMMath::push( vm, Vec2f( CGM::cos(c.x), CGM::cos(c.y) ) );
         return 1;
      }
      case VMMath::VEC3:
      {
         Vec3f c = CGM::cirToRad( ((VMVec3*)vec)->_val );
         VMMath::push( vm, Vec3f( CGM::cos(c.x), CGM::cos(c.y), CGM::cos(c.z) ) );
         return 1;
      }
      case VMMath::VEC4:
      {
         Vec4f c = CGM::cirToRad( ((VMVec4*)vec)->_val );
         VMMath::push( vm, Vec4f( CGM::cos(c.x), CGM::cos(c.y), CGM::cos(c.z), CGM::cos(c.w) ) );
         return 1;
      }
      default:;
   }
   return 0;
}

//------------------------------------------------------------------------------
//!
const VM::Reg math_funcs[] = {
   { "equal"          , equal           },
   { "vec2"           , vec2            },
   { "vec3"           , vec3            },
   { "vec4"           , vec4            },
   { "quat"           , quat            },
   { "mat2"           , mat2            },
   { "mat3"           , mat3            },
   { "mat4"           , mat4            },
   { "dot"            , dot             },
   { "cross"          , cross           },
   { "abs"            , abs             },
   { "min"            , min             },
   { "max"            , max             },
   { "clamp"          , clamp           },
   { "floor"          , floor           },
   { "ceil"           , ceil            },
   { "round"          , round           },
   { "fract"          , fract           },
   { "sqrt"           , sqrt            },
   { "mix"            , mix             },
   { "length"         , length          },
   { "maxLength"      , maxLength       },
   { "normalize"      , normalize       },
   { "inverse"        , inverse         },
   { "perlin1"        , perlin1         },
   { "perlin2"        , perlin2         },
   { "perlin3"        , perlin3         },
   { "cellnoise1"     , cellnoise1      },
   { "cellnoise2"     , cellnoise2      },
   { "cellnoise3"     , cellnoise3      },
   { "filteredPerlin1", filteredPerlin1 },
   { "filteredPerlin2", filteredPerlin2 },
   { "filteredPerlin3", filteredPerlin3 },
   { "tex"            , tex             },
   { "smoothStep"     , smoothStep      },
   { "counter"        , counter         },
   { "sin"            , sinVM           },
   { "cos"            , cosVM           },
   { "RNG"            , rng_create      },
   { 0, 0 }
};

//------------------------------------------------------------------------------
//!
int vec_get( VMState* vm )
{
   VMRType* ud = (VMRType*)lua_touserdata( vm, 1 );
   const char* k;
   if( VM::isNumber( vm, 2 ) )
   {
      const char* idxToStr[] = {
      //<=0   1    2    3    4   >=5
         "", "x", "y", "z", "w", ""
      };
      uint idx = VM::toUInt( vm, 2 );
      idx = CGM::clamp( idx, (uint)0, (uint)5 );
      k = idxToStr[idx];
   }
   else
   {
      k = VM::toCString( vm, 2 );
   }

   // Swizzling (including single-term accessors).
   // The following code allows various special cases:
   //   vec2.xxyy => vec4
   //   vec4.xyz0 => vec4
   //   vec4.xyz1 => vec4
   // and silently rejects (i.e. returns nil) invalid cases, such as:
   //   vec2.z
   //   vec3.w
   //   vec.blah

   uint n = uint(strlen( k ));
   IF_ERR( n > 4, "Invalid swizzle code: " << k << "." )

   bool ok = true;
   float tmp[4];
   for( uint i = 0; i < n; ++i )
   {
      switch( k[i] )
      {
         case 'x':
            tmp[i] = ud->_val[0];
            break;
         case 'y':
            tmp[i] = ud->_val[1];
            break;
         case 'z':
            if( ud->_type >= VMMath::VEC3 )  tmp[i] = ud->_val[2];
            else                             ok = false;
            break;
         case 'w':
            if( ud->_type >= VMMath::VEC4 )  tmp[i] = ud->_val[3];
            else                             ok = false;
            break;
         default:
            ok = false;
            break;
      }
   }

   if( ok )
   {
      // Note: For speed improvements, could recode the push code by hand here and avoid the switch.
      switch( n )
      {
         case 1: VM::push( vm, tmp[0] ); return 1;
         case 2: VMMath::push( vm, Vec2f(tmp[0], tmp[1]) ); return 1;
         case 3: VMMath::push( vm, Vec3f(tmp[0], tmp[1], tmp[2]) ); return 1;
         case 4: VMMath::push( vm, Vec4f(tmp[0], tmp[1], tmp[2], tmp[3]) ); return 1;
      }
   }

   return 0;
}

//------------------------------------------------------------------------------
//!
int vec_set( VMState* vm )
{
   VMRType* dst = (VMRType*)lua_touserdata( vm, 1 );

   const char* k;
   if( VM::isNumber( vm, 2 ) )
   {
      const char* idxToStr[] = {
      //<=0   1    2    3    4   >=5
         "", "x", "y", "z", "w", ""
      };
      uint idx = VM::toUInt( vm, 2 );
      idx = CGM::clamp( idx, (uint)0, (uint)5 );
      k = idxToStr[idx];
   }
   else
   {
      k = VM::toCString( vm, 2 );
   }
   uint n = uint(strlen( k ));
   const uint n_max = dst->_type - VMMath::VEC2 + 2;
   IF_ERR( n > n_max, "Too many swizzle selectors (" << n << ") for a destination " << typeToStr(dst->_type) << "." )

   VMRType* src = (VMRType*)lua_touserdata( vm, 3 );
   float   tmpF;
   float*  srcF;
   if( src )
   {
      srcF = src->_val;
   }
   else
   {
      // Assigning a scalar.
      CHECK( n == 1 );
      tmpF = (float)lua_tonumber( vm, 3 );
      srcF = &tmpF;
   }

   uint8_t used = 0x00;
   for( uint i = 0; i < n; ++i )
   {
      switch( k[i] )
      {
         case 'x':
            IF_ERR( used & 0x01, "Duplicated destination channel selector: " << k[i] << " on a Vec" << n_max << "." )
            dst->_val[0] = srcF[i];
            used |= 0x01;
            break;
         case 'y':
            IF_ERR( used & 0x02, "Duplicated destination channel selector: " << k[i] << " on a Vec" << n_max << "." )
            dst->_val[1] = srcF[i];
            used |= 0x02;
            break;
         case 'z':
            IF_ERR( used & 0x04, "Duplicated destination channel selector: " << k[i] << " on a Vec" << n_max << "." )
            IF_ERR( dst->_type < VMMath::VEC3, "Invalid channel selector: " << k[i] << " on a Vec" << n_max << "." )
            dst->_val[2] = srcF[i];
            used |= 0x04;
            break;
         case 'w':
            IF_ERR( used & 0x08, "Duplicated destination channel selector: " << k[i] << " on a Vec" << n_max << "." )
            IF_ERR( dst->_type < VMMath::VEC4, "Invalid channel selector: " << k[i] << " on a Vec" << n_max << "." )
            dst->_val[3] = srcF[i];
            used |= 0x08;
            break;
         default:
            break;
      }
   }

   return 0;
}

//------------------------------------------------------------------------------
//!
int vec_add( VMState* vm )
{
   VMRType* op1 = (VMRType*)lua_touserdata( vm, 1 );
   VMRType* op2 = (VMRType*)lua_touserdata( vm, 2 );

   if( !op1 )
   {
      float v = VM::toFloat( vm, 1 );
      switch( op2->_type )
      {
         case VMMath::VEC2: VMMath::push( vm, ((VMVec2*)op2)->_val + v ); return 1;
         case VMMath::VEC3: VMMath::push( vm, ((VMVec3*)op2)->_val + v ); return 1;
         case VMMath::VEC4: VMMath::push( vm, ((VMVec4*)op2)->_val + v ); return 1;
         default:
            IF_ERR( true, "Unsupported float+" << typeToStr(op2->_type) << "." );
            return 0;
      }
   }
   if( !op2 )
   {
      float v = VM::toFloat( vm, 2 );
      switch( op1->_type )
      {
         case VMMath::VEC2: VMMath::push( vm, ((VMVec2*)op1)->_val + v ); return 1;
         case VMMath::VEC3: VMMath::push( vm, ((VMVec3*)op1)->_val + v ); return 1;
         case VMMath::VEC4: VMMath::push( vm, ((VMVec4*)op1)->_val + v ); return 1;
         default:
            IF_ERR( true, "Unsupported " << typeToStr(op1->_type) << "+float." );
            return 0;
      }
   }

   IF_ERR( op1->_type != op2->_type, "Type mismatch on " << typeToStr(op1->_type) << "+" << typeToStr(op2->_type) << "." );

   switch( op1->_type )
   {
      case VMMath::VEC2: VMMath::push( vm, ((VMVec2*)op1)->_val + ((VMVec2*)op2)->_val ); return 1;
      case VMMath::VEC3: VMMath::push( vm, ((VMVec3*)op1)->_val + ((VMVec3*)op2)->_val ); return 1;
      case VMMath::VEC4: VMMath::push( vm, ((VMVec4*)op1)->_val + ((VMVec4*)op2)->_val ); return 1;
      default:
         IF_ERR( true, "Unsupported " << typeToStr(op1->_type) << "+" << typeToStr(op2->_type) << "." );
   }

   return 0;
}

//------------------------------------------------------------------------------
//!
int vec_sub( VMState* vm )
{
   VMRType* op1 = (VMRType*)lua_touserdata( vm, 1 );
   VMRType* op2 = (VMRType*)lua_touserdata( vm, 2 );

   if( !op1 )
   {
      float v = VM::toFloat( vm, 1 );
      switch( op2->_type )
      {
         case VMMath::VEC2: VMMath::push( vm, Vec2f(v) - ((VMVec2*)op2)->_val ); return 1;
         case VMMath::VEC3: VMMath::push( vm, Vec3f(v) - ((VMVec3*)op2)->_val ); return 1;
         case VMMath::VEC4: VMMath::push( vm, Vec4f(v) - ((VMVec4*)op2)->_val ); return 1;
         default:
            IF_ERR( true, "Unsupported float-" << typeToStr(op2->_type) << "." );
            return 0;
      }
   }
   if( !op2 )
   {
      float v = VM::toFloat( vm, 2 );
      switch( op1->_type )
      {
         case VMMath::VEC2: VMMath::push( vm, ((VMVec2*)op1)->_val - v ); return 1;
         case VMMath::VEC3: VMMath::push( vm, ((VMVec3*)op1)->_val - v ); return 1;
         case VMMath::VEC4: VMMath::push( vm, ((VMVec4*)op1)->_val - v ); return 1;
         default:
            IF_ERR( true, "Unsupported " << typeToStr(op1->_type) << "-float." );
            return 0;
      }
   }

   IF_ERR( op1->_type != op2->_type, "Type mismatch on " << typeToStr(op1->_type) << "-" << typeToStr(op2->_type) << "." );

   switch( op1->_type )
   {
      case VMMath::VEC2: VMMath::push( vm, ((VMVec2*)op1)->_val - ((VMVec2*)op2)->_val ); return 1;
      case VMMath::VEC3: VMMath::push( vm, ((VMVec3*)op1)->_val - ((VMVec3*)op2)->_val ); return 1;
      case VMMath::VEC4: VMMath::push( vm, ((VMVec4*)op1)->_val - ((VMVec4*)op2)->_val ); return 1;
      default:
         IF_ERR( true, "Unsupported " << typeToStr(op1->_type) << "-" << typeToStr(op2->_type) << "." );
   }

   return 0;
}

//------------------------------------------------------------------------------
//!
int vec_mul( VMState* vm )
{
   VMRType* op1 = (VMRType*)lua_touserdata( vm, 1 );
   VMRType* op2 = (VMRType*)lua_touserdata( vm, 2 );

   if( !op1 )
   {
      float v = VM::toFloat( vm, 1 );
      switch( op2->_type )
      {
         case VMMath::VEC2: VMMath::push( vm, ((VMVec2*)op2)->_val * v ); return 1;
         case VMMath::VEC3: VMMath::push( vm, ((VMVec3*)op2)->_val * v ); return 1;
         case VMMath::VEC4: VMMath::push( vm, ((VMVec4*)op2)->_val * v ); return 1;
         default:
            IF_ERR( true, "Unsupported float*" << typeToStr(op2->_type) << "." );
            return 0;
      }
   }
   if( !op2 )
   {
      float v = VM::toFloat( vm, 2 );
      switch( op1->_type )
      {
         case VMMath::VEC2: VMMath::push( vm, ((VMVec2*)op1)->_val * v ); return 1;
         case VMMath::VEC3: VMMath::push( vm, ((VMVec3*)op1)->_val * v ); return 1;
         case VMMath::VEC4: VMMath::push( vm, ((VMVec4*)op1)->_val * v ); return 1;
         default:
            IF_ERR( true, "Unsupported " << typeToStr(op1->_type) << "*float." );
            return 0;
      }
   }

   IF_ERR( op1->_type != op2->_type, "Type mismatch on " << typeToStr(op1->_type) << "*" << typeToStr(op2->_type) << "." );

   switch( op1->_type )
   {
      case VMMath::VEC2: VMMath::push( vm, ((VMVec2*)op1)->_val * ((VMVec2*)op2)->_val ); return 1;
      case VMMath::VEC3: VMMath::push( vm, ((VMVec3*)op1)->_val * ((VMVec3*)op2)->_val ); return 1;
      case VMMath::VEC4: VMMath::push( vm, ((VMVec4*)op1)->_val * ((VMVec4*)op2)->_val ); return 1;
      default:
         IF_ERR( true, "Unsupported " << typeToStr(op1->_type) << "*" << typeToStr(op2->_type) << "." );
   }

   return 0;
}

//------------------------------------------------------------------------------
//!
int vec_div( VMState* vm )
{
   VMRType* op1 = (VMRType*)lua_touserdata( vm, 1 );
   VMRType* op2 = (VMRType*)lua_touserdata( vm, 2 );

   if( !op1 )
   {
      float v = VM::toFloat( vm, 1 );
      switch( op2->_type )
      {
         case VMMath::VEC2: VMMath::push( vm, Vec2f(v) / ((VMVec2*)op2)->_val ); return 1;
         case VMMath::VEC3: VMMath::push( vm, Vec3f(v) / ((VMVec3*)op2)->_val ); return 1;
         case VMMath::VEC4: VMMath::push( vm, Vec4f(v) / ((VMVec4*)op2)->_val ); return 1;
         default:
            IF_ERR( true, "Unsupported float/" << typeToStr(op2->_type) << "." );
            return 0;
      }
   }
   if( !op2 )
   {
      float v = VM::toFloat( vm, 2 );
      switch( op1->_type )
      {
         case VMMath::VEC2: VMMath::push( vm, ((VMVec2*)op1)->_val / v ); return 1;
         case VMMath::VEC3: VMMath::push( vm, ((VMVec3*)op1)->_val / v ); return 1;
         case VMMath::VEC4: VMMath::push( vm, ((VMVec4*)op1)->_val / v ); return 1;
         default:
            IF_ERR( true, "Unsupported " << typeToStr(op1->_type) << "/float." );
            return 0;
      }
   }

   IF_ERR( op1->_type != op2->_type, "Type mismatch on " << typeToStr(op1->_type) << "/" << typeToStr(op2->_type) << "." );

   switch( op1->_type )
   {
      case VMMath::VEC2: VMMath::push( vm, ((VMVec2*)op1)->_val / ((VMVec2*)op2)->_val ); return 1;
      case VMMath::VEC3: VMMath::push( vm, ((VMVec3*)op1)->_val / ((VMVec3*)op2)->_val ); return 1;
      case VMMath::VEC4: VMMath::push( vm, ((VMVec4*)op1)->_val / ((VMVec4*)op2)->_val ); return 1;
      default:
         IF_ERR( true, "Unsupported " << typeToStr(op1->_type) << "/" << typeToStr(op2->_type) << "." );
   }

   return 0;
}

//------------------------------------------------------------------------------
//!
int vec_unm( VMState* vm )
{
   VMRType* op1 = (VMRType*)lua_touserdata( vm, 1 );

   switch( op1->_type )
   {
      case VMMath::VEC2: VMMath::push( vm, -((VMVec2*)op1)->_val ); return 1;
      case VMMath::VEC3: VMMath::push( vm, -((VMVec3*)op1)->_val ); return 1;
      case VMMath::VEC4: VMMath::push( vm, -((VMVec4*)op1)->_val ); return 1;
      default:;
   }

   return 0;
}

//------------------------------------------------------------------------------
//!
int vec_eq( VMState* vm )
{
   VMRType* op1 = (VMRType*)lua_touserdata( vm, 1 );
   VMRType* op2 = (VMRType*)lua_touserdata( vm, 2 );
   IF_ERR( op1->_type != op2->_type, "Type mismatch on " << typeToStr(op1->_type) << "==" << typeToStr(op2->_type) << "." );

   switch( op1->_type )
   {
      case VMMath::VEC2: VM::push( vm, ((VMVec2*)op1)->_val == ((VMVec2*)op2)->_val ); return 1;
      case VMMath::VEC3: VM::push( vm, ((VMVec3*)op1)->_val == ((VMVec3*)op2)->_val ); return 1;
      case VMMath::VEC4: VM::push( vm, ((VMVec4*)op1)->_val == ((VMVec4*)op2)->_val ); return 1;
      default:;
   }

   return 0;
}

//------------------------------------------------------------------------------
//!
int vec_str( VMState* vm )
{
   VMRType* op1 = (VMRType*)lua_touserdata( vm, 1 );
   String str;
   TextStream stream( str );

   switch( op1->_type )
   {
      case VMMath::VEC2:
         stream << ((VMVec2*)op1)->_val;
         VM::push( vm, str );
         return 1;
      case VMMath::VEC3:
         stream << ((VMVec3*)op1)->_val;
         VM::push( vm, str );
         return 1;
      case VMMath::VEC4:
         stream << ((VMVec4*)op1)->_val;
         VM::push( vm, str );
         return 1;
      default:;
   }

   return 0;
}

//------------------------------------------------------------------------------
//!
int vec_len( VMState* vm )
{
   VMRType* op1 = (VMRType*)lua_touserdata( vm, 1 );

   switch( op1->_type )
   {
      case VMMath::VEC2:
         VM::push( vm, 2 );
         return 1;
      case VMMath::VEC3:
         VM::push( vm, 3 );
         return 1;
      case VMMath::VEC4:
         VM::push( vm, 4 );
         return 1;
      default:;
   }

   return 0;
}

#define DEFINE_QUAT_METHOD( method ) \
   int quat_##method( VMState* vm ) \
   { \
      VMQuat* qPtr = (VMQuat*)VM::thisPtr( vm ); \
      VMMath::push( vm, qPtr->_val.method() ); \
      return 1; \
   }

#define DEFINE_QUAT_METHOD_2( method, Type1, Type2 ) \
   int quat_##method( VMState* vm ) \
   { \
      Type1  arg1; \
      Type2  arg2; \
      VMQuat* qPtr = (VMQuat*)VM::thisPtr( vm ); \
      qPtr->_val.method( arg1, arg2 ); \
      VMMath::push( vm, arg1 ); \
      VMMath::push( vm, arg2 ); \
      return 2; \
   }

DEFINE_QUAT_METHOD( conjugate )
DEFINE_QUAT_METHOD( inverse   )
DEFINE_QUAT_METHOD( negate    )
DEFINE_QUAT_METHOD( normalize )
DEFINE_QUAT_METHOD( toMatrix  )
DEFINE_QUAT_METHOD( toMatrix3 )
DEFINE_QUAT_METHOD_2( toAxisAngle, Vec3f, float )
DEFINE_QUAT_METHOD_2( toAxisCir  , Vec3f, float )
DEFINE_QUAT_METHOD_2( toAxisCos  , Vec3f, float )

//------------------------------------------------------------------------------
//!
int quat_get( VMState* vm )
{
   VMQuat* qPtr = (VMQuat*)lua_touserdata( vm, 1 );
   const char* k;
   if( VM::isNumber( vm, 2 ) )
   {
      const char* idxToStr[] = {
      //<=0   1    2    3    4   >=5
         "", "x", "y", "z", "w", ""
      };
      uint idx = VM::toUInt( vm, 2 );
      idx = CGM::clamp( idx, (uint)0, (uint)5 );
      k = idxToStr[idx];
   }
   else
   {
      k = VM::toCString( vm, 2 );
   }

   // Swizzling (including single-term accessors).
   // The following code allows various special cases:
   //   q.xxyy => vec4
   //   q.xyz0 => vec4
   //   q.xyz1 => vec4
   // and silently rejects (i.e. returns nil) invalid cases, such as:
   //   q.blah

   uint n = uint(strlen( k ));
   if( n > 4 )
   {
      switch( _quat_attr[k] )
      {
         case ATTRIB_CONJUGATE    :  VM::push( vm, qPtr, quat_conjugate   ); return 1;
         case ATTRIB_INVERSE      :  VM::push( vm, qPtr, quat_inverse     ); return 1;
         case ATTRIB_NEGATE       :  VM::push( vm, qPtr, quat_negate      ); return 1;
         case ATTRIB_NORMALIZE    :  VM::push( vm, qPtr, quat_normalize   ); return 1;
         case ATTRIB_TO_AXIS_ANGLE:  VM::push( vm, qPtr, quat_toAxisAngle ); return 1;
         case ATTRIB_TO_AXIS_CIR  :  VM::push( vm, qPtr, quat_toAxisCir   ); return 1;
         case ATTRIB_TO_AXIS_COS  :  VM::push( vm, qPtr, quat_toAxisCos   ); return 1;
         case ATTRIB_TO_MATRIX    :  VM::push( vm, qPtr, quat_toMatrix    ); return 1;
         case ATTRIB_TO_MATRIX3   :  VM::push( vm, qPtr, quat_toMatrix3   ); return 1;
         default: break;
      }
      return 0;
   }

   bool ok = true;
   float tmp[4];
   for( uint i = 0; i < n; ++i )
   {
      switch( k[i] )
      {
         case 'x':
            tmp[i] = qPtr->_val.x();
            break;
         case 'y':
            tmp[i] = qPtr->_val.y();
            break;
         case 'z':
            tmp[i] = qPtr->_val.z();
            break;
         case 'w':
            tmp[i] = qPtr->_val.w();
            break;
         default:
            ok = false;
            break;
      }
   }

   if( ok )
   {
      // Note: For speed improvements, could recode the push code by hand here and avoid the switch.
      switch( n )
      {
         case 1: VM::push( vm, tmp[0] ); return 1;
         case 2: VMMath::push( vm, Vec2f(tmp[0], tmp[1]) ); return 1;
         case 3: VMMath::push( vm, Vec3f(tmp[0], tmp[1], tmp[2]) ); return 1;
         case 4: VMMath::push( vm, Vec4f(tmp[0], tmp[1], tmp[2], tmp[3]) ); return 1;
      }
   }

   return 0;
}

//------------------------------------------------------------------------------
//!
int quat_set( VMState* vm )
{
   VMRType* dst = (VMRType*)lua_touserdata( vm, 1 );

   const char* k;
   if( VM::isNumber( vm, 2 ) )
   {
      const char* idxToStr[] = {
      //<=0   1    2    3    4   >=5
         "", "x", "y", "z", "w", ""
      };
      uint idx = VM::toUInt( vm, 2 );
      idx = CGM::clamp( idx, (uint)0, (uint)5 );
      k = idxToStr[idx];
   }
   else
   {
      k = VM::toCString( vm, 2 );
   }
   uint n = uint(strlen( k ));
   const uint n_max = 4;
   IF_ERR( n > n_max, "Too many swizzle selectors (" << n << ") for a destination " << typeToStr(dst->_type) << "." )

   VMRType* src = (VMRType*)lua_touserdata( vm, 3 );
   float   tmpF;
   float*  srcF;
   if( src )
   {
      srcF = src->_val;
   }
   else
   {
      // Assigning a scalar.
      CHECK( n == 1 );
      tmpF = (float)lua_tonumber( vm, 3 );
      srcF = &tmpF;
   }

   uint8_t used = 0x00;
   for( uint i = 0; i < n; ++i )
   {
      switch( k[i] )
      {
         case 'x':
            IF_ERR( used & 0x01, "Duplicated destination channel selector: " << k[i] << " on a Quat." )
            dst->_val[0] = srcF[i];
            used |= 0x01;
            break;
         case 'y':
            IF_ERR( used & 0x02, "Duplicated destination channel selector: " << k[i] << " on a Quat." )
            dst->_val[1] = srcF[i];
            used |= 0x02;
            break;
         case 'z':
            IF_ERR( used & 0x04, "Duplicated destination channel selector: " << k[i] << " on a Quat." )
            dst->_val[2] = srcF[i];
            used |= 0x04;
            break;
         case 'w':
            IF_ERR( used & 0x08, "Duplicated destination channel selector: " << k[i] << " on a Quat." )
            dst->_val[3] = srcF[i];
            used |= 0x08;
            break;
         default:
            break;
      }
   }

   return 0;
}

//------------------------------------------------------------------------------
//!
int quat_eq( VMState* vm )
{
   VMRType* op1 = (VMRType*)lua_touserdata( vm, 1 );
   VMRType* op2 = (VMRType*)lua_touserdata( vm, 2 );
   IF_ERR( op1->_type != VMMath::QUAT, "Non-quaternion comparison " << typeToStr(op1->_type) << "==" << typeToStr(op2->_type) << "." );
   IF_ERR( op1->_type != op2->_type, "Type mismatch on " << typeToStr(op1->_type) << "==" << typeToStr(op2->_type) << "." );
   VM::push( vm, ((VMQuat*)op1)->_val == ((VMQuat*)op2)->_val );
   return 1;
}

//------------------------------------------------------------------------------
//!
int quat_mul( VMState* vm )
{
   VMRType* op1 = (VMRType*)lua_touserdata( vm, 1 );
   VMRType* op2 = (VMRType*)lua_touserdata( vm, 2 );

   if( !op1 )
   {
      float v = VM::toFloat( vm, 1 );
      switch( op2->_type )
      {
         case VMMath::QUAT: VMMath::push( vm, ((VMQuat*)op2)->_val * v ); return 1;
         default:
            IF_ERR( true, "Unsupported Quat multiplication between float and " << typeToStr(op2->_type) << "." );
            return 0;
      }
   }
   if( !op2 )
   {
      float v = VM::toFloat( vm, 2 );
      switch( op1->_type )
      {
         case VMMath::QUAT: VMMath::push( vm, ((VMQuat*)op1)->_val * v ); return 1;
         default:
            IF_ERR( true, "Unsupported Quat multiplication between " << typeToStr(op1->_type) << " and float." );
            return 0;
      }
   }

   switch( op2->_type )
   {
      case VMMath::VEC3: VMMath::push( vm, ((VMQuat*)op1)->_val * ((VMVec3*)op2)->_val ); return 1;
      //case VMMath::VEC4: VMMath::push( vm, ((VMQuat*)op1)->_val * ((VMVec4*)op2)->_val ); return 1;
      case VMMath::QUAT: VMMath::push( vm, ((VMQuat*)op1)->_val * ((VMQuat*)op2)->_val ); return 1;
      default:
         IF_ERR( true, "Unsupported Quat multiplication between " << typeToStr(op1->_type) << " and " << typeToStr(op2->_type) << "." );
         break;
   }

   return 0;
}

//------------------------------------------------------------------------------
//!
int quat_str( VMState* vm )
{
   VMRType* op1 = (VMRType*)lua_touserdata( vm, 1 );
   String str;
   TextStream stream( str );
   stream << ((VMQuat*)op1)->_val;
   VM::push( vm, str );
   return 1;
}

//------------------------------------------------------------------------------
//!
int quat_len( VMState* vm )
{
   VM::push( vm, 4 );
   return 1;
}

//------------------------------------------------------------------------------
//! This routine accepts multiple variants:
//!
//! Row-major scalar access:   Column-major scalar access:
//!   [ x0 x1 x2 x3 ]            [ _x0 _y0 _z0 _w0 ]
//!   [ y0 y1 y2 y3 ]            [ _x1 _y1 _z1 _w1 ]
//!   [ z0 z1 z2 z3 ]            [ _x2 _y2 _z2 _w2 ]
//!   [ w0 w1 w2 w3 ]            [ _x3 _y3 _z3 _w3 ]
//!
//! Row vector access:         Column vector access:
//!   [ x[0] x[1] x[2] x[3] ]    [ _x[0] _y[0] _z[0] _w[0] ]
//!   [ y[0] y[1] y[2] y[3] ]    [ _x[1] _y[1] _z[1] _w[1] ]
//!   [ z[0] z[1] z[2] z[3] ]    [ _x[2] _y[2] _z[2] _w[2] ]
//!   [ w[0] w[1] w[2] w[3] ]    [ _x[3] _y[3] _z[3] _w[3] ]
//!
//! Row-column access:         Column-row access:
//!   [ xx xy xz xw ]            [ _xx _yx _zx _wx ]
//!   [ yx yy yz yw ]            [ _xy _yy _zy _wy ]
//!   [ zx zy zz zw ]            [ _xz _yz _zz _wz ]
//!   [ wx wy wz ww ]            [ _xw _yw _zw _ww ]
//!
//! Scalar access (NOT IMPLEMENTED):
//!   [ a b c d ]
//!   [ e f g h ]
//!   [ i j k l ]
//!   [ m n o p ]
//!
//! Ex:
//!   v    = mat4.y2
//!   v    = mat4.yz
//!   v    = mat4._z1
//!   v    = mat4._zy
//!   vec4 = mat4.x
//!   vec4 = mat4._y
//!   vec3 = mat4.afk  (NOT IMPLEMENTED)
int mat_get( VMState* vm )
{
   VMRType* ud = (VMRType*)lua_touserdata( vm, 1 );
   const char* k = VM::toCString( vm, 2 );

   uint n = uint(strlen( k ));

   switch( k[0] )
   {
      case '_':
      {
         // Column access.
         uint col = xyzwToIdx( k[1] );
         switch( n )
         {
            case 2:
            {
               // Column vector access.
               switch( ud->_type )
               {
                  case VMMath::MAT2:
                     IF_ERR( col >= 2, "Invalid column specifier for Mat2: '" << k << "'." )
                     VMMath::push( vm, ((VMMat2*)ud)->_val.col(col) );
                     return 1;
                  case VMMath::MAT3:
                     IF_ERR( col >= 3, "Invalid column specifier for Mat3: '" << k << "'." )
                     VMMath::push( vm, ((VMMat3*)ud)->_val.col(col) );
                     return 1;
                  case VMMath::MAT4:
                     IF_ERR( col >= 4, "Invalid column specifier for Mat4: '" << k << "'." )
                     VMMath::push( vm, ((VMMat4*)ud)->_val.col(col) );
                     return 1;
                  default:
                     IF_ERR( true, "ERROR - mat_get not called on a matrix: _type=" << ud->_type );
               }
            }  break;
            case 3:
            {
               // Column-major scalar access.
               uint row = digitToIdx( k[2] );
               if( row > 4 )  row = xyzwToIdx( k[2] );
               IF_ERR( col > 4 || row > 4, "Invalid column-major specifier: '" << k << "'." )
               switch( ud->_type )
               {
                  case VMMath::MAT2:
                     IF_ERR( col >= 2 || row >= 2, "Invalid specifier for Mat2: '" << k << "'." )
                     VM::push( vm, ((VMMat2*)ud)->_val(row, col) );
                     return 1;
                  case VMMath::MAT3:
                     IF_ERR( col >= 3 || row >= 3, "Invalid specifier for Mat3: '" << k << "'." )
                     VM::push( vm, ((VMMat3*)ud)->_val(row, col) );
                     return 1;
                  case VMMath::MAT4:
                     IF_ERR( col >= 4 || row >= 4, "Invalid specifier for Mat4: '" << k << "'." )
                     VM::push( vm, ((VMMat4*)ud)->_val(row, col) );
                     return 1;
                  default:
                     IF_ERR( true, "ERROR - mat_get not called on a matrix: _type=" << ud->_type );
               }
            }  break;
            default:
               IF_ERR( true, "Invalid matrix accessor: '" << k << "'." );
         }
         // Column access.
      }  break;
      case 'x':
      case 'y':
      case 'z':
      case 'w':
      {
         // Row access.
         uint row = xyzwToIdx( k[0] );
         switch( n )
         {
            case 1:
            {
               // Row vector access.
               switch( ud->_type )
               {
                  case VMMath::MAT2:
                     IF_ERR( row >= 2, "Invalid row specifier for Mat2: '" << k << "'." )
                     VMMath::push( vm, ((VMMat2*)ud)->_val.row(row) );
                     return 1;
                  case VMMath::MAT3:
                     IF_ERR( row >= 3, "Invalid row specifier for Mat3: '" << k << "'." )
                     VMMath::push( vm, ((VMMat3*)ud)->_val.row(row) );
                     return 1;
                  case VMMath::MAT4:
                     IF_ERR( row >= 4, "Invalid row specifier for Mat4: '" << k << "'." )
                     VMMath::push( vm, ((VMMat4*)ud)->_val.row(row) );
                     return 1;
                  default:
                     IF_ERR( true, "ERROR - mat_get not called on a matrix: _type=" << ud->_type );
               }
            }  break;
            case 2:
            {
               // Row-major scalar access.
               uint col = digitToIdx( k[1] );
               if( col > 4 )  col = xyzwToIdx( k[1] );
               IF_ERR( col > 4 || row > 4, "Invalid row-major specifier: '" << k << "'." )
               switch( ud->_type )
               {
                  case VMMath::MAT2:
                     IF_ERR( col >= 2 || row >= 2, "Invalid specifier for Mat2: '" << k << "'." )
                     VM::push( vm, ((VMMat2*)ud)->_val(row, col) );
                     return 1;
                  case VMMath::MAT3:
                     IF_ERR( col >= 3 || row >= 3, "Invalid specifier for Mat3: '" << k << "'." )
                     VM::push( vm, ((VMMat3*)ud)->_val(row, col) );
                     return 1;
                  case VMMath::MAT4:
                     IF_ERR( col >= 4 || row >= 4, "Invalid specifier for Mat4: '" << k << "'." )
                     VM::push( vm, ((VMMat4*)ud)->_val(row, col) );
                     return 1;
                  default:
                     IF_ERR( true, "ERROR - mat_get not called on a matrix: _type=" << ud->_type );
               }
            }  break;
            default:
            IF_ERR( true, "Invalid matrix accessor: '" << k << "'." );
         }
         // Row access.
      }  break;
#if 0
      case 'a':
      case 'b':
      case 'c':
      case 'd':
      case 'e':
      case 'f':
      case 'g':
      case 'h':
      case 'i':
      case 'j':
      case 'k':
      case 'l':
      case 'm':
      case 'n':
      case 'o':
      case 'p':
      {
         // Scalar access.
         float tmp[4];
      }  break;
#endif
      default:
         IF_ERR( true, "Invalid matrix accessor: '" << k << "'." );
   }

   return 0;
}

//------------------------------------------------------------------------------
//!
int mat_set( VMState* vm )
{
   VMRType* ud = (VMRType*)lua_touserdata( vm, 1 );
   const char* k = VM::toCString( vm, 2 );
   //VMRType* v = (VMRType*)lua_touserdata( vm, 3 );

   uint n = uint(strlen( k ));

   switch( k[0] )
   {
      case '_':
      {
         // Column access.
         uint col = xyzwToIdx( k[1] );
         switch( n )
         {
            case 2:
            {
               // Column vector access.
               VMRType* v = (VMRType*)lua_touserdata( vm, 3 );
               switch( ud->_type )
               {
                  case VMMath::MAT2:
                     IF_ERR( col >= 2, "Invalid column specifier for Mat2: '" << k << "'." )
                     IF_ERR( v->_type != VMMath::VEC2, "Mat2." << k << " requires a vec2, got a " << typeToStr(v->_type) << "." )
                     ((VMMat2*)ud)->_val.col( col, ((VMVec2*)v)->_val );
                     return 0;
                  case VMMath::MAT3:
                     IF_ERR( col >= 3, "Invalid column specifier for Mat3: '" << k << "'." )
                     IF_ERR( v->_type != VMMath::VEC3, "Mat3." << k << " requires a vec3, got a " << typeToStr(v->_type) << "." )
                     ((VMMat3*)ud)->_val.col( col, ((VMVec3*)v)->_val );
                     return 0;
                  case VMMath::MAT4:
                     IF_ERR( col >= 4, "Invalid column specifier for Mat4: '" << k << "'." )
                     IF_ERR( v->_type != VMMath::VEC4, "Mat4." << k << " requires a vec4, got a " << typeToStr(v->_type) << "." )
                     ((VMMat4*)ud)->_val.col( col, ((VMVec4*)v)->_val );
                     return 1;
                  default:
                     IF_ERR( true, "ERROR - mat_set not called on a matrix: _type=" << ud->_type );
               }
            }  break;
            case 3:
            {
               // Column-major scalar access.
               uint row = digitToIdx( k[2] );
               if( row > 4 )  row = xyzwToIdx( k[2] );
               IF_ERR( col > 4 || row > 4, "Invalid column-major specifier: '" << k << "'." )
               IF_ERR( !VM::isNumber(vm, 3), "Matrix column-major scalar assign didn't get a number." )
               switch( ud->_type )
               {
                  case VMMath::MAT2:
                     IF_ERR( col >= 2 || row >= 2, "Invalid specifier for Mat2: '" << k << "'." )
                     ((VMMat2*)ud)->_val( row, col ) = VM::toFloat( vm, 3 );
                     return 0;
                  case VMMath::MAT3:
                     IF_ERR( col >= 3 || row >= 3, "Invalid specifier for Mat3: '" << k << "'." )
                     ((VMMat3*)ud)->_val( row, col ) = VM::toFloat( vm, 3 );
                     return 0;
                  case VMMath::MAT4:
                     IF_ERR( col >= 4 || row >= 4, "Invalid specifier for Mat4: '" << k << "'." )
                     ((VMMat4*)ud)->_val( row, col ) = VM::toFloat( vm, 3 );
                     return 0;
                  default:
                     IF_ERR( true, "ERROR - mat_get not called on a matrix: _type=" << ud->_type );
               }
            }  break;
            default:
               IF_ERR( true, "Invalid matrix accessor: '" << k << "'." );
         }
         // Column access.
      }  break;
      case 'x':
      case 'y':
      case 'z':
      case 'w':
      {
         // Row access.
         uint row = xyzwToIdx( k[0] );
         switch( n )
         {
            case 1:
            {
               // Row vector access.
               VMRType* v = (VMRType*)lua_touserdata( vm, 3 );
               switch( ud->_type )
               {
                  case VMMath::MAT2:
                     IF_ERR( row >= 2, "Invalid row specifier for Mat2: '" << k << "'." )
                     IF_ERR( v->_type != VMMath::VEC2, "Mat2." << k << " requires a vec2, got a " << typeToStr(v->_type) << "." )
                     ((VMMat2*)ud)->_val.row( row, ((VMVec2*)v)->_val );
                     return 0;
                  case VMMath::MAT3:
                     IF_ERR( row >= 3, "Invalid row specifier for Mat3: '" << k << "'." )
                     IF_ERR( v->_type != VMMath::VEC3, "Mat3." << k << " requires a vec3, got a " << typeToStr(v->_type) << "." )
                     ((VMMat3*)ud)->_val.row( row, ((VMVec3*)v)->_val );
                     return 0;
                  case VMMath::MAT4:
                     IF_ERR( row >= 4, "Invalid row specifier for Mat4: '" << k << "'." )
                     IF_ERR( v->_type != VMMath::VEC4, "Mat4." << k << " requires a vec4, got a " << typeToStr(v->_type) << "." )
                     ((VMMat4*)ud)->_val.row( row, ((VMVec4*)v)->_val );
                     return 0;
                  default:
                     IF_ERR( true, "ERROR - mat_get not called on a matrix: _type=" << ud->_type );
               }
            }  break;
            case 2:
            {
               // Row-major scalar access.
               uint col = digitToIdx( k[1] );
               if( col > 4 )  col = xyzwToIdx( k[1] );
               IF_ERR( col > 4 || row > 4, "Invalid row-major specifier: '" << k << "'." )
               IF_ERR( !VM::isNumber(vm, 3), "Matrix row-major scalar assign didn't get a number." )
               switch( ud->_type )
               {
                  case VMMath::MAT2:
                     IF_ERR( col >= 2 || row >= 2, "Invalid specifier for Mat2: '" << k << "'." )
                     ((VMMat2*)ud)->_val( row, col ) = VM::toFloat( vm, 3 );
                     return 0;
                  case VMMath::MAT3:
                     IF_ERR( col >= 3 || row >= 3, "Invalid specifier for Mat3: '" << k << "'." )
                     ((VMMat3*)ud)->_val( row, col ) = VM::toFloat( vm, 3 );
                     return 0;
                  case VMMath::MAT4:
                     IF_ERR( col >= 4 || row >= 4, "Invalid specifier for Mat4: '" << k << "'." )
                     ((VMMat4*)ud)->_val( row, col ) = VM::toFloat( vm, 3 );
                     return 0;
                  default:
                     IF_ERR( true, "ERROR - mat_get not called on a matrix: _type=" << ud->_type );
               }
            }  break;
            default:
            IF_ERR( true, "Invalid matrix accessor: '" << k << "'." );
         }
         // Row access.
      }  break;
      default:
         IF_ERR( true, "Uknown matrix access: '" << k << "'." );
   }

   return 0;
}

//------------------------------------------------------------------------------
//!
int mat_add( VMState* vm )
{
   VMRType* op1 = (VMRType*)lua_touserdata( vm, 1 );
   VMRType* op2 = (VMRType*)lua_touserdata( vm, 2 );
   IF_ERR( op1->_type != op2->_type, "Type mismatch on " << typeToStr(op1->_type) << "+" << typeToStr(op2->_type) << "." );

   switch( op1->_type )
   {
      case VMMath::MAT2: VMMath::push( vm, ((VMMat2*)op1)->_val + ((VMMat2*)op2)->_val ); return 1;
      case VMMath::MAT3: VMMath::push( vm, ((VMMat3*)op1)->_val + ((VMMat3*)op2)->_val ); return 1;
      case VMMath::MAT4: VMMath::push( vm, ((VMMat4*)op1)->_val + ((VMMat4*)op2)->_val ); return 1;
      default:;
   }

   return 0;
}

//------------------------------------------------------------------------------
//!
int mat_sub( VMState* vm )
{
   VMRType* op1 = (VMRType*)lua_touserdata( vm, 1 );
   VMRType* op2 = (VMRType*)lua_touserdata( vm, 2 );
   IF_ERR( op1->_type != op2->_type, "Type mismatch on " << typeToStr(op1->_type) << "-" << typeToStr(op2->_type) << "." );

   switch( op1->_type )
   {
      case VMMath::MAT2: VMMath::push( vm, ((VMMat2*)op1)->_val - ((VMMat2*)op2)->_val ); return 1;
      case VMMath::MAT3: VMMath::push( vm, ((VMMat3*)op1)->_val - ((VMMat3*)op2)->_val ); return 1;
      case VMMath::MAT4: VMMath::push( vm, ((VMMat4*)op1)->_val - ((VMMat4*)op2)->_val ); return 1;
      default:;
   }

   return 0;
}

//------------------------------------------------------------------------------
//!
int mat_mul( VMState* vm )
{
   VMRType* op1 = (VMRType*)lua_touserdata( vm, 1 );
   VMRType* op2 = (VMRType*)lua_touserdata( vm, 2 );

   if( op1->_type == op2->_type )
   {
      switch( op1->_type )
      {
         case VMMath::MAT2: VMMath::push( vm, ((VMMat2*)op1)->_val * ((VMMat2*)op2)->_val ); return 1;
         case VMMath::MAT3: VMMath::push( vm, ((VMMat3*)op1)->_val * ((VMMat3*)op2)->_val ); return 1;
         case VMMath::MAT4: VMMath::push( vm, ((VMMat4*)op1)->_val * ((VMMat4*)op2)->_val ); return 1;
         default:;
      }
   }
   else
   {
      switch( op1->_type )
      {
         case VMMath::MAT2:
            IF_ERR( op2->_type != VMMath::VEC2, "Invalid operation: " << typeToStr(op1->_type) << "*" << typeToStr(op2->_type) << "." );
            VMMath::push( vm, ((VMMat2*)op1)->_val * ((VMVec2*)op2)->_val );
            return 1;
         case VMMath::MAT3:
            IF_ERR( op2->_type != VMMath::VEC3, "Invalid operation: " << typeToStr(op1->_type) << "*" << typeToStr(op2->_type) << "." );
            VMMath::push( vm, ((VMMat3*)op1)->_val * ((VMVec3*)op2)->_val );
            return 1;
         case VMMath::MAT4:
            IF_ERR( op2->_type != VMMath::VEC4, "Invalid operation: " << typeToStr(op1->_type) << "*" << typeToStr(op2->_type) << "." );
            VMMath::push( vm, ((VMMat4*)op1)->_val * ((VMVec4*)op2)->_val );
            return 1;
         default:;
      }
   }

   return 0;
}

//------------------------------------------------------------------------------
//!
int mat_div( VMState* vm )
{
   VMRType* op1 = (VMRType*)lua_touserdata( vm, 1 );
   VMRType* op2 = (VMRType*)lua_touserdata( vm, 2 );
   IF_ERR( op1->_type != op2->_type, "Type mismatch on " << typeToStr(op1->_type) << "/" << typeToStr(op2->_type) << "." );

   switch( op1->_type )
   {
      case VMMath::MAT2: VMMath::push( vm, ((VMMat2*)op1)->_val / ((VMMat2*)op2)->_val ); return 1;
      case VMMath::MAT3: VMMath::push( vm, ((VMMat3*)op1)->_val / ((VMMat3*)op2)->_val ); return 1;
      case VMMath::MAT4: VMMath::push( vm, ((VMMat4*)op1)->_val / ((VMMat4*)op2)->_val ); return 1;
      default:;
   }
   return 0;
}

//------------------------------------------------------------------------------
//!
int mat_unm( VMState* vm )
{
   VMRType* op1 = (VMRType*)lua_touserdata( vm, 1 );

   switch( op1->_type )
   {
      case VMMath::MAT2: VMMath::push( vm, -((VMMat2*)op1)->_val ); return 1;
      case VMMath::MAT3: VMMath::push( vm, -((VMMat3*)op1)->_val ); return 1;
      case VMMath::MAT4: VMMath::push( vm, -((VMMat4*)op1)->_val ); return 1;
      default:;
   }

   return 0;
}

//------------------------------------------------------------------------------
//!
int mat_eq( VMState* vm )
{
   VMRType* op1 = (VMRType*)lua_touserdata( vm, 1 );
   VMRType* op2 = (VMRType*)lua_touserdata( vm, 2 );
   IF_ERR( op1->_type != op2->_type, "Type mismatch on " << typeToStr(op1->_type) << "==" << typeToStr(op2->_type) << "." );

   switch( op1->_type )
   {
      case VMMath::MAT2: VM::push( vm, ((VMMat2*)op1)->_val == ((VMMat2*)op2)->_val ); return 1;
      case VMMath::MAT3: VM::push( vm, ((VMMat3*)op1)->_val == ((VMMat3*)op2)->_val ); return 1;
      case VMMath::MAT4: VM::push( vm, ((VMMat4*)op1)->_val == ((VMMat4*)op2)->_val ); return 1;
      default:;
   }

   return 0;
}

//------------------------------------------------------------------------------
//!
int mat_str( VMState* vm )
{
   VMRType* op1 = (VMRType*)lua_touserdata( vm, 1 );
   String str;
   TextStream stream( str );

   switch( op1->_type )
   {
      case VMMath::MAT2:
         stream << ((VMMat2*)op1)->_val;
         VM::push( vm, str );
         return 1;
      case VMMath::MAT3:
         stream << ((VMMat3*)op1)->_val;
         VM::push( vm, str );
         return 1;
      case VMMath::MAT4:
         stream << ((VMMat4*)op1)->_val;
         VM::push( vm, str );
         return 1;
      default:;
   }

   return 0;
}

//------------------------------------------------------------------------------
//!
int mat_len( VMState* vm )
{
   VMRType* op1 = (VMRType*)lua_touserdata( vm, 1 );

   switch( op1->_type )
   {
      case VMMath::MAT2:
         VM::push( vm, 2 );
         return 1;
      case VMMath::MAT3:
         VM::push( vm, 3 );
         return 1;
      case VMMath::MAT4:
         VM::push( vm, 4 );
         return 1;
      default:;
   }

   return 0;
}

//------------------------------------------------------------------------------
//!
int counter_set_func( VMState* vm )
{
   VMType<VMCounter>* c = (VMType<VMCounter>*)lua_touserdata( vm, 1 );
   c->_val.set( VM::toNumber(vm, 2), VM::toNumber(vm, 3) );
   return 0;
}

//------------------------------------------------------------------------------
//!
int counter_next( VMState* vm )
{
   VMType<VMCounter>* c = (VMType<VMCounter>*)lua_touserdata( vm, 1 );
   VM::push( vm, c->_val.next() );
   return 1;
}

//------------------------------------------------------------------------------
//!
int counter_set( VMState* vm )
{
   VMType<VMCounter>* c = (VMType<VMCounter>*)lua_touserdata( vm, 1 );
   switch( _counter_attr[lua_tostring( vm, 2 )] )
   {
      case ATTRIB_CURRENT:
         c->_val.current( lua_tonumber( vm, 3 ) );
         return 0;
      case ATTRIB_INCREMENT:
         c->_val.increment( lua_tonumber( vm, 3 ) );
         return 0;
      case ATTRIB_SET:
         // Read-only.
         return 0;
      default:
         break;
   }
   return 0;
}

//------------------------------------------------------------------------------
//!
int counter_get( VMState* vm )
{
   VMType<VMCounter>* c = (VMType<VMCounter>*)lua_touserdata( vm, 1 );
   switch( _counter_attr[lua_tostring( vm, 2 )] )
   {
      case ATTRIB_CURRENT:
         VM::push( vm, c->_val.current() );
         return 1;
      case ATTRIB_INCREMENT:
         VM::push( vm, c->_val.increment() );
         return 1;
      case ATTRIB_SET:
         VM::push( vm, counter_set_func );
         return 1;
      default:
         break;
   }
   return 0;
}

//------------------------------------------------------------------------------
//!
const char* _rng_mt = "rng";

//------------------------------------------------------------------------------
//!
int rng_create( VMState* vm )
{
   RNG_WELL* rngp = (RNG_WELL*)lua_newuserdata( vm, sizeof(RNG_WELL) );
   new( rngp ) RNG_WELL(); // In-place constructor (doesn't allocate memory).
                           // Note: the destructor is not being called (should be OK).

   // Set meta-table.
   luaL_getmetatable( vm, _rng_mt );
   lua_setmetatable( vm, -2 );

   if( VM::getTop( vm ) > 0 )  rngp->seed( VM::toUInt(vm, 1) );

   // Return the userdata (already on stack).
   return 1;
}

//------------------------------------------------------------------------------
//!
int rng_pick( VMState* vm )
{
   RNG_WELL& rng = *(RNG_WELL*)VM::thisPtr( vm );

   // Get number of elements in table.
   int size = VM::getTableSize( vm, 1 );

   // Choose a random key.
   int key;
   if( VM::getTop( vm ) == 1 )
   {
      // Equally distributed keys.
      key = (rng.getUInt() % size)+1;
   }
   else
   {
      // Weighted keys.
      // Compute normalization factor (in doubles, since that is what the RNGs use).
      double total = 0.0;
      int wSize = VM::getTableSize( vm, 2 );
      if( size < wSize )  wSize = size;
      for( int i = 1; i <= wSize; ++i )
      {
         VM::geti( vm, 2, i );
         total += VM::toNumber( vm, -1 );
         VM::pop( vm );
      }
      // Generate a random number.
      double r = rng() * total;
      // Retrieve the associated key.
      total = 0.0;
      for( key = 1; key <= wSize; ++key )
      {
         VM::geti( vm, 2, key );
         total += VM::toNumber( vm, -1 );
         VM::pop( vm );
         if( total > r )  break; // Favor the latter when hitting boundary cases.
      }
   }

   // Return value attached to the selected key.
   VM::geti( vm, 1, key );

   return 1;
}

//------------------------------------------------------------------------------
//!
int rng_random( VMState* vm, RNG_WELL& rng, int argOffset )
{
   switch( VM::getTop(vm) - argOffset )
   {
      case 0:
      {
         // No parameters: Generate a real number in [0..1)
         VM::push( vm, rng.getDouble() );
         return 1;
      }
      case 1:
      {
         // One parameter: Generate an integer number in [1..p1]
         uint p1 = VM::toUInt( vm, 1+argOffset );
         CHECK( p1 != 0 );
         uint v  = rng.getUInt( p1 ) + 1;
         VM::push( vm, v );
         return 1;
      }
      case 2:
      {
         // Two parameters: Generate an integer number in [p1..p2]
         uint p1 = VM::toUInt( vm, 1+argOffset );
         uint p2 = VM::toUInt( vm, 2+argOffset );
         uint d  = p2 - p1 + 1;
         CHECK( d != 0 );
         uint v  = rng.getUInt( d ) + p1;
         VM::push( vm, v );
         return 1;
      }
      default:
      {
         return 0;
      }
   }
}

//------------------------------------------------------------------------------
//!
int rng_random( VMState* vm )
{
   return rng_random( vm, *(RNG_WELL*)VM::thisPtr( vm ), 0 );
}

//------------------------------------------------------------------------------
//!
int rng_call( VMState* vm )
{
   return rng_random( vm, *(RNG_WELL*)lua_touserdata( vm, 1 ), 1 );
}

//------------------------------------------------------------------------------
//!
int rng_set( VMState* vm )
{
   RNG_WELL& rng = *(RNG_WELL*)lua_touserdata( vm, 1 );
   switch( _rng_attr[lua_tostring( vm, 2 )] )
   {
      case ATTRIB_PICK:
      case ATTRIB_RANDOM:
         // Read-only.
         return 0;
      case ATTRIB_SEED:
         rng.seed( VM::toUInt( vm, 3 ) );
         return 0;
      default:
         break;
   }
   return 0;
}

//------------------------------------------------------------------------------
//!
int rng_get( VMState* vm )
{
   RNG_WELL* rngp = (RNG_WELL*)lua_touserdata( vm, 1 );
   switch( _rng_attr[lua_tostring( vm, 2 )] )
   {
      case ATTRIB_PICK:
         VM::push( vm, rngp, rng_pick );
         return 1;
      case ATTRIB_RANDOM:
         VM::push( vm, rngp, rng_random );
         return 1;
      case ATTRIB_SEED:
         // Write-only.
         return 0;
      default:
         break;
   }
   return 0;
}



//------------------------------------------------------------------------------
//!
void
initVM( VMState* vm, uint /*mask*/ )
{
   VM::registerFunctions( vm, "_G", bit_funcs );
   VM::registerFunctions( vm, "_G", math_funcs );

   // Vec.
   VM::newMetaTable( vm, "vec" );
   VM::set( vm, -1, "__index", vec_get );
   VM::set( vm, -1, "__newindex", vec_set );
   VM::set( vm, -1, "__add", vec_add );
   VM::set( vm, -1, "__sub", vec_sub );
   VM::set( vm, -1, "__mul", vec_mul );
   VM::set( vm, -1, "__div", vec_div );
   VM::set( vm, -1, "__unm", vec_unm );
   VM::set( vm, -1, "__eq", vec_eq );
   VM::set( vm, -1, "__tostring", vec_str );
   VM::set( vm, -1, "__len", vec_len );
   VM::pop( vm );

   // Quat.
   VM::newMetaTable( vm, "quat" );
   VM::set( vm, -1, "__index", quat_get );
   VM::set( vm, -1, "__newindex", quat_set );
   VM::set( vm, -1, "__mul", quat_mul );
   VM::set( vm, -1, "__eq", quat_eq );
   VM::set( vm, -1, "__tostring", quat_str );
   VM::set( vm, -1, "__len", quat_len );
   VM::pop( vm );

   // Mat.
   VM::newMetaTable( vm, "mat" );
   VM::set( vm, -1, "__index", mat_get );
   VM::set( vm, -1, "__newindex", mat_set );
   VM::set( vm, -1, "__add", mat_add );
   VM::set( vm, -1, "__sub", mat_sub );
   VM::set( vm, -1, "__mul", mat_mul );
   VM::set( vm, -1, "__div", mat_div );
   VM::set( vm, -1, "__unm", mat_unm );
   VM::set( vm, -1, "__eq", mat_eq );
   VM::set( vm, -1, "__tostring", mat_str );
   VM::set( vm, -1, "__len", mat_len );
   VM::pop( vm );

   // Counter.
   VM::newMetaTable( vm, "counter" );
   VM::set( vm, -1, "__index", counter_get );
   VM::set( vm, -1, "__newindex", counter_set );
   VM::set( vm, -1, "__call", counter_next );
   VM::pop( vm );

   // RNG.
   VM::newMetaTable( vm, _rng_mt );
   VM::set( vm, -1, "__index", rng_get );
   VM::set( vm, -1, "__newindex", rng_set );
   VM::set( vm, -1, "__call", rng_call );
   VM::pop( vm );
}

UNNAMESPACE_END

NAMESPACE_BEGIN


/*==============================================================================
  CLASS VMMath
==============================================================================*/


//------------------------------------------------------------------------------
//!
void
VMMath::initialize()
{
   VMRegistry::add( initVM, VM_CAT_MATH );
}

//------------------------------------------------------------------------------
//!
void
VMMath::push( VMState* vm, const Vec2f& v )
{
   create( vm, VEC2, v, "vec" );
}

//------------------------------------------------------------------------------
//!
void
VMMath::push( VMState* vm, const Vec3f& v )
{
   create( vm, VEC3, v, "vec" );
}

//------------------------------------------------------------------------------
//!
void
VMMath::push( VMState* vm, const Vec4f& v )
{
   create( vm, VEC4, v, "vec" );
}

//------------------------------------------------------------------------------
//!
void
VMMath::push( VMState* vm, const Quatf& v )
{
   create( vm, QUAT, v, "quat" );
}

//------------------------------------------------------------------------------
//!
void
VMMath::push( VMState* vm, const Mat2f& v )
{
   create( vm, MAT2, v, "mat" );
}

//------------------------------------------------------------------------------
//!
void
VMMath::push( VMState* vm, const Mat3f& v )
{
   create( vm, MAT3, v, "mat" );
}

//------------------------------------------------------------------------------
//!
void
VMMath::push( VMState* vm, const Mat4f& v )
{
   create( vm, MAT4, v, "mat" );
}

//------------------------------------------------------------------------------
//!
void
VMMath::push( VMState* vm, const VMCounter& c )
{
   create( vm, COUNTER, c, "counter" );
}

//------------------------------------------------------------------------------
//!
Vec2f
VMMath::toVec2( VMState* vm, int idx )
{
   VMType<Vec2f>* ud = (VMType<Vec2f>*)lua_touserdata( vm, idx );
   CHECK( ud->_type == VEC2 );
   return ud->_val;
}

//------------------------------------------------------------------------------
//!
Vec3f
VMMath::toVec3( VMState* vm, int idx )
{
   VMType<Vec3f>* ud = (VMType<Vec3f>*)lua_touserdata( vm, idx );
   CHECK( ud->_type == VEC3 );
   return ud->_val;
}

//------------------------------------------------------------------------------
//!
Vec4f
VMMath::toVec4( VMState* vm, int idx )
{
   VMType<Vec4f>* ud = (VMType<Vec4f>*)lua_touserdata( vm, idx );
   CHECK( ud->_type == VEC4 );
   return ud->_val;
}

//------------------------------------------------------------------------------
//!
Quatf
VMMath::toQuat( VMState* vm, int idx )
{
   VMType<Quatf>* ud = (VMType<Quatf>*)lua_touserdata( vm, idx );
   CHECK( ud->_type == QUAT );
   return ud->_val;
}

//------------------------------------------------------------------------------
//!
Mat2f
VMMath::toMat2( VMState* vm, int idx )
{
   VMType<Mat2f>* ud = (VMType<Mat2f>*)lua_touserdata( vm, idx );
   CHECK( ud->_type == MAT2 );
   return ud->_val;
}

//------------------------------------------------------------------------------
//!
Mat3f
VMMath::toMat3( VMState* vm, int idx )
{
   VMType<Mat3f>* ud = (VMType<Mat3f>*)lua_touserdata( vm, idx );
   CHECK( ud->_type == MAT3 );
   return ud->_val;
}

//------------------------------------------------------------------------------
//!
Mat4f
VMMath::toMat4( VMState* vm, int idx )
{
   VMType<Mat4f>* ud = (VMType<Mat4f>*)lua_touserdata( vm, idx );
   CHECK( ud->_type == MAT4 );
   return ud->_val;
}

//------------------------------------------------------------------------------
//! We return a pointer to avoid obscure bugs in the following construct:
//!   Counter c = VMMath::toCounter( vm, -1 );
//!   float f = c.next();
//! where c.next() increments a *copy* of the counter rather than the original.
VMCounter*
VMMath::toCounter( VMState* vm, int idx )
{
   VMType<VMCounter>* ud = (VMType<VMCounter>*)lua_touserdata( vm, idx );
   CHECK( ud->_type == COUNTER );
   return &(ud->_val);
}

//------------------------------------------------------------------------------
//!
int
VMMath::type( VMState* vm, int idx )
{
   VMRType* abstract = (VMRType*)lua_touserdata( vm, idx );
   return abstract->_type;
}

NAMESPACE_END
