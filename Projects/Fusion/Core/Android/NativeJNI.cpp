/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Fusion/Core/Android/CoreAndroid.h>
#include <Fusion/Fusion.h>

#include <Base/Dbg/DebugStream.h>

#include <string.h>
#include <jni.h>

#include <GLES/gl.h>

/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN

DBG_STREAM( os_jni, "Fusion" );

// Ref:
//   http://developer.android.com/reference/android/view/MotionEvent.html#constants
enum
{
   ACTION_DOWN    = 0x00000000,
   ACTION_UP      = 0x00000001,
   ACTION_MOVE    = 0x00000002,
   ACTION_CANCEL  = 0x00000003,
   ACTION_OUTSIDE = 0x00000004
};

/*==============================================================================
  CLASS PointerCache
==============================================================================*/
template< typename K >
class PointerCache
{
public:

   /*----- methods -----*/

   PointerCache( K invalid ): _invalid( invalid ) {}

   inline Pointer&  create( K key, bool persistent = false );
   inline void      destroy( K key );
   inline uint      getID( K key ) const;
   inline Pointer&  get( K key ) const;

protected:

   /*----- data members -----*/

   Vector< K >  _cache;
   K            _invalid;

private:
}; //class PointerCache

//-----------------------------------------------------------------------------
//!
template< typename K > inline Pointer&
PointerCache<K>::create( K key, bool persistent )
{
   Pointer& p = Core::createPointer( persistent );
   if( p.id() >= _cache.size() )
   {
      _cache.resize( p.id() + 1, _invalid );
   }
   _cache[p.id()] = key;
   return p;
}

//-----------------------------------------------------------------------------
//!
template< typename K > inline void
PointerCache<K>::destroy( K key )
{
#if defined(_DEBUG)
   CHECK( _cache.replace( key, _invalid ) );  // Sanity check.
#else
   _cache.replace( key, _invalid );
#endif
}

//-----------------------------------------------------------------------------
//!
template< typename K > inline uint
PointerCache<K>::getID( K key ) const
{
   for( uint cur = Core::pointerBegin(), end = Core::pointerEnd(); cur != end; cur = Core::pointerNext(cur) )
   {
      if( _cache[cur] == key )
      {
         return cur;
      }
   }

   CHECK( false );
   return Core::pointerEnd();
}

//-----------------------------------------------------------------------------
//!
template< typename K > inline Pointer&
PointerCache<K>::get( K key ) const
{
   return Core::pointer( getID(key) );
}

//-----------------------------------------------------------------------------
//!
inline double fix( jlong time )
{
   CoreAndroid::millisecondsToSeconds( time );
}

//-----------------------------------------------------------------------------
//!
inline int toKey( jint k )
{
   // TODO: Convert.
   return k;
}


//-----------------------------------------------------------------------------
//!
inline int toChar( jint c )
{
   // TODO: Convert?
   return c;
}

PointerCache<jint>  _pointerCache = PointerCache<jint>( -1 );

UNNAMESPACE_END

extern "C" {

//=============================================================================
// Application callbacks
//=============================================================================
extern void  nativeAppCreate();
extern void  nativeAppStart();

//-----------------------------------------------------------------------------
//! Same trick as Google uses with app_dummy() in their android_native_app_glue code.
void  nativeAppDummy()
{
   DBG_BLOCK( os_jni, "nativeAppDummy()" );
}

//=============================================================================
//=============================================================================
// FusionActivity
//=============================================================================
//=============================================================================

//=============================================================================
// Application callbacks
//=============================================================================

//-----------------------------------------------------------------------------
//!
JNIEXPORT void JNICALL
Java_com_ludosapiens_NativeActivity_nativeSetTime( JNIEnv* env,
                                                          jobject obj,
                                                          jlong   ms )
{
   DBG_BLOCK( os_jni, "nativeSetTime(" << ms << ")" );
   CoreAndroid::startTime( ms );
}

//-----------------------------------------------------------------------------
//!
JNIEXPORT void JNICALL
Java_com_ludosapiens_NativeActivity_nativeSetPaths( JNIEnv* env,
                                                           jobject obj,
                                                           jstring files,
                                                           jstring ext,
                                                           jstring cache,
                                                           jstring data )
{
   const char* filesStr = env->GetStringUTFChars( files, 0 );
   const char* extStr   = env->GetStringUTFChars( ext  , 0 );
   const char* cacheStr = env->GetStringUTFChars( cache, 0 );
   const char* dataStr  = env->GetStringUTFChars( data , 0 );

   //DBG_BLOCK( os_jni, "nativeSetPaths(" << filesStr << "," << extStr << "," << cacheStr << "," << dataStr << ")" );
   DBG_BLOCK( os_jni, "nativeSetPaths()" );
   CoreAndroid::setPaths( filesStr, extStr, cacheStr, dataStr );

   env->ReleaseStringUTFChars( files, filesStr );
   env->ReleaseStringUTFChars( ext  , extStr   );
   env->ReleaseStringUTFChars( cache, cacheStr );
   env->ReleaseStringUTFChars( data , dataStr  );
}

//-----------------------------------------------------------------------------
//!
JNIEXPORT void JNICALL
Java_com_ludosapiens_NativeActivity_nativeOnCreate( JNIEnv* env,
                                                           jobject obj )
{
   DBG_BLOCK( os_jni, "nativeOnCreate()" );
   nativeAppCreate();
}

//-----------------------------------------------------------------------------
//!
JNIEXPORT void JNICALL
Java_com_ludosapiens_NativeActivity_nativeOnStart( JNIEnv* env,
                                                          jobject obj )
{
   DBG_BLOCK( os_jni, "nativeOnStart()" );
   //nativeAppStart(); // Moved to nativeOnResume(), since the surface
}

//-----------------------------------------------------------------------------
//!
JNIEXPORT void JNICALL
Java_com_ludosapiens_NativeActivity_nativeOnRestart( JNIEnv* env,
                                                            jobject obj )
{
   DBG_BLOCK( os_jni, "nativeOnRestart()" );
}

//-----------------------------------------------------------------------------
//!
JNIEXPORT void JNICALL
Java_com_ludosapiens_NativeActivity_nativeOnResume( JNIEnv* env,
                                                           jobject obj )
{
   DBG_BLOCK( os_jni, "nativeOnResume()" );
}

//-----------------------------------------------------------------------------
//!
JNIEXPORT void JNICALL
Java_com_ludosapiens_NativeActivity_nativeOnPause( JNIEnv* env,
                                                          jobject obj )
{
   DBG_BLOCK( os_jni, "nativeOnPause()" );
}

//-----------------------------------------------------------------------------
//!
JNIEXPORT void JNICALL
Java_com_ludosapiens_NativeActivity_nativeOnStop( JNIEnv* env,
                                                         jobject obj )
{
   DBG_BLOCK( os_jni, "nativeOnStop()" );
}

//-----------------------------------------------------------------------------
//!
JNIEXPORT void JNICALL
Java_com_ludosapiens_NativeActivity_nativeOnDestroy( JNIEnv* env,
                                                            jobject obj )
{
   DBG_BLOCK( os_jni, "nativeOnDestroy()" );
}


//=============================================================================
// Events callbacks
//=============================================================================

//-----------------------------------------------------------------------------
//!
JNIEXPORT void JNICALL
Java_com_ludosapiens_NativeActivity_onKeyDown(
   JNIEnv* env,
   jobject obj,
   jobject event,
   jlong   time,
   jint    keyCode,
   jint    metaState,
   jint    unicodeChar,
   jint    repeatCount
)
{
   DBG_BLOCK( os_jni, String().format( "C++ onKeyDown(ev=%p time=%d (%f) keyCode=%d metaState=0x%04x uniChar=%d (0x%04x) repeat=%d)", (void*)event, (int)time, fix(time), keyCode, metaState, unicodeChar, unicodeChar, repeatCount ) );
   if( repeatCount == 0 )
   {
      // Press only on the first event.
      Core::submitEvent(
         Event::KeyPress( fix(time), toKey(keyCode) )
      );
   }
   Core::submitEvent(
      Event::Char( fix(time), toChar(unicodeChar), repeatCount+1 )
   );
}

//-----------------------------------------------------------------------------
//!
JNIEXPORT void JNICALL
Java_com_ludosapiens_NativeActivity_onKeyUp(
   JNIEnv* env,
   jobject obj,
   jobject event,
   jlong   time,
   jint    keyCode,
   jint    metaState,
   jint    unicodeChar,
   jint    repeatCount
)
{
   DBG_BLOCK( os_jni, String().format( "C++ onKeyUp(ev=%p time=%d (%f) keyCode=%d metaState=0x%04x uniChar=%d (0x%04x) repeat=%d)", (void*)event, (int)time, fix(time), keyCode, metaState, unicodeChar, unicodeChar, repeatCount ) );
   Core::submitEvent(
      Event::KeyRelease( fix(time), toKey(keyCode) )
   );
}

//-----------------------------------------------------------------------------
//!
JNIEXPORT void JNICALL
Java_com_ludosapiens_NativeActivity_onTouch(
   JNIEnv* env,
   jobject obj,
   jobject event,
   jlong   time,
   jint    action,
   jint    index,
   jfloat  x,
   jfloat  y
)
{
   DBG_BLOCK( os_jni, String().format( "C++ onTouch(ev=%p time=%d (%f) action=%d index=%d x=%f y=%f)", (void*)event, (int)time, fix(time), action, index, x, y ) );
   switch( action )
   {
      case ACTION_DOWN:
      {
         Pointer& pointer = _pointerCache.create( index );
         Vec2i        pos = Vec2i( x+0.5f, y+0.5f );  // Round, as it is slightly more accurate.
         uint       count = 0; // FIXME
         Core::submitEvent(
            Event::PointerPress( fix(time), pointer.id(), 1, pos, count )
         );
      }  break;
      case ACTION_UP:
      {
         Pointer& pointer = _pointerCache.get( index );
         Vec2i        pos = Vec2i( x+0.5f, y+0.5f );  // Round, as it is slightly more accurate.
         uint       count = 0; // FIXME
         Core::submitEvent(
            Event::PointerRelease( fix(time), pointer.id(), 1, pos, count )
         );
         _pointerCache.destroy( index );
      }  break;
      case ACTION_MOVE:
      {
         Pointer& pointer = _pointerCache.get( index );
         Vec2i        pos = Vec2i( x+0.5f, y+0.5f );  // Round, as it is slightly more accurate.
         Core::submitEvent(
            Event::PointerMove( fix(time), pointer.id(), pos )
         );
      }  break;
      case ACTION_CANCEL:
      {
         Pointer& pointer = _pointerCache.get( index );
         Core::submitEvent(
            Event::PointerCancel( fix(time), pointer.id() )
         );
         _pointerCache.destroy( index );
      }  break;
      //case ACTION_OUTSIDE:
      //{
      //}  break;
      default:
      {
         StdErr << "onTouch() - Unknown action type: " << action << "." << nl;
      }
   }
}


//=============================================================================
//=============================================================================
// FusionGLView
//=============================================================================
//=============================================================================

//-----------------------------------------------------------------------------
//!
JNIEXPORT void JNICALL
Java_com_ludosapiens_NativeGLView_00024Renderer_nativeInit(
   JNIEnv* env,
   jobject obj
)
{
   DBG_BLOCK( os_jni, "nativeInit()" );
   CoreAndroid::initGfx(); // Finish initializing Core.
   nativeAppStart();       // Call app->run().
}

//-----------------------------------------------------------------------------
//!
JNIEXPORT void JNICALL
Java_com_ludosapiens_NativeGLView_00024Renderer_nativeRender(
   JNIEnv* env,
   jobject obj
)
{
   DBG_BLOCK( os_jni, "nativeRender()" );
   CoreAndroid::doLoop();
}

//-----------------------------------------------------------------------------
//!
JNIEXPORT void JNICALL
Java_com_ludosapiens_NativeGLView_00024Renderer_nativeResize(
   JNIEnv* env,
   jobject obj,
   jint    width,
   jint    height
)
{
   DBG_BLOCK( os_jni, "nativeResize(" << width << "," << height << ")" );
   CoreAndroid::size( Vec2i(width, height) );
}

} // extern "C"
