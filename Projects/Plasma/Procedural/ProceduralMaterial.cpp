/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/Procedural/ProceduralMaterial.h>
#include <Plasma/Geometry/MaterialSet.h>
#include <Plasma/Procedural/ProceduralContext.h>

#include <Fusion/VM/VMRegistry.h>
#include <Fusion/Resource/ResManager.h>
#include <Fusion/Resource/Image.h>

#include <CGMath/Variant.h>

/*==============================================================================
   UNNAME NAMESPACE
==============================================================================*/

UNNAMESPACE_BEGIN

ConstString  _kstr_data;

/*==============================================================================
   CLASS MaterialContext
==============================================================================*/

class MaterialContext:
   public ProceduralContext
{
public:

   MaterialContext():
      ProceduralContext( MATERIAL, nullptr ),
      _materialSet( nullptr )
   {}

   /*----- data members -----*/

   MaterialSet*         _materialSet;
   Vector< RCP<Image> > _images;
};

//------------------------------------------------------------------------------
//!
inline MaterialContext* getContext( VMState* vm )
{
   return (MaterialContext*)VM::userData(vm);
}

//------------------------------------------------------------------------------
//!
void initLayers( VMState* vm, BaseMaterial* mat )
{
   // Read material parameters.
   if( VM::get( vm, -1, "color" ) )
   {
      mat->color( VM::toVec4f( vm, -1 ) );
      VM::pop(vm);
   }

   // Read brdf parameters.
   if( VM::get( vm, -1, "brdf" ) )
   {
      mat->brdf( VM::toFloat( vm, -1 ) );
      VM::pop(vm);
   }

   // Read each layer.
   uint numLayers = VM::getTableSize( vm, 1 );
   for( uint i = 1; i <= numLayers; ++i  )
   {
      BaseMaterial::Layer layer;
      VM::geti( vm, 1, i );
      // Clamping.
      VM::get( vm, -1, "clamp", layer._clamp );

      // Color.
      if( VM::get( vm, -1, "color" ) )
      {
         if( VM::isObject( vm, -1 ) )
         {
            layer._color = (Image*)VM::toPtr( vm, -1 );
         }
         else
         {
            StdErr << "Image format not supported.\n";
         }
         VM::pop(vm);
      }

      // Displacement.
      // DBRDF.
      VM::pop(vm);
      mat->addLayer( layer );
   }
}

//------------------------------------------------------------------------------
//!
int baseMaterialVM( VMState* vm )
{
   MaterialContext* context = getContext(vm);
   BaseMaterial* mat        = new BaseMaterial();
   context->_materialSet->add( mat );

   initLayers( vm, mat );

   return 0;
}

//------------------------------------------------------------------------------
//!
int refPlanarMaterialVM( VMState* vm )
{
   MaterialContext* context      = getContext(vm);
   ReflectivePlanarMaterial* mat = new ReflectivePlanarMaterial();
   context->_materialSet->add( mat );

   initLayers( vm, mat );

   Vec3f pos;
   Vec3f normal;
   if( VM::get( vm, -1, "position", pos ) ) mat->position( pos );
   if( VM::get( vm, -1, "normal", normal ) ) mat->normal( normal );

   return 0;
}

//------------------------------------------------------------------------------
//! Parses a custom material specification.
//! Example:
//!    customMaterial{
//!       program = "someProg",
//!       {
//!          someSamplerName = {
//!             data  = image( "checker" ),
//!             clamp = TexClamp.LAST,
//!          },
//!          color    = vec4( 1, 0, 0, 1 ),
//!          position = vec3( 1, 2, 3 ),
//!       }
//!    }
int customMaterialVM( VMState* vm )
{
   MaterialContext* context = getContext(vm);
   CustomMaterial* mat      = new CustomMaterial();
   context->_materialSet->add( mat );

   if( !VM::isTable( vm, 1 ) )
   {
      StdErr << "customMaterialVM() - Did not receive parameters in a table." << nl;
      return 0;
   }

   String str;
   if( VM::get( vm, 1, "program", str ) )
   {
      mat->programName( str );
   }
   else
   {
      StdErr << "customMaterialVM() - Missing program name." << nl;
      return 0;
   }


   int n = VM::getTableSize( vm, 1 );
   if( n != 0 )
   {
      // Read the constant/sampler table.
      VM::geti( vm, 1, 1 );
#if 0
      // This won't work, because objects aren't stored in Table, and result in nil.
      VM::toTable( vm, -1, mat->variants() );
      mat->variants().print();
#else
      if( VM::isTable( vm, -1 ) )
      {
         VM::push( vm ); // Start iterating at index 0 (nil).
         while( VM::next( vm, -2 ) )
         {
            ConstString key = ConstString( VM::toCString( vm, -2 ) );
            // Samplers are tables with a 'data' key.
            if( VM::isTable( vm, -1 ) && VM::get( vm, -1, "data" ) )
            {
               Image* image = NULL;
               if( VM::isObject( vm, -1 ) )
               {
                  image = (Image*)VM::toPtr( vm, -1 );
               }
               else
               {
                  StdErr << "customMaterialVM() - Image format not supported: " << VM::type( vm, -1 ) << "." << nl;
               }
               VM::pop( vm );

               RCP<Table> samplerTable = new Table();
               // Read the sampler as a subtable.
               // Note: trying to create the SamplerList right away will fail, as the Image's
               // Gfx::Texture resource might not be allocated until the first rendering.
               VM::toTable( vm, -1, *samplerTable );
               // The Image is special; need to keep it alive, and store it as an index.
               uint imageIdx = mat->addImage( image );
               samplerTable->set( _kstr_data, (float)imageIdx );

               mat->variants().set( key, samplerTable.ptr() );
            }
            else
            {
               // Read as a Variant.
               VM::toVariant( vm, mat->variants() );
            }
            VM::pop( vm, 1 ); // Pop the value, keep the key.
         }
      }
      else
      {
         StdErr << "customMaterialVM() - Did not receive a table for the constants/samplers." << nl;
         CHECK( false );
      }
#endif
      VM::pop( vm );

      if( n > 1 )
      {
         StdErr << "customMaterialVM() - Too many indexed values specified: ignoring the last " << n-1 << " entries." << nl;
         CHECK( false );
      }
   }

   return 0;
}

//------------------------------------------------------------------------------
//!
int imageVM( VMState* vm )
{
   MaterialContext* context = getContext(vm);

   String id = VM::toString( vm, 1 );
   id = ResManager::expand( context->curDir(), id );
   Image* img = data( ResManager::getImage( id ) );

   // Keep image alive.
   context->_images.pushBack( img );

   VM::push( vm, img );
   return 1;
}

//------------------------------------------------------------------------------
//!
int imageCubeVM( VMState* vm )
{
   MaterialContext* context = getContext(vm);

   String id = VM::toString( vm, 1 );
   id = ResManager::expand( context->curDir(), id );
   Image* img = data( ResManager::getImageCube( id ) );

   // Keep image alive.
   context->_images.pushBack( img );

   VM::push( vm, img );
   return 1;
}

//------------------------------------------------------------------------------
//!
int imageGenVM( VMState* vm )
{
   MaterialContext* context = getContext(vm);

   Image* img = NULL;
   switch( VM::getTop( vm ) )
   {
      case 0:
         StdErr << "imageGen() - Missing arguments." << nl;
         return 0;
      case 1:
      {
         String name = VM::toString( vm, 1 );
         name = ResManager::expand( context->curDir(), name );
         img = data( ResManager::getImage( name ) );
      }  break;
      case 2:
      {
         String name = VM::toString( vm, 1 );
         name = ResManager::expand( context->curDir(), name );
         img = data( ResManager::getImage( name, VM::toVec2i(vm, 2) ) );
      }  break;
      case 3:
      {
         String      name = VM::toString( vm, 1 );
         Vec2i       size = VM::toVec2i( vm, 2 );
         RCP<Table> table = new Table();
         VM::toTable( vm, 3, *table );
         name = ResManager::expand( context->curDir(), name );
         img = data( ResManager::getImage( name, size, table.ptr() ) );
      }  break;
      default:
         StdErr << "imageGen() - Too many arguments." << nl;
         return 0;
   }

   // Keep image alive.
   context->_images.pushBack( img );

   VM::push( vm, (void*)img );
   return 1;
}

//------------------------------------------------------------------------------
//!
const VM::Reg funcs[] = {
   { "baseMaterial",             baseMaterialVM      },
   { "reflectivePlanarMaterial", refPlanarMaterialVM },
   { "customMaterial",           customMaterialVM    },
   { "image",                    imageVM             },
   { "imageCube",                imageCubeVM         },
   { "imageGen",                 imageGenVM          },
   { 0, 0 }
};

//------------------------------------------------------------------------------
//!
const VM::EnumReg texClampEnums[] = {
   { "WRAP",              Gfx::TEX_CLAMP_WRAP              },
   { "MIRROR",            Gfx::TEX_CLAMP_MIRROR            },
   { "LAST",              Gfx::TEX_CLAMP_LAST              },
   { "BORDER",            Gfx::TEX_CLAMP_BORDER            },
   { "MIRRORONCE_LAST",   Gfx::TEX_CLAMP_MIRRORONCE_LAST   },
   { "MIRRORONCE_BORDER", Gfx::TEX_CLAMP_MIRRORONCE_BORDER },
   { 0, 0 }
};

//------------------------------------------------------------------------------
//!
const VM::EnumReg texFilterEnums[] = {
   { "NONE"  , Gfx::TEX_FILTER_NONE   },
   { "MINMAG", Gfx::TEX_FILTER_MINMAG },
   { "POINT" , Gfx::TEX_FILTER_POINT  },
   { "LINEAR", Gfx::TEX_FILTER_LINEAR },
   { "CUBIC" , Gfx::TEX_FILTER_CUBIC  },
   { 0, 0 }
};

//------------------------------------------------------------------------------
//!
void initVM( VMState* vm, uint /*mask*/ )
{
   VM::registerFunctions( vm, "_G", funcs );
   VM::registerEnum( vm, "TexClamp" , texClampEnums  );
   VM::registerEnum( vm, "TexFilter", texFilterEnums );
}

UNNAMESPACE_END

NAMESPACE_BEGIN

/*==============================================================================
   CLASS ProceduralMaterial
==============================================================================*/

//------------------------------------------------------------------------------
//!
void
ProceduralMaterial::initialize()
{
   VMRegistry::add( initVM, VM_CAT_MAT );
   _kstr_data = "data";
}

//------------------------------------------------------------------------------
//!
void
ProceduralMaterial::terminate()
{
   _kstr_data = ConstString();
}

//------------------------------------------------------------------------------
//!
ProceduralMaterial::ProceduralMaterial(
   Resource<MaterialSet>* res,
   const String&          id,
   const String&          path,
   const Table*           params,
   const MaterialMap*     map
):
   _res( res ), _params( params ), _map( map ), _id( id ), _path( path )
{
}

//------------------------------------------------------------------------------
//!
void
ProceduralMaterial::execute()
{
   VMState* vm = VM::open( VM_CAT_MAT | VM_CAT_MATH, true );

   // Create working context.
   RCP<MaterialSet> materialSet = new MaterialSet();
   MaterialContext context;
   context._materialSet = materialSet.ptr();
   context.curDir( ResManager::dir(_id) );

   // Keep context pointer into vm.
   VM::userData( vm, &context );

   // Execute and close.
   if( _params.isValid() )
   {
      VM::push( vm, *_params );
      VM::doFile( vm, _path, 1, 0 );
   }
   else
   {
      VM::doFile( vm, _path, 0 );
   }

   VM::close( vm );

   if( _map.isValid() ) materialSet->remap( *_map );

   if( materialSet->numMaterials() == 0 )
   {
      materialSet->add( Material::white() );
   }

   _res->data( materialSet.ptr() );
}

NAMESPACE_END
