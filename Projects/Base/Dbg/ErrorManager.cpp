/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Base/Dbg/ErrorManager.h>
#include <Base/ADT/Vector.h>

/*==============================================================================
  UNNAME NAMESPACE
==============================================================================*/

UNNAMESPACE_BEGIN

//------------------------------------------------------------------------------
//!
Vector<String> _errors;

UNNAMESPACE_END
 
/*==============================================================================
  CLASS ErrorManager
==============================================================================*/


NAMESPACE_BEGIN

//------------------------------------------------------------------------------
//!
void
ErrorManager::addError
( const String& error )
{
   _errors.pushBack( error );
}

//------------------------------------------------------------------------------
//!
String
ErrorManager::getError
( bool remove )
{
   if( remove )
   {
      String error = _errors.back();
      _errors.popBack();
      return error;
   }

   return _errors.back();
}

//------------------------------------------------------------------------------
//!
bool
ErrorManager::hasErrors()
{
   return !_errors.empty();
}

//------------------------------------------------------------------------------
//!
void
ErrorManager::clear()
{
   _errors.clear();
}


NAMESPACE_END
