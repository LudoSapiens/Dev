/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Base/Msg/Subject.h>
#include <Base/Msg/Observer.h>

NAMESPACE_BEGIN

/*==============================================================================
  CLASS Subject
==============================================================================*/

//------------------------------------------------------------------------------
//!
Subject::Subject()
{}

//------------------------------------------------------------------------------
//!
Subject::~Subject()
{
   Vector< Observer* >::Iterator it  = _observers.begin();
   Vector< Observer* >::Iterator end = _observers.end();

   for( ; it != end; ++it )
   {
      (*it)->destroy();
   }
}

//------------------------------------------------------------------------------
//!
void
Subject::attach
( Observer* observer )
{
   _observers.pushBack( observer );
}

//------------------------------------------------------------------------------
//!
void
Subject::detach
( Observer* observer )
{
   _observers.remove( observer );
}

//------------------------------------------------------------------------------
//!
void
Subject::detachAll
( void )
{
   _observers.clear();
}

//------------------------------------------------------------------------------
//!
void
Subject::notify()
{
   Vector< Observer* >::Iterator it  = _observers.begin();
   Vector< Observer* >::Iterator end = _observers.end();

   for( ; it != end; ++it )
   {
      (*it)->update();
   }
}

NAMESPACE_END
