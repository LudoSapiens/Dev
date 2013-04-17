/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_PROBE_H
#define PLASMA_PROBE_H

#include <Plasma/StdDefs.h>

#include <Fusion/Resource/Image.h>

#include <CGMath/Vec3.h>

#include <Base/ADT/String.h>
#include <Base/Util/RCObject.h>
#include <Base/Util/RCP.h>

NAMESPACE_BEGIN

/*==============================================================================
  CLASS Probe
==============================================================================*/
class Probe:
   public RCObject
{
public:

   /*----- types -----*/

   enum Type
   {
      CUBEMAP,
   };

   /*----- methods -----*/

   virtual ~Probe() {}

   Type  type() const { return _type; }

   const String&  id() const { return _id; }

   const Vec3f&  position() const { return _pos; }

   PLASMA_DLL_API virtual RCP<Probe>  clone() const = 0;

protected:

   /*----- data members -----*/

   Type    _type;
   String  _id;
   Vec3f   _pos;

   /*----- methods -----*/

   Probe( Type type, const String& id, const Vec3f& pos ):
      _type( type ), _id( id ), _pos( pos ) {}

private:
}; //class Probe

//------------------------------------------------------------------------------
//!
inline const char*  toStr( Probe::Type t )
{
   switch( t )
   {
      case Probe::CUBEMAP:  return "cubemap";
      default            :  return "<unknown>";
   }
}

//------------------------------------------------------------------------------
//!
inline Probe::Type  toProbeType( const char* /*s*/ )
{
   return Probe::CUBEMAP;
}


/*==============================================================================
  CLASS CubemapProbe
==============================================================================*/
class CubemapProbe:
   public Probe
{
public:

   /*----- methods -----*/

   PLASMA_DLL_API CubemapProbe( const String& id, const Vec3f& pos, Image* img );

   PLASMA_DLL_API virtual ~CubemapProbe();

   const RCP<Image>&  image() const { return _image; }

   PLASMA_DLL_API virtual RCP<Probe>  clone() const;

protected:

   /*----- data members -----*/

   RCP<Image>  _image;

private:
}; //class CubemapProbe


NAMESPACE_END

#endif //PLASMA_PROBE_H
