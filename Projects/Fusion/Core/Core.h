/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef FUSION_CORE_H
#define FUSION_CORE_H

#include <Fusion/StdDefs.h>
#include <Fusion/Core/Event.h>
#include <Fusion/Core/HID.h>
#include <Fusion/Core/Pointer.h>

#include <CGMath/Random.h>
#include <CGMath/Variant.h>
#include <CGMath/Vec2.h>
#include <CGMath/Vec4.h>

#include <Gfx/Mgr/Manager.h>

#include <Snd/Manager.h>

#include <Base/ADT/DynArray.h>
#include <Base/ADT/Queue.h>
#include <Base/IO/Path.h>
#include <Base/Msg/DelegateList.h>
#include <Base/Util/IDPool.h>
#include <Base/Util/RCP.h>
#include <Base/Util/Timer.h>

struct lua_State;

NAMESPACE_BEGIN

class Animator;
class Bitmap;
class Desktop;
class Event;
class EventProfiler;
class FileDialog;
class TaskEvent;
class Widget;

typedef struct lua_State VMState;

/*==============================================================================
  CLASS Core
==============================================================================*/

//! Main class for the Fusion GUI.
//! Notes when adding support for a new platform.
//! - You need to implement Core::initialize() to instantiate your Core subclass.
//! - You need to implement Core::finalize() to deallocate that instance.
//! - You need to implement Core::exec() and call both Core::initializeGUI() and
//!   Core::finalizeGUI() at proper times (before and after the event loop).
//! - You need to implement all of the pure virtual functions.

class Core
{

public:

   /*----- types and enumerations ----*/

   typedef Delegate0< bool >              RenderDelegate;
   typedef Delegate0List< bool >          RenderDelegateList;
   typedef Delegate1< const Event& >      EventDelegate;
   typedef Delegate1List< const Event& >  EventDelegateList;
   typedef Delegate0<>                    ReadyToExecDelegate;
   typedef Delegate0<>                    GoToSleepDelegate;
   typedef Delegate0<>                    WakeUpDelegate;
   typedef DynArray<Pointer>              PointerContainer;

   enum Orientation
   {
      ORIENTATION_DEFAULT,
      ORIENTATION_CCW_90,
      ORIENTATION_UPSIDE_DOWN,
      ORIENTATION_CW_90
   };

   /*----- static methods -----*/

   // Initialization.
   FUSION_DLL_API static void initialize();
   FUSION_DLL_API static void finalize();

   // Debugging information.
   FUSION_DLL_API static void printInfo( TextStream& os );

   static inline Core&  singleton()    { return *_singleton; }
   static inline Core*  singletonPtr() { return  _singleton; }

   static inline VMState* vm();

   // Window size.
   FUSION_DLL_API static void fullScreen( bool enable );
   FUSION_DLL_API static bool fullScreen();

   FUSION_DLL_API static void size( const Vec2i& size );
   FUSION_DLL_API static const Vec2i& size();

   // Pointer.
   FUSION_DLL_API static Pointer&  createPointer( bool persistent = true );
   FUSION_DLL_API static Pointer&  pointer( uint pointerID );
   FUSION_DLL_API static     void  pointerIcon( uint id, uint icon );
   FUSION_DLL_API static     bool  grabPointer( uint pointerID, Widget* withWidget );
   FUSION_DLL_API static     void  releasePointer( uint pointerID );
   FUSION_DLL_API static     void  releasePointer( Widget* grabbingWidget );
   FUSION_DLL_API static     bool  otherPointerHovering( const Widget* widget, const uint pointerIDToExclude );
   FUSION_DLL_API static  Widget*  widgetUnderPointer( uint pointerID );
   static inline uint  numPressedPointers();
   static inline uint  pointerBegin();
   static inline uint  pointerNext( uint id );
   static inline uint  pointerEnd();

   // Keyboard.
   FUSION_DLL_API static void grabFocus( Widget* );
   FUSION_DLL_API static void releaseFocus( Widget* );
   FUSION_DLL_API static bool grabKeyboard( Widget* );
   FUSION_DLL_API static void releaseKeyboard( Widget* );
   FUSION_DLL_API static bool grabbedKeyboard();
   FUSION_DLL_API static Widget* keyboardGrabbingWidget();
   inline         static bool isKeyPressed( int key );

   // Accelerometer.
   FUSION_DLL_API static void  accelerometerFrequency( float hz );
   FUSION_DLL_API static float accelerometerFrequency();
   FUSION_DLL_API static void  registerAccelerometer( const EventDelegate& d );
   FUSION_DLL_API static void  unregisterAccelerometer( const EventDelegate& d );

   FUSION_DLL_API static void  orientation( Orientation ori, bool force = false );
   static inline  Orientation  orientation();
   static inline         void  lockOrientation( bool v );
   static inline         bool  orientationLocked();

   // Main desktop.
   FUSION_DLL_API static void desktop( const RCP<Desktop>& );
   FUSION_DLL_API static const RCP<Desktop>& desktop();

   // Native file dialogs.
   FUSION_DLL_API static void ask( FileDialog& diag );

   // HID.
   FUSION_DLL_API static void addOnHID( const EventDelegate& );
   FUSION_DLL_API static void removeOnHID( const EventDelegate& );

   // Animator.
   FUSION_DLL_API static void addAnimator( const RCP<Animator>& );
   FUSION_DLL_API static void addAnimator( const RCP<Animator>&, double startTime );
   FUSION_DLL_API static void removeAnimator( const RCP<Animator>& );

   //
   FUSION_DLL_API static void exec();
   FUSION_DLL_API static void exit();

   // Starting callback.
   FUSION_DLL_API static void callWhenReadyToExec( const ReadyToExecDelegate& cb );

   // Sleep and wake-up.
   FUSION_DLL_API static void callWhenGoingToSleep( const GoToSleepDelegate& cb );
   FUSION_DLL_API static void callWhenWakingUp( const WakeUpDelegate& cb );
   FUSION_DLL_API static void goToSleep();
   FUSION_DLL_API static void wakeUp();

   // Render callback.
   FUSION_DLL_API static void addRenderBegin( const RenderDelegate& );
   FUSION_DLL_API static void removeRenderBegin( const RenderDelegate& );

   // Events.
   FUSION_DLL_API static void submitEvent( const Event& );
   FUSION_DLL_API static void submitTaskEvent( const RCP<TaskEvent>& );
   static inline const Event& currentEvent();

   // Gfx.
   FUSION_DLL_API static void gfx( const RCP<Gfx::Manager>& mgr );
   FUSION_DLL_API static Gfx::Manager* gfx();
   FUSION_DLL_API static const String&  gfxAPI();
   FUSION_DLL_API static uint  gfxVersion();
   FUSION_DLL_API static RCP<Bitmap> screenGrab();
   FUSION_DLL_API static const RCP<Gfx::Program> defaultProgram();

   // Snd.
   FUSION_DLL_API static void snd( const RCP<Snd::Manager>& snd );
   FUSION_DLL_API static Snd::Manager* snd();
   FUSION_DLL_API static const String&  sndAPI();
   FUSION_DLL_API static void play( const Snd::Sound* snd );

   // Paths.
   static inline const Path&  config();
   static inline const Path&  userRoot();
   static inline const Path&  cacheRoot();

   static inline void  config( const Path& path );
   static inline void  userRoot( const Path& path );
   static inline void  cacheRoot( const Path& path );

   FUSION_DLL_API static void  addRoot( const Path& path, uint atIndex = CGConstu::max() );
   static inline void  removeRoot( uint idx );
   static inline uint  numRoots();
   static inline const Path&  root( uint idx = 0 );

   static inline double  lastTime();
   FUSION_DLL_API static EventProfiler&  profiler();

   static inline Table&  info();

   static inline RNG_WELL&  rng();

   static inline bool  open( const String& url );

protected:

   /*----- types -----*/
   struct PointerDeleteGuard
   {
      inline PointerDeleteGuard(): _id(-2) {}
      inline ~PointerDeleteGuard()
      {
         switch( _id + 2 )
         {
            case 0:   // -2, delete nothing.
               break;
            case 1:   // -1, delete all of the non-persistent pointers.
               for( uint cur = pointerBegin(), end = pointerEnd(); cur != end; cur = pointerNext(cur) )
               {
                  Pointer& ptr = Core::pointer( cur );
                  if( !ptr.persistent() )  Core::deletePointer( cur );
               }
               break;
            default:  // ptrID, delete just this one.
               Core::deletePointer( (uint)_id );
               break;
         }
      }
      inline void scheduleDelete( uint pid ) { _id = (int)pid; }
      inline void scheduleDeleteAll() { _id = -1; }
      int  _id;
   };

   struct EventPtrGuard
   {
      inline EventPtrGuard( Event& ev, Event*& ptr ): _evPtr(ptr) { _evPtr = &ev; }
      inline ~EventPtrGuard()
      {
         _evPtr = NULL;
      }
      Event*&  _evPtr;
   };

   friend struct PointerDeleteGuard;

   /*----- methods -----*/

   Core();
   virtual ~Core();

   void readyToExec();

   void setConfig();
   void setRoots();
   void readConfig( const Path& path );

   void processEvents();
   void processEvent( Event& event );
   void fixPointerStates();

   void executeAnimators( double time );

   static void deletePointer( uint pointerID );

   void performResize( int w, int h );
   void performRender();

   virtual void performExec() = 0;
   virtual void performExit() = 0;
   virtual void performShow() = 0;
   virtual void performHide() = 0;
   //virtual void performRedraw() = 0;

   virtual void performGrabPointer( uint pointerID );
   virtual void performReleasePointer( uint pointerID );
   virtual void performGrabKeyboard();
   virtual void performReleaseKeyboard();
   virtual bool performIsKeyPressed( int key );

   virtual void performSetPointerIcon( uint id, uint icon ) = 0;

   virtual void performSetAccelerometerFrequency( float hz );
   virtual void startAccelerometer();
   virtual void stopAccelerometer();

   virtual void performChangeOrientation( Orientation oldOri, Orientation newOri );

   virtual bool performOpen( const String& url );
   virtual void performAsk( FileDialog& diag );

   // These two routines should be called inside the Core::exec() routine of each platform.
   void initializeGUI();
   void finalizeGUI();

   // Texture management.
   void allocateTextures();


   /*----- data members -----*/

private:
   static FUSION_DLL_API Core*  _singleton;

   RCP<Gfx::Manager>        _gfx;
   RCP<Snd::Manager>        _snd;
   VMState*                 _vm;
   RNG_WELL                 _rng;

protected:
   StopTimer                _timer;
   double                   _lastTime;

   RCP<Gfx::RenderNode>     _filters;

   Event*                   _curEventPtr;
   Queue<Event>             _events;
   Queue< RCP<TaskEvent> >  _taskEvents;

   PointerContainer         _pointers;
   IDPool                   _pointerIDP;
   uint                     _numPressedPointers;

   float                    _accelerometerFrequency;
   EventDelegateList        _accelerometerDelegates;
   Orientation              _orientation;
   bool                     _orientationLocked;

   Delegate0List<>          _readyToExec;
   Delegate0List<>          _goToSleep;
   Delegate0List<>          _wakeUp;
   RenderDelegateList       _renderBegin;

   Path          _config;
   Path          _userRoot;
   Path          _cacheRoot;
   Vector<Path>  _roots;

   RCP<Table>  _infoTable;
};

//------------------------------------------------------------------------------
//!
inline VMState*
Core::vm()
{
   return singleton()._vm;
}

//------------------------------------------------------------------------------
//!
inline uint
Core::numPressedPointers()
{
   return singleton()._numPressedPointers;
}

//------------------------------------------------------------------------------
//!
inline uint
Core::pointerBegin()
{
   DynArray<Pointer>& pointers = singleton()._pointers;
   const uint n = uint(pointers.size());
   for( uint id = 0; id < n; ++id )
   {
      if( pointers[id].isValid() )  return id;
   }
   return pointerEnd();
}

//------------------------------------------------------------------------------
//!
inline uint
Core::pointerNext( uint id )
{
   DynArray<Pointer>& pointers = singleton()._pointers;
   const uint n = uint(pointers.size());
   for( ++id; id < n; ++id )
   {
      if( pointers[id].isValid() )  return id;
   }
   return pointerEnd();
}

//------------------------------------------------------------------------------
//!
inline uint
Core::pointerEnd()
{
   return (uint)-1;
}

//------------------------------------------------------------------------------
//!
inline Core::Orientation
Core::orientation()
{
   return singleton()._orientation;
}

//------------------------------------------------------------------------------
//!
inline void
Core::lockOrientation( bool v )
{
   singleton()._orientationLocked = v;
}

//------------------------------------------------------------------------------
//!
inline bool
Core::orientationLocked()
{
   return singleton()._orientationLocked;
}

//------------------------------------------------------------------------------
//!
inline const Event&
Core::currentEvent()
{
   Event* evPtr = singleton()._curEventPtr;
   return evPtr ? *evPtr : Event::Invalid();
}

//------------------------------------------------------------------------------
//!
inline const Path&
Core::config()
{
   return singleton()._config;
}

//------------------------------------------------------------------------------
//!
inline const Path&
Core::userRoot()
{
   return singleton()._userRoot;
}

//------------------------------------------------------------------------------
//!
inline const Path&
Core::cacheRoot()
{
   return singleton()._cacheRoot;
}

//------------------------------------------------------------------------------
//!
inline void
Core::config( const Path& path )
{
   singleton()._config = path;
}

//------------------------------------------------------------------------------
//!
inline void
Core::userRoot( const Path& path )
{
   singleton()._userRoot = path;
   singleton()._userRoot.toDir();
}

//------------------------------------------------------------------------------
//!
inline void
Core::cacheRoot( const Path& path )
{
   singleton()._cacheRoot = path;
   singleton()._cacheRoot.toDir();
}

//------------------------------------------------------------------------------
//!
inline uint
Core::numRoots()
{
   return uint(singleton()._roots.size());
}

//------------------------------------------------------------------------------
//!
inline const Path&
Core::root( uint idx )
{
   CHECK( idx < singleton()._roots.size() );
   return singleton()._roots[idx];
}

//------------------------------------------------------------------------------
//!
inline double
Core::lastTime()
{
   return singleton()._lastTime;
}

//------------------------------------------------------------------------------
//!
inline Table&
Core::info()
{
   return *(singleton()._infoTable);
}

//------------------------------------------------------------------------------
//!
inline RNG_WELL&
Core::rng()
{
   return singleton()._rng;
}

//------------------------------------------------------------------------------
//!
inline bool
Core::open( const String& url )
{
   return singleton().performOpen( url );
}

//------------------------------------------------------------------------------
//!
inline bool
Core::isKeyPressed( int key )
{
   return singleton().performIsKeyPressed( key );
}

NAMESPACE_END

#endif //FUSION_CORE_H
