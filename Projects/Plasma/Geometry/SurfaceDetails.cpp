/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/Geometry/SurfaceDetails.h>
#include <Plasma/Geometry/ParametricPatch.h>

#include <Fusion/Resource/RectPacker.h>
#include <Fusion/Resource/BitmapManipulator.h>
#include <Fusion/VM/VMRegistry.h>
#include <Fusion/VM/VMMath.h>
#include <Fusion/Core/Core.h>

#include <Base/ADT/StringMap.h>

/*==============================================================================
  UNNAME NAMESPACE
==============================================================================*/

UNNAMESPACE_BEGIN

//------------------------------------------------------------------------------
//!
enum {
   ATTRIB_COLOR,
   ATTRIB_ID,
   ATTRIB_FACE,
   ATTRIB_N,
   ATTRIB_POS,
   ATTRIB_UV,
};

StringMap _attributes(
   "color", ATTRIB_COLOR,
   "id",    ATTRIB_ID,
   "f",     ATTRIB_FACE,
   "n",     ATTRIB_N,
   "pos",   ATTRIB_POS,
   "uv",    ATTRIB_UV,
   ""
);

//------------------------------------------------------------------------------
//!
int in_get( VMState* vm )
{
   SurfaceDetails::Context* context = *(SurfaceDetails::Context**)VM::toPtr( vm, 1 );
   switch( _attributes[ VM::toCString( vm, 2 ) ] )
   {
      case ATTRIB_ID:   VM::push( vm, context->_id );       return 1;
      case ATTRIB_FACE: VM::push( vm, context->_fid );      return 1;
      case ATTRIB_N:    VMMath::push( vm, context->_inor ); return 1;
      case ATTRIB_POS:  VMMath::push( vm, context->_ipos ); return 1;
      case ATTRIB_UV:   VMMath::push( vm, context->_uv );   return 1;
   }

   return 0;
}

//------------------------------------------------------------------------------
//!
int out_set( VMState* vm )
{
   SurfaceDetails::Context* context = *(SurfaceDetails::Context**)VM::toPtr( vm, 1 );
   switch( _attributes[ VM::toCString( vm, 2 ) ] )
   {
      case ATTRIB_COLOR: context->_ocol = VMMath::toVec3( vm , 3 ); break;
      case ATTRIB_POS:   context->_opos = VMMath::toVec3( vm , 3 ); break;
   }
   return 0;
}

//------------------------------------------------------------------------------
//!
inline void scanline(
   uchar* src,
   int    inc,
   int    u0,
   int    u1,
   int    flag,
   float  error,
   int&   res
)
{
   Vec3f p0   = *(Vec3f*)src;
   Vec3f p1   = *(Vec3f*)(src+inc*(u1-u0));
   Vec3f pinc = (p1-p0)/float(u1-u0);
   for( int i = u0+1; i < u1; ++i )
   {
      p0  += pinc;
      src += inc;
      if( (p0-*(Vec3f*)src).sqrLength() > error ) { res |= flag; return; }
   }
}

UNNAMESPACE_END

NAMESPACE_BEGIN

/*==============================================================================
   CLASS SurfaceDetails
==============================================================================*/

//------------------------------------------------------------------------------
//!
SurfaceDetails::SurfaceDetails():
   _vm( NULL )
{
}

//------------------------------------------------------------------------------
//!
SurfaceDetails::~SurfaceDetails()
{
}

//------------------------------------------------------------------------------
//!
void
SurfaceDetails::clear()
{
   _patches.clear();
}

//------------------------------------------------------------------------------
//!
void
SurfaceDetails::reservePatches( uint num )
{
   _patches.reserve( num );
}

//------------------------------------------------------------------------------
//!
uint
SurfaceDetails::addPatch()
{
   uint i = uint(_patches.size());
   Patch p;
   _patches.pushBack( p );
   return i;
}

//------------------------------------------------------------------------------
//!
uint
SurfaceDetails::addPatch( const Vec2i& size )
{
   uint i = uint(_patches.size());
   Patch p;
   p._size    = CGM::max( size, Vec2i( 2, 2 ) );
   p._flipped = 0;
   //p._size.x = CGM::nextPow2( p._size.x );
   //p._size.y = CGM::nextPow2( p._size.y );
   _patches.pushBack( p );
   return i;
}

//------------------------------------------------------------------------------
//! Creates a minimal atlas (white color) to barely be able to see the surface.
void
SurfaceDetails::makeMinimalAtlas()
{
   if( _patches.size() == 0 )
   {
      addPatch( Vec2i( 2, 2 ) );
   }
   createAtlas();
   beginAtlas();
   endAtlas();
   memset( _color->pixels(), 0xFF, _color->size() );
}

//------------------------------------------------------------------------------
//!
void
SurfaceDetails::createAtlas()
{
   Timer timer;
   // Compute region occupied by each patch in texture atlas.
   RectPackerGreedy packer;
   packer.pack(
      ConstArrayAdaptor<Vec2i>( _patches.data(), sizeof(Patch) ),
      _patches.size(),
      Vec2i(2048),
      true, // Force power-of-two sizes.
      true  // Allow rotations.
   );
   // Copy resuls in patches.
   for( uint i = 0; i < _patches.size(); ++i )
   {
      _patches[i]._offset  = packer.rects()[i]._position;
      if( packer.rects()[i]._flipped )
      {
         _patches[i]._flipped = 1;
         CGM::swap( _patches[i]._size.x, _patches[i]._size.y );
      }
      else
      {
         _patches[i]._flipped = 0;
      }
   }

   // Allocate atlas memory.
   _size = packer.size();
   _color = new Bitmap( packer.size(), Bitmap::BYTE, 4 );
   memset( _color->pixels(), 0, _color->size() );
   _displacement = new Bitmap( packer.size(), Bitmap::FLOAT, 3 );

   StdErr << "Texture atlas size: " << _size << " (" << timer.elapsed() << "s)" << nl;

   // Debugging.
   //packer.print();
   //RCP<Bitmap> bmp = packer.getBitmap();
   //bmp->saveFile( "packer" );
}

//------------------------------------------------------------------------------
//!
void
SurfaceDetails::beginAtlas()
{
   if( _progName.empty() )  return;

   // Create vm.
   _vm = VM::open( VM_CAT_MAT | VM_CAT_MATH, true );

   // Compile script.
   VM::loadFile( _vm, _progName );

   // IN
   Context** inObj = (Context**)VM::newObject( _vm, sizeof(Context*) );
   *inObj = &_context;
   VM::newMetaTable( _vm, "IN" );
   VM::set( _vm, -1, "__index", in_get );
   VM::setMetaTable( _vm, -2 );
   VM::setGlobal( _vm, "IN" );

   // OUT
   Context** outObj = (Context**)VM::newObject( _vm, sizeof(Context*) );
   *outObj = &_context;
   VM::newMetaTable( _vm, "OUT" );
   VM::set( _vm, -1, "__newindex", out_set );
   VM::setMetaTable( _vm, -2 );
   VM::setGlobal( _vm, "OUT" );
}

//------------------------------------------------------------------------------
//!
void
SurfaceDetails::endAtlas()
{
   // End vm.
   VM::close( _vm );

   // Fix borders between patches to enable bilinear fetches.
   fixBorders();

   // Create normal map with the displacement texture.
   computeNormals();

   // Debugging.
   //_color->saveFile( "color" );
   //_displacement->saveFile( "displacement" );
}

//------------------------------------------------------------------------------
//!
void
SurfaceDetails::evaluatePatch( uint i, const ParametricPatch& pp, uint id, uint fid )
{
   _context._id = id;
   _context._fid = fid;

   int sx = patch(i)._size.x-1;
   int sy = patch(i)._size.y-1;

   Vec2i off = patch(i)._offset;

   bool hasProg = !_progName.empty();
   if( !patch(i)._flipped )
   {
      // Compute material values for each pixels of the patch.
      for( int y = 0; y <= sy; ++y )
      {
         uchar* c = _color->pixel( off + Vec2i(0,y) );
         float* d = (float*)_displacement->pixel( off + Vec2i(0,y) );

         float v = float(y)/float(sy);
         for( int x = 0; x <= sx; ++x )
         {
            float u = float(x)/float(sx);

            // Compute input parameters.
            _context._uv   = Vec2f( u, v );
            pp.parameters( _context._uv, _context._ipos, _context._inor );

            // Reset ouput parameters.
            _context._opos = _context._ipos;
            _context._ocol = Vec3f(1.0f);

            if( hasProg )
            {
               // Execute material program.
               VM::pushValue( _vm, -1 );
               VM::ecall( _vm, 0, 0 );
            }

            // Read back context and write results.
            Vec3f col = (_context._ocol * 255.0f).clamp( 0.0f, 255.0f );
            *c++ = (uchar)col.x;
            *c++ = (uchar)col.y;
            *c++ = (uchar)col.z;
            *c++ = 255;
            *d++ = _context._opos.x;
            *d++ = _context._opos.y;
            *d++ = _context._opos.z;
         }
      }
   }
   else
   {
      // Compute material values for each pixels of the patch.
      for( int y = 0; y <= sy; ++y )
      {
         uchar* c = _color->pixel( off + Vec2i(0,y) );
         float* d = (float*)_displacement->pixel( off + Vec2i(0,y) );

         float u = float(y)/float(sy);
         for( int x = 0; x <= sx; ++x )
         {
            float v = float(sx-x)/float(sx);

            // Compute input parameters.
            _context._uv   = Vec2f( u, v );
            pp.parameters( _context._uv, _context._ipos, _context._inor );

            // Reset ouput parameters.
            _context._opos = _context._ipos;
            _context._ocol = Vec3f(1.0f);

            if( hasProg )
            {
               // Execute material program.
               VM::pushValue( _vm, -1 );
               VM::ecall( _vm, 0, 0 );
            }

            // Read back context and write results.
            Vec3f col = (_context._ocol * 255.0f).clamp( 0.0f, 255.0f );
            *c++ = (uchar)col.x;
            *c++ = (uchar)col.y;
            *c++ = (uchar)col.z;
            *c++ = 255;
            *d++ = _context._opos.x;
            *d++ = _context._opos.y;
            *d++ = _context._opos.z;
         }
      }
   }
}

//------------------------------------------------------------------------------
//!
void
SurfaceDetails::flipNormals( uint i )
{
   int sx = patch(i)._size.x-1;
   int sy = patch(i)._size.y-1;
   Vec2i off = patch(i)._offset;

   for( int y = 0; y <= sy; ++y )
   {
      uchar* p  = _normal->pixel( off + Vec2i(0, y) );
      for( int x = 0; x <= sx; ++x )
      {
         *p = 255-*p; ++p;
         *p = 255-*p; ++p;
         *p = 255-*p; ++p;
         ++p;
      }
   }
}

//------------------------------------------------------------------------------
//!
void
SurfaceDetails::programName( const String& name )
{
   _progName = name;
}

//------------------------------------------------------------------------------
//!
void
SurfaceDetails::mapping( uint i, Vec2f& uv, Vec2f& du, Vec2f& dv ) const
{
   if( !patch(i)._flipped )
   {
      uv = (Vec2f( patch(i)._offset ) + Vec2f(0.5f)) / _size;
      du = Vec2f( (patch(i)._size.x-1) / _size.x, 0.0f );
      dv = Vec2f( 0.0f, (patch(i)._size.y-1) / _size.y );
   }
   else
   {
      uv = (Vec2f( patch(i)._offset ) + Vec2f((float)patch(i)._size.x-0.5f, 0.5f)) / _size;
      du = Vec2f( 0.0f, (patch(i)._size.y-1) / _size.y );
      dv = Vec2f( -(patch(i)._size.x-1) / _size.x, 0.0f );
   }
}

//------------------------------------------------------------------------------
//!
Vec3f
SurfaceDetails::position( uint i, const Vec2f& uv ) const
{
   const Patch& pi = patch(i);
   Vec2f st( pi._offset );

   if( !pi._flipped )
   {
      st.x += uv.x * float(pi._size.x-1);
      st.y += uv.y * float(pi._size.y-1);
   }
   else
   {
      st.y += uv.x * float(pi._size.y-1);
      st.x += (1.0f-uv.y) * float(pi._size.x-1);
   }
   int   xi, yi;
   float xf, yf;
   CGM::splitIntFrac( st.x, xi, xf );
   CGM::splitIntFrac( st.y, yi, yf );

   int incx = xi < pi._offset.x + pi._size.x-1 ? 1 : 0;

   uchar* p = _displacement->pixel( Vec2i( xi, yi ) );
   Vec3f bl = ((Vec3f*)p)[0];
   Vec3f br = ((Vec3f*)p)[incx];

   if( yi < pi._offset.y + pi._size.y-1 ) p += _displacement->lineSize();

   Vec3f tl = ((Vec3f*)p)[0];
   Vec3f tr = ((Vec3f*)p)[incx];

   return CGM::bilinear(
      bl, br-bl,
      tl, tr-tl,
      xf, yf
   );
}

//------------------------------------------------------------------------------
//!
void
SurfaceDetails::parameters( uint i, const Vec2f& uv, Vec3f& pos, Vec3f& normal ) const
{
   const Patch& pi = patch(i);
   Vec2f st( pi._offset );

   if( !pi._flipped )
   {
      st.x += uv.x * float(pi._size.x-1);
      st.y += uv.y * float(pi._size.y-1);
   }
   else
   {
      st.y += uv.x * float(pi._size.y-1);
      st.x += (1.0f-uv.y) * float(pi._size.x-1);
   }
   int   xi, yi;
   float xf, yf;
   CGM::splitIntFrac( st.x, xi, xf );
   CGM::splitIntFrac( st.y, yi, yf );

   int incx = xi < pi._offset.x + pi._size.x-1 ? 1 : 0;

   uchar* p = _displacement->pixel( Vec2i( xi, yi ) );
   Vec3f bl = ((Vec3f*)p)[0];
   Vec3f br = ((Vec3f*)p)[incx];

   if( yi < pi._offset.y + pi._size.y-1 ) p += _displacement->lineSize();

   Vec3f tl = ((Vec3f*)p)[0];
   Vec3f tr = ((Vec3f*)p)[incx];

   pos = CGM::bilinear(
      bl, br-bl,
      tl, tr-tl,
      xf, yf
   );

   incx *= 4;

   p = _normal->pixel( Vec2i( xi, yi ) );
   bl = (Vec3f( p[0], p[1], p[2] ) / 127.5f) - 1.0f;
   br = (Vec3f( p[0+incx], p[1+incx], p[2+incx] ) / 127.5f) - 1.0f;

   if( yi < pi._offset.y + pi._size.y-1 ) p += _normal->lineSize();

   tl = (Vec3f( p[0], p[1], p[2] ) / 127.5f) - 1.0f;
   tr = (Vec3f( p[0+incx], p[1+incx], p[2+incx] ) / 127.5f) - 1.0f;

   normal = CGM::bilinear(
      bl, br-bl,
      tl, tr-tl,
      xf, yf
   );
}

//------------------------------------------------------------------------------
//!
int
SurfaceDetails::flatness(
   uint         i,
   const Vec2f& uv0,
   const Vec2f& uv1,
   int          edge,
   float        error
) const
{
   float error2 = error*error;

   // Find uv coordinates in texture space.
   Vec2f st0( patch(i)._offset );
   Vec2f st1( patch(i)._offset );
   if( !patch(i)._flipped )
   {
      st0.x += uv0.x * float(patch(i)._size.x-1);
      st0.y += uv0.y * float(patch(i)._size.y-1);
      st1.x += uv1.x * float(patch(i)._size.x-1);
      st1.y += uv1.y * float(patch(i)._size.y-1);

      const float r = 0.4990234375;
      int u0 = int( st0.x+r );
      int v0 = int( st0.y+r );
      int u1 = int( st1.x+r );
      int v1 = int( st1.y+r );

      int subu   = 0;
      int subv   = 0;
      uchar* src = 0;
      // Possible subdivision in uv.
      // Test u.
      if( u1-u0 > 1 )
      {
         int inc = int(_displacement->pixelSize());

         // Edge 0.
         if( edge&1 )
         {
            src = _displacement->pixel( Vec2i( u0, v0 ) );
            scanline( src, inc, u0, u1, (1|4), error2, subu );
         }

         // Edge 2.
         if( edge&4 )
         {
            src = _displacement->pixel( Vec2i( u0, v1 ) );
            scanline( src, inc, u0, u1, (1|16), error2, subu );
         }

         // Interior.
         for( int j = v0+1; (j < v1)&&(!subu); ++j )
         {
            src = _displacement->pixel( Vec2i( u0, j ) );
            scanline( src, inc, u0, u1, 1, error2, subu );
         }
      }

      // Test v.
      if( v1-v0 > 1 )
      {
         int inc = int(_displacement->lineSize());

         // Edge 3.
         if( edge&8 )
         {
            src = _displacement->pixel( Vec2i( u0, v0 ) );
            scanline( src, inc, v0, v1, (2|32), error2, subv );
         }

         // Edge 1.
         if( edge&2 )
         {
            src = _displacement->pixel( Vec2i( u1, v0 ) );
            scanline( src, inc, v0, v1, (2|8), error2, subv );
         }

         // Interior.
         for( int i = u0+1; (i < u1)&&(!subv); ++i )
         {
            uchar* src = _displacement->pixel( Vec2i( i, v0 ) );
            scanline( src, inc, v0, v1, 2, error2, subv );
         }
      }
      return (subu|subv);
   }
   else
   {
      st0.y += uv0.x * float(patch(i)._size.y-1);
      st0.x += (1.0f-uv0.y) * float(patch(i)._size.x-1);
      st1.y += uv1.x * float(patch(i)._size.y-1);
      st1.x += (1.0f-uv1.y) * float(patch(i)._size.x-1);

      const float r = 0.4990234375;
      int u0 = int( st1.x+r );
      int v0 = int( st0.y+r );
      int u1 = int( st0.x+r );
      int v1 = int( st1.y+r );

      int subu   = 0;
      int subv   = 0;
      uchar* src = 0;
      // Possible subdivision in uv.
      // Test u.
      if( u1-u0 > 1 )
      {
         int inc = int(_displacement->pixelSize());

         // Edge 3.
         if( edge&8 )
         {
            src = _displacement->pixel( Vec2i( u0, v0 ) );
            scanline( src, inc, u0, u1, (2|32), error2, subu );
         }

         // Edge 1.
         if( edge&2 )
         {
            src = _displacement->pixel( Vec2i( u0, v1 ) );
            scanline( src, inc, u0, u1, (2|8), error2, subu );
         }

         // Interior.
         for( int j = v0+1; (j < v1)&&(!subu); ++j )
         {
            src = _displacement->pixel( Vec2i( u0, j ) );
            scanline( src, inc, u0, u1, 2, error2, subu );
         }
      }

      // Test v.
      if( v1-v0 > 1 )
      {
         int inc = int(_displacement->lineSize());

         // Edge 2.
         if( edge&4 )
         {
            src = _displacement->pixel( Vec2i( u0, v0 ) );
            scanline( src, inc, v0, v1, (1|16), error2, subv );
         }

         // Edge 0.
         if( edge&1 )
         {
            src = _displacement->pixel( Vec2i( u1, v0 ) );
            scanline( src, inc, v0, v1, (1|4), error2, subv );
         }

         // Interior.
         for( int i = u0+1; (i < u1)&&(!subv); ++i )
         {
            src = _displacement->pixel( Vec2i( i, v0 ) );
            scanline( src, inc, v0, v1, 1, error2, subv );
         }
      }
      return (subu|subv);
   }

   return 0;
}

//------------------------------------------------------------------------------
//!
void
SurfaceDetails::computeSamplers()
{
   // Create texture.
   uint width  = _color->dimension().x;
   uint height = _color->dimension().y;
   RCP<Gfx::Texture> colorTex = Core::gfx()->create2DTexture(
      width, height, Gfx::TEX_FMT_8_8_8_8, Gfx::TEX_CHANS_RGBA, Gfx::TEX_FLAGS_NONE//Gfx::TEX_FLAGS_MIPMAPPED
   );
   Core::gfx()->setData( colorTex, 0, 0, 0, width, height, _color->pixels() );
   //Core::gfx()->generateMipmaps( texture );

   RCP<Gfx::Texture> normalTex = Core::gfx()->create2DTexture(
      width, height, Gfx::TEX_FMT_8_8_8_8, Gfx::TEX_CHANS_RGBA, Gfx::TEX_FLAGS_NONE
   );
   Core::gfx()->setData( normalTex, 0, 0, 0, width, height, _normal->pixels() );

   // FIXME: Remove _color and _normal?

   // TODO: use only one texture state for all surface details.
   Gfx::TextureState texState;
   texState.setBilinear();

   _colorSampler  = new Gfx::Sampler( "colorTex", colorTex, texState );
   _normalSampler = new Gfx::Sampler( "normalTex", normalTex, texState );
}

//------------------------------------------------------------------------------
//!
void
SurfaceDetails::fixBorders()
{
   if( _patches.size() <= 1 )  return;
   // Fix corners.
   for( uint i = 0; i < _patches.size(); ++i )
   {
      for( uint e = 0; e < 4; ++e )
      {
         // Compute the mean of all corners.
         Vec3f sumV(0.0f);
         uint numV  = 0;
         uint cpi   = i;
         uint ce    = e;
         do
         {
            // Add another corner to the sum.
            ++numV;
            Patch& cp  = patch( cpi );
            Vec2i off  = patchOffset( cp, ce );
            Vec3f* src = (Vec3f*)_displacement->pixel( off );
            sumV      += *src;
            // Compute next corner.
            cpi       = neighborPatch( cp, ce );
            ce        = (neighborEdge( cp, ce ) + 1) % 4;
         } while ( cpi != i );
         sumV /= (float)numV;
         // Assign position to all neighbor patches.
         do
         {
            Patch& cp  = patch( cpi );
            Vec2i off  = patchOffset( cp, ce );
            Vec3f* src = (Vec3f*)_displacement->pixel( off );
            *src = sumV;
            // Compute next corner.
            cpi       = neighborPatch( cp, ce );
            ce        = (neighborEdge( cp, ce ) + 1) % 4;
         } while ( cpi != i );
      }
   }

   // Fix edges.
   for( uint i = 0; i < _patches.size(); ++i )
   {
      Patch& p = patch(i);
      Vec2i s  = p._size - Vec2i(1);

      // Edge.
      for( uint e = 0; e < 4; ++e )
      {
         int edge = p._flipped ? (e+3)%4 : e;
         uint npi = neighborPatch( p, edge );
         if( npi < i ) continue;

         Patch& np = patch(npi);
         Vec2i ns  = np._size - Vec2i(1);
         uint ne   = neighborEdge( p, edge );

         if( np._flipped ) ne = (ne+1)%4;

         // Find offsets and increments.
         int axis  = e&1;
         int naxis = ne&1;
         int len   = s(axis);
         int nlen  = ns(naxis);

         Vec2i off  = p._offset;
         Vec2i noff = np._offset;
         int inc    = 0;
         int ninc   = 0;
         float duv  = 0;
         float nduv = 0;
         switch( e )
         {
            case 0:
               duv    = float(len)/float(nlen);
               inc    = int(_displacement->pixelSize());
               break;
            case 1:
               off.x += s.x;
               duv    = float(len)/float(nlen);
               inc    = int(_displacement->lineSize());
               break;
            case 2:
               off   += s;
               duv    = -float(len)/float(nlen);
               inc    = -int(_displacement->pixelSize());
               break;
            case 3:
               off.y += s.y;
               duv    = -float(len)/float(nlen);
               inc    = -int(_displacement->lineSize());
               break;
         }
         switch( ne )
         {
            case 0:
               noff.x += ns.x;
               nduv    = -float(nlen)/float(len);
               ninc    = -int(_displacement->pixelSize());
               break;
            case 1:
               noff   += ns;
               nduv    = -float(nlen)/float(len);
               ninc    = -int(_displacement->lineSize());
               break;
            case 2:
               noff.y += ns.y;
               nduv    = float(nlen)/float(len);
               ninc    = int(_displacement->pixelSize());
               break;
            case 3:
               nduv    = float(nlen)/float(len);
               ninc    = int(_displacement->lineSize());
               break;
         }
         Vec3f displ;
         if( len <= nlen )
         {
            // Blend on smallest edge.
            uchar* dst  = _displacement->pixel( off ) + inc;
            Vec2f pos   = noff;
            pos(naxis) += nduv;
            for( int j = 1; j < len; ++j, dst += inc, pos(naxis) += nduv )
            {
               BitmapManipulator::linearRGB32f( *_displacement, pos, naxis, (uchar*)&displ );
               *(Vec3f*)dst = (*(Vec3f*)dst + displ)*0.5f;
            }
            // Copy on longest edge.
            pos = off;
            pos(axis) += duv;
            dst = _displacement->pixel( noff ) + ninc;
            for( int j = 1; j < nlen; ++j, dst += ninc, pos(axis) += duv )
            {
               BitmapManipulator::linearRGB32f( *_displacement, pos, axis, dst );
            }
         }
         else
         {
            // Blend on smallest edge.
            uchar* dst = _displacement->pixel( noff ) + ninc;
            Vec2f pos  = off;
            pos(axis) += duv;
            for( int j = 1; j < nlen; ++j, dst += ninc, pos(axis) += duv )
            {
               BitmapManipulator::linearRGB32f( *_displacement, pos, axis, (uchar*)&displ );
               *(Vec3f*)dst = (*(Vec3f*)dst + displ)*0.5f;
            }
            // Copy on longest edge.
            pos = noff;
            pos(naxis) += nduv;
            dst = _displacement->pixel( off ) + inc;
            for( int j = 1; j < len; ++j, dst += inc, pos(naxis) += nduv )
            {
               BitmapManipulator::linearRGB32f( *_displacement, pos, naxis, dst );
            }
         }
      }
   }
}

//------------------------------------------------------------------------------
//!
void
SurfaceDetails::computeNormals()
{
   // Allocate memory.
   _normal = new Bitmap( _size, Bitmap::BYTE, 4 );

   // Compute normals for each patch.
   for( uint i = 0; i < _patches.size(); ++i )
   {
      int sx = patch(i)._size.x-1;
      int sy = patch(i)._size.y-1;
      Vec2i off = patch(i)._offset;

      for( int y = 0; y <= sy; ++y )
      {
         Vec3f* p1 = (Vec3f*)_displacement->pixel( off + Vec2i(0,y) );
         Vec3f* p0 = (y == 0)  ? p1 : (Vec3f*)_displacement->pixel( off + Vec2i(0,y-1) );
         Vec3f* p2 = (y == sy) ? p1 : (Vec3f*)_displacement->pixel( off + Vec2i(0,y+1) );
         uchar* p  = _normal->pixel( off + Vec2i(0, y) );

         for( int x = 0; x <= sx; ++x )
         {
            Vec3f n10 = p0[0];
            Vec3f n11 = p1[0];
            Vec3f n12 = p2[0];
            Vec3f n00 = (x == 0)  ? n10 : p0[-1];
            Vec3f n01 = (x == 0)  ? n11 : p1[-1];
            Vec3f n02 = (x == 0)  ? n12 : p2[-1];
            Vec3f n20 = (x == sx) ? n10 : p0[1];
            Vec3f n21 = (x == sx) ? n11 : p1[1];
            Vec3f n22 = (x == sx) ? n12 : p2[1];

            ++p0;
            ++p1;
            ++p2;

#define NORMAL_DUDV
            // 4 triangles.
#ifdef NORMAL_4
            Vec3f e0 = n10-n11;
            Vec3f e1 = n21-n11;
            Vec3f e2 = n12-n11;
            Vec3f e3 = n01-n11;
            Vec3f n  = (
               CGM::cross( e0, e1 ) +
               CGM::cross( e1, e2 ) +
               CGM::cross( e2, e3 ) +
               CGM::cross( e3, e0 ) ).getNormalized();
#endif
            // 8 triangles.
            // du/dv.
#ifdef NORMAL_DUDV
            Vec3f du = n21-n01;
            Vec3f dv = n12-n10;
            Vec3f n  =  CGM::cross( du, dv ).getNormalized();
#endif
            // Sobel.
#ifdef NORMAL_SOBEL
            Vec3f du = n20 + n21*2.0f + n22 - n00 - n01*2.0f - n02;
            Vec3f dv = n00 + n10*2.0f + n20 - n02 - n12*2.0f - n22;
            Vec3f n  =  CGM::cross( dv, du ).getNormalized();
#endif
            n = ((n + 1.0f)*127.5f).clamp( 0.0f, 255.0f );
            *p++ = (uchar)n.x;
            *p++ = (uchar)n.y;
            *p++ = (uchar)n.z;
            *p++ = 255;
         }
      }
   }

   if( _patches.size() <= 1 )  return;

   // Filter corners.
   for( uint i = 0; i < _patches.size(); ++i )
   {
      for( uint e = 0; e < 4; ++e )
      {
         // Find starting index.
         uint sce = (e+3)%4;
         uint si  = i;
         do
         {
            Patch& cp = patch( si );
            if( crease( cp, sce ) ) break;
            si  = neighborPatch( cp, sce );
            sce = (neighborEdge( cp, sce ) + 3)%4;
         } while( si != i );

         sce = (sce+1)%4;
         Vec4i sumNormal(0);
         uint cpi  = si;
         uint ce   = sce;
         uint numV = 0;
         // Compute mean normal.
         do
         {
            ++numV;
            Patch& cp        = patch( cpi );
            Vec2i off        = patchOffset( cp, ce );
            Vec4<uchar>* src = (Vec4<uchar>*)_normal->pixel( off );
            sumNormal       += *src;
            // Compute next corner.
            if( crease( cp, ce ) ) break;
            cpi       = neighborPatch( cp, ce );
            ce        = (neighborEdge( cp, ce ) + 1) % 4;
         } while( cpi != si );
         sumNormal /= numV;
         // Assign color.
         cpi = si;
         ce  = sce;
         do
         {
            Patch& cp        = patch( cpi );
            Vec2i off        = patchOffset( cp, ce );
            Vec4<uchar>* src = (Vec4<uchar>*)_normal->pixel( off );
            *src = sumNormal;
            // Compute next corner.
            if( crease( cp, ce ) ) break;
            cpi       = neighborPatch( cp, ce );
            ce        = (neighborEdge( cp, ce ) + 1) % 4;
         } while( cpi != si );
      }
   }

   // Filter normals on non creased edges.
   for( uint i = 0; i < _patches.size(); ++i )
   {
      Patch& p = patch(i);
      Vec2i s  = p._size - Vec2i(1);

      // Edge.
      for( uint e = 0; e < 4; ++e )
      {
         int edge = p._flipped ? (e+3)%4 : e;
         if( crease( p, edge ) ) continue;

         uint npi = neighborPatch( p, edge );
         if( npi < i ) continue;

         Patch& np = patch(npi);
         Vec2i ns  = np._size - Vec2i(1);
         uint ne   = neighborEdge( p, edge );

         if( np._flipped ) ne = (ne+1)%4;

         // Find offsets and increments.
         int axis  = e&1;
         int naxis = ne&1;
         int len   = s(axis);
         int nlen  = ns(naxis);

         Vec2i off  = p._offset;
         Vec2i noff = np._offset;
         int inc    = 0;
         int ninc   = 0;
         float duv  = 0;
         float nduv = 0;
         switch( e )
         {
            case 0:
               duv    = float(len)/float(nlen);
               inc    = int(_normal->pixelSize());
               break;
            case 1:
               off.x += s.x;
               duv    = float(len)/float(nlen);
               inc    = int(_normal->lineSize());
               break;
            case 2:
               off   += s;
               duv    = -float(len)/float(nlen);
               inc    = -int(_normal->pixelSize());
               break;
            case 3:
               off.y += s.y;
               duv    = -float(len)/float(nlen);
               inc    = -int(_normal->lineSize());
               break;
         }
         switch( ne )
         {
            case 0:
               noff.x += ns.x;
               nduv    = -float(nlen)/float(len);
               ninc    = -int(_normal->pixelSize());
               break;
            case 1:
               noff   += ns;
               nduv    = -float(nlen)/float(len);
               ninc    = -int(_normal->lineSize());
               break;
            case 2:
               noff.y += ns.y;
               nduv    = float(nlen)/float(len);
               ninc    = int(_normal->pixelSize());
               break;
            case 3:
               nduv    = float(nlen)/float(len);
               ninc    = int(_normal->lineSize());
               break;
         }
         uchar color[4];
         if( len <= nlen )
         {
            // Blend on smallest edge.
            uchar* dst = _normal->pixel( off );
            Vec2f pos  = noff;
            for( int j = 0; j <= len; ++j, dst += inc, pos(naxis) += nduv )
            {
               BitmapManipulator::linearRGBA8( *_normal, pos, naxis, color );
               dst[0] = (dst[0] + color[0])/2;
               dst[1] = (dst[1] + color[1])/2;
               dst[2] = (dst[2] + color[2])/2;
            }
            // Copy on longest edge.
            pos = off;
            dst = _normal->pixel( noff );
            for( int j = 0; j <= nlen; ++j, dst += ninc, pos(axis) += duv )
            {
               BitmapManipulator::linearRGBA8( *_normal, pos, axis, dst );
            }
         }
         else
         {
            // Blend on smallest edge.
            uchar* dst = _normal->pixel( noff );
            Vec2f pos  = off;
            for( int j = 0; j <= nlen; ++j, dst += ninc, pos(axis) += duv )
            {
               BitmapManipulator::linearRGBA8( *_normal, pos, axis, color );
               dst[0] = (dst[0] + color[0])/2;
               dst[1] = (dst[1] + color[1])/2;
               dst[2] = (dst[2] + color[2])/2;
            }
            // Copy on longest edge.
            pos = noff;
            dst = _normal->pixel( off );
            for( int j = 0; j <= len; ++j, dst += inc, pos(naxis) += nduv )
            {
               BitmapManipulator::linearRGBA8( *_normal, pos, naxis, dst );
            }
         }
      }
   }

   // Debugging.
   //_normal->saveFile( "normals_4" );
   //_normal->saveFile( "normals_dudv" );
   //_normal->saveFile( "normals_sobel" );
}

NAMESPACE_END
