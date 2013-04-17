/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef GFX_PROGRAM_H
#define GFX_PROGRAM_H

#include <Gfx/StdDefs.h>

#include <Base/Util/RCObject.h>
#include <Base/Util/RCP.h>
#include <Base/ADT/String.h>
#include <Base/ADT/Vector.h>


NAMESPACE_BEGIN


namespace Gfx
{

/*==============================================================================
   TYPES
==============================================================================*/

enum ShaderType
{
   FIXED_FUNCTION_SHADER,
   VERTEX_SHADER,
   GEOMETRY_SHADER,
   FRAGMENT_SHADER,
   COMPUTE_SHADER
};

enum GSInputType
{
   GS_IN_POINTS,
   GS_IN_LINES,
   GS_IN_LINES_ADJACENCY,
   GS_IN_TRIANGLES,
   GS_IN_TRIANGLES_ADJACENCY,
};

enum GSOutputType
{
   GS_OUT_POINTS,
   GS_OUT_LINE_STRIP,
   GS_OUT_TRIANGLE_STRIP,
};

//------------------------------------------------------------------------------
//!
inline const char*  toStr( ShaderType type )
{
   switch( type )
   {
      case FIXED_FUNCTION_SHADER: return "Fixed-function shader";
      case VERTEX_SHADER        : return "Vertex shader";
      case FRAGMENT_SHADER      : return "Fragment shader";
      case GEOMETRY_SHADER      : return "Geometry shader";
      case COMPUTE_SHADER       : return "Compute shader";
      default                   : return "Unknown shader";
   }
}

//------------------------------------------------------------------------------
//!
inline const char*  toStr( GSInputType type )
{
   switch( type )
   {
      case GS_IN_POINTS             : return "Points";
      case GS_IN_LINES              : return "Lines";
      case GS_IN_LINES_ADJACENCY    : return "Lines adjacency";
      case GS_IN_TRIANGLES          : return "Triangles";
      case GS_IN_TRIANGLES_ADJACENCY: return "Triangles adjacency";
      default                       : return "<invalid>";
   }
}

//------------------------------------------------------------------------------
//!
inline const char*  toStr( GSOutputType type )
{
   switch( type )
   {
      case GS_OUT_POINTS        : return "Points";
      case GS_OUT_LINE_STRIP    : return "Line strip";
      case GS_OUT_TRIANGLE_STRIP: return "Triangle strip";
      default                   : return "<invalid>";
   }
}


/*==============================================================================
   CLASS Shader
==============================================================================*/

class Shader:
   public RCObject
{
public:

   inline ShaderType type() const { return _type; }

protected:


   /*----- methods -----*/

   // Only Managers can create this object.
   GFX_DLL_API Shader( ShaderType type );
   GFX_DLL_API virtual ~Shader();

   ShaderType  _type;
private:

   GFX_MAKE_MANAGERS_FRIENDS();

};

/*==============================================================================
   CLASS Program
==============================================================================*/

class Program:
   public RCObject
{

public:

   /*----- methods -----*/

   GFX_DLL_API virtual void addShader( const RCP<Shader>& );
   GFX_DLL_API virtual bool removeShader( const RCP<Shader>& );

   inline bool  hasGeometryShader() const;
   inline void  setGSParameters( GSInputType inType, GSOutputType outType, uint maxOutVerts );
   inline void  getGSParameters( GSInputType& inType, GSOutputType& outType, uint& maxOutVerts ) const;

protected:

   /*----- methods -----*/

   // Only Managers can create this object.
   Program();
   virtual ~Program();

   /*----- data members -----*/

   Vector< RCP<Shader> > _shaders;
   uint16_t              _parameters;

   //! NAME          SIZE   LOCATION             DESCRIPTION
   //! GS in type     (3)   _parameters[ 2: 0]   The geometry shader input primitive type.
   //! GS out type    (2)   _parameters[ 4: 3]   The geometry shader output primitive type.
   //! GS max verts  (11)   _parameters[15: 5]   The max number of vertices emitted per input primitive.

private:

   GFX_MAKE_MANAGERS_FRIENDS();

};

//-----------------------------------------------------------------------------
//!
inline bool
Program::hasGeometryShader() const
{
   for( auto cur = _shaders.begin(); cur != _shaders.end(); ++cur )
   {
      if( (*cur)->type() == GEOMETRY_SHADER )  return true;
   }
   return false;
}

//-----------------------------------------------------------------------------
//!
inline void
Program::setGSParameters( GSInputType inType, GSOutputType outType, uint maxOutVerts )
{
   uint16_t bits = maxOutVerts & 0x07FF;
   bits <<= 2;
   bits |= outType & 0x03;
   bits <<= 3;
   bits |= inType & 0x07;
   _parameters = bits;
}

//-----------------------------------------------------------------------------
//!
inline void
Program::getGSParameters( GSInputType& inType, GSOutputType& outType, uint& maxOutVerts ) const
{
   uint16_t bits = _parameters;
   inType = (GSInputType)(bits & 0x07);
   bits >>= 3;
   outType = (GSOutputType)(bits & 0x03);
   bits >>= 2;
   maxOutVerts = (bits & 0x07FF);
}


}  //namespace Gfx

NAMESPACE_END


#endif //GFX_PROGRAM_H
