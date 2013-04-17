/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef BASE_ERRORMANAGER_H
#define BASE_ERRORMANAGER_H

#include <Base/StdDefs.h>
#include <Base/ADT/String.h>

NAMESPACE_BEGIN

/*==============================================================================
  CLASS ErrorManager
==============================================================================*/

//!

class ErrorManager
{

public: 

   /*----- static methods -----*/

   static void addError( const String& );
   static String getError( bool remove = true );
   static bool hasErrors();
   static void clear();
};


NAMESPACE_END

#endif

