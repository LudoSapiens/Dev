/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/World/Probe.h>

#include <Fusion/Resource/ResManager.h>

USING_NAMESPACE

/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN



UNNAMESPACE_END


/*==============================================================================
  CLASS CubemapProbe
==============================================================================*/

//------------------------------------------------------------------------------
//!
CubemapProbe::CubemapProbe( const String& id, const Vec3f& pos, Image* img ):
   Probe( CUBEMAP, id, pos ),
   _image( img )
{
   if( _image.isNull() )  _image = data( ResManager::getImageCube( id ) );
}


//------------------------------------------------------------------------------
//!
CubemapProbe::~CubemapProbe()
{
}

//------------------------------------------------------------------------------
//!
RCP<Probe>
CubemapProbe::clone() const
{
   return new CubemapProbe( _id, _pos, _image.ptr() );
}
