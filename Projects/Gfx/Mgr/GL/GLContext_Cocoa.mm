/*==============================================================================
   Copyright (c) 2008, Ludo Sapiens Inc.
   All rights reserved.

   These coded instructions, statements, and computer programs contain
   unpublished, proprietary information and are protected by Federal copyright
   law. They may not be disclosed to third parties or copied or duplicated in
   any form, in whole or in part, without prior written consent.
==============================================================================*/
#include <Gfx/Mgr/GL/GLContext_Cocoa.h>

#import <AppKit/NSOpenGL.h>

USING_NAMESPACE

using namespace Gfx;

/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN



UNNAMESPACE_END

//------------------------------------------------------------------------------
//!
GLContext_Cocoa::GLContext_Cocoa()
{
   
}

//------------------------------------------------------------------------------
//!
GLContext_Cocoa::~GLContext_Cocoa()
{
   
}

//------------------------------------------------------------------------------
//!
bool
GLContext_Cocoa::vsync() const
{
   GLint tmp = 0;
   [_context getValues:&tmp
          forParameter:NSOpenGLCPSwapInterval];
   return tmp != 0;
}

//------------------------------------------------------------------------------
//!
void
GLContext_Cocoa::vsync( bool v )
{
   GLint tmp = (v?1:0);
   [_context setValues:&tmp
          forParameter:NSOpenGLCPSwapInterval];
}

//------------------------------------------------------------------------------
//!
bool
GLContext_Cocoa::makeCurrent()
{
   [_context makeCurrentContext];
   return true;
}

//------------------------------------------------------------------------------
//!
void
GLContext_Cocoa::swapBuffers()
{
   [_context flushBuffer];
}
