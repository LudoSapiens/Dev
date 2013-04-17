/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef FUSION_COREXGL_H
#define FUSION_COREXGL_H

#include <Fusion/StdDefs.h>
#include <Fusion/Core/Core.h>

#include <Gfx/Mgr/GL/GLContext_GLX.h>
#include <X11/Xlib.h>
#include <GL/glx.h>

NAMESPACE_BEGIN

/*==============================================================================
  CLASS CoreXGL
==============================================================================*/

//! Specialized singleton for opengl on X platform.

class CoreXGL
   : public Core
{

public:

   /*----- methods -----*/

   CoreXGL();
   virtual ~CoreXGL();

protected:

   /*----- methods -----*/

   virtual void performExec();
   virtual void performExit();
   virtual void performShow();
   virtual void performHide();
   virtual void performRedraw();
   virtual void performGrabPointer( uint );
   virtual void performReleasePointer( uint );
   virtual void performSetPointerIcon( uint, uint );

private:

   /*----- methods -----*/

   void initWin();
   void resize( int w, int h );
   void render();
   bool handleEvents( XEvent& ev );

   /*----- data members -----*/

   static Vec2i  _size;
   static uint   _mainPointerID;
   Atom          _wmDeleteWindow;

   Display*      _display;
   Window        _window;

   Time          _dblClickDelay;   // in milliseconds
};

NAMESPACE_END

#endif
