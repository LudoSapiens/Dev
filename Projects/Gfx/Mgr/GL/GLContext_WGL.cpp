/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Gfx/Mgr/GL/GLContext_WGL.h>

#include <Base/IO/TextStream.h>

#if GFX_OGL_SUPPORT

/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN

BOOL (APIENTRY *wglSwapIntervalEXT)(int) = nullptr;
int  (APIENTRY *wglGetSwapIntervalEXT)() = nullptr;

BOOL APIENTRY nullSwapInterval( int )
{
   return TRUE;
}

int  APIENTRY nullGetSwapInterval()
{
   return 0;
}

UNNAMESPACE_END

NAMESPACE_BEGIN

namespace Gfx
{

/*==============================================================================
   CLASS GLContext_WGL
==============================================================================*/

//------------------------------------------------------------------------------
//!
GLContext_WGL::GLContext_WGL( HWND window )
{
   init( window );
}

//------------------------------------------------------------------------------
//!
GLContext_WGL::~GLContext_WGL()
{
   // Destroy context.
   wglMakeCurrent( 0, 0 );

   if( _glContext )
   {
      wglDeleteContext( _glContext );
   }
}

//------------------------------------------------------------------------------
//!
bool
GLContext_WGL::vsync() const
{
   return wglGetSwapIntervalEXT() != 0;
}

//------------------------------------------------------------------------------
//!
void
GLContext_WGL::vsync( bool v )
{
   int s = v ? -1 : 0;
   if( !wglSwapIntervalEXT( s ) )
   {
      wglSwapIntervalEXT( -s );
   }
}

//------------------------------------------------------------------------------
//!
bool
GLContext_WGL::makeCurrent()
{
   HDC hdc = GetDC( _winHandle );
   wglMakeCurrent( hdc, _glContext );


   if( wglSwapIntervalEXT == nullptr )
   {
      wglSwapIntervalEXT = (BOOL (APIENTRY*)(int))wglGetProcAddress( "wglSwapIntervalEXT" );
      if( wglSwapIntervalEXT == nullptr )
      {
         StdErr << "wglSwapIntervalEXT doesn't seem supported." << nl;
         wglSwapIntervalEXT = nullSwapInterval;
      }
      wglGetSwapIntervalEXT = (int (APIENTRY*)())wglGetProcAddress( "wglGetSwapIntervalEXT" );
      if( wglGetSwapIntervalEXT == nullptr )
      {
         StdErr << "wglGetSwapIntervalEXT doesn't seem supported." << nl;
         wglGetSwapIntervalEXT = nullGetSwapInterval;
      }
   }

   return true;
}

//------------------------------------------------------------------------------
//!
bool
GLContext_WGL::init( HWND window )
{
   _winHandle = window;

   ////////////////////////
   // Create OpenGL context
   PIXELFORMATDESCRIPTOR pfd;

   HDC hdc = GetDC( _winHandle );

   // Init the pixel format
   ZeroMemory( &pfd, sizeof( PIXELFORMATDESCRIPTOR ) );
   pfd.nSize        = sizeof( PIXELFORMATDESCRIPTOR );
   pfd.nVersion     = 1;
   pfd.dwFlags      = PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW;
   pfd.iPixelType   = PFD_TYPE_RGBA;
   pfd.cColorBits   = 32;
   pfd.cDepthBits   = 0;
   pfd.cStencilBits = 0;
   //pfd.cDepthBits   = 24;
   //pfd.cStencilBits = 8;

   SetPixelFormat( hdc, ChoosePixelFormat( hdc, &pfd ), &pfd );

   // Create the GL context
   _glContext = wglCreateContext( hdc );

   ReleaseDC( _winHandle, hdc );

   return true;
}

//------------------------------------------------------------------------------
//!
void
GLContext_WGL::swapBuffers()
{
   HDC hdc = GetDC( _winHandle );
   SwapBuffers( hdc );
}

}

NAMESPACE_END

#endif //GFX_OGL_SUPPORT
