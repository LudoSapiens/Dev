/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Gfx/Mgr/GL/GLContext_GLX.h>

#include <Base/Dbg/DebugStream.h>

#include <cassert>

USING_NAMESPACE

using namespace Gfx;

/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN

DBG_STREAM( os_glx, "GLContext_GLX" );

UNNAMESPACE_END

bool  isBad( const Window w )
{
   return (w == BadAlloc) ||
          (w == BadColor) ||
          (w == BadCursor) ||
          (w == BadMatch) ||
          (w == BadPixmap) ||
          (w == BadValue) ||
          (w == BadWindow);
}

/*==============================================================================
   CLASS GLContext_GLX
==============================================================================*/

//------------------------------------------------------------------------------
//!
GLContext_GLX::GLContext_GLX( WindowStructure* ws ):
   _display(0),
   _window(BadWindow),
   _vinfo(0),
   _context(0)
{
   init( ws->_display, ws->_window );
}

//------------------------------------------------------------------------------
//!
GLContext_GLX::GLContext_GLX( Display* display, Window window ):
   _display(0),
   _window(BadWindow),
   _vinfo(0),
   _context(0)
{
   DBG_BLOCK( os_glx, "Creating GLContext_GLX" );
   init( display, window );
}

//------------------------------------------------------------------------------
//!
GLContext_GLX::~GLContext_GLX
( void )
{
   DBG_BLOCK( os_glx, "Destroying GLContext_GLX" );

   if( _display == 0 )
   {
      DBG_MSG( os_glx, "Display not set." );
      assert(0);
      return;
   }

   // Destroy context.
   glXMakeCurrent( _display, None, 0 );

   if( _context )
   {
      glXDestroyContext( _display, _context );
   }
}

//------------------------------------------------------------------------------
//!
bool
GLContext_GLX::vsync() const
{
#if GLX_MESA_swap_control
   int tmp = glXGetSwapIntervalMESA();
   return (tmp != 0);
#elif GLX_SWAP_INTERVAL_EXT
   unsigned int tmp;
   GLXDrawable drawable = glXGetCurrentDrawable();
   glXQueryDrawable( _display, drawable, GLX_SWAP_INTERVAL_EXT, &tmp );
   return (tmp != 0);
#else
   return false;
#endif
}

//------------------------------------------------------------------------------
//!
void
GLContext_GLX::vsync( bool v )
{
#if GLX_MESA_swap_control
   int tmp = (v?1:0);
   glXSwapIntervalMESA( tmp );
#elif GLX_SWAP_INTERVAL_EXT
   int tmp = (v?1:0);
   GLXDrawable drawable = glXGetCurrentDrawable();
   glXSwapIntervalEXT( _display, drawable, tmp );
#else
   v = v;
#endif
}

//------------------------------------------------------------------------------
//!
bool
GLContext_GLX::makeCurrent
( void )
{
   DBG_BLOCK( os_glx, "makeCurrent" );
   if( _context != 0 )
   {
      return glXMakeCurrent( _display, _window, _context );
   }
   else
   {
      DBG_MSG( os_glx, "MakeCurrent is missing context." );
      return false;
   }
}

//------------------------------------------------------------------------------
//!
bool
GLContext_GLX::init( Display* display, Window window )
{
   DBG_BLOCK( os_glx, "Initialize" );

   _display = display;
   if( _display == NULL )
   {
      DBG_MSG( os_glx, "No display connection." );
      return false;
   }

   _window = window;
   if( _window == BadWindow )
   {
      DBG_MSG( os_glx, "Received bad window." );
      return false;
   }
   
   // Make sure the GLX extensions are supported
   if( !glXQueryExtension( _display, 0, 0 ) )
   {
      DBG_MSG( os_glx, "GLX extensions not supported." );
      return false;
   }

   // Find an OpenGL-capable RGBA visual with a depth buffer.
   DBG_MSG( os_glx, "Need to create a visual; using RGBA_8888 w/D24 by default." );
   int conf[] = {
      GLX_DOUBLEBUFFER,
      GLX_RGBA,
      GLX_DEPTH_SIZE, 24,
      GLX_RED_SIZE, 8,
      GLX_GREEN_SIZE, 8,
      GLX_BLUE_SIZE, 8,
      None
   };

   _vinfo = glXChooseVisual( _display, DefaultScreen( _display ), conf );

   if( _vinfo == NULL )
   {
      DBG_MSG( os_glx, "No visual matching." );
      return false;
   }
   
   // Create the GL context.
   _context = glXCreateContext( _display, _vinfo, 0, True );

   if( _context == NULL )
   {
      DBG_MSG( os_glx, "Error creating context." );
      return false;
   }

   return true;
}

//------------------------------------------------------------------------------
//!
void
GLContext_GLX::swapBuffers
( void )
{
   DBG_BLOCK( os_glx, "Swapping buffers" );
   glXSwapBuffers( _display, _window );
}
