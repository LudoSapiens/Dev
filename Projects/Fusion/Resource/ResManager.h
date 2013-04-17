/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef FUSION_RESMANAGER_H
#define FUSION_RESMANAGER_H

#include <Fusion/StdDefs.h>
#include <Fusion/Resource/Resource.h>
#include <Fusion/VM/VM.h>

#include <Gfx/Mgr/Manager.h>
#include <Gfx/Prog/Program.h>

#include <Snd/Sound.h>

#include <CGMath/Variant.h>

#include <Base/ADT/String.h>
#include <Base/Util/RCP.h>
#include <Base/Util/SHA.h>

NAMESPACE_BEGIN

class Font;
class Image;
class TaskQueue;

/*==============================================================================
  CLASS ResManager
==============================================================================*/

namespace ResManager
{

   /*----- methods -----*/


   typedef void (*ClearCallback)();
   FUSION_DLL_API void _register( ClearCallback clear );
   FUSION_DLL_API void _unregister( ClearCallback clear );

   //------------------------------------------------------------------------------
   // Private ResManager Common API
   //------------------------------------------------------------------------------
   class DigestKey
   {
   public:
      DigestKey() {}
      DigestKey( const String& s ): _name( s ) {}
      DigestKey( const String& s, const Table& t ): _name( s ) { digest(t); }
      DigestKey( const String& s, const Table* t ): _name( s ) { if( t ) digest(*t); }
      DigestKey( const String& s, const Vec2i& v, const Table* t ): _name( s ) { digest(v, t); }

      inline const SHA1::Digest&  digest() const { return _digest; }

      inline const String&  name() const { return _name; }

      bool operator<( const DigestKey& k ) const
      {
         if( k._name != _name ) return _name < k._name;
         return _digest < k._digest;
      }

   protected:
      String       _name;
      SHA1::Digest _digest;

      inline void  digest( const Table& t );
      inline void  digest( const Vec2i& v, const Table* t );
   };

   //------------------------------------------------------------------------------
   //!
   inline TextStream&  operator<<( TextStream& os, const DigestKey& dk )
   {
      return os << dk.name() << ":" << dk.digest().str();
   }

   // Compute the SHA1 digest for the specified Table.
   FUSION_DLL_API void  getDigestPart( const Variant& v, SHA1& sha );
   FUSION_DLL_API void  getDigestPart( const Table& table, SHA1& sha );
   FUSION_DLL_API void  getDigest( const Table& table, SHA1::Digest& digest );
   FUSION_DLL_API void  getDigest( const Vec2i& v, const Table* table, SHA1::Digest& digest );

   inline void DigestKey::digest( const Table& t ) { getDigest(t, _digest); }
   inline void DigestKey::digest( const Vec2i& v, const Table* t ) { getDigest(v, t, _digest); }


   //------------------------------------------------------------------------------
   // Public ResManager Common API
   //------------------------------------------------------------------------------
   FUSION_DLL_API void printInfo( TextStream& os );

   FUSION_DLL_API void clear();

   FUSION_DLL_API String  dir( const String& id );
   FUSION_DLL_API String  expand( const String& dir, const String& id );
   FUSION_DLL_API String  idToPath( const String& id, const char** ext );
   FUSION_DLL_API String  getURLType( const String& str );

   inline TaskQueue*  dispatchQueue();

   //------------------------------------------------------------------------------
   // Private Fusion API
   //------------------------------------------------------------------------------
   void initializeFusion();
   void terminateFusion();
   void clearFusion();
   void fusionGfxManagerChanged();
   void numThreads( uint n );
   extern FUSION_DLL_API TaskQueue*  _dispatchQueue;

   //------------------------------------------------------------------------------
   // Public Fusion API
   //------------------------------------------------------------------------------

   // Image.
   FUSION_DLL_API RCP< Resource<Image> > getImage( const String& id );
   FUSION_DLL_API RCP< Resource<Image> > getImage( const String& id, const Vec2i& size, const Table* args = NULL );
   FUSION_DLL_API String getImageName( const Resource<Image>* );
   FUSION_DLL_API String getImageName( const Image* );

   // ImageCube.
   FUSION_DLL_API RCP< Resource<Image> > getImageCube( const String& id );

   // Font.
   FUSION_DLL_API RCP< Resource<Font> > getFont( const String& id );
   FUSION_DLL_API String getFontName( const Resource<Font>* );
   FUSION_DLL_API String getFontName( const Font* );

   // Script.
   FUSION_DLL_API bool handleRootCandidate( const String& path );
   FUSION_DLL_API void executeScript( VMState* vm, const String& id, bool once = true );
   FUSION_DLL_API void loadScript( VMState* vm, const String& id );

   // Shader.
   FUSION_DLL_API RCP< Resource<Gfx::Shader> > getVertexShader( const String& id );
   FUSION_DLL_API RCP< Resource<Gfx::Shader> > getGeometryShader( const String& id );
   FUSION_DLL_API RCP< Resource<Gfx::Shader> > getFragmentShader( const String& id );
   FUSION_DLL_API RCP< Resource<Gfx::Shader> > getFixedFunctionShader( const String& id );

   // Program.
   FUSION_DLL_API RCP< Resource<Gfx::Program> > getProgram( const String& id );
   FUSION_DLL_API String getProgramName( const Resource<Gfx::Program>* );
   FUSION_DLL_API String getProgramName( const Gfx::Program* );

   // Sound.
   FUSION_DLL_API RCP< Resource<Snd::Sound> > getSound( const String& id );
   FUSION_DLL_API String  getSoundName( const Resource<Snd::Sound>* );
   FUSION_DLL_API String  getSoundName( const Snd::Sound* );

   // Renderable texture.
   FUSION_DLL_API RCP<Gfx::Texture> getRenderableTexture( const String& id );
   FUSION_DLL_API void insertRenderableTexture( const String& id, const RCP<Gfx::Texture>& );

} //namespace ResManager

//------------------------------------------------------------------------------
//!
inline TaskQueue*
ResManager::dispatchQueue()
{
   return _dispatchQueue;
}

NAMESPACE_END

#endif //FUSION_RESMANAGER_H
