/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Gfx/Tex/Sampler.h>

#include <algorithm>


USING_NAMESPACE

using namespace Gfx;


UNNAMESPACE_BEGIN

/*==============================================================================
   CLASS SamplerNamesEqual
==============================================================================*/

//! A predicate class for STL's remove_if call
class SamplerNamesEqual
{
public:
   const ConstString& _name;
   SamplerNamesEqual( const ConstString& name ): _name(name) { }

   bool operator()( const RCP<Sampler>& sampler ) { return sampler->name() == _name; }
};

UNNAMESPACE_END


/*==============================================================================
   CLASS Sampler
==============================================================================*/



/*==============================================================================
   CLASS SamplerList
==============================================================================*/

//------------------------------------------------------------------------------
//!
RCP<SamplerList>
SamplerList::create( const RCP<Sampler>& sampler )
{
   RCP<SamplerList> sl = new SamplerList();
   sl->addSampler( sampler );
   return sl;
}

//------------------------------------------------------------------------------
//!
SamplerList::SamplerList()
{
}

//------------------------------------------------------------------------------
//!
SamplerList::~SamplerList()
{
}

//------------------------------------------------------------------------------
//!
void
SamplerList::setSampler( const RCP<Sampler>& sampler )
{
   uint i = getSamplerID( sampler->name() );
   if( i != (uint)-1 )
   {
      _samplers[i] = sampler;
   }
   else
   {
      _samplers.pushBack( sampler );
   }
}

//------------------------------------------------------------------------------
//!
bool
SamplerList::removeSampler( const ConstString& name )
{
   SamplerNamesEqual names_equal(name);
   ContainerType::Iterator newEnd = std::remove_if(
      _samplers.begin(), _samplers.end(), names_equal
   );

   if( newEnd != _samplers.end() )
   {
      _samplers.erase( newEnd, _samplers.end() );
      return true;
   }
   return false;
}

//------------------------------------------------------------------------------
//!
Sampler*
SamplerList::getSampler( const ConstString& name )
{
   for( uint i = 0; i < _samplers.size(); ++i )
   {
      Sampler* s = _samplers[i].ptr();
      if( s->name() == name ) return s;
   }
   return NULL;
}

//------------------------------------------------------------------------------
//!
const Sampler*
SamplerList::getSampler( const ConstString& name ) const
{
   for( uint i = 0; i < _samplers.size(); ++i )
   {
      const Sampler* s = _samplers[i].ptr();
      if( s->name() == name ) return s;
   }
   return NULL;
}

//------------------------------------------------------------------------------
//! Returns the index of the sampler, or (uint)-1 if the name isn't found.
uint
SamplerList::getSamplerID( const ConstString& name ) const
{
   for( uint i = 0; i < _samplers.size(); ++i )
   {
      if( _samplers[i]->name() == name ) return i;
   }
   return (uint)-1;
}

//------------------------------------------------------------------------------
//!
void
SamplerList::print()
const
{
   printf("Samplers (%d)\n", (uint)_samplers.size());
   for( uint i = 0; i < _samplers.size(); ++i )
   {
      printf("%02d: %s\n", i, _samplers[i]->name().cstr());
   }
}
