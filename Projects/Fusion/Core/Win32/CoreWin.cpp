/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
// Define windows version for IDC_HAND MSVC 6.0

// Windows 7                                          _WIN32_WINNT_WIN7  (0x0601)
// Windows Server 2008                                _WIN32_WINNT_WS08  (0x0600)
// Windows Vista                                      _WIN32_WINNT_VISTA (0x0600)
// Windows Server 2003 with SP1, Windows XP with SP2  _WIN32_WINNT_WS03  (0x0502)
// Windows Server 2003, Windows XP                    _WIN32_WINNT_WINXP (0x0501)
// Windows 2000                                       _WIN32_WINNT_WIN2K (0x0500)

#ifndef WINVER
#define WINVER 0x0600
#endif

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif

#include <Fusion/Core/Win32/CoreWin.h>
#include <Fusion/Core/Key.h>
#include <Fusion/Core/Event.h>
#include <Fusion/Core/EventProfiler.h>
#include <Fusion/Widget/FileDialog.h>

#include <Gfx/Mgr/D3D/D3DContext.h>
#include <Gfx/Mgr/GL/GLContext_WGL.h>
#include <Gfx/Mgr/GLES/GLESContext_EGL.h>

#include <Base/Dbg/DebugStream.h>
#include <Base/IO/FileSystem.h>
#include <Base/Util/Timer.h>

#include <CdErr.h>
#include <Commdlg.h>
#if !defined(__CYGWIN__)
#include <Tchar.h>
#endif

USING_NAMESPACE

/*==============================================================================
  UNNAME NAMESPACE
==============================================================================*/

UNNAMESPACE_BEGIN

DBG_STREAM( os_win, "CoreWin" );

CoreWin* _single;
HCURSOR  _pointer[Pointer::_COUNT];
bool     _init = false;
double   _doubleClickDelay = 0.5;
float    _doubleClickRange = 4.0f;

uint _count_ch = 0;

//-----------------------------------------------------------------------------
//!
void applyOptions( const FileDialog& src, OPENFILENAME& ofn )
{
   unused(src);
   unused(ofn);
   //if( !src.title().empty()   )  ofn.lpstrTitle = src.title().cstr();

#if 0
   if( !src.prompt().empty()  )  [dst  setPrompt: convert(src.prompt()) ];

   if( !src.message().empty() )  [dst setMessage: convert(src.message())];

   if( !src.startingLocation().empty() )
   {
      FS::Entry entry( src.startingLocation() );
      if( entry.type() == FS::TYPE_DIRECTORY )
      {
         NSString* str = [NSString stringWithUTF8String: entry.path().cstr()];
         NSURL*    url = [NSURL fileURLWithPath: str isDirectory: YES];
         [dst setDirectoryURL: url];
      }
      else
      {
         Path d = entry.path();
         Path f = d.split();
         NSString* str = [NSString stringWithUTF8String: d.cstr()];
         NSURL*    url = [NSURL fileURLWithPath: str isDirectory: YES];
         [dst setDirectoryURL: url];
         str = [NSString stringWithUTF8String: f.cstr()];
         [dst setNameFieldStringValue: str];
      }
   }

   const Vector<String>& types = src.allowedTypes();
   if( !types.empty() )
   {
      NSMutableArray* array = [NSMutableArray arrayWithCapacity: types.size()];
      for( auto cur = types.begin(); cur != types.end(); ++cur )
      {
         [array addObject: convert( *cur )];
      }
      [dst setAllowedFileTypes: array];
   }

   [dst setShowsHiddenFiles: src.showHidden()];

   [dst setCanCreateDirectories: src.canCreateDirectories()];

   if( !src.message().empty() )  [dst setMessage: convert(src.message())];
   [dst setAllowsOtherFileTypes: YES]; // Always allow to override the extension to our choice.
#endif
}

//------------------------------------------------------------------------------
//!
bool
changeResolution( const Vec2i& size, int pixelDepth )
{
   DEVMODE dmScreenSettings;
   memset( &dmScreenSettings, 0, sizeof( dmScreenSettings) );
   dmScreenSettings.dmSize       = sizeof( dmScreenSettings );
   dmScreenSettings.dmPelsWidth  = size.x;
   dmScreenSettings.dmPelsHeight = size.y;
   dmScreenSettings.dmBitsPerPel = pixelDepth;
   dmScreenSettings.dmFields     = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

   return ChangeDisplaySettings( &dmScreenSettings, CDS_FULLSCREEN ) !=  DISP_CHANGE_SUCCESSFUL;
}


int  _wparamsToKeyTable[] = {
   // Fusion               WinUser.h               hex    description
   0                 ,  // VK_LBUTTON              0x00 : undefined
   0                 ,  // VK_LBUTTON              0x01
   0                 ,  // VK_RBUTTON              0x02
   Key::CANCEL       ,  // VK_CANCEL               0x03
   0                 ,  // VK_MBUTTON              0x04    /* NOT contiguous with L & RBUTTON */
   0                 ,  // VK_XBUTTON1             0x05    /* NOT contiguous with L & RBUTTON */
   0                 ,  // VK_XBUTTON2             0x06    /* NOT contiguous with L & RBUTTON */
   0                 ,  //                         0x07 : unassigned
   Key::BACKSPACE    ,  // VK_BACK                 0x08
   Key::TAB          ,  // VK_TAB                  0x09
   0                 ,  //                         0x0A : reserved
   0                 ,  //                         0x0B : reserved
   0                 ,  // VK_CLEAR                0x0C
   Key::RETURN       ,  // VK_RETURN               0x0D
   0                 ,  //                         0x0E : unassigned
   0                 ,  //                         0x0F : unassigned
   Key::SHIFT        ,  // VK_SHIFT                0x10
   Key::CTRL         ,  // VK_CONTROL              0x11
   Key::ALT          ,  // VK_MENU                 0x12
   Key::PAUSE        ,  // VK_PAUSE                0x13
   Key::CAPS_LOCK    ,  // VK_CAPITAL              0x14
   0                 ,  // VK_KANA                 0x15 (also VK_HANGEUL, VK_HANGUL)
   0                 ,  //                         0x16 : undefined
   0                 ,  // VK_JUNJA                0x17
   0                 ,  // VK_FINAL                0x18
   0                 ,  // VK_HANJA                0x19 (also VK_KANJI)
   0                 ,  //                         0x1A : undefined
   Key::ESCAPE       ,  // VK_ESCAPE               0x1B
   0                 ,  // VK_CONVERT              0x1C
   0                 ,  // VK_NONCONVERT           0x1D
   0                 ,  // VK_ACCEPT               0x1E
   0                 ,  // VK_MODECHANGE           0x1F
   Key::SPACE        ,  // VK_SPACE                0x20
   Key::PAGE_UP      ,  // VK_PRIOR                0x21
   Key::PAGE_DOWN    ,  // VK_NEXT                 0x22
   Key::END          ,  // VK_END                  0x23
   Key::HOME         ,  // VK_HOME                 0x24
   Key::LEFT_ARROW   ,  // VK_LEFT                 0x25
   Key::UP_ARROW     ,  // VK_UP                   0x26
   Key::RIGHT_ARROW  ,  // VK_RIGHT                0x27
   Key::DOWN_ARROW   ,  // VK_DOWN                 0x28
   0                 ,  // VK_SELECT               0x29
   Key::PRINT        ,  // VK_PRINT                0x2A
   0                 ,  // VK_EXECUTE              0x2B
   0                 ,  // VK_SNAPSHOT             0x2C
   Key::INSERT       ,  // VK_INSERT               0x2D
   Key::DELETE       ,  // VK_DELETE               0x2E
   0                 ,  // VK_HELP                 0x2F
   Key::_0           ,  // VK_0                    0x30
   Key::_1           ,  // VK_1                    0x31
   Key::_2           ,  // VK_2                    0x32
   Key::_3           ,  // VK_3                    0x33
   Key::_4           ,  // VK_4                    0x34
   Key::_5           ,  // VK_5                    0x35
   Key::_6           ,  // VK_6                    0x36
   Key::_7           ,  // VK_7                    0x37
   Key::_8           ,  // VK_8                    0x38
   Key::_9           ,  // VK_9                    0x39
   0                 ,  //                         0x3A : unassigned
   0                 ,  //                         0x3B : unassigned
   0                 ,  //                         0x3C : unassigned
   0                 ,  //                         0x3D : unassigned
   0                 ,  //                         0x3E : unassigned
   0                 ,  //                         0x3F : unassigned
   0                 ,  //                         0x40 : unassigned
   Key::A            ,  // VK_A                    0x41
   Key::B            ,  // VK_B                    0x42
   Key::C            ,  // VK_C                    0x43
   Key::D            ,  // VK_D                    0x44
   Key::E            ,  // VK_E                    0x45
   Key::F            ,  // VK_F                    0x46
   Key::G            ,  // VK_G                    0x47
   Key::H            ,  // VK_H                    0x48
   Key::I            ,  // VK_I                    0x49
   Key::J            ,  // VK_J                    0x4A
   Key::K            ,  // VK_K                    0x4B
   Key::L            ,  // VK_L                    0x4C
   Key::M            ,  // VK_M                    0x4D
   Key::N            ,  // VK_N                    0x4E
   Key::O            ,  // VK_O                    0x4F
   Key::P            ,  // VK_P                    0x50
   Key::Q            ,  // VK_Q                    0x51
   Key::R            ,  // VK_R                    0x52
   Key::S            ,  // VK_S                    0x53
   Key::T            ,  // VK_T                    0x54
   Key::U            ,  // VK_U                    0x55
   Key::V            ,  // VK_V                    0x56
   Key::W            ,  // VK_W                    0x57
   Key::X            ,  // VK_X                    0x58
   Key::Y            ,  // VK_Y                    0x59
   Key::Z            ,  // VK_Z                    0x5A
   Key::MENU         ,  // VK_LWIN                 0x5B
   Key::MENU         ,  // VK_RWIN                 0x5C
   0                 ,  // VK_APPS                 0x5D
   0                 ,  //                         0x5E : reserved
   0                 ,  // VK_SLEEP                0x5F
   Key::NUM_0        ,  // VK_NUMPAD0              0x60
   Key::NUM_1        ,  // VK_NUMPAD1              0x61
   Key::NUM_2        ,  // VK_NUMPAD2              0x62
   Key::NUM_3        ,  // VK_NUMPAD3              0x63
   Key::NUM_4        ,  // VK_NUMPAD4              0x64
   Key::NUM_5        ,  // VK_NUMPAD5              0x65
   Key::NUM_6        ,  // VK_NUMPAD6              0x66
   Key::NUM_7        ,  // VK_NUMPAD7              0x67
   Key::NUM_8        ,  // VK_NUMPAD8              0x68
   Key::NUM_9        ,  // VK_NUMPAD9              0x69
   Key::ASTERISK     ,  // VK_MULTIPLY             0x6A
   Key::PLUS         ,  // VK_ADD                  0x6B
   Key::COMMA        ,  // VK_SEPARATOR            0x6C
   Key::MINUS        ,  // VK_SUBTRACT             0x6D
   Key::PERIOD       ,  // VK_DECIMAL              0x6E
   Key::SLASH        ,  // VK_DIVIDE               0x6F
   Key::F1           ,  // VK_F1                   0x70
   Key::F2           ,  // VK_F2                   0x71
   Key::F3           ,  // VK_F3                   0x72
   Key::F4           ,  // VK_F4                   0x73
   Key::F5           ,  // VK_F5                   0x74
   Key::F6           ,  // VK_F6                   0x75
   Key::F7           ,  // VK_F7                   0x76
   Key::F8           ,  // VK_F8                   0x77
   Key::F9           ,  // VK_F9                   0x78
   Key::F10          ,  // VK_F10                  0x79
   Key::F11          ,  // VK_F11                  0x7A
   Key::F12          ,  // VK_F12                  0x7B
   Key::F13          ,  // VK_F13                  0x7C
   Key::F14          ,  // VK_F14                  0x7D
   Key::F15          ,  // VK_F15                  0x7E
   Key::F16          ,  // VK_F16                  0x7F
   Key::F17          ,  // VK_F17                  0x80
   Key::F18          ,  // VK_F18                  0x81
   Key::F19          ,  // VK_F19                  0x82
   Key::F20          ,  // VK_F20                  0x83
   Key::F21          ,  // VK_F21                  0x84
   Key::F22          ,  // VK_F22                  0x85
   Key::F23          ,  // VK_F23                  0x86
   Key::F24          ,  // VK_F24                  0x87
   0                 ,  //                         0x88 : unassigned
   0                 ,  //                         0x89 : unassigned
   0                 ,  //                         0x8A : unassigned
   0                 ,  //                         0x8B : unassigned
   0                 ,  //                         0x8C : unassigned
   0                 ,  //                         0x8D : unassigned
   0                 ,  //                         0x8E : unassigned
   0                 ,  //                         0x8F : unassigned
   Key::NUM_LOCK     ,  // VK_NUMLOCK              0x90
   Key::SCROLL_LOCK  ,  // VK_SCROLL               0x91
   0                 ,  // VK_OEM_FJ_JISHO         0x92   // 'Dictionary' key (also VK_OEM_NEC_EQUAL)
   0                 ,  // VK_OEM_FJ_MASSHOU       0x93   // 'Unregister word' key
   0                 ,  // VK_OEM_FJ_TOUROKU       0x94   // 'Register word' key
   0                 ,  // VK_OEM_FJ_LOYA          0x95   // 'Left OYAYUBI' key
   0                 ,  // VK_OEM_FJ_ROYA          0x96   // 'Right OYAYUBI' key
   0                 ,  //                         0x97 : unassigned
   0                 ,  //                         0x98 : unassigned
   0                 ,  //                         0x99 : unassigned
   0                 ,  //                         0x9A : unassigned
   0                 ,  //                         0x9B : unassigned
   0                 ,  //                         0x9C : unassigned
   0                 ,  //                         0x9D : unassigned
   0                 ,  //                         0x9E : unassigned
   0                 ,  //                         0x9F : unassigned
   Key::SHIFT        ,  // VK_LSHIFT               0xA0
   Key::SHIFT        ,  // VK_RSHIFT               0xA1
   Key::CTRL         ,  // VK_LCONTROL             0xA2
   Key::CTRL         ,  // VK_RCONTROL             0xA3
   Key::MENU         ,  // VK_LMENU                0xA4
   Key::MENU         ,  // VK_RMENU                0xA5
   0                 ,  // VK_BROWSER_BACK         0xA6
   0                 ,  // VK_BROWSER_FORWARD      0xA7
   0                 ,  // VK_BROWSER_REFRESH      0xA8
   0                 ,  // VK_BROWSER_STOP         0xA9
   0                 ,  // VK_BROWSER_SEARCH       0xAA
   0                 ,  // VK_BROWSER_FAVORITES    0xAB
   0                 ,  // VK_BROWSER_HOME         0xAC
   0                 ,  // VK_VOLUME_MUTE          0xAD
   0                 ,  // VK_VOLUME_DOWN          0xAE
   0                 ,  // VK_VOLUME_UP            0xAF
   0                 ,  // VK_MEDIA_NEXT_TRACK     0xB0
   0                 ,  // VK_MEDIA_PREV_TRACK     0xB1
   0                 ,  // VK_MEDIA_STOP           0xB2
   0                 ,  // VK_MEDIA_PLAY_PAUSE     0xB3
   0                 ,  // VK_LAUNCH_MAIL          0xB4
   0                 ,  // VK_LAUNCH_MEDIA_SELECT  0xB5
   0                 ,  // VK_LAUNCH_APP1          0xB6
   0                 ,  // VK_LAUNCH_APP2          0xB7
   0                 ,  //                         0xB8 : reserved
   0                 ,  //                         0xB9 : reserved
   Key::SEMICOLON    ,  // VK_OEM_1                0xBA   // ';:' for US
   Key::PLUS         ,  // VK_OEM_PLUS             0xBB   // '+' any country
   Key::COMMA        ,  // VK_OEM_COMMA            0xBC   // ',' any country
   Key::MINUS        ,  // VK_OEM_MINUS            0xBD   // '-' any country
   Key::PERIOD       ,  // VK_OEM_PERIOD           0xBE   // '.' any country
   Key::SLASH        ,  // VK_OEM_2                0xBF   // '/?' for US
   Key::BACKQUOTE    ,  // VK_OEM_3                0xC0   // '`~' for US
   0                 ,  //                         0xC1 : reserved
   0                 ,  //                         0xC2 : reserved
   0                 ,  //                         0xC3 : reserved
   0                 ,  //                         0xC4 : reserved
   0                 ,  //                         0xC5 : reserved
   0                 ,  //                         0xC6 : reserved
   0                 ,  //                         0xC7 : reserved
   0                 ,  //                         0xC8 : reserved
   0                 ,  //                         0xC9 : reserved
   0                 ,  //                         0xCA : reserved
   0                 ,  //                         0xCB : reserved
   0                 ,  //                         0xCC : reserved
   0                 ,  //                         0xCD : reserved
   0                 ,  //                         0xCE : reserved
   0                 ,  //                         0xCF : reserved
   0                 ,  //                         0xD0 : reserved
   0                 ,  //                         0xD1 : reserved
   0                 ,  //                         0xD2 : reserved
   0                 ,  //                         0xD3 : reserved
   0                 ,  //                         0xD4 : reserved
   0                 ,  //                         0xD5 : reserved
   0                 ,  //                         0xD6 : reserved
   0                 ,  //                         0xD7 : reserved
   0                 ,  //                         0xD8 : unassigned
   0                 ,  //                         0xD9 : unassigned
   0                 ,  //                         0xDA : unassigned
   Key::LEFT_BRACKET ,  // VK_OEM_4                0xDB  //  '[{' for US
   Key::BACKSLASH    ,  // VK_OEM_5                0xDC  //  '\|' for US
   Key::RIGHT_BRACKET,  // VK_OEM_6                0xDD  //  ']}' for US
   Key::APOSTROPHE   ,  // VK_OEM_7                0xDE  //  ''"' for US
   0                 ,  // VK_OEM_8                0xDF
   0                 ,  //                         0xE0 : reserved
   0                 ,  // VK_OEM_AX               0xE1  //  'AX' key on Japanese AX kbd
   0                 ,  // VK_OEM_102              0xE2  //  "<>" or "\|" on RT 102-key kbd.
   0                 ,  // VK_ICO_HELP             0xE3  //  Help key on ICO
   0                 ,  // VK_ICO_00               0xE4  //  00 key on ICO
   0                 ,  // VK_PROCESSKEY           0xE5
   0                 ,  // VK_ICO_CLEAR            0xE6
   0                 ,  // VK_PACKET               0xE7
   0                 ,  //                         0xE8 : unassigned
   0                 ,  // VK_OEM_RESET            0xE9
   0                 ,  // VK_OEM_JUMP             0xEA
   0                 ,  // VK_OEM_PA1              0xEB
   0                 ,  // VK_OEM_PA2              0xEC
   0                 ,  // VK_OEM_PA3              0xED
   0                 ,  // VK_OEM_WSCTRL           0xEE
   0                 ,  // VK_OEM_CUSEL            0xEF
   0                 ,  // VK_OEM_ATTN             0xF0
   0                 ,  // VK_OEM_FINISH           0xF1
   0                 ,  // VK_OEM_COPY             0xF2
   0                 ,  // VK_OEM_AUTO             0xF3
   0                 ,  // VK_OEM_ENLW             0xF4
   0                 ,  // VK_OEM_BACKTAB          0xF5
   0                 ,  // VK_ATTN                 0xF6
   0                 ,  // VK_CRSEL                0xF7
   0                 ,  // VK_EXSEL                0xF8
   0                 ,  // VK_EREOF                0xF9
   0                 ,  // VK_PLAY                 0xFA
   0                 ,  // VK_ZOOM                 0xFB
   0                 ,  // VK_NONAME               0xFC
   0                 ,  // VK_PA1                  0xFD
   0                 ,  // VK_OEM_CLEAR            0xFE
   0                 ,  //                         0xFF : reserved
};

int _keyToVirtKeyTable[] = {
   0            , //   NUL               =   0, // NULL
   0            , //   SOH               =   1, // START OF HEADING
   0            , //   STX               =   2, // START OF TEXT
   0            , //   ETX               =   3, // END OF TEXT
   0            , //   EOT               =   4, // END OF TRANSMISSION
   0            , //   ENQ               =   5, // ENQUIRY
   0            , //   ACK               =   6, // ACKNOWLEDGE
   0            , //   BEL               =   7, // BELL
   VK_BACK      , //   BACKSPACE         =   8, //***
   VK_TAB       , //   TAB               =   9, //***
   VK_RETURN    , //   NEWLINE           =  10, //*** NEW LINE
   0            , //   VTAB              =  11, //*** VERTICAL TABULATION
   0            , //   FORMFEED          =  12, //***
   VK_RETURN    , //   RETURN            =  13, //***
   0            , //   SO                =  14, // SHIFT OUT (LOCKING-SHIFT ONE)
   0            , //   SI                =  15, // SHIFT IN (LOCKING-SHIFT ZERO)
   0            , //   DLE               =  16, // DATA LINK ESCAPE
   0            , //   DC1               =  17, // DEVICE CONTROL ONE
   0            , //   DC2               =  18, // DEVICE CONTROL TWO
   0            , //   DC3               =  19, // DEVICE CONTROL THREE
   0            , //   DC4               =  20, // DEVICE CONTROL FOUR
   0            , //   NAK               =  21, // NEGATIVE ACKNOWLEDGE
   0            , //   SYN               =  22, // SYNCHRONOUS IDLE
   0            , //   ETB               =  23, // END OF TRANSMISSION BLOCK
   VK_CANCEL    , //   CANCEL            =  24, // CANCEL
   0            , //   EM                =  25, // END OF MEDIUM
   0            , //   SUB               =  26, // SUBSTITUTE
   VK_ESCAPE    , //   ESC               =  27, // ESCAPE
   0            , //   FS                =  28, // FILE SEPARATOR
   0            , //   GS                =  29, // GROUP SEPARATOR
   0            , //   RS                =  30, // RECORD SEPARATOR
   0            , //   US                =  31, // UNIT SEPARATOR
   VK_SPACE     , //   SPACE             =  32, // SPACE
   0            , //   EXCLAMATION_MARK  =  33, // EXCLAMATION MARK
   VK_OEM_7     , //   QUOTE             =  34, // QUOTATION MARK
   0            , //   HASH              =  35, // NUMBER SIGN
   0            , //   DOLLAR            =  36, // DOLLAR SIGN
   0            , //   PERCENT           =  37, // PERCENT SIGN
   0            , //   AMPERSAND         =  38, // AMPERSAND
   VK_OEM_7     , //   APOSTROPHE        =  39, // APOSTROPHE
   0            , //   LEFT_PARENTHESIS  =  40, // LEFT PARENTHESIS
   0            , //   RIGHT_PARENTHESIS =  41, // RIGHT PARENTHESIS
   VK_MULTIPLY  , //   ASTERISK          =  42, // ASTERISK
   VK_OEM_PLUS  , //   PLUS              =  43, // PLUS SIGN
   VK_OEM_COMMA , //   COMMA             =  44, // COMMA
   VK_OEM_MINUS , //   MINUS             =  45, // HYPHEN-MINUS
   VK_OEM_PERIOD, //   PERIOD            =  46, //***
   VK_OEM_2     , //   SLASH             =  47, // SOLIDUS (SLASH)
   '0'          , //   DIGIT_0           =  48, // DIGIT ZERO
   '1'          , //   DIGIT_1           =  49, // DIGIT ONE
   '2'          , //   DIGIT_2           =  50, // DIGIT TWO
   '3'          , //   DIGIT_3           =  51, // DIGIT THREE
   '4'          , //   DIGIT_4           =  52, // DIGIT FOUR
   '5'          , //   DIGIT_5           =  53, // DIGIT FIVE
   '6'          , //   DIGIT_6           =  54, // DIGIT SIX
   '7'          , //   DIGIT_7           =  55, // DIGIT SEVEN
   '8'          , //   DIGIT_8           =  56, // DIGIT EIGHT
   '9'          , //   DIGIT_9           =  57, // DIGIT NINE
   VK_OEM_1     , //   COLON             =  58, // COLON
   VK_OEM_1     , //   SEMICOLON         =  59, // SEMICOLON
   0            , //   LT                =  60, // LESS-THAN SIGN
   0            , //   EQ                =  61, // EQUALS SIGN
   0            , //   GT                =  62, // GREATER-THAN SIGN
   0            , //   QUESTION_MARK     =  63, // QUESTION MARK
   0            , //   AT                =  64, // COMMERCIAL AT
   'A'          , //   A                 =  65, // LATIN CAPITAL LETTER A
   'B'          , //   B                 =  66, // LATIN CAPITAL LETTER B
   'C'          , //   C                 =  67, // LATIN CAPITAL LETTER C
   'D'          , //   D                 =  68, // LATIN CAPITAL LETTER D
   'E'          , //   E                 =  69, // LATIN CAPITAL LETTER E
   'F'          , //   F                 =  70, // LATIN CAPITAL LETTER F
   'G'          , //   G                 =  71, // LATIN CAPITAL LETTER G
   'H'          , //   H                 =  72, // LATIN CAPITAL LETTER H
   'I'          , //   I                 =  73, // LATIN CAPITAL LETTER I
   'J'          , //   J                 =  74, // LATIN CAPITAL LETTER J
   'K'          , //   K                 =  75, // LATIN CAPITAL LETTER K
   'L'          , //   L                 =  76, // LATIN CAPITAL LETTER L
   'M'          , //   M                 =  77, // LATIN CAPITAL LETTER M
   'N'          , //   N                 =  78, // LATIN CAPITAL LETTER N
   'O'          , //   O                 =  79, // LATIN CAPITAL LETTER O
   'P'          , //   P                 =  80, // LATIN CAPITAL LETTER P
   'Q'          , //   Q                 =  81, // LATIN CAPITAL LETTER Q
   'R'          , //   R                 =  82, // LATIN CAPITAL LETTER R
   'S'          , //   S                 =  83, // LATIN CAPITAL LETTER S
   'T'          , //   T                 =  84, // LATIN CAPITAL LETTER T
   'U'          , //   U                 =  85, // LATIN CAPITAL LETTER U
   'V'          , //   V                 =  86, // LATIN CAPITAL LETTER V
   'W'          , //   W                 =  87, // LATIN CAPITAL LETTER W
   'X'          , //   X                 =  88, // LATIN CAPITAL LETTER X
   'Y'          , //   Y                 =  89, // LATIN CAPITAL LETTER Y
   'Z'          , //   Z                 =  90, // LATIN CAPITAL LETTER Z
   VK_OEM_4     , //   LEFT_BRACKET      =  91, // LEFT SQUARE BRACKET
   VK_OEM_5     , //   BACKSLASH         =  92, // REVERSE SOLIDUS (BACKSLASH)
   VK_OEM_6     , //   RIGHT_BRACKET     =  93, // RIGHT SQUARE BRACKET
   0            , //   CARET             =  94, // CIRCUMFLEX ACCENT (CARET)
   0            , //   UNDERSCORE        =  95, // LOW LINE (UNDERSCORE)
   VK_OEM_3     , //   BACKQUOTE         =  96, // GRAVE ACCENT (BACKQUOTE)
   'a'          , //   a                 =  97, // LATIN SMALL LETTER A
   'b'          , //   b                 =  98, // LATIN SMALL LETTER B
   'c'          , //   c                 =  99, // LATIN SMALL LETTER C
   'd'          , //   d                 = 100, // LATIN SMALL LETTER D
   'e'          , //   e                 = 101, // LATIN SMALL LETTER E
   'f'          , //   f                 = 102, // LATIN SMALL LETTER F
   'g'          , //   g                 = 103, // LATIN SMALL LETTER G
   'h'          , //   h                 = 104, // LATIN SMALL LETTER H
   'i'          , //   i                 = 105, // LATIN SMALL LETTER I
   'j'          , //   j                 = 106, // LATIN SMALL LETTER J
   'k'          , //   k                 = 107, // LATIN SMALL LETTER K
   'l'          , //   l                 = 108, // LATIN SMALL LETTER L
   'm'          , //   m                 = 109, // LATIN SMALL LETTER M
   'n'          , //   n                 = 110, // LATIN SMALL LETTER N
   'o'          , //   o                 = 111, // LATIN SMALL LETTER O
   'p'          , //   p                 = 112, // LATIN SMALL LETTER P
   'q'          , //   q                 = 113, // LATIN SMALL LETTER Q
   'r'          , //   r                 = 114, // LATIN SMALL LETTER R
   's'          , //   s                 = 115, // LATIN SMALL LETTER S
   't'          , //   t                 = 116, // LATIN SMALL LETTER T
   'u'          , //   u                 = 117, // LATIN SMALL LETTER U
   'v'          , //   v                 = 118, // LATIN SMALL LETTER V
   'w'          , //   w                 = 119, // LATIN SMALL LETTER W
   'x'          , //   x                 = 120, // LATIN SMALL LETTER X
   'y'          , //   y                 = 121, // LATIN SMALL LETTER Y
   'z'          , //   z                 = 122, // LATIN SMALL LETTER Z
   VK_OEM_4     , //   LEFT_CURLY_BRACE  = 123, // LEFT CURLY BRACKET
   VK_OEM_102   , //   PIPE              = 124, // VERTICAL LINE (PIPE)
   VK_OEM_6     , //   RIGHT_CURLY_BRACE = 125, // RIGHT CURLY BRACKET
   VK_OEM_3     , //   TILDE             = 126, // TILDE
   VK_DELETE    , //   DELETE            = 127, // DELETE
   VK_LEFT      , //   LEFT_ARROW = 128,
   VK_UP        , //   UP_ARROW    ,
   VK_RIGHT     , //   RIGHT_ARROW ,
   VK_DOWN      , //   DOWN_ARROW  ,
   VK_HOME      , //   HOME        ,
   VK_END       , //   END         ,
   VK_PRIOR     , //   PAGE_UP     ,
   VK_NEXT      , //   PAGE_DOWN   ,
   //   // Begin "DEAD" keys.
   VK_LSHIFT    , //   SHIFT       ,
   VK_MENU      , //   ALT         ,
   VK_LCONTROL  , //   CTRL        , // Under OSX, this is the CMD key.
   VK_LCONTROL  , //   META        , // Under OSX, this is the CTRL key.
   VK_LMENU     , //   MENU        , // WINDOWS KEY
   //   // End "DEAD" keys.
   VK_CAPITAL   , //   CAPS_LOCK   ,
   VK_SCROLL    , //   SCROLL_LOCK ,
   VK_PAUSE     , //   PAUSE       ,
   VK_PRINT     , //   PRINT       ,
   VK_INSERT    , //   INSERT      ,
   VK_F1        , //   F1          ,
   VK_F2        , //   F2          ,
   VK_F3        , //   F3          ,
   VK_F4        , //   F4          ,
   VK_F5        , //   F5          ,
   VK_F6        , //   F6          ,
   VK_F7        , //   F7          ,
   VK_F8        , //   F8          ,
   VK_F9        , //   F9          ,
   VK_F10       , //   F10         ,
   VK_F11       , //   F11         ,
   VK_F12       , //   F12         ,
   VK_F13       , //   F13         ,
   VK_F14       , //   F14         ,
   VK_F15       , //   F15         ,
   VK_F16       , //   F16         ,
   VK_F17       , //   F17         ,
   VK_F18       , //   F18         ,
   VK_F19       , //   F19         ,
   VK_F20       , //   F20         ,
   VK_F21       , //   F21         ,
   VK_F22       , //   F22         ,
   VK_F23       , //   F23         ,
   VK_F24       , //   F24         ,
   //   // Numeric keypad.
   VK_NUMLOCK   , //   NUM_LOCK    ,
   VK_NUMPAD0   , //   NUM_0       ,
   VK_NUMPAD1   , //   NUM_1       ,
   VK_NUMPAD2   , //   NUM_2       ,
   VK_NUMPAD3   , //   NUM_3       ,
   VK_NUMPAD4   , //   NUM_4       ,
   VK_NUMPAD5   , //   NUM_5       ,
   VK_NUMPAD6   , //   NUM_6       ,
   VK_NUMPAD7   , //   NUM_7       ,
   VK_NUMPAD8   , //   NUM_8       ,
   VK_NUMPAD9   , //   NUM_9       ,
   VK_OEM_PERIOD, //   NUM_DOT     ,
   VK_ADD       , //   NUM_ADD     ,
   VK_SUBTRACT  , //   NUM_SUB     ,
   VK_MULTIPLY  , //   NUM_MUL     ,
   VK_DIVIDE    , //   NUM_DIV     ,
   VK_RETURN    , //   NUM_ENTER   ,
};

//------------------------------------------------------------------------------
//! Translates some special key events to a cross-platform code.
//! Ref:
//!   http://msdn.microsoft.com/en-us/library/ms646268.aspx
//!   http://msdn.microsoft.com/en-us/library/ms645540.aspx
inline int  wparamToKey( WPARAM wparam )
{
   uint8_t idx = (uint8_t)wparam;
   int key = _wparamsToKeyTable[idx];
   //StdErr << "Key: wparam=" << wparam << "(" << toHex(uint16_t(wparam)) << ") key=" << key << "(" << toHex(uint16_t(key)) << ")" << nl;
   if( key != 0 )  return key;
   return (int)wparam;
}

UNNAMESPACE_END


/*==============================================================================
  CLASS CoreWin
==============================================================================*/

Vec2i   CoreWin::_size( 0, 0 );
uint    CoreWin::_mainPointerID = 0;
HWND    CoreWin::_winHandle = 0;

//------------------------------------------------------------------------------
//!
void
Core::initialize()
{
   _single = new CoreWin();
   _doubleClickDelay = double(GetDoubleClickTime()) / 1000.0;
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
   os << "Core: Win32" << nl;
}

//------------------------------------------------------------------------------
//!
LRESULT APIENTRY
CoreWin::winProc( HWND win, UINT message, WPARAM wparam, LPARAM lparam )
{
   //static uint clickCount = 0;

   int nx = (short)(LOWORD(lparam));
   int ny = (short)(HIWORD(lparam));

   Vec2i pos( nx, _size.y - ny - 1 );

   switch( message )
   {
      case WM_CREATE:
         break;

      case WM_SIZE:
         if( _init )
         {
            _single->resize( nx, ny );
         }
         break;

      //case WM_ERASEBKGND:
      //   return 1;	// Can eliminate screen flash.

      //case WM_PAINT:
      //   //screen->_redraw = false;
      //   //lScreen->Render();
      //   break;

      case WM_DESTROY:
         PostQuitMessage(0);
         break;

      case WM_CHAR:
         //if( wparam != UNICODE_NOCHAR )
         {
            _count_ch = ((lparam & 0x40000000) != 0) ? _count_ch+1 : 1;
            _single->submitEvent(
               Event::Char( Core::lastTime(), int(wparam), _count_ch )
            );
         }
         break;

      case WM_KEYDOWN:
         if( (lparam & 0x40000000) == 0 )
         {
            _single->submitEvent(
               Event::KeyPress( Core::lastTime(), wparamToKey(wparam) )
            );
         }
         break;

      case WM_KEYUP:
         _single->submitEvent(
            Event::KeyRelease( Core::lastTime(), wparamToKey(wparam) )
         );
         break;
      case WM_SYSKEYDOWN:
      {
         if( (lparam & 0x40000000) == 0 )
         {
            _single->submitEvent(
               Event::KeyPress( Core::lastTime(), wparamToKey(wparam) )
            );
         }
      } break;
      case WM_SYSKEYUP:
      {
         _single->submitEvent(
            Event::KeyRelease( Core::lastTime(), wparamToKey(wparam) )
         );
      } break;

      case WM_LBUTTONDOWN:
      {
         SetCapture(win);
         const Pointer& ptr = Core::pointer(_mainPointerID);
         double delay = Core::lastTime() - ptr.lastReleaseEvent().timestamp();
         int count = (delay <= _doubleClickDelay && ptr.withinPress(_doubleClickRange)) ? ptr.lastPressEvent().count() + 1u : 1u;
         _single->submitEvent( Event::PointerPress( Core::lastTime(), _mainPointerID, 1, pos, count ) );
      }  break;

      case WM_MBUTTONDOWN:
      {
         SetCapture(win);
         const Pointer& ptr = Core::pointer(_mainPointerID);
         double delay = Core::lastTime() - ptr.lastReleaseEvent().timestamp();
         int count = (delay <= _doubleClickDelay && ptr.withinPress(_doubleClickRange)) ? ptr.lastPressEvent().count() + 1u : 1u;
         _single->submitEvent( Event::PointerPress( Core::lastTime(), _mainPointerID, 2, pos, count ) );
      }  break;

      case WM_RBUTTONDOWN:
      {
         SetCapture(win);
         const Pointer& ptr = Core::pointer(_mainPointerID);
         double delay = Core::lastTime() - ptr.lastReleaseEvent().timestamp();
         int count = (delay <= _doubleClickDelay && ptr.withinPress(_doubleClickRange)) ? ptr.lastPressEvent().count() + 1u : 1u;
         _single->submitEvent( Event::PointerPress( Core::lastTime(), _mainPointerID, 3, pos, count ) );
      }  break;

      case WM_LBUTTONDBLCLK:
      {
         const Pointer& ptr = Core::pointer(_mainPointerID);
         double delay = Core::lastTime() - ptr.lastPressEvent().timestamp();
         int count = (delay <= _doubleClickDelay && ptr.withinPress(_doubleClickRange)) ? ptr.lastPressEvent().count() + 1u : 1u;
         _single->submitEvent( Event::PointerPress( Core::lastTime(), _mainPointerID, 1, pos, count ) );
      }  break;

      case WM_MBUTTONDBLCLK:
      {
         const Pointer& ptr = Core::pointer(_mainPointerID);
         double delay = Core::lastTime() - ptr.lastPressEvent().timestamp();
         int count = (delay <= _doubleClickDelay && ptr.withinPress(_doubleClickRange)) ? ptr.lastPressEvent().count() + 1u : 1u;
         _single->submitEvent( Event::PointerPress( Core::lastTime(), _mainPointerID, 2, pos, count ) );
      }  break;

      case WM_RBUTTONDBLCLK:
      {
         const Pointer& ptr = Core::pointer(_mainPointerID);
         double delay = Core::lastTime() - ptr.lastPressEvent().timestamp();
         int count = (delay <= _doubleClickDelay && ptr.withinPress(_doubleClickRange)) ? ptr.lastPressEvent().count() + 1u : 1u;
         _single->submitEvent( Event::PointerPress( Core::lastTime(), _mainPointerID, 3, pos, count ) );
      }  break;

      case WM_LBUTTONUP:
      {
         _single->submitEvent(
            Event::PointerRelease( Core::lastTime(), _mainPointerID, 1, pos, Core::pointer(_mainPointerID).lastPressEvent().count() )
         );
         ReleaseCapture();
      }  break;

      case WM_MBUTTONUP:
      {
         _single->submitEvent(
            Event::PointerRelease( Core::lastTime(), _mainPointerID, 2, pos, Core::pointer(_mainPointerID).lastPressEvent().count() )
         );
         ReleaseCapture();
      }   break;

      case WM_RBUTTONUP:
      {
         _single->submitEvent(
            Event::PointerRelease( Core::lastTime(), _mainPointerID, 3, pos, Core::pointer(_mainPointerID).lastPressEvent().count() )
         );
         ReleaseCapture();
      }  break;

      case WM_MOUSEMOVE:
      {
         _single->submitEvent( Event::PointerMove( Core::lastTime(), _mainPointerID, pos ) );
      }  break;

      case WM_MOUSEWHEEL:
      {
         // The original position of this event is in screen coordinate, so we
         // replace it with the current pointer position.
         pos = _single->pointer( _mainPointerID ).position();

         int zDelta = GET_WHEEL_DELTA_WPARAM(wparam);
         //WHEEL_DELTA is currently 120, for fine-tuning
         //  http://windowssdk.msdn.microsoft.com/en-us/library/ms645617(VS.80).aspx
         float val = float(zDelta)/WHEEL_DELTA;
         _single->submitEvent( Event::PointerScroll( Core::lastTime(), _mainPointerID, val, pos ) );
      }  break;

      case WM_MOUSEHWHEEL:
      {
         // The original position of this event is in screen coordinate, so we
         // replace it with the current pointer position.
         pos = _single->pointer( _mainPointerID ).position();

         int zDelta = GET_WHEEL_DELTA_WPARAM(wparam);
         //WHEEL_DELTA is currently 120, for fine-tuning
         //  http://windowssdk.msdn.microsoft.com/en-us/library/ms645617(VS.80).aspx
         Vec2f val( 0.0f );
         val.x = float(zDelta)/WHEEL_DELTA;
         _single->submitEvent( Event::PointerScroll( Core::lastTime(), _mainPointerID, val, pos ) );
      }  break;

      case WM_HSCROLL:
      {
         // The original position of this event is in screen coordinate, so we
         // replace it with the current pointer position.
         pos = _single->pointer( _mainPointerID ).position();

         int zDelta = int(wparam)*2 - 1;
         Vec2f val( 0.0f );
         val.x = float(zDelta);
         _single->submitEvent( Event::PointerScroll( Core::lastTime(), _mainPointerID, val, pos ) );
      }  break;

      //case WM_VSCROLL:
      //{
      //}  break;

      default:
         //StdErr << "msg: " << toHex(message) << " w=" << toHex((uint)wparam) << " l=" << toHex((uint)lparam) << nl;
         break;
   }

   return DefWindowProc( win, message, wparam, lparam );
}

//------------------------------------------------------------------------------
//!
CoreWin::CoreWin()
{
   DBG_BLOCK( os_win, "CoreWin::CoreWin" );

   _pointer[Pointer::DEFAULT]  = LoadCursor( 0, IDC_ARROW );
   _pointer[Pointer::SIZE_ALL] = LoadCursor( 0, IDC_SIZEALL );
   _pointer[Pointer::SIZE_T]   = LoadCursor( 0, IDC_SIZENS );
   _pointer[Pointer::SIZE_B]   = LoadCursor( 0, IDC_SIZENS );
   _pointer[Pointer::SIZE_BL]  = LoadCursor( 0, IDC_SIZENESW );
   _pointer[Pointer::SIZE_BR]  = LoadCursor( 0, IDC_SIZENWSE );
   _pointer[Pointer::SIZE_TL]  = LoadCursor( 0, IDC_SIZENWSE );
   _pointer[Pointer::SIZE_TR]  = LoadCursor( 0, IDC_SIZENESW );
   _pointer[Pointer::SIZE_L]   = LoadCursor( 0, IDC_SIZEWE );
   _pointer[Pointer::SIZE_R ]  = LoadCursor( 0, IDC_SIZEWE );
   _pointer[Pointer::TEXT]     = LoadCursor( 0, IDC_IBEAM );
   _pointer[Pointer::HAND]     = LoadCursor( 0, IDC_HAND );
   _pointer[Pointer::WAIT]     = LoadCursor( 0, IDC_WAIT );
   _pointer[Pointer::INVALID]  = _pointer[Pointer::DEFAULT];

   _mainPointerID = Core::createPointer().id();

   setPaths();

   // Create window.
   initWin();
}

//------------------------------------------------------------------------------
//!
CoreWin::~CoreWin()
{
   DBG_BLOCK( os_win, "CoreWin::~CoreWin" );
}

//------------------------------------------------------------------------------
//!
void
CoreWin::initializeGUI()
{
   DBG_BLOCK( os_win, "CoreWin::initializeGUI" );

   // Gfx manager.
   RCP<Gfx::Context> cntx = Gfx::Context::getDefaultContext( Core::gfxAPI().cstr(), _winHandle );
   //RCP<Gfx::Context> cntx = Gfx::Context::getDefaultContext( "OpenGL ES", _winHandle );
   Core::gfx( Gfx::Manager::create( cntx.ptr() ) );
   _init = true;

   Core::initializeGUI();
}

//------------------------------------------------------------------------------
//!
void
CoreWin::finalizeGUI()
{
   DBG_BLOCK( os_win, "CoreWin::finalizeGUI" );
   Core::finalizeGUI();
}

//------------------------------------------------------------------------------
//!
void
CoreWin::performExec()
{
   DBG_BLOCK( os_win, "CoreWin::performExec" );

   initializeGUI();
   readyToExec();
   performShow();

   MSG msg;
   bool done = false;

   // Start the message loop.
   _timer.restart();

#if PROFILE_EVENTS
   EventProfiler& profiler = Core::profiler();
   profiler.add( EventProfiler::LOOPS_BEGIN );
#endif

   while( !done  )
   {
#if PROFILE_EVENTS
      profiler.add( EventProfiler::LOOP_BEGIN );
#endif

      while( PeekMessage( &msg, 0, 0, 0, PM_REMOVE ) )
      {
         done |= (msg.message == WM_QUIT);
         TranslateMessage( &msg );
         DispatchMessage( &msg );
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
CoreWin::performExit()
{
   DBG_BLOCK( os_win, "CoreWin::performExit()" );
   PostQuitMessage(0);
}

//------------------------------------------------------------------------------
//!
void
CoreWin::performShow()
{
   DBG_BLOCK( os_win, "CoreWin::performShow" );

   // Show window
   ShowWindow( _winHandle, SW_SHOWDEFAULT );
   SetForegroundWindow( _winHandle );

   // Resize??
   resize( _size.x, _size.y );
   UpdateWindow( _winHandle );
}

//------------------------------------------------------------------------------
//!
void
CoreWin::performHide()
{
   DBG_BLOCK( os_win, "CoreWin::performHide" );
   if( fullScreen() )
   {
      ChangeDisplaySettings( 0, 0 );
   }
}

//------------------------------------------------------------------------------
//!
void
CoreWin::performRedraw()
{
   DBG_BLOCK( os_win, "CoreWin::performRedraw" );
   /*
    if( !_redraw )
    PostMessage( _winHandle, WM_PAINT, 0, 0 );

    _redraw = true;
   */
}

//------------------------------------------------------------------------------
//!
bool
CoreWin::performIsKeyPressed( int key )
{
   if( key < Key::_ )
   {
      int winKey = _keyToVirtKeyTable[key];
      if( winKey != 0 )
      {
         SHORT res = GetKeyState( winKey );
         return (res&0x8000) != 0x0;
      }
   }
   return Core::performIsKeyPressed( key );  // Send to default implementation.
}

//------------------------------------------------------------------------------
//!
void
CoreWin::performGrabPointer( uint pointerID )
{
   if( pointerID != _mainPointerID )
   {
      StdErr << "WARNING - Don't know what to do with non-main pointer grabbing." << nl;
      return;
   }

   SetCapture( _single->_winHandle );
}

//------------------------------------------------------------------------------
//!
void
CoreWin::performReleasePointer( uint pointerID )
{
   if( pointerID != _mainPointerID )
   {
      StdErr << "WARNING - Don't know what to do with non-main pointer release." << nl;
      return;
   }

   ReleaseCapture();
}

//------------------------------------------------------------------------------
//!
void
CoreWin::performSetPointerIcon( uint pointerID, uint icon )
{
   DBG_BLOCK( os_win, "CoreWin::performSetPointerIcon(" << icon << ")" );

   if( pointerID != _mainPointerID )
   {
      StdErr << "WARNING - Don't know what to do with non-main pointer icon setting." << nl;
      return;
   }

   SetCursor( _pointer[icon] );
}

//-----------------------------------------------------------------------------
//!
void
CoreWin::performAsk( FileDialog& dlg )
{
   char file[4096];
   file[0] = '\0';
   char filter[4096];

   OPENFILENAMEA ofn;
   ZeroMemory( &ofn , sizeof(ofn) );
   ofn.lStructSize = sizeof(ofn);
   ofn.hwndOwner   = _single->_winHandle; //NULL;
   ofn.lpstrFile   = file;
   ofn.nMaxFile    = sizeof(file);
   ofn.Flags       = OFN_EXPLORER;

   if( !dlg.title().empty() )  ofn.lpstrTitle = dlg.title().cstr();

   if( !dlg.startingLocation().empty() )
   {
      FS::Entry entry( dlg.startingLocation() );
      if( entry.type() == FS::TYPE_DIRECTORY )
      {
         ofn.lpstrInitialDir = entry.path().cstr();
      }
      else
      {
         //Path d = entry.path();
         //Path f = d.split();
         strncpy( ofn.lpstrFile, entry.path().cstr(), 4096 );
      }
   }

   const Vector<String>& types = dlg.allowedTypes();
   if( !types.empty() )
   {
      char* dst = filter;
      // Write as "EXT1/EXT2/EXT3\0*.EXT1;*.EXT2;*.EXT3\Any\0*.*\0\0".

      // 1. First pair: all of the specified extensions.
      // 1.1. Description: "EXT1/EXT2/EXT3"
      for( auto cur = types.begin(); cur != types.end(); ++cur )
      {
         const String& ext = (*cur);
         strcpy( dst, ext.cstr() );
         dst += ext.size();
         *dst++ = '/';
         CHECK( size_t(dst - filter) < sizeof(filter) );
      }
      dst[-1] = '\0'; // Replace last '/' with a '\0'.
      // 1.2. Extension list: "*.EXT1;*.EXT2;*.EXT3"
      for( auto cur = types.begin(); cur != types.end(); ++cur )
      {
         const String& ext = (*cur);
         *dst++ = '*';
         *dst++ = '.';
         strcpy( dst, ext.cstr() );
         dst += ext.size();
         *dst++ = ';';
         CHECK( size_t(dst - filter) < sizeof(filter) );
      }
      dst[-1] = '\0'; // Replace last ';' with a '\0'.

      // 2. Second pair: The "Any" wildcard.
      strcpy( dst, "Any" ); dst += 3;
      *dst++ = '\0';
      strcpy( dst, "*.*" ); dst += 3;
      *dst++ = '\0';
      *dst = '\0';    // Extra '\0' character at the end.

      ofn.lpstrFilter  = filter;
      ofn.nFilterIndex = 1;
   }

#if !defined(__CYGWIN__)
   if( dlg.showHidden() )  ofn.Flags |= OFN_FORCESHOWHIDDEN;
#endif

   if( dlg.multipleSelect() )  ofn.Flags |= OFN_ALLOWMULTISELECT;

   BOOL ok = false;
   switch( dlg.type() )
   {
      case FileDialog::OPEN:
      {
         ok = GetOpenFileNameA( &ofn );
      }  break;
      case FileDialog::SAVE:
      {
         ofn.Flags |= OFN_OVERWRITEPROMPT;
         ok = GetSaveFileNameA( &ofn );
      }  break;
   }

   if( ok )
   {
      size_t s = strlen(file);
      Path::bs2fs( file, s ); // Backward slashes to forward slashes (use sizeof(file)?).
      // If a single file is selected, 'file' is complete.
      // Otherwise, it contains dir + file1 + file2 + file3, with NULL in-between (extra NULL to mark end).
      if( dlg.multipleSelect() && ofn.nFileOffset == (s+1) )
      {
         Path dir = file;
         dir.toDir();
         const char* cur = file + ofn.nFileOffset;
         const char* end = file + sizeof(file);
         while( cur < end )
         {
            s = strlen( cur );
            if( s == 0 || end < (cur+s) )  break;
            Path path = dir / String(cur, s);
            dlg.add( path );
            cur += s + 1;
         }
         /**
         StdErr << dlg.numPaths() << " paths:" << nl;
         for( uint i = 0; i < dlg.numPaths(); ++i )
         {
            StdErr << i << ": _" << dlg.path(i).cstr() << "_" << nl;
         }
         **/
      }
      else
      {
         dlg.add( file );
      }
      dlg.onConfirm();
   }
   else
   {
      dlg.onCancel();
      DWORD err = CommDlgExtendedError();
      switch( err )
      {
         case 0:
            // Everything is fine.
            break;
         case CDERR_STRUCTSIZE:
            StdErr << "CoreWin::performAsk() - Invalid struct size." << nl;
            break;
         case CDERR_INITIALIZATION:
            StdErr << "CoreWin::performAsk() - Initialization error (out of memory?)." << nl;
            break;
         case CDERR_NOTEMPLATE:
            StdErr << "CoreWin::performAsk() - Missing template." << nl;
            break;
         case CDERR_NOHINSTANCE:
            StdErr << "CoreWin::performAsk() - Missing hinstance." << nl;
            break;
         case CDERR_LOADSTRFAILURE:
            StdErr << "CoreWin::performAsk() - Load string failure." << nl;
            break;
         case CDERR_FINDRESFAILURE:
            StdErr << "CoreWin::performAsk() - Could not find resource." << nl;
            break;
         case CDERR_LOADRESFAILURE:
            StdErr << "CoreWin::performAsk() - Could not load resource." << nl;
            break;
         case CDERR_LOCKRESFAILURE:
            StdErr << "CoreWin::performAsk() - Could not lock resource." << nl;
            break;
         case CDERR_MEMALLOCFAILURE:
            StdErr << "CoreWin::performAsk() - Could not allocate memory." << nl;
            break;
         case CDERR_MEMLOCKFAILURE:
            StdErr << "CoreWin::performAsk() - Could not lock memory." << nl;
            break;
         case CDERR_NOHOOK:
            StdErr << "CoreWin::performAsk() - Missing hook." << nl;
            break;
         case CDERR_REGISTERMSGFAIL:
            StdErr << "CoreWin::performAsk() - Could not register message." << nl;
            break;

         case FNERR_SUBCLASSFAILURE:
            StdErr << "CoreWin::performAsk() - Could not allocate memory for subclass." << nl;
            break;
         case FNERR_INVALIDFILENAME:
            StdErr << "CoreWin::performAsk() - Invalid filename." << nl;
            break;
         case FNERR_BUFFERTOOSMALL:
            StdErr << "CoreWin::performAsk() - Buffer too small." << nl;
            break;

         case CDERR_DIALOGFAILURE:
            StdErr << "CoreWin::performAsk() - Dialog failure (bad window handle?)." << nl;
            break;

         default:
            StdErr << "CoreWin::performAsk() - Unknown error (" << String().format("0x%04x", err) << ")." << nl;
            break;

      }
   }
}

//------------------------------------------------------------------------------
//!
void
CoreWin::setPaths()
{
   // Search for a Data subdirectory starting from current working directory.
   Path dir = Path::getCurrentDirectory();
   do
   {
      FS::Entry entry( dir / "Data" );
      if( entry.type() == FS::TYPE_DIRECTORY )
      {
         addRoot( entry.path().string() );
         break;
      }
   } while( dir.goUp() );
}

//------------------------------------------------------------------------------
//!
void
CoreWin::initWin()
{
   DBG_BLOCK( os_win, "CoreWin::initWin" );

   // create class
   static TCHAR name[] = TEXT("Fusion");
   WNDCLASS    winClass;

   winClass.style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC | CS_DBLCLKS;
   winClass.lpfnWndProc   = winProc;
   winClass.cbClsExtra    = 0;
   winClass.cbWndExtra    = 0;
   winClass.hInstance     = 0;
   winClass.hIcon         = 0;
   winClass.hCursor       = LoadCursor(NULL, IDC_ARROW);
   winClass.hbrBackground = 0;
   winClass.lpszMenuName  = 0;
   winClass.lpszClassName = name;
   RegisterClass( &winClass );

   if( fullScreen() )
   {
      // Change Resolution
      changeResolution( size(), 32 );

      // Get current mode info
      DEVMODE mode;
      EnumDisplaySettings( 0, ENUM_CURRENT_SETTINGS, &mode );

      _size = Vec2i( mode.dmPelsWidth, mode.dmPelsHeight );

      // create window
      _winHandle = CreateWindowEx(
         WS_EX_APPWINDOW,           // Extended Style
         name,                      // Class name
         name,                      // Window name
         winClass.style,            // Style  (was  "WS_POPUP /*| WS_BORDER*/,"")
         0,                         // x position (left)
         0,                         // y position (upper)
         _size.x,                   // Width
         _size.y,                   // Height
         0,                         // Window parent handle
         0,                         // Menu handle
         0,                         // Instance handle
         0                          // Pointer to param
      );
   }
   else
   {
      _size = size();

      // Calculate require size to get the specified interior size of contents
      RECT rect;
      rect.left       = 0;
      rect.top        = 0;
      rect.right      = rect.left + _size.x;
      rect.bottom     = rect.top  + _size.y;
      DWORD dwStyle   = WS_OVERLAPPEDWINDOW;
      DWORD dwStyleEx = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
      if( !AdjustWindowRectEx( &rect, dwStyle, FALSE, dwStyleEx ) )
      {
         printf("ERROR - Failed to adjust WindowRect\n");
      }

      dwStyle |=  WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
      // Create the window
      _winHandle = CreateWindowEx(
         dwStyleEx,
         name,
         name,
         dwStyle,
         0, //rect.left,
         0, //rect.top,
         rect.right - rect.left,
         rect.bottom - rect.top,
         0,
         0,
         0,
         0
      );
   }
}

//------------------------------------------------------------------------------
//!
void
CoreWin::resize( int w, int h )
{
   DBG_BLOCK( os_win, "CoreWin::resize" );

   _size.x = w;
   _size.y = h;

   performResize( w, h );

   InvalidateRect( _winHandle, nullptr, FALSE );
}

//------------------------------------------------------------------------------
//!
void
CoreWin::render()
{
   performRender();
}
