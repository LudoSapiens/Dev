/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef MAIN_H
#define MAIN_H

#include <Base/ADT/String.h>


//TEST_API can be one of the following:
#define TEST_API_UNKNOWN 0
#define TEST_API_GLX     1
#define TEST_API_WGL     2
#define TEST_API_UNUSED  3
#define TEST_API_D3D     4

#if defined(_MSC_VER)
#define TEST_API TEST_API_WGL  // Force WGL
//#define TEST_API TEST_API_D3D  // Force D3D
#endif

#define MANAGER_STR "opengl"
#define PS_STR glps_str

#if   TEST_API == TEST_API_GLX
#warning Using GLX
#elif TEST_API == TEST_API_WGL
# if defined(_MSC_VER)
# pragma message("Using WGL")
# else
# warning Using WGL
# endif
#elif TEST_API == TEST_API_UNUSED
# error Invalid test API
#elif TEST_API == TEST_API_D3D
# if defined(_MSC_VER)
# pragma message("Using D3D")
# else
# warning Using D3D
# endif
#undef MANAGER_STR
#define MANAGER_STR "d3d"
#undef PS_STR
#define PS_STR d3dps_str
#else
#error TEST_API is not set
#endif //TEST_API


#if TEST_API == TEST_API_GLX

#include <Gfx/Mgr/GL/GLContext_GLX.h>
#include <X11/Xlib.h>
#include <GL/glx.h>

NAMESPACE_BEGIN
namespace Gfx
{

/*==============================================================================
   CLASS Window
==============================================================================*/

//------------------------------------------------------------------------------
//!

class Window
{

public:

   /*----- methods -----*/

   Window()
   {
      _winStruct._display = NULL;
      _winStruct._window = BadWindow;
   }

   ~Window()
   {
   }

   void setSize( uint width, uint height )
   {
      _width  = width;
      _height = height;
   }

   void* getHandle()
   {
      return &_winStruct;
   }

   bool initialize()
   {
      // Window creation.
      XSetWindowAttributes attr;

      attr.background_pixel = 0;
      attr.border_pixel     = 0;
      attr.colormap         = CopyFromParent;
      attr.event_mask =
         ExposureMask | StructureNotifyMask |
         KeyPressMask | KeyReleaseMask |
         ButtonPressMask | ButtonReleaseMask | PointerMotionMask;

      unsigned long mask = CWBackPixel | CWBorderPixel | CWColormap | CWEventMask;

      _winStruct._display = XOpenDisplay( getenv("DISPLAY") );

      _winStruct._window = XCreateWindow(
         _winStruct._display,
         DefaultRootWindow( _winStruct._display ),
         0, 0,
         _width, _height,
         0,
         CopyFromParent,
         InputOutput,
         CopyFromParent,
         mask,
         &attr
      );

      //Show window as top-most
      XMapRaised( _winStruct._display, _winStruct._window );

      return true;
   }

private:

   /*----- data members -----*/

   uint  _width;
   uint  _height;
   GLContext_GLX::WindowStructure _winStruct;

};

}  //namespace Gfx
NAMESPACE_END


#endif //TEST_API == TEST_API_GLX


#if TEST_API == TEST_API_WGL || TEST_API == TEST_API_D3D

#include <Base/Util/windows.h>

NAMESPACE_BEGIN
namespace Gfx
{

//------------------------------------------------------------------------------
//!
LRESULT APIENTRY
winProc( HWND win, UINT message, WPARAM wparam, LPARAM lparam )
{
   return DefWindowProc( win, message, wparam, lparam );
}


/*==============================================================================
   CLASS Window
==============================================================================*/

//------------------------------------------------------------------------------
//!

class Window
{

public:

   /*----- methods -----*/

   Window()
   {
   }

   ~Window()
   {
   }

   void setSize( uint width, uint height )
   {
      _width  = width;
      _height = height;
   }

   void* getHandle()
   {
      return _winHandle;
   }

   bool initialize()
   {
      //////////////////
      // Create Windows.
      const LPCTSTR name = TEXT("Gfx");
      WNDCLASS    winClass;

      winClass.style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC | CS_DBLCLKS;
      winClass.lpfnWndProc   = winProc;
      winClass.cbClsExtra    = 0;
      winClass.cbWndExtra    = 0;
      winClass.hInstance     = 0;
      winClass.hIcon         = 0;
      winClass.hCursor       = 0;
      winClass.hbrBackground = 0;
      winClass.lpszMenuName  = 0;
      winClass.lpszClassName = name;
      RegisterClass( &winClass );

      printf( "create: %d x %d\n", _width, _height );

      // Calculate require size to get the specified interior size of contents
      RECT rect;
      rect.left = 50;
      rect.top = 50;
      rect.right = rect.left + _width;
      rect.bottom = rect.left + _height;
      DWORD dwStyle = WS_OVERLAPPEDWINDOW;
      DWORD dwStyleEx = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
      if( !AdjustWindowRectEx(&rect, dwStyle, FALSE, dwStyleEx) )
      {
         printf("ERROR - Failed to adjust WindowRect\n");
      }

      // Create the window
      _winHandle = CreateWindowEx(
         dwStyleEx,
         name,
         name,
         dwStyle,
         rect.left,
         rect.top,
         rect.right - rect.left,
         rect.bottom - rect.top,
         0,
         0,
         0,
         0
      );

      ////////////////////////
      ShowWindow( _winHandle, SW_SHOW );

      return true;
   }

private:

   /*----- data members -----*/

   uint   _width;
   uint   _height;
   HWND   _winHandle;

};

} //namespace Gfx
NAMESPACE_END

#endif //TEST_API == TEST_API_WGL || TEST_API == TEST_API_D3D

#endif //MAIN_H
