/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef BASE_SUBJECT_H
#define BASE_SUBJECT_H

#include <Base/StdDefs.h>
#include <Base/ADT/Vector.h>

NAMESPACE_BEGIN

class Observer;

/*==============================================================================
  CLASS Subject
==============================================================================*/

//! Abstract class for a subject.
//!
//! We could add later a guard to prevents multiple notification:
//! changeGuard( subject ) -> subject.notification++
//! ~changeGuard() -> subject.notification--; if 0 then notify;
//!
class Subject
{

public:

   /*----- methods -----*/

   BASE_DLL_API Subject();
   BASE_DLL_API virtual ~Subject();
   
   BASE_DLL_API void attach( Observer* );
   BASE_DLL_API void detach( Observer* );
   BASE_DLL_API void detachAll();
   BASE_DLL_API void notify();
   
private: 

   /*----- data members -----*/

   Vector< Observer* > _observers;
};

NAMESPACE_END

#endif
