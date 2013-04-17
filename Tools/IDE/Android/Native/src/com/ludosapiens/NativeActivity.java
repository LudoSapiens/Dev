/*==============================================================================
   Copyright (c) 2011, Ludo Sapiens Inc.
   All rights reserved.

   These coded instructions, statements, and computer programs contain
   unpublished, proprietary information and are protected by Federal copyright
   law. They may not be disclosed to third parties or copied or duplicated in
   any form, in whole or in part, without prior written consent.
==============================================================================*/
package com.ludosapiens;

import android.app.Activity;
import android.os.Bundle;
import android.os.SystemClock;
import android.util.Log;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.View;

public class NativeActivity
   extends Activity
{
   //=============================================================================
   // Application callbacks.
   //=============================================================================

   @Override protected void onCreate( Bundle savedInstanceState )
   {
      super.onCreate( savedInstanceState );

      _height = getWindowManager().getDefaultDisplay().getHeight();
      //nativeOnCreatePre();

      _view = new NativeGLView( getApplication() );
      setContentView( _view );

      nativeOnCreate();
      nativeSetTime( SystemClock.uptimeMillis() );

      String files = getFilesDir().getAbsolutePath();
      String ext   = getExternalFilesDir(null).getAbsolutePath();
      String cache = getCacheDir().getAbsolutePath();
      String data  = getDir("Data", 0).getAbsolutePath();
      nativeSetPaths( files, ext, cache, data );
   }

   @Override protected void onStart()
   {
      super.onStart();
      nativeOnStart();
   }

   @Override protected void onRestart()
   {
      super.onRestart();
      nativeOnRestart();
   }

   @Override protected void onResume()
   {
      super.onResume();
      nativeOnResume();
   }

   @Override protected void onPause()
   {
      super.onPause();
      nativeOnPause();
   }

   @Override protected void onStop()
   {
      super.onStop();
      nativeOnStop();
   }

   @Override protected void onDestroy()
   {
      super.onDestroy();
      nativeOnDestroy();
   }

   public native void nativeSetTime( long ms );
   public native void nativeSetPaths( String files, String ext, String cache, String data );
   public native void nativeOnCreate();
   public native void nativeOnStart();
   public native void nativeOnRestart();
   public native void nativeOnResume();
   public native void nativeOnPause();
   public native void nativeOnStop();
   public native void nativeOnDestroy();


   //=============================================================================
   // Events callbacks.
   //=============================================================================

   @Override
   public boolean onKeyDown( int keyCode, KeyEvent event )
   {
      super.onKeyDown( keyCode, event );
      onKeyDown( event, event.getEventTime(), event.getKeyCode(), event.getMetaState(), event.getUnicodeChar(), event.getRepeatCount() );
      return true;
   }

   @Override
   public boolean onKeyUp( int keyCode, KeyEvent event )
   {
      super.onKeyUp( keyCode, event );
      onKeyUp( event, event.getEventTime(), event.getKeyCode(), event.getMetaState(), event.getUnicodeChar(), event.getRepeatCount() );
      return true;
   }

   @Override
   public boolean onTouchEvent( MotionEvent event )
   {
      super.onTouchEvent( event );
      onTouch( event, event.getEventTime(), event.getAction(), event.getActionIndex(), event.getRawX(), _height - event.getRawY() );
      return true;
   }

   /**
    * A native routine used to handle the keyDown events.
    * We convert the necessary parameters inside the JVM in order to limit the number JVM/JNI crossings.
    * @param event      : A reference to the original MotionEvent object (just in case we need more info).
    * @param time       : The event's native timestamp (converted to double in the JNI).
    * @param keyCode    : The key code.
    * @param metaState  : The state of the meta keys.
    * @param unicodeChar: The Unicode character represented (considers the meta state).
    * @param repeatCount: The repeat count.
    */
   public native void  onKeyDown( KeyEvent event, long time, int keyCode, int metaState, int unicodeChar, int repeatCount );

   /**
    * A native routine used to handle the keyUp events.
    * We convert the necessary parameters inside the JVM in order to limit the number JVM/JNI crossings.
    * @param event      : A reference to the original MotionEvent object (just in case we need more info).
    * @param time       : The event's native timestamp (converted to double in the JNI).
    * @param keyCode    : The key code.
    * @param metaState  : The state of the meta keys.
    * @param unicodeChar: The Unicode character represented (considers the meta state).
    * @param repeatCount: The repeat count.
    */
   public native void  onKeyUp( KeyEvent event, long time, int keyCode, int metaState, int unicodeChar, int repeatCount );

   /**
    * A native routine used to handle all of the touch events.
    * We convert the necessary parameters inside the JVM in order to limit the number JVM/JNI crossings.
    * @param event : A reference to the original MotionEvent object (just in case we need more info).
    * @param time  : The event's native timestamp (converted to double in the JNI).
    * @param action: One of {ACTION_DOWN|ACTION_MOVE|ACTION_UP|ACTION_CANCEL|ACTION_OUTSIDE}.
    * @param index : The pointer index.
    * @param x     : The current event's X coordinate.
    * @param y     : The current event's Y coordinate.
    */
   public native void  onTouch( MotionEvent event, long time, int action, int index, float x, float y );


   /* Load the JNI shared library.
    */
   static
   {
      System.loadLibrary( "NativeApp" );
   }

   //private static final String TAG = "Fusion";
   private float         _height;
   private NativeGLView  _view;
}
