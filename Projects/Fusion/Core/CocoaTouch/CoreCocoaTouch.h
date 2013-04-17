/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef FUSION_CORE_COCOA_TOUCH_H
#define FUSION_CORE_COCOA_TOUCH_H

#include <Fusion/StdDefs.h>
#include <Fusion/Core/Core.h>

#import <Foundation/Foundation.h>

@class NSAutoreleasePool;
@class UITouch;

NAMESPACE_BEGIN


/*==============================================================================
  CLASS CoreCocoaTouch
==============================================================================*/

//! Specialized singleton for CocoaTouch.

class CoreCocoaTouch:
   public Core
{

public:

   /*----- methods -----*/

   CoreCocoaTouch();
   virtual ~CoreCocoaTouch();

   static inline CoreCocoaTouch&  singleton();

   // Shorthand to make some routines public.
   static inline void resize( int w, int h );
   static inline void render();

   static inline void doLoop();

   static inline uint mainPointerID();

   void  initializeGUI();
   void  finalizeGUI();

   static inline Pointer&  pointer( UITouch* t );
   static inline Pointer&  createPointer( UITouch* t );
   static inline     void  deletePointer( UITouch* t );

   static NSTimeInterval  currentSystemTime();
   static inline double  toCoreTime( NSTimeInterval ti );

protected:

   /*----- types -----*/
   typedef Vector< UITouch* >  PointerCache;

   /*----- methods -----*/

   void  performLoop();

   virtual void performExec();
   virtual void performExit();
   virtual void performShow();
   virtual void performHide();
   virtual void performRedraw();
   virtual void performGrabPointer( uint );
   virtual void performReleasePointer( uint );
   virtual void performSetPointerIcon( uint, uint );
   virtual void performGrabKeyboard();
   virtual void performReleaseKeyboard();
   virtual void performSetAccelerometerFrequency( float hz );
   virtual void performChangeOrientation( Orientation oldOri, Orientation newOri );
   virtual void startAccelerometer();
   virtual void stopAccelerometer();
   virtual bool performOpen( const String& url );

private:

   /*----- static methods -----*/

   /*----- methods -----*/
   void  setPaths();

   /*----- data members -----*/
   NSAutoreleasePool*  _pool;
   PointerCache        _pointerCache;
   NSTimeInterval      _startTI;

}; //class CoreCocoaTouch

//------------------------------------------------------------------------------
//!
inline
CoreCocoaTouch&
CoreCocoaTouch::singleton()
{
   Core* core = &(Core::singleton());
   return *(CoreCocoaTouch*)core;
}

//------------------------------------------------------------------------------
//!
inline void
CoreCocoaTouch::resize( int w, int h )
{
   size( Vec2i(w, h) );
}

//------------------------------------------------------------------------------
//!
inline void
CoreCocoaTouch::render()
{
   singleton().performRender();
}

//------------------------------------------------------------------------------
//!
inline void
CoreCocoaTouch::doLoop()
{
   singleton().performLoop();
}


//------------------------------------------------------------------------------
//! Converts a UITouch event into a pointer ID to use in Core.
//! Since Apple made the UITouch event persistent, it can be used to map a pointer ID.
//! This routine performs a linear search amongst all active UITouch (a handful... or two).
inline Pointer&
CoreCocoaTouch::pointer( UITouch* t )
{
   PointerCache& cache = singleton()._pointerCache;
   for( uint cur = Core::pointerBegin(), end = Core::pointerEnd(); cur != end; cur = Core::pointerNext(cur) )
   {
      if( cache[cur] == t )
      {
         //StdErr << "UITouch" << (void*)t << ">>" << cur << nl;
         return Core::pointer( cur );
      }
   }

   CHECK( false );
   return Core::pointer( Core::pointerEnd() );
}

//------------------------------------------------------------------------------
//!
inline Pointer&
CoreCocoaTouch::createPointer( UITouch* t )
{
   PointerCache& cache = singleton()._pointerCache;
   Pointer& p = Core::createPointer( false );
   if( p.id() >= cache.size() )
   {
      cache.resize( p.id() + 1, NULL );
   }
   cache[p.id()] = t;
   //StdErr << "CreateUITouch" << (void*)t << ">>" << p.id() << nl;
   return p;
}

//------------------------------------------------------------------------------
//!
inline void
CoreCocoaTouch::deletePointer( UITouch* t )
{
   PointerCache& cache = singleton()._pointerCache;
#if _DEBUG
   CHECK( cache.replace( t, NULL ) );  // Sanity check.
#else
   cache.replace( t, NULL );
#endif
   //StdErr << "DeleteUITouch" << (void*)t << nl;
}

//------------------------------------------------------------------------------
//!
inline double
CoreCocoaTouch::toCoreTime( NSTimeInterval ti )
{
   return ti - singleton()._startTI;
}

NAMESPACE_END

#endif //FUSION_CORE_COCOA_TOUCH_H
