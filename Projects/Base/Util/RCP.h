/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef BASE_RCP_H
#define BASE_RCP_H

#include <Base/StdDefs.h>

NAMESPACE_BEGIN

/*==============================================================================
  CLASS RCP
==============================================================================*/

//! Reference count pointer.

template< typename T > class RCP
{

public:

   /*----- methods -----*/

   template <class S> RCP<T>( const RCP<S>& ptr )
      : _pointee( ptr.ptr() )
   {
      init();
   }

   RCP( const RCP<T>& ptr );
   RCP( T* ptr = 0 );

   ~RCP();
   
   //@{
   //! Operators
   RCP& operator=( T* rhs );
   RCP& operator=( const RCP<T>& rhs );
   T*   operator->() const;
   T&   operator*() const;

   bool operator==( const T* rhs ) const;
   bool operator==( const RCP<const T>& rhs ) const;
   bool operator!=( const T* rhs ) const;
   bool operator!=( const RCP<const T>& rhs ) const;
   bool operator< ( const T* rhs ) const;
   bool operator< ( const RCP<const T>& rhs ) const;
   //@}
   bool isValid() const;
   bool isNull() const;
   bool isUnique() const;
   
   T*   ptr() const;

private:

   /*----- methods -----*/

   void init();

   /*----- data members -----*/

   T* _pointee;

};

//------------------------------------------------------------------------------
//!
template< typename T > inline void
RCP<T>::init()
{
   if( _pointee == 0 )
   {
      return;
   }
   _pointee->addReference();
}

//------------------------------------------------------------------------------
//!
template< typename T > inline
RCP<T>::RCP( T* ptr )
   : _pointee( ptr )
{
   init();
}

//------------------------------------------------------------------------------
//!
template< typename T > inline
RCP<T>::RCP( const RCP<T>& ptr )
   : _pointee( ptr._pointee )
{
   init();
}

//------------------------------------------------------------------------------
//!
template< typename T > inline
RCP<T>::~RCP()
{
   if( _pointee )
   {
      _pointee->removeReference();
   }
}

//------------------------------------------------------------------------------
//!
template< typename T > inline RCP<T>&
RCP<T>::operator=( T* rhs )
{
   if( _pointee != rhs )
   {
      T* pointee = _pointee;
      _pointee   = rhs;

      init();

      if( pointee )
      {
         pointee->removeReference();
      }
   }
   return *this;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline RCP<T>&
RCP<T>::operator=( const RCP<T>& rhs )
{
   if( _pointee != rhs._pointee )
   {
      T* pointee = _pointee;
      _pointee   = rhs._pointee;

      init();

      if( pointee )
      {
         pointee->removeReference();
      }
   }
   return *this;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline T*
RCP<T>::operator->() const
{
   return _pointee;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline T&
RCP<T>::operator*() const
{
   return *_pointee;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline bool
RCP<T>::operator==( const T* rhs ) const
{
   return _pointee == rhs;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline bool
RCP<T>::operator==( const RCP<const T>& rhs ) const
{
   return _pointee == rhs.ptr();
}

//------------------------------------------------------------------------------
//!
template< typename T > inline bool
RCP<T>::operator!=( const T* rhs ) const
{
   return _pointee != rhs;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline bool
RCP<T>::operator!=( const RCP<const T>& rhs ) const
{
   return _pointee != rhs.ptr();
}

//------------------------------------------------------------------------------
//!
template< typename T > inline bool
RCP<T>::operator<( const T* rhs ) const
{
   return _pointee < rhs;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline bool
RCP<T>::operator<( const RCP<const T>& rhs ) const
{
   return _pointee < rhs.ptr();
}

//-----------------------------------------------------------------------------
//!
template< typename T > inline bool
operator==( const T* lhs, const RCP<const T>& rhs )
{
   return lhs == rhs.ptr();
}

//-----------------------------------------------------------------------------
//!
template< typename T > inline bool
operator==( const T* lhs, const RCP<T>& rhs )
{
   return lhs == rhs.ptr();
}

//-----------------------------------------------------------------------------
//!
template< typename T > inline bool
operator!=( const T* lhs, const RCP<const T>& rhs )
{
   return lhs != rhs.ptr();
}

//-----------------------------------------------------------------------------
//!
template< typename T > inline bool
operator!=( const T* lhs, const RCP<T>& rhs )
{
   return lhs != rhs.ptr();
}

//-----------------------------------------------------------------------------
//!
template< typename T > inline bool
operator< ( const T* lhs, const RCP<const T>& rhs )
{
   return lhs < rhs.ptr();
}

//-----------------------------------------------------------------------------
//!
template< typename T > inline bool
operator< ( const T* lhs, const RCP<T>& rhs )
{
   return lhs < rhs.ptr();
}

//------------------------------------------------------------------------------
//!
template< typename T > inline bool
RCP<T>::isValid() const
{
   return _pointee != 0;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline bool
RCP<T>::isNull() const
{
   return _pointee == 0;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline bool
RCP<T>::isUnique() const
{
   return isValid() && _pointee->isUnique();
}

//------------------------------------------------------------------------------
//!
template< typename T > inline T*
RCP<T>::ptr() const
{
   return _pointee;
}

NAMESPACE_END

#endif
