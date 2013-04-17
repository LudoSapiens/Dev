/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef FUSION_CORE_ANDROID_INCLUDED
#define FUSION_CORE_ANDROID_INCLUDED

#include <Fusion/StdDefs.h>

#include <Fusion/Core/Core.h>

NAMESPACE_BEGIN

/*==============================================================================
  CLASS CoreAndroid
==============================================================================*/
class CoreAndroid:
   public Core
{
public:

   /*----- methods -----*/

   CoreAndroid();
   virtual ~CoreAndroid();

   static inline CoreAndroid&  singleton();

   static void    startTime( uint64_t ms );
   static double  millisecondsToSeconds( uint64_t ms );

   static void  setPaths( const char* files, const char* ext, const char* cache, const char* data );

   static void  initGfx();

   static inline void doLoop();

protected:

   /*----- methods -----*/

   void  performLoop();

   virtual void performExec();
   virtual void performExit();
   virtual void performShow();
   virtual void performHide();
   //virtual void performRedraw();
   //virtual void performGrabPointer( uint );
   //virtual void performReleasePointer( uint );
   virtual void performSetPointerIcon( uint, uint );
   //virtual void performGrabKeyboard();
   //virtual void performReleaseKeyboard();
   //virtual void performSetAccelerometerFrequency( float hz );
   //virtual void performChangeOrientation( Orientation oldOri, Orientation newOri );
   //virtual void startAccelerometer();
   //virtual void stopAccelerometer();
   //virtual bool performOpen( const String& url );

   /*----- data members -----*/

   uint64_t  _startTimeMS;

private:
}; //class CoreAndroid

//------------------------------------------------------------------------------
//!
inline
CoreAndroid&
CoreAndroid::singleton()
{
   Core* core = &(Core::singleton());
   return *(CoreAndroid*)core;
}

//------------------------------------------------------------------------------
//!
inline void
CoreAndroid::doLoop()
{
   singleton().performLoop();
}


NAMESPACE_END

#endif //FUSION_CORE_ANDROID_INCLUDED
