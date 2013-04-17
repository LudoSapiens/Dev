/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Gfx/Mgr/Context.h>

#include <Base/Dbg/DebugStream.h>


//------------------------
// Null configuration
//------------------------
#if GFX_NULL_SUPPORT
#  include <Gfx/Mgr/Null/NullContext.h>
//#  if _MSC_VER
//#    pragma message("Adding Null support")
//#  else
//#    warning "Adding Null support"
//#  endif
#endif //GFX_NULL_SUPPORT
//------------------------


//------------------------
// Direct3D configuration
//------------------------
#if GFX_D3D_SUPPORT
#  include <Gfx/Mgr/D3D/D3DContext.h>
//#  if _MSC_VER
//#    pragma message("Adding Direct3D support")
//#  else
//#    warning "Adding Direct3D support"
//#  endif
#endif //GFX_D3D_SUPPORT
//------------------------


//----------------------
// OpenGL configuration
//----------------------
#if GFX_OGL_SUPPORT
#include <Gfx/Mgr/GL/GLContext.h>
//#  if _MSC_VER
//#    pragma message("Adding OpenGL support")
//#  else
//#    warning "Adding OpenGL support"
//#  endif

//Support for various contexts
#if GFX_OGL_GLX_SUPPORT
#  include <Gfx/Mgr/GL/GLContext_GLX.h>
//#  if _MSC_VER
//#    pragma message("Adding GLX support")
//#  else
//#    warning "Adding GLX support"
//#  endif
#endif
#if GFX_OGL_WGL_SUPPORT
#  include <Gfx/Mgr/GL/GLContext_WGL.h>
//#  if _MSC_VER
//#    pragma message("Adding WGL support")
//#  else
//#    warning "Adding WGL support"
//#  endif
#endif

#endif //GFX_OGL_SUPPORT
//------------------------

//------------------------
// OpenGL ES configuration
//------------------------
#if GFX_GLES_1_SUPPORT || GFX_GLES_2_SUPPORT
#  include <Gfx/Mgr/GLES/GLESContext.h>
//#  if _MSC_VER
//#    pragma message("Adding OpenGL ES support")
//#  else
//#    warning "Adding OpenGL ES support"
//#  endif

//Support for various contexts
#if GFX_GLES_EGL_SUPPORT
#  include <Gfx/Mgr/GLES/GLESContext_EGL.h>
#endif

#endif //GFX_GLES_*_SUPPORT
//------------------------


USING_NAMESPACE

using namespace Gfx;

/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN

DBG_STREAM( os_cntx, "Context" );

UNNAMESPACE_END


/*==============================================================================
   CLASS Context
==============================================================================*/

//------------------------------------------------------------------------------
//!
Context::Context
( void )
{
   DBG_BLOCK( os_cntx, "Context::Context()" );
}

//------------------------------------------------------------------------------
//!
Context::~Context
( void )
{
   DBG_BLOCK( os_cntx, "Context::~Context()" );
}

//------------------------------------------------------------------------------
//!
RCP<Context>
Context::getDefaultContext( const String& preferredAPI, void* window )
{
   unused( window );
   DBG_BLOCK( os_cntx, "Context::getDefaultContext(" << preferredAPI << ", " << window << ")" );

   String api_name_lower;

   const char* mgr_str = getenv("GFX_MGR");
   if( mgr_str == NULL )
   {
      api_name_lower = preferredAPI.lower();
   }
   else
   {
      api_name_lower = mgr_str;
      api_name_lower = api_name_lower.lower();
   }

   printf("Creating graphics manager using: %s\n", api_name_lower.cstr());

#if GFX_NULL_SUPPORT
   if( api_name_lower.empty() ||
       api_name_lower == "null" )
   {
      return new NullContext();
   }
   else
#endif //GFX_NULL_SUPPORT
#if GFX_GLES_1_SUPPORT || GFX_GLES_2_SUPPORT
   if( api_name_lower.startsWith("gles") ||
       api_name_lower.startsWith("opengles") ||
       api_name_lower.startsWith("opengl es") ||
       api_name_lower.startsWith("opengl-es") )
   {
#if   GFX_GLES_EGL_SUPPORT
      return new GLESContext_EGL( EGL_DEFAULT_DISPLAY, (EGLNativeWindowType)window );
#else
      printf("GLES doesn't support default context; using null.\n");
      return new NullContext();
#endif
   }
   else
#endif //GFX_GLES_*_SUPPORT
#if GFX_OGL_SUPPORT
   if( api_name_lower.empty() ||
       api_name_lower == "opengl" || api_name_lower == "opengl:sw" ||
       api_name_lower == "ogl" || api_name_lower == "ogl:sw" )
   {
#if   GFX_OGL_GLX_SUPPORT
      return new GLContext_GLX( (GLContext_GLX::WindowStructure*)window );
#elif GFX_OGL_WGL_SUPPORT
      return new GLContext_WGL( (HWND)window );
#else
      return new NullContext();
#endif
   }
   else
#endif //GFX_OGL_SUPPORT
#if GFX_D3D_SUPPORT
   if( api_name_lower.empty() ||
       api_name_lower == "direct3d" ||
       api_name_lower == "directx" ||
       api_name_lower == "d3d" )
   {
      return new D3DContext( (HWND)window );
   }
   else
#endif //GFX_D3D_SUPPORT
   {
      return new NullContext();
   }
}
