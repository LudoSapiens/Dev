/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef BASE_RCOBJECT_H
#define BASE_RCOBJECT_H

#include <Base/StdDefs.h>

#include <Base/MT/Atomic.h>

NAMESPACE_BEGIN

/*==============================================================================
  CLASS RCObject
==============================================================================*/

//! Base class for reference count object.
//! Object deriving from this class are be default non-copyable.

class RCObject
{

public:

   /*----- methods -----*/

   inline void addReference() const;
   inline void removeReference() const;
   inline int count() const;
   inline bool isUnique() const;
protected:

   /*----- methods -----*/

   BASE_DLL_API RCObject();

   BASE_DLL_API virtual ~RCObject();


private:

   /*----- methods -----*/

   //! Disallow copying for this class and its children.
   RCObject( const RCObject& );
   RCObject& operator=( const RCObject& );

   /*----- data members -----*/

   mutable AtomicInt32 _count;
};

//------------------------------------------------------------------------------
//!
inline void
RCObject::addReference()  const
{
   ++_count;
}

//------------------------------------------------------------------------------
//!
inline void
RCObject::removeReference() const
{
   if( --_count == 0 )
   {
      delete this;
   }
}

//------------------------------------------------------------------------------
//!
inline int
RCObject::count() const
{
   return _count;
}

//------------------------------------------------------------------------------
//!
inline bool
RCObject::isUnique() const
{
   return _count == 1;
}


NAMESPACE_END

#endif //BASE_RCOBJECT_H
