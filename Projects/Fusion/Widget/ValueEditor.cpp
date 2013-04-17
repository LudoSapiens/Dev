/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Fusion/Widget/ValueEditor.h>
#include <Fusion/VM/VMObjectPool.h>
#include <Fusion/Core/Core.h>
#include <Fusion/Core/Event.h>
#include <Fusion/Core/Key.h>

#include <CGMath/CGMath.h>

#include <Base/ADT/StringMap.h>

/*==============================================================================
  UNNAME NAMESPACE
==============================================================================*/

UNNAMESPACE_BEGIN

//------------------------------------------------------------------------------
//!
inline bool hasMoved( Pointer& ptr )
{
   return !ptr.withinPress( 5.0f );
}

//------------------------------------------------------------------------------
//!
inline void
execute( const VMRef& ref, Widget* widget, float value )
{
   if( ref.isValid() )
   {
      VMState* vm = ref.vm();
      VM::push( vm, ref );
      VM::pushProxy( vm, widget );
      VM::push( vm, value );
      VM::ecall( vm, 2, 0 );
   }
}

//------------------------------------------------------------------------------
//!
enum {
   ATTRIB_INTEGER,
   ATTRIB_MIN,
   ATTRIB_MAX,
   ATTRIB_STEP,
   ATTRIB_VALUE,
   ATTRIB_EDITTEXT,
   ATTRIB_FORMAT,
   ATTRIB_CURSOR,
   ATTRIB_INEDIT,
   ATTRIB_ONMODIFY
};

StringMap _attributes(
   "integer",    ATTRIB_INTEGER,
   "min",        ATTRIB_MIN,
   "max",        ATTRIB_MAX,
   "step",       ATTRIB_STEP,
   "value",      ATTRIB_VALUE,
   "editText",   ATTRIB_EDITTEXT,
   "format",     ATTRIB_FORMAT,
   "cursor",     ATTRIB_CURSOR,
   "inEdit",     ATTRIB_INEDIT,
   "onModify",   ATTRIB_ONMODIFY,
   ""
);

//------------------------------------------------------------------------------
//!
const char* _valueEditor_str_ = "valueEditor";

UNNAMESPACE_END

NAMESPACE_BEGIN

/*==============================================================================
  CLASS ValueEditor
==============================================================================*/

//------------------------------------------------------------------------------
//!
void
ValueEditor::initialize()
{
   VMObjectPool::registerObject( "UI", _valueEditor_str_, stdCreateVM<ValueEditor>, stdGetVM<ValueEditor>, stdSetVM<ValueEditor> );
}

//------------------------------------------------------------------------------
//!
ValueEditor::ValueEditor():
   Widget(),
   _integer( false ),
   _inEdit( false ),
   _value( 0.0f ),
   _minValue( 0.0f ),
   _maxValue( 100.0f ),
   _step( 0.1f ),
   _format( "%.3g" ),
   _cursor( 0 )
{
}

//------------------------------------------------------------------------------
//!
ValueEditor::~ValueEditor()
{}

//------------------------------------------------------------------------------
//!
void
ValueEditor::addOnModify( const Delegate1<float>& delegate )
{
   _onModify.addDelegate( delegate );
}

//------------------------------------------------------------------------------
//!
void
ValueEditor::removeOnModify( const Delegate1<float>& delegate )
{
   _onModify.removeDelegate( delegate );
}

//------------------------------------------------------------------------------
//!
void
ValueEditor::onPointerMove( const Event& ev )
{
   // Update value?
   Pointer& ptr = ev.pointer();
   if( !_inEdit && pressed() && hasMoved( ptr ) )
   {
      float dx  = ptr.deltaPosition().x;
      float f   = CGM::round( CGM::sign(dx) * CGM::pow( CGM::abs(dx), 1.5f ) );
      float val = _value + _step * f;
      value( val );
   }

   Widget::onPointerMove( ev );
}

//------------------------------------------------------------------------------
//!
void
ValueEditor::onPointerPress( const Event& ev )
{
   Widget::onPointerPress( ev );
}

//------------------------------------------------------------------------------
//!
void
ValueEditor::onPointerRelease( const Event& ev )
{
   // Get into editing mode.
   Pointer& ptr = ev.pointer();
   if( !_inEdit && !hasMoved( ptr ) && pressed() )
   {
      Core::grabFocus( this );
      _inEdit = true;
      _editText.format( _format.cstr(), _value );
      _cursor = uint(_editText.size());
   }
   Widget::onPointerRelease( ev );
}

//------------------------------------------------------------------------------
//!
void
ValueEditor::onKeyPress( const Event& ev )
{
   Widget::onKeyPress( ev );
   if( !_inEdit ) return;

   switch( ev.value() )
   {
      case Key::BACKSPACE:
         if( _cursor > 0 )
         {
            --_cursor;
            _editText.erase( _cursor );
         }
         break;
      case Key::RETURN:
         Core::releaseFocus( this );
         break;
      case Key::ESC:
         _editText.format( _format.cstr(), _value );
         Core::releaseFocus( this );
         break;
      case Key::END:
         _cursor = uint(_editText.size());
         break;
      case Key::HOME:
         _cursor = 0;
         break;
      case Key::LEFT_ARROW:
         if( _cursor > 0 ) --_cursor;
         break;
      case Key::RIGHT_ARROW:
         if( _cursor < _editText.size() ) ++_cursor;
         break;
      case Key::DELETE:
         if( _cursor < _editText.size() ) _editText.erase( _cursor );
         break;
      default:
         return;
   }
   updateLook();
}

//------------------------------------------------------------------------------
//!
void
ValueEditor::onChar( const Event& ev )
{
   Widget::onChar( ev );
   if( !_inEdit ) return;

   if( isprint( ev.value() ) )
   {
      char chr = (char)ev.value();
      if( _cursor < _editText.size() )
         _editText.insert( _cursor, chr );
      else
         _editText += chr;
      ++_cursor;

      updateLook();
   }
}

//------------------------------------------------------------------------------
//!
void
ValueEditor::onFocusLose( const Event& ev )
{
   value( _editText.toFloat() );
   _inEdit = false;
   _editText.clear();
   Widget::onFocusLose( ev );
}

//------------------------------------------------------------------------------
//!
void
ValueEditor::value( float val, bool update )
{
   // Set the value to be in the range.
   val = CGM::clamp( val, _minValue, _maxValue );

   // Convert to integer.
   if( _integer ) val = CGM::round( val );

   if( val == _value ) return;

   _value = val;

   if( update && eventsEnabled() )
   {
      _onModify.exec( _value );
      execute( _onModifyRef, this, _value );
   }

   updateLook();
}

//------------------------------------------------------------------------------
//!
void
ValueEditor::format( const char* fmt )
{
   if( _format == fmt ) return;

   _format = fmt;

   updateLook();
}

//------------------------------------------------------------------------------
//!
const char*
ValueEditor::meta() const
{
   return _valueEditor_str_;
}

//------------------------------------------------------------------------------
//!
void
ValueEditor::init( VMState* vm )
{
   VM::get( vm, 1, "integer", _integer );
   VM::get( vm, 1, "min", _minValue );
   VM::get( vm, 1, "max", _maxValue );
   VM::get( vm, 1, "step", _step );
   VM::get( vm, 1, "value", _value );
   VM::get( vm, 1, "format", _format );
   VM::get( vm, 1, "onModify", _onModifyRef );

   _value = CGM::clamp( _value, _minValue, _maxValue );

   // Base class init.
   Widget::init( vm );
}

//------------------------------------------------------------------------------
//!
bool
ValueEditor::performGet( VMState* vm )
{
   switch( _attributes[ VM::toCString( vm, 2 ) ] )
   {
      case ATTRIB_INTEGER:
         VM::push( vm, _integer );
         return true;
      case ATTRIB_MIN:
         VM::push( vm, _minValue );
         return true;
      case ATTRIB_MAX:
         VM::push( vm, _maxValue );
         return true;
      case ATTRIB_STEP:
         VM::push( vm, _step );
         return true;
      case ATTRIB_VALUE:
         VM::push( vm, _value );
         return true;
      case ATTRIB_EDITTEXT:
         VM::push( vm, _editText );
         return true;
      case ATTRIB_FORMAT:
         VM::push( vm, _format );
         return true;
      case ATTRIB_CURSOR:
         VM::push( vm, _cursor );
         return true;
      case ATTRIB_INEDIT:
         VM::push( vm, _inEdit );
         return true;
      case ATTRIB_ONMODIFY:
         VM::push( vm, _onModifyRef );
         return true;
      default: break;
   }
   return Widget::performGet( vm );
}

//------------------------------------------------------------------------------
//!
bool
ValueEditor::performSet( VMState* vm )
{
   switch( _attributes[ VM::toCString( vm, 2 ) ] )
   {
      case ATTRIB_INTEGER:
         _integer = VM::toBoolean( vm, 3 );
         return true;
      case ATTRIB_MIN:
         _minValue = VM::toFloat( vm, 3 );
         value( _value );
         return true;
      case ATTRIB_MAX:
         _maxValue = VM::toFloat( vm, 3 );
         value( _value );
         return true;
      case ATTRIB_STEP:
         _step = VM::toFloat( vm, 3 );
         return true;
      case ATTRIB_VALUE:
         value( VM::toFloat( vm, 3 ) );
         return true;
      case ATTRIB_EDITTEXT: // Read only.
         return true;
      case ATTRIB_FORMAT:
         format( VM::toCString( vm, 3 ) );
         return true;
      case ATTRIB_CURSOR:
         _cursor = VM::toUInt( vm, 3 );
         return true;
      case ATTRIB_INEDIT: // Read only.
         return true;
      case ATTRIB_ONMODIFY:
         VM::toRef( vm, 3, _onModifyRef );
         return true;
      default: break;
   }
   return Widget::performSet( vm );
}

//------------------------------------------------------------------------------
//!
bool
ValueEditor::isAttribute( const char* name ) const
{
   if( _attributes[ name ] != StringMap::INVALID ) return true;
   return Widget::isAttribute( name );
}

NAMESPACE_END
