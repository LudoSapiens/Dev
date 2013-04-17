/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Fusion/Core/Core.h>
#include <Fusion/Core/TaskEvent.h>
#include <Fusion/Core/Animator.h>
#include <Fusion/Core/EventProfiler.h>
#include <Fusion/Core/Key.h>
#include <Fusion/Drawable/Text.h>
#include <Fusion/Drawable/TQuad.h>
#include <Fusion/Resource/Bitmap.h>
#include <Fusion/Resource/ImageGenerator.h>
#include <Fusion/Resource/GlyphManager.h>
#include <Fusion/Resource/ResManager.h>
#include <Fusion/VM/BaseProxies.h>
#include <Fusion/VM/VMRegistry.h>
#include <Fusion/VM/VMMath.h>
#include <Fusion/VM/VMSubject.h>
#include <Fusion/VM/VMObjectPool.h>
#include <Fusion/Widget/Box.h>
#include <Fusion/Widget/Button.h>
#include <Fusion/Widget/Canvas.h>
#include <Fusion/Widget/ComboBox.h>
#include <Fusion/Widget/Desktop.h>
#include <Fusion/Widget/EventProfileViewer.h>
#include <Fusion/Widget/FileDialog.h>
#include <Fusion/Widget/Grid.h>
#include <Fusion/Widget/HotspotContainer.h>
#include <Fusion/Widget/Label.h>
#include <Fusion/Widget/Layer.h>
#include <Fusion/Widget/Menu.h>
#include <Fusion/Widget/MenuItem.h>
#include <Fusion/Widget/RadialButton.h>
#include <Fusion/Widget/RadialMenu.h>
#include <Fusion/Widget/Spacer.h>
#include <Fusion/Widget/Splitter.h>
#include <Fusion/Widget/TextEntry.h>
#include <Fusion/Widget/TreeList.h>
#include <Fusion/Widget/ValueEditor.h>

#include <Gfx/Mgr/Context.h>

#include <Snd/Sound.h>
#include <Snd/Source.h>

#include <CGMath/CGMath.h>
#include <CGMath/CGConst.h>

#include <Base/ADT/Queue.h>
#include <Base/Dbg/DebugStream.h>
#include <Base/IO/FileSystem.h>
#include <Base/MT/Lock.h>
#include <Base/MT/Thread.h>
#include <Base/Util/Application.h>
#include <Base/Util/Timer.h>

#include <map>
#include <time.h>

#if !defined( DBG_EVENT_QUEUE )
#  if defined(_DEBUG) && 0
#    define DBG_EVENT_QUEUE 1
#  else
#    define DBG_EVENT_QUEUE 0
#  endif
#endif


#define FUSION_CORE_HDR_PATH 0
#if !defined( FUSION_CORE_HDR_PATH )
#  if TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR || __IPHONE_OS_VERSION_MIN_REQUIRED || defined(__ANDROID__)
#    define FUSION_CORE_HDR_PATH 0
#  else
#    define FUSION_CORE_HDR_PATH 1
#  endif
#endif

/*==============================================================================
  UNNAME NAMESPACE
==============================================================================*/

UNNAMESPACE_BEGIN

DBG_STREAM( os_core, "Core" );

#if DBG_EVENT_QUEUE
Queue<Event>  _dbgEvents;
#endif

// FPS-related variables.
Timer  _fpsTimer;
uint   _fpsFrames;
double _fpsLastFrame;
double _fpsMinFrameDur;
double _fpsMaxFrameDur;
double _fpsPrintDelta = 0.0;

EventProfiler  _profiler;

// Gfx-related preferences.
String _gfxAPI("OpenGL");
uint   _gfxVersion = 0;
bool   _gfxVSync   = true;

// Snd-related preferences.
String _sndAPI(""); // "", "OpenAL"

// ResManager-related preferences.
uint  _numResourceThreads = 0;

// HID-related variables.
Delegate1List<const Event&>  _onHID;
VMRef                        _onHIDRef;

Lock  _renderBeginLock;

//------------------------------------------------------------------------------
//!
void
initConfigVM( VMState* vm, uint /*mask*/ )
{
   VM::push( vm, Core::info() );
   VM::setGlobal( vm, "Info" );
}

//------------------------------------------------------------------------------
//!
int
coreConfigVM( VMState* vm )
{
   if( VM::isTable( vm, -1 ) )
   {
      String str;
      Vec2i  v2i;
      VM::get( vm, -1, "showFps", _fpsPrintDelta );
      if( VM::get( vm, -1, "userRoot" , str ) )  Core::userRoot( str );
      if( VM::get( vm, -1, "cacheRoot", str ) )  Core::cacheRoot( str );
      if( VM::get( vm, -1, "root" ) )
      {
         // Add all of the specified path in order, but before any previously-specified.
         if( VM::isTable( vm, -1 ) )
         {
            for( uint i = 1; VM::geti( vm, -1, i ); ++i )
            {
               ResManager::handleRootCandidate( VM::toString( vm, -1 ) );
               VM::pop( vm );
            }
         }
         else
         if( VM::isString( vm, -1 ) )
         {
            ResManager::handleRootCandidate( VM::toString( vm, -1 ) );
         }
         else
         {
            StdErr << "Unsupported root format in config: " << VM::toTypename( vm, -1 ) << "." << nl;
         }
         VM::pop( vm );
      }
      if( VM::get( vm, -1, "debugConf", str ) )  DebugStream::readConfigFile( str.cstr() );
      VM::get( vm, -1, "gfxAPI", _gfxAPI );
      VM::get( vm, -1, "gfxVersion", _gfxVersion );
      VM::get( vm, -1, "gfxVSync", _gfxVSync );
      if( VM::get( vm, -1, "size", v2i ) )  Core::size( v2i );
      VM::get( vm, -1, "numResourceThreads", _numResourceThreads );
   }
   return 0;
}

//------------------------------------------------------------------------------
//!
int
getDesktop( VMState* vm )
{
   VM::pushProxy( vm, Core::desktop().ptr() );
   return 1;
}

//------------------------------------------------------------------------------
//!
int
setDesktop( VMState* vm )
{
   RCP<Desktop> widget( (Desktop*)VM::toProxy( vm, 1 ) );
   Core::desktop( widget );
   return 0;
}

//------------------------------------------------------------------------------
//!
int
isKeyPressed( VMState* vm )
{
   uint key = VM::toUInt( vm, 1 );
   VM::push( vm, Core::isKeyPressed(key) );
   return 1;
}

//------------------------------------------------------------------------------
//!
int
orientation( VMState* vm )
{
   int n = VM::getTop( vm );
   switch( n )
   {
      case 0:
         VM::push( vm, Core::orientation() );
         return 1;
      case 1:
         Core::orientation( (Core::Orientation)VM::toUInt( vm, 1 ) );
         return 0;
      default:
         Core::orientation( (Core::Orientation)VM::toUInt( vm, 1 ), VM::toBoolean( vm, 2 ) );
         return 0;
   }
}

//------------------------------------------------------------------------------
//!
int
lockOrientation( VMState* vm )
{
   Core::lockOrientation( VM::toBoolean( vm, 1 ) );
   return 0;
}

//------------------------------------------------------------------------------
//!
int
orientationLocked( VMState* vm )
{
   VM::push( vm, Core::orientationLocked() );
   return 1;
}

//------------------------------------------------------------------------------
//!
int
exit( VMState* )
{
   Core::exit();
   return 0;
}

//------------------------------------------------------------------------------
//!
int
open( VMState* vm )
{
   String url = VM::toString( vm, 1 );
   return Core::open( url );
}

//------------------------------------------------------------------------------
//!
int
lastTime( VMState* vm )
{
   VM::push( vm, Core::lastTime() );
   return 1;
}

//------------------------------------------------------------------------------
//!
int
setPath( VMState* vm )
{
   Path::setToken( VM::toString( vm, 1 ), VM::toString( vm, 2 ) );
   return 0;
}

//------------------------------------------------------------------------------
//!
int
getPath( VMState* vm )
{
   VM::push( vm, Path::getPath( VM::toString( vm, 1 ) ) );
   return 1;
}

//------------------------------------------------------------------------------
//!
int
grabKeyboard( VMState* vm )
{
   Widget* w = (Widget*)VM::toProxy( vm, -1 );
   VM::push( vm, Core::grabKeyboard(w) );
   return 1;
}

//------------------------------------------------------------------------------
//!
int
releaseKeyboard( VMState* vm )
{
   Widget* w = (Widget*)VM::toProxy( vm, -1 );
   Core::releaseKeyboard( w );
   return 0;
}

//------------------------------------------------------------------------------
//!
int
accelerometerFrequency( VMState* vm )
{
   if( VM::getTop(vm) == 0 )
   {
      VM::push( vm, Core::accelerometerFrequency() );
      return 1;
   }
   else
   {
      Core::accelerometerFrequency( VM::toFloat(vm, 1) );
      return 0;
   }
}

//------------------------------------------------------------------------------
//!
int
addAnimator( VMState* vm )
{
   RCP< VMAnimator > anim( new VMAnimator() );
   VM::toRef( vm, 1, anim->ref() );
   if( VM::getTop(vm) == 1 )
   {
      Core::addAnimator( anim );
   }
   else
   {
      Core::addAnimator( anim, VM::toNumber( vm, 2 ) );
   }

   return 0;
}

//------------------------------------------------------------------------------
//!
int
screenGrab( VMState* vm )
{
   String path = VM::toString( vm, 1 );
   Core::screenGrab()->save( path );
   return 0;
}

//------------------------------------------------------------------------------
//!
int
size( VMState* vm )
{
   if( VM::getTop(vm) == 0 )
   {
      VM::push( vm, Core::size() );
      return 1;
   }
   else
   {
      Core::size( VM::toVec2i(vm, 1) );
      return 0;
   }
}

//------------------------------------------------------------------------------
//!
int
onHID( VMState* vm )
{
   VM::toRef( vm, -1, _onHIDRef );
   return 0;
}

//------------------------------------------------------------------------------
//!
int
createPointer( VMState* vm )
{
   uint id = 0;
   switch( VM::getTop(vm) )
   {
      case 0:
      {
         id = Core::createPointer().id();
      }  break;
      case 1:
      {
         bool persistent = VM::toBoolean( vm, 1 );
         Pointer& pointer = Core::createPointer( persistent );
         id = pointer.id();
      }  break;
      default:
      {
         bool pers = VM::toBoolean( vm, 1 );
         uint icon = VM::toUInt( vm, 2 );
         Pointer& pointer = Core::createPointer( pers );
         pointer.icon( icon );
         id = pointer.id();
      }  break;
   }

   VM::push( vm, id );
   return 1;
}

//------------------------------------------------------------------------------
//!
int
deletePointer( VMState* vm )
{
   uint id = VM::toUInt( vm, 1 );
   Core::submitEvent( Event::PointerDelete(Core::lastTime(), id) );
   return 0;
}

//------------------------------------------------------------------------------
//!
int
setPointer( VMState* vm )
{
   uint  id = VM::toUInt( vm, 1 );
   int icon = VM::toUInt( vm, 2 );
   icon     = CGM::clamp( icon, 0, Pointer::_COUNT-1 );
   Core::pointerIcon( id, icon );
   return 0;
}

//------------------------------------------------------------------------------
//!
int
getPointerPosition( VMState* vm )
{
   uint id = VM::toUInt( vm, 1 );
   VM::push( vm, Core::pointer(id).position() );
   return 1;
}

//------------------------------------------------------------------------------
//!
int
getPointerSpeed( VMState* vm )
{
   uint id = VM::toUInt( vm, 1 );
   VM::push( vm, Core::pointer(id).getSpeed() );
   return 1;
}

//------------------------------------------------------------------------------
//!
int
widgetUnderPointer( VMState* vm )
{
   uint id = VM::toUInt( vm, 1 );
   VM::pushProxy( vm, Core::widgetUnderPointer(id) );
   return 1;
}

//------------------------------------------------------------------------------
//!
int
pointerFollow( VMState* vm )
{
   uint id = VM::toUInt( vm, 1 );
   Core::pointer(id).follow( (Widget*)VM::toProxy( vm, 2 ) );
   return 0;
}

//------------------------------------------------------------------------------
//!
int
pointerFollowing( VMState* vm )
{
   uint id = VM::toUInt( vm, 1 );
   VM::pushProxy( vm, Core::pointer(id).following() );
   return 1;
}

//------------------------------------------------------------------------------
//!
int
lastPointerEvent( VMState* vm )
{
   uint pid = VM::toUInt( vm, 1 );
   VM::push( vm, Core::pointer(pid).lastEvent() );
   return 1;
}

//------------------------------------------------------------------------------
//!
int
lastPointerPressEvent( VMState* vm )
{
   uint pid = VM::toUInt( vm, 1 );
   VM::push( vm, Core::pointer(pid).lastPressEvent() );
   return 1;
}

//------------------------------------------------------------------------------
//!
int
lastPointerReleaseEvent( VMState* vm )
{
   uint pid = VM::toUInt( vm, 1 );
   VM::push( vm, Core::pointer(pid).lastReleaseEvent() );
   return 1;
}

//------------------------------------------------------------------------------
//!
int
submitChar( VMState* vm )
{
   Core::submitEvent(
      Event::Char( Core::lastTime(), VM::toInt(vm, 1), VM::toUInt(vm, 2) )
   );
   return 0;
}

//------------------------------------------------------------------------------
//!
int
submitHID( VMState* vm )
{
   Core::submitEvent(
      Event::HID( Core::lastTime(), VM::toInt(vm, 1), VM::toInt(vm, 2), VM::toInt(vm, 3), (float)VM::toNumber(vm, 4) )
   );
   return 0;
}

//------------------------------------------------------------------------------
//!
int
submitKeyPress( VMState* vm )
{
   Core::submitEvent(
      Event::KeyPress( Core::lastTime(), VM::toInt(vm, 1) )
   );
   return 0;
}

//------------------------------------------------------------------------------
//!
int
submitKeyRelease( VMState* vm )
{
   Core::submitEvent(
      Event::KeyRelease( Core::lastTime(), VM::toInt(vm, 1) )
   );
   return 0;
}

//------------------------------------------------------------------------------
//!
int
submitPointerEnter( VMState* vm )
{
   Core::submitEvent(
      Event::PointerEnter( Core::lastTime(), VM::toUInt(vm, 1), VM::toVec2f(vm, 2) )
   );
   return 0;
}

//------------------------------------------------------------------------------
//!
int
submitPointerLeave( VMState* vm )
{
   Core::submitEvent(
      Event::PointerLeave( Core::lastTime(), VM::toUInt(vm, 1), VM::toVec2f(vm, 2) )
   );
   return 0;
}

//------------------------------------------------------------------------------
//!
int
submitPointerMove( VMState* vm )
{
   Core::submitEvent(
      Event::PointerMove( Core::lastTime(), VM::toUInt(vm, 1), VM::toVec2f(vm, 2) )
   );
   return 0;
}

//------------------------------------------------------------------------------
//!
int
submitPointerPress( VMState* vm )
{
   Core::submitEvent(
      Event::PointerPress( Core::lastTime(), VM::toUInt(vm, 1), VM::toInt(vm, 2), VM::toVec2f(vm, 3), VM::toBoolean(vm, 4) )
   );
   return 0;
}

//------------------------------------------------------------------------------
//!
int
submitPointerRelease( VMState* vm )
{
   Core::submitEvent(
      Event::PointerRelease( Core::lastTime(), VM::toUInt(vm, 1), VM::toInt(vm, 2), VM::toVec2f(vm, 3), VM::toBoolean(vm, 4) )
   );
   return 0;
}

//------------------------------------------------------------------------------
//!
int
submitPointerScroll( VMState* vm )
{
   Core::submitEvent(
      Event::PointerScroll( Core::lastTime(), VM::toUInt(vm, 1), VM::toFloat(vm, 2), VM::toVec2f(vm, 3) )
   );
   return 0;
}

//------------------------------------------------------------------------------
//!
int
submitPointerChange( VMState* vm )
{
   Core::submitEvent(
      Event::PointerChange( Core::lastTime(), VM::toUInt(vm, 1), VM::toInt(vm, 2) )
   );
   return 0;
}

//------------------------------------------------------------------------------
//!
int
configPath( VMState* vm )
{
   VM::push( vm, Core::config().cstr() );
   return 1;
}

//------------------------------------------------------------------------------
//!
int
rootPath( VMState* vm )
{
   if( VM::getTop( vm ) == 0 )
   {
      VM::push( vm, Core::root().cstr() );
   }
   else
   {
      uint idx = VM::toUInt( vm, 1 );
      VM::push( vm, Core::root(idx).cstr() );
   }
   return 1;
}

//------------------------------------------------------------------------------
//!
int
userRootPath( VMState* vm )
{
   VM::push( vm, Core::userRoot().cstr() );
   return 1;
}

//------------------------------------------------------------------------------
//!
int
cacheRootPath( VMState* vm )
{
   VM::push( vm, Core::cacheRoot().cstr() );
   return 1;
}

//------------------------------------------------------------------------------
//!
int
appName( VMState* vm )
{
   VM::push( vm, sApp->name() );
   return 1;
}

//------------------------------------------------------------------------------
//!
int
appVersion( VMState* vm )
{
   VM::push( vm, sApp->version() );
   return 1;
}

//------------------------------------------------------------------------------
//!
int
imageGen( VMState* vm )
{
   Image* img = nullptr;

   switch( VM::getTop( vm ) )
   {
      case 0:
         StdErr << "imageGen() - Missing arguments." << nl;
         return 0;
      case 1:
         img = data( ResManager::getImage( VM::toString(vm, 1) ) );
         break;
      case 2:
         img = data( ResManager::getImage( VM::toString(vm, 1), VM::toVec2i(vm, 2) ) );
         break;
      case 3:
      {
         String name  = VM::toString( vm, 1 );
         Vec2i  size  = VM::toVec2i( vm, 2 );
         RCP<Table>  table = new Table();
         VM::toTable( vm, 3, *table );
         img = data( ResManager::getImage( name, size, table.ptr() ) );
      }  break;
      default:
         StdErr << "imageGen() - Too many arguments." << nl;
         return 0;
   }

   VM::push( vm, (void*)img );
   return 1;
}

//------------------------------------------------------------------------------
//!
int
playSoundVM( VMState* vm )
{
   if( VM::getTop( vm ) == 1 )
   {
      RCP<Snd::Sound> snd;
      if( VM::isString( vm, 1 ) )
      {
         snd = data( ResManager::getSound( VM::toString( vm, 1 ) ) );
      }
      else
      {
         snd = (Snd::Sound*)VM::toPtr( vm, 1 );
      }
      Core::play( snd.ptr() );
   }
   else
   {
      StdErr << "playSoundVM() - Too many arguments (" << VM::getTop(vm) << ")." << nl;
   }
   return 0;
}

//------------------------------------------------------------------------------
//!
int
toUTF8VM( VMState* vm )
{
   switch( VM::type( vm, 1 ) )
   {
      case VM::NUMBER:
      {
         char32_t c = VM::toUInt( vm, 1 );
         VM::push( vm, toStr(c) );
         return 1;
      }  break;
      case VM::STRING:
      {
         // Assume UTF8 for now (use second parameter for encoding?).
         VM::insert( vm, 1 );
         return 1;
      }
   }
   return 0;
}

int printProfileVM( VMState* /*vm*/ )
{
   _profiler.print();
   return 0;
}

int saveProfileVM( VMState* vm )
{
   const char* filename = VM::toCString( vm, 1 );
   StdErr << "Saving " << _profiler.numEvents() << nl;
   StdErr << "Last=" << toStr(_profiler.last()) << nl;
   VM::push( vm, _profiler.save( filename ) );
   return 1;
}

int loadProfileVM( VMState* vm )
{
   const char* filename = VM::toCString( vm, 1 );
   EventProfiler profiler;
   VM::push( vm, profiler.load( filename ) );
   StdErr << "Loaded " << profiler.numEvents() << nl;
   StdErr << "Last=" << toStr(profiler.last()) << nl;
   profiler.print();
   return 1;
}

#if defined(_DEBUG)
//------------------------------------------------------------------------------
//!
int
printStackVM( VMState* vm )
{
   if( VM::getTop( vm ) == 0 )
   {
      VM::printStack( vm );
   }
   else
   {
      VM::printStack( vm, VM::toUInt( vm, 1 ) );
   }
   return 0;
}
#endif

//------------------------------------------------------------------------------
//!
RCP<Desktop> _desktop;

typedef std::map<int, bool>  keypressmap_type;
keypressmap_type*  _keypressedmap = nullptr;

bool      _fullscreen         = false;
//Vec2i     _desiredSize        = Vec2i( 1280, 720 );
Vec2i     _desiredSize        = Vec2i( 800, 600 );
Vec2i     _size               = Vec2i( 0, 0 );
Vec4f     _color              = Vec4f( 0, 0, 0, 1 );
Widget*   _keyGrabWidget      = nullptr;
Widget*   _focusedWidget      = nullptr;
Lock      _taskLock;

Vector< RCP<Animator> > _animators;
Vector< RCP<Animator> > _newAnimators;


const VM::Reg _fusionFuncs[] = {
   // Global core functions.
   { "getDesktop",              getDesktop              },
   { "setDesktop",              setDesktop              },
   { "isKeyPressed",            isKeyPressed            },
   { "grabKeyboard",            grabKeyboard            },
   { "releaseKeyboard",         releaseKeyboard         },
   { "accelerometerFrequency",  accelerometerFrequency  },
   { "releaseKeyboard",         releaseKeyboard         },
   { "orientation",             orientation             },
   { "lockOrientation",         lockOrientation         },
   { "orientationLocked",       orientationLocked       },
   { "addAnimator",             addAnimator             },
   { "screenGrab",              screenGrab              },
   { "size",                    size                    },
   { "exit",                    exit                    },
   { "open",                    open                    },
   { "lastTime",                lastTime                },
   { "setPath",                 setPath                 },
   { "getPath",                 getPath                 },
   { "appName",                 appName                 },
   { "appVersion",              appVersion              },
   { "onHID",                   onHID                   },
   { "createPointer",           createPointer           },
   { "deletePointer",           deletePointer           },
   { "getPointerPosition",      getPointerPosition      },
   { "getPointerSpeed",         getPointerSpeed         },
   { "setPointer",              setPointer              },
   { "widgetUnderPointer",      widgetUnderPointer      },
   { "pointerFollow",           pointerFollow           },
   { "pointerFollowing",        pointerFollowing        },
   { "lastPointerEvent",        lastPointerEvent        },
   { "lastPointerPressEvent",   lastPointerPressEvent   },
   { "lastPointerReleaseEvent", lastPointerReleaseEvent },
   { "submitChar",              submitChar              },
   { "submitHID",               submitHID               },
   { "submitKeyPress",          submitKeyPress          },
   { "submitKeyRelease",        submitKeyRelease        },
   { "submitPointerEnter",      submitPointerEnter      },
   { "submitPointerLeave",      submitPointerLeave      },
   { "submitPointerMove",       submitPointerMove       },
   { "submitPointerPress",      submitPointerPress      },
   { "submitPointerRelease",    submitPointerRelease    },
   { "submitPointerScroll",     submitPointerScroll     },
   { "submitPointerChange",     submitPointerChange     },
   { "imageGen",                imageGen                },
   { "playSound",               playSoundVM             },
   { "toUTF8",                  toUTF8VM                },
   { "printProfile",            printProfileVM          },
   { "saveProfile",             saveProfileVM           },
   { "loadProfile",             loadProfileVM           },
#if defined(_DEBUG)
   { "printStack",              printStackVM            },
#endif
   { 0, 0 }
};

const VM::Reg _pathFuncs[] = {
   { "config",    configPath    },
   { "root",      rootPath      },
   { "userRoot",  userRootPath  },
   { "cacheRoot", cacheRootPath },
   { 0, 0 }
};

const VM::EnumReg _enumsEventType[] = {
   { "POINTER_PRESS"  , Event::POINTER_PRESS   },
   { "POINTER_RELEASE", Event::POINTER_RELEASE },
   { "POINTER_MOVE"   , Event::POINTER_MOVE    },
   { "POINTER_ENTER"  , Event::POINTER_ENTER   },
   { "POINTER_LEAVE"  , Event::POINTER_LEAVE   },
   { "POINTER_CHANGE" , Event::POINTER_CHANGE  },
   { "POINTER_SCROLL" , Event::POINTER_SCROLL  },
   { "POINTER_CANCEL" , Event::POINTER_CANCEL  },
   { "KEY_PRESS"      , Event::KEY_PRESS       },
   { "KEY_RELEASE"    , Event::KEY_RELEASE     },
   { "CHAR"           , Event::CHAR            },
   { "HID_EVENT"      , Event::HID_EVENT       },
   { "ACCELERATE"     , Event::ACCELERATE      },
   { 0, 0 }
};

const VM::EnumReg _enumsOrientation[] = {
   { "DEFAULT"    ,  Core::ORIENTATION_DEFAULT     },
   { "CCW_90"     ,  Core::ORIENTATION_CCW_90      },
   { "UPSIDE_DOWN",  Core::ORIENTATION_UPSIDE_DOWN },
   { "CW_90"      ,  Core::ORIENTATION_CW_90       },
   { 0, 0 }
};

const VM::EnumReg _enumsPointer[] = {
   { "NONE" ,     Pointer::NONE     },
   { "DEFAULT" ,  Pointer::DEFAULT  },
   { "SIZE_ALL",  Pointer::SIZE_ALL },
   { "SIZE_T"  ,  Pointer::SIZE_T   },
   { "SIZE_B"  ,  Pointer::SIZE_B   },
   { "SIZE_BL" ,  Pointer::SIZE_BL  },
   { "SIZE_BR" ,  Pointer::SIZE_BR  },
   { "SIZE_TL" ,  Pointer::SIZE_TL  },
   { "SIZE_TR" ,  Pointer::SIZE_TR  },
   { "SIZE_L"  ,  Pointer::SIZE_L   },
   { "SIZE_R"  ,  Pointer::SIZE_R   },
   { "TEXT"    ,  Pointer::TEXT     },
   { "HAND"    ,  Pointer::HAND     },
   { "WAIT"    ,  Pointer::WAIT     },
   { "INVALID" ,  Pointer::INVALID  },
   { 0, 0 }
};

RCP<Gfx::RenderNode>  _rn;
RCP<Gfx::Pass>        _pass;
Mat4f                 _ortho;
RCP<Gfx::Program>     _program;

//------------------------------------------------------------------------------
//!
void
executeHIDEvent( const VMRef& ref, const Event& ev )
{
   if( ref.isValid() )
   {
      VMState* vm = ref.vm();
      VM::push( vm, ref );
      VM::push( vm, ev );
      VM::ecall( vm, 1, 0 );
   }
}

//------------------------------------------------------------------------------
//!
void
initVM( VMState* vm, uint /*mask*/ )
{
   // Create fusion script library.
   VM::registerFunctions( vm, "UI", _fusionFuncs );

#if defined(_DEBUG)
   VM::registerFunctions( vm, "Path", _pathFuncs );
#endif

   // Initialize the HID manager.
   HIDManager::initialize( vm, "HID" );

   // Register enums.
   VM::registerEnum( vm, "UI.Event", _enumsEventType );
   VM::registerEnum( vm, "UI.Orientation", _enumsOrientation );
   VM::registerEnum( vm, "UI.Pointer", _enumsPointer );

   // Add Subject proxy.
   VMSubjectVM::initialize( vm, "UI" );

   // Add Base proxies.
   BaseProxies::initialize( vm );
}

//------------------------------------------------------------------------------
//!
bool executeAnimators(
   double time,
   double delta,
   Vector< RCP<Animator> >& animators,
   size_t startIndex
)
{
   bool removed = false;

   Vector< RCP<Animator> >::Iterator cur = animators.begin() + startIndex;
   Vector< RCP<Animator> >::Iterator end = animators.end();

   for( ; cur != end; ++cur )
   {
      Animator* anim = (*cur).ptr();
      if( anim != nullptr )
      {
         if( anim->nextTime() <= time )
         {
            // Compute the delta since the last call.
            double animDelta = CGM::min( delta, anim->period() + time - anim->nextTime() );

            // Compute the next time before calling exec().
            anim->nextTime( anim->nextTime() + anim->period() ); // Respect animator's frequency.
            DBG_BEGIN();
            // Some sanity check in debug only.
            if( anim->nextTime() < time && anim->period() > 0.0 )
            {
               double n = (time - anim->nextTime()) / anim->period();
               if( n > 3 )
               {
                  StdErr << "WARNING: An animator is running more than 3 periods late: "
                         << " time=" << time << " delta=" << delta
                         << " animDelta=" << animDelta << " anim_next=" << anim->nextTime() << " anim_period=" << anim->period() << nl;
               }
            }
            DBG_END();

            // Call the exec() routine; animators could delete themselves from the queue.
            if( anim->exec( time, animDelta ) )
            {
               (*cur)  = nullptr; // Using iterator, which could point to nullptr already if the exec did a remove.
               removed = true;
            }
         }
      }
      else
      {
         removed = true;
      }
   }

   return removed;
}

//------------------------------------------------------------------------------
//!
void executeDelegates( Core::RenderDelegateList& delegates )
{
   bool removed = false;
   for( auto cur = delegates.begin(); cur != delegates.end(); ++cur )
   {
      Core::RenderDelegate& del = (*cur);
      if( del() )
      {
         del = Core::RenderDelegate();
         removed = true;
      }
   }
   if( removed )
   {
      delegates.clearEmpty();
   }
}

UNNAMESPACE_END

NAMESPACE_BEGIN

/*==============================================================================
  CLASS Core
==============================================================================*/

//------------------------------------------------------------------------------
//!
Core*  Core::_singleton = nullptr;

//------------------------------------------------------------------------------
//!
void
Core::gfx( const RCP<Gfx::Manager>& mgr )
{
   singleton()._gfx = mgr;
   if( mgr.isValid() )
   {
      _gfxAPI = mgr->API();
      if( _gfxAPI.endsWith("1") )  _gfxVersion = 1;
      else
      if( _gfxAPI.endsWith("2") )  _gfxVersion = 2;
      else
                                   _gfxVersion = 0;
   }
   else
   {
      _gfxAPI.clear();
      // _gfxVersion = 0; // Keep it, as maybe a subsequent call will succeed.
   }
   mgr->context()->vsync( _gfxVSync );
   ResManager::fusionGfxManagerChanged();
}

//------------------------------------------------------------------------------
//!
Gfx::Manager*
Core::gfx()
{
   return singleton()._gfx.ptr();
}

//------------------------------------------------------------------------------
//! Returns the desired Gfx API (specified in the configuration file).
const String&
Core::gfxAPI()
{
   return _gfxAPI;
}

//------------------------------------------------------------------------------
//! Returns the desired Gfx version (specified in the configuration file).
uint
Core::gfxVersion()
{
   return _gfxVersion;
}

//------------------------------------------------------------------------------
//!
const RCP<Gfx::Program>
Core::defaultProgram()
{
   return _program;
}

//------------------------------------------------------------------------------
//!
void
Core::snd( const RCP<Snd::Manager>& mgr )
{
   singleton()._snd = mgr;
   if( mgr.isValid() )
   {
      _sndAPI = mgr->API();
   }
   else
   {
      _sndAPI.clear();
   }
}

//------------------------------------------------------------------------------
//!
Snd::Manager*
Core::snd()
{
   return singleton()._snd.ptr();
}

//------------------------------------------------------------------------------
//! Returns the desired Snd API (specified in the configuration file), or the
//! actual API once the manager is instantiated.
const String&
Core::sndAPI()
{
   return _sndAPI;
}

//------------------------------------------------------------------------------
//!
void
Core::play( const Snd::Sound* snd )
{
   if( snd )
   {
      RCP<Snd::Source> source = singleton().snd()->createSource();
      source->bind( snd );
      source->autoDelete( true );
      source->update();
      source->play();
   }
}

//------------------------------------------------------------------------------
//!
void
Core::fullScreen( bool enable )
{
   _fullscreen = enable;
   // FIXME if show now then refresh!!
}

//------------------------------------------------------------------------------
//!
bool
Core::fullScreen()
{
   return _fullscreen;
}

//------------------------------------------------------------------------------
//!
void
Core::size( const Vec2i& size )
{
   _desiredSize = size;
   // FIXME if show now then refresh!!
   if( singletonPtr() && singleton()._gfx.isValid() )
   {
      singleton().performResize( size.x, size.y );
   }
}

//------------------------------------------------------------------------------
//!
const Vec2i&
Core::size()
{
   return _desiredSize;
}

//------------------------------------------------------------------------------
//!
bool
Core::grabPointer( uint pointerID, Widget* withWidget )
{
   Pointer& p = pointer( pointerID );
   if( p.grabbed() ) return false;

   p.grabWith( withWidget );
   singleton().performGrabPointer( pointerID );
   return true;
}

//------------------------------------------------------------------------------
//!
void
Core::releasePointer( uint pointerID )
{
   Pointer& p = pointer( pointerID );
   if( p.isValid() )
   {
      p.grabWith( 0 );
      singleton().performReleasePointer( p.id() );
   }
}

//------------------------------------------------------------------------------
//!
void
Core::releasePointer( Widget* w )
{
   for( uint cur = pointerBegin(), end = pointerEnd(); cur != end; cur = pointerNext(cur) )
   {
      Pointer& p = Core::pointer( cur );
      if( p.grabbedWith() == w )
      {
         p.grabWith( 0 );

         // Send a pointer move event to notify of a possible pointer in/out.
         Event moveEvt = Event::PointerMove( Core::lastTime(), p.id(), p.position() );
         singleton().processEvent( moveEvt ); // Process this event immediately.

         singleton().performReleasePointer( p.id() );
      }
   }
}

//------------------------------------------------------------------------------
//!
void
Core::grabFocus( Widget* w )
{
   if( _focusedWidget == w ) return;
   if( _focusedWidget ) _focusedWidget->handleEvent( Event::FocusLose( Core::lastTime() ) );
   _focusedWidget = w;
   if( _focusedWidget ) _focusedWidget->handleEvent( Event::FocusGrab( Core::lastTime() ) );
}

//------------------------------------------------------------------------------
//!
void
Core::releaseFocus( Widget* w )
{
   if( _focusedWidget != w || w == nullptr ) return;
   if( _focusedWidget ) _focusedWidget->handleEvent( Event::FocusLose( Core::lastTime() ) );
   _focusedWidget = nullptr;
}

//------------------------------------------------------------------------------
//!
bool
Core::grabKeyboard( Widget* w )
{
   _keyGrabWidget = w;
   singleton().performGrabKeyboard();

   return true;
}

//------------------------------------------------------------------------------
//!
void
Core::releaseKeyboard( Widget* w )
{
   if( _keyGrabWidget != w ) return;
   singleton().performReleaseKeyboard();
   _keyGrabWidget = nullptr;
}

//------------------------------------------------------------------------------
//!
bool
Core::grabbedKeyboard()
{
   return _keyGrabWidget != nullptr;
}

//------------------------------------------------------------------------------
//!
Widget*
Core::keyboardGrabbingWidget()
{
   return _keyGrabWidget;
}

//------------------------------------------------------------------------------
//!
void
Core::pointerIcon( uint id, uint icon )
{
   Pointer& p = pointer(id);
   if( p.icon() != icon )
   {
      p.icon( icon );
      singleton().performSetPointerIcon( id, icon );
   }
}

//------------------------------------------------------------------------------
//!
void
Core::desktop( const RCP<Desktop>& widget )
{
   _desktop = widget;
   _desktop->geometry( Vec2f(0.0f), Vec2f(0.0f), _size );
}

//------------------------------------------------------------------------------
//!
const RCP<Desktop>&
Core::desktop()
{
   return _desktop;
}

//-----------------------------------------------------------------------------
//!
void
Core::ask( FileDialog& diag )
{
   singleton().performAsk( diag );
}

//------------------------------------------------------------------------------
//!
bool
Core::otherPointerHovering( const Widget* w, const uint idToExclude )
{
   for( uint cur = pointerBegin(), end = pointerEnd(); cur != end; cur = pointerNext(cur) )
   {
      const Pointer& p = singleton()._pointers[cur];
      if( p.hoveringOver() == w && cur != idToExclude ) return true;
   }
   return false;
}

//------------------------------------------------------------------------------
//!
Widget*
Core::widgetUnderPointer( uint id )
{
   return pointer(id).hoveringOver();
}

//------------------------------------------------------------------------------
//!
void
Core::callWhenReadyToExec( const ReadyToExecDelegate& cb )
{
   singleton()._readyToExec.addDelegate( cb );
}

//------------------------------------------------------------------------------
//!
void
Core::readyToExec()
{
   _readyToExec.exec();
   // Create a fake desktop to avoid event crashes.
   if( _desktop.isNull() )  _desktop = new Desktop();
}

//------------------------------------------------------------------------------
//!
void
Core::callWhenGoingToSleep( const GoToSleepDelegate& cb )
{
   singleton()._goToSleep.addDelegate( cb );
}

//------------------------------------------------------------------------------
//!
void
Core::callWhenWakingUp( const WakeUpDelegate& cb )
{
   singleton()._wakeUp.addDelegate( cb );
}

//------------------------------------------------------------------------------
//!
void
Core::goToSleep()
{
   singleton()._goToSleep.exec();
   singleton()._timer.pause();
   singleton()._snd->masterStop();
}

//------------------------------------------------------------------------------
//!
void
Core::wakeUp()
{
   singleton()._snd->masterResume();
   singleton()._timer.resume();
   singleton()._wakeUp.exec();
}

//------------------------------------------------------------------------------
//!
void
Core::addRenderBegin( const RenderDelegate& cb )
{
   LockGuard guard( _renderBeginLock );
   singleton()._renderBegin.addDelegate( cb );
}

//------------------------------------------------------------------------------
//!
void
Core::removeRenderBegin( const RenderDelegate& cb )
{
   LockGuard guard( _renderBeginLock );
   singleton()._renderBegin.removeDelegate( cb );
}

//------------------------------------------------------------------------------
//!
void
Core::accelerometerFrequency( float hz )
{
   singleton()._accelerometerFrequency = hz;
   singleton().performSetAccelerometerFrequency( hz );
}
//------------------------------------------------------------------------------
//!
float
Core::accelerometerFrequency()
{
   return singleton()._accelerometerFrequency;
}

//------------------------------------------------------------------------------
//!
void
Core::registerAccelerometer( const EventDelegate& d )
{
   if( singleton()._accelerometerDelegates.empty() )  singleton().startAccelerometer();
   singleton()._accelerometerDelegates.addDelegate( d );
}

//------------------------------------------------------------------------------
//!
void
Core::unregisterAccelerometer( const EventDelegate& d )
{
   singleton()._accelerometerDelegates.removeDelegate( d );
   if( singleton()._accelerometerDelegates.empty() )  singleton().stopAccelerometer();
}

//------------------------------------------------------------------------------
//!
void
Core::orientation( Orientation ori, bool force )
{
   DBG_BLOCK( os_core, "Core::orientation(ori=" << (int)ori << ",force=" << force << ") ori=" << singleton()._orientation << " locked=" << orientationLocked() );
   if( !orientationLocked() || force )
   {
      Orientation oldOri = singleton()._orientation;
      CHECK( ori != oldOri );
      singleton().performChangeOrientation( oldOri, ori );
      singleton()._orientation = ori;
   }
}

//------------------------------------------------------------------------------
//!
void
Core::exec()
{
   PROFILE_EVENT( EventProfiler::EXEC_BEGIN );
   DBG_BLOCK( os_core, "Core::exec()" );
   singleton()._vm = VM::open( VM_CAT_APP | VM_CAT_MATH );

   singleton().setConfig();
   singleton().readConfig( singleton()._config );
   singleton().setRoots();
   ResManager::numThreads( _numResourceThreads );

   DBG_MSG( os_core, "Starting main loop" );
   singleton().performExec();
   DBG_MSG( os_core, "Ended main loop" );
   PROFILE_EVENT( EventProfiler::EXEC_END );
}

//------------------------------------------------------------------------------
//!
void
Core::exit()
{
   DBG_BLOCK( os_core, "Core::exit()" );
   singleton().performExit();
}

//------------------------------------------------------------------------------
//!
RCP<Bitmap>
Core::screenGrab()
{
   RCP<Bitmap> bmp = new Bitmap( _size, Bitmap::BYTE, 4 );
   gfx()->screenGrab( 0, 0, _size.x, _size.y, bmp->pixels() );
   return bmp;
}

//------------------------------------------------------------------------------
//!
void
Core::addAnimator( const RCP<Animator>& anim )
{
   anim->nextTime( lastTime() );
   _newAnimators.pushBack( anim );
}

//------------------------------------------------------------------------------
//!
void
Core::addAnimator( const RCP<Animator>& anim, double startTime )
{
   anim->nextTime( startTime );
   _newAnimators.pushBack( anim );
}

//------------------------------------------------------------------------------
//!
void
Core::removeAnimator( const RCP<Animator>& anim )
{
   if( !_animators.replace( anim, nullptr ) )
   {
      // Didn't find it in _animators, so falling back in _newAnimators.
      if( !_newAnimators.replace( anim, nullptr ) )
      {
         StdErr << "ERROR - Core::removeAnimator() could not find animator: " << (void*)anim.ptr() << nl;
      }
   }
}

//------------------------------------------------------------------------------
//!
void
Core::addOnHID( const EventDelegate& delegate )
{
   _onHID.addDelegate( delegate );
}

//------------------------------------------------------------------------------
//!
void
Core::removeOnHID( const EventDelegate& delegate )
{
   _onHID.removeDelegate( delegate );
}

//------------------------------------------------------------------------------
//!
Pointer&
Core::createPointer( bool persistent )
{
   uint id = singleton()._pointerIDP.next();
   Pointer& pointer = singleton()._pointers[id];
   pointer.init( id, persistent );
   return pointer;
}

//------------------------------------------------------------------------------
//! This is a private method.  Submit an Event::PointerDelete(id) instead.
void
Core::deletePointer( uint id )
{
   singleton()._pointers[id].invalidate();
   singleton()._pointerIDP.release( id );
}

//------------------------------------------------------------------------------
//!
Pointer&
Core::pointer( uint id )
{
   return singleton()._pointers[id];
}

//------------------------------------------------------------------------------
//!
Core::Core():
   _vm( nullptr ),
   _lastTime( 0.0 ),
   _numPressedPointers( 0 ),
   _orientation( ORIENTATION_DEFAULT ),
   _orientationLocked( false )
{
   DBG_BLOCK( os_core, "Core::Core" );

   PROFILE_EVENT( EventProfiler::CORE_BEGIN );

   CHECK( _singleton == nullptr );
   _singleton = this;

   _infoTable = new Table();
   _infoTable->set( ConstString("numHardwareThreads"), (float)Thread::numHardwareThreads() );

   _rng.seed( uint32_t(::time(nullptr)) );  // Initialize with the current time.

#if DBG_EVENT_QUEUE
   for( uint i = 0; i < 8; ++i )
   {
      _dbgEvents.push( Event::Invalid() );
   }
#endif

   _rn   = new Gfx::RenderNode();
   _pass = new Gfx::Pass();

   VMRegistry::add( initVM, VM_CAT_APP );

   // Register config routines.
   VMRegistry::add( "Core", coreConfigVM, VM_CAT_CFG );
   VMRegistry::add( initConfigVM, VM_CAT_CFG );

   // Register VMObjectPool routines.
   VMObjectPool::initialize();
   Desktop::initialize();
   Box::initialize();
   Button::initialize();
   Canvas::initialize();
   ComboBox::initialize();
   EventProfileViewer::initialize();
   FileDialog::initialize();
   Grid::initialize();
   HotspotContainer::initialize();
   Label::initialize();
   Layer::initialize();
   Menu::initialize();
   MenuItem::initialize();
   RadialButton::initialize();
   RadialMenu::initialize();
   Spacer::initialize();
   Splitter::initialize();
   TextEntry::initialize();
   TreeList::initialize();
   ValueEditor::initialize();
   Widget::initialize();
   WidgetContainer::initialize();

   // Register drawable.
   Text::initialize();
   TQuad::initialize();

   // Other.
   Key::initialize();

   // Initialize the math library for VM.
   VMMath::initialize();

   // Initialize resource manager.
   ResManager::initializeFusion();
   ImageGenerator::initialize();

   // Allocate keypressed map.
   _keypressedmap = new keypressmap_type();

   PROFILE_EVENT( EventProfiler::CORE_BEGIN );
}

//------------------------------------------------------------------------------
//!
Core::~Core()
{
   DBG_BLOCK( os_core, "Core::~Core" );

   PROFILE_EVENT( EventProfiler::CORE_END );

   Text::terminate();

   ResManager::terminateFusion();

   // Kill the app's VM.
   if( _vm )  VM::close( _vm );

   // Clear pointers.
   singleton()._pointers.clear();

   // Deallocate HID.
   _onHID.clear();
   HIDManager::terminate();

   // Clear animators.
   _animators.clear();
   _newAnimators.clear();

   VMObjectPool::terminate();

   // Deallocate keypressed map.
   delete _keypressedmap;
   _keypressedmap = nullptr;
   _singleton     = nullptr;

   PROFILE_EVENT( EventProfiler::CORE_END );
}

//------------------------------------------------------------------------------
//!
void
Core::setConfig()
{
   DBG_BLOCK( os_core, "Core::setConfig()" );

   DBG_MSG( os_core, "Checking environment variable." );
   const char* str = getenv( "CONFIG_FILE" );
   if( str )
   {
      DBG_MSG( os_core, "Forcing CONFIG_FILE: " << str );
      _config = str;
   }
   else
   if( _config.empty() )
   {
      // Search directories up.
      Path cwd = Path::getCurrentDirectory();
      DBG_MSG( os_core, "Search up from: " << cwd.string() << "." );
      Path config("Data");
      config /= "config.lua";
      FS::Entry entry;
      const char* patterns[] = { "config.lua", config.string().cstr(), nullptr };
      if( FS::searchUp( entry, FS::TYPE_FILE, cwd, patterns ) )
      {
         DBG_MSG( os_core, "Found: " << entry.path().string() );
         _config = entry.path();
      }
      else
      {
         DBG_MSG( os_core, "No config file found." );
      }
   }
   else
   {
      // Likely set in a Core subclass constructor.
      DBG_MSG( os_core, "Config already set to: " << _config.string() );
   }
   DBG_MSG( os_core, "Config=" << _config.string() );
}

//------------------------------------------------------------------------------
//!
void
Core::setRoots()
{
   DBG_MSG( os_core, "Setting roots" );

   if( numRoots() == 0 )
   {
      StdErr << "WARNING - Your Core* platform should at least call addRoot(\"\") in its constructor." << nl;
   }

   DBG_BEGIN();
   os_core << numRoots() << " roots:" << nl;
   for( uint i = 0; i < _roots.size(); ++i )
   {
      os_core << " " << _roots[i].string() << nl;
   }
   os_core << nl;
   DBG_END();
   //Path::setToken( "Root", _root.string() );

   if( _userRoot.empty() ) { _userRoot = root(); _userRoot /= "user"; }
   _userRoot.toDir();
   DBG_MSG( os_core, "UserRoot=" << _userRoot.string() );
   //Path::setToken( "UserRoot", _userRoot.string() );

   if( _cacheRoot.empty() ) { _cacheRoot = root(); _cacheRoot /= "cache"; }
   _cacheRoot.toDir();
   DBG_MSG( os_core, "CacheRoot=" << _cacheRoot.string() );
   //Path::setToken( "CacheRoot", _cacheRoot.string() );
}

//------------------------------------------------------------------------------
//!
void
Core::readConfig( const Path& path )
{
   DBG_BLOCK( os_core, "Core::readConfig()" );
   if( FS::Entry(path).exists() )
   {
      DBG_MSG( os_core, "Loading config: " << path.string() );
      VMState* vm = VM::open( VM_CAT_CFG | VM_CAT_MATH );
      VM::doFile( vm, path.string() );
      VM::close( vm );
   }
   else
   {
      DBG_MSG( os_core, "Config file does not exist: " << path.string() );
   }
}

//------------------------------------------------------------------------------
//!
void
Core::submitEvent( const Event& event )
{
#if DBG_EVENT_QUEUE
   StdErr << "Core::submitEvent(" << event.toStr() << ")" << nl;
#endif

   Queue<Event>& events = singleton()._events;
   if( !events.empty() && event.type() == Event::POINTER_MOVE )
   {
      const Event& last = events.back();
      // Only collapse move events of the same pointer.
      if( (event.type() == last.type()) && (event.pointerID() == last.pointerID()) )
      {
         events.back() = event;
         return;
      }
   }

   events.push(event);
}

//------------------------------------------------------------------------------
//!
void
Core::submitTaskEvent( const RCP<TaskEvent>& event )
{
   LockGuard lock( _taskLock );
   singleton()._taskEvents.push( event );
}

//------------------------------------------------------------------------------
//!
void
Core::processEvents()
{
   PROFILE_EVENT( EventProfiler::PROCESS_EVENTS_BEGIN );

   // Process events.
   while( !_events.empty() )
   {
      processEvent( _events.front() );
      _events.pop();
   }

   // Generate possibly-missing glyphs (will be ready next frame).
   GlyphManager::generateMissingGlyphs();

   // Process task events.
   while( !_taskEvents.empty() )
   {
      RCP<TaskEvent> event;
      {
         LockGuard lock( _taskLock );
         event = _taskEvents.front();
         _taskEvents.pop();
      }
      event->execute();
   }

   PROFILE_EVENT( EventProfiler::PROCESS_EVENTS_END );
}

//------------------------------------------------------------------------------
//!
void
Core::processEvent( Event& event )
{
   EventPtrGuard epg( event, _curEventPtr );
   PointerDeleteGuard pdg;
#if DBG_EVENT_QUEUE
   StdErr << "  Core::processEvent(" << event.toStr() << ")" << nl;
   _dbgEvents.pop();
   _dbgEvents.push( event );
#endif

   switch( event.type() )
   {
      case Event::POINTER_PRESS:
         Core::pointer( event.pointerID() ).pressed( event.value(), true );
         ++_numPressedPointers;
         break;
      case Event::POINTER_RELEASE:
      {
         Pointer& p = Core::pointer( event.pointerID() );
         if( p.pressed( event.value() ) )
         {
            p.pressed( event.value(), false );
            --_numPressedPointers;
            if( !p.persistent() )  pdg.scheduleDelete( event.pointerID() );
         }
         else
         {
#if defined(_WIN32)
            // Simply ignore.
            // Happens when we close a FileDialog with the mouse;
            // the window under the dialog receives the release
            // instead of the dialog (which disappeared after the press).
#else
            CHECK( false ); // Where did the press go?
#endif
         }
      }  break;
      case Event::POINTER_CHANGE:
         pointerIcon( event.pointerID(), event.value() );
         return;
      case Event::POINTER_DELETE:
      {
         Pointer& p     = pointer( event.pointerID() );
         Widget* widget = p.hoveringOver();
         if( widget )
         {
            widget->handleEvent(
               Event::PointerLeave( Core::lastTime(), p.id(), widget->screenToLayer(event.position()) )
            );
         }
         deletePointer( event.pointerID() );
      }  return;
      case Event::POINTER_CANCEL:
      {
         _desktop->closePopups();
#if 0
         // Clear all of the non-persistent pointers.
         for( uint cur = pointerBegin(), end = pointerEnd(); cur != end; cur = pointerNext(cur) )
         {
            Pointer& p = _pointers[cur];
            p.pressedState( 0 );
            Widget* w = p.grabbedWith();
            if( w )
            {
               w->handleEvent( event );
               p.grabWith( nullptr );
            }
         }
         pdg.scheduleDeleteAll();
         _numPressedPointers = 0;
         return;
#else
         // Clear only this pointer.
         Pointer& p = pointer( event.pointerID() );
         p.pressedState( 0 );
         Widget* w = p.grabbedWith();
         if( w )
         {
            w->handleEvent( event );
            p.grabWith( nullptr );
         }
         if( !p.persistent() )  pdg.scheduleDelete( event.pointerID() );
         CHECK( _numPressedPointers > 0 );
         --_numPressedPointers;
         return;
#endif
      }  break;
      case Event::KEY_PRESS:
         (*_keypressedmap)[event.value()] = true;
         break;
      case Event::KEY_RELEASE:
         (*_keypressedmap)[event.value()] = false;
         break;
      case Event::HID_EVENT:
         _onHID.exec( event );
         executeHIDEvent( _onHIDRef, event );
         return;
      case Event::ACCELERATE:
         // Only registered widgets receive acceleration events.
         _accelerometerDelegates.exec( event );
         return;
      default:
         break;
   }

   if( event.isPointerEvent() )
   {
      Widget* widget = nullptr;
      Pointer& p     = pointer(event.pointerID());
      p.curEvent( Core::lastTime(), event );
      if( p.grabbed() )
      {
         widget = p.grabbedWith();
      }
      else
      {
         // Find widget under pointer
         widget = (event.type() == Event::POINTER_CANCEL) ? nullptr : _desktop->getWidgetAt( event.position() );

         Widget* oldHover = p.hoveringOver();

         // new widget?
         if( widget != oldHover )
         {
            // Leave event
            if( oldHover )
            {
               oldHover->handleEvent(
                  Event::PointerLeave( Core::lastTime(), p.id(), oldHover->screenToLayer(event.position()) )
               );
            }

            p.hoverOver( widget );

            if( !widget ) return;

            // Enter event
            pointerIcon( p.id(), widget->hoverIcon() );
            widget->handleEvent(
               Event::PointerEnter( Core::lastTime(), p.id(), widget->screenToLayer(event.position()) )
            );
         }
         else
         if( !widget ) return;
      }

      // Releasing focus is necessary.
      if( _focusedWidget && _focusedWidget != widget )
      {
         switch( event.type() )
         {
            case Event::POINTER_PRESS:
            case Event::POINTER_RELEASE:
            case Event::POINTER_CANCEL:
               releaseFocus( _focusedWidget );
               break;
            default: ;
         }
      }

      // Check if we have to close popups
      if( !widget->belongToPopup() )
      {
         switch( event.type() )
         {
            case Event::POINTER_PRESS:
            case Event::POINTER_RELEASE:
            case Event::POINTER_CANCEL:
               _desktop->closePopups();
               break;
            default: ;
         }
      }

      // Convert pointer location.
      event.position( widget->screenToLayer(event.position()) );

      // Send event to the widget.
      widget->handleEvent( event );
   }
   else
   if( event.isKeyboardEvent() )
   {
      if( _focusedWidget )
      {
         // The focused widget has the higher priority for keyboard handling.
         _focusedWidget->handleEvent( event );
      }
      else
      if( _keyGrabWidget )
      {
         // Send it only to the grabbing widget.
         _keyGrabWidget->handleEvent( event );
      }
      else
      {
         // Execute the event on all widgets under the default pointer from top to bottom.
         Vector<Widget*> whierarchy;
         _desktop->getWidgetsAt( pointer(0).position(), whierarchy );
         for( auto it = whierarchy.begin(); it != whierarchy.end(); ++it )
         {
            (*it)->handleEvent( event );
         }
      }
   }
   else
   {
      // Redirect the event to the current desktop.
      _desktop->handleEvent( event );
   }
}

//------------------------------------------------------------------------------
//!
void Core::fixPointerStates()
{
   for( uint cur = pointerBegin(), end = pointerEnd(); cur != end; cur = pointerNext(cur) )
   {
      Pointer& pointer = _pointers[cur];
      if( pointer.following() )
      {
         // Adjust for pointers following widgets.
         Event ev = Event::PointerMove( Core::lastTime(), cur, pointer.following()->getCenterGlobal() );
         processEvent( ev );
      }
      else
      if( pointer.persistent() && !pointer.grabbed() )
      {
         Widget* oldHover = pointer.hoveringOver();
         Widget* newHover = _desktop->getWidgetAt( pointer.position() );
         if( newHover != oldHover )
         {
            // Leave event
            if( oldHover )
            {
               oldHover->handleEvent(
                  Event::PointerLeave( Core::lastTime(), pointer.id(), oldHover->screenToLayer(pointer.position()) )
               );
            }

            pointer.hoverOver( newHover );

            if( newHover )
            {
               // Enter event
               pointerIcon( pointer.id(), newHover->hoverIcon() );
               newHover->handleEvent(
                  Event::PointerEnter( Core::lastTime(), pointer.id(), newHover->screenToLayer(pointer.position()) )
               );
            }
         }
      }
   }
}

//------------------------------------------------------------------------------
//!
void
Core::executeAnimators( double time )
{
   double delta = time - _lastTime;
   bool removed = ::executeAnimators( time, delta, _animators, 0 );
   while( !_newAnimators.empty() )
   {
      size_t oldSize = _animators.size();
      _animators.append( _newAnimators );
      _newAnimators.clear();
      removed |= ::executeAnimators( time, delta, _animators, oldSize );
   }

   if( removed )
   {
      _animators.removeAll( nullptr );
   }

   _lastTime = time;
}

//------------------------------------------------------------------------------
//!
void
Core::performResize( int w, int h )
{
   DBG_BLOCK( os_core, "Core::performResize(" << w << ", " << h << ")" );

   // Do we need to resize?
   if( _size.x == w && _size.y == h ) return;

   _size.x      = w;
   _size.y      = h;
   _desiredSize = _size;

   _ortho  = Mat4f::ortho( 0.0f, (float)w, 0.0f, (float)h, 0.0f, 1.0f );

#if FUSION_CORE_HDR_PATH
   // Reallocate all rendering textures to the new size.
   allocateTextures();
#else
   // Nothing.
#endif

   // Resize gfx manager.
   _gfx->setSize( w, h );

   // Resize UI.
   if( _desktop.isValid() )  _desktop->geometry( Vec2f(0.0f), Vec2f(0.0f), _size );

#if FUSION_CORE_HDR_PATH
   // Full HDR+Blur.

   // Rendering target.
   RCP<Gfx::Texture> hdrBuffer   = ResManager::getRenderableTexture( String( "hdrColor1" ) );
   RCP<Gfx::Texture> backBuffer  = ResManager::getRenderableTexture( String( "color1" ) );
   RCP<Gfx::Texture> buffer1     = ResManager::getRenderableTexture( String( "color1_4" ) );
   RCP<Gfx::Texture> buffer2     = ResManager::getRenderableTexture( String( "color2_4" ) );
   RCP<Gfx::Texture> buffer3     = ResManager::getRenderableTexture( String( "color1_16" ) );
   RCP<Gfx::Texture> buffer4     = ResManager::getRenderableTexture( String( "color2_16" ) );

   // Set main buffer.
   RCP<Gfx::Framebuffer> fb = gfx()->createFramebuffer();
   fb->setColorBuffer( hdrBuffer );
   _pass->setFramebuffer( fb );

   Vec2i lowSize     = _size / 4;
   Vec2i veryLowSize = lowSize / 4;

   // Texture states used for filtering.
   Gfx::TextureState nearest;
   nearest.setPointSampling();
   nearest.clamp( Gfx::TEX_CLAMP_LAST );

   Gfx::TextureState bilinear;
   bilinear.setBilinear();
   bilinear.clamp( Gfx::TEX_CLAMP_LAST );

   Vec2f const1;

   // Create render node with its filtering passes.
   _filters = new Gfx::RenderNode();

   // 1. Compute bright colors.
   RCP<Gfx::Program> pgmGlow = data( ResManager::getProgram( "shader/program/f_glow_detect" ) );
   RCP<Gfx::SamplerList> samplerGlow( new Gfx::SamplerList() );
   samplerGlow->addSampler( "tex", hdrBuffer, nearest );
   _filters->addPass( _gfx->createFilterPass( samplerGlow, pgmGlow, backBuffer, 0 ) );

   // 2. Scale down.
   RCP<Gfx::Program> pgmDown4x4 = data( ResManager::getProgram( "shader/program/f_downsample4x4" ) );
   RCP<Gfx::SamplerList> samplerDown4x4( new Gfx::SamplerList() );
   samplerDown4x4->addSampler( "tex", backBuffer, bilinear );
   const1 = Vec2f( 1.0f/w, 1.0f/h );
   RCP<Gfx::ConstantBuffer> constDown4x4 = _gfx->createConstants( pgmDown4x4 );
   constDown4x4->setConstant( "texelSize", const1.ptr() );
   _filters->addPass( _gfx->createFilterPass( samplerDown4x4, pgmDown4x4, Gfx::ConstantList::create(constDown4x4), buffer1 ) );

   // 3. Filter horizontally.
   RCP<Gfx::Program> pgmBlurH = data( ResManager::getProgram( "shader/program/f_gauss9x9_h" ) );
   RCP<Gfx::SamplerList> samplerBlurH( new Gfx::SamplerList() );
   samplerBlurH->addSampler( "tex", buffer1, bilinear );
   const1 = Vec2f( 1.386283f, 3.253459f ) / (float)lowSize.x;
   RCP<Gfx::ConstantBuffer> constBlurH = _gfx->createConstants( pgmBlurH );
   constBlurH->setConstant("disp", const1.ptr() );
   _filters->addPass( _gfx->createFilterPass( samplerBlurH, pgmBlurH, Gfx::ConstantList::create(constBlurH), buffer2 ) );

   // 4. Filter Vertically.
   RCP<Gfx::Program> pgmBlurV = data( ResManager::getProgram( "shader/program/f_gauss9x9_v" ) );
   RCP<Gfx::SamplerList> samplerBlurV( new Gfx::SamplerList() );
   samplerBlurV->addSampler( "tex", buffer2, bilinear );
   const1 = Vec2f( 1.386283f, 3.253459f ) / (float)lowSize.y;
   RCP<Gfx::ConstantBuffer> constBlurV = _gfx->createConstants( pgmBlurV );
   constBlurV->setConstant("disp", const1.ptr() );
   _filters->addPass( _gfx->createFilterPass( samplerBlurV, pgmBlurV, Gfx::ConstantList::create(constBlurV), buffer1 ) );

   // 5. Scale down again.
   RCP<Gfx::SamplerList> samplerVeryLow( new Gfx::SamplerList() );
   samplerVeryLow->addSampler( "tex", buffer1, bilinear );
   const1 = Vec2f( 1.0f ) / Vec2f( lowSize );
   RCP<Gfx::ConstantBuffer> constVeryLow = _gfx->createConstants( pgmDown4x4 );
   constVeryLow->setConstant( "texelSize", const1.ptr() );
   _filters->addPass( _gfx->createFilterPass( samplerVeryLow, pgmDown4x4, Gfx::ConstantList::create(constVeryLow), buffer3 ) );

   // 6. Filter horizontally.
   RCP<Gfx::SamplerList> samplerBlurH2( new Gfx::SamplerList() );
   samplerBlurH2->addSampler( "tex", buffer3, bilinear );
   const1 = Vec2f( 1.386283f, 3.253459f ) / (float)veryLowSize.x;
   RCP<Gfx::ConstantBuffer> constBlurH2 = _gfx->createConstants( pgmBlurH );
   constBlurH2->setConstant( "disp", const1.ptr() );
   _filters->addPass( _gfx->createFilterPass( samplerBlurH2, pgmBlurH, Gfx::ConstantList::create(constBlurH2), buffer4 ) );

   // 7. Filter Vertically.
   RCP<Gfx::SamplerList> samplerBlurV2( new Gfx::SamplerList() );
   samplerBlurV2->addSampler( "tex", buffer4, bilinear );
   const1 = Vec2f( 1.386283f, 3.253459f ) / (float)veryLowSize.y;
   RCP<Gfx::ConstantBuffer> constBlurV2 = _gfx->createConstants( pgmBlurV );
   constBlurV2->setConstant( "disp", const1.ptr() );
   _filters->addPass( _gfx->createFilterPass( samplerBlurV2, pgmBlurV, Gfx::ConstantList::create(constBlurV2), buffer3 ) );

   // 8. Final compositing.
   RCP<Gfx::Program> pgmComp = data( ResManager::getProgram( "shader/program/f_final_blend" ) );
   RCP<Gfx::SamplerList> samplerComp( new Gfx::SamplerList() );
   samplerComp->addSampler( "baseTex", hdrBuffer, nearest );
   samplerComp->addSampler( "bloomTex1", buffer1, bilinear );
   samplerComp->addSampler( "bloomTex2", buffer3, bilinear );
   _filters->addPass( _gfx->createFilterPass( samplerComp, pgmComp, nullptr ) );

#else
   // No filter, simply show the backbuffer.
#endif
}

//------------------------------------------------------------------------------
//!
void Core::performRender()
{
   PROFILE_EVENT( EventProfiler::RENDER_BEGIN );

   {
      LockGuard guard( _renderBeginLock );
      ::executeDelegates( _renderBegin );
   }

   //DBG_BLOCK( os_core, "Core::performRender" );
   _rn->addPass( _pass );

   // Set background color.
   _color = _desktop->color();
   _pass->setClearColor( _color.x, _color.y, _color.z, _color.w );

   if( _desktop->needUpdate() )
   {
      _desktop->geometry( Vec2f(0.0f), Vec2f(0.0f), _desktop->actualSize() );
      fixPointerStates();
   }

   _pass->setViewport( 0, 0, _size.x, _size.y );
   _pass->setScissor( 0, 0, _size.x, _size.y );
   _pass->setProjectionMatrixPtr( _ortho.ptr() );
   _pass->setProgram( _program );

   _desktop->render( _rn );

   _gfx->render( _rn );
   if( _filters.isValid() ) _gfx->render( _filters );

   PROFILE_EVENT( EventProfiler::DISPLAY_BEGIN );
   _gfx->display();
   PROFILE_EVENT( EventProfiler::DISPLAY_END );

   _pass->clear();
   _rn->clear();

   if( _fpsPrintDelta > 0.0 )
   {
      ++_fpsFrames;
      double elapsed = _fpsTimer.elapsed();
      double delta = elapsed - _fpsLastFrame;
      _fpsMinFrameDur = CGM::min( _fpsMinFrameDur, delta );
      _fpsMaxFrameDur = CGM::max( _fpsMaxFrameDur, delta );
      _fpsLastFrame = elapsed;
      if( elapsed >= _fpsPrintDelta )
      {
         double fps = _fpsFrames/elapsed;
         printf("%g fps [min=%g, avg=%g, max=%g]\n", fps, _fpsMinFrameDur * 1000.0, 1000.0 / fps, _fpsMaxFrameDur * 1000.0);
         _fpsFrames      = 0;
         _fpsMinFrameDur = CGConstd::infinity();
         _fpsMaxFrameDur = 0.0;
         _fpsLastFrame   = 0.0;
         _fpsTimer.restart();
      }
   }

   PROFILE_EVENT( EventProfiler::RENDER_END );
}

//------------------------------------------------------------------------------
//!
void Core::performGrabPointer( uint )
{}

//------------------------------------------------------------------------------
//!
void Core::performReleasePointer( uint )
{}

//------------------------------------------------------------------------------
//!
void Core::performGrabKeyboard()
{}

//------------------------------------------------------------------------------
//!
void Core::performReleaseKeyboard()
{}

//------------------------------------------------------------------------------
//! Default behavior (original code, software state caching).
bool Core::performIsKeyPressed( int key )
{
   keypressmap_type::const_iterator cur = _keypressedmap->find( key );
   if( cur != _keypressedmap->end() )
   {
       return (*cur).second;
   }
   else
   {
       return false;
   }
}

//------------------------------------------------------------------------------
//!
void
Core::performSetAccelerometerFrequency( float /*hz*/ )
{}

//------------------------------------------------------------------------------
//!
void
Core::startAccelerometer()
{}

//------------------------------------------------------------------------------
//!
void
Core::stopAccelerometer()
{}

//------------------------------------------------------------------------------
//!
void
Core::performChangeOrientation( Orientation /*oldOri*/, Orientation /*newOri*/ )
{}

//------------------------------------------------------------------------------
//!
bool
Core::performOpen( const String& /*url*/ )
{
   StdErr << "Core:performOpen() not implemented." << nl;
   return false;
}

//-----------------------------------------------------------------------------
//!
void
Core::performAsk( FileDialog& /*diag*/ )
{
   StdErr << "Core::performAsk() not implemented." << nl;
}

//------------------------------------------------------------------------------
//!
void
Core::allocateTextures()
{
   // Full resolution textures.
   ResManager::insertRenderableTexture(
      String( "hdrColor1" ),
      gfx()->create2DTexture(
         _size.x, _size.y,
         Gfx::TEX_FMT_16F_16F_16F_16F, Gfx::TEX_CHANS_RGBA, Gfx::TEX_FLAGS_RENDERABLE
      )
   );
   ResManager::insertRenderableTexture(
      String( "color1" ),
      gfx()->create2DTexture(
         _size.x, _size.y,
         Gfx::TEX_FMT_8_8_8_8, Gfx::TEX_CHANS_RGBA, Gfx::TEX_FLAGS_RENDERABLE
      )
   );

   // Quater resolution textures.
   Vec2i lowSize = _size / 4;

   ResManager::insertRenderableTexture(
      String( "color1_4" ),
      gfx()->create2DTexture(
         lowSize.x, lowSize.y,
         Gfx::TEX_FMT_8_8_8_8, Gfx::TEX_CHANS_RGBA, Gfx::TEX_FLAGS_RENDERABLE
      )
   );
   ResManager::insertRenderableTexture(
      String( "color2_4" ),
      gfx()->create2DTexture(
         lowSize.x, lowSize.y,
         Gfx::TEX_FMT_8_8_8_8, Gfx::TEX_CHANS_RGBA, Gfx::TEX_FLAGS_RENDERABLE
      )
   );

   // 1/16th resolution textures.
   Vec2i veryLowSize = lowSize / 4;

   ResManager::insertRenderableTexture(
      String( "color1_16" ),
      gfx()->create2DTexture(
         veryLowSize.x, veryLowSize.y,
         Gfx::TEX_FMT_8_8_8_8, Gfx::TEX_CHANS_RGBA, Gfx::TEX_FLAGS_RENDERABLE
      )
   );
   ResManager::insertRenderableTexture(
      String( "color2_16" ),
      gfx()->create2DTexture(
         veryLowSize.x, veryLowSize.y,
         Gfx::TEX_FMT_8_8_8_8, Gfx::TEX_CHANS_RGBA, Gfx::TEX_FLAGS_RENDERABLE
      )
   );
}

//------------------------------------------------------------------------------
//!
void
Core::initializeGUI()
{
   DBG_BLOCK( os_core, "Core::initializeGUI" );

   PROFILE_EVENT( EventProfiler::GUI_BEGIN );

   if( Core::snd() == nullptr )  Core::snd( Snd::Manager::createManager( _sndAPI ) );

   //if( _desktop.isNull() )
   //{
   //   DBG_MSG( os_core, "no desktop" );
   //   return;
   //}

   _fpsFrames      = 0;
   _fpsMinFrameDur = CGConstd::infinity();
   _fpsMaxFrameDur = 0.0;
   _fpsLastFrame   = 0.0;
   _fpsTimer.restart();

   const String& api = gfx()->API();
   if( api == "OpenGL ES 1.1" )
   {
      _program = data( ResManager::getProgram( "shader/program/colorTexFixed" ) );
   }
   else
   {
      _program = data( ResManager::getProgram( "shader/program/colorTex" ) );
   }
}

//------------------------------------------------------------------------------
//!
void
Core::finalizeGUI()
{
   DBG_BLOCK( os_core, "Core::finalizeGUI" );

   // Global RCPs (unnamed namespace).
   _desktop = nullptr;
   _program = nullptr;
   _pass    = nullptr;
   _rn      = nullptr;

   // Local RCPs.
   _filters = nullptr;

   // Clear events (maybe we received an exit while handling events).
   _events.clear();
   _taskEvents.clear();

   PROFILE_EVENT( EventProfiler::GUI_END );
}

//------------------------------------------------------------------------------
//!
void
Core::addRoot( const Path& path, uint idx )
{
   Path p;
   if( path.isRelative() )
   {
      p = Path::getCurrentDirectory();
      p /= path;
   }
   else
   {
      p = path;
   }
   p.toDir(); // Force a trailing '/' for simpler concatenations.

   Vector<Path>& roots = singleton()._roots;
   if( idx > roots.size() )
   {
      roots.pushBack( p );
   }
   else
   {
      roots.insert( roots.begin() + idx, p );
   }

#if 0
   for( uint i = 0; i < roots.size(); ++i )
   {
      StdErr << i << ": " << roots[i].cstr() << nl;
   }
#endif
}

//------------------------------------------------------------------------------
//!
void
Core::removeRoot( uint idx )
{
   singleton()._roots.erase( singleton()._roots.begin() + idx );
}

//------------------------------------------------------------------------------
//!
EventProfiler&
Core::profiler()
{
   return _profiler;
}

NAMESPACE_END
