/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef FUSION_VALUEEDITOR_H
#define FUSION_VALUEEDITOR_H

#include <Fusion/StdDefs.h>
#include <Fusion/Core/Event.h>
#include <Fusion/Widget/Widget.h>

#include <Base/ADT/String.h>


NAMESPACE_BEGIN

/*==============================================================================
  CLASS ValueEditor
==============================================================================*/

//!
//! Attributes list for WM:
//!
//!   read only:
//!     editText:  The edit text of the value.
//!     cursor:  Position of the text cursor in character.
//!     inEdit:  True if the value is in text edit mode.
//!
//!
//!   read/write:
//!     min: Minimum accepted value.
//!     max:  Maximum accepted value.
//!     step:  Increment used to modify the value.
//!     value:  Current floating point value.
//!
//!   callBacks -> the params are ( widget, value )
//!     onModify:  Cb called when the value change.

class ValueEditor
   : public Widget
{

public:

   /*----- static methods -----*/

   static void initialize();

   /*----- methods -----*/

   FUSION_DLL_API ValueEditor();

   FUSION_DLL_API void value( float val, bool update = true );
   FUSION_DLL_API void format( const char* fmt );

   //! Callback
   FUSION_DLL_API void addOnModify( const Delegate1<float>& );
   FUSION_DLL_API void removeOnModify( const Delegate1<float>& );

   //! Event handlers
   FUSION_DLL_API virtual void onPointerMove( const Event& );
   FUSION_DLL_API virtual void onPointerPress( const Event& );
   FUSION_DLL_API virtual void onPointerRelease( const Event& );
   FUSION_DLL_API virtual void onKeyPress( const Event& );
   FUSION_DLL_API virtual void onChar( const Event& );
   FUSION_DLL_API virtual void onFocusLose( const Event& );

   // VM.
   const char* meta() const;
   FUSION_DLL_API void init( VMState* vm );
   FUSION_DLL_API bool performGet( VMState* );
   FUSION_DLL_API bool performSet( VMState* );

protected:

   /*----- methods -----*/

   virtual ~ValueEditor();

   virtual bool isAttribute( const char* ) const;

private:

   /*----- data members -----*/

   bool                 _integer;
   bool                 _inEdit;
   float                _value;
   float                _minValue;
   float                _maxValue;
   float                _step;
   String               _editText;
   String               _format;
   uint                 _cursor;
   Delegate1List<float> _onModify;
   VMRef                _onModifyRef;
};

NAMESPACE_END

#endif
