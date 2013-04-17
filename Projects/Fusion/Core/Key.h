/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef FUSION_KEY_H
#define FUSION_KEY_H

#include <Fusion/StdDefs.h>

#include <Base/Util/Platform.h>

NAMESPACE_BEGIN

namespace Key {

void initialize();

enum Key
{
   //------------------------------------------------------------------------------
   //! The 128 ASCII codes, incidently, a subset of Unicode.
   //! Ref: http://www.unicode.org/charts/PDF/U0000.pdf
   NUL               =   0, // NULL
   SOH               =   1, // START OF HEADING
   STX               =   2, // START OF TEXT
   ETX               =   3, // END OF TEXT
   EOT               =   4, // END OF TRANSMISSION
   ENQ               =   5, // ENQUIRY
   ACK               =   6, // ACKNOWLEDGE
   BEL               =   7, // BELL
   BS                =   8, // BACKSPACE
   BACKSPACE         =   8, //***
   HT                =   9, // CHARACTER TABULATION (horizontal tab)
   TAB               =   9, //***
   LF                =  10, // LINE FEED
   LINEFEED          =  10, //***
   NL                =  10, //*** NEW LINE
   NEWLINE           =  10, //***
   EOL               =  10, //*** END OF LINE
   END_OF_LINE       =  10, //***
   VT                =  11, // LINE TABULATION (vertical tab)
   VTAB              =  11, //*** VERTICAL TABULATION
   FF                =  12, // FORM FEED
   FORMFEED          =  12, //***
   NEWPAGE           =  12, //***
   CR                =  13, // CARRIAGE RETURN
   CARRIAGE_RETURN   =  13, //***
   RETURN            =  13, //***
   ENTER             =  13, //***
   SO                =  14, // SHIFT OUT (LOCKING-SHIFT ONE)
   SI                =  15, // SHIFT IN (LOCKING-SHIFT ZERO)
   DLE               =  16, // DATA LINK ESCAPE
   DC1               =  17, // DEVICE CONTROL ONE
   DC2               =  18, // DEVICE CONTROL TWO
   DC3               =  19, // DEVICE CONTROL THREE
   DC4               =  20, // DEVICE CONTROL FOUR
   NAK               =  21, // NEGATIVE ACKNOWLEDGE
   SYN               =  22, // SYNCHRONOUS IDLE
   ETB               =  23, // END OF TRANSMISSION BLOCK
   CAN               =  24, // CANCEL
   CANCEL            =  24, // CANCEL
   EM                =  25, // END OF MEDIUM
   SUB               =  26, // SUBSTITUTE
   ESC               =  27, // ESCAPE
   ESCAPE            =  27, //***
   FS                =  28, // FILE SEPARATOR
   GS                =  29, // GROUP SEPARATOR
   RS                =  30, // RECORD SEPARATOR
   US                =  31, // UNIT SEPARATOR
   SPACE             =  32, // SPACE
   EXCLAMATION_MARK  =  33, // EXCLAMATION MARK
   QUOTE             =  34, // QUOTATION MARK
   HASH              =  35, // NUMBER SIGN
   DOLLAR            =  36, // DOLLAR SIGN
   PERCENT           =  37, // PERCENT SIGN
   PERCENTAGE        =  37, //*** PERCENT SIGN
   AMPERSAND         =  38, // AMPERSAND
   AND               =  38, //***
   APOSTROPHE        =  39, // APOSTROPHE
   LEFT_PARENTHESIS  =  40, // LEFT PARENTHESIS
   RIGHT_PARENTHESIS =  41, // RIGHT PARENTHESIS
   ASTERISK          =  42, // ASTERISK
   STAR              =  42, //*** ASTERISK
   PLUS              =  43, // PLUS SIGN
   COMMA             =  44, // COMMA
   MINUS             =  45, // HYPHEN-MINUS
   HYPHEN            =  45, //*** HYPHEN-MINUS
   FULL_STOP         =  46, // FULL STOP
   PERIOD            =  46, //***
   POINT             =  46, //***
   SLASH             =  47, // SOLIDUS (SLASH)
   DIGIT_0           =  48, // DIGIT ZERO
   _0                =  48, //***
   DIGIT_1           =  49, // DIGIT ONE
   _1                =  49, //***
   DIGIT_2           =  50, // DIGIT TWO
   _2                =  50, //***
   DIGIT_3           =  51, // DIGIT THREE
   _3                =  51, //***
   DIGIT_4           =  52, // DIGIT FOUR
   _4                =  52, //***
   DIGIT_5           =  53, // DIGIT FIVE
   _5                =  53, //***
   DIGIT_6           =  54, // DIGIT SIX
   _6                =  54, //***
   DIGIT_7           =  55, // DIGIT SEVEN
   _7                =  55, //***
   DIGIT_8           =  56, // DIGIT EIGHT
   _8                =  56, //***
   DIGIT_9           =  57, // DIGIT NINE
   _9                =  57, //***
   COLON             =  58, // COLON
   SEMICOLON         =  59, // SEMICOLON
   LT                =  60, // LESS-THAN SIGN
   LESS_THAN         =  60, //***
   EQ                =  61, // EQUALS SIGN
   EQUAL             =  61, //***
   GT                =  62, // GREATER-THAN SIGN
   GREATER_THAN      =  62, //***
   QUESTION_MARK     =  63, // QUESTION MARK
   AT                =  64, // COMMERCIAL AT
   A                 =  65, // LATIN CAPITAL LETTER A
   B                 =  66, // LATIN CAPITAL LETTER B
   C                 =  67, // LATIN CAPITAL LETTER C
   D                 =  68, // LATIN CAPITAL LETTER D
   E                 =  69, // LATIN CAPITAL LETTER E
   F                 =  70, // LATIN CAPITAL LETTER F
   G                 =  71, // LATIN CAPITAL LETTER G
   H                 =  72, // LATIN CAPITAL LETTER H
   I                 =  73, // LATIN CAPITAL LETTER I
   J                 =  74, // LATIN CAPITAL LETTER J
   K                 =  75, // LATIN CAPITAL LETTER K
   L                 =  76, // LATIN CAPITAL LETTER L
   M                 =  77, // LATIN CAPITAL LETTER M
   N                 =  78, // LATIN CAPITAL LETTER N
   O                 =  79, // LATIN CAPITAL LETTER O
   P                 =  80, // LATIN CAPITAL LETTER P
   Q                 =  81, // LATIN CAPITAL LETTER Q
   R                 =  82, // LATIN CAPITAL LETTER R
   S                 =  83, // LATIN CAPITAL LETTER S
   T                 =  84, // LATIN CAPITAL LETTER T
   U                 =  85, // LATIN CAPITAL LETTER U
   V                 =  86, // LATIN CAPITAL LETTER V
   W                 =  87, // LATIN CAPITAL LETTER W
   X                 =  88, // LATIN CAPITAL LETTER X
   Y                 =  89, // LATIN CAPITAL LETTER Y
   Z                 =  90, // LATIN CAPITAL LETTER Z
   LEFT_BRACKET      =  91, // LEFT SQUARE BRACKET
   BACKSLASH         =  92, // REVERSE SOLIDUS (BACKSLASH)
   RIGHT_BRACKET     =  93, // RIGHT SQUARE BRACKET
   CARET             =  94, // CIRCUMFLEX ACCENT (CARET)
   UNDERSCORE        =  95, // LOW LINE (UNDERSCORE)
   BACKQUOTE         =  96, // GRAVE ACCENT (BACKQUOTE)
   a                 =  97, // LATIN SMALL LETTER A
   b                 =  98, // LATIN SMALL LETTER B
   c                 =  99, // LATIN SMALL LETTER C
   d                 = 100, // LATIN SMALL LETTER D
   e                 = 101, // LATIN SMALL LETTER E
   f                 = 102, // LATIN SMALL LETTER F
   g                 = 103, // LATIN SMALL LETTER G
   h                 = 104, // LATIN SMALL LETTER H
   i                 = 105, // LATIN SMALL LETTER I
   j                 = 106, // LATIN SMALL LETTER J
   k                 = 107, // LATIN SMALL LETTER K
   l                 = 108, // LATIN SMALL LETTER L
   m                 = 109, // LATIN SMALL LETTER M
   n                 = 110, // LATIN SMALL LETTER N
   o                 = 111, // LATIN SMALL LETTER O
   p                 = 112, // LATIN SMALL LETTER P
   q                 = 113, // LATIN SMALL LETTER Q
   r                 = 114, // LATIN SMALL LETTER R
   s                 = 115, // LATIN SMALL LETTER S
   t                 = 116, // LATIN SMALL LETTER T
   u                 = 117, // LATIN SMALL LETTER U
   v                 = 118, // LATIN SMALL LETTER V
   w                 = 119, // LATIN SMALL LETTER W
   x                 = 120, // LATIN SMALL LETTER X
   y                 = 121, // LATIN SMALL LETTER Y
   z                 = 122, // LATIN SMALL LETTER Z
   LEFT_CURLY_BRACE  = 123, // LEFT CURLY BRACKET
   PIPE              = 124, // VERTICAL LINE (PIPE)
   RIGHT_CURLY_BRACE = 125, // RIGHT CURLY BRACKET
   TILDE             = 126, // TILDE
   DELETE            = 127, // DELETE

   LEFT_ARROW = 128,
   UP_ARROW    ,
   RIGHT_ARROW ,
   DOWN_ARROW  ,
   HOME        ,
   END         ,
   PAGE_UP     ,
   PAGE_DOWN   ,
   // Begin "DEAD" keys.
   SHIFT       ,
   ALT         ,
   CTRL        , // The Control key, on all platforms.
   CMD         , // The Command key, typical on Macs.
   MENU        , // WINDOWS KEY
   // End "DEAD" keys.
   CAPS_LOCK   ,
   SCROLL_LOCK ,
   PAUSE       ,
   PRINT       ,
   INSERT      ,
   F1          ,
   F2          ,
   F3          ,
   F4          ,
   F5          ,
   F6          ,
   F7          ,
   F8          ,
   F9          ,
   F10         ,
   F11         ,
   F12         ,
   F13         ,
   F14         ,
   F15         ,
   F16         ,
   F17         ,
   F18         ,
   F19         ,
   F20         ,
   F21         ,
   F22         ,
   F23         ,
   F24         ,

   // Numeric keypad.
   NUM_LOCK    ,
   NUM_0       ,
   NUM_1       ,
   NUM_2       ,
   NUM_3       ,
   NUM_4       ,
   NUM_5       ,
   NUM_6       ,
   NUM_7       ,
   NUM_8       ,
   NUM_9       ,
   NUM_DOT     ,
   NUM_ADD     ,
   NUM_SUB     ,
   NUM_MUL     ,
   NUM_DIV     ,
   NUM_ENTER   ,

   // Special cases.
#if PLAT_APPLE
   META = CMD,
#else
   META = CTRL,
#endif

   // Sentinel, to not have to remove the trailing coma on the last one above.
   _
};

//------------------------------------------------------------------------------
//! A routine to convert a character into it's key code.
FUSION_DLL_API Key  charToKey( int c );

}

NAMESPACE_END

#endif
