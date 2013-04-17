/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/Geometry/Material.h>

#include <Fusion/Core/Core.h>
#include <Fusion/Resource/ResManager.h>

/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN


ConstString _sampler_clamp_str;
ConstString _sampler_data_str;
ConstString _sampler_minFilter_str;
ConstString _sampler_magFilter_str;
ConstString _sampler_mipFilter_str;
ConstString _sampler_baseLevel_str;
ConstString _sampler_lastLevel_str;
ConstString _sampler_LODBias_str;

UNNAMESPACE_END

NAMESPACE_BEGIN

/*==============================================================================
   CLASS Material
==============================================================================*/

//------------------------------------------------------------------------------
//!
void
Material::initialize()
{
   _sampler_clamp_str     = "clamp";
   _sampler_data_str      = "data";
   _sampler_minFilter_str = "minFilter";
   _sampler_magFilter_str = "magFilter";
   _sampler_mipFilter_str = "mipFilter";
   _sampler_baseLevel_str = "baseLevel";
   _sampler_lastLevel_str = "lastLevel";
   _sampler_LODBias_str   = "LODBias";
}

//------------------------------------------------------------------------------
//!
void
Material::terminate()
{
   _sampler_clamp_str     = ConstString();
   _sampler_data_str      = ConstString();
   _sampler_minFilter_str = ConstString();
   _sampler_magFilter_str = ConstString();
   _sampler_mipFilter_str = ConstString();
   _sampler_baseLevel_str = ConstString();
   _sampler_lastLevel_str = ConstString();
   _sampler_LODBias_str   = ConstString();
}

//------------------------------------------------------------------------------
//!
Material*
Material::white()
{
   BaseMaterial* mat = new BaseMaterial();
   BaseMaterial::Layer l;
   l._color = data( ResManager::getImage( "image/white" ) );
   mat->addLayer(l);
   return mat;
}
//------------------------------------------------------------------------------
//!
Material::Material( int type ): _type( type )
{
}

//------------------------------------------------------------------------------
//!
Material::~Material()
{
}

/*==============================================================================
   CLASS BaseMaterial
==============================================================================*/

//------------------------------------------------------------------------------
//!
BaseMaterial::BaseMaterial():
   Material( Material::BASE ), _color( 1.0f ), _brdfID(0.5f/128.0f)
{
   _brdf = data( ResManager::getImage( "image/dbrdf" ) );
}

//------------------------------------------------------------------------------
//!
BaseMaterial::~BaseMaterial()
{
}

//------------------------------------------------------------------------------
//!
void
BaseMaterial::addLayer( const Layer& l )
{
   _layers.pushBack( l );
   _samplers = 0;
}

//------------------------------------------------------------------------------
//!
void
BaseMaterial::removeLayer( uint i )
{
   _layers.erase( _layers.begin() + i );
   _samplers = 0;
}

//------------------------------------------------------------------------------
//!
void
BaseMaterial::removeAllLayers()
{
   _layers.clear();
   _samplers = 0;
}
//------------------------------------------------------------------------------
//!
void
BaseMaterial::color( const Vec4f& color )
{
   _color = color;
   if( _constants.isValid() ) (*_constants)[0]->setConstant( "color", _color.ptr() );
}

//------------------------------------------------------------------------------
//!
void
BaseMaterial::brdf( float v )
{
   _brdfID = v;
   if( _constants.isValid() ) (*_constants)[0]->setConstant( "brdfID", &_brdfID );
}

//------------------------------------------------------------------------------
//!
void
BaseMaterial::updateSamplers()
{
   // Do we have samplers to update?
   if( _samplers.isNull() ) return;

   if( _samplers->size() != _layers.size() )
   {
      createSamplers();
      return;
   }

   Gfx::TextureState state;
   state.maxAniso(16);
   auto& samplers = _samplers->samplers();
   for( size_t i = 1; i < samplers.size(); ++i )
   {
      state.clamp( (Gfx::TexClamp)_layers[i]._clamp );
      samplers[i]->state( state );
      samplers[i]->texture( _layers[i]._color->texture() );
   }
}

//------------------------------------------------------------------------------
//!
void
BaseMaterial::createSamplers()
{
   Gfx::TextureState state;
   state.setBilinear();
   state.clamp( Gfx::TEX_CLAMP_LAST );
   _samplers = new Gfx::SamplerList();
   _samplers->reserve( uint(_layers.size()) + 1 );
   _samplers->addSampler( "brdfTex", _brdf->texture(), state );
   state.maxAniso(16);
   for( uint i = 0; i < _layers.size(); ++i )
   {
      state.clamp( (Gfx::TexClamp)_layers[i]._clamp );
      _samplers->addSampler(
         (String( "colorTex" ) + String(i)).cstr(),
         _layers[i]._color->texture(),
         state
      );
   }
}

//------------------------------------------------------------------------------
//!
void
BaseMaterial::createConstants()
{
   RCP<Gfx::ConstantBuffer> constant = Core::gfx()->createConstants( 5*sizeof(float) );
   constant->addConstant( "color", Gfx::CONST_FLOAT4, 0, _color.ptr() );
   constant->addConstant( "brdfID", Gfx::CONST_FLOAT, 4*sizeof(float), &_brdfID );
   _constants = Gfx::ConstantList::create( constant );
}

/*==============================================================================
   CLASS ReflectivePlanarMaterial
==============================================================================*/

//------------------------------------------------------------------------------
//!
ReflectivePlanarMaterial::ReflectivePlanarMaterial():
   _pos( 0.0f ), _normal( 0.0f, 1.0f, 0.0f )
{
   _type = REFLECTIVE_PLANAR;
}

//------------------------------------------------------------------------------
//!
void
ReflectivePlanarMaterial::position( const Vec3f& p )
{
   _pos = p;
}

//------------------------------------------------------------------------------
//!
void
ReflectivePlanarMaterial::normal( const Vec3f& n )
{
   _normal = n;
}

/*==============================================================================
   CLASS CustomMaterial
==============================================================================*/

//------------------------------------------------------------------------------
//!
CustomMaterial::CustomMaterial():
   Material( CUSTOM )
{
   _variants = new Table();
}

//------------------------------------------------------------------------------
//!
void
CustomMaterial::updateSamplers()
{
   // Do we have samplers to update?
   if( _samplers.isNull() ) return;

   // TODO: try to optimized this code path.
   createSamplers();
}

//------------------------------------------------------------------------------
//!
void
CustomMaterial::createSamplers()
{
   guaranteeProgram();
   _samplers = new Gfx::SamplerList();
   //_samplers->reserve(n);
   for( auto cur = _variants->begin(); cur != _variants->end(); ++cur )
   {
      const ConstString& key = cur->first;
      const     Variant& var = cur->second;
      if( var.isTable() )
      {
         uint imgIdx = 0;
         Gfx::TextureState state;
         const Table& samplerTable = *(var.getTable());
         for( auto scur = samplerTable.begin(); scur != samplerTable.end(); ++scur )
         {
            const ConstString& skey = scur->first;
            const     Variant& svar = scur->second;

            if( skey == _sampler_clamp_str )
               state.clamp( (Gfx::TexClamp)uint32_t(svar.getFloat()) );
            else
            if( skey == _sampler_minFilter_str )
               state.minFilter( (Gfx::TexFilter)uint32_t(svar.getFloat()) );
            else
            if( skey == _sampler_magFilter_str )
               state.magFilter( (Gfx::TexFilter)uint32_t(svar.getFloat()) );
            else
            if( skey == _sampler_mipFilter_str )
               state.mipFilter( (Gfx::TexFilter)uint32_t(svar.getFloat()) );
            else
            if( skey == _sampler_baseLevel_str )
               state.baseLevel( uint32_t(svar.getFloat()) );
            else
            if( skey == _sampler_lastLevel_str )
               state.lastLevel( uint32_t(svar.getFloat()) );
            else
            if( skey == _sampler_LODBias_str )
               state.LODBias( svar.getFloat() );
            else
            if( skey == _sampler_data_str )
               imgIdx = (uint)svar.getFloat();
            else
              StdErr << "Unknown sampler key: " << skey << "." << nl;

         }
         const auto& img = _images[imgIdx];
         if( img.isValid() )  _samplers->addSampler( key, img->texture(), state );
      }
   }
}

//------------------------------------------------------------------------------
//!
void
CustomMaterial::createConstants()
{
   guaranteeProgram();

   // First pass to determine size.
   size_t constSize = 0;
   for( auto cur = _variants->begin(); cur != _variants->end(); ++cur )
   {
      //const ConstString& key = cur->first;
      const     Variant& var = cur->second;
      switch( var.type() )
      {
         case Variant::NIL   :
            StdErr << "Unsupported CustomMaterial variant type: nil." << nl;
            break;
         case Variant::BOOL  :
            StdErr << "Unsupported CustomMaterial variant type: bool." << nl;
            break;
         case Variant::FLOAT :
            constSize += sizeof( float );
            break;
         case Variant::VEC2  :
            constSize += 2*sizeof( float );
            break;
         case Variant::VEC3  :
            constSize += 3*sizeof( float );
            break;
         case Variant::VEC4  :
            constSize += 4*sizeof( float );
            break;
         case Variant::QUAT  :
            constSize += 4*sizeof( float );
            break;
         case Variant::STRING:
            StdErr << "Unsupported CustomMaterial variant type: string." << nl;
            break;
         case Variant::TABLE :
            // Sampler.
            break;
         default:
            StdErr << "Unknown CustomMaterial variant type: " << var.type() << "." << nl;
            break;
      }
   }

   // Second pass to fill in the data.
   RCP<Gfx::ConstantBuffer> constant = Core::gfx()->createConstants( constSize );
   constSize = 0;
   for( auto cur = _variants->begin(); cur != _variants->end(); ++cur )
   {
      const ConstString& key = cur->first;
      const     Variant& var = cur->second;
      switch( var.type() )
      {
         case Variant::NIL   :
            // Error message already printed.
            break;
         case Variant::BOOL  :
            // Error message already printed.
            break;
         case Variant::FLOAT :
         {
            float f = var.getFloat();
            constant->addConstant( key, Gfx::CONST_FLOAT, constSize, &f );
            constSize += sizeof( float );
         }  break;
         case Variant::VEC2  :
            constant->addConstant( key, Gfx::CONST_FLOAT2, constSize, var.getVec2().ptr() );
            constSize += 2*sizeof( float );
            break;
         case Variant::VEC3  :
            constant->addConstant( key, Gfx::CONST_FLOAT3, constSize, var.getVec3().ptr() );
            constSize += 3*sizeof( float );
            break;
         case Variant::VEC4  :
            constant->addConstant( key, Gfx::CONST_FLOAT4, constSize, var.getVec4().ptr() );
            constSize += 4*sizeof( float );
            break;
         case Variant::QUAT  :
            constant->addConstant( key, Gfx::CONST_FLOAT4, constSize, var.getQuat().ptr() );
            constSize += 4*sizeof( float );
            break;
         case Variant::STRING:
            // Error message already printed.
            break;
         case Variant::TABLE :
            // Sampler.
            break;
         default:
            // Error message already printed.
            break;
      }
   }

   // Register a single constant buffer.
   _constants = Gfx::ConstantList::create( constant );
}

//------------------------------------------------------------------------------
//!
void CustomMaterial::guaranteeProgram()
{
   if( _prog.isNull() )  _prog = data( ResManager::getProgram( _progName ) );
}

//------------------------------------------------------------------------------
//!
void
CustomMaterial::setConstant( const ConstString& name, const void* value )
{
   if( _constants.isNull() )
   {
      // TODO
   }
   else
   {
      (*_constants)[0]->setConstant( name, value );
   }

}

//------------------------------------------------------------------------------
//!
void
CustomMaterial::print( TextStream& os ) const
{
   os << "CustomMaterial{"
      << "prog=" << _progName
      << ", params=";
   _variants->print(os);
   os << ", " << _images.size() << " image(s)}";
}

NAMESPACE_END
