/*==============================================================================
   Copyright (c) 2011, Ludo Sapiens Inc.
   All rights reserved.

   These coded instructions, statements, and computer programs contain
   unpublished, proprietary information and are protected by Federal copyright
   law. They may not be disclosed to third parties or copied or duplicated in
   any form, in whole or in part, without prior written consent.
==============================================================================*/
package com.ludosapiens;

import android.content.Context;
//import android.graphics.PixelFormat;
import android.opengl.GLSurfaceView;
//import android.util.Log;

import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.egl.EGLContext;
import javax.microedition.khronos.egl.EGLDisplay;
import javax.microedition.khronos.opengles.GL10;

class NativeGLView
   extends GLSurfaceView
{
   //private static final String TAG = "Fusion";

   public NativeGLView( Context context )
   {
      super( context );
      setRenderer( new Renderer() );
   }

   private static class Renderer
      implements GLSurfaceView.Renderer
   {

      public void onDrawFrame(GL10 gl)
      {
         nativeRender();
      }

      public void onSurfaceChanged( GL10 gl, int width, int height )
      {
         nativeResize( width, height );
      }

      public void onSurfaceCreated( GL10 gl, EGLConfig config )
      {
         nativeInit();
      }

      public native void nativeInit();
      public native void nativeRender();
      public native void nativeResize( int width, int height );

   }

}
