/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Fusion/Core/X11/CoreXGL.h>
#include <Fusion/Core/Event.h>
#include <Fusion/Core/EventProfiler.h>
#include <Fusion/Core/Key.h>

#include <Base/Dbg/DebugStream.h>
#include <Base/Util/Timer.h>

#define XK_MISCELLANY
#define XK_LATIN1
#include <X11/keysymdef.h>
#include <X11/cursorfont.h>
#include <X11/Xatom.h>

/*==============================================================================
  UNNAME NAMESPACE
==============================================================================*/

UNNAMESPACE_BEGIN

DBG_STREAM( os_xgl, "CoreXGL" );

uint _count_ch = 0;

//------------------------------------------------------------------------------
//!
int
findKey
( XEvent& ev, char& chr )
{
   KeySym key;

   if( XLookupString( &ev.xkey, &chr, 1, &key, 0 ) == 0 )
   {
      chr = 0;
   }

   // XK_* code listed under /usr/X11/include/X11/keysymdef.h.
   switch( key )
   {
      case XK_space:        return Key::SPACE;
      case XK_quoteright:   return Key::QUOTE;
      case XK_comma:        return Key::COMMA;
      case XK_minus:        return Key::MINUS;
      case XK_period:       return Key::PERIOD;
      case XK_slash:        return Key::SLASH;
      case XK_semicolon:    return Key::SEMICOLON;
      case XK_equal:        return Key::EQUAL;
      case XK_bracketleft:  return Key::LEFT_BRACKET;
      case XK_backslash:    return Key::BACKSLASH;
      case XK_bracketright: return Key::RIGHT_BRACKET;
      case XK_a:            return Key::A;
      case XK_b:            return Key::B;
      case XK_c:            return Key::C;
      case XK_d:            return Key::D;
      case XK_e:            return Key::E;
      case XK_f:            return Key::F;
      case XK_g:            return Key::G;
      case XK_h:            return Key::H;
      case XK_i:            return Key::I;
      case XK_j:            return Key::J;
      case XK_k:            return Key::K;
      case XK_l:            return Key::L;
      case XK_m:            return Key::M;
      case XK_n:            return Key::N;
      case XK_o:            return Key::O;
      case XK_p:            return Key::P;
      case XK_q:            return Key::Q;
      case XK_r:            return Key::R;
      case XK_s:            return Key::S;
      case XK_t:            return Key::T;
      case XK_u:            return Key::U;
      case XK_v:            return Key::V;
      case XK_w:            return Key::W;
      case XK_x:            return Key::X;
      case XK_y:            return Key::Y;
      case XK_z:            return Key::Z;
      case XK_Caps_Lock:    return Key::CAPS_LOCK;
      case XK_Shift_L:      return Key::SHIFT;
      case XK_Shift_R:      return Key::SHIFT;
      case XK_Control_L:    return Key::CTRL;
      case XK_Control_R:    return Key::CTRL;
      case XK_Alt_L:        return Key::ALT;
      case XK_Alt_R:        return Key::ALT;
      case XK_Menu:         return Key::MENU;
      //case XK_Super_L:      return Key::LWIN; //FIXME
      //case XK_Super_R:      return Key::RWIN; //FIXME
      case XK_BackSpace:    return Key::BACKSPACE;
      case XK_Tab:          return Key::TAB;
      case XK_Pause:        return Key::PAUSE;
      case XK_Scroll_Lock:  return Key::SCROLL_LOCK;
      case XK_Return:       return Key::RETURN;
      case XK_Delete:       return Key::DELETE;
      case XK_Escape:       return Key::ESC;
      case XK_Home:         return Key::HOME;
      case XK_Left:         return Key::LEFT_ARROW;
      case XK_Up:           return Key::UP_ARROW;
      case XK_Right:        return Key::RIGHT_ARROW;
      case XK_Down:         return Key::DOWN_ARROW;
      case XK_Page_Up:      return Key::PAGE_UP;
      case XK_Page_Down:    return Key::PAGE_DOWN;
      case XK_End:          return Key::END;
      case XK_Print:        return Key::PRINT;
      case XK_Insert:       return Key::INSERT;
      case XK_Mode_switch:  return Key::ALT;
      case XK_Num_Lock:     return Key::NUM_LOCK;
      case XK_F1:           return Key::F1;
      case XK_F2:           return Key::F2;
      case XK_F3:           return Key::F3;
      case XK_F4:           return Key::F4;
      case XK_F5:           return Key::F5;
      case XK_F6:           return Key::F6;
      case XK_F7:           return Key::F7;
      case XK_F8:           return Key::F8;
      case XK_F9:           return Key::F9;
      case XK_F10:          return Key::F10;
      case XK_F11:          return Key::F11;
      case XK_F12:          return Key::F12;
      default: return key;
   }

   return key;
}

//------------------------------------------------------------------------------
//!
Core*   _single;
Cursor  _pointer[Pointer::_COUNT];

UNNAMESPACE_END

NAMESPACE_BEGIN

/*==============================================================================
  CLASS CoreXGL
==============================================================================*/

Vec2i   CoreXGL::_size( 0, 0 );
uint    CoreXGL::_mainPointerID = 0;

//------------------------------------------------------------------------------
//!
void
Core::initialize()
{
   _single = new CoreXGL();
}

//------------------------------------------------------------------------------
//!
void
Core::finalize()
{
   delete _single;
   _single = NULL;
}

//------------------------------------------------------------------------------
//!
void
Core::printInfo( TextStream& os )
{
   os << "Core: X11" << nl;
}

//------------------------------------------------------------------------------
//!
CoreXGL::CoreXGL()
   : _dblClickDelay( 300 )
{
   DBG_BLOCK( os_xgl, "CoreXGL::CoreXGL" );

   addRoot( "" );

   _mainPointerID = Core::createPointer().id();

   // Create window.
   initWin();
}

//------------------------------------------------------------------------------
//!
CoreXGL::~CoreXGL()
{
   DBG_BLOCK( os_xgl, "CoreXGL::~CoreXGL" );
}

//------------------------------------------------------------------------------
//!
void
CoreXGL::performExec()
{
   DBG_BLOCK( os_xgl, "CoreXGL::performExec" );

   // Create Gfx manager and context.
   RCP<Gfx::GLContext_GLX> cntx = new Gfx::GLContext_GLX( _display, _window );
   Core::gfx( Gfx::Manager::create( cntx.ptr() ) );

   initializeGUI();
   readyToExec();
   performShow();

   bool done = false;
   XEvent event;

   _timer.restart();

#if PROFILE_EVENTS
   EventProfiler& profiler = Core::profiler();
   profiler.add( EventProfiler::LOOPS_BEGIN );
#endif

   while( !done )
   {
#if PROFILE_EVENTS
      profiler.add( EventProfiler::LOOP_BEGIN );
#endif

      while( XPending( _display ) > 0 )
      {
         XNextEvent( _display, &event );
         done = handleEvents( event );
      }

      processEvents();

      executeAnimators( _timer.elapsed() );

      render();

#if PROFILE_EVENTS
      profiler.add( EventProfiler::LOOP_END );
#endif
   }

#if PROFILE_EVENTS
   profiler.add( EventProfiler::LOOPS_BEGIN );
#endif

   performHide();
   finalizeGUI();
}

//------------------------------------------------------------------------------
//!
void
CoreXGL::performExit()
{
   XEvent ev;
   memset( &ev, 0, sizeof(ev) );
   ev.xclient.type      = ClientMessage;
   ev.xclient.window    = _window;
   ev.xclient.format    = 32;
   ev.xclient.data.l[0] = _wmDeleteWindow;
   XSendEvent( _display, _window, False, StructureNotifyMask, &ev );
}

//------------------------------------------------------------------------------
//!
void
CoreXGL::performShow()
{
   DBG_BLOCK( os_xgl, "CoreXGL::performShow" );

   // Show... todo
}

//------------------------------------------------------------------------------
//!
void
CoreXGL::performHide()
{
   DBG_BLOCK( os_xgl, "CoreXGL::performHide" );
}

//------------------------------------------------------------------------------
//!
void
CoreXGL::performRedraw()
{
   DBG_BLOCK( os_xgl, "CoreXGL::performRedraw" );
}

//------------------------------------------------------------------------------
//!
void
CoreXGL::performGrabPointer( uint pointerID )
{
   if( pointerID != _mainPointerID )
   {
      StdErr << "WARNING - Don't know what to do with non-main pointer grabbing." << nl;
      return;
   }

   XGrabPointer(
      _display, _window,
      True,
      ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
      GrabModeAsync, GrabModeAsync,
      None, None, CurrentTime
   );
}

//------------------------------------------------------------------------------
//!
void
CoreXGL::performReleasePointer( uint pointerID )
{
   if( pointerID != _mainPointerID )
   {
      StdErr << "WARNING - Don't know what to do with non-main pointer release." << nl;
      return;
   }

   XUngrabPointer( _display, CurrentTime );
}

//------------------------------------------------------------------------------
//!
void
CoreXGL::performSetPointerIcon( uint pointerID, uint icon )
{
   if( pointerID != _mainPointerID )
   {
      StdErr << "WARNING - Don't know what to do with non-main pointer icon setting." << nl;
      return;
   }

   XDefineCursor( _display, _window, _pointer[icon] );
}

//------------------------------------------------------------------------------
//!
void
CoreXGL::initWin()
{
   DBG_BLOCK( os_xgl, "CoreXGL::initWin" );

   // Open a connection to the X Server.
   _display = XOpenDisplay( getenv("DISPLAY") );

   if( _display == 0 )
   {
      DBG_MSG( os_xgl, "No display connection." );
      return;
   }

   // Window creation.
   XSetWindowAttributes attr;

   attr.background_pixel = 0;
   attr.border_pixel     = 0;
   attr.colormap         = CopyFromParent;
   attr.event_mask =
      ExposureMask | StructureNotifyMask |
      KeyPressMask | KeyReleaseMask |
      ButtonPressMask | ButtonReleaseMask | PointerMotionMask;

   unsigned long mask = CWBackPixel | CWBorderPixel | CWColormap | CWEventMask;


   if( fullScreen() )
   {
      int defScreen = DefaultScreen( _display );
      _size.x = XDisplayWidth( _display, defScreen );
      _size.y = XDisplayHeight( _display, defScreen );
   }
   else
   {
      _size = size();
   }

   _window = XCreateWindow(
      _display,
      DefaultRootWindow( _display ),
      0, 0,
      _size.x, _size.y,
      0,
      CopyFromParent,
      InputOutput,
      CopyFromParent,
      mask,
      &attr
   );

   // Interaction with the WM.
   _wmDeleteWindow = XInternAtom( _display, "WM_DELETE_WINDOW", True );
   XSetWMProtocols( _display, _window, &_wmDeleteWindow, 1 );

   // Load pointer.
   _pointer[Pointer::DEFAULT]  = XCreateFontCursor( _display, XC_left_ptr );
   _pointer[Pointer::SIZE_ALL] = XCreateFontCursor( _display, XC_center_ptr );

   _pointer[Pointer::SIZE_T]   = XCreateFontCursor( _display, XC_top_side );
   _pointer[Pointer::SIZE_B]   = XCreateFontCursor( _display, XC_bottom_side );
   _pointer[Pointer::SIZE_BL]  = XCreateFontCursor( _display, XC_bottom_left_corner );
   _pointer[Pointer::SIZE_BR]  = XCreateFontCursor( _display, XC_bottom_right_corner );
   _pointer[Pointer::SIZE_TL]  = XCreateFontCursor( _display, XC_top_left_corner );
   _pointer[Pointer::SIZE_TR]  = XCreateFontCursor( _display, XC_top_right_corner );
   _pointer[Pointer::SIZE_L]   = XCreateFontCursor( _display, XC_left_side );
   _pointer[Pointer::SIZE_R]   = XCreateFontCursor( _display, XC_right_side );

   _pointer[Pointer::TEXT]     = XCreateFontCursor( _display, XC_xterm );
   _pointer[Pointer::HAND]     = XCreateFontCursor( _display, XC_hand1 );
   _pointer[Pointer::WAIT]     = XCreateFontCursor( _display, XC_watch );

   _pointer[Pointer::INVALID]  = _pointer[Pointer::DEFAULT];

   XMapRaised( _display, _window );

   // Resize window to cover the screen.
   if( fullScreen() )
   {
      // No border.
      Atom wmhints = XInternAtom( _display, "_MOTIF_WM_HINTS", False );

      struct {
         unsigned long flags;
         unsigned long functions;
         unsigned long decorations;
         long input_mode;
         unsigned long status;
      } MWMHints = { (1L << 1), 0, 0, 0, 0 };

      XChangeProperty(
         _display,
         _window,
         wmhints, wmhints,
         32, PropModeReplace,
         (uchar*)&MWMHints, 5
      );
      // Resizing and moving.
      XMoveResizeWindow( _display, _window, 0, 0, _size.x, _size.y );

      Atom props[6];
      props[0] = XInternAtom( _display, "_NET_WM_STATE_FULLSCREEN", False );
      props[1] = XInternAtom( _display, "_NET_WM_STATE_SKIP_TASKBAR", False );
      props[2] = XInternAtom( _display, "_NET_WM_STATE_SKIP_PAGER", False );
      props[3] = XInternAtom( _display, "_NET_WM_STATE_ABOVE", False );
      props[4] = XInternAtom( _display, "_NET_WM_STATE_STICKY", False );
      props[5] = 0;
      XChangeProperty(
         _display,
         _window,
         XInternAtom( _display, "_NET_WM_STATE", False ),
         XA_ATOM,
         32, PropModeReplace,
         (unsigned char*)props, 4 // Use 5 if you want sticky!!
      );
   }
}

//------------------------------------------------------------------------------
//!
void
CoreXGL::resize
( int w, int h )
{
   DBG_BLOCK( os_xgl, "CoreXGL::resize" );

   _size.x = w;
   _size.y = h;

   performResize( w, h );
}

//------------------------------------------------------------------------------
//!
void
CoreXGL::render()
{
   performRender();
}


//------------------------------------------------------------------------------
//!
bool
CoreXGL::handleEvents( XEvent& ev )
{
   DBG_BLOCK( os_xgl, "CoreXGL::handleEvents" );

   static char keys[32];
   static uint repeatKey = 0xffff;
   static Time lastButtonPress = 0x0;

   switch( ev.type )
   {
      case Expose:
      {
         //render();
      } break;

      case ConfigureNotify:
      {
         resize( ev.xconfigure.width, ev.xconfigure.height );
      } break;

      case KeyPress:
      {
         char chr;
         int key   = findKey( ev, chr );
         uint code = ev.xkey.keycode;
         _count_ch = (code == repeatKey) ? _count_ch+1 : 1;
#undef KeyPress
         submitEvent( Event::KeyPress( Core::lastTime(), key ) );

         if( chr != 0 )
         {
            submitEvent( Event::Char( Core::lastTime(), chr, _count_ch ) );
         }
      } break;

      case KeyRelease:
      {
         // Detects and remove repeated keys.
         XQueryKeymap( _display, keys );
         uint code = ev.xkey.keycode;
         if( keys[code/8] & ( 1 << (code%8) ) )
         {
            repeatKey = code;
         }
         else
         {
            repeatKey = 0xffff;
            char chr;
#undef KeyRelease
            submitEvent(
               Event::KeyRelease( Core::lastTime(), findKey( ev, chr ) )
            );
         }
      } break;


      case ButtonPress:
      {
         Vec2i pos( ev.xbutton.x, _size(1) - ev.xbutton.y - 1 );
         if( ev.xbutton.button == 4 )
         {
            submitEvent(
               Event::PointerScroll( Core::lastTime(), _mainPointerID, +1, pos )
            );
         }
         else
         if( ev.xbutton.button == 5 )
         {
            submitEvent(
               Event::PointerScroll( Core::lastTime(), _mainPointerID, -1, pos )
            );
         }
         else
         if( ev.xbutton.time - lastButtonPress < _dblClickDelay )
         {
            submitEvent(
               Event::PointerPress( Core::lastTime(), _mainPointerID, ev.xbutton.button, pos, Core::pointer(_mainPointerID).lastPressEvent().count()+1u )
            );
         }
         else
         {
            submitEvent(
               Event::PointerPress( Core::lastTime(), _mainPointerID, ev.xbutton.button, pos, 1 )
            );
         }
         lastButtonPress = ev.xbutton.time;
      } break;

      case ButtonRelease:
      {
         if( ev.xbutton.button == 4 || ev.xbutton.button == 5 )
         {
            break;
         }
         Vec2i pos( ev.xbutton.x, _size(1) - ev.xbutton.y - 1 );
         submitEvent(
            Event::PointerRelease( Core::lastTime(), _mainPointerID, ev.xbutton.button, pos, Core::pointer(_mainPointerID).lastPressEvent().count() )
         );
      } break;

      case MotionNotify:
      {
         Vec2i pos( ev.xmotion.x, _size(1) - ev.xmotion.y - 1 );
         submitEvent( Event::PointerMove( Core::lastTime(), _mainPointerID, pos ) );
         DBG_MSG( os_xgl, "Pointer Move: " << pos );
      } break;

      case ClientMessage:
      {
         DBG_MSG( os_xgl, "Client message" );

         if( (Atom)ev.xclient.data.l[0] == _wmDeleteWindow )
         {
            DBG_MSG( os_xgl, "Quit" );
            return true;
         }
      } break;
   }

   return false;
}

NAMESPACE_END
