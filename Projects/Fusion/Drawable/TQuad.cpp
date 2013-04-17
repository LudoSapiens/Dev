/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Fusion/Drawable/TQuad.h>
#include <Fusion/VM/VMRegistry.h>
#include <Fusion/Resource/ResManager.h>
#include <Fusion/Resource/Image.h>
#include <Fusion/Core/Core.h>

#include <CGMath/CGMath.h>

#include <Base/ADT/StringMap.h>

/*==============================================================================
  UNNAME NAMESPACE
==============================================================================*/

UNNAMESPACE_BEGIN

Gfx::TextureState  _texState;
Vector<Vec4f> _vdata( 16 );

uint _verticesSize[] = { 4*16, 8*16, 8*16, 16*16, 16*16, 4*16, 16*16 };
uint _indicesSize[]  = { 6*2, 18*2, 18*2, 54*2, 54*2, 6*2, 48*2 };

ushort _verIndices[] =
{
   0, 1, 3,
   0, 3, 2,
   2, 3, 5,
   2, 5, 4,
   4, 5, 7,
   4, 7, 6
};

ushort _horIndices[] =
{
   0,   1,  5,
   0,   5,  4,
   1,   2,  6,
   1,   6,  5,
   2,   3,  7,
   2,   7,  6,
   4,   5,  9,
   4,   9,  8,
   5,   6, 10,
   5,  10,  9,
   6,   7, 11,
   6,  11, 10,
   8,   9, 13,
   8,  13, 12,
   9,  10, 14,
   9,  14, 13,
   10, 11, 15,
   10, 15, 14
};

ushort _contourIndices[] =
{
   0,   1,  5,
   0,   5,  4,
   1,   2,  6,
   1,   6,  5,
   2,   3,  7,
   2,   7,  6,
   4,   5,  9,
   4,   9,  8,
   6,   7, 11,
   6,  11, 10,
   8,   9, 13,
   8,  13, 12,
   9,  10, 14,
   9,  14, 13,
   10, 11, 15,
   10, 15, 14
};

enum {
   ATTRIB_COLOR,
   ATTRIB_IMAGE,
   ATTRIB_TYPE,
   ATTRIB_U,
   ATTRIB_V,
   ATTRIB_POSITION,
   ATTRIB_SIZE,
   ATTRIB_IMAGESIZE
};

StringMap _attributes(
   "color",     ATTRIB_COLOR,
   "image",     ATTRIB_IMAGE,
   "type",      ATTRIB_TYPE,
   "u",         ATTRIB_U,
   "v",         ATTRIB_V,
   "position",  ATTRIB_POSITION,
   "size",      ATTRIB_SIZE,
   "imageSize", ATTRIB_IMAGESIZE,
   ""
);

const VM::EnumReg _enumsTQuadType[] = {
   { "NORMAL",          TQuad::NORMAL         },
   { "HORIZONTAL",      TQuad::HORIZONTAL     },
   { "VERTICAL",        TQuad::VERTICAL       },
   { "GRID",            TQuad::GRID           },
   { "ADJUST_BORDERS",  TQuad::ADJUST_BORDERS },
   { "CENTERED",        TQuad::CENTERED       },
   { "CONTOUR",         TQuad::CONTOUR        },
   { 0, 0 }
};


//------------------------------------------------------------------------------
//!
const char* _tquad_str_ = "tquad";

//------------------------------------------------------------------------------
//!
void
initVM( VMState* vm, uint /*mask*/ )
{
   VM::registerEnum( vm, "UI.TQuadType", _enumsTQuadType );
}

//------------------------------------------------------------------------------
//!
int
tquad_get( VMState* vm )
{
   TQuad* d = (TQuad*)VM::toProxy( vm, 1 );
   return d->performGet( vm ) ? 1 : 0;
}

//------------------------------------------------------------------------------
//!
int
tquad_set( VMState* vm )
{
   TQuad* d = (TQuad*)VM::toProxy( vm, 1 );
   d->performSet( vm );
   return 0;
}

UNNAMESPACE_END

NAMESPACE_BEGIN

/*==============================================================================
  CLASS TQuad
==============================================================================*/

//------------------------------------------------------------------------------
//!
void
TQuad::initialize()
{
   _texState.magFilter( Gfx::TEX_FILTER_LINEAR );
   _texState.minFilter( Gfx::TEX_FILTER_LINEAR );
   _texState.mipFilter( Gfx::TEX_FILTER_NONE );
   _texState.clampX( Gfx::TEX_CLAMP_LAST );
   _texState.clampY( Gfx::TEX_CLAMP_LAST );

   VMRegistry::add( _tquad_str_, NULL, tquad_get, tquad_set, VM_CAT_APP );
   VMRegistry::add( initVM, VM_CAT_APP );
}

//------------------------------------------------------------------------------
//!
TQuad::TQuad()
   : Drawable(),
     _size( 0.0f, 0.0f ),
     _type( GRID ),
     _u( 0, 0.2f, 0.8f, 1.0f ),
     _v( 0, 0.2f, 0.8f, 1.0f ),
     _color( 1.0f, 1.0f, 1.0f, 1.0f )
{
   _geom = Core::gfx()->createGeometry( Gfx::PRIM_TRIANGLES );
   _constants = Core::gfx()->createConstants( 16 );
   _constants->addConstant( "color", Gfx::CONST_FLOAT4, 0 );
   _constants->setConstant( "color", _color.ptr() );
   _cl = Gfx::ConstantList::create( _constants );
}

//------------------------------------------------------------------------------
//!
TQuad::~TQuad()
{}

//------------------------------------------------------------------------------
//!
void
TQuad::type( Type val )
{
   _type = val;
   update();
}

//------------------------------------------------------------------------------
//!
void
TQuad::u( const Vec4f& val )
{
   _u = val;
   update();
}

//------------------------------------------------------------------------------
//!
void
TQuad::v( const Vec4f& val )
{
   _v = val;
   update();
}

//------------------------------------------------------------------------------
//!
void
TQuad::color( const Vec4f& val )
{
   _color = val;
   _constants->setConstant( "color", _color.ptr() );
}

//------------------------------------------------------------------------------
//!
void
TQuad::size( const Vec2f& val )
{
   if( val == _size ) return;

   _size = val;

   CGM::clampMin( _size.x, 0.0f );
   CGM::clampMin( _size.y, 0.0f );

   update();
}

//------------------------------------------------------------------------------
//!
Vec2f
TQuad::position() const
{
   return Vec2f( _mat(0), _mat(1) );
}

//------------------------------------------------------------------------------
//!
void
TQuad::position( const Vec2f& pos )
{
   _mat = Mat4f::translation( pos.x, pos.y, 0.0f );
}

//------------------------------------------------------------------------------
//!
void
TQuad::image( Image* img )
{
   // Replace only if different.
   if( img != _img )
   {
      _img = img;
      // Do not set the size (let the user handle this in Lua).
      // Set the samplers (allocate list if init didn't specify an image).
      if( _samplers.isValid() )
      {
         _samplers->samplers().clear();
      }
      else
      {
         _samplers = new Gfx::SamplerList();
      }
      _samplers->addSampler( "colorTex", _img->texture(), _texState );
      update();
   }
}

//------------------------------------------------------------------------------
//!
void
TQuad::update()
{
   if( _img.isNull() )  return;

   Gfx::Texture* tex = _img->texture();

   int xdim    = tex->definedWidth();
   int ydim    = tex->definedHeight();
   float txdim = (float)tex->width();
   float tydim = (float)tex->height();

   Vec4i ui( _u * (float)xdim );
   Vec4f uf( Vec4f( ui ) / txdim );
   int du0 = ui(1) - ui(0);
   int du1 = ui(3) - ui(2);
   int duc = ui(2) - ui(1);

   Vec4i vi( _v * (float)ydim );
   Vec4f vf( Vec4f( vi ) / tydim );
   int dv0 = vi(1) - vi(0);
   int dv1 = vi(3) - vi(2);
   int dvc = vi(2) - vi(1);

   ushort* indices;

   float off = Core::gfx()->oneToOneOffset();

   switch( _type )
   {
      case NORMAL:
      default:
      {
         Vec2f xf( 0.0f, _size.x );
         Vec2f yf( 0.0f, _size.y );

         xf += off;
         yf += off;

         _vdata[0] = Vec4f( xf(0), yf(0), uf(0), vf(0) );
         _vdata[1] = Vec4f( xf(1), yf(0), uf(1), vf(0) );
         _vdata[2] = Vec4f( xf(0), yf(1), uf(0), vf(1) );
         _vdata[3] = Vec4f( xf(1), yf(1), uf(1), vf(1) );
         indices = _verIndices;
      } break;

      case HORIZONTAL:
      {
         int maxdu0 = (du0 * (int)_size.x) / (du0 + du1);
         int maxdu1 = (int)_size.x - maxdu0;
         CGM::clampMax( du0, maxdu0 );
         CGM::clampMax( du1, maxdu1 );

         Vec4f xf( 0.0f, (float)du0, _size.x - float(du1), _size.x );
         Vec2f yf( 0.0f, _size.y );

         xf += off;
         yf += off;

         _vdata[0] = Vec4f( xf(0), yf(0), uf(0), vf(0) );
         _vdata[1] = Vec4f( xf(1), yf(0), uf(1), vf(0) );
         _vdata[2] = Vec4f( xf(2), yf(0), uf(2), vf(0) );
         _vdata[3] = Vec4f( xf(3), yf(0), uf(3), vf(0) );
         _vdata[4] = Vec4f( xf(0), yf(1), uf(0), vf(1) );
         _vdata[5] = Vec4f( xf(1), yf(1), uf(1), vf(1) );
         _vdata[6] = Vec4f( xf(2), yf(1), uf(2), vf(1) );
         _vdata[7] = Vec4f( xf(3), yf(1), uf(3), vf(1) );
         indices = _horIndices;
      } break;

      case VERTICAL:
      {
         int maxdv0 = (dv0 * (int)_size.y) / (dv0 + dv1);
         int maxdv1 = (int)_size.y - maxdv0;
         CGM::clampMax( dv0, maxdv0 );
         CGM::clampMax( dv1, maxdv1 );

         Vec2f xf( 0.0f, _size.x );
         Vec4f yf( 0.0f, (float)dv0, _size.y - (float)(dv1), _size.y );

         xf += off;
         yf += off;

         _vdata[0] = Vec4f( xf(0), yf(0), uf(0), vf(0) );
         _vdata[1] = Vec4f( xf(1), yf(0), uf(1), vf(0) );
         _vdata[2] = Vec4f( xf(0), yf(1), uf(0), vf(1) );
         _vdata[3] = Vec4f( xf(1), yf(1), uf(1), vf(1) );
         _vdata[4] = Vec4f( xf(0), yf(2), uf(0), vf(2) );
         _vdata[5] = Vec4f( xf(1), yf(2), uf(1), vf(2) );
         _vdata[6] = Vec4f( xf(0), yf(3), uf(0), vf(3) );
         _vdata[7] = Vec4f( xf(1), yf(3), uf(1), vf(3) );
         indices = _verIndices;
      } break;

      case GRID:
      case CONTOUR:
      {
         int maxdu0 = (du0 * (int)_size.x) / (du0 + du1);
         int maxdu1 = (int)_size.x - maxdu0;
         CGM::clampMax( du0, maxdu0 );
         CGM::clampMax( du1, maxdu1 );

         int maxdv0 = (dv0 * (int)_size.y) / (dv0 + dv1);
         int maxdv1 = (int)_size.y - maxdv0;
         CGM::clampMax( dv0, maxdv0 );
         CGM::clampMax( dv1, maxdv1 );

         Vec4f xf( 0.0f, (float)du0, _size.x-(float)(du1), _size.x );
         Vec4f yf( 0.0f, (float)dv0, _size.y-(float)(dv1), _size.y );

         xf += off;
         yf += off;

         _vdata[0]  = Vec4f( xf(0), yf(0), uf(0), vf(0) );
         _vdata[1]  = Vec4f( xf(1), yf(0), uf(1), vf(0) );
         _vdata[2]  = Vec4f( xf(2), yf(0), uf(2), vf(0) );
         _vdata[3]  = Vec4f( xf(3), yf(0), uf(3), vf(0) );
         _vdata[4]  = Vec4f( xf(0), yf(1), uf(0), vf(1) );
         _vdata[5]  = Vec4f( xf(1), yf(1), uf(1), vf(1) );
         _vdata[6]  = Vec4f( xf(2), yf(1), uf(2), vf(1) );
         _vdata[7]  = Vec4f( xf(3), yf(1), uf(3), vf(1) );
         _vdata[8]  = Vec4f( xf(0), yf(2), uf(0), vf(2) );
         _vdata[9]  = Vec4f( xf(1), yf(2), uf(1), vf(2) );
         _vdata[10] = Vec4f( xf(2), yf(2), uf(2), vf(2) );
         _vdata[11] = Vec4f( xf(3), yf(2), uf(3), vf(2) );
         _vdata[12] = Vec4f( xf(0), yf(3), uf(0), vf(3) );
         _vdata[13] = Vec4f( xf(1), yf(3), uf(1), vf(3) );
         _vdata[14] = Vec4f( xf(2), yf(3), uf(2), vf(3) );
         _vdata[15] = Vec4f( xf(3), yf(3), uf(3), vf(3) );
         indices = _type == GRID ? _horIndices : _contourIndices;
      } break;

      case ADJUST_BORDERS:
      {
         du0 = du0 * ((int)_size.x-duc ) / (du0+du1);
         du1 = (int)_size.x-duc-du0;

         CGM::clampMin( du0, 0 );
         CGM::clampMin( du1, 0 );

         dv0 = dv0 * ((int)_size.y-dvc ) / (dv0+dv1);
         dv1 = (int)_size.y-dvc-dv0;

         CGM::clampMin( dv0, 0 );
         CGM::clampMin( dv1, 0 );

         Vec4f xf( 0.0f, (float)du0, _size.x-(float)(du1), _size.x );
         Vec4f yf( 0.0f, (float)dv0, _size.y-(float)(dv1), _size.y );

         xf += off;
         yf += off;

         _vdata[0]  = Vec4f( xf(0), yf(0), uf(0), vf(0) );
         _vdata[1]  = Vec4f( xf(1), yf(0), uf(1), vf(0) );
         _vdata[2]  = Vec4f( xf(2), yf(0), uf(2), vf(0) );
         _vdata[3]  = Vec4f( xf(3), yf(0), uf(3), vf(0) );
         _vdata[4]  = Vec4f( xf(0), yf(1), uf(0), vf(1) );
         _vdata[5]  = Vec4f( xf(1), yf(1), uf(1), vf(1) );
         _vdata[6]  = Vec4f( xf(2), yf(1), uf(2), vf(1) );
         _vdata[7]  = Vec4f( xf(3), yf(1), uf(3), vf(1) );
         _vdata[8]  = Vec4f( xf(0), yf(2), uf(0), vf(2) );
         _vdata[9]  = Vec4f( xf(1), yf(2), uf(1), vf(2) );
         _vdata[10] = Vec4f( xf(2), yf(2), uf(2), vf(2) );
         _vdata[11] = Vec4f( xf(3), yf(2), uf(3), vf(2) );
         _vdata[12] = Vec4f( xf(0), yf(3), uf(0), vf(3) );
         _vdata[13] = Vec4f( xf(1), yf(3), uf(1), vf(3) );
         _vdata[14] = Vec4f( xf(2), yf(3), uf(2), vf(3) );
         _vdata[15] = Vec4f( xf(3), yf(3), uf(3), vf(3) );
         indices = _horIndices;
      } break;

      case CENTERED:
      {
         Vec2f xf( (_size.x - (float)(xdim))/2.0f, (_size.x + (float)(xdim))/2.0f );
         Vec2f yf( (_size.y - (float)(ydim))/2.0f, (_size.y + (float)(ydim))/2.0f );

         xf += off;
         yf += off;

         // Vertices
         _vdata[0] = Vec4f( xf(0), yf(0), uf(0), vf(0) );
         _vdata[1] = Vec4f( xf(1), yf(0), uf(1), vf(0) );
         _vdata[2] = Vec4f( xf(0), yf(1), uf(0), vf(1) );
         _vdata[3] = Vec4f( xf(1), yf(1), uf(1), vf(1) );
         indices = _verIndices;
      } break;
   }

   RCP<Gfx::IndexBuffer> indexBuffer;
   RCP<Gfx::VertexBuffer> vertexBuffer;

   // Create buffers if not done yet.
   if( _geom->numBuffers() > 0 )
   {
      indexBuffer  = _geom->indexBuffer();
      vertexBuffer = _geom->buffers()[0];
   }
   else
   {
      indexBuffer  = Core::gfx()->createBuffer( Gfx::INDEX_FMT_16, Gfx::BUFFER_FLAGS_NONE, 0, 0 );
      vertexBuffer = Core::gfx()->createBuffer( Gfx::BUFFER_FLAGS_NONE, 0, 0 );
      vertexBuffer->addAttribute( Gfx::ATTRIB_TYPE_POSITION, Gfx::ATTRIB_FMT_32F_32F, 0 );
      vertexBuffer->addAttribute( Gfx::ATTRIB_TYPE_TEXCOORD0, Gfx::ATTRIB_FMT_32F_32F, 8 );

      _geom->indexBuffer( indexBuffer );
      _geom->addBuffer( vertexBuffer );
   }

   // Fill buffer.
   Core::gfx()->setData( indexBuffer, (size_t)_indicesSize[_type], indices );
   Core::gfx()->setData( vertexBuffer, (size_t)_verticesSize[_type], _vdata.data() );
}

//------------------------------------------------------------------------------
//!
void
TQuad::draw( const RCP<Gfx::RenderNode>& rn ) const
{
   if( _img.isNull() )  return;

   Gfx::Pass& pass = *(rn->current());
   pass.setWorldMatrixPtr( _mat.ptr() );
   pass.setConstants( _cl );
   pass.setSamplers( _samplers );
   pass.execGeometry( _geom );
}

//------------------------------------------------------------------------------
//!
const char*
TQuad::meta() const
{
   return _tquad_str_;
}

//------------------------------------------------------------------------------
//!
void
TQuad::init( VMState* vm )
{
   VM::get( vm, 1, "color", _color );
   VM::get( vm, 1, "u", _u );
   VM::get( vm, 1, "v", _v );
   VM::get( vm, 1, "type", _type );

   // Image.
   if( VM::get( vm, 1, "image" ) )
   {
      RCP<Image> img;
      if( VM::isString( vm, -1 ) )
      {
         String imgID = VM::toString( vm, -1 );
         img = data( ResManager::getImage( imgID ) );
      }
      else
      {
         img = (Image*)VM::toPtr( vm, -1 );
      }
      VM::pop( vm );

      image( img.ptr() );

      // Set the default size to the texture size.
      if( img.isValid() )
      {
         RCP<Gfx::Texture> tex = _img->texture();
         _size.x   = (float)tex->definedWidth();
         _size.y   = (float)tex->definedHeight();
      }
   }

   Vec2f pos( 0.0f, 0.0f );
   VM::get( vm, 1, "position", pos );
   VM::get( vm, 1, "size", _size );
   VM::setTop( vm, 0 );

   _constants->setConstant( "color", _color.ptr() );

   position( pos );
   update();
}

//------------------------------------------------------------------------------
//!
bool
TQuad::performGet( VMState* vm )
{
   switch( _attributes[ VM::toCString( vm, 2 ) ] )
   {
      case ATTRIB_COLOR:
         VM::push( vm, _color );
         return true;
      case ATTRIB_IMAGE:
         /// @todo - We need to finish this case.
         return true;
      case ATTRIB_TYPE:
         VM::push( vm, _type );
         return true;
      case ATTRIB_U:
         VM::push( vm, _u );
         return true;
      case ATTRIB_V:
         VM::push( vm, _v );
         return true;
      case ATTRIB_POSITION:
         {
            Vec2f pos( _mat(12), _mat(13) );
            VM::push( vm, pos );
         }
         return true;
      case ATTRIB_SIZE:
         VM::push( vm, _size );
         return true;
      case ATTRIB_IMAGESIZE:
         VM::push( vm, Vec2i( _img->texture()->definedWidth(), _img->texture()->definedHeight() ) );
         return true;
      default:
         break;
   }

   return Drawable::performGet( vm );
}

//------------------------------------------------------------------------------
//!
bool
TQuad::performSet( VMState* vm )
{
   switch( _attributes[ VM::toCString( vm, 2 ) ] )
   {
      case ATTRIB_COLOR:
         color( VM::toVec4f( vm, 3 ) );
         return true;
      case ATTRIB_IMAGE:
      {
         // Image.
         RCP<Image> img;
         if( VM::isString( vm, 3 ) )
         {
            String imgID = VM::toString( vm, 3 );
            // Search for image in manager.
            img = data( ResManager::getImage( imgID ) );
         }
         else
         {
            img = (Image*)VM::toPtr( vm, -1 );
         }
         image( img.ptr() );
      }  return true;
      case ATTRIB_TYPE:
         _type = VM::toInt( vm, 3 );
         update();
         return true;
      case ATTRIB_U:
         _u = VM::toVec4f( vm, 3 );
         update();
         return true;
      case ATTRIB_V:
         _v = VM::toVec4f( vm, 3 );
         update();
         return true;
      case ATTRIB_POSITION:
         position( VM::toVec2f( vm, 3 ) );
         return true;
      case ATTRIB_SIZE:
         size( VM::toVec2f( vm, 3 ) );
         return true;
      case ATTRIB_IMAGESIZE:
         // read-only
         return true;
      default:
         break;
   }

   return Drawable::performSet( vm );
}

NAMESPACE_END
