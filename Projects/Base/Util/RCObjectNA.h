/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef BASE_RCOBJECT_NON_ATOMIC_H
#define BASE_RCOBJECT_NON_ATOMIC_H

#include <Base/StdDefs.h>

NAMESPACE_BEGIN

/*==============================================================================
  CLASS RCObjectNA
==============================================================================*/

//! Base class for reference count object.
//! Object deriving from this class are be default non-copyable.

class RCObjectNA
{

public:

   /*----- methods -----*/

   inline void addReference() const;
   inline void removeReference() const;
   inline int count() const;
   inline bool isUnique() const;
protected:

   /*----- methods -----*/

   BASE_DLL_API RCObjectNA();

   BASE_DLL_API virtual ~RCObjectNA();


private:

   /*----- methods -----*/

   //! Disallow copying for this class and its children.
   RCObjectNA( const RCObjectNA& );
   RCObjectNA& operator=( const RCObjectNA& );

   /*----- data members -----*/

   mutable int _count;
};

//------------------------------------------------------------------------------
//!
inline void
RCObjectNA::addReference()  const
{
   ++_count;
}

//------------------------------------------------------------------------------
//!
inline void
RCObjectNA::removeReference() const
{
   if( --_count == 0 )
   {
      delete this;
   }
}

//------------------------------------------------------------------------------
//!
inline int
RCObjectNA::count() const
{
   return _count;
}

//------------------------------------------------------------------------------
//!
inline bool
RCObjectNA::isUnique() const
{
   return _count == 1;
}


NAMESPACE_END

#endif //BASE_RCOBJECT_NON_ATOMIC_H
