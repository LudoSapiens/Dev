/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef GFX_TEXTURE_STATE_H
#define GFX_TEXTURE_STATE_H

#include <Gfx/StdDefs.h>

#include <Base/Util/Bits.h>
#include <Base/Util/RCObject.h>
#include <Base/ADT/String.h>


NAMESPACE_BEGIN

namespace Gfx
{


/*----- types -----*/
typedef enum
{
   TEX_FILTER_NONE   = 0,
   TEX_FILTER_MINMAG = 0,
   TEX_FILTER_POINT  = 1,
   TEX_FILTER_LINEAR = 2,
   TEX_FILTER_CUBIC  = 3
} TexFilter;

typedef enum
{
   TEX_CLAMP_WRAP,
   TEX_CLAMP_MIRROR,
   TEX_CLAMP_LAST,
   TEX_CLAMP_BORDER,
   TEX_CLAMP_MIRRORONCE_LAST,
   TEX_CLAMP_MIRRORONCE_BORDER
} TexClamp;

class TextureState
{
public:

   /*----- methods -----*/

   GFX_DLL_API TextureState();

   //GFX_DLL_API virtual ~TextureState();

   GFX_DLL_API void  setDefaults();
   GFX_DLL_API void  setPointSampling();
   GFX_DLL_API void  setBilinear();
   GFX_DLL_API void  setTrilinear();
   GFX_DLL_API void  setVolumeTrilinear();

   void  minFilter( const TexFilter f ) { _fields = setbits(_fields, 0, 2, f); }
   void  magFilter( const TexFilter f ) { _fields = setbits(_fields, 2, 2, f); }
   void  zFilter  ( const TexFilter f ) { _fields = setbits(_fields, 4, 2, f); }
   void  mipFilter( const TexFilter f ) { _fields = setbits(_fields, 6, 2, f); }
   TexFilter  minFilter() const         { return (TexFilter)getbits(_fields, 0, 2); }
   TexFilter  magFilter() const         { return (TexFilter)getbits(_fields, 2, 2); }
   TexFilter  zFilter  () const         { return (TexFilter)getbits(_fields, 4, 2); }
   TexFilter  mipFilter() const         { return (TexFilter)getbits(_fields, 6, 2); }

   void  clamp( const TexClamp c )      { _fields = setbits(_fields, 16, 3, c); _fields = setbits(_fields, 19, 3, c); _fields = setbits(_fields, 22, 3, c); }
   void  clampX( const TexClamp c )     { _fields = setbits(_fields, 16, 3, c); }
   void  clampY( const TexClamp c )     { _fields = setbits(_fields, 19, 3, c); }
   void  clampZ( const TexClamp c )     { _fields = setbits(_fields, 22, 3, c); }
   TexClamp  clampX() const             { return (TexClamp)getbits(_fields, 16, 3); }
   TexClamp  clampY() const             { return (TexClamp)getbits(_fields, 19, 3); }
   TexClamp  clampZ() const             { return (TexClamp)getbits(_fields, 22, 3); }

   void  baseLevel( const uint l )      { _fields = setbits( _fields, 8, 4, (l>15)?(15):(l) ); }
   uint  baseLevel() const              { return getbits( _fields, 8, 4 ); }

   void  lastLevel( const uint l )      { _fields = setbits( _fields, 12, 4, (l>15)?(15):(l) ); }
   uint  lastLevel() const              { return getbits( _fields, 12, 4 ); }

   void  maxAniso( const uint m )       { _fields = setbits( _fields, 27, 5, (m>16)?(16):(m) ); }
   uint  maxAniso() const               { return getbits(_fields, 27, 5); }

   void   LODBias( const float b )      { _LODBias = b; }
   float  LODBias() const               { return _LODBias; }

   GFX_DLL_API void  print() const;

   void  setInvalidFields()             { _fields = kINVALID_FIELDS; }
   bool  isInvalid() const              { return _fields == kINVALID_FIELDS; }

   uint  getFields() const              { return _fields; }
   uint  getFilterFields() const        { return getbits(_fields, 0, 4*3); }  //maxAniso?
   uint  getClampFields() const         { return getbits(_fields, 16, 3*3); }

   TextureState&  operator=( const TextureState& ts ) { _fields = ts._fields; _LODBias = ts._LODBias; return *this; }
   bool  operator==( const TextureState& ts ) const   { return _fields == ts._fields && _LODBias == ts._LODBias; }
   bool  operator!=( const TextureState& ts ) const   { return !operator==(ts); }

   // Compares the states and returns a comparison code, where each bit corresponds to a difference in some fields:
   //  code[0]  The filtering state differs
   //  code[1]  The clamping state differs
   //  code[2]  The anisotropy differs
   //  code[3]  The mip level/bias differs
   // If code == 0, then there is no difference between both states.
   uint  compare( const TextureState& as ) const
   {
      uint bits = _fields ^ as._fields;
      uint code = ( _LODBias != as._LODBias ) || ( (bits & 0x0000FF00) != 0 );
      if( bits != 0x0 )
      {
         code <<= 1;
         code |= ((bits & 0xF8000000) != 0);
         code <<= 1;
         code |= ((bits & 0x01FF0000) != 0);
         code <<= 1;
         code |= ((bits & 0x000000FF) != 0);
      }
      else
      {
         code <<= 3;
      }
      return code;
   }


protected:

   /*----- data members -----*/

   static const uint32_t kINVALID_FIELDS = 0xFFFFFFFF;

   //! NAME         SIZE   LOCATION         DESCRIPTION
   //! xyMinFilter   (2)   _fields[ 1: 0]   The minification filter for X and Y (POINT, LINEAR, CUBIC)
   //! xyMagFilter   (2)   _fields[ 3: 2]   The magnification filter for X and Y (POINT, LINEAR, CUBIC)
   //! zFilter       (2)   _fields[ 5: 4]   The filter to use between Z slices (MINMAG, POINT, LINEAR)
   //! mipFilter     (2)   _fields[ 7: 6]   The filter to use between mip levels (NONE, POINT, LINEAR)
   //!
   //! baseLevel     (4)   _fields[11: 8]   The largest mip level allowed (from 0 to numLevels-1)
   //! lastLevel     (4)   _fields[15:12]   The smallest mip level allowed (from baseLevel to numLevels-1)
   //!
   //! clampX        (3)   _fields[18:16]   The clamp policy to use on the X coordinate (aka S, aka U)
   //! clampY        (3)   _fields[21:19]   The clamp policy to use on the Y coordinate (aka T, aka V)
   //! clampZ        (3)   _fields[24:22]   The clamp policy to use on the Z coordinate (aka U, aka W)
   //!
   //! maxAniso      (5)   _fields[31:27]   Max aniso ratio (0 means anisotropic filtering is disabled)
   uint32_t  _fields;
   float     _LODBias;   //!< The LOD bias to apply to the computed MIP level

private:

   GFX_MAKE_MANAGERS_FRIENDS();
};


//------------------------------------------------------------------------------
//!
String toStr( const TexFilter f );


//------------------------------------------------------------------------------
//!
String toStr( const TexClamp c );


} //namespace Gfx

NAMESPACE_END


#endif //GFX_TEXTURE_STATE_H
