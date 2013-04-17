/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/Renderable/BasicsRenderable.h>


/*==============================================================================
  CLASS 
==============================================================================*/


NAMESPACE_BEGIN

//------------------------------------------------------------------------------
//!
GridRenderable::GridRenderable()
   : _qty( 10 ),
     _space( 1 )
{}

//------------------------------------------------------------------------------
//!
GridRenderable::~GridRenderable()
{}

//------------------------------------------------------------------------------
//!
void
GridRenderable::render( const RCP<Gfx::Pass>& ) const
{
#if GFX   
   GfxState::set( TextureObject() );
   
   GL::color4f( 0.1f, 0.1f, 0.1f, 1.0f );
   
   int i;
   int qty = _qty >> 1;
   
   GL::begin( GL::LINES );
   {      
      for( i = -qty; i <= qty; i++ )
      {
         GL::vertex3f( i*_space, 0, -qty*_space );
         GL::vertex3f( i*_space, 0,  qty*_space );
      }
   }
   GL::end();
      
   GL::begin( GL::LINES );
   {
      for( i = -qty; i <= qty; i++ )
      {
         GL::vertex3f( -qty*_space, 0, i*_space );
         GL::vertex3f(  qty*_space, 0, i*_space );
      }
   }
   GL::end();
#endif   
}


NAMESPACE_END
