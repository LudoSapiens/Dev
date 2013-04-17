/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef GFX_PASS_H
#define GFX_PASS_H

#include <Gfx/StdDefs.h>

#include <Gfx/Geom/Buffer.h>
#include <Gfx/Prog/Constants.h>
#include <Gfx/FB/Framebuffer.h>
#include <Gfx/Geom/Geometry.h>
#include <Gfx/Prog/Program.h>
#include <Gfx/FB/RenderState.h>
#include <Gfx/Tex/Sampler.h>


#include <Base/ADT/List.h>
#include <Base/Util/RCObject.h>
#include <Base/Util/RCP.h>

NAMESPACE_BEGIN

namespace Gfx
{

/*----- types -----*/

enum ClearType
{
   CLEAR_NONE    = 0x0,
   CLEAR_COLOR   = 0x1,
   CLEAR_DEPTH   = 0x2,
   CLEAR_STENCIL = 0x4,
   CLEAR_ALL     = CLEAR_COLOR | CLEAR_DEPTH | CLEAR_STENCIL
};


// Forward declarations.
class RenderNode;


/*==============================================================================
   CLASS Pass
==============================================================================*/

class Pass:
   public RCObject
{

public:
   /*----- static methods -----*/
   static inline const float* identityMatrix() { return _identityMatrix; }

   /*----- methods -----*/

   GFX_DLL_API Pass();

   GFX_DLL_API virtual ~Pass();

   GFX_DLL_API void clear();

   inline void  exec();

   // Sub-rendering.
   GFX_DLL_API void  render( const RCP<const RenderNode>& rn );

   // Framebuffer.
   inline void  setFramebuffer( const RCP<const Framebuffer>& fb );
   inline const RCP<const Framebuffer>& framebuffer() const;
   inline void  mipmaps( bool );

   // Clearing.
   GFX_DLL_API void setClearColor( float r, float g, float b, float a );
   GFX_DLL_API void setClearDepth( float z );
   GFX_DLL_API void setClearStencil( int s );
   GFX_DLL_API void setClear( ClearType buffer );

   // RenderState.
   inline void  setAlphaState( const RCP<const AlphaState>& state );
   inline void  setColorState( const RCP<const ColorState>& state );
   inline void  setCullState( const RCP<const CullState>& state );
   inline void  setDepthState( const RCP<const DepthState>& state );
   inline void  setOffsetState( const RCP<const OffsetState>& state );
   inline void  setStencilState( const RCP<const StencilState>& state );

   // Rendering fill mode.
   inline void setWireframe( bool );

   // Programs.
   inline void  setProgram( const RCP<const Program>& prog );

   // Constants.
   inline void  setConstants( const RCP<const ConstantList>& constants );

   // Camera.
   inline const float* setCamPosition( const float* position );
   inline const float* setCamPositionPtr( const float* position );

   // Textures.
   inline void  setSamplers( const RCP<const SamplerList>& sl );
   inline void  copyColor( const RCP<const Texture>& dst );
   inline void  copyDepth( const RCP<const Texture>& dst );
   inline void  copyDepthStencil( const RCP<const Texture>& dst );
   inline void  copyStencil( const RCP<const Texture>& dst );

   // Geometry.
   inline void  setGeometry( const RCP<const Geometry>& geom );
   inline void  execGeometry( const RCP<const Geometry>& geom );
   inline void  setRangeGeometry( const RCP<const Geometry>& geom, const uint* range );
   inline void  execRangeGeometry( const RCP<const Geometry>& geom, const uint* range );
   inline void  setRangeGeometryPtr( const RCP<const Geometry>& geom, const uint* range );
   inline void  execRangeGeometryPtr( const RCP<const Geometry>& geom, const uint* range );

   // Scissoring.
   inline const int* setScissor( int x, int y, int width, int height );
   inline const int* setScissor( const int* ptr );
   inline const int* setScissorPtr( const int* ptr );
   inline const int* addScissor( int x, int y, int width, int height );
   inline const int* addScissor( const int* ptr );

   // Matrices.
   inline const int* setViewport( int x, int y, int width, int height );
   inline const int* setViewport( const int* ptr );
   inline const int* setViewportPtr( const int* ptr );
   inline const float* setProjectionMatrix( const float* );
   inline const float* setProjectionMatrixPtr( const float* );
   inline const float* setViewMatrix( const float* );
   inline const float* setViewMatrixPtr( const float* );
   inline const float* setWorldMatrix( const float* );
   inline const float* setWorldMatrixPtr( const float* );

   // Debugging.
   inline void  message( const char* msg );

private:

   GFX_DLL_API static const float _identityMatrix[16];

   GFX_MAKE_MANAGERS_FRIENDS();

   /*----- Types -----*/
   typedef enum
   {
      // Debugging.
      PASS_CMD_NOOP,  //for debugging
      PASS_CMD_MESSAGE,
      // Executing.
      PASS_CMD_EXEC,
      // Sub-rendering.
      PASS_CMD_RENDER,
      // RenderState.
      PASS_CMD_SET_ALPHA_STATE,
      PASS_CMD_SET_COLOR_STATE,
      PASS_CMD_SET_CULL_STATE,
      PASS_CMD_SET_DEPTH_STATE,
      PASS_CMD_SET_OFFSET_STATE,
      PASS_CMD_SET_STENCIL_STATE,
      // Wireframe.
      PASS_CMD_SET_WIREFRAME,
      PASS_CMD_SET_FILL,
      // Programs.
      PASS_CMD_SET_PROGRAM,
      PASS_CMD_SET_PROGRAM_PARAMETER,
      // Constants.
      PASS_CMD_SET_CONSTANTS,
      // Camera.
      PASS_CMD_SET_CAMERA_POSITION,
      // Textures.
      PASS_CMD_SET_SAMPLERS,
      PASS_CMD_COPY_COLOR,
      PASS_CMD_COPY_DEPTH,
      PASS_CMD_COPY_DEPTH_STENCIL,
      PASS_CMD_COPY_STENCIL,
      // Geometry.
      PASS_CMD_SET_GEOMETRY,
      PASS_CMD_EXEC_GEOMETRY,
      PASS_CMD_SET_RANGEGEOMETRY,
      PASS_CMD_EXEC_RANGEGEOMETRY,
      // Scissoring.
      PASS_CMD_SET_SCISSOR,
      // Matrices.
      PASS_CMD_SET_VIEWPORT,
      PASS_CMD_SET_PROJECTION,
      PASS_CMD_SET_VIEW,
      PASS_CMD_SET_WORLD
   } CommandType;

   static inline const char* toStr( CommandType cmd );

   /*----- structures -----*/

   class Command
   {
   public:
      CommandType          _id;
      RCP<const RCObject>  _data;
      union {
         const float*  _fPtr;
         const uint*   _uPtr;
         const int*    _iPtr;
         const char*   _cPtr;
      };
   };

   /*----- Types -----*/

   typedef Vector< Command >                 CommandContainer;
   typedef CommandContainer::iterator        iterator;
   typedef CommandContainer::const_iterator  const_iterator;

   typedef struct { uint _[2]; }             Range;
   typedef struct { int _[4]; }              Rect;
   typedef struct { float _[16]; }           Matrix;
   typedef struct { float _[3]; }            Position;

   // Should not be changed to any other container type than a list!!!
   typedef List<Range>                       RangeContainer;
   typedef List<Rect>                        RectContainer;
   typedef List<Matrix>                      MatrixContainer;
   typedef List<Position>                    PositionContainer;

   /*----- data members -----*/

   int               _size[4];
   const int*        _scissor;
   const int*        _viewport;
   const float*      _camPosition;
   const float*      _projMatrix;
   const float*      _viewMatrix;
   const float*      _worldMatrix;
   float             _color[4];
   float             _z;
   int               _stencil;
   ClearType         _clearBuffers;
   CommandContainer  _commands;
   RangeContainer    _ranges;
   RectContainer     _rects;
   MatrixContainer   _matrices;
   PositionContainer _positions;
   RCP<const Framebuffer>  _framebuffer;
   bool              _fbGenerateMipmaps;

   /*----- methods -----*/
   const uint*  storeRangePtr( const uint* range );
};

//------------------------------------------------------------------------------
//!
inline void
Pass::exec( void )
{
   Command cmd;
   cmd._id = PASS_CMD_EXEC;
   _commands.pushBack(cmd);
}

//------------------------------------------------------------------------------
//!
inline void
Pass::setFramebuffer( const RCP<const Framebuffer>& fb )
{
   _framebuffer = fb;
   if( fb.isValid() )
   {
      _size[2] = fb->width();
      _size[3] = fb->height();
   }
}

//------------------------------------------------------------------------------
//!
inline const RCP<const Gfx::Framebuffer>&
Pass::framebuffer() const
{
   return _framebuffer;
}

//------------------------------------------------------------------------------
//!
inline void
Pass::mipmaps( bool generateMipmaps )
{
   _fbGenerateMipmaps = generateMipmaps;
}

//------------------------------------------------------------------------------
//!
inline void
Pass::setAlphaState( const RCP<const AlphaState>& state )
{
   Command cmd;
   cmd._id   = PASS_CMD_SET_ALPHA_STATE;
   cmd._data = state;
   _commands.pushBack(cmd);
}

//------------------------------------------------------------------------------
//!
inline void
Pass::setColorState( const RCP<const ColorState>& state )
{
   Command cmd;
   cmd._id   = PASS_CMD_SET_COLOR_STATE;
   cmd._data = state;
   _commands.pushBack(cmd);
}

//------------------------------------------------------------------------------
//!
inline void
Pass::setCullState( const RCP<const CullState>& state )
{
   Command cmd;
   cmd._id   = PASS_CMD_SET_CULL_STATE;
   cmd._data = state;
   _commands.pushBack(cmd);
}

//------------------------------------------------------------------------------
//!
inline void
Pass::setDepthState( const RCP<const DepthState>& state )
{
   Command cmd;
   cmd._id   = PASS_CMD_SET_DEPTH_STATE;
   cmd._data = state;
   _commands.pushBack(cmd);
}

//------------------------------------------------------------------------------
//!
inline void
Pass::setOffsetState( const RCP<const OffsetState>& state )
{
   Command cmd;
   cmd._id   = PASS_CMD_SET_OFFSET_STATE;
   cmd._data = state;
   _commands.pushBack(cmd);
}

//------------------------------------------------------------------------------
//!
inline void
Pass::setStencilState( const RCP<const StencilState>& state )
{
   Command cmd;
   cmd._id   = PASS_CMD_SET_STENCIL_STATE;
   cmd._data = state;
   _commands.pushBack(cmd);
}

//------------------------------------------------------------------------------
//!
inline void
Pass::setWireframe( bool mode )
{
   Command cmd;
   cmd._id = mode ? PASS_CMD_SET_WIREFRAME : PASS_CMD_SET_FILL;
   _commands.pushBack(cmd);
}

//------------------------------------------------------------------------------
//!
inline void
Pass::setProgram( const RCP<const Program>& prog )
{
   Command cmd;
   cmd._id   = PASS_CMD_SET_PROGRAM;
   cmd._data = prog;
   _commands.pushBack(cmd);
}

//------------------------------------------------------------------------------
//!
inline void
Pass::setConstants( const RCP<const ConstantList>& constants )
{
   Command cmd;
   cmd._id   = PASS_CMD_SET_CONSTANTS;
   cmd._data = constants;
   _commands.pushBack( cmd );
}

//------------------------------------------------------------------------------
//!
inline const float*
Pass::setCamPosition( const float* position )
{
   _positions.pushBack( *(const Position*)position );
   const float* tmp  = _camPosition;
   _camPosition = (const float*)&_positions.back();

   Command cmd;
   cmd._id  = PASS_CMD_SET_CAMERA_POSITION;
   cmd._fPtr = _camPosition;
   _commands.pushBack(cmd);

   return tmp;
}

//------------------------------------------------------------------------------
//!
inline const float*
Pass::setCamPositionPtr( const float* position )
{
   Command cmd;
   cmd._id  = PASS_CMD_SET_CAMERA_POSITION;
   cmd._fPtr = position;
   _commands.pushBack(cmd);

   const float* tmp  = _camPosition;
   _camPosition = position;
   return tmp;
}

//------------------------------------------------------------------------------
//!
inline void
Pass::setSamplers( const RCP<const SamplerList>& sl )
{
   Command cmd;
   cmd._id   = PASS_CMD_SET_SAMPLERS;
   cmd._data = sl;
   _commands.pushBack(cmd);
}

//------------------------------------------------------------------------------
//!
inline void
Pass::copyColor( const RCP<const Texture>& dst )
{
   Command cmd;
   cmd._id   = PASS_CMD_COPY_COLOR;
   cmd._data = dst;
   _commands.pushBack(cmd);
}

//------------------------------------------------------------------------------
//!
inline void
Pass::copyDepth( const RCP<const Texture>& dst )
{
   Command cmd;
   cmd._id   = PASS_CMD_COPY_DEPTH;
   cmd._data = dst;
   _commands.pushBack(cmd);
}

//------------------------------------------------------------------------------
//!
inline void
Pass::copyDepthStencil( const RCP<const Texture>& dst )
{
   Command cmd;
   cmd._id   = PASS_CMD_COPY_DEPTH_STENCIL;
   cmd._data = dst;
   _commands.pushBack(cmd);
}

//------------------------------------------------------------------------------
//!
inline void
Pass::copyStencil( const RCP<const Texture>& dst )
{
   Command cmd;
   cmd._id   = PASS_CMD_COPY_STENCIL;
   cmd._data = dst;
   _commands.pushBack(cmd);
}

//------------------------------------------------------------------------------
//!
inline void
Pass::setGeometry( const RCP<const Geometry>& geom )
{
   Command cmd;
   cmd._id   = PASS_CMD_SET_GEOMETRY;
   cmd._data = geom;
   _commands.pushBack(cmd);
}

//------------------------------------------------------------------------------
//!
inline void
Pass::execGeometry( const RCP<const Geometry>& geom )
{
   Command cmd;
   cmd._id   = PASS_CMD_EXEC_GEOMETRY;
   cmd._data = geom;
   _commands.pushBack(cmd);
}

//------------------------------------------------------------------------------
//!
inline void
Pass::setRangeGeometry( const RCP<const Geometry>& geom, const uint* range )
{
   range = storeRangePtr(range);

   Command cmd;
   cmd._id   = PASS_CMD_SET_RANGEGEOMETRY;
   cmd._data = geom;
   cmd._uPtr = range;
   _commands.pushBack(cmd);
}

//------------------------------------------------------------------------------
//!
inline void
Pass::execRangeGeometry( const RCP<const Geometry>& geom, const uint* range )
{
   range = storeRangePtr(range);

   Command cmd;
   cmd._id   = PASS_CMD_EXEC_RANGEGEOMETRY;
   cmd._data = geom;
   cmd._uPtr = range;
   _commands.pushBack(cmd);
}

//------------------------------------------------------------------------------
//!
inline void
Pass::setRangeGeometryPtr( const RCP<const Geometry>& geom, const uint* range )
{
   Command cmd;
   cmd._id   = PASS_CMD_SET_RANGEGEOMETRY;
   cmd._data = geom;
   cmd._uPtr = range;
   _commands.pushBack(cmd);
}

//------------------------------------------------------------------------------
//!
inline void
Pass::execRangeGeometryPtr( const RCP<const Geometry>& geom, const uint* range )
{
   Command cmd;
   cmd._id   = PASS_CMD_EXEC_RANGEGEOMETRY;
   cmd._data = geom;
   cmd._uPtr = range;
   _commands.pushBack(cmd);
}

//------------------------------------------------------------------------------
//!
inline const int*
Pass::setScissor( int x, int y, int width, int height )
{
   Rect r;
   r._[0] = x;
   r._[1] = y;
   r._[2] = width;
   r._[3] = height;
   _rects.pushBack( r );

   const int* tmp = _scissor;
   _scissor = (const int*)&_rects.back();

   Command cmd;
   cmd._id   = PASS_CMD_SET_SCISSOR;
   cmd._iPtr = _scissor;
   _commands.pushBack(cmd);

   return tmp;
}

//------------------------------------------------------------------------------
//!
inline const int*
Pass::setScissor( const int* ptr )
{
   Rect r;
   r._[0] = ptr[0];
   r._[1] = ptr[1];
   r._[2] = ptr[2];
   r._[3] = ptr[3];
   _rects.pushBack( r );

   const int* tmp = _scissor;
   _scissor = (const int*)&_rects.back();

   Command cmd;
   cmd._id   = PASS_CMD_SET_SCISSOR;
   cmd._iPtr = _scissor;
   _commands.pushBack(cmd);

   return tmp;
}

//------------------------------------------------------------------------------
//!
inline const int*
Pass::setScissorPtr( const int* ptr )
{
   const int* tmp = _scissor;
   _scissor = ptr;

   Command cmd;
   cmd._id   = PASS_CMD_SET_SCISSOR;
   cmd._iPtr = _scissor;
   _commands.pushBack(cmd);

   return tmp;
}

//------------------------------------------------------------------------------
//!
inline const int*
Pass::addScissor( int x, int y, int width, int height )
{
   int oldR = _scissor[0] + _scissor[2];
   int oldB = _scissor[1] + _scissor[3];
   int newR = x + width;
   int newB = y + height;
   int finalR = newR < oldR ? newR : oldR;
   int finalB = newB < oldB ? newB : oldB;
   Rect r;
   r._[0] = x < _scissor[0] ? _scissor[0] : x;
   r._[1] = y < _scissor[1] ? _scissor[1] : y;
   r._[2] = r._[0] < finalR ? finalR - r._[0] : 0;
   r._[3] = r._[1] < finalB ? finalB - r._[1] : 0;
   _rects.pushBack( r );

   const int* tmp = _scissor;
   _scissor = (const int*)&_rects.back();

   Command cmd;
   cmd._id   = PASS_CMD_SET_SCISSOR;
   cmd._iPtr = _scissor;
   _commands.pushBack(cmd);

   return tmp;
}

//------------------------------------------------------------------------------
//!
inline const int*
Pass::addScissor( const int* ptr )
{
   int oldR = _scissor[0] + _scissor[2];
   int oldB = _scissor[1] + _scissor[3];
   int newR = ptr[0] + ptr[2];
   int newB = ptr[1] + ptr[3];
   int finalR = newR < oldR ? newR : oldR;
   int finalB = newB < oldB ? newB : oldB;
   Rect r;
   r._[0] = ptr[0] < _scissor[0] ? _scissor[0] : ptr[0];
   r._[1] = ptr[1] < _scissor[1] ? _scissor[1] : ptr[1];
   r._[2] = r._[0] < finalR ? finalR - r._[0] : 0;
   r._[3] = r._[1] < finalB ? finalB - r._[1] : 0;
   _rects.pushBack( r );

   const int* tmp = _scissor;
   _scissor = (const int*)&_rects.back();

   Command cmd;
   cmd._id   = PASS_CMD_SET_SCISSOR;
   cmd._iPtr = _scissor;
   _commands.pushBack(cmd);

   return tmp;
}

//------------------------------------------------------------------------------
//!
inline const int*
Pass::setViewport( int x, int y, int width, int height )
{
   Rect r;
   r._[0] = x;
   r._[1] = y;
   r._[2] = width;
   r._[3] = height;
   _rects.pushBack( r );

   const int* tmp = _viewport;
   _viewport = (const int*)&_rects.back();

   Command cmd;
   cmd._id   = PASS_CMD_SET_VIEWPORT;
   cmd._iPtr = _viewport;
   _commands.pushBack(cmd);

   return tmp;
}

//------------------------------------------------------------------------------
//!
inline const int*
Pass::setViewport( const int* ptr )
{
   Rect r;
   r._[0] = ptr[0];
   r._[1] = ptr[1];
   r._[2] = ptr[2];
   r._[3] = ptr[3];
   _rects.pushBack( r );

   const int* tmp = _viewport;
   _viewport = (const int*)&_rects.back();

   Command cmd;
   cmd._id   = PASS_CMD_SET_VIEWPORT;
   cmd._iPtr = _viewport;
   _commands.pushBack(cmd);

   return tmp;
}

//------------------------------------------------------------------------------
//!
inline const int*
Pass::setViewportPtr( const int* ptr )
{
   const int* tmp = _viewport;
   _viewport = ptr;

   Command cmd;
   cmd._id   = PASS_CMD_SET_VIEWPORT;
   cmd._iPtr = _viewport;
   _commands.pushBack(cmd);

   return tmp;
}

//------------------------------------------------------------------------------
//!
inline const float*
Pass::setProjectionMatrix( const float* matrix )
{
   _matrices.pushBack( *(const Matrix*)matrix );
   const float* tmp  = _projMatrix;
   _projMatrix = (const float*)&_matrices.back();

   Command cmd;
   cmd._id  = PASS_CMD_SET_PROJECTION;
   cmd._fPtr = _projMatrix;
   _commands.pushBack(cmd);

   return tmp;
}

//------------------------------------------------------------------------------
//!
inline const float*
Pass::setProjectionMatrixPtr( const float* matrix )
{
   Command cmd;
   cmd._id  = PASS_CMD_SET_PROJECTION;
   cmd._fPtr = matrix;
   _commands.pushBack(cmd);

   const float* tmp  = _projMatrix;
   _projMatrix = matrix;
   return tmp;
}

//------------------------------------------------------------------------------
//!
inline const float*
Pass::setViewMatrix( const float* matrix )
{
   _matrices.pushBack( *(Matrix*)matrix );
   const float* tmp  = _viewMatrix;
   _viewMatrix = (const float*)&_matrices.back();

   Command cmd;
   cmd._id  = PASS_CMD_SET_VIEW;
   cmd._fPtr = _viewMatrix;
   _commands.pushBack(cmd);

   return tmp;
}

//------------------------------------------------------------------------------
//!
inline const float*
Pass::setViewMatrixPtr( const float* matrix )
{
   Command cmd;
   cmd._id  = PASS_CMD_SET_VIEW;
   cmd._fPtr = matrix;
   _commands.pushBack(cmd);

   const float* tmp  = _viewMatrix;
   _viewMatrix = matrix;
   return tmp;
}

//------------------------------------------------------------------------------
//!
inline const float*
Pass::setWorldMatrix( const float* matrix )
{
   _matrices.pushBack( *(Matrix*)matrix );
   const float* tmp   = _worldMatrix;
   _worldMatrix = (const float*)&_matrices.back();

   Command cmd;
   cmd._id  = PASS_CMD_SET_WORLD;
   cmd._fPtr = _worldMatrix;
   _commands.pushBack(cmd);

   return tmp;
}

//------------------------------------------------------------------------------
//!
inline const float*
Pass::setWorldMatrixPtr( const float* matrix )
{
   Command cmd;
   cmd._id  = PASS_CMD_SET_WORLD;
   cmd._fPtr = matrix;
   _commands.pushBack(cmd);

   const float* tmp   = _worldMatrix;
   _worldMatrix = matrix;
   return tmp;
}

//------------------------------------------------------------------------------
//!
inline const uint*
Pass::storeRangePtr( const uint* range )
{
   if( range != NULL )
   {
      Range r;
      r._[0] = range[0];
      r._[1] = range[1];
      _ranges.pushBack(r);

      range = (const uint*)&_ranges.back();
   }

   return range;
}

//------------------------------------------------------------------------------
//!
inline void
Pass::message( const char* msg )
{
   Command cmd;
   cmd._id  = PASS_CMD_MESSAGE;
   cmd._cPtr = msg;
   _commands.pushBack(cmd);
}

//------------------------------------------------------------------------------
//!
inline const char*
Pass::toStr( Pass::CommandType cmd )
{
   switch( cmd )
   {
      case Pass::PASS_CMD_NOOP                 : return "NOOP";
      case Pass::PASS_CMD_MESSAGE              : return "MESSAGE";
      case Pass::PASS_CMD_EXEC                 : return "EXEC";
      case Pass::PASS_CMD_RENDER               : return "RENDER";
      case Pass::PASS_CMD_SET_ALPHA_STATE      : return "SET_ALPHA_STATE";
      case Pass::PASS_CMD_SET_COLOR_STATE      : return "SET_COLOR_STATE";
      case Pass::PASS_CMD_SET_CULL_STATE       : return "SET_CULL_STATE";
      case Pass::PASS_CMD_SET_DEPTH_STATE      : return "SET_DEPTH_STATE";
      case Pass::PASS_CMD_SET_STENCIL_STATE    : return "SET_STENCIL_STATE";
      case Pass::PASS_CMD_SET_WIREFRAME        : return "SET_WIREFRAME";
      case Pass::PASS_CMD_SET_FILL             : return "SET_FILL";
      case Pass::PASS_CMD_SET_PROGRAM          : return "SET_PROGRAM";
      case Pass::PASS_CMD_SET_PROGRAM_PARAMETER: return "SET_PROGRAM_PARAMETER";
      case Pass::PASS_CMD_SET_CONSTANTS        : return "SET_CONSTANTS";
      case Pass::PASS_CMD_SET_CAMERA_POSITION  : return "SET_CAMERA_POSITION";
      case Pass::PASS_CMD_SET_SAMPLERS         : return "SET_SAMPLERS";
      case Pass::PASS_CMD_COPY_COLOR           : return "COPY_COLOR";
      case Pass::PASS_CMD_COPY_DEPTH           : return "COPY_DEPTH";
      case Pass::PASS_CMD_COPY_DEPTH_STENCIL   : return "COPY_DEPTH_STENCIL";
      case Pass::PASS_CMD_COPY_STENCIL         : return "COPY_STENCIL";
      case Pass::PASS_CMD_SET_GEOMETRY         : return "SET_GEOMETRY";
      case Pass::PASS_CMD_EXEC_GEOMETRY        : return "EXEC_GEOMETRY";
      case Pass::PASS_CMD_SET_RANGEGEOMETRY    : return "SET_RANGEGEOMETRY";
      case Pass::PASS_CMD_EXEC_RANGEGEOMETRY   : return "EXEC_RANGEGEOMETRY";
      case Pass::PASS_CMD_SET_SCISSOR          : return "SET_SCISSOR";
      case Pass::PASS_CMD_SET_VIEWPORT         : return "SET_VIEWPORT";
      case Pass::PASS_CMD_SET_PROJECTION       : return "SET_PROJECTION";
      case Pass::PASS_CMD_SET_VIEW             : return "SET_VIEW";
      case Pass::PASS_CMD_SET_WORLD            : return "SET_WORLD";
      default                                  : return "<unknown>";
   }
}

} //namespace Gfx

NAMESPACE_END


#endif //GFX_PASS_H
