/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/Resource/ResExporter.h>

#include <Plasma/Animation/SkeletalAnimation.h>
#include <Plasma/Geometry/MetaGeometry.h>
#include <Plasma/Geometry/MeshGeometry.h>
#include <Plasma/Resource/ResManager.h>
#include <Plasma/Resource/Serializer.h>
#include <Plasma/World/Camera.h>
#include <Plasma/World/Light.h>
#include <Plasma/World/Probe.h>
#include <Plasma/World/RigidEntity.h>
#include <Plasma/World/SkeletalEntity.h>
#include <Plasma/World/ParticleEntity.h>
#include <Plasma/World/ProxyEntity.h>
#include <Plasma/World/World.h>

#include <Plasma/DataFlow/DFGraph.h>
#include <Plasma/DataFlow/DFStrokes.h>
#include <Plasma/DataFlow/DFAnimNodes.h>
#include <Plasma/DataFlow/DFGeomNodes.h>
#include <Plasma/DataFlow/DFImageNodes.h>
#include <Plasma/DataFlow/DFStrokesNodes.h>
#include <Plasma/DataFlow/DFWorldNodes.h>

#include <Fusion/Resource/BitmapManipulator.h>

#if MOTION_BULLET
#include <MotionBullet/Collision/BasicShapes.h>
#include <MotionBullet/Collision/CollisionGroup.h>
#else
#include <Motion/Collision/BasicShapes.h>
#include <Motion/Collision/CollisionGroup.h>
#endif

#include <Fusion/Fusion.h>
#include <Fusion/VM/VMRegistry.h>
#include <Fusion/VM/VMFmt.h>

#include <Base/IO/FileDevice.h>
#include <Base/IO/FileSystem.h>
#include <Base/IO/GZippedFileDevice.h>
#include <Base/Util/Application.h>
#include <Base/Util/Date.h>

#define USE_Z_UP 1

/*==============================================================================
  UNNAME NAMESPACE
==============================================================================*/

UNNAMESPACE_BEGIN

typedef Map<String, ResExporter::WorldHandler>  WorldHandlerContainer;
typedef Map<String, ResExporter::GeomHandler>   GeomHandlerContainer;
typedef Map<String, ResExporter::AnimHandler>   AnimHandlerContainer;

WorldHandlerContainer  _worldHandlers;
GeomHandlerContainer   _geomHandlers;
AnimHandlerContainer   _animHandlers;
String                 _worldDefaultExt = "dae";
String                 _geomDefaultExt  = "mesh";
String                 _animDefaultExt  = "anim";


#if 0
//------------------------------------------------------------------------------
//!
inline String toXml( const Vec2f& v )
{
   return String().format( "%g %g", v.x, v.y );
}
#endif

//------------------------------------------------------------------------------
//!
inline String toXml( const Vec3f& v )
{
   return String().format( "%g %g %g", v.x, v.y, v.z );
}

//------------------------------------------------------------------------------
//!
inline String toXml( const Vec4f& v )
{
   return String().format( "%g %g %g %g", v.x, v.y, v.z, v.w );
}

//------------------------------------------------------------------------------
//!
const String& xs_ID( const String& str )
{
   return str;
}

#if 0
//------------------------------------------------------------------------------
//! Converts the string into an xs::NCName, which basically is a "no-colon" string.
String  xs_NCName( const String& str )
{
#if 0
   String tmp;
   String::SizeType s = 0;
   String::SizeType c = s;
   c = str.find('/', s);
   while( c != String::npos )
   {
      tmp += str.sub( s, c-s );
      tmp += "__";
      s = c + 1;
      c = str.find('/', s);
   }
   if( s != c )  tmp += str.sub( s, c-s );
   return tmp;
#else
   String tmp;
   String::SizeType s = 0;
   String::SizeType c = s;
   c = str.find(':', s);
   while( c != String::npos )
   {
      tmp += str.sub( s, c-s );
      tmp += "__COLON__";
      s = c + 1;
      c = str.find(':', s);
   }
   if( s != c )  tmp += str.sub( s, c-s );
   return tmp;
#endif
}
#endif

//------------------------------------------------------------------------------
//!
String  xs_token( const String& str )
{
   const String set( "\n\t\r" );
   String tmp;
   String::SizeType s = 0;
   String::SizeType c = str.findFirstOf(set, s);
   while( c != String::npos )
   {
      tmp += str.sub( s, c-s );
      s = c + 1;
      c = str.findFirstOf(set, s);
   }
   if( s != c )  tmp += str.sub( s, c-s );
   // TODO: Remove double spaces.
   return tmp.eatWhites();
}

//------------------------------------------------------------------------------
//!
inline String cameraName( uint id )
{
   return String().format( "cam%d", id );
}

//------------------------------------------------------------------------------
//!
inline String geometryName( uint id )
{
   return String().format( "geom%d", id );
}

#if 0
//------------------------------------------------------------------------------
//!
inline String geometryID( const Geometry* geom )
{
   return String().format( "geom" FMT_VOID_PTR, (void*)geom );
}
#endif

//------------------------------------------------------------------------------
//!
inline String geometryName( const Geometry* geom )
{
   return ResManager::getGeometryName( geom );
}

//------------------------------------------------------------------------------
//!
inline String commonImageName( const Image* img )
{
   String tmp = ResManager::getImageName( img );
   bool oneByOne = (img->bitmap()->size() == img->bitmap()->pixelSize());
   if( oneByOne )
   {
      tmp += String().format( FMT_VOID_PTR, (void*)img );
   }
   return tmp;
}

//------------------------------------------------------------------------------
//!
inline String imageName( const Image* img )
{
   return String("img-") + commonImageName( img );
}

//------------------------------------------------------------------------------
//!
inline String imageID( const Image* img )
{
   return imageName( img ) + "-ID";
}

//------------------------------------------------------------------------------
//!
inline String lightName( uint id )
{
   return String().format( "light%d", id );
}

//------------------------------------------------------------------------------
//!
inline String materialID( const Material* mat )
{
   return String().format( "mat" FMT_VOID_PTR, (void*)mat ) + "-ID";
}

//------------------------------------------------------------------------------
//!
inline String materialName( const Material* mat )
{
   return String().format( "mat" FMT_VOID_PTR, (void*)mat );
}

//------------------------------------------------------------------------------
//!
inline String effectID( const Material* mat )
{
   return String().format( "fx" FMT_VOID_PTR, (void*)mat ) + "-ID";
}

//------------------------------------------------------------------------------
//!
inline String effectName( const Material* mat )
{
   return String().format( "fx" FMT_VOID_PTR, (void*)mat );
}

//------------------------------------------------------------------------------
//!
inline const char* clampToWrap( int clamp )
{
   switch( clamp )
   {
      case Gfx::TEX_CLAMP_WRAP             : return "WRAP"       ;
      case Gfx::TEX_CLAMP_MIRROR           : return "MIRROR"     ;
      case Gfx::TEX_CLAMP_LAST             : return "CLAMP"      ;
      case Gfx::TEX_CLAMP_BORDER           : return "BORDER"     ;
      case Gfx::TEX_CLAMP_MIRRORONCE_LAST  : return "MIRROR_ONCE";
      case Gfx::TEX_CLAMP_MIRRORONCE_BORDER: return "MIRROR_ONCE";
      default: CHECK(false); return "";
   }
}

//------------------------------------------------------------------------------
//!
template< typename T >
inline void  dumpMatrix( const Mat4<T>& mat, TextStream& os, StreamIndent& indent )
{
   os << indent << mat(0, 0) << " " << mat(0, 1) << " " << mat(0, 2) << " " << mat(0, 3) << nl;
   os << indent << mat(1, 0) << " " << mat(1, 1) << " " << mat(1, 2) << " " << mat(1, 3) << nl;
   os << indent << mat(2, 0) << " " << mat(2, 1) << " " << mat(2, 2) << " " << mat(2, 3) << nl;
   os << indent << mat(3, 0) << " " << mat(3, 1) << " " << mat(3, 2) << " " << mat(3, 3) << nl;
}

//------------------------------------------------------------------------------
//!
template< typename T >
inline Mat4f fixMatrix( const Ref<T>& ref )
{
#if USE_Z_UP
   Reff r = ref;
   r.rotate( Vec3f(0.0f), Vec3f(1.0f, 0.0f, 0.0f), CGConstf::pi_2() );
   return r.toMatrix();
#else
   return ref.toMatrix();
#endif
}

UNNAMESPACE_END

NAMESPACE_BEGIN

/*==============================================================================
   NAMESPACE ResExporter
==============================================================================*/

#if 0

//------------------------------------------------------------------------------
//!
void
ResExporter::saveMaterial( TextStream& os, const RCP<Material>& material, StreamIndent& indent )
{
   os << indent << "Plasma.material{\n";
   ++indent;

   // Save the program.
   os << indent << "program=" << luaFormat(ResManager::getProgramName(material->program())) << ",\n";

   // Save the textures.
   const RCP<Gfx::SamplerList>& sl = material->samplers();
   if( sl.isValid() )
   {
      if( sl->size() != 0 )
      {
         os << indent << "textures={\n";
         ++indent;
         Gfx::SamplerList::ContainerType::ConstIterator curSamp = sl->samplers().begin();
         Gfx::SamplerList::ContainerType::ConstIterator endSamp = sl->samplers().end();
         for( ; curSamp != endSamp; ++curSamp )
         {
            os << indent << "{" << luaFormat(ResManager::getTextureName((*curSamp)->texture()))
                         << "," << luaFormat((*curSamp)->name())
            // TODO: Dump TextureState as well.
                         << "},\n";
         }
         --indent;
         os << indent << "},\n";
      }
   }

   // Save all of the user-specified constants.
   const RCP<Gfx::ConstantBuffer>& matConst = material->constants();
   if( matConst.isValid() )
   {
      const Gfx::ConstantBuffer::Container& constants = matConst->constants();
      Gfx::ConstantBuffer::Container::ConstIterator curConst = constants.begin();
      Gfx::ConstantBuffer::Container::ConstIterator endConst = constants.end();
      for( ; curConst != endConst; ++curConst )
      {
         os << indent << (*curConst).name() << "=";
         float f[16];
         matConst->getConstant(&(*curConst), &f);
         switch( (*curConst).type() )
         {
            case Gfx::CONST_FLOAT:
               os << f[0];
               break;
            case Gfx::CONST_FLOAT2:
               os << luaFormat( Vec2f(f[0], f[1]) );
               break;
            case Gfx::CONST_FLOAT3:
               os << luaFormat( Vec3f(f[0], f[1], f[2]) );
               break;
            case Gfx::CONST_FLOAT4:
               os << luaFormat( Vec4f(f[0], f[1], f[2], f[3]) );
               break;
            case Gfx::CONST_MAT2:
               os << luaFormat( Mat2f( f[0], f[1],
                                       f[2], f[3] ) );
               break;
            case Gfx::CONST_MAT3:
               os << luaFormat( Mat3f( f[0], f[1], f[2],
                                       f[3], f[4], f[5],
                                       f[6], f[7], f[8] ) );
               break;
            case Gfx::CONST_MAT4:
               os << luaFormat( Mat4f( f[ 0], f[ 1], f[ 2], f[ 3],
                                       f[ 4], f[ 5], f[ 6], f[ 7],
                                       f[ 8], f[ 9], f[10], f[11],
                                       f[12], f[13], f[14], f[15] ) );
               break;
            default:
               CHECK(false);
               os << "nil";
               break;
         }
         os << ",\n";
      }
   }

   --indent;
   os << indent << "}"; //material
}

//------------------------------------------------------------------------------
//!
void
ResExporter::saveSkeleton(
   TextStream&          stream,
   const RCP<Skeleton>& skel,
   StreamIndent&        indent
)
{
   stream << indent << "Plasma.skeleton{\n";
   ++indent;

   for( uint i = 0; i < skel->numBones(); ++i )
   {
      stream << indent << "{\n";
      ++indent;
      stream << indent << "name="   << luaFormat( skel->bone(i).name() )        << ",\n";
      stream << indent << "parent=" << luaFormat( skel->bone(i).parent() )      << ",\n";
      stream << indent << "dof="    << luaFormat( skel->bone(i).dof() )         << ",\n";
      stream << indent << "e="      << luaFormat( skel->bone(i).endPoint() )    << ",\n";
      stream << indent << "p="      << luaFormat( skel->bone(i).position() )    << ",\n";
      stream << indent << "q="      << luaFormat( skel->bone(i).orientation() ) << ",\n";
      --indent;
      stream << indent << "},\n";
   }

   stream << indent << "limbs={";
   for( uint i = 0; i < skel->numLimbs(); ++i )
   {
      stream << skel->limb(i).boneID() << ",";
   }
   stream << "}\n";

   --indent;
   stream << indent << "}";
}

//------------------------------------------------------------------------------
//!
void
ResExporter::saveWorld( TextStream& stream, const RCP<World>& world, StreamIndent& indent )
{
#if FIXME
   // World.
   stream << "local world=Plasma.world{\n";
   ++indent;
   stream << indent << "backgroundColor=" << luaFormat( world->backgroundColor() ) << ",\n";
   stream << indent << "gravity="         << luaFormat( world->gravity() )         << ",\n";
   stream << indent << "simSpeed="        << luaFormat( world->simulationSpeed() ) << ",\n";
   --indent;
   stream << "}\n"; //world

   // Skeleton.
   Map<Skeleton*, uint> skeletons;
   stream << "local skeletons={\n";
   ++indent;
   for( uint i = 0, s = 0; i < world->numEntities(); ++i )
   {
      if( world->entities()[i]->type() == Entity::SKELETAL )
      {
         SkeletalEntity* ent = (SkeletalEntity*)world->entities()[i].ptr();
         if( ent->skeleton().isValid() && !skeletons.has( ent->skeleton().ptr() ) )
         {
            skeletons[ent->skeleton().ptr()] = s++;
            saveSkeleton( stream, ent->skeleton().ptr(), indent );
            stream << ",\n";
         }
      }
   }
   --indent;
   stream << "}\n"; // skeletons

   // Entities.
   stream << "local entities={\n";
   ++indent;

   for( uint i = 0; i < world->numEntities(); ++i )
   {
      Entity* ent = world->entities()[i].ptr();
      switch( ent->type() )
      {
         case Entity::STATIC:
         {
            StaticEntity* se = (StaticEntity*)ent;
            stream << indent  << "Plasma.staticEntity{\n";
            ++indent;
            stream << indent << "friction="   << luaFormat( se->friction() )    << ",\n";
            stream << indent << "resitution=" << luaFormat( se->restitution() ) << ",\n";
         } break;
         case Entity::DYNAMIC:
         {
            DynamicEntity* de = (DynamicEntity*)ent;
            stream << indent  << "Plasma.dynamicEntity{\n";
            ++indent;
            stream << indent << "mass="       << luaFormat( de->mass() )        << ",\n";
            stream << indent << "friction="   << luaFormat( de->friction() )    << ",\n";
            stream << indent << "resitution=" << luaFormat( de->restitution() ) << ",\n";
         } break;
         case Entity::SKELETAL:
         {
            SkeletalEntity* se = (SkeletalEntity*)ent;
            stream << indent << "Plasma.skeletalEntity{\n";
            ++indent;
            stream << indent << "mass="       << luaFormat( se->mass() )        << ",\n";
            stream << indent << "friction="   << luaFormat( se->friction() )    << ",\n";
            stream << indent << "resitution=" << luaFormat( se->restitution() ) << ",\n";

            // Skeleton.
            if( skeletons.has( se->skeleton().ptr() ) )
            {
               stream << indent << "skeleton=skeletons[" << skeletons[se->skeleton().ptr()]+1 << "],\n";
            }
         } break;
      }

      stream << indent << "position="             << luaFormat( ent->position() )             << ",\n";
      stream << indent << "orientation="          << luaFormat( ent->orientation() )          << ",\n";
      stream << indent << "materials="            << luaFormat( ent->materialGroup() )        << ",\n";
      stream << indent << "attractionCategories=" << luaFormat( ent->attractionCategories() ) << ",\n";
      stream << indent << "collisionCategories="  << luaFormat( ent->collisionCategories() )  << ",\n";
      stream << indent << "collisionMask="        << luaFormat( ent->collisionMask() )        << ",\n";
      stream << indent << "callbackMask="         << luaFormat( ent->callbackMask() )         << ",\n";

      --indent;
      stream << indent  << "},\n"; //{static,dynamic,skeletal}Entity
   }

   --indent;
   stream << "}\n"; //entities
   stream << "world:addEntities( entities )\n";

   // Materials.
   // 1. Create materials table (superset of all materials).
   Map<Material*, uint> materials;
   stream << "local mats={\n";
   ++indent;
   World::MaterialContainer::ConstIterator curMatGrp = world->materialGroups().begin();
   World::MaterialContainer::ConstIterator endMatGrp = world->materialGroups().end();
   uint numMats = 0;
   for( ; curMatGrp != endMatGrp; ++curMatGrp )
   {
      const MaterialGroup* matGrp = (*curMatGrp).second.ptr();
      uint n = matGrp->size();
      for( uint i = 0; i < n; ++i )
      {
         const RCP<Material>& mat = (*matGrp)[i];
         if( mat.isValid() && !materials.has(mat.ptr()) )
         {
            ++numMats;
            materials[mat.ptr()] = numMats;
            saveMaterial( stream, mat, indent );
            stream << ",\n";
         }
      }
   }
   --indent;
   stream << "}\n"; //materials
   // 2. Add all material groups.
   for( curMatGrp = world->materialGroups().begin(); curMatGrp != endMatGrp; ++curMatGrp )
   {
      const MaterialGroup* matGrp = (*curMatGrp).second.ptr();
      stream << indent << "world:addMaterials(" << luaFormat((*curMatGrp).first) << ",{";
      uint n = matGrp->size();
      for( uint i = 0; i < n; ++i )
      {
         Material* mat = (*matGrp)[i].ptr();
         if( i != 0 ) stream << ",";
         stream << "mats[" << materials[mat] << "]";
      }
      stream << "}";
      stream << ")\n"; //world:addMaterials
   }

   // Result.
   stream << "return world\n";
#endif
}
#endif

/*==============================================================================
   ResExporter
==============================================================================*/

namespace ResExporter
{

//------------------------------------------------------------------------------
//!
void  initialize()
{
   addSupport( "world" , WorldHandler(Lua::save) );
   addSupport( "dae"   , WorldHandler(Collada::save) );

   addSupport( "mesh"  , GeomHandler(Lua::save) );
   addSupport( "bin"   , GeomHandler(Bin::save) );
   addSupport( "bin.gz", GeomHandler(Bin::saveCompressed) );
   addSupport( "obj"   , GeomHandler(Obj::save) );

   addSupport( "anim"  , AnimHandler(Lua::save) );
   //addSupport( "bin"   , AnimHandler(Bin::save) );
   //addSupport( "bin.gz", AnimHandler(Bin::saveCompressed) );
}

//------------------------------------------------------------------------------
//!
template<>
const String& defaultExtension<World>()
{
   return _worldDefaultExt;
}

//------------------------------------------------------------------------------
//!
template<>
const String& defaultExtension<Geometry>()
{
   return _geomDefaultExt;
}

//------------------------------------------------------------------------------
//!
template<>
const String& defaultExtension<SkeletalAnimation>()
{
   return _animDefaultExt;
}

//------------------------------------------------------------------------------
//!
void addSupport( const String& ext, const WorldHandler& cb )
{
   _worldHandlers[ext] = cb;
}

//------------------------------------------------------------------------------
//!
void addSupport( const String& ext, const GeomHandler& cb )
{
   _geomHandlers[ext] = cb;
}

//------------------------------------------------------------------------------
//!
void addSupport( const String& ext, const AnimHandler& cb )
{
   _animHandlers[ext] = cb;
}

//------------------------------------------------------------------------------
//!
bool save( const World& world, const Path& path )
{
   String ext = path.getLongExt().lower();
   if( ext.empty() )
   {
      const String& defExt = defaultExtension<World>();
      if( !defExt.empty() )
      {
         Path pathWithExt = path;
         pathWithExt     += ".";
         pathWithExt     += defExt;
         return save( world, pathWithExt );
      }
   }

   for( ; !ext.empty(); ext = Path::nextExt(ext) )
   {
      auto it = _worldHandlers.find( ext );
      if( it != _worldHandlers.end() )
      {
         auto& handler = it->second;
         FS::createDirectories( path.dirname() );
         return handler( world, path );
      }
   }

   StdErr << "ERROR - Failed to find handler when saving world (ext='" << ext << "')." << nl;
   return false;
}

//------------------------------------------------------------------------------
//!
bool save( const Geometry& geom, const Path& path )
{
   String ext = path.getLongExt().lower();
   if( ext.empty() )
   {
      const String& defExt = defaultExtension<Geometry>();
      if( !defExt.empty() )
      {
         Path pathWithExt = path;
         pathWithExt     += ".";
         pathWithExt     += defExt;
         return save( geom, pathWithExt );
      }
   }

   for( ; !ext.empty(); ext = Path::nextExt(ext) )
   {
      auto it = _geomHandlers.find( ext );
      if( it != _geomHandlers.end() )
      {
         auto& handler = it->second;
         FS::createDirectories( path.dirname() );
         return handler( geom, path );
      }
   }

   StdErr << "ERROR - Failed to find handler when saving geometry (ext='" << ext << "')." << nl;
   return false;
}

//------------------------------------------------------------------------------
//!
bool save( const SkeletalAnimation& src, const Path& path )
{
   String ext = path.getLongExt().lower();
   if( ext.empty() )
   {
      const String& defExt = defaultExtension<SkeletalAnimation>();
      if( !defExt.empty() )
      {
         Path pathWithExt = path;
         pathWithExt     += ".";
         pathWithExt     += defExt;
         return save( src, pathWithExt );
      }
   }

   for( ; !ext.empty(); ext = Path::nextExt(ext) )
   {
      auto it = _animHandlers.find( ext );
      if( it != _animHandlers.end() )
      {
         auto& handler = it->second;
         FS::createDirectories( path.dirname() );
         return handler( src, path );
      }
   }

   StdErr << "ERROR - Failed to find handler when saving skeletal animation (ext='" << ext << "')." << nl;
   return false;
}

//------------------------------------------------------------------------------
//!
bool save( DFGraph& graph, const Path& path, const Table* params )
{
   DFNode* out = graph.output();
   if( !out ) return false;

   // What type of graph are we saving?
   switch( out->output()->type() )
   {
      case DFSocket::GEOMETRY:
      {
         DFGeomOutput* geomOutput = (DFGeomOutput*)out->output();
         RCP<DFGeometry> geom     = geomOutput->getGeometry();
         if( geom.isNull() ) return false;
         RCP<MeshGeometry> mesh   = geom->createMesh();
         save( *mesh, path );
      }  break;
      case DFSocket::ANIMATION:
      {
         DFAnimOutput*    animOutput = (DFAnimOutput*)out->output();
         RCP<SkeletalAnimation> anim = animOutput->getAnimation();
         if( anim.isNull() ) return false;
         return save( *anim, path );
      }  break;
      case DFSocket::IMAGE:
      {
         Vec2f rpos(0.0f);
         Vec2f rsize(1.0f);
         Vec2i bsize(256);
         if( params )
         {
            const Variant& rp = params->get( "imgPosition" );
            if( rp.isVec2() ) rpos = rp.getVec2();
            const Variant& rs = params->get( "imgSize" );
            if( rs.isVec2() ) rsize = rs.getVec2();
            const Variant& bs = params->get( "imgBufferSize" );
            if( bs.isVec2() ) bsize = bs.getVec2();
         }
         DFImageParams params( rpos, rsize, bsize );
         DFImageOutput* imageOutput = (DFImageOutput*)out->output();
         RCP<Bitmap> img            = imageOutput->getImage( params );
         if( img.isNull() ) return false;
         BitmapManipulator::unscaleColorByAlpha( *img );
         img->save( path.string() );
      }  break;
      case DFSocket::STROKES:
      {
         DFStrokesOutput* strokesOutput = (DFStrokesOutput*)out->output();
         RCP<DFStrokes>   strokes       = strokesOutput->getStrokes();
         if( strokes.isNull() ) return false;
         RCP<MeshGeometry> mesh         = strokes->geometry()->createMesh();
         save( *mesh, path );
      }  break;
      case DFSocket::WORLD:
      {
         DFWorldOutput* worldOutout = (DFWorldOutput*)out->output();
         RCP<DFWorld> dfw           = worldOutout->getWorld();
         if( dfw.isNull() ) return false;
         RCP<World> world           = dfw->createWorld();
         save( *world, path );
      }  break;
      default:
         StdErr << "Saving not implemented for this type: " << out->output()->typeAsStr() << nl;
         return false;
   }
   return false;
}

//=============================================================================
// Lua
//=============================================================================

bool Lua::save( const World& world, const Path& path )
{
   // Saving geometries.
   Path worldLocalPath = path.removeExt();
   String worldName    = path.basename().removeExt().string();

   for( uint i = 0, g=0; i < world.numEntities(); ++i )
   {
      Entity* e = world.entity(i);
      if( e->geometry() )
      {
         String geometryName = ResManager::getGeometryName( e->geometry() );
         // Do we need to save the geometry?
         if( geometryName.empty() )
         {
            // Create geometry path name.
            String name   = String( "geom" ) + String(g);
            Path geomName = worldLocalPath / (name + ".mesh");

            // Create path if necessary.
            if( g == 0 )
               if( !FS::Entry( worldLocalPath ).exists() ) FS::createDirectory( worldLocalPath );

            // Save geometry.
            save( *e->geometry(), geomName );

            // FIXME: Id name
            String id = String( "~/" ) + worldName + String( "/" ) + name;

            // Add geometry to resource manager to be retrieved by the world.
            ResManager::registerGeometry( id, e->geometry() );

            // Next geometry.
            ++g;
         }
      }
   }

   // Saving materials.
   for( uint i = 0, m = 0; i < world.numEntities(); ++i )
   {
      Entity* e = world.entity(i);
      if( e->materialSet() )
      {
         // Create materialSet path name.
         String name  = String( "matSet" ) + String(i);
         Path setName = worldLocalPath / (name + ".mat");

         // Create path if necessary.
         if( m == 0 )
            if( !FS::Entry( worldLocalPath ).exists() ) FS::createDirectory( worldLocalPath );

         // Save materialSet.
         save( *e->materialSet(), setName );

         ++m;
      }
   }


   // Saving probe textures.
   uint n = world.numProbes();
   for( uint i = 0; i < n; ++i )
   {
      Probe* p = world.probe(i);
      switch( p->type() )
      {
         case Probe::CUBEMAP:
         {
            CubemapProbe* cp = (CubemapProbe*)p;
            if( cp->image().isValid() )
            {
               String name = String( "probe" ) + String(i);
               Path   path = worldLocalPath / name;
               if( !FS::Entry( worldLocalPath ).exists() ) FS::createDirectory( worldLocalPath );
               if( !cp->image()->bitmap()->save( path.string() ) )
               {
                  StdErr << "ERROR - Could not save image '" << path.string() << "'." << nl;
               }
            }
            else
            {
               StdErr << "ERROR - Cubemap probe index " << i << " is missing image; not saving texture data." << nl;
            }
         }  break;
         default:
            CHECK( false );
            break;
      }
   }

   // Saving world.
   RCP<FileDevice> fd = new FileDevice( path, IODevice::MODE_WRITE|IODevice::MODE_STRICT );
   if( fd.isValid() && fd->ok() )
   {
      TextStream os( fd.ptr() );
      StreamIndent indent;
      return save( world, worldLocalPath, os, indent );
   }
   else
   {
      StdErr << "ERROR - Lua::save(World*, Path&) could not open file for writing: '" << path.string() << "'." << nl;
      return false;
   }
}

//------------------------------------------------------------------------------
//!
bool Lua::save( const World& world, const Path& localPath, TextStream& os, StreamIndent& indent )
{
   String worldName = localPath.basename().string();

   // Save world parameters.
   os << indent << "background = " << VMFmt( world.backgroundColor() ) << nl;
   os << indent << "gravity = "    << VMFmt( world.gravity() ) << nl;

   // Save entities.
   ++indent;
   for( uint i = 0; i < world.numEntities(); ++i )
   {
      Entity* e = world.entity(i);
      switch( e->type() )
      {
         case Entity::RIGID:
         {
            const char* types[] = { "object{", "staticObject{", "kinematicObject{" };
            RigidEntity* re = (RigidEntity*)e;
            os << types[re->bodyType()]      << nl;
            os << indent << "mass = "        << re->mass()            << "," << nl;
            os << indent << "friction = "    << re->friction()        << "," << nl;
            os << indent << "restitution = " << re->restitution()     << "," << nl;
            os << indent << "exists = "      << toHex( re->exists() ) << "," << nl;
            os << indent << "senses = "      << toHex( re->senses() ) << "," << nl;
            os << indent << "attractionCategories = " << toHex( re->attractionCategories() ) << "," << nl;
            //os << indent << "linearVelocity = "  << VMFmt( re->linearVelocity() )  << "," << nl;
            //os << indent << "angularVelocity = " << VMFmt( re->angularVelocity() ) << "," << nl;
         }  break;
         case Entity::CAMERA:
         {
            const char* projs[] = { "perspective", "ortho" };
            const char* modes[] = { "x", "y", "smallest", "largest" };
            Camera* c = (Camera*)e;
            os << "camera{" << nl;
            os << indent << "focal = "      << c->focalLength()       << "," << nl;
            os << indent << "fov = "        << c->fov()               << "," << nl;
            os << indent << "front = "      << c->front()             << "," << nl;
            os << indent << "back = "       << c->back()              << "," << nl;
            os << indent << "orthoScale = " << c->orthoScale()        << "," << nl;
            os << indent << "shear = "      << VMFmt( c->shear() )    << "," << nl;
            os << indent << "projection = " << projs[c->projection()] << "," << nl;
            os << indent << "fovMode = "    << modes[c->fovMode()]    << "," << nl;
         }  break;
         case Entity::LIGHT:
         {
            const char* shapes[] = { "directional", "geometry", "point", "spot" };
            Light* l = (Light*)e;
            os << "light{" << nl;
            os << indent << "fov = "        << l->fov()                << "," << nl;
            os << indent << "front = "      << l->front()              << "," << nl;
            os << indent << "back = "       << l->back()               << "," << nl;
            os << indent << "intensity = "  << VMFmt( l->intensity() ) << "," << nl;
            os << indent << "shape = "      << shapes[l->shape()]      << "," << nl;
         }  break;
         case Entity::SKELETAL:
         {
            SkeletalEntity* se = (SkeletalEntity*)e;
            os << "skeletalObject{" << nl;
            os << indent << "mass = "        << se->mass()            << "," << nl;
            os << indent << "friction = "    << se->friction()        << "," << nl;
            os << indent << "restitution = " << se->restitution()     << "," << nl;
            os << indent << "exists = "      << toHex( se->exists() ) << "," << nl;
            os << indent << "senses = "      << toHex( se->senses() ) << "," << nl;
            os << indent << "attractionCategories = " << toHex( se->attractionCategories() ) << "," << nl;

         }  break;
         case Entity::PROXY:
         {
            //ProxyEntity* pe = (ProxyEntity*)e;
            os << "proxy{" << nl;
            // TODO
         }  break;
         case Entity::PARTICLE:
         {
            //ParticleEntity* pe = (ParticleEntity*)e;
            os << "particleObject{" << nl;
            // TODO
         }  break;
         case Entity::FLUID:
         case Entity::SOFT:
            continue;
      }
      // Parameters common to all entities.
      os << indent << "position = "     << VMFmt( e->position() )     << "," << nl;
      os << indent << "orientation = "  << VMFmt( e->orientation() )  << "," << nl;
      os << indent << "scale = "        << e->scale()                 << "," << nl;
      os << indent << "visible = "      << VMFmt( e->visible() )      << "," << nl;
      os << indent << "castsShadows = " << VMFmt( e->castsShadows() ) << "," << nl;
      os << indent << "ghost = "        << VMFmt( e->ghost() )        << "," << nl;

      if( !e->id().isNull() )
         os << indent << "id = "        << VMFmt( e->id() )           << "," << nl;

      // Geometry.
      if( e->geometry() )
      {
         String geometryName = ResManager::getGeometryName( e->geometry() );
         if( !geometryName.empty() )
            os << indent << "geometry = geometry( " << VMFmt( geometryName ) << " )," << nl;
      }

      // Material.
      if( e->materialSet() )
      {
         String setName = String( "~/" ) + worldName + String( "/matSet" ) + String(i);
         os << indent << "material = material( " << VMFmt( setName ) << " )," << nl;
      }

      // Brain.
      Brain* brain = e->brain();
      if( brain )
      {
         String progName = ResManager::getBrainProgramName( brain->program() );
         if( !progName.empty() )
            os << indent << "brain = brain( " << VMFmt( progName ) << ", nil, true )," << nl;
      }

      // User attributes.
      if( e->attributes() )
         os << indent << "attributes = " << VMFmt( *e->attributes(), indent ) << "," << nl;

      os << "}" << nl;
   }
   --indent;

   // Save constraints.

   // Save probes.
   uint n = world.numProbes();
   for( uint i = 0; i < n; ++i )
   {
      Probe* p = world.probe(i);
      switch( p->type() )
      {
         case Probe::CUBEMAP:
         {
            CubemapProbe* cp = (CubemapProbe*)p;
            //String imgName = ResManager::getImageName( cp->image().ptr() );
            String imgName = String( "~/" ) + worldName + String( "/probe" ) + String(i);
            os << indent
               << "probe{ "
               << "type=" << VMFmt(toStr(p->type()));
            if( !p->id().empty() )
            {
               os << ", id=" << VMFmt(p->id());
            }
            os << ", position=" << VMFmt(p->position())
               << ", image=imageCube(" << VMFmt(imgName) << ")"
               << " }"
               << endl;
         }  break;
         default:
            CHECK( false );
            break;
      }
   }

   return true;
}

//------------------------------------------------------------------------------
//!
bool Lua::save( const Geometry& geom, const Path& path )
{
   RCP<FileDevice> fd = new FileDevice( path, IODevice::MODE_WRITE|IODevice::MODE_STRICT );
   if( fd.isValid() && fd->ok() )
   {
      TextStream os( fd.ptr() );
      StreamIndent indent;
      return save( geom, os, indent ) && save( geom.collisionType(), geom.collisionShape(), os, indent );
   }
   else
   {
      StdErr << "ERROR - Lua::save(Geometry&, Path&) could not open file for writing: '" << path.string() << "'." << nl;
      return false;
   }
}

//------------------------------------------------------------------------------
//!
bool Lua::save( const Geometry& geom, TextStream& os, StreamIndent& indent )
{
   // Retreived mesh geometry.
   RCP<MeshGeometry> mesh;

   switch( geom.type() )
   {
      case Geometry::MESH: mesh = geom.mesh(); break;
      case Geometry::METAGEOMETRY:
         mesh = new MeshGeometry();
         geom.metaGeometry()->createMesh( *mesh );
         break;
      case Geometry::DFGEOMETRY:
         mesh = geom.dfGeometry()->createMesh();
         break;
      default:
         return false;
   }

   // Save mesh.
   String meshStr;

   // 1. Indices.
   os << "local i = {\n";
   ++indent;
   switch( mesh->primitiveType() )
   {
      case MeshGeometry::POINTS:
         meshStr = "pointmesh";
         for( uint i = 0; i < mesh->numPrimitives(); ++i )
         {
            os << indent << mesh->indices()[i] + 1 << "," << nl;
         }
         break;
      case MeshGeometry::LINES:
         meshStr = "linemesh";
         for( uint i = 0; i < mesh->numPrimitives(); ++i )
         {
            os << indent << mesh->indices()[i*2]   + 1 << "," <<
                            mesh->indices()[i*2+1] + 1 << "," << nl;
         }
         break;
      case MeshGeometry::TRIANGLES:
         meshStr = "trimesh";
         for( uint i = 0; i < mesh->numPrimitives(); ++i )
         {
            os << indent << mesh->indices()[i*3]   + 1 << "," <<
                            mesh->indices()[i*3+1] + 1 << "," <<
                            mesh->indices()[i*3+2] + 1 << "," << nl;
         }
         break;
   }
   --indent;
   os << "}\n";

   // 2. Vertices.
   os << "local v = {\n";
   ++indent;
   os << indent << "format = { ";
   for( uint i = 0; i < mesh->numAttributes(); ++i )
   {
      os << "Attribute." << MeshGeometry::attrToStr( mesh->attributeType(i) ) << ", ";
   }
   os << "},\n";
   size_t numFloats = mesh->vertexStride();
   for( size_t i = 0; i < mesh->numVertices(); ++i )
   {
      os << indent;
      for( size_t j = 0; j < numFloats; ++j )
      {
         os << mesh->vertices()[i*numFloats+j] << ",";
      }
      os << "\n";
   }
   --indent;
   os << "}\n";

   // 3.Mesh.
   os << meshStr << "{ indices=i, vertices=v, ranges={";
   for( uint i = 0; i < mesh->numPatches() ; ++i )
   {
      os << mesh->patchInfo(i).rangeSize() << ",";
   }
   os << "}, materials={";
   for( uint i = 0; i < mesh->numPatches() ; ++i )
   {
      os << mesh->patchInfo(i).materialID() + 1 << ",";
   }
   os << "} }\n";

   return true;
}

//------------------------------------------------------------------------------
//!
bool Lua::save( const MaterialSet& set, const Path& path )
{
   RCP<FileDevice> fd = new FileDevice( path, IODevice::MODE_WRITE|IODevice::MODE_STRICT );
   if( fd.isValid() && fd->ok() )
   {
      TextStream os( fd.ptr() );
      StreamIndent indent;
      return save( set, os, indent );
   }
   else
   {
      StdErr << "ERROR - Lua::save(MaterialSet&, Path&) could not open file for writing: '" << path.string() << "'." << nl;
      return false;
   }
}

//------------------------------------------------------------------------------
//!
bool Lua::save( const MaterialSet& set, TextStream& os, StreamIndent& indent )
{
   for( uint i = 0; i < set.numMaterials(); ++i )
   {
      Material* mat = set.material(i);
      switch( mat->type() )
      {
         case Material::BASE:
         {
            BaseMaterial* bmat = (BaseMaterial*)mat;
            os << indent << "baseMaterial{" << nl;
            ++indent;
            os << indent << "color = " << VMFmt( bmat->color() ) << "," << nl;
            os << indent << "brdf = "  << bmat->brdf()           << "," << nl;
            for( uint l = 0; l < bmat->numLayers(); ++l )
            {
               os << indent << "{" << nl;
               ++indent;
               String imgName = ResManager::getImageName( bmat->layer(l)._color.ptr() );
               os << indent << "clamp = " << bmat->layer(l)._clamp   << ","    << nl;
               os << indent << "color = image( " << VMFmt( imgName ) << " )," << nl;
               --indent;
               os << indent << "}," << nl;
            }
            --indent;
            os << indent << "}" << nl;
         }  break;
         case Material::REFLECTIVE_PLANAR:
         {
            ReflectivePlanarMaterial* rmat = (ReflectivePlanarMaterial*)mat;
            os << indent << "reflectivePlanarMaterial{" << nl;
            ++indent;
            os << indent << "position = " << VMFmt( rmat->position() ) << "," << nl;
            os << indent << "normal = "   << VMFmt( rmat->normal() )   << "," << nl;
            os << indent << "color = "    << VMFmt( rmat->color() )    << "," << nl;
            os << indent << "brdf = "     << rmat->brdf()              << "," << nl;
            for( uint l = 0; l < rmat->numLayers(); ++l )
            {
               os << indent << "{" << nl;
               ++indent;
               String imgName = ResManager::getImageName( rmat->layer(l)._color.ptr() );
               os << indent << "clamp = " << rmat->layer(l)._clamp   << ","    << nl;
               os << indent << "color = image( " << VMFmt( imgName ) << " )," << nl;
               --indent;
               os << indent << "}," << nl;
            }
            --indent;
            os << indent << "}" << nl;
         }  break;
         case Material::CUSTOM:
         {
            CustomMaterial* cmat = (CustomMaterial*)mat;
            os << indent << "customMaterial{" << nl;
            ++indent;
            os << indent << "program = " << VMFmt( cmat->programName() ) << "," << nl;
            os << indent << "{" << nl;
            ++indent;
            for( auto cur = cmat->variants().begin(); cur != cmat->variants().end(); ++cur )
            {
               if( (*cur).second.isTable() )
               {
                  const Table& samplerTable = *(*cur).second.getTable();
                  os << indent << (*cur).first << " = {" << nl;
                  ++indent;
                  for( auto k = samplerTable.begin(); k != samplerTable.end(); ++k )
                  {
                     if( (*k).first == "data" )
                     {
                        uint imgIdx = (uint)(*k).second.getFloat();
                        Image* img  = cmat->image(imgIdx);
                        if( img )
                        {
                           os << indent << "data = ";
                           if( img->cubemap() )
                              os << "imageCube( ";
                           else
                              os << "image( ";
                           os << VMFmt( ResManager::getImageName( img ) ) << " )," << nl;
                        }
                        else
                        {
                           StdErr << "Lua::save() - Custom material has invalid image; ignoring." << nl;
                        }
                     }
                     else
                        os << indent << (*k).first << " = " << VMFmt( (*k).second, indent ) << "," << nl;
                  }
                  --indent;
                  os << indent << "}," << nl;
               }
               else
                  os << indent << (*cur).first << " = " << VMFmt( (*cur).second, indent ) << "," << nl;
            }
            --indent;
            os << indent << "}," << nl;
            --indent;
            os << indent << "}" << nl;
         }  break;
      }
   }
   return true;
}

//------------------------------------------------------------------------------
//!
bool
Lua::save(
   Geometry::CollisionType type,
   const CollisionShape*   shape,
   TextStream&             os,
   StreamIndent&           indent
)
{
   switch( type )
   {
      case Geometry::MANUAL:             return save( shape, nullptr, os, indent );
      case Geometry::AUTO_BOX:           os << indent << "collision( \"box\" )"     << nl; break;
      case Geometry::AUTO_SPHERE:        os << indent << "collision( \"sphere\" )"  << nl; break;
      case Geometry::AUTO_CONVEX_HULL:   os << indent << "collision( \"hull\" )"    << nl; break;
      case Geometry::AUTO_TRIANGLE_MESH: os << indent << "collision( \"trimesh\" )" << nl; break;
   }
   return true;
}

//------------------------------------------------------------------------------
//!
bool
Lua::save( const CollisionShape* shape, const Reff* ref, TextStream& os, StreamIndent& indent )
{
   if( !shape ) return true;

   if( !ref )
   {
      os << indent << "collision{\n";
      ++indent;
   }

   switch( shape->type() )
   {
      case CollisionShape::GROUP:
      {
         if( ref )
         {
            os << indent << "{\n";
            ++indent;
            os << indent << "referential=" << VMFmt( *ref ) << ",\n";
         }
         CollisionGroup* group = (CollisionGroup*)shape;
         for( uint i = 0; i < group->numShapes(); ++i )
         {
            Reff sref = group->referential(i);
            save( group->shape(i), &sref, os, indent );
         }
         if( ref )
         {
            --indent;
            os << indent << "},\n";
         }
      }  break;
      case CollisionShape::SPHERE:
      {
         SphereShape* sphere = (SphereShape*)shape;
         os << indent << "collisionSphere{ radius=" << sphere->radius();
         if( ref )
            os << ", referential=" << VMFmt( *ref ) << " },\n";
         else
            os << " }\n";
      }  break;
      case CollisionShape::BOX:
      {
         BoxShape* box = (BoxShape*)shape;
         os << indent << "collisionBox{ size=" << VMFmt( box->size() );
         if( ref )
            os << ", referential=" << VMFmt( *ref ) << " },\n";
         else
            os << " }\n";
      }  break;
      case CollisionShape::CYLINDER:
      {
         CylinderShape* cylinder = (CylinderShape*)shape;
         os << indent << "collisionCylinder{ radius=" << cylinder->radius() << ", height=" << cylinder->height();
         if( ref )
            os << ", referential=" << VMFmt( *ref ) << " },\n";
         else
            os << " }\n";
      }  break;
      case CollisionShape::CONE:
      {
         ConeShape* cone = (ConeShape*)shape;
         os << indent << "collisionCone{ radius=" << cone->radius() << ", height=" << cone->height();
         if( ref )
            os << ", referential=" << VMFmt( *ref ) << " },\n";
         else
            os << " }\n";
      }  break;
      case CollisionShape::CONVEXHULL:
         // TODO
         break;
      case CollisionShape::SPHEREHULL:
         // TODO
         break;
      case CollisionShape::TRIMESH:
         break;
   }

   if( !ref )
   {
      --indent;
      os << indent << "}\n";
   }

   return true;
}

//------------------------------------------------------------------------------
//!
bool Lua::save( const SkeletalAnimation& anim, const Path& path )
{
   RCP<FileDevice> fd = new FileDevice( path, IODevice::MODE_WRITE|IODevice::MODE_STRICT );
   if( fd.isValid() && fd->ok() )
   {
      TextStream os( fd.ptr() );
      StreamIndent indent;
      return save( anim, os, indent );
   }
   else
   {
      StdErr << "ERROR - Lua::save(SkeletalAnimation*, Path&) could not open file for writing: '" << path.string() << "'." << nl;
      return false;
   }
}

//------------------------------------------------------------------------------
//!
bool Lua::save( const SkeletalAnimation& anim, TextStream& os, StreamIndent& indent )
{
   bool ok = true;

   // Save mesh.
   String meshStr;

   os << indent << "animation{\n";
   ++indent;

   // 1. Parameters.
   os << indent << "rate="     << anim.rate()              << ",\n";
   //os << indent << "duration=" << anim->duration() << ",\n";
   os << indent << "type="     << anim.type()              << ",\n";
   if( anim.cyclic() )  os << indent << "cyclic=true,\n";
   os << indent << "offset="   << VMFmt( anim.offset() )   << ",\n";
   os << indent << "velocity=" << VMFmt( anim.velocity() ) << ",\n";

   // 2. Poses.
   os << indent << "poses={\n";
   ++indent;
   uint n = anim.numPoses();
   for( uint i = 0; i < n; ++i )
   {
      SkeletalPose* pose = anim.pose(i);
      ok &= Lua::save( *pose, os, indent );
   }
   --indent;
   os << indent << "},\n"; // poses={

   --indent;
   os << indent << "}\n"; // animation{

   return ok;
}

//------------------------------------------------------------------------------
//!
bool Lua::save( const SkeletalPose& pose, TextStream& os, StreamIndent& indent )
{
   os << indent << "{\n";
   ++indent;
   os << indent << "p=" << VMFmt( pose.position() )    << ",\n";
   os << indent << "q=" << VMFmt( pose.orientation() ) << ",\n";
   for( auto cur = pose.bones().begin(), end = pose.bones().end(); cur != end; ++cur )
   {
      os << indent << VMFmt( (*cur) ) << ",\n";
   }
   --indent;
   os << indent << "},\n";
   return true;
}


//=============================================================================
// Collada
//=============================================================================

namespace Collada
{

#define XML_BLOCK_OPEN( tag ) \
   _os << _indent << tag << nl; \
   ++_indent

#define XML_BLOCK_CLOSE( tag ) \
   --_indent; \
   _os << _indent << tag << nl

/*==============================================================================
  CLASS State
==============================================================================*/
class State
{
public:

   /*----- members -----*/

   State( const World& w, TextStream& os, StreamIndent& indent ):
      _world( &w ), _os( os ), _indent( indent )
   {}

   void  preprocess();

   void  save();
   void  saveAsset();
   // Libraries.
   void  saveCameras();
   void  saveCamera( const Camera* cam, uint idx );
   void  saveImages();
   void  saveImage( const Image* img );
   void  saveLights();
   void  saveLight( const Light* light, uint idx );
   void  saveMaterials();
   void  saveMaterial( const Material* mat );
   void  saveEffects();
   void  saveEffect( const Material* mat );
   void  saveGeometries();
   void  saveMeshGeometry( const MeshGeometry* tm, uint idx );
   void  saveMetaGeometry( const MetaGeometry* mg, uint idx );
   void  saveVisualScenes();
   // Instances.
   void  saveCameraInstances();
   void  saveCameraInstance( const Camera* cam, uint idx );
   void  saveLightInstances();
   void  saveLightInstance( const Light* light, uint idx );
   void  saveGeometryInstances();
   void  saveGeometryInstance( const Entity* e, uint idx );
   void  saveScene();

protected:

   /*----- data members -----*/

   RCP<const World>      _world;
   TextStream&           _os;
   StreamIndent&         _indent;
   Set<const Material*>  _materials;
   Set<const Image*>     _images;
   Set<String>           _matID;
   Set<String>           _fxID;
}; //class State

//------------------------------------------------------------------------------
//!
bool save( const World& world, const Path& path )
{
   RCP<FileDevice> fd = new FileDevice( path, IODevice::MODE_WRITE|IODevice::MODE_STRICT );
   if( fd.isValid() )
   {
      TextStream os( fd.ptr() );
      StreamIndent indent;
      return save( world, os, indent );
   }
   else
   {
      return false;
   }
}

//------------------------------------------------------------------------------
//!
bool save( const World& world, TextStream& os, StreamIndent& indent )
{
   State state( world, os, indent );
   state.save();
   return os.ok();
}

//------------------------------------------------------------------------------
//!
void State::preprocess()
{
   CHECK( _world.isValid() );

   for( uint i = 0; i < _world->numEntities(); ++i )
   {
      const Entity*   e = _world->entity( i );
      const Geometry* g = e->geometry();
      if( g == NULL )  continue;
      //_geometries.add( g );

      const MaterialSet* ms = e->materialSet();
      if( ms == NULL )  continue;
      for( uint i = 0; i < ms->numMaterials(); ++i )
      {
         const Material* mat = ms->material( i );
         if( mat == NULL )  continue;
         _materials.add( mat );

         switch( mat->type() )
         {
            case Material::BASE:
            {
               BaseMaterial* bmat = (BaseMaterial*)mat;
               for( uint i = 0; i < bmat->numLayers(); ++i )
               {
                  _images.add( bmat->layer(i)._color.ptr() );
               }
            }  break;
            case Material::REFLECTIVE_PLANAR:
            {
               ReflectivePlanarMaterial* rmat = (ReflectivePlanarMaterial*)mat;
               for( uint i = 0; i < rmat->numLayers(); ++i )
               {
                  _images.add( rmat->layer(i)._color.ptr() );
               }
            }  break;

            default: StdErr << "material type not supported.\n";
         }
      }
   }

   //StdErr << _geometries.size() << " geoms" << nl;
   StdErr << _materials.size() << " mats" << nl;
   StdErr << _images.size() << " images" << nl;
}

//------------------------------------------------------------------------------
//!
void State::save()
{
   preprocess();

   _os << _indent << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>" << nl;
   XML_BLOCK_OPEN( "<COLLADA xmlns=\"http://www.collada.org/2005/11/COLLADASchema\" version=\"1.4.1\">" );
   //XML_BLOCK_OPEN( "<COLLADA xmlns=\"http://www.collada.org/2008/03/COLLADASchema\" version=\"1.5.0\">" );

   saveAsset();
   saveCameras();
   saveLights();
   saveImages();
   saveMaterials();
   saveEffects();
   saveGeometries();
   saveVisualScenes();
   saveScene();

   XML_BLOCK_CLOSE( "</COLLADA>" );
}

//------------------------------------------------------------------------------
//!
void State::saveAsset()
{
   XML_BLOCK_OPEN( "<asset>" );

   XML_BLOCK_OPEN( "<contributor>" );
   // <author>someone</author>
   _os << _indent << "<authoring_tool>Plasma</authoring_tool>" << nl;
   //_os << _indent << "<comments>" << nl;
   //FusionApp* fApp = (FusionApp*)sApp;
   //fApp->printInfo( _os );
   //_os << _indent << "</comments>" << nl;
   _os << _indent << "<copyright>Copyright (c) Ludo Sapiens Inc.  All rights reserved.</copyright>" << nl;
   XML_BLOCK_CLOSE( "</contributor>" );

   Date now = Date::now();
   String nowStr = now.toISO8601();
   _os << _indent << "<created>" << nowStr << "</created>" << nl;
   _os << _indent << "<modified>" << nowStr << "</modified>" << nl;
   _os << _indent << "<unit meter=\"1.0\" name=\"meter\"/>" << nl;
#if USE_Z_UP
   _os << _indent << "<up_axis>Z_UP</up_axis>" << nl;
#else
   _os << _indent << "<up_axis>Y_UP</up_axis>" << nl;
#endif

   XML_BLOCK_CLOSE( "</asset>" );
}

//------------------------------------------------------------------------------
//!
void State::saveCameras()
{
   XML_BLOCK_OPEN( "<library_cameras>" );

   for( uint i = 0; i < _world->numCameras(); ++i )
   {
      const Camera* cam = _world->camera(i);
      saveCamera( cam, i );
   }

   XML_BLOCK_CLOSE( "</library_cameras>" );
}

//------------------------------------------------------------------------------
//!
void State::saveCamera( const Camera* cam, uint idx )
{
   String name = cameraName( idx );
   XML_BLOCK_OPEN( "<camera id=\"" << name << "\" name=\"" << name << "\">" );
   XML_BLOCK_OPEN( "<optics>" );
   XML_BLOCK_OPEN( "<technique_common>" );
   if( cam->projection() == Camera::PERSPECTIVE )
   {
      XML_BLOCK_OPEN( "<perspective>" );
      _os << _indent << "<yfov>" << cam->fov() << "</yfov>" << nl;  // TODO: support all FOVModes.
      _os << _indent << "<aspect_ratio>1</aspect_ratio>" << nl;
      _os << _indent << "<znear>" << cam->front() << "</znear>" << nl;
      _os << _indent << "<zfar>" << cam->back() << "</zfar>" << nl;
      CHECK( cam->shearX() == 0.0f );
      CHECK( cam->shearY() == 0.0f );
      XML_BLOCK_CLOSE( "</perspective>" );
   }
   else
   {
      StdErr << "ResExporter::Collada doesn't support ortho cameras for now." << nl;
      CHECK( false );
   }
   XML_BLOCK_CLOSE( "</technique_common>" );
   XML_BLOCK_CLOSE( "</optics>" );
   XML_BLOCK_CLOSE( "</camera>" );
}

//------------------------------------------------------------------------------
//!
void State::saveEffects()
{
   XML_BLOCK_OPEN( "<library_effects>" );

   for( Set<const Material*>::ConstIterator cur = _materials.begin();
        cur != _materials.end();
        ++cur )
   {
      const Material* mat = *cur;
      saveEffect( mat );
   }

   XML_BLOCK_CLOSE( "</library_effects>" );
}

//------------------------------------------------------------------------------
//!
void State::saveEffect( const Material* mat )
{
   String fxID   = xs_ID( effectID( mat ) );
   String fxName = xs_token( effectName( mat ) );

   if( _fxID.has( fxID ) ) return;
   _fxID.add( fxID );

   XML_BLOCK_OPEN( "<effect id=\"" << fxID << "\" name=\"" << fxName << "\">" );
   XML_BLOCK_OPEN( "<profile_COMMON>" );

   switch( mat->type() )
   {
      case Material::BASE:
      {
         BaseMaterial* bmat = (BaseMaterial*)mat;

         const Image* img = (bmat->numLayers() > 0) ? bmat->layer(0)._color.ptr() : NULL;

         if( img != NULL )
         {
            bool oneByOne = img->bitmap()->size() == img->bitmap()->pixelSize();

            if( oneByOne )
            {
               // Merge both material color and texture pixel into a single ambient color.
               Vec4f color = img->bitmap()->getColor( Vec2i(0) ) * bmat->color();
               XML_BLOCK_OPEN( "<technique sid=\"COMMON\">" );
               XML_BLOCK_OPEN( "<lambert>" );
               XML_BLOCK_OPEN( "<diffuse>" );
               _os << _indent << "<color>" << toXml(color) << "</color>" << nl;
               XML_BLOCK_CLOSE( "</diffuse>" );
               XML_BLOCK_CLOSE( "</lambert>" );
               XML_BLOCK_CLOSE( "</technique>" );
            }
            else
            {
               // Apparently, despite being huge, Collada doesn't support multiple
               // diffuse sources:
               //   http://collada.org/public_forum/viewtopic.php?f=12&t=1080&start=0
               // So we only dump the texture data in this case.
               // (Note: Blender doesn't seem to honor an emissive term)

               String imgID     = xs_ID( imageID(img) );
               String samplerID = imgID + "-sampler";
               String surfaceID = imgID + "-surface";

               // Dump sampler information.
               XML_BLOCK_OPEN( "<newparam sid=\"" << surfaceID << "\">" );
               XML_BLOCK_OPEN( "<surface type=\"2D\">" );
               _os << _indent << "<init_from>" << imgID << "</init_from>" << nl;
               XML_BLOCK_CLOSE( "</surface>" );
               XML_BLOCK_CLOSE( "</newparam>" );

               XML_BLOCK_OPEN( "<newparam sid=\"" << samplerID << "\">" );
               XML_BLOCK_OPEN( "<sampler2D>" );
               _os << _indent << "<source>" << surfaceID << "</source>" << nl;
               int clamp = bmat->layer(0)._clamp;
               if( clamp != Gfx::TEX_CLAMP_WRAP )
               {
                  // Only dump if it is different than the default.
                  String wrap = clampToWrap( clamp );
                  _os << _indent << "<wrap_s>" << wrap << "</wrap_s>" << nl;
                  _os << _indent << "<wrap_t>" << wrap << "</wrap_t>" << nl;
               }
               XML_BLOCK_CLOSE( "</sampler2D>" );
               XML_BLOCK_CLOSE( "</newparam>" );

               // Dump diffuse reference.
               XML_BLOCK_OPEN( "<technique sid=\"COMMON\">" );
               XML_BLOCK_OPEN( "<lambert>" );
               XML_BLOCK_OPEN( "<diffuse>" );
               _os << _indent << "<texture texture=\"" << samplerID << "\" texcoord=\"UVSET0\"/>" << nl;
               XML_BLOCK_CLOSE( "</diffuse>" );
               XML_BLOCK_CLOSE( "</lambert>" );
               XML_BLOCK_CLOSE( "</technique>" );
            }
         }
         else
         {
            XML_BLOCK_OPEN( "<technique sid=\"COMMON\">" );
            XML_BLOCK_OPEN( "<lambert>" );
            XML_BLOCK_OPEN( "<diffuse>" );
            _os << _indent << "<color>" << toXml(bmat->color()) << "</color>" << nl;
            XML_BLOCK_CLOSE( "</diffuse>" );
            XML_BLOCK_CLOSE( "</lambert>" );
            XML_BLOCK_CLOSE( "</technique>" );
         }
      }  break;
      case Material::REFLECTIVE_PLANAR:
      {
         //ReflectivePlanarMaterial* rmat = (ReflectivePlanarMaterial*)mat;
      }  break;
      default:
      {
         StdErr << "State::saveEffect() - Material type not supported." << nl;
      }  break;
   }

   XML_BLOCK_CLOSE( "</profile_COMMON>" );
   XML_BLOCK_CLOSE( "</effect>" );
}

//------------------------------------------------------------------------------
//!
void State::saveImages()
{
   XML_BLOCK_OPEN( "<library_images>" );

   for( Set<const Image*>::ConstIterator cur = _images.begin();
        cur != _images.end();
        ++cur )
   {
      const Image* img = *cur;
      saveImage( img );
   }

   XML_BLOCK_CLOSE( "</library_images>" );
}

//------------------------------------------------------------------------------
//!
void State::saveImage( const Image* img )
{
   bool oneByOne = img->bitmap()->size() == img->bitmap()->pixelSize();
   if( oneByOne )  return;  // Optimized path, see saveEffect().

   const String& srcPath = img->sourcePath();
   if( srcPath.getExt() == "img" )
   {
      StdErr << "Warning - '" << srcPath << "' is a procedural texture and isn't currently dumped." << nl;
   }
   String imgID   = xs_ID( imageID( img ) );
   String imgName = xs_token( imageName( img ) );
   String imgPath = String("..") + srcPath.sub( srcPath.find("/texture/"), String::npos );
   XML_BLOCK_OPEN( "<image id=\"" << imgID << "\" name=\"" << imgName << "\">" );
   _os << _indent << "<init_from>" << imgPath << "</init_from>" << nl;
   XML_BLOCK_CLOSE( "</image>" );
}

//------------------------------------------------------------------------------
//!
void State::saveLights()
{
   uint n = _world->numLights();
   if( n == 0 )  return;

   XML_BLOCK_OPEN( "<library_lights>" );
   for( uint i = 0; i < n; ++i )
   {
      const Light* light = _world->light(i);
      saveLight( light, i );
   }
   XML_BLOCK_CLOSE( "</library_lights>" );
}

//------------------------------------------------------------------------------
//!
void State::saveLight( const Light* light, uint idx )
{
   String name = lightName( idx );
   XML_BLOCK_OPEN( "<light id=\"" << name << "\" name=\"" << name << "\">" );
   XML_BLOCK_OPEN( "<technique_common>" );
   // Only directional lights for now.
   XML_BLOCK_OPEN( "<spot>" );
   _os << _indent << "<color>" << toXml(light->intensity()) << "</color>" << nl;
   _os << _indent << "<falloff_angle>" << light->fov() << "</falloff_angle>" << nl;
   XML_BLOCK_CLOSE( "</spot>" );
   XML_BLOCK_CLOSE( "</technique_common>" );
   XML_BLOCK_CLOSE( "</light>" );
}

//------------------------------------------------------------------------------
//!
void State::saveMaterials()
{
   XML_BLOCK_OPEN( "<library_materials>" );

   for( Set<const Material*>::ConstIterator cur = _materials.begin();
        cur != _materials.end();
        ++cur )
   {
      const Material* mat = *cur;
      saveMaterial( mat );
   }

   XML_BLOCK_CLOSE( "</library_materials>" );
}

//------------------------------------------------------------------------------
//!
void State::saveMaterial( const Material* mat )
{
   String matID     = xs_ID( materialID( mat ) );
   String matName   = xs_token( materialName( mat ) );
   String matEffect = xs_ID( effectID( mat ) );

   if( _matID.has( matID ) ) return;
   _matID.add( matID );

   XML_BLOCK_OPEN( "<material id=\"" << matID << "\" name=\"" << matName << "\">" );

   _os << _indent << "<instance_effect url=\"#" << matEffect << "\"/>" << nl;

   XML_BLOCK_CLOSE( "</material>" );
}

//------------------------------------------------------------------------------
//!
void State::saveGeometries()
{
   XML_BLOCK_OPEN( "<library_geometries>" );

   for( uint i = 0; i < _world->numEntities(); ++i )
   {
      const   Entity* e = _world->entity( i );
      const Geometry* g = e->geometry();
      if( g == NULL )  continue;
      switch( g->type() )
      {
         case Geometry::SURFACE:
            // TODO.
            //saveSurface( g->surface(), i );
            break;
         case Geometry::MESH:
            saveMeshGeometry( g->mesh(), i );
            break;
         case Geometry::METAGEOMETRY:
            saveMetaGeometry( g->metaGeometry(), i );
            break;
         default:
            CHECK( false );
            break;
      }
   }

   XML_BLOCK_CLOSE( "</library_geometries>" );
}

//------------------------------------------------------------------------------
//!
void State::saveMeshGeometry( const MeshGeometry* tm, uint idx )
{
   // FIXME: Only supports triangles mesh for now.
   if( tm->primitiveType() != MeshGeometry::TRIANGLES ) return;

   //String id       = xs_ID( geometryID(tm) );
   String id       = xs_ID( geometryName(idx) );
   String name     = xs_token( geometryName(tm) );
   String nm_xyz    = id + "_XYZ";
   String nm_xyz_d  = nm_xyz + "_data";
   String nm_uv     = id + "_UV";
   String nm_uv_d   = nm_uv + "_data";
   String nm_norm   = id + "_N";
   String nm_norm_d = nm_norm + "_data";
   String vname     = id + "_vertices";

   XML_BLOCK_OPEN( "<geometry id=\"" << id << "\" name=\"" << name << "\">" );
   XML_BLOCK_OPEN( "<mesh>" );

   // Dump vertex data.
   uint nv = tm->numVertices();

   // XYZ.
   XML_BLOCK_OPEN( "<source id=\"" << nm_xyz << "\" name=\"XYZ\">" );
   XML_BLOCK_OPEN( "<float_array id=\"" << nm_xyz_d << "\" count=\"" << nv*3 << "\">" );
   for( uint i = 0; i < nv; ++i )
   {
      const Vec3f& p = tm->position(i);
      _os << _indent << p.x << " " << p.y << " " << p.z << nl;
   }
   XML_BLOCK_CLOSE( "</float_array>" );
   XML_BLOCK_OPEN( "<technique_common>" );
   XML_BLOCK_OPEN( "<accessor count=\"" << nv << "\" offset=\"0\" stride=\"3\" source=\"#" << nm_xyz << "\">" );
   _os << _indent << "<param name=\"X\" type=\"float\"/>" << nl;
   _os << _indent << "<param name=\"Y\" type=\"float\"/>" << nl;
   _os << _indent << "<param name=\"Z\" type=\"float\"/>" << nl;
   XML_BLOCK_CLOSE( "</accessor>" );
   XML_BLOCK_CLOSE( "</technique_common>" );
   XML_BLOCK_CLOSE( "</source>" );

   // Normal.
   if( tm->hasNormal() )
   {
      XML_BLOCK_OPEN( "<source id=\"" << nm_norm << "\" name=\"Normal\">" );
      XML_BLOCK_OPEN( "<float_array id=\"" << nm_norm_d << "\" count=\"" << nv*3 << "\">" );
      for( uint i = 0; i < nv; ++i )
      {
         const Vec3f& n = tm->normal(i);
         _os << _indent << n.x << " " << n.y << " " << n.z << nl;
      }
      XML_BLOCK_CLOSE( "</float_array>" );
      XML_BLOCK_OPEN( "<technique_common>" );
      XML_BLOCK_OPEN( "<accessor count=\"" << nv << "\" offset=\"0\" stride=\"3\" source=\"#" << nm_norm << "\">" );
      _os << _indent << "<param name=\"X\" type=\"float\"/>" << nl;
      _os << _indent << "<param name=\"Y\" type=\"float\"/>" << nl;
      _os << _indent << "<param name=\"Z\" type=\"float\"/>" << nl;
      XML_BLOCK_CLOSE( "</accessor>" );
      XML_BLOCK_CLOSE( "</technique_common>" );
      XML_BLOCK_CLOSE( "</source>" );
   }

   // UV.
   if( tm->hasMapping() )
   {
      XML_BLOCK_OPEN( "<source id=\"" << nm_uv << "\" name=\"UV\">" );
      XML_BLOCK_OPEN( "<float_array id=\"" << nm_uv_d << "\" count=\"" << nv*2 << "\">" );
      for( uint i = 0; i < nv; ++i )
      {
         const Vec2f& m = tm->mapping(i);
         _os << _indent << m.x << " " << m.y << nl;
      }
      XML_BLOCK_CLOSE( "</float_array>" );
      XML_BLOCK_OPEN( "<technique_common>" );
      XML_BLOCK_OPEN( "<accessor count=\"" << nv << "\" offset=\"0\" stride=\"2\" source=\"#" << nm_uv << "\">" );
      _os << _indent << "<param name=\"U\" type=\"float\"/>" << nl;
      _os << _indent << "<param name=\"V\" type=\"float\"/>" << nl;
      XML_BLOCK_CLOSE( "</accessor>" );
      XML_BLOCK_CLOSE( "</technique_common>" );
      XML_BLOCK_CLOSE( "</source>" );
   }

   // Dump vertex information.
   XML_BLOCK_OPEN( "<vertices id=\"" << vname << "\">" );
   _os << _indent << "<input semantic=\"POSITION\" source=\"#" << nm_xyz << "\"/>" << nl;
   XML_BLOCK_CLOSE( "</vertices>" );

   // Dump index buffer.
   MaterialSet* matSet = _world->entity(idx)->materialSet();
   uint nPatches = tm->numPatches();
   for( uint patchID = 0; patchID < nPatches; ++patchID )
   {
      const Geometry::PatchInfo& patchInfo = tm->patchInfo( patchID );
      const Material*   mat = tm->material( matSet, patchID );
      CHECK( mat );
      const String  matName = materialName( mat );
      XML_BLOCK_OPEN( "<triangles count=\"" << patchInfo.rangeSize()/3 << "\" material=\"" << matName << "\">" );
      _os << _indent << "<input offset=\"0\" semantic=\"VERTEX\" source=\"#" << vname << "\"/>" << nl;
      if( tm->hasNormal()  )  _os << _indent << "<input offset=\"0\" semantic=\"NORMAL\" source=\"#" << nm_norm << "\"/>" << nl;
      if( tm->hasMapping() )  _os << _indent << "<input offset=\"0\" semantic=\"TEXCOORD\" source=\"#" << nm_uv << "\"/>" << nl;
      XML_BLOCK_OPEN( "<p>" );
      const uint* curU = tm->indices() + patchInfo.rangeStart();
      const uint* endU = curU + patchInfo.rangeSize();
      while( curU < endU )
      {
         _os << _indent << curU[0] << " " << curU[1] << " " << curU[2] << nl;
         curU += 3;
      }
      XML_BLOCK_CLOSE( "</p>" );
      XML_BLOCK_CLOSE( "</triangles>" );
   }

   XML_BLOCK_CLOSE( "</mesh>" );
   XML_BLOCK_CLOSE( "</geometry>" );
}

//------------------------------------------------------------------------------
//!
void State::saveMetaGeometry( const MetaGeometry* mg, uint idx )
{
   Vector<float>  vData;
   Vector<uint>   iData;
   mg->triangulate( vData, iData );

   //String id       = xs_ID( geometryID(tm) );
   String id       = xs_ID( geometryName(idx) );
   String name     = xs_token( geometryName(mg) );
   String nm_xyz   = id + "_XYZ";
   String nm_xyz_d = nm_xyz + "_data";
   String nm_uv    = id + "_UV";
   String nm_uv_d  = nm_uv + "_data";
   String vname    = id + "_vertices";

   XML_BLOCK_OPEN( "<geometry id=\"" << id << "\" name=\"" << name << "\">" );
   XML_BLOCK_OPEN( "<mesh>" );

   // Dump vertex data (XYZUV).
   uint nv = uint(vData.size()/5);
   uint nt = uint(iData.size()/3);
   const float* curF;
   const float* endF;
   const uint*  curU;
   const uint*  endU;
   CHECK( (nv*5) == vData.size() ); // Means the last XYZUV chunk is incomplete.
   CHECK( (nt*3) == iData.size() ); // Means the last triangle is incomplete.

   // XYZ.
   XML_BLOCK_OPEN( "<source id=\"" << nm_xyz << "\" name=\"XYZ\">" );
   XML_BLOCK_OPEN( "<float_array id=\"" << nm_xyz_d << "\" count=\"" << nv*3 << "\">" );
   for( curF = vData.data(), endF = curF + nv*5;
        curF != endF;
        curF += 5 )
   {
      _os << _indent << curF[0] << " " << curF[1] << " " << curF[2] << nl;
      // Account for Blender's "Z is up" situation, i.e. XYZ --> YZX
      //_os << _indent << curF[2] << " " << curF[0] << " " << curF[1] << nl;
   }
   XML_BLOCK_CLOSE( "</float_array>" );
   XML_BLOCK_OPEN( "<technique_common>" );
   XML_BLOCK_OPEN( "<accessor count=\"" << nv << "\" offset=\"0\" stride=\"3\" source=\"#" << nm_xyz << "\">" );
   _os << _indent << "<param name=\"X\" type=\"float\"/>" << nl;
   _os << _indent << "<param name=\"Y\" type=\"float\"/>" << nl;
   _os << _indent << "<param name=\"Z\" type=\"float\"/>" << nl;
   XML_BLOCK_CLOSE( "</accessor>" );
   XML_BLOCK_CLOSE( "</technique_common>" );
   XML_BLOCK_CLOSE( "</source>" );

   // UV.
   XML_BLOCK_OPEN( "<source id=\"" << nm_uv << "\" name=\"UV\">" );
   XML_BLOCK_OPEN( "<float_array id=\"" << nm_uv_d << "\" count=\"" << nv*2 << "\">" );
   for( curF = vData.data(), endF = curF + nv*5;
        curF != endF;
        curF += 5 )
   {
      _os << _indent << curF[3] << " " << curF[4] << nl;
   }
   XML_BLOCK_CLOSE( "</float_array>" );
   XML_BLOCK_OPEN( "<technique_common>" );
   XML_BLOCK_OPEN( "<accessor count=\"" << nv << "\" offset=\"0\" stride=\"2\" source=\"#" << nm_uv << "\">" );
   _os << _indent << "<param name=\"U\" type=\"float\"/>" << nl;
   _os << _indent << "<param name=\"V\" type=\"float\"/>" << nl;
   XML_BLOCK_CLOSE( "</accessor>" );
   XML_BLOCK_CLOSE( "</technique_common>" );
   XML_BLOCK_CLOSE( "</source>" );

   // Dump vertex information.
   XML_BLOCK_OPEN( "<vertices id=\"" << vname << "\">" );
   _os << _indent << "<input semantic=\"POSITION\" source=\"#" << nm_xyz << "\"/>" << nl;
   XML_BLOCK_CLOSE( "</vertices>" );

   // Dump index buffer.
   XML_BLOCK_OPEN( "<triangles count=\"" << nt << "\" material=\"\">" ); // Empty material name to avoid crash in Blender's loading code.
   _os << _indent << "<input offset=\"0\" semantic=\"VERTEX\" source=\"#" << vname << "\"/>" << nl;
   _os << _indent << "<input offset=\"0\" semantic=\"TEXCOORD\" source=\"#" << nm_uv << "\"/>" << nl;
   XML_BLOCK_OPEN( "<p>" );
   for( curU = iData.data(), endU = curU + nt*3;
        curU != endU;
        curU += 3 )
   {
      _os << _indent << curU[0] << " " << curU[1] << " " << curU[2] << nl;
   }
   XML_BLOCK_CLOSE( "</p>" );
   XML_BLOCK_CLOSE( "</triangles>" );

   XML_BLOCK_CLOSE( "</mesh>" );
   XML_BLOCK_CLOSE( "</geometry>" );
}

//------------------------------------------------------------------------------
//!
void State::saveVisualScenes()
{
   XML_BLOCK_OPEN( "<library_visual_scenes>" );
   XML_BLOCK_OPEN( "<visual_scene id=\"world\">" );
   //XML_BLOCK_OPEN( "<node id=\"root\">" );
   //_os << _indent << "<rotate>1 0 0 90</rotate>" << nl; // Blender uses Z_UP axis.
   saveCameraInstances();
   saveLightInstances();
   saveGeometryInstances();
   //XML_BLOCK_CLOSE( "</node>" );
   XML_BLOCK_CLOSE( "</visual_scene>" );
   XML_BLOCK_CLOSE( "</library_visual_scenes>" );
}

//------------------------------------------------------------------------------
//!
void State::saveCameraInstances()
{
   for( uint i = 0; i < _world->numCameras(); ++i )
   {
      const Camera* cam = _world->camera(i);
      saveCameraInstance( cam, i );
   }
}

//------------------------------------------------------------------------------
//!
void State::saveCameraInstance( const Camera* cam, uint idx )
{
   String name = cameraName( idx );
   XML_BLOCK_OPEN( "<node sid=\"ref_" << name << "\">" );
   XML_BLOCK_OPEN( "<matrix> ");
   dumpMatrix( fixMatrix(cam->referential()), _os, _indent );
   XML_BLOCK_CLOSE( "</matrix>" );
   _os << _indent << "<instance_camera url=\"#" << name << "\"/>" << nl;
   XML_BLOCK_CLOSE( "</node>" );
}

//------------------------------------------------------------------------------
//!
void State::saveLightInstances()
{
   for( uint i = 0; i < _world->numLights(); ++i )
   {
      const Light* light = _world->light(i);
      saveLightInstance( light, i );
   }
}

//------------------------------------------------------------------------------
//!
void State::saveLightInstance( const Light* light, uint idx )
{
   String name = lightName( idx );
   XML_BLOCK_OPEN( "<node sid=\"ref_" << name << "\">" );
   XML_BLOCK_OPEN( "<matrix> ");
   dumpMatrix( fixMatrix(light->referential()), _os, _indent );
   XML_BLOCK_CLOSE( "</matrix>" );
   _os << _indent << "<instance_light url=\"#" << name << "\"/>" << nl;
   XML_BLOCK_CLOSE( "</node>" );
}

//------------------------------------------------------------------------------
//!
void State::saveGeometryInstances()
{
   for( uint i = 0; i < _world->numEntities(); ++i )
   {
      const Entity* e   = _world->entity( i );
      const Geometry* g = e->geometry();
      if( g == NULL )  continue;
      saveGeometryInstance( e, i );
   }
}

//------------------------------------------------------------------------------
//!
void State::saveGeometryInstance( const Entity* e, uint idx )
{
   String name = geometryName( idx );
   XML_BLOCK_OPEN( "<node sid=\"ref_" << name << "\">" );

   XML_BLOCK_OPEN( "<matrix> ");
   dumpMatrix( fixMatrix(e->referential()), _os, _indent );
   XML_BLOCK_CLOSE( "</matrix>" );

   XML_BLOCK_OPEN( "<instance_geometry url=\"#" << name << "\">" );
   const MaterialSet* ms = e->materialSet();
   if( ms != NULL )
   {
      XML_BLOCK_OPEN( "<bind_material>" );
      XML_BLOCK_OPEN( "<technique_common>" );
      _matID.clear();
      for( uint i = 0; i < ms->numMaterials(); ++i )
      {
         const Material* mat = ms->material( i );
         String matID   = xs_ID( materialID( mat ) );
         String matName = xs_token( materialName( mat ) );
         if( _matID.has( matID ) ) continue;
         _matID.add( matID );
         XML_BLOCK_OPEN( "<instance_material symbol=\"" << matName << "\" target=\"#" << matID << "\">" );
         // Only dump vertex binding if we have texture data.
         /*
         if( mat->numLayers() > 0 )
         {
            const Image* img = mat->layer(0)._color.ptr();
            if( img != NULL && img->bitmap()->size() != img->bitmap()->pixelSize() )
            {
               _os << _indent << "<bind_vertex_input semantic=\"UVSET0\" input_semantic=\"TEXCOORD\" input_set=\"0\"/>" << nl;
            }
         }
         */
         XML_BLOCK_CLOSE( "</instance_material>" );
      }
      XML_BLOCK_CLOSE( "</technique_common>" );
      XML_BLOCK_CLOSE( "</bind_material>" );
   }
   XML_BLOCK_CLOSE( "</instance_geometry>" );

   XML_BLOCK_CLOSE( "</node>" );
}

//------------------------------------------------------------------------------
//!
void State::saveScene()
{
   XML_BLOCK_OPEN( "<scene>" );
   _os << _indent << "<instance_visual_scene url=\"#world\"/>" << nl;
   XML_BLOCK_CLOSE( "</scene>" );
}

} // namespace Collada

/*==============================================================================
   Bin
==============================================================================*/

//------------------------------------------------------------------------------
//!
bool Bin::save( const Geometry& geom, const Path& path )
{
   RCP<FileDevice> fd = new FileDevice( path.cstr(), IODevice::MODE_WRITE );
   return save( geom, fd.ptr() );
}

//------------------------------------------------------------------------------
//!
bool Bin::saveCompressed( const Geometry& geom, const Path& path )
{
   RCP<GZippedFileDevice> fd = new GZippedFileDevice( path.cstr(), IODevice::MODE_WRITE );
   return save( geom, fd.ptr() );
}

//------------------------------------------------------------------------------
//!
bool Bin::save( const Geometry& geom, IODevice* dev )
{
   if( !dev ) return false;

   RCP<MeshGeometry> mesh = nullptr;
   switch( geom.type() )
   {
      case Geometry::MESH:
         mesh = geom.mesh();
         break;
      case Geometry::METAGEOMETRY:
         mesh = new MeshGeometry();
         geom.metaGeometry()->createMesh( *mesh );
         break;
      default:
         return false;
   }
   if( mesh.isNull() ) return false;

   BinaryStream os = BinaryStream( dev );
   return Serializer::dumpMeshGeometry( *mesh, os );
}

/*==============================================================================
   Obj
==============================================================================*/

//------------------------------------------------------------------------------
//!
bool Obj::save( const Geometry& geom, const Path& path )
{
   RCP<FileDevice> fd = new FileDevice( path, IODevice::MODE_WRITE|IODevice::MODE_STRICT );
   if( fd.isValid() && fd->ok() )
   {
      TextStream os( fd.ptr() );
      StreamIndent indent;
      return save( geom, os, indent );
   }
   else
   {
      StdErr << "ERROR - Obj::save(Geometry&, Path&) could not open file for writing: '" << path.string() << "'." << nl;
      return false;
   }
}

//------------------------------------------------------------------------------
//!
bool Obj::save( const Geometry& geom, TextStream& os, StreamIndent& /*indent*/ )
{
   // Retreived mesh geometry.
   MeshGeometry* mesh;
   Vector<uint> faceInfos;

   switch( geom.type() )
   {
      case Geometry::MESH: mesh = geom.mesh(); break;
      case Geometry::METAGEOMETRY:
         mesh = new MeshGeometry();
         geom.metaGeometry()->createMesh( *mesh, &faceInfos );
         break;
      default:
         return false;
   }

   // Vertices.
   os << "#vertices\n";
   for( uint i = 0; i < mesh->numVertices(); ++i )
   {
      const Vec3f& v = mesh->position(i);
      os << "v " << v.x << " " << v.y << " " << v.z << "\n";
   }

   // Texture mapping.
   if( mesh->hasMapping() )
   {
      os << "#mapping\n";
      for( uint i = 0; i < mesh->numVertices(); ++i )
      {
         const Vec2f& v = mesh->mapping(i);
         os << "vt " << v.x << " " << v.y << "\n";
      }
   }

   // Normals.
   if( mesh->hasNormal() )
   {
      os << "#normals\n";
      for( uint i = 0; i < mesh->numVertices(); ++i )
      {
         const Vec3f& v = mesh->normal(i);
         os << "vn " << v.x << " " << v.y << " " << v.z << "\n";
      }
   }

   // Faces.
   os << "#faces\n";
   switch( mesh->primitiveType() )
   {
      case MeshGeometry::POINTS:
         break;
      case MeshGeometry::LINES:
         break;
      case MeshGeometry::TRIANGLES:
         if( mesh->hasNormal() && mesh->hasMapping() )
         {
            for( uint i = 0; i < mesh->numPrimitives(); ++i )
            {
               uint32_t i0 = mesh->indices()[i*3]+1;
               uint32_t i1 = mesh->indices()[i*3+1]+1;
               uint32_t i2 = mesh->indices()[i*3+2]+1;
               os << "f "
                  << i0 << "/" << i0 << "/" << i0 << " "
                  << i1 << "/" << i1 << "/" << i1 << " "
                  << i2 << "/" << i2 << "/" << i2 << "\n";
            }
         }
         else if( mesh->hasNormal() )
         {
            for( uint i = 0; i < mesh->numPrimitives(); ++i )
            {
               uint32_t i0 = mesh->indices()[i*3]+1;
               uint32_t i1 = mesh->indices()[i*3+1]+1;
               uint32_t i2 = mesh->indices()[i*3+2]+1;
               os << "f "
                  << i0 << "//" << i0 << " "
                  << i1 << "//" << i1 << " "
                  << i2 << "//" << i2 << "\n";
            }
         }
         else
         {
            for( uint i = 0; i < mesh->numPrimitives(); ++i )
            {
               uint32_t i0 = mesh->indices()[i*3]+1;
               uint32_t i1 = mesh->indices()[i*3+1]+1;
               uint32_t i2 = mesh->indices()[i*3+2]+1;
               os << "f " << i0  << " " << i1 << " " << i2 << "\n";
            }
         }
         break;
   }

   if( !faceInfos.empty() )
   {
      os << "#face informations: blockID, faceID\n";
      for( uint i = 0; i < faceInfos.size(); i += 2 )
      {
         os << "bf " << ((faceInfos[i])&(0x1fffffff)) << " " << ((faceInfos[i]) >> 29) << " " << faceInfos[i+1] << "\n";
      }
   }

   return true;
}

} // namespace ResExporter


NAMESPACE_END
