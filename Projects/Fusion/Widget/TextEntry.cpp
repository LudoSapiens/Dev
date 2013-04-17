/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Fusion/Widget/TextEntry.h>
#include <Fusion/VM/VMObjectPool.h>
#include <Fusion/Core/Core.h>
#include <Fusion/Core/Event.h>
#include <Fusion/Core/Key.h>

#include <Base/ADT/StringMap.h>
#include <Base/Util/UnicodeIterator.h>

#include <cctype>

/*==============================================================================
  UNNAME NAMESPACE
==============================================================================*/

UNNAMESPACE_BEGIN

//------------------------------------------------------------------------------
//!
inline void
execute( const VMRef& ref, Widget* widget )
{
   if( ref.isValid() )
   {
      VMState* vm = ref.vm();
      VM::push( vm, ref );
      VM::pushProxy( vm, widget );
      VM::ecall( vm, 1, 0 );
   }
}

//------------------------------------------------------------------------------
//!
inline void
execute( const VMRef& ref, Widget* widget, const String& text )
{
   if( ref.isValid() )
   {
      VMState* vm = ref.vm();
      VM::push( vm, ref );
      VM::pushProxy( vm, widget );
      VM::push( vm, text );
      VM::ecall( vm, 2, 0 );
   }
}

//------------------------------------------------------------------------------
//!
int  editVM( VMState* vm )
{
   TextEntry* te = (TextEntry*)VM::thisPtr( vm );
   VM::push( vm, te->edit() );
   return 1;
}

//------------------------------------------------------------------------------
//!
enum {
   ATTRIB_CURSOR,
   ATTRIB_EDIT,
   ATTRIB_IN_EDIT,
   ATTRIB_MULTI_EDIT,
   ATTRIB_ON_EDIT,
   ATTRIB_ON_END_EDIT,
   ATTRIB_ON_MODIFY,
   ATTRIB_ON_START_EDIT,
   ATTRIB_TEXT,
   ATTRIB_TEXT_DRAWABLE
};

StringMap _attributes(
   "cursor",         ATTRIB_CURSOR,
   "edit",           ATTRIB_EDIT,
   "inEdit",         ATTRIB_IN_EDIT,
   "multiEdit",      ATTRIB_MULTI_EDIT,
   "onEdit",         ATTRIB_ON_EDIT,
   "onEndEdit",      ATTRIB_ON_END_EDIT,
   "onModify",       ATTRIB_ON_MODIFY,
   "onStartEdit",    ATTRIB_ON_START_EDIT,
   "text",           ATTRIB_TEXT,
   "textDrawable",   ATTRIB_TEXT_DRAWABLE,
   ""
);

//------------------------------------------------------------------------------
//!
const char* _textEntry_str_ = "textEntry";

//-----------------------------------------------------------------------------
//!
bool  isPrint( char32_t codepoint )
{
   if( codepoint <= 0x007F )  return (isprint(codepoint) != 0);

   switch( codepoint )
   {
      case Key::LEFT_ARROW:
      case Key::UP_ARROW:
      case Key::RIGHT_ARROW:
      case Key::DOWN_ARROW:
         return false;
   }

   // Private user area.
   if( 0xE000 <= codepoint && codepoint <= 0xF8FF )
   {
      return false;
   }

   return true;
}

//-----------------------------------------------------------------------------
//!
String::SizeType  cursorToBytes( const String& str, uint cursor )
{
   UTF8Iterator it = str.cstr();
   it += cursor;
   return it.cur() - str.cstr();
}

UNNAMESPACE_END

NAMESPACE_BEGIN

/*==============================================================================
  CLASS TextEntry
==============================================================================*/

//------------------------------------------------------------------------------
//!
void
TextEntry::initialize()
{
   VMObjectPool::registerObject( "UI", _textEntry_str_, stdCreateVM<TextEntry>, stdGetVM<TextEntry>, stdSetVM<TextEntry> );
}

//------------------------------------------------------------------------------
//!
TextEntry::TextEntry():
   Widget(),
   _multiEdit( false ),
   _inEdit( false ),
   _cursor( 0 )
{
   hoverIcon( Pointer::TEXT );
}

//------------------------------------------------------------------------------
//!
TextEntry::~TextEntry()
{}

//------------------------------------------------------------------------------
//!
bool
TextEntry::edit()
{
   if( !_inEdit )
   {
      Core::grabFocus( this );
      _prevText   = _text;
      _inEdit     = true;
      _cursor     = uint(_text.length());
      _onStartEdit.exec();
      execute( _onStartEditRef, this );
      return true;
   }
   else
   {
      return false;
   }
}

//------------------------------------------------------------------------------
//!
void
TextEntry::addOnModify( const Delegate1<String>& delegate )
{
   _onModify.addDelegate( delegate );
}

//------------------------------------------------------------------------------
//!
void
TextEntry::removeOnModify( const Delegate1<String>& delegate )
{
   _onModify.removeDelegate( delegate );
}

//------------------------------------------------------------------------------
//!
void
TextEntry::addOnStartEdit( const Delegate0<>& delegate )
{
   _onStartEdit.addDelegate( delegate );
}

//------------------------------------------------------------------------------
//!
void
TextEntry::removeOnStartEdit( const Delegate0<>& delegate )
{
   _onStartEdit.removeDelegate( delegate );
}

//------------------------------------------------------------------------------
//!
void
TextEntry::addOnEdit( const Delegate0<>& delegate )
{
   _onEdit.addDelegate( delegate );
}

//------------------------------------------------------------------------------
//!
void
TextEntry::removeOnEdit( const Delegate0<>& delegate )
{
   _onEdit.removeDelegate( delegate );
}

//------------------------------------------------------------------------------
//!
void
TextEntry::addOnEndEdit( const Delegate0<>& delegate )
{
   _onEndEdit.addDelegate( delegate );
}

//------------------------------------------------------------------------------
//!
void
TextEntry::removeOnEndEdit( const Delegate0<>& delegate )
{
   _onEndEdit.removeDelegate( delegate );
}

//------------------------------------------------------------------------------
//!
void
TextEntry::onPointerPress( const Event& ev )
{
   if( !_inEdit ) edit();
   if( _textDrawable.isValid() ) _cursor = _textDrawable->charIndex( ev.position() );
   Widget::onPointerPress( ev );
}

//------------------------------------------------------------------------------
//!
void
TextEntry::onKeyPress( const Event& ev )
{
   Widget::onKeyPress( ev );
   if( !_inEdit ) return;

   switch( ev.value() )
   {
      case Key::BACKSPACE:
      {
         if( _cursor > 0 )
         {
            --_cursor;

            // Need to erase the UTF8 character at _cursor.
            UTF8Iterator it = _text.cstr();
            it += _cursor;
            const char* s = it.cur();
            ++it;
            const char* e = it.cur();
            _text.erase( s-_text.cstr(), e-s );

            _onModify.exec( _text );
            execute( _onModifyRef, this, _text );
         }
      } break;
      case Key::RETURN:
      {
         if( _multiEdit )
         {
            _onEdit.exec();
            execute( _onEditRef, this );
         }
         else
            Core::releaseFocus( this );
      } break;
      case Key::ESC:
      {
         if( _text != _prevText )
         {
            _text = _prevText;
            _onModify.exec( _text );
            execute( _onModifyRef, this, _text );
         }
         Core::releaseFocus( this );
      } break;
      case Key::END:
      {
         _cursor = uint(_text.length());
      } break;
      case Key::HOME:
      {
         _cursor = 0;
      } break;
      case Key::LEFT_ARROW:
      {
         if( _cursor > 0 ) --_cursor;
      } break;
      case Key::RIGHT_ARROW:
      {
         if( _cursor < _text.length() ) ++_cursor;
      } break;
      case Key::DELETE:
      {
         if( _cursor < _text.length() )
         {
            String::SizeType pos = cursorToBytes( _text, _cursor );
            _text.erase( pos, getUTF8Size(_text.cstr() + pos) );
            _onModify.exec( _text );
            execute( _onModifyRef, this, _text );
         }
      } break;
      default:
         return;
   }
   updateLook();
}

//------------------------------------------------------------------------------
//!
void
TextEntry::onChar( const Event& ev )
{
   Widget::onChar( ev );
   if( !_inEdit ) return;

   char32_t codepoint = (char32_t)ev.value();

   String str = toStr(codepoint);
   if( !str.empty() && isPrint(codepoint) )
   {
      String::SizeType pos = cursorToBytes( _text, _cursor );
      if( pos < _text.size() )
      {
         _text.insert( pos, str );
      }
      else
      {
         _text += str;
      }
      ++_cursor;

      _onModify.exec( _text );
      execute( _onModifyRef, this, _text );

      updateLook();
   }
}

//------------------------------------------------------------------------------
//!
void
TextEntry::onFocusLose( const Event& ev )
{
   _onEdit.exec();
   execute( _onEditRef, this );
   _inEdit = false;
   _onEndEdit.exec();
   execute( _onEndEditRef, this );
   Widget::onFocusLose( ev );
}

//------------------------------------------------------------------------------
//!
void
TextEntry::text( const String& val, bool update )
{
   if( val == _text ) return;

   _text   = val;
   _cursor = uint(_text.length());

   if( update )
   {
      _onModify.exec( _text );
      execute( _onModifyRef, this, _text );
   }

   updateLook();
}

//------------------------------------------------------------------------------
//!
const char*
TextEntry::meta() const
{
   return _textEntry_str_;
}

//------------------------------------------------------------------------------
//!
void
TextEntry::init( VMState* vm )
{
   VM::get( vm, 1, "text", _text );
   VM::get( vm, 1, "multiEdit", _multiEdit );
   VM::get( vm, 1, "onEdit", _onEditRef );
   VM::get( vm, 1, "onEndEdit", _onEndEditRef );
   VM::get( vm, 1, "onModify", _onModifyRef );
   VM::get( vm, 1, "onStartEdit", _onStartEditRef );

   if( !VM::get( vm, 1, "cursor", _cursor ) ) _cursor = uint(_text.length());

   // Base class init.
   Widget::init( vm );
}

//------------------------------------------------------------------------------
//!
bool
TextEntry::performGet( VMState* vm )
{
   switch( _attributes[ VM::toCString( vm, 2 ) ] )
   {
      case ATTRIB_CURSOR:
         VM::push( vm, _cursor );
         return true;
      case ATTRIB_EDIT:
         VM::push( vm, this, editVM );
         return true;
      case ATTRIB_IN_EDIT:
         VM::push( vm, _inEdit );
         return true;
      case ATTRIB_MULTI_EDIT:
         VM::push( vm, _multiEdit );
         return true;
      case ATTRIB_ON_EDIT:
         VM::push( vm, _onEditRef );
         return true;
      case ATTRIB_ON_END_EDIT:
         VM::push( vm, _onEndEditRef );
         return true;
      case ATTRIB_ON_MODIFY:
         VM::push( vm, _onModifyRef );
         return true;
      case ATTRIB_ON_START_EDIT:
         VM::push( vm, _onStartEditRef );
         return true;
      case ATTRIB_TEXT:
         VM::push( vm, _text );
         return true;
      case ATTRIB_TEXT_DRAWABLE:
         VM::pushProxy( vm, _textDrawable.ptr() );
         return true;
      default: break;
   }
   return Widget::performGet( vm );
}

//------------------------------------------------------------------------------
//!
bool
TextEntry::performSet( VMState* vm )
{
   switch( _attributes[ VM::toCString( vm, 2 ) ] )
   {
      case ATTRIB_CURSOR:
         _cursor = VM::toUInt( vm, 3 );
         return true;
      case ATTRIB_EDIT:   // Read only
      case ATTRIB_IN_EDIT: // Read only
         return true;
      case ATTRIB_MULTI_EDIT:
         multiEdit( VM::toBoolean( vm, 3 ) );
         return true;
      case ATTRIB_ON_EDIT:
         VM::toRef( vm, 3, _onEditRef );
         return true;
      case ATTRIB_ON_END_EDIT:
         VM::toRef( vm, 3, _onEndEditRef );
         return true;
      case ATTRIB_ON_MODIFY:
         VM::toRef( vm, 3, _onModifyRef );
         return true;
      case ATTRIB_ON_START_EDIT:
         VM::toRef( vm, 3, _onStartEditRef );
         return true;
      case ATTRIB_TEXT:
         text( VM::toCString( vm, 3 ) );
         return true;
      case ATTRIB_TEXT_DRAWABLE:
         _textDrawable = (Text*)VM::toProxy( vm, 3 );
         return true;
      default: break;
   }
   return Widget::performSet( vm );
}

//------------------------------------------------------------------------------
//!
bool
TextEntry::isAttribute( const char* name ) const
{
   if( _attributes[ name ] != StringMap::INVALID ) return true;
   return Widget::isAttribute( name );
}

NAMESPACE_END
