/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Fusion/Core/Key.h>
#include <Fusion/VM/VMRegistry.h>

/*==============================================================================
  UNNAME NAMESPACE
==============================================================================*/

UNNAMESPACE_BEGIN

const VM::EnumReg _enumsKeys[] = {
   { "NUL"              , Key::NUL               },
   { "SOH"              , Key::SOH               },
   { "STX"              , Key::STX               },
   { "ETX"              , Key::ETX               },
   { "EOT"              , Key::EOT               },
   { "ENQ"              , Key::ENQ               },
   { "ACK"              , Key::ACK               },
   { "BEL"              , Key::BEL               },
   { "BACKSPACE"        , Key::BACKSPACE         },
   { "TAB"              , Key::TAB               },
   { "LINEFEED"         , Key::LINEFEED          },
   { "NEWLINE"          , Key::NEWLINE           },
   { "END_OF_LINE"      , Key::END_OF_LINE       },
   { "EOL"              , Key::EOL               },
   { "VTAB"             , Key::VT                },
   { "FORMFEED"         , Key::FORMFEED          },
   { "RETURN"           , Key::RETURN            },
   { "SO"               , Key::SO                },
   { "SI"               , Key::SI                },
   { "DLE"              , Key::DLE               },
   { "DC1"              , Key::DC1               },
   { "DC2"              , Key::DC2               },
   { "DC3"              , Key::DC3               },
   { "DC4"              , Key::DC4               },
   { "NAK"              , Key::NAK               },
   { "SYN"              , Key::SYN               },
   { "ETB"              , Key::ETB               },
   { "CAN"              , Key::CAN               },
   { "EM"               , Key::EM                },
   { "SUB"              , Key::SUB               },
   { "ESC"              , Key::ESCAPE            },
   { "FS"               , Key::FS                },
   { "GS"               , Key::GS                },
   { "RS"               , Key::RS                },
   { "US"               , Key::US                },
   { "SPACE"            , Key::SPACE             },
   { "EXCLAMATION_MARK" , Key::EXCLAMATION_MARK  },
   { "QUOTE"            , Key::QUOTE             },
   { "HASH"             , Key::HASH              },
   { "DOLLAR"           , Key::DOLLAR            },
   { "PERCENT"          , Key::PERCENT           },
   { "AND"              , Key::AMPERSAND         },
   { "QUOTE"            , Key::APOSTROPHE        },
   { "LEFT_PARENTHESIS" , Key::LEFT_PARENTHESIS  },
   { "RIGHT_PARENTHESIS", Key::RIGHT_PARENTHESIS },
   { "ASTERISK"         , Key::ASTERISK          },
   { "STAR"             , Key::ASTERISK          },
   { "PLUS"             , Key::PLUS              },
   { "COMMA"            , Key::COMMA             },
   { "MINUS"            , Key::MINUS             },
   { "PERIOD"           , Key::PERIOD            },
   { "SLASH"            , Key::SLASH             },
   { "DIGIT_0"          , Key::_0                },
   { "DIGIT_1"          , Key::_1                },
   { "DIGIT_2"          , Key::_2                },
   { "DIGIT_3"          , Key::_3                },
   { "DIGIT_4"          , Key::_4                },
   { "DIGIT_5"          , Key::_5                },
   { "DIGIT_6"          , Key::_6                },
   { "DIGIT_7"          , Key::_7                },
   { "DIGIT_8"          , Key::_8                },
   { "DIGIT_9"          , Key::_9                },
   { "COLON"            , Key::COLON             },
   { "SEMICOLON"        , Key::SEMICOLON         },
   { "LESS_THAN"        , Key::LT                },
   { "EQUAL"            , Key::EQ                },
   { "GREATER_THAN"     , Key::GT                },
   { "AT"               , Key::AT                },
   { "A"                , Key::A                 },
   { "B"                , Key::B                 },
   { "C"                , Key::C                 },
   { "D"                , Key::D                 },
   { "E"                , Key::E                 },
   { "F"                , Key::F                 },
   { "G"                , Key::G                 },
   { "H"                , Key::H                 },
   { "I"                , Key::I                 },
   { "J"                , Key::J                 },
   { "K"                , Key::K                 },
   { "L"                , Key::L                 },
   { "M"                , Key::M                 },
   { "N"                , Key::N                 },
   { "O"                , Key::O                 },
   { "P"                , Key::P                 },
   { "Q"                , Key::Q                 },
   { "R"                , Key::R                 },
   { "S"                , Key::S                 },
   { "T"                , Key::T                 },
   { "U"                , Key::U                 },
   { "V"                , Key::V                 },
   { "W"                , Key::W                 },
   { "X"                , Key::X                 },
   { "Y"                , Key::Y                 },
   { "Z"                , Key::Z                 },
   { "LEFT_BRACKET"     , Key::LEFT_BRACKET      },
   { "BACKSLASH"        , Key::BACKSLASH         },
   { "RIGHT_BRACKET"    , Key::RIGHT_BRACKET     },
   { "CARET"            , Key::CARET             },
   { "UNDERSCORE"       , Key::UNDERSCORE        },
   { "BACKQUOTE"        , Key::BACKQUOTE         },
   { "a"                , Key::a                 },
   { "b"                , Key::b                 },
   { "c"                , Key::c                 },
   { "d"                , Key::d                 },
   { "e"                , Key::e                 },
   { "f"                , Key::f                 },
   { "g"                , Key::g                 },
   { "h"                , Key::h                 },
   { "i"                , Key::i                 },
   { "j"                , Key::j                 },
   { "k"                , Key::k                 },
   { "l"                , Key::l                 },
   { "m"                , Key::m                 },
   { "n"                , Key::n                 },
   { "o"                , Key::o                 },
   { "p"                , Key::p                 },
   { "q"                , Key::q                 },
   { "r"                , Key::r                 },
   { "s"                , Key::s                 },
   { "t"                , Key::t                 },
   { "u"                , Key::u                 },
   { "v"                , Key::v                 },
   { "w"                , Key::w                 },
   { "x"                , Key::x                 },
   { "y"                , Key::y                 },
   { "z"                , Key::z                 },
   { "LEFT_CURLY_BRACE" , Key::LEFT_CURLY_BRACE  },
   { "PIPE"             , Key::PIPE              },
   { "RIGHT_CURLY_BRACE", Key::RIGHT_CURLY_BRACE },
   { "TILDE"            , Key::TILDE             },
   { "DELETE"           , Key::DELETE            },
   { "LEFT_ARROW"       , Key::LEFT_ARROW        },
   { "UP_ARROW"         , Key::UP_ARROW          },
   { "RIGHT_ARROW"      , Key::RIGHT_ARROW       },
   { "DOWN_ARROW"       , Key::DOWN_ARROW        },
   { "HOME"             , Key::HOME              },
   { "END"              , Key::END               },
   { "PAGE_UP"          , Key::PAGE_UP           },
   { "PAGE_DOWN"        , Key::PAGE_DOWN         },
   { "SHIFT"            , Key::SHIFT             },
   { "ALT"              , Key::ALT               },
   { "CTRL"             , Key::CTRL              },
   { "CMD"              , Key::CMD               },
   { "MENU"             , Key::MENU              },
   { "CAPS_LOCK"        , Key::CAPS_LOCK         },
   { "SCROLL_LOCK"      , Key::SCROLL_LOCK       },
   { "PAUSE"            , Key::PAUSE             },
   { "PRINT"            , Key::PRINT             },
   { "INSERT"           , Key::INSERT            },
   { "F1"               , Key::F1                },
   { "F2"               , Key::F2                },
   { "F3"               , Key::F3                },
   { "F4"               , Key::F4                },
   { "F5"               , Key::F5                },
   { "F6"               , Key::F6                },
   { "F7"               , Key::F7                },
   { "F8"               , Key::F8                },
   { "F9"               , Key::F9                },
   { "F10"              , Key::F10               },
   { "F11"              , Key::F11               },
   { "F12"              , Key::F12               },
   { "F13"              , Key::F13               },
   { "F14"              , Key::F14               },
   { "F15"              , Key::F15               },
   { "F16"              , Key::F16               },
   { "F17"              , Key::F17               },
   { "F18"              , Key::F18               },
   { "F19"              , Key::F19               },
   { "F20"              , Key::F20               },
   { "F21"              , Key::F21               },
   { "F22"              , Key::F22               },
   { "F23"              , Key::F23               },
   { "F24"              , Key::F24               },
   { "NUM_LOCK"         , Key::NUM_LOCK          },
   { "META"             , Key::META              },
   { 0, 0 }
};

const VM::EnumReg _enumsKeysReversed[] = {
   { "<NUL>"        , Key::NUL               },
   { "<SOH>"        , Key::SOH               },
   { "<STX>"        , Key::STX               },
   { "<ETX>"        , Key::ETX               },
   { "<EOT>"        , Key::EOT               },
   { "<ENQ>"        , Key::ENQ               },
   { "<ACK>"        , Key::ACK               },
   { "<BEL>"        , Key::BEL               },
   { "<BACKSPACE>"  , Key::BACKSPACE         },
   { "<TAB>"        , Key::TAB               },
   { "<LINEFEED>"   , Key::LINEFEED          },
   { "<NEWLINE>"    , Key::NEWLINE           },
   { "<END_OF_LINE>", Key::END_OF_LINE       },
   { "<VTAB>"       , Key::VT                },
   { "<FORMFEED>"   , Key::FORMFEED          },
   { "<RETURN>"     , Key::RETURN            },
   { "<SO>"         , Key::SO                },
   { "<SI>"         , Key::SI                },
   { "<DLE>"        , Key::DLE               },
   { "<DC1>"        , Key::DC1               },
   { "<DC2>"        , Key::DC2               },
   { "<DC3>"        , Key::DC3               },
   { "<DC4>"        , Key::DC4               },
   { "<NAK>"        , Key::NAK               },
   { "<SYN>"        , Key::SYN               },
   { "<ETB>"        , Key::ETB               },
   { "<CAN>"        , Key::CAN               },
   { "<EM>"         , Key::EM                },
   { "<SUB>"        , Key::SUB               },
   { "<ESCAPE>"     , Key::ESCAPE            },
   { "<FS>"         , Key::FS                },
   { "<GS>"         , Key::GS                },
   { "<RS>"         , Key::RS                },
   { "<US>"         , Key::US                },
   { "<SPACE>"      , Key::SPACE             },
   { "!"            , Key::EXCLAMATION_MARK  },
   { "\""           , Key::QUOTE             },
   { "#"            , Key::HASH              },
   { "$"            , Key::DOLLAR            },
   { "%"            , Key::PERCENT           },
   { "&"            , Key::AMPERSAND         },
   { "'"            , Key::APOSTROPHE        },
   { "("            , Key::LEFT_PARENTHESIS  },
   { ")"            , Key::RIGHT_PARENTHESIS },
   { "*"            , Key::ASTERISK          },
   { "+"            , Key::PLUS              },
   { ","            , Key::COMMA             },
   { "-"            , Key::MINUS             },
   { "."            , Key::PERIOD            },
   { "/"            , Key::SLASH             },
   { "0"            , Key::_0                },
   { "1"            , Key::_1                },
   { "2"            , Key::_2                },
   { "3"            , Key::_3                },
   { "4"            , Key::_4                },
   { "5"            , Key::_5                },
   { "6"            , Key::_6                },
   { "7"            , Key::_7                },
   { "8"            , Key::_8                },
   { "9"            , Key::_9                },
   { ":"            , Key::COLON             },
   { ";"            , Key::SEMICOLON         },
   { "<"            , Key::LT                },
   { "="            , Key::EQ                },
   { ">"            , Key::GT                },
   { "@"            , Key::AT                },
   { "A"            , Key::A                 },
   { "B"            , Key::B                 },
   { "C"            , Key::C                 },
   { "D"            , Key::D                 },
   { "E"            , Key::E                 },
   { "F"            , Key::F                 },
   { "G"            , Key::G                 },
   { "H"            , Key::H                 },
   { "I"            , Key::I                 },
   { "J"            , Key::J                 },
   { "K"            , Key::K                 },
   { "L"            , Key::L                 },
   { "M"            , Key::M                 },
   { "N"            , Key::N                 },
   { "O"            , Key::O                 },
   { "P"            , Key::P                 },
   { "Q"            , Key::Q                 },
   { "R"            , Key::R                 },
   { "S"            , Key::S                 },
   { "T"            , Key::T                 },
   { "U"            , Key::U                 },
   { "V"            , Key::V                 },
   { "W"            , Key::W                 },
   { "X"            , Key::X                 },
   { "Y"            , Key::Y                 },
   { "Z"            , Key::Z                 },
   { "["            , Key::LEFT_BRACKET      },
   { "\\"           , Key::BACKSLASH         },
   { "]"            , Key::RIGHT_BRACKET     },
   { "^"            , Key::CARET             },
   { "_"            , Key::UNDERSCORE        },
   { "`"            , Key::BACKQUOTE         },
   { "a"            , Key::a                 },
   { "b"            , Key::b                 },
   { "c"            , Key::c                 },
   { "d"            , Key::d                 },
   { "e"            , Key::e                 },
   { "f"            , Key::f                 },
   { "g"            , Key::g                 },
   { "h"            , Key::h                 },
   { "i"            , Key::i                 },
   { "j"            , Key::j                 },
   { "k"            , Key::k                 },
   { "l"            , Key::l                 },
   { "m"            , Key::m                 },
   { "n"            , Key::n                 },
   { "o"            , Key::o                 },
   { "p"            , Key::p                 },
   { "q"            , Key::q                 },
   { "r"            , Key::r                 },
   { "s"            , Key::s                 },
   { "t"            , Key::t                 },
   { "u"            , Key::u                 },
   { "v"            , Key::v                 },
   { "w"            , Key::w                 },
   { "x"            , Key::x                 },
   { "y"            , Key::y                 },
   { "z"            , Key::z                 },
   { "{"            , Key::LEFT_CURLY_BRACE  },
   { "|"            , Key::PIPE              },
   { "}"            , Key::RIGHT_CURLY_BRACE },
   { "~"            , Key::TILDE             },
   { "<DELETE>"     , Key::DELETE            },
   { "<LEFT_ARROW>" , Key::LEFT_ARROW        },
   { "<UP_ARROW>"   , Key::UP_ARROW          },
   { "<RIGHT_ARROW>", Key::RIGHT_ARROW       },
   { "<DOWN_ARROW>" , Key::DOWN_ARROW        },
   { "<HOME>"       , Key::HOME              },
   { "<END>"        , Key::END               },
   { "<PAGE_UP>"    , Key::PAGE_UP           },
   { "<PAGE_DOWN>"  , Key::PAGE_DOWN         },
   { "<SHIFT>"      , Key::SHIFT             },
   { "<ALT>"        , Key::ALT               },
   { "<CTRL>"       , Key::CTRL              },
   { "<CMD>"        , Key::CMD               },
   { "<MENU>"       , Key::MENU              },
   { "<CAPS_LOCK>"  , Key::CAPS_LOCK         },
   { "<SCROLL_LOCK>", Key::SCROLL_LOCK       },
   { "<PAUSE>"      , Key::PAUSE             },
   { "<PRINT>"      , Key::PRINT             },
   { "<INSERT>"     , Key::INSERT            },
   { "<F1>"         , Key::F1                },
   { "<F2>"         , Key::F2                },
   { "<F3>"         , Key::F3                },
   { "<F4>"         , Key::F4                },
   { "<F5>"         , Key::F5                },
   { "<F6>"         , Key::F6                },
   { "<F7>"         , Key::F7                },
   { "<F8>"         , Key::F8                },
   { "<F9>"         , Key::F9                },
   { "<F10>"        , Key::F10               },
   { "<F11>"        , Key::F11               },
   { "<F12>"        , Key::F12               },
   { "<F13>"        , Key::F13               },
   { "<F14>"        , Key::F14               },
   { "<F15>"        , Key::F15               },
   { "<F16>"        , Key::F16               },
   { "<F17>"        , Key::F17               },
   { "<F18>"        , Key::F18               },
   { "<F19>"        , Key::F19               },
   { "<F20>"        , Key::F20               },
   { "<F21>"        , Key::F21               },
   { "<F22>"        , Key::F22               },
   { "<F23>"        , Key::F23               },
   { "<F24>"        , Key::F24               },
   { "<NUM_LOCK>"   , Key::NUM_LOCK          },
   //{ "<META>"     , Key::META              }, // Will be either CTRL or CMD.
   { 0, 0 }
};

//------------------------------------------------------------------------------
//!
void
initVM( VMState* vm, uint /*mask*/ )
{
   VM::registerEnum( vm, "UI.Key", _enumsKeys );
   VM::registerEnumReversed( vm, "UI.KeyToString", _enumsKeysReversed );
}

//------------------------------------------------------------------------------
//! A routine to convert a character into it's key code.
inline Key::Key  auxCharToKey( int c )
{
   switch( c )
   {
      case '\t': return Key::TAB;
      case '\f': return Key::LINEFEED;
      case '\n': return Key::NEWLINE;
      case ' ' : return Key::SPACE;
      case '!' : return Key::EXCLAMATION_MARK;
      case '"' : return Key::QUOTE;
      case '#' : return Key::HASH;
      case '$' : return Key::DOLLAR;
      case '%' : return Key::PERCENT;
      case '&' : return Key::AMPERSAND;
      case '\'': return Key::APOSTROPHE;
      case '(' : return Key::LEFT_PARENTHESIS;
      case ')' : return Key::RIGHT_PARENTHESIS;
      case '*' : return Key::ASTERISK;
      case '+' : return Key::PLUS;
      case ',' : return Key::COMMA;
      case '-' : return Key::MINUS;
      case '.' : return Key::PERIOD;
      case '/' : return Key::SLASH;
      case '0' : return Key::_0;
      case '1' : return Key::_1;
      case '2' : return Key::_2;
      case '3' : return Key::_3;
      case '4' : return Key::_4;
      case '5' : return Key::_5;
      case '6' : return Key::_6;
      case '7' : return Key::_7;
      case '8' : return Key::_8;
      case '9' : return Key::_9;
      case ':' : return Key::COLON;
      case ';' : return Key::SEMICOLON;
      case '<' : return Key::LT;
      case '=' : return Key::EQ;
      case '>' : return Key::GT;
      case '@' : return Key::AT;
      case 'A' : return Key::A;
      case 'B' : return Key::B;
      case 'C' : return Key::C;
      case 'D' : return Key::D;
      case 'E' : return Key::E;
      case 'F' : return Key::F;
      case 'G' : return Key::G;
      case 'H' : return Key::H;
      case 'I' : return Key::I;
      case 'J' : return Key::J;
      case 'K' : return Key::K;
      case 'L' : return Key::L;
      case 'M' : return Key::M;
      case 'N' : return Key::N;
      case 'O' : return Key::O;
      case 'P' : return Key::P;
      case 'Q' : return Key::Q;
      case 'R' : return Key::R;
      case 'S' : return Key::S;
      case 'T' : return Key::T;
      case 'U' : return Key::U;
      case 'V' : return Key::V;
      case 'W' : return Key::W;
      case 'X' : return Key::X;
      case 'Y' : return Key::Y;
      case 'Z' : return Key::Z;
      case '[' : return Key::LEFT_BRACKET;
      case '\\': return Key::BACKSLASH;
      case ']' : return Key::RIGHT_BRACKET;
      case '^' : return Key::CARET;
      case '_' : return Key::UNDERSCORE;
      case '`' : return Key::BACKQUOTE;
      case 'a' : return Key::a;
      case 'b' : return Key::b;
      case 'c' : return Key::c;
      case 'd' : return Key::d;
      case 'e' : return Key::e;
      case 'f' : return Key::f;
      case 'g' : return Key::g;
      case 'h' : return Key::h;
      case 'i' : return Key::i;
      case 'j' : return Key::j;
      case 'k' : return Key::k;
      case 'l' : return Key::l;
      case 'm' : return Key::m;
      case 'n' : return Key::n;
      case 'o' : return Key::o;
      case 'p' : return Key::p;
      case 'q' : return Key::q;
      case 'r' : return Key::r;
      case 's' : return Key::s;
      case 't' : return Key::t;
      case 'u' : return Key::u;
      case 'v' : return Key::v;
      case 'w' : return Key::w;
      case 'x' : return Key::x;
      case 'y' : return Key::y;
      case 'z' : return Key::z;
      case '{' : return Key::LEFT_CURLY_BRACE;
      case '|' : return Key::PIPE;
      case '}' : return Key::RIGHT_CURLY_BRACE;
      case '~' : return Key::TILDE;
      default  : return Key::NUL;
   }
}

UNNAMESPACE_END

/*==============================================================================
   NAMESPACE Key
==============================================================================*/
NAMESPACE_BEGIN

namespace Key
{

//------------------------------------------------------------------------------
//!
void initialize()
{
   VMRegistry::add( initVM, VM_CAT_APP );
}

//-----------------------------------------------------------------------------
//!
Key  charToKey( int c )
{
   return auxCharToKey( c );
}

} // namespace Key


NAMESPACE_END
