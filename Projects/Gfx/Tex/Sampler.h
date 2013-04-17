/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef GFX_SAMPLER_H
#define GFX_SAMPLER_H

#include <Gfx/StdDefs.h>
#include <Gfx/Tex/Texture.h>
#include <Gfx/Tex/TextureState.h>

#include <Base/ADT/ConstString.h>
#include <Base/Util/RCObject.h>
#include <Base/Util/RCP.h>
#include <Base/ADT/Vector.h>


NAMESPACE_BEGIN

namespace Gfx
{

/*==============================================================================
   CLASS Sampler
==============================================================================*/
class Sampler:
   public RCObject
{
public:

   /*----- methods -----*/

   Sampler() { }
   Sampler( const ConstString& name, const RCP<Texture>& texture, const TextureState& state ):
      _name(name), _texture(texture), _state(state) { }

   void  name( const ConstString& name )        { _name = name; }
   const ConstString&  name() const             { return _name; }

   void  texture( const RCP<Texture>& texture ) { _texture = texture; }
         RCP<Texture>&  texture()               { return _texture; }
   const RCP<Texture>&  texture() const         { return _texture; }

   void  state( const TextureState& state )     { _state = state; }
         TextureState&  state()                 { return _state; }
   const TextureState&  state() const           { return _state; }

protected:

   /*----- data members -----*/

   ConstString   _name;
   RCP<Texture>  _texture;
   TextureState  _state;
};

/*==============================================================================
   CLASS SamplerList
==============================================================================*/
class SamplerList:
   public RCObject
{
public:
   /*----- types -----*/

   typedef Vector< RCP<Sampler> >  ContainerType;

   /*----- static methods -----*/

   static GFX_DLL_API RCP<SamplerList> create( const RCP<Sampler>& sampler );

   /*----- methods -----*/

   GFX_DLL_API SamplerList();

   // Creation.
   inline void reserve( uint i ) { _samplers.reserve(i); }

   GFX_DLL_API void setSampler( const RCP<Sampler>& sampler );
   inline void addSamplers( const RCP<SamplerList>& samplerList );
   inline void addSampler( const RCP<Sampler>& sampler );
   inline void addSampler(
      const ConstString&   name,
      const RCP<Texture>&  texture,
      const TextureState&  state
   );
   inline void setSampler(
      const ConstString&   name,
      const RCP<Texture>&  texture,
      const TextureState&  state
   );
   inline bool removeSampler( const RCP<Sampler>& sampler );
   GFX_DLL_API bool removeSampler( const ConstString& name );

   // Accessors.
   GFX_DLL_API       Sampler* getSampler( const ConstString& name );
   GFX_DLL_API const Sampler* getSampler( const ConstString& name ) const;
   GFX_DLL_API uint getSamplerID( const ConstString& name ) const;

         ContainerType&  samplers()       { return _samplers; }
   const ContainerType&  samplers() const { return _samplers; }

   inline uint size() const                              { return (uint)_samplers.size(); }
   inline const RCP<Sampler>& operator[]( uint i ) const { return _samplers[i]; }
   inline void clear()                                   { _samplers.clear(); }

   GFX_DLL_API void print() const;

protected:

   /*----- methods -----*/

   GFX_DLL_API virtual ~SamplerList();

   /*----- data members -----*/

   ContainerType  _samplers;

private:

   GFX_MAKE_MANAGERS_FRIENDS();

}; //class SamplerList

//------------------------------------------------------------------------------
//! 
inline void
SamplerList::addSampler( const RCP<Sampler>& sampler )
{
   _samplers.pushBack( sampler );
}

//------------------------------------------------------------------------------
//! 
inline bool
SamplerList::removeSampler( const RCP<Sampler>& sampler )
{
   return _samplers.remove( sampler );
}

//------------------------------------------------------------------------------
//!
inline void
SamplerList::addSamplers( const RCP<SamplerList>& samplerList )
{
   ContainerType::ConstIterator cur = samplerList->samplers().begin();
   ContainerType::ConstIterator end = samplerList->samplers().end();
   while( cur != end )
   {
      addSampler( *cur );
      ++cur;
   }
}

//------------------------------------------------------------------------------
//!
inline void
SamplerList::addSampler(
   const ConstString&   name,
   const RCP<Texture>&  texture,
   const TextureState&  state
)
{
   addSampler( RCP<Sampler>(new Sampler(name, texture, state)) );
}

//------------------------------------------------------------------------------
//!
inline void
SamplerList::setSampler(
   const ConstString&   name,
   const RCP<Texture>&  texture,
   const TextureState&  state
)
{
   setSampler( RCP<Sampler>(new Sampler(name, texture, state)) );
}


} //namespace Gfx

NAMESPACE_END


#endif //GFX_SAMPLER_H
