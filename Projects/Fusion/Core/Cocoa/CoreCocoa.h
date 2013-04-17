/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef FUSION_CORE_COCOA_H
#define FUSION_CORE_COCOA_H

#include <Fusion/StdDefs.h>
#include <Fusion/Core/Core.h>

#include <Gfx/Mgr/GL/GLContext_Cocoa.h>

#import <AppKit/NSApplication.h>
#import <Foundation/Foundation.h>

#import <OpenGL/OpenGL.h>
#import <OpenGL/gl.h>

@class NSOpenGLContext;

NAMESPACE_BEGIN


/*==============================================================================
  CLASS CoreCocoa
==============================================================================*/

//! Specialized singleton for OSX's Cocoa.

class CoreCocoa:
   public Core
{

public:

   /*----- methods -----*/

   CoreCocoa();
   virtual ~CoreCocoa();

   static inline CoreCocoa&  singleton();

   // Shorthand to make some routines public.
   static inline void resize( int w, int h );
   static inline void render();

   static inline void doLoop();

   static inline uint mainPointerID();

   static inline void  context( NSOpenGLContext* context );

   void  initializeGUI();
   void  finalizeGUI();

   static NSTimeInterval  currentSystemTime();
   static inline double  toCoreTime( NSTimeInterval ti );

protected:

   /*----- methods -----*/

   void  performLoop();

   virtual void performExec();
   virtual void performExit();
   virtual void performShow();
   virtual void performHide();
   virtual void performRedraw();
   virtual bool performIsKeyPressed( int key );
   virtual void performGrabPointer( uint );
   virtual void performReleasePointer( uint );
   virtual void performSetPointerIcon( uint, uint );
   virtual void performAsk( FileDialog& diag );

private:

   /*----- methods -----*/
   void  setPaths();

   /*----- data members -----*/
   NSAutoreleasePool*         _pool;
   RCP<Gfx::GLContext_Cocoa>  _context;
   uint                       _mainPointerID;
   NSTimeInterval             _startTI;

}; //class CoreCocoa

//------------------------------------------------------------------------------
//!
inline
CoreCocoa&
CoreCocoa::singleton()
{
   Core* core = &(Core::singleton());
   return *(CoreCocoa*)core;
}

//------------------------------------------------------------------------------
//!
inline void
CoreCocoa::resize( int w, int h )
{
   size( Vec2i(w, h) );
}

//------------------------------------------------------------------------------
//!
inline void
CoreCocoa::render()
{
   if( desktop().isValid() )
   {
      singleton()._context->makeCurrent();
      // Execute animator.
      singleton().performRender();
   }
   else
   {
      // This avoids visual corruption of the very first screen, since
      // the NSApp awoke from NIB, called drawRect, but Core hasn't initialized
      // Gfx yet (since there were no context available yet).
      // Hopefully, this 'if' won't ever show up in performance profiles.
      glClear( GL_COLOR_BUFFER_BIT );
   }
}

//------------------------------------------------------------------------------
//!
inline void
CoreCocoa::doLoop()
{
   singleton().performLoop();
}

//------------------------------------------------------------------------------
//!
inline uint
CoreCocoa::mainPointerID()
{
   return singleton()._mainPointerID;
}

//------------------------------------------------------------------------------
//!
inline void
CoreCocoa::context( NSOpenGLContext* c )
{
   singleton()._context->context( c );
}

//------------------------------------------------------------------------------
//!
inline double
CoreCocoa::toCoreTime( NSTimeInterval ti )
{
   return ti - singleton()._startTI;
}

NAMESPACE_END

#endif //FUSION_CORE_COCOA_H
