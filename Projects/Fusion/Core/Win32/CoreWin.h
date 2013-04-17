/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef FUSION_COREWIN_H
#define FUSION_COREWIN_H

#include <Fusion/StdDefs.h>
#include <Fusion/Core/Core.h>

#include <Base/Util/windows.h>

NAMESPACE_BEGIN

/*==============================================================================
  CLASS CoreWin
==============================================================================*/

//! Specialized singleton for windows platform.

class CoreWin
   : public Core
{

public:

   /*----- methods -----*/

   CoreWin();
   virtual ~CoreWin();

   void  initializeGUI();
   void  finalizeGUI();

protected:

   /*----- methods -----*/

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

   /*----- static methods -----*/

   static LRESULT APIENTRY winProc(
      HWND   win,
      UINT   message,
      WPARAM wparam,
      LPARAM lparam
   );

   /*----- methods -----*/

   void setPaths();

   void initWin();
   void resize( int w, int h );
   void render();

   /*----- static data members -----*/

   static Vec2i   _size;
   static uint    _mainPointerID;
   static HWND    _winHandle;

};

NAMESPACE_END

#endif
