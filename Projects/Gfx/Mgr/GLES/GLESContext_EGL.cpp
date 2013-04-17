/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Gfx/Mgr/GLES/GLESContext_EGL.h>

#if GFX_GLES_EGL_SUPPORT

#include <Base/Dbg/DebugStream.h>
#include <Base/Util/Platform.h>

#if PLAT_WINDOWS
#define snprintf _snprintf
#endif

USING_NAMESPACE

using namespace Gfx;

/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN

//------------------------------------------------------------------------------
//!
inline const char* gluErrorString( GLenum err )
{
   static char tmp[1024];
   const char* errStr;
   switch( err )
   {
      case GL_NO_ERROR         : errStr = "<none>"; break;
      case GL_INVALID_ENUM     : errStr = "INVALID ENUM"; break;
      case GL_INVALID_VALUE    : errStr = "INVALID VALUE"; break;
      case GL_INVALID_OPERATION: errStr = "INVALID OPERATION"; break;
      case GL_OUT_OF_MEMORY    : errStr = "OUT OF MEMORY"; break;
      default                  : errStr = "<unknown>"; break;
   }
   snprintf( tmp, 1024, "0x%04x (%s)", err, errStr );
   return tmp;
}

//------------------------------------------------------------------------------
//!
inline void displayGLError(const char* fmt, GLenum err)
{
   printf(fmt, gluErrorString(err));
}

//------------------------------------------------------------------------------
//!
bool  checkForErrors( const char* fmt )
{
   GLenum err = glGetError();

   if( err == GL_NO_ERROR )
   {
      return false;
   }
   else
   {
      while( err != GL_NO_ERROR )
      {
         displayGLError(fmt, err);
         CHECK( false );
         err = glGetError();
      }
      return true;
   }
}


UNNAMESPACE_END


//------------------------------------------------------------------------------
//!
GLESContext_EGL::GLESContext_EGL(
   EGLNativeDisplayType nativeDisplay,
   EGLNativeWindowType  nativeWindow
):
   _nativeDisplay( nativeDisplay ),
   _nativeWindow( nativeWindow ),
   _eglDisplay( EGL_NO_DISPLAY ),
   _eglSurface( EGL_NO_SURFACE ),
   _eglContext( EGL_NO_CONTEXT )

{
   // Get the display handle.
   _eglDisplay = eglGetDisplay( _nativeDisplay );
   if( _eglDisplay == EGL_NO_DISPLAY )
   {
      StdErr << "ERROR - GLESContext_EGL() could not get display." << nl;
      return;
   }
   if( checkForErrors( "ERROR - GLESContext_EGL() display: '%s'\n" ) )  return;

   // Initialize EGL.
   EGLint major = 0;
   EGLint minor = 0;
   if( !eglInitialize( _eglDisplay, &major, &minor ) )
   {
      StdErr << "ERROR - GLESContext_EGL() could not initialize." << nl;
      return;
   }
   if( checkForErrors( "ERROR - GLESContext_EGL() initialize: '%s'\n" ) )  return;
   //StdErr << "EGL v" << major << "." << minor << nl;

   // Select a specific config.
   EGLint attrs[] = {
      EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
      EGL_DEPTH_SIZE     , 24,
      EGL_NONE
   };
   EGLint numConfig = 0;
   if( !eglChooseConfig( _eglDisplay, attrs, &_eglConfig, 1, &numConfig ) )
   {
      StdErr << "ERROR - GLESContext_EGL() could not find suitable config." << nl;
      return;
   }
   if( checkForErrors( "ERROR - GLESContext_EGL() chooseConfig: '%s'\n" ) )  return;
   version(2); // EGL_OPENGL_ES2_BIT above.

   // Check if the config is native (accelerated?).
   EGLint nativeVid;
   if( !eglGetConfigAttrib( _eglDisplay, _eglConfig, EGL_NATIVE_VISUAL_ID, &nativeVid ))
   {
      StdErr << "ERROR - GLESContext_EGL() could not find native config." << nl;
      return;
   }
   if( checkForErrors( "ERROR - GLESContext_EGL() getConfigAttrib: '%s'\n" ) )  return;

   // Create a surface.
   _eglSurface = eglCreateWindowSurface( _eglDisplay, _eglConfig, _nativeWindow, NULL );
   if( _eglSurface == EGL_NO_SURFACE )
   {
      StdErr << "ERROR - GLESContext_EGL() could not create a surface." << nl;
      return;
   }
   if( checkForErrors( "ERROR - GLESContext_EGL() createWindowSurface: '%s'\n" ) )  return;

    // Create a context.
   _eglContext = eglCreateContext( _eglDisplay, _eglConfig, EGL_NO_CONTEXT, NULL );
   if( _eglContext == EGL_NO_CONTEXT )
   {
      StdErr << "ERROR - GLESContext_EGL() could not create a context." << nl;
      return;
   }
   if( checkForErrors( "ERROR - GLESContext_EGL() createContext: '%s'\n" ) )  return;
}

//------------------------------------------------------------------------------
//!
GLESContext_EGL::~GLESContext_EGL()
{
   eglMakeCurrent( EGL_NO_DISPLAY, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT );
   eglDestroyContext( _eglDisplay, _eglContext );
   eglDestroySurface( _eglDisplay, _eglSurface );
   eglTerminate( _eglDisplay );
}

//------------------------------------------------------------------------------
//!
bool
GLESContext_EGL::vsync() const
{
   return false;
}

//------------------------------------------------------------------------------
//!
void
GLESContext_EGL::vsync( bool /*v*/ )
{
}

//------------------------------------------------------------------------------
//!
bool
GLESContext_EGL::makeCurrent()
{
   return eglMakeCurrent( _eglDisplay, _eglSurface, _eglSurface, _eglContext ) != EGL_FALSE;
}

//------------------------------------------------------------------------------
//!
void
GLESContext_EGL::swapBuffers()
{
   eglSwapBuffers( _eglDisplay, _eglSurface );
}

#endif //GFX_GLES_EGL_SUPPORT
