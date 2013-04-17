/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Fusion/Resource/ResManager.h>

#include <Fusion/Core/Animator.h>
#include <Fusion/Resource/Font.h>
#include <Fusion/Resource/Image.h>
#include <Fusion/Resource/ImageGenerator.h>
#include <Fusion/Resource/ResCache.h>
#include <Fusion/VM/VMRegistry.h>
#include <Fusion/Core/Core.h>

#include <CGMath/Variant.h>

#include <Base/ADT/Map.h>
#include <Base/Dbg/DebugStream.h>
#include <Base/IO/FileDevice.h>
#include <Base/IO/FileSystem.h>
#include <Base/MT/TaskQueue.h>
#include <Base/Util/Application.h>

#include <algorithm>

USING_NAMESPACE

using ResManager::ClearCallback;


/*==============================================================================
  UNNAME NAMESPACE
==============================================================================*/

UNNAMESPACE_BEGIN

DBG_STREAM( os_res, "ResManager" );

const char*   _programExt[] = { ".prog" , 0 };
const char**  _shaderExt  = nullptr;
const char*   _shaderExt_OpenGL[]    = { ".glsl", ".xglsl", 0 };
const char*   _shaderExt_OpenGLES1[] = { ".glff", 0 };
const char*   _shaderExt_OpenGLES2[] = { ".essl", ".xglsl", 0 };
const char*   _shaderExt_Direct3D[]  = { ".hlsl", 0 };

const char*   _soundExt[] = { ".ogg" , 0 };

//------------------------------------------------------------------------------
//!
String                     _vmNamespace;
Vector< ClearCallback >    _cbClear;
Map< String, bool >        _scriptManager;


ResCache< Image, ResManager::DigestKey >*  _rc_image     = NULL;
ResCache< Image, ResManager::DigestKey >*  _rc_imageCube = NULL;
ResCache< Gfx::Texture  >*  _rc_texture = NULL;
ResCache< Font          >*  _rc_font    = NULL;
ResCache< Gfx::Shader   >*  _rc_vs      = NULL;
ResCache< Gfx::Shader   >*  _rc_gs      = NULL;
ResCache< Gfx::Shader   >*  _rc_fs      = NULL;
//ResCache< Gfx::Shader >*  _rc_cs      = NULL;
ResCache< Gfx::Shader   >*  _rc_ff      = NULL;
ResCache< Gfx::Program  >*  _rc_prog    = NULL;
ResCache< Snd::Sound    >*  _rc_snd     = NULL;


//------------------------------------------------------------------------------
//!
struct KeyVariant
{
   const char*    _key;
   const Variant* _variant;
};

//------------------------------------------------------------------------------
//!
inline bool tableSort( const KeyVariant& a, const KeyVariant& b )
{
   return strcmp( a._key, b._key ) < 0;
}

//------------------------------------------------------------------------------
//!
int includeVM( VMState* vm )
{
   String id   = VM::toString( vm, 1 );
   String path = ResManager::idToPath( id, nullptr );
   if( !path.empty() )
   {
      FS::Entry entry( path );
      if( entry.exists() && (entry.type() == FS::TYPE_DIRECTORY) )
      {
         // Calling:
         //   include("theme/something")
         // on a directory results in:
         //   addRoot("theme/something")
         //   include("theme/something/script/something")
         // where the latter should load "theme/something/script/something.ui".
         DBG_MSG( os_res, "Including module: " << id << " path=" << path );
         Core::addRoot( id, 0 );
         String script = id + "/script/" + entry.path().basename().string();
         ResManager::executeScript( vm, script );
         return 0;
      }
   }
   ResManager::executeScript( vm, id );
   return 0;
}

//------------------------------------------------------------------------------
//!
int doScriptVM( VMState* vm )
{
   ResManager::executeScript( vm, VM::toString( vm, 1 ), false );
   return 0;
}

//------------------------------------------------------------------------------
//!
int loadScriptVM( VMState* vm )
{
   ResManager::loadScript( vm, VM::toString( vm, 1 ) );
   return 1;
}

//------------------------------------------------------------------------------
//!
int addRootVM( VMState* vm )
{
   String str = VM::toString( vm, 1 );
   if( VM::getTop( vm ) > 1 )
   {
      Core::addRoot( str, VM::toUInt( vm, 2 ) );
   }
   else
   {
      Core::addRoot( str );
   }
   return 0;
}

//------------------------------------------------------------------------------
//!
void initVM( VMState* vm, uint /*mask*/ )
{
   // Add global functions.
   VM::push( vm, includeVM );
   VM::setGlobal( vm, "include" );

   VM::push( vm, doScriptVM );
   VM::setGlobal( vm, "doScript" );

   VM::push( vm, loadScriptVM );
   VM::setGlobal( vm, "loadScript" );

   VM::push( vm, addRootVM );
   VM::setGlobal( vm, "addRoot" );
}

/*==============================================================================
  CLASS ResManagerAnimator
==============================================================================*/
class ResManagerAnimator:
   public Animator
{
public:

   /*----- methods -----*/

   ResManagerAnimator(): Animator( 3.0 ) {}
   virtual ~ResManagerAnimator() {}

   virtual bool exec( double time, double /*delta*/ )
   {
      ResCacheBase::cleanAll( time );
      return false;
   }

protected:

private:
}; //class ResManagerAnimator

UNNAMESPACE_END

/*==============================================================================
   NAMESPACE ResManager
==============================================================================*/

//------------------------------------------------------------------------------
//!
void ResManager::_register( ClearCallback clear )
{
   _cbClear.pushBack( clear );
}

//------------------------------------------------------------------------------
//!
void ResManager::_unregister( ClearCallback clear )
{
   _cbClear.remove( clear );
}

//------------------------------------------------------------------------------
//!
void ResManager::getDigestPart( const Variant& var, SHA1& sha )
{
   switch( var.type() )
   {
      case Variant::BOOL:
      {
         sha.put( (uint8_t)var.getBoolean() );
      }  break;
      case Variant::FLOAT:
      {
         sha.put( var.getFloat() );
      }  break;
      case Variant::VEC2:
      {
         const Vec2f& v = var.getVec2();
         sha.put( v.x );
         sha.put( v.y );
      } break;
      case Variant::VEC3:
      {
         const Vec3f& v = var.getVec3();
         sha.put( v.x );
         sha.put( v.y );
         sha.put( v.z );
      } break;
      case Variant::VEC4:
      {
         const Vec4f& v = var.getVec4();
         sha.put( v.x );
         sha.put( v.y );
         sha.put( v.z );
         sha.put( v.w );
      } break;
      case Variant::STRING:
      {
         sha.put( var.getString().cstr() );
      }  break;
      case Variant::TABLE:
      {
         getDigestPart( *(var.getTable()), sha );
      }  break;
      default:;
   }
}

//------------------------------------------------------------------------------
//!
void ResManager::getDigestPart( const Table& table, SHA1& sha )
{
   // 1. Integer keys.
   for( uint i = 0; i < table.arraySize(); ++i )
   {
      // Add value.
      getDigestPart( table[i], sha );
   }

   // 2. String keys.

   // Gather all parameters.
   Vector<KeyVariant> params;
   params.reserve( table.mapSize() );

   Table::ConstIterator end = table.end();
   for( Table::ConstIterator iter = table.begin(); iter != end; ++iter )
   {
      KeyVariant kv;
      kv._key     = (*iter).first.cstr();
      kv._variant = &(*iter).second;
      params.pushBack(kv);
   }

   // Sort all parameters by keys (for a consistent order).
   std::sort( params.begin(), params.end(), tableSort );

   // Add each parameter to the SHA.
   for( uint i = 0; i < params.size(); ++i )
   {
      // Add key.
      sha.put( params[i]._key );
      // Add value.
      getDigestPart( *params[i]._variant, sha );
   }
}

//------------------------------------------------------------------------------
//! Compute the SHA1 fingerprint of the table.
void ResManager::getDigest( const Table& table, SHA1::Digest& digest )
{
   SHA1 sha;
   sha.begin();

   getDigestPart( table, sha );

   digest = sha.end();
}

//------------------------------------------------------------------------------
//! Compute the SHA1 fingerprint of the table.
void ResManager::getDigest( const Vec2i& v, const Table* table, SHA1::Digest& digest )
{
   SHA1 sha;
   sha.begin();

   // Add explicit arguments (only v for now).
   sha.put( (uint32_t)v.x );
   sha.put( (uint32_t)v.y );

   if( table )  getDigestPart( *table, sha );

   digest = sha.end();
}

//------------------------------------------------------------------------------
//!
void ResManager::printInfo( TextStream& os )
{
   os << "ResManager:";
   os << nl;
}

//------------------------------------------------------------------------------
//!
void ResManager::clear()
{
   // Call all of the callbacks.
   Vector< ClearCallback >::iterator cur = _cbClear.begin();
   Vector< ClearCallback >::iterator end = _cbClear.end();
   for( ; cur != end; ++cur )
   {
      (*(*cur))();
   }
}

//------------------------------------------------------------------------------
//!
String
ResManager::getURLType( const String& str )
{
   String::SizeType p = str.find( "://" );
   if( p != String::npos )
   {
      return String( str, 0, p );
   }
   return String();
}
//------------------------------------------------------------------------------
//! Returns the directory part of the id.
String
ResManager::dir( const String& id )
{
   String::SizeType siz = id.size();
   String::SizeType pos = id.findLastOf("/", siz-1);
   if( pos != String::npos )
   {
      return id.sub( 0, pos );
   }
   else
   {
      return String();
   }
}

//------------------------------------------------------------------------------
//! A routine to perform tilde expansion in a resource id.
//! Some corner cases:
//!  ""        "~"             -->  ""
//!  ""        "~/"            -->  ""
//!  ""        "~/a"           -->  "a"
//!  "/"       "~/a"           -->  "/a"
//!  "dir"     "~/a"           -->  "dir/a"
//!  "dir/"    "~/a"           -->  "dir/a"
//!  "dir/sub" "~/a"           -->  "dir/sub/a"
//!  "dir/sub" "~/../a"        -->  "dir/a"
//!  "1/2/3/4" "~/../../a"     -->  "1/a"
//!  "1/2"     "~/../../../a"  -->  "../a"
String
ResManager::expand( const String& dir, const String& id )
{
   if( id.empty() ) return id;

   if( id[0] == '~' )
   {
      if( id[1] == '/' )
      {
         String nid = dir;
         if( !nid.empty() && nid[nid.size()-1] != '/' )  nid += '/';
         nid += id.sub( 2 );
         for( String::SizeType p = nid.find( "/../" ); p != String::npos; p = nid.find( "/../" ) )
         {
            String::SizeType s = nid.rfind( '/', p-1 );
            if( s != String::npos )
            {
               nid.erase( s+1, p+3-s );
            }
            else
            {
               nid.erase( 0, p+3-s );
               break;
            }
         }
         return nid;
      }
      else
      {
         StdErr << "ERROR - ResManager::expand() starts with '~' rather than '~/'." << nl;
         return id;
      }
   }

   return id;
}

//------------------------------------------------------------------------------
//! A routine which converts an id string into a full path.
//! If the id is not found, return a NULL string.
String
ResManager::idToPath( const String& id, const char** ext )
{
   String urlType = getURLType( id );
   if( urlType.empty() )
   {
      for( int i = Core::numRoots()-1; i >= 0; --i )
      {
         String path = Core::root(i).string() + id;

         // Try to find a file with a valid extension.
         if( ext )
         {
            for( int i = 0; ext[i] != 0; ++i )
            {
               String fileName = path + ext[i];
               DBG_MSG( os_res, "Trying: " << fileName );
               if( FS::Entry(fileName).exists() ) return fileName;
            }
         }
         else if( FS::Entry(path).exists() ) return path;

         // Try to find a file with no extension.
         //if( FS::Entry(path).exists() )
         //{
         //   return path;
         //}
      }

      // Failed to find any valid file.
      return String();
   }
   else if( urlType == "file" )
   {
      return id.sub( 7 );
   }
   else
   {
      StdErr << "Error with resource " << id << "\n";
      StdErr << " --> URL type not supported\n";
      return String();
   }
}

//------------------------------------------------------------------------------
// Private Fusion API
//------------------------------------------------------------------------------

TaskQueue*  ResManager::_dispatchQueue = NULL;

//------------------------------------------------------------------------------
//!
void
ResManager::initializeFusion()
{
   //os_res.activate();

   _rc_image     = new ResCache< Image, DigestKey >( "Image"          );
   _rc_imageCube = new ResCache< Image, DigestKey >( "ImageCube"      );
   _rc_texture   = new ResCache< Gfx::Texture     >( "Texture"        );
   _rc_font      = new ResCache< Font             >( "Font"           );
   _rc_vs        = new ResCache< Gfx::Shader      >( "VertexShader"   );
   _rc_gs        = new ResCache< Gfx::Shader      >( "GeometryShader" );
   _rc_fs        = new ResCache< Gfx::Shader      >( "FragmentShader" );
   //_rc_cs      = new ResCache< Gfx::Shader      >( "ComputeShader"  );
   _rc_ff        = new ResCache< Gfx::Shader      >( "FixedFunction"  );
   _rc_prog      = new ResCache< Gfx::Program     >( "Program"        );
   _rc_snd       = new ResCache< Snd::Sound       >( "Sound"          );

   VMRegistry::add( initVM, VM_CAT_APP );
   _register( clearFusion );

   Core::addAnimator( new ResManagerAnimator() );
}

//------------------------------------------------------------------------------
//!
void
ResManager::terminateFusion()
{
   delete _dispatchQueue;
   clear();
   _unregister( clearFusion );

   delete _rc_image    ; _rc_image     = NULL;
   delete _rc_imageCube; _rc_imageCube = NULL;
   delete _rc_texture  ; _rc_texture   = NULL;
   delete _rc_font     ; _rc_font      = NULL;
   delete _rc_vs       ; _rc_vs        = NULL;
   delete _rc_gs       ; _rc_gs        = NULL;
   delete _rc_fs       ; _rc_fs        = NULL;
   //delete _rc_cs     ; _rc_cs        = NULL;
   delete _rc_ff       ; _rc_ff        = NULL;
   delete _rc_prog     ; _rc_prog      = NULL;
   delete _rc_snd      ; _rc_snd       = NULL;
}

//------------------------------------------------------------------------------
//!
void
ResManager::clearFusion()
{
   _scriptManager.clear();  //should we skip this one?
   _rc_image->clear();
   _rc_imageCube->clear();
   _rc_texture->clear();
   _rc_font->clear();
   _rc_vs->clear();
   _rc_gs->clear();
   _rc_fs->clear();
   _rc_ff->clear();
   _rc_prog->clear();
   _rc_snd->clear();
}

//------------------------------------------------------------------------------
//!
void
ResManager::fusionGfxManagerChanged()
{
   CHECK( Core::gfx() );
   const String& api = Core::gfx()->API();
   if( api == "OpenGL" )
   {
      _shaderExt = _shaderExt_OpenGL;
   }
   else
   if( api == "OpenGL ES 1.1" )
   {
      _shaderExt = _shaderExt_OpenGLES1;
   }
   else
   if( api == "OpenGL ES 2" )
   {
      _shaderExt = _shaderExt_OpenGLES2;
   }
   else
   if( api == "Direct3D" )
   {
      _shaderExt = _shaderExt_Direct3D;
   }
}

//------------------------------------------------------------------------------
//!
void
ResManager::numThreads( uint n )
{
   delete _dispatchQueue;
   _dispatchQueue = new TaskQueue( n );
}

//------------------------------------------------------------------------------
// Public Fusion API
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//!
RCP< Resource<Image> >
ResManager::getImage( const String& id )
{
   DigestKey key( id );

   RCP< Resource<Image> > imgRes( _rc_image->get( key ) );

   // Load image from disk.
   if( imgRes.isNull() )
   {
      const char* ext[] = { ".png", ".jpg", ".df", 0 };
      String path = idToPath( id, ext );
      if( !path.empty() )
      {
         RCP<Image> img;

         String e = path.getExt();
         if( e == "df" )
         {
            // DataFlow.
            StdErr << "Dataflow not yet supported." << nl;
            return nullptr;
         }
         else
         {
            // Image creation.
            img = new Image( path );
            img->load();
         }

         // Add image to resource.
         imgRes = _rc_image->add( key );
         imgRes->data( img.ptr() );
      }
      else
      {
         StdErr << "Error with image " << id << ".\n";
         StdErr << " --> doesn't know anything about it.\n";
      }
   }
   return imgRes;
}

//------------------------------------------------------------------------------
//!
RCP< Resource<Image> >
ResManager::getImage( const String& id, const Vec2i& size, const Table* args )
{
   DigestKey key( id, size, args );

   RCP< Resource<Image> > imgRes( _rc_image->get( key ) );

   // Generate image from program.
   if( imgRes.isNull() )
   {
      const char* ext[] = { ".img", 0 };
      String path = idToPath( id, ext );
      if( !path.empty() )
      {
         imgRes = _rc_image->add( key );
         // Image generation.
         ImageGenerator  gen( path, args, size ); // TODO: specify format and number of channels.
         gen.execute();                           // TODO: launch in a separate thread.
         // Add image to resource.
         imgRes->data( gen.image() );
      }
      else
      {
         StdErr << "Error with procedural image " << id << ".\n";
         StdErr << " --> doesn't know anything about it.\n";
      }
   }
   return imgRes;
}

//------------------------------------------------------------------------------
//!
String
ResManager::getImageName( const Resource<Image>* res )
{
   if( res->data()->cubemap() )  return _rc_imageCube->get( res ).name();
   else                          return _rc_image->get( res ).name();
}

//------------------------------------------------------------------------------
//!
String
ResManager::getImageName( const Image* img )
{
   if( img->cubemap() )  return _rc_imageCube->get( img ).name();
   else                  return _rc_image->get( img ).name();
}

//------------------------------------------------------------------------------
//!
RCP< Resource<Image> >
ResManager::getImageCube( const String& id )
{
   DigestKey key( id );

   RCP< Resource<Image> > imgRes( _rc_imageCube->get( key ) );

   // Load image from disk.
   if( imgRes.isNull() )
   {
      const char* ext[] = { ".png", ".jpg", ".df", 0 };
      String path = idToPath( id+"_nx", ext );
      if( !path.empty() )
      {
         RCP<Image> img;

         String e = path.getExt();
         if( e == "df" )
         {
            // DataFlow.
            StdErr << "Dataflow not yet supported." << nl;
            return nullptr;
         }
         else
         {
            // Image creation.
            img = new Image();
            img->loadCubemap( path );
         }

         // Add image to resource.
         imgRes = _rc_imageCube->add( key );
         imgRes->data( img.ptr() );
      }
      else
      {
         StdErr << "Error with image " << id << ".\n";
         StdErr << " --> doesn't know anything about it.\n";
      }
   }
   return imgRes;
}

//------------------------------------------------------------------------------
//!
RCP<Gfx::Texture>
ResManager::getRenderableTexture( const String& id )
{
   return data( _rc_texture->get( id ) );
}

//------------------------------------------------------------------------------
//!
void
ResManager::insertRenderableTexture(
   const String&            id,
   const RCP<Gfx::Texture>& texture
)
{
   Resource<Gfx::Texture>* res = _rc_texture->add( id );
   res->data( texture.ptr() );
}

//------------------------------------------------------------------------------
//!
RCP< Resource<Font> >
ResManager::getFont( const String& id )
{
   RCP< Resource<Font> > fontRes( _rc_font->get( id ) );

   // Load font from database.
   if( fontRes.isNull() )
   {
      const char* ext[] = { ".ttf", 0 };
      String path = idToPath( id, ext );
      if( !path.empty() )
      {
         fontRes = _rc_font->add( id );
         // Font creation.
         RCP<Font> font = new Font( path );
         // Add font to resource.
         fontRes->data( font.ptr() );
      }
      else
      {
         StdErr << "Error with font " << id << ".\n";
         StdErr << " --> doesn't know anything about it.\n";
      }
   }
   return fontRes;
}

//------------------------------------------------------------------------------
//!
String
ResManager::getFontName( const Resource<Font>* res )
{
   return _rc_font->get( res );
}

//------------------------------------------------------------------------------
//!
String
ResManager::getFontName( const Font* f )
{
   return _rc_font->get( f );
}

//------------------------------------------------------------------------------
//!
bool
ResManager::handleRootCandidate( const String& path )
{
   DBG_BLOCK( os_res, "ResManager::handleRootCandidate(" << path << ")" );
   // 1. Try to find a subdirectory in the current roots.
   const char* ext[] = { "", 0 };
   String dir = idToPath( path, ext );
   DBG_MSG( os_res, "Finding subdir: '" << dir << "'." );
   if( !dir.empty() )
   {
      DBG_MSG( os_res, "Adding as a subroot." );
      Core::addRoot( dir );
      return true;
   }

   // 2. Try to find an absolute path (or relative to cwd).
   FS::Entry entry( path );
   if( entry.type() == FS::TYPE_DIRECTORY )
   {
      DBG_MSG( os_res, "Adding as a new root." << entry.path().isRelative() );
      Core::addRoot( entry.path() );
      return true;
   }

   return false;
}

//------------------------------------------------------------------------------
//!
void
ResManager::executeScript( VMState* vm, const String& id, bool once )
{
   // Already executed?
   if( once && _scriptManager.find( id ) != _scriptManager.end() ) return;

   const char* ext[] = { ".lua", ".ui", ".pl", 0 };
   String path = idToPath( id, ext );
   if( !path.empty() )
   {
      // Execute script.
      VM::doFile( vm, path );
      VM::setTop( vm, 0 );

      // Flag as executed.
      if( once ) _scriptManager[ id ] = true;
   }
   else
   {
      StdErr << "Error with script " << id << ".\n";
      StdErr << " --> doesn't know anything about it.\n";
   }
}

//------------------------------------------------------------------------------
//!
void
ResManager::loadScript( VMState* vm, const String& id )
{
   const char* ext[] = { ".lua", ".ui", ".pl", 0 };
   String path = idToPath( id, ext );
   if( !path.empty() )
   {
      // Execute script.
      VM::loadFile( vm, path );
   }
   else
   {
      StdErr << "Error with script " << id << ".\n";
      StdErr << " --> doesn't know anything about it.\n";
   }
}

//------------------------------------------------------------------------------
//!
RCP< Resource<Gfx::Shader> >
ResManager::getVertexShader( const String& id )
{
   RCP< Resource<Gfx::Shader> > shaderRes( _rc_vs->get( id ) );

   // Load vertex shader.
   if( shaderRes.isNull() )
   {
      String path = idToPath( id, _shaderExt );
      if( !path.empty() )
      {
         shaderRes = _rc_vs->add( id );

         // Shader creation.
         RCP<FileDevice> file = new FileDevice( path, IODevice::MODE_READ );
         if( file->isOpen() )
         {
            String src;
            file->readAll( src );
            shaderRes->data( Core::gfx()->createShader( Gfx::VERTEX_SHADER, src ).ptr() );
         }
         else
         {
            StdErr << "ERROR - Could not open file: " << path << "\n";
            shaderRes->state( Resource<Gfx::Shader>::LOADED );
         }
      }
      else
      {
         StdErr << "Error with vertex shader " << id << ".\n";
         StdErr << " --> doesn't know anything about it\n";
      }
   }
   return shaderRes;
}

//------------------------------------------------------------------------------
//!
RCP< Resource<Gfx::Shader> >
ResManager::getGeometryShader( const String& id )
{
   RCP< Resource<Gfx::Shader> > shaderRes( _rc_vs->get( id ) );

   // Load vertex shader.
   if( shaderRes.isNull() )
   {
      String path = idToPath( id, _shaderExt );
      if( !path.empty() )
      {
         shaderRes = _rc_vs->add( id );

         // Shader creation.
         RCP<FileDevice> file = new FileDevice( path, IODevice::MODE_READ );
         if( file->isOpen() )
         {
            String src;
            file->readAll( src );
            shaderRes->data( Core::gfx()->createShader( Gfx::GEOMETRY_SHADER, src ).ptr() );
         }
         else
         {
            StdErr << "ERROR - Could not open file: " << path << "\n";
            shaderRes->state( Resource<Gfx::Shader>::LOADED );
         }
      }
      else
      {
         StdErr << "Error with geometry shader " << id << ".\n";
         StdErr << " --> doesn't know anything about it\n";
      }
   }
   return shaderRes;
}

//------------------------------------------------------------------------------
//!
RCP< Resource<Gfx::Shader> >
ResManager::getFragmentShader( const String& id )
{
   RCP< Resource<Gfx::Shader> > shaderRes( _rc_fs->get( id ) );

   // Load fragment shader.
   if( shaderRes.isNull() )
   {
      String path = idToPath( id, _shaderExt );
      if( !path.empty() )
      {
         shaderRes = _rc_fs->add( id );

         // Shader creation.
         RCP<FileDevice> file = new FileDevice( path, IODevice::MODE_READ );
         if( file->isOpen() )
         {
            String src;
            file->readAll( src );
            shaderRes->data( Core::gfx()->createShader( Gfx::FRAGMENT_SHADER, src ).ptr() );
         }
         else
         {
            StdErr << "ERROR - Could not open file: " << path << "\n";
            shaderRes->state( Resource<Gfx::Shader>::LOADED );
         }
      }
      else
      {
         StdErr << "Error with fragment shader " << id << ".\n";
         StdErr << " --> doesn't know anything about it.\n";
      }
   }
   return shaderRes;
}

//------------------------------------------------------------------------------
//!
RCP< Resource<Gfx::Shader> >
ResManager::getFixedFunctionShader( const String& id )
{
   RCP< Resource<Gfx::Shader> > shaderRes( _rc_ff->get( id ) );

   // Load vertex shader.
   if( shaderRes.isNull() )
   {
      String path = idToPath( id, _shaderExt );
      if( !path.empty() )
      {
         shaderRes = _rc_ff->add( id );

         // Shader creation.
         RCP<FileDevice> file = new FileDevice( path, IODevice::MODE_READ );
         if( file->isOpen() )
         {
            String src;
            file->readAll( src );
            shaderRes->data( Core::gfx()->createShader( Gfx::FIXED_FUNCTION_SHADER, src ).ptr() );
         }
         else
         {
            StdErr << "ERROR - Could not open file: " << path << "\n";
            shaderRes->state( Resource<Gfx::Shader>::LOADED );
         }
      }
      else
      {
         StdErr << "Error with fixed function shader " << id << ".\n";
         StdErr << " --> doesn't know anything about it\n";
      }
   }
   return shaderRes;
}

//------------------------------------------------------------------------------
//!
RCP< Resource<Gfx::Program> >
ResManager::getProgram( const String& id )
{
   RCP< Resource<Gfx::Program> > programRes = _rc_prog->get( id );

   // Load program from database.
   if( programRes.isNull() )
   {
      String path = idToPath( id, _programExt );
      if( !path.empty() )
      {
         programRes = _rc_prog->add( id );

         TextStream fstream( new FileDevice( path, IODevice::MODE_READ ) );
         String name;
         RCP<Gfx::Program> program = Core::gfx()->createProgram();
         for( TextStream::LineIterator iter = fstream.lines(); iter.isValid(); ++iter )
         {
            const String& line = *iter;
            if( line.empty() )  continue;
            String::SizeType pos = line.findFirstWhite();
            if( pos > 1 && pos != String::npos )
            {
               String type = line.sub( 0, pos );
               String name = line.sub( pos+1 ).eatWhites();
               switch( tolower(type[0]) )
               {
                  case '/': // C/C++ style comment: "// Something"
                  case '#': // Shell style comment: "# Something"
                  case '-': // Lua   style comment: "-- Something"
                     // Comment lines (skip).
                     break;
                  case 'v': // "vertex name" or "vs name"
                  {
                     Gfx::Shader* vs = data( getVertexShader( name ) );
                     if( vs )
                     {
                        program->addShader( vs );
                     }
                     else
                     {
                        StdErr << "Error loading vertex shader '" << name << "'" << nl;
                     }
                  }  break;
                  case 'g': // "geometry name {point|line|line_adjacency|triangle|triangle_adjacency} {point|line|triangle} <maxVerts>"
                  {
                     Vector<String> args;
                     name.split( " \t", args );
                     if( args.size() != 4 )
                     {
                        StdErr << "ERROR in program '" << id << "': Wrong number of geometry shader arguments (" << args.size() << " instead of 4)." << nl;
                        break;
                     }
                     Gfx::Shader* gs = data( getGeometryShader( args[0] ) );
                     if( gs == nullptr )
                     {
                        StdErr << "Error loading geometry shader '" << args[0] << "'" << nl;
                        break;
                     }
                     Gfx::GSInputType  iType;
                     char first = tolower(args[1][0]);
                     char last  = tolower(args[1][args[1].size()-1]);
                     switch( first )
                     {
                        case 'p': iType = Gfx::GS_IN_POINTS;                                                   break;
                        case 'l': iType = (last=='y') ? Gfx::GS_IN_LINES_ADJACENCY     : Gfx::GS_IN_LINES;     break;
                        case 't': iType = (last=='y') ? Gfx::GS_IN_TRIANGLES_ADJACENCY : Gfx::GS_IN_TRIANGLES; break;
                        default :
                           StdErr << "ERROR: Unknown input type: '" << args[1] << "'." << nl;
                           iType = Gfx::GS_IN_TRIANGLES;
                           break;
                     }
                     Gfx::GSOutputType oType;
                     first = tolower(args[2][0]);
                     last  = tolower(args[2][args[2].size()-1]);
                     switch( first )
                     {
                        case 'p': oType = Gfx::GS_OUT_POINTS;         break;
                        case 'l': oType = Gfx::GS_OUT_LINE_STRIP;     break;
                        case 't': oType = Gfx::GS_OUT_TRIANGLE_STRIP; break;
                        default :
                           StdErr << "ERROR: Unknown output type: '" << args[2] << "'." << nl;
                           oType = Gfx::GS_OUT_TRIANGLE_STRIP;
                           break;
                     }
                     uint maxOut = args[3].toInt();
                     program->addShader( gs );
                     program->setGSParameters( iType, oType, maxOut );
                  }  break;
                  case 'f':
                  {
                     switch( tolower(type[1]) )
                     {
                        case 'r': // "fragment name"
                        case 's': // "fs name"
                        {
                           Gfx::Shader* fs = data( getFragmentShader( name ) );
                           if( fs )
                           {
                              program->addShader( fs );
                           }
                           else
                           {
                              StdErr << "Error loading fragment shader '" << name << "'" << nl;
                           }
                        }  break;
                        case 'i': // "fixed name"
                        case 'f': // "ff name"
                        {
                           Gfx::Shader* ff = data( getFixedFunctionShader( name ) );
                           if( ff )
                           {
                              program->addShader( ff );
                           }
                           else
                           {
                              StdErr << "Error loading fixed function shader '" << name << "'" << nl;
                           }
                        }  break;
                        default:
                        {
                           StdErr << "Unknown shader type: '" << type << "' in program '" << id << "'" << nl;
                        }
                     }
                  }  break;
                  default:
                  {
                     StdErr << "Unknown shader type: '" << type << "' in program '" << id << "'" << nl;
                  }
               }
            }
            else
            {
               StdErr << "Could not determine a valid shader type from '" << line << "' in program '" << id << "'" << nl;
            }
         }
         programRes->data( program.ptr() );
      }
      else
      {
         StdErr << "Error with program " << id << ".\n";
         StdErr << " --> doesn't know anything about it.\n";
      }
   }

   return programRes;
}

//------------------------------------------------------------------------------
//!
String
ResManager::getProgramName( const Resource<Gfx::Program>* res )
{
   return _rc_prog->get( res );
}

//------------------------------------------------------------------------------
//!
String
ResManager::getProgramName( const Gfx::Program* p )
{
   return _rc_prog->get( p );
}

//------------------------------------------------------------------------------
//!
RCP< Resource<Snd::Sound> >
ResManager::getSound( const String& id )
{
   RCP< Resource<Snd::Sound> > soundRes = _rc_snd->get( id );

   // Load sound from database.
   if( soundRes.isNull() )
   {
      String path = idToPath( id, _soundExt );
      if( !path.empty() )
      {
         soundRes = _rc_snd->add( id );

         // Sound creation.
         soundRes->data( Core::snd()->makeSoundFromFile( path ).ptr() );
         if( !soundRes->data() )
         {
            StdErr << "Error with sound '" << id << "' :" << nl
                   << " --> Could not create sound from '" << path << "'." << nl;
         }
      }
      else
      {
         StdErr << "Error with sound '" << id << "' :" << nl
                << " --> Doesn't know anything about it." << nl;
      }
   }

   return soundRes;
}

//------------------------------------------------------------------------------
//!
String
ResManager::getSoundName( const Resource<Snd::Sound>* res )
{
   return _rc_snd->get( res );
}

//------------------------------------------------------------------------------
//!
String
ResManager::getSoundName( const Snd::Sound* snd )
{
   return _rc_snd->get( snd );
}
