/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef GFX_RENDER_STATE_H
#define GFX_RENDER_STATE_H

#include <Gfx/StdDefs.h>

#include <Base/Util/Bits.h>
#include <Base/Util/RCObject.h>

/**
 * Missing
 * - Clearing commands + values
 * - RasterOps
 * - Fog
 * - Clipping
 * - Polygon offsets
 * - Two-sided stencil ops
 * - ...
 */

NAMESPACE_BEGIN

namespace Gfx
{

/*----- types -----*/

typedef enum
{
   ALPHA_BLEND_ZERO                = 0x01,
   ALPHA_BLEND_ONE                 = 0x02,
   ALPHA_BLEND_SRC_COLOR           = 0x03,
   ALPHA_BLEND_ONE_MINUS_SRC_COLOR = 0x04,
   ALPHA_BLEND_INV_SRC_COLOR       = 0x04,
   ALPHA_BLEND_SRC_ALPHA           = 0x05,
   ALPHA_BLEND_ONE_MINUS_SRC_ALPHA = 0x06,
   ALPHA_BLEND_INV_SRC_ALPHA       = 0x06,
   ALPHA_BLEND_DST_ALPHA           = 0x07,
   ALPHA_BLEND_ONE_MINUS_DST_ALPHA = 0x08,
   ALPHA_BLEND_INV_DST_ALPHA       = 0x08,
   ALPHA_BLEND_DST_COLOR           = 0x09,
   ALPHA_BLEND_ONE_MINUS_DST_COLOR = 0x0A,
   ALPHA_BLEND_INV_DST_COLOR       = 0x0A,
   ALPHA_BLEND_SRC_ALPHA_SAT       = 0x0B,
   ALPHA_BLEND_BOTH_SRC_ALPHA      = 0x0C,
   ALPHA_BLEND_BOTH_INV_SRC_ALPHA  = 0x0D,
   ALPHA_BLEND_BLEND_FACTOR        = 0x0E,
   ALPHA_BLEND_INV_BLEND_FACTOR    = 0x0F
} AlphaBlend;

typedef enum
{
   COMPARE_FUNC_NEVER,
   COMPARE_FUNC_LESS,
   COMPARE_FUNC_EQUAL,
   COMPARE_FUNC_LESSEQUAL,
   COMPARE_FUNC_GREATER,
   COMPARE_FUNC_NOTEQUAL,
   COMPARE_FUNC_GREATEREQUAL,
   COMPARE_FUNC_ALWAYS
} CompareFunc;

typedef enum
{
   FACE_NONE           = 0,
   FACE_FRONT          = 1,
   FACE_BACK           = 2,
   FACE_FRONT_AND_BACK = 3
} FaceSide;

typedef enum
{
   STENCIL_OP_KEEP,
   STENCIL_OP_ZERO,
   STENCIL_OP_REPLACE,
   STENCIL_OP_INCR_SAT,
   STENCIL_OP_DECR_SAT,
   STENCIL_OP_INVERT,
   STENCIL_OP_INCR,
   STENCIL_OP_DECR
} StencilOp;

/*==============================================================================
   CLASS AlphaState
==============================================================================*/

class AlphaState:
   public RCObject
{

public: 

   /*----- methods -----*/

   AlphaState(): _fields(0x0) { setDefaults(); }
   AlphaState( const AlphaState& s ) { *this = s; }
   
   GFX_DLL_API void  setDefaults();

   void setInvalidFields() { _fields = 0xFFFFFFFF; }

   void         alphaBlending( bool ab )        { _fields = setbits(_fields, 0, 1, ab); }
   void         alphaBlendSrc( AlphaBlend ab )  { _fields = setbits(_fields, 1, 4, ab); }
   void         alphaBlendDst( AlphaBlend ab )  { _fields = setbits(_fields, 5, 4, ab); }
   void         setAlphaBlend( AlphaBlend ab_src, AlphaBlend ab_dst );
   bool         alphaBlending() const           { return getbits(_fields, 0, 1) & 0x01; }
   AlphaBlend   alphaBlendSrc() const           { return (AlphaBlend)getbits(_fields, 1, 4); }
   AlphaBlend   alphaBlendDst() const           { return (AlphaBlend)getbits(_fields, 5, 4); }
   void         getAlphaBlend( AlphaBlend& src, AlphaBlend& dst ) const;

   void         alphaTesting( bool at )         { _fields = setbits(_fields, 9, 1, at); }
   void         alphaTestFunc( CompareFunc cf ) { _fields = setbits(_fields, 10, 3, cf); }
   void         alphaTestRef( float atr )       { _alphaTestRef = atr; }
   bool         alphaTesting() const            { return getbits(_fields, 9, 1) & 0x01; }
   CompareFunc  alphaTestFunc() const           { return (CompareFunc)getbits(_fields, 10, 3); }
   float        alphaTestRef() const            { return _alphaTestRef; }

   AlphaState&  operator=( const AlphaState& as ) { _fields = as._fields; _alphaTestRef = as._alphaTestRef; return *this; }
   bool  operator==( const AlphaState& as ) const { return _fields == as._fields && _alphaTestRef == as._alphaTestRef; }
   bool  operator!=( const AlphaState& as ) const { return !operator==(as); }

   // Compares the states and returns a comparison code, where each bit corresponds to a difference in some fields:
   //  code[0]  The alpha blending state differs
   //  code[1]  The alpha blend source and/or destination differ
   //  code[2]  The alpha testing state differs
   //  code[3]  The alpha test func differs
   //  code[4]  The alpha test ref differs
   // If code == 0, then there is no difference between both states.
   uint  compare( const AlphaState& as ) const
   {
      uint bits = _fields ^ as._fields;
      uint code = (alphaTestRef() != as.alphaTestRef());
      if( bits != 0x0 )
      {
         code <<= 1;
         code |= ((bits & 0x1C00) != 0);
         code <<= 1;
         code |= ((bits & 0x0200) != 0);
         code <<= 1;
         code |= ((bits & 0x01FE) != 0);
         code <<= 1;
         code |= ((bits & 0x0001) != 0);
      }
      else
      {
         code <<= 4;
      }
      return code;
   }

private: 

   /*----- data members -----*/

   //! NAME          SIZE   LOCATION         DESCRIPTION
   //! alphaBlending  (1)   _fields[ 0: 0]   Controls whether or not alpha blending is enabled.
   //! alphaBlendSrc  (4)   _fields[ 4: 1]   Source of the alpha blend operation.
   //! alphaBlendDst  (4)   _fields[ 8: 5]   Destination of the alpha blend operation.
   //!
   //! alphaTesting   (1)   _fields[ 9: 9]   Controls whether or not alpha test is enabled.
   //! alphaTestFunc  (3)   _fields[12:10]   The compare function to use for alpha testing.
   uint  _fields;
   float _alphaTestRef; //!< Specifies the reference alpha value to test against
};

//------------------------------------------------------------------------------
//!
inline void 
AlphaState::setAlphaBlend( AlphaBlend ab_src, AlphaBlend ab_dst ) 
{ 
   _fields = setbits( setbits(_fields, 5, 4, ab_dst), 1, 4, ab_src); 
}

//------------------------------------------------------------------------------
//!
inline void 
AlphaState::getAlphaBlend( AlphaBlend& src, AlphaBlend& dst ) const 
{ 
   src = (AlphaBlend)getbits(_fields, 1, 4); 
   dst = (AlphaBlend)getbits(_fields, 5, 4); 
}

/*==============================================================================
   CLASS ColorState
==============================================================================*/

class ColorState:
   public RCObject
{

public: 

   /*----- methods -----*/

   ColorState() { setDefaults(); }
   ColorState( const ColorState& s ) { *this = s; }
   
   GFX_DLL_API void  setDefaults();

   void setInvalidFields()       { _colorWriting = true; }

   void  colorWriting( bool cw ) { _colorWriting = cw; }
   bool  colorWriting() const    { return _colorWriting; }
   
   ColorState&  operator=( const ColorState& cs ) { _colorWriting = cs._colorWriting; return *this; }
   bool  operator==( const ColorState& cs ) const { return _colorWriting == cs._colorWriting; }
   bool  operator!=( const ColorState& cs ) const { return !operator==(cs); }
   
private: 

   /*----- data members -----*/

   bool  _colorWriting;   //!< Optionnally kills the writing of the color value
};

/*==============================================================================
   CLASS CullState
==============================================================================*/

class CullState:
   public RCObject
{

public: 

   /*----- methods -----*/

   CullState() { setDefaults(); }
   CullState( const CullState& s ) { *this = s; }

   GFX_DLL_API void  setDefaults();

   void setInvalidFields()         { _side = 0xFFFFFFFF; }

   void      side( FaceSide s )    { _side = s; }
   FaceSide  side() const          { return (FaceSide)_side; }

   CullState&  operator=( const CullState& cs )  { _side = cs._side; return *this; }
   bool  operator==( const CullState& cs ) const { return _side == cs._side; }
   bool  operator!=( const CullState& cs ) const { return !operator==(cs); }

private: 

   /*----- data members -----*/

   uint _side;
};

/*==============================================================================
   CLASS DepthState
==============================================================================*/

class DepthState:
   public RCObject
{

public: 

   /*----- methods -----*/

   DepthState(): _fields(0x0) { setDefaults(); }
   DepthState( const DepthState& s ) { *this = s; }
   
   GFX_DLL_API void  setDefaults();

   void setInvalidFields() { _fields = 0xFFFFFFFF; }
   
   void         depthTesting( bool dt )         { _fields = setbits(_fields, 0, 1, dt); }
   void         depthTestFunc( CompareFunc cf ) { _fields = setbits(_fields, 1, 3, cf); }
   void         depthWriting( bool dw )         { _fields = setbits(_fields, 4, 1, dw); }
   bool         depthTesting() const            { return getbits(_fields, 0, 1) & 0x01; }
   CompareFunc  depthTestFunc() const           { return (CompareFunc)getbits(_fields, 1, 3); }
   bool         depthWriting() const            { return getbits(_fields, 4, 1) & 0x01; }

   DepthState&  operator=( const DepthState& ds ) { _fields = ds._fields; return *this; }
   bool  operator==( const DepthState& ds ) const { return _fields == ds._fields; }
   bool  operator!=( const DepthState& ds ) const { return !operator==(ds); }

   // Compares the states and returns a comparison code, where each bit corresponds to a difference in some fields:
   //  code[0]  The depth testing state differs
   //  code[1]  The depth test func differ
   //  code[2]  The depth writing state differs
   // If code == 0, then there is no difference between both states.
   uint  compare( const DepthState& ds ) const
   {
      uint bits = _fields ^ ds._fields;
      uint code;
      code  = ((bits & 0x0010) != 0);
      code <<= 1;
      code |= ((bits & 0x000E) != 0);
      code <<= 1;
      code |= ((bits & 0x0001) != 0);
      return code;
   }

private: 

   /*----- data members -----*/

   //! NAME          SIZE   LOCATION         DESCRIPTION
   //! depthTesting   (1)   _fields[ 0: 0]   Controls whether or not the depth test is enabled.
   //! depthTestFunc  (3)   _fields[ 3: 1]   The depth compare function to use.
   //! depthWriting   (1)   _fields[ 4: 4]   Optionnally kills the writing of the depth value.
   uint _fields;
};
/*==============================================================================
   CLASS StencilState
==============================================================================*/

class StencilState:
   public RCObject
{
public:
   
   /*----- methods -----*/

   StencilState(): _fields1(0x0), _fields2(0x0) { setDefaults(); }
   StencilState( const StencilState& s ) { *this = s; }

   GFX_DLL_API void  setDefaults();

   void setInvalidFields() { _fields1 = 0xFFFFFFFF; _fields2 = 0xFFFFFFFF; }

   void         stencilTesting( bool st )        { _fields1 = setbits(_fields1, 0, 1, st); }
   void         stencilTestRef( int ref )        { _fields1 = setbits(_fields1, 1, 8, ref); }
   void         stencilTestRefMask( int mask )   { _fields1 = setbits(_fields1, 9, 8, mask); }
   void         stencilTestWriteMask( int mask ) { _fields1 = setbits(_fields1, 17, 8, mask); }
   bool         stencilTesting() const           { return getbits(_fields1, 0, 1) & 0x01; }
   int          stencilTestRef() const           { return getbits(_fields1, 1, 8 ); }
   int          stencilTestRefMask() const       { return getbits(_fields1, 9, 8); }
   int          stencilTestWriteMask() const     { return getbits(_fields1, 17, 8); }
   
   void         stencilTestFunc( CompareFunc cf )      { _fields2 = setbits(_fields2, 0, 3, cf); }
   void         stencilFailOp( StencilOp op )          { _fields2 = setbits(_fields2, 3, 3, op); }
   void         stencilPassDepthFailOp( StencilOp op ) { _fields2 = setbits(_fields2, 6, 3, op); }
   void         stencilPassDepthPassOp( StencilOp op ) { _fields2 = setbits(_fields2, 9, 3, op); }
   void         setStencilOps( StencilOp sf, StencilOp sp_df, StencilOp sp_dp );
   CompareFunc  stencilTestFunc() const                { return (CompareFunc)getbits(_fields2, 0, 3); }
   StencilOp    stencilFailOp() const                  { return (StencilOp)getbits(_fields2, 3, 3); }
   StencilOp    stencilPassDepthFailOp() const         { return (StencilOp)getbits(_fields2, 6, 3); }
   StencilOp    stencilPassDepthPassOp() const         { return (StencilOp)getbits(_fields2, 9, 3); }
   void         getStencilOps( StencilOp& sf, StencilOp& sp_df, StencilOp& sp_dp ) const;

   StencilState&  operator=( const StencilState& ss ) { _fields1 = ss._fields1; _fields2 = ss._fields2; return *this; }
   bool  operator==( const StencilState& ss ) const { return _fields1 == ss._fields1 && _fields2 == ss._fields2; }
   bool  operator!=( const StencilState& ss ) const { return !operator==(ss); }

   // Compares the states and returns a comparison code, where each bit corresponds to a difference in some fields:
   //  code[0]  The stencil testing state differs
   //  code[1]  The stencil test function differ
   //  code[2]  The stencil ref masks differ
   //  code[3]  The stencil write masks differ
   //  code[4]  The stencil test function differs
   //  code[5]  The stencil-fail operation differs
   //  code[6]  The stencil-pass-depth-fail operation differs
   //  code[7]  The stencil-pass-depth-pass operation differs
   // If code == 0, then there is no difference between both states.
   uint  compare( const StencilState& ss ) const
   {
      uint bits;
      uint code;

      bits = _fields2 ^ ss._fields2;
      if( bits != 0x0 )
      {
         code  = ((bits & 0x0E00) != 0);
         code <<= 1;
         code |= ((bits & 0x01C0) != 0);
         code <<= 1;
         code |= ((bits & 0x0038) != 0);
         code <<= 1;
         code |= ((bits & 0x0007) != 0);
      }
      else
      {
         code = 0;
      }

      bits = _fields1 ^ ss._fields1;
      if( bits != 0x0 )
      {
         code <<= 1;
         code |= ((bits & 0x01FE0000) != 0);
         code <<= 1;
         code |= ((bits & 0x0001FE00) != 0);
         code <<= 1;
         code |= ((bits & 0x000001FE) != 0);
         code <<= 1;
         code |= ((bits & 0x00000001) != 0);
      }
      else
      {
         code <<= 4;
      }
      return code;
   }

private:

   /*----- data members -----*/

   //! NAME                 SIZE   LOCATION         DESCRIPTION
   //! stencilTesting       (1)   _fields[ 0: 0]   Controls whether or not stencil test is enabled.
   //! stencilTestRef       (8)   _fields[ 8: 1]   Specifies the reference stencil value to test against.
   //! stencilTestRefMask   (8)   _fields[16: 9]   Mask being applied to the reference value.
   //! stencilTestWriteMask (8)   _fields[24:17]   Mask being applied to the stencil value before writing it.
   uint _fields1;
   
   //! NAME                    SIZE   LOCATION         DESCRIPTION
   //! stencilTestFunc         (3)   _fields[ 2: 0]   The compare function to use for stencil testing.
   //! stencilFailOp           (3)   _fields[ 5: 3]   The operation to perform when the stencil test fails (regardless of depth test).
   //! stencilPassDepthFailOp  (3)   _fields[ 8: 6]   The operation to perform when the stencil test passes, but the Z test fails.
   //! stencilPassDepthPassOp  (3)   _fields[11: 9]   The operation to perform when the stencil test and the Z test both pass.
   uint _fields2;
};

//------------------------------------------------------------------------------
//!
inline void 
StencilState::setStencilOps( StencilOp sf, StencilOp sp_df, StencilOp sp_dp ) 
{ 
   _fields2 = setbits(_fields2, 3, 3, sf);
   _fields2 = setbits(_fields2, 6, 3, sp_df);
   _fields2 = setbits(_fields2, 9, 3, sp_dp);
}

//------------------------------------------------------------------------------
//!
inline void
StencilState::getStencilOps( StencilOp& sf, StencilOp& sp_df, StencilOp& sp_dp ) const 
{ 
   sf    = (StencilOp)getbits(_fields2, 3, 3); 
   sp_df = (StencilOp)getbits(_fields2, 6, 3); 
   sp_dp = (StencilOp)getbits(_fields2, 9, 3);
}

/*==============================================================================
   class OffsetState
==============================================================================*/

class OffsetState:
   public RCObject
{

public: 

   /*----- methods -----*/

   OffsetState() { setDefaults(); }
   OffsetState( const OffsetState& s ) { *this = s; }

   GFX_DLL_API void  setDefaults();

   void setInvalidFields()   { _factor = 0.0f; _k = 0.0f; }

   void  factor( float f )   { _factor = f; }
   float factor() const      { return _factor; }

   void  constant( float k ) { _k = k; }
   float constant() const    { return _k; }

   OffsetState&  operator=( const OffsetState& os ) { _factor = os._factor; _k = os._k; return *this; }
   bool  operator==( const OffsetState& os ) const  { return _factor == os._factor && _k == os._k; }
   bool  operator!=( const OffsetState& os ) const  { return !operator==(os); }

private: 

   /*----- data members -----*/

   float _factor;
   float _k;
};

}  //namespace Gfx

NAMESPACE_END

#endif //GFX_RENDER_STATE_H
