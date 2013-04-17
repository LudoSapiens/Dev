/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef BASE_DELEGATE_LIST_H
#define BASE_DELEGATE_LIST_H

#include <Base/StdDefs.h>

#include <Base/ADT/Vector.h>
#include <Base/Msg/Delegate.h>

NAMESPACE_BEGIN

/*==============================================================================
  CLASS Delegate0List
==============================================================================*/
//! Delegate list for functions with no parameter.
template< typename RT=void >
class Delegate0List
{
public:

   /*----- types -----*/
   typedef typename Delegate0<RT>::type               DelegateType;
   typedef Vector< DelegateType >                     DelegateContainer;
   typedef typename DelegateContainer::Iterator       Iterator;
   typedef typename DelegateContainer::ConstIterator  ConstIterator;

   /*----- methods -----*/

   Delegate0List() {}

   void addDelegate( const DelegateType& delegate )
   {
      _delegates.pushBack( delegate );
   }

   void removeDelegate( const DelegateType& delegate )
   {
      _delegates.remove( delegate );
   }

   void clear()
   {
      _delegates.clear();
   }

   void clearEmpty( const DelegateType& v = DelegateType() )
   {
      _delegates.removeAllSwap( v );
   }

   bool empty()
   {
      return _delegates.empty();
   }

   size_t size()
   {
      return _delegates.size();
   }

   Iterator       begin()       { return _delegates.begin(); }
   Iterator       end()         { return _delegates.end();   }

   ConstIterator  begin() const { return _delegates.begin(); }
   ConstIterator  end()   const { return _delegates.end();   }

   void exec() const
   {
      typename DelegateContainer::ConstIterator it  = _delegates.begin();
      typename DelegateContainer::ConstIterator end = _delegates.end();

      for ( ; it != end; ++it )
      {
         (*it)();
      }
   }

private:

   /*----- data members -----*/

   DelegateContainer  _delegates;
}; //class Delegate0List


/*==============================================================================
  CLASS Delegate1List
==============================================================================*/
//! Delegate list for functions with one parameter.
template< typename P1, typename RT=void >
class Delegate1List
{
public:

   /*----- types -----*/
   typedef typename Delegate1<P1, RT>::type           DelegateType;
   typedef Vector< DelegateType >                     DelegateContainer;
   typedef typename DelegateContainer::Iterator       Iterator;
   typedef typename DelegateContainer::ConstIterator  ConstIterator;

   /*----- methods -----*/

   Delegate1List() {}

   void addDelegate( const DelegateType& delegate )
   {
      _delegates.pushBack( delegate );
   }

   void removeDelegate( const DelegateType& delegate )
   {
      _delegates.remove( delegate );
   }

   void clear()
   {
      _delegates.clear();
   }

   void clearEmpty( const DelegateType& v = DelegateType() )
   {
      _delegates.removeAllSwap( v );
   }

   bool empty()
   {
      return _delegates.empty();
   }

   size_t size()
   {
      return _delegates.size();
   }

   Iterator       begin()       { return _delegates.begin(); }
   Iterator       end()         { return _delegates.end();   }

   ConstIterator  begin() const { return _delegates.begin(); }
   ConstIterator  end()   const { return _delegates.end();   }

   void exec( P1 p1 ) const
   {
      typename DelegateContainer::ConstIterator it  = _delegates.begin();
      typename DelegateContainer::ConstIterator end = _delegates.end();

      for ( ; it != end; ++it )
      {
         (*it)( p1 );
      }
   }

private:

   /*----- data members -----*/

   DelegateContainer  _delegates;
}; //class Delegate1List


/*==============================================================================
  CLASS Delegate2List
==============================================================================*/
//! Delegate list for functions with two parameters.
template< typename P1, typename P2, typename RT=void >
class Delegate2List
{
public:

   /*----- types -----*/
   typedef typename Delegate2<P1, P2, RT>::type       DelegateType;
   typedef Vector< DelegateType >                     DelegateContainer;
   typedef typename DelegateContainer::Iterator       Iterator;
   typedef typename DelegateContainer::ConstIterator  ConstIterator;

   /*----- methods -----*/

   Delegate2List() {}

   void addDelegate( const DelegateType& delegate )
   {
      _delegates.pushBack( delegate );
   }

   void removeDelegate( const DelegateType& delegate )
   {
      _delegates.remove( delegate );
   }

   void clear()
   {
      _delegates.clear();
   }

   void clearEmpty( const DelegateType& v = DelegateType() )
   {
      _delegates.removeAllSwap( v );
   }

   bool empty()
   {
      return _delegates.empty();
   }

   size_t size()
   {
      return _delegates.size();
   }

   Iterator       begin()       { return _delegates.begin(); }
   Iterator       end()         { return _delegates.end();   }

   ConstIterator  begin() const { return _delegates.begin(); }
   ConstIterator  end()   const { return _delegates.end();   }

   void exec( P1 p1, P2 p2 ) const
   {
      typename DelegateContainer::ConstIterator it  = _delegates.begin();
      typename DelegateContainer::ConstIterator end = _delegates.end();

      for ( ; it != end; ++it )
      {
         (*it)( p1, p2 );
      }
   }

private:

   /*----- data members -----*/

   DelegateContainer  _delegates;
}; //class Delegate2List


/*==============================================================================
  CLASS Delegate3List
==============================================================================*/
//! Delegate list for functions with three parameters.
template< typename P1, typename P2, typename P3, typename RT=void >
class Delegate3List
{
public:

   /*----- types -----*/
   typedef typename Delegate3<P1, P2, P3, RT>::type   DelegateType;
   typedef Vector< DelegateType >                     DelegateContainer;
   typedef typename DelegateContainer::Iterator       Iterator;
   typedef typename DelegateContainer::ConstIterator  ConstIterator;

   /*----- methods -----*/

   Delegate3List() {}

   void addDelegate( const DelegateType& delegate )
   {
      _delegates.pushBack( delegate );
   }

   void removeDelegate( const DelegateType& delegate )
   {
      _delegates.remove( delegate );
   }

   void clear()
   {
      _delegates.clear();
   }

   void clearEmpty( const DelegateType& v = DelegateType() )
   {
      _delegates.removeAllSwap( v );
   }

   bool empty()
   {
      return _delegates.empty();
   }

   size_t size()
   {
      return _delegates.size();
   }

   Iterator       begin()       { return _delegates.begin(); }
   Iterator       end()         { return _delegates.end();   }

   ConstIterator  begin() const { return _delegates.begin(); }
   ConstIterator  end()   const { return _delegates.end();   }

   void exec( P1 p1, P2 p2, P3 p3 ) const
   {
      typename DelegateContainer::ConstIterator it  = _delegates.begin();
      typename DelegateContainer::ConstIterator end = _delegates.end();

      for ( ; it != end; ++it )
      {
         (*it)( p1, p2, p3 );
      }
   }

private:

   /*----- data members -----*/

   DelegateContainer  _delegates;
}; //class Delegate3List


/*==============================================================================
  CLASS Delegate4List
==============================================================================*/
//! Delegate list for functions with four parameters.
template< typename P1, typename P2, typename P3, typename P4, typename RT=void >
class Delegate4List
{
public:

   /*----- types -----*/
   typedef typename Delegate4<P1, P2, P3, P4, RT>::type  DelegateType;
   typedef Vector< DelegateType >                        DelegateContainer;
   typedef typename DelegateContainer::Iterator          Iterator;
   typedef typename DelegateContainer::ConstIterator     ConstIterator;

   /*----- methods -----*/

   Delegate4List() {}

   void addDelegate( const DelegateType& delegate )
   {
      _delegates.pushBack( delegate );
   }

   void removeDelegate( const DelegateType& delegate )
   {
      _delegates.remove( delegate );
   }

   void clear()
   {
      _delegates.clear();
   }

   void clearEmpty( const DelegateType& v = DelegateType() )
   {
      _delegates.removeAllSwap( v );
   }

   bool empty()
   {
      return _delegates.empty();
   }

   size_t size()
   {
      return _delegates.size();
   }

   Iterator       begin()       { return _delegates.begin(); }
   Iterator       end()         { return _delegates.end();   }

   ConstIterator  begin() const { return _delegates.begin(); }
   ConstIterator  end()   const { return _delegates.end();   }

   void exec( P1 p1, P2 p2, P3 p3, P4 p4 ) const
   {
      typename DelegateContainer::ConstIterator it  = _delegates.begin();
      typename DelegateContainer::ConstIterator end = _delegates.end();

      for ( ; it != end; ++it )
      {
         (*it)( p1, p2, p3, p4 );
      }
   }

private:

   /*----- data members -----*/

   DelegateContainer  _delegates;
}; //class Delegate4List


/*==============================================================================
  CLASS Delegate5List
==============================================================================*/
//! Delegate list for functions with five parameters.
template< typename P1, typename P2, typename P3, typename P4, typename P5, typename RT=void >
class Delegate5List
{
public:

   /*----- types -----*/
   typedef typename Delegate5<P1, P2, P3, P4, P5, RT>::type  DelegateType;
   typedef Vector< DelegateType >                            DelegateContainer;
   typedef typename DelegateContainer::Iterator              Iterator;
   typedef typename DelegateContainer::ConstIterator         ConstIterator;

   /*----- methods -----*/

   Delegate5List() {}

   void addDelegate( const DelegateType& delegate )
   {
      _delegates.pushBack( delegate );
   }

   void removeDelegate( const DelegateType& delegate )
   {
      _delegates.remove( delegate );
   }

   void clear()
   {
      _delegates.clear();
   }

   void clearEmpty( const DelegateType& v = DelegateType() )
   {
      _delegates.removeAllSwap( v );
   }

   bool empty()
   {
      return _delegates.empty();
   }

   size_t size()
   {
      return _delegates.size();
   }

   Iterator       begin()       { return _delegates.begin(); }
   Iterator       end()         { return _delegates.end();   }

   ConstIterator  begin() const { return _delegates.begin(); }
   ConstIterator  end()   const { return _delegates.end();   }

   void exec( P1 p1, P2 p2, P3 p3, P4 p4, P5 p5 ) const
   {
      typename DelegateContainer::ConstIterator it  = _delegates.begin();
      typename DelegateContainer::ConstIterator end = _delegates.end();

      for ( ; it != end; ++it )
      {
         (*it)( p1, p2, p3, p4, p5 );
      }
   }

private:

   /*----- data members -----*/

   DelegateContainer  _delegates;
}; //class Delegate5List


/*==============================================================================
  CLASS Delegate6List
==============================================================================*/
//! Delegate list for functions with six parameters.
template< typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename RT=void >
class Delegate6List
{
public:

   /*----- types -----*/
   typedef typename Delegate6<P1, P2, P3, P4, P5, P6, RT>::type  DelegateType;
   typedef Vector< DelegateType >                                DelegateContainer;
   typedef typename DelegateContainer::Iterator                  Iterator;
   typedef typename DelegateContainer::ConstIterator             ConstIterator;

   /*----- methods -----*/

   Delegate6List() {}

   void addDelegate( const DelegateType& delegate )
   {
      _delegates.pushBack( delegate );
   }

   void removeDelegate( const DelegateType& delegate )
   {
      _delegates.remove( delegate );
   }

   void clear()
   {
      _delegates.clear();
   }

   void clearEmpty( const DelegateType& v = DelegateType() )
   {
      _delegates.removeAllSwap( v );
   }

   bool empty()
   {
      return _delegates.empty();
   }

   size_t size()
   {
      return _delegates.size();
   }

   Iterator       begin()       { return _delegates.begin(); }
   Iterator       end()         { return _delegates.end();   }

   ConstIterator  begin() const { return _delegates.begin(); }
   ConstIterator  end()   const { return _delegates.end();   }

   void exec( P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6 ) const
   {
      typename DelegateContainer::ConstIterator it  = _delegates.begin();
      typename DelegateContainer::ConstIterator end = _delegates.end();

      for ( ; it != end; ++it )
      {
         (*it)( p1, p2, p3, p4, p5, p6 );
      }
   }

private:

   /*----- data members -----*/

   DelegateContainer  _delegates;
}; //class Delegate6List


/*==============================================================================
  CLASS Delegate7List
==============================================================================*/
//! Delegate list for functions with seven parameters.
template< typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename RT=void >
class Delegate7List
{
public:

   /*----- types -----*/
   typedef typename Delegate7<P1, P2, P3, P4, P5, P6, P7, RT>::type  DelegateType;
   typedef Vector< DelegateType >                                    DelegateContainer;
   typedef typename DelegateContainer::Iterator                      Iterator;
   typedef typename DelegateContainer::ConstIterator                 ConstIterator;

   /*----- methods -----*/

   Delegate7List() {}

   void addDelegate( const DelegateType& delegate )
   {
      _delegates.pushBack( delegate );
   }

   void removeDelegate( const DelegateType& delegate )
   {
      _delegates.remove( delegate );
   }

   void clear()
   {
      _delegates.clear();
   }

   void clearEmpty( const DelegateType& v = DelegateType() )
   {
      _delegates.removeAllSwap( v );
   }

   bool empty()
   {
      return _delegates.empty();
   }

   size_t size()
   {
      return _delegates.size();
   }

   Iterator       begin()       { return _delegates.begin(); }
   Iterator       end()         { return _delegates.end();   }

   ConstIterator  begin() const { return _delegates.begin(); }
   ConstIterator  end()   const { return _delegates.end();   }

   void exec( P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7 ) const
   {
      typename DelegateContainer::ConstIterator it  = _delegates.begin();
      typename DelegateContainer::ConstIterator end = _delegates.end();

      for ( ; it != end; ++it )
      {
         (*it)( p1, p2, p3, p4, p5, p6, p7 );
      }
   }

private:

   /*----- data members -----*/

   DelegateContainer  _delegates;
}; //class Delegate7List


/*==============================================================================
  CLASS Delegate8List
==============================================================================*/
//! Delegate list for functions with eight parameters.
template< typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename RT=void >
class Delegate8List
{
public:

   /*----- types -----*/
   typedef typename Delegate8<P1, P2, P3, P4, P5, P6, P7, P8, RT>::type  DelegateType;
   typedef Vector< DelegateType >                                        DelegateContainer;
   typedef typename DelegateContainer::Iterator                          Iterator;
   typedef typename DelegateContainer::ConstIterator                     ConstIterator;

   /*----- methods -----*/

   Delegate8List() {}

   void addDelegate( const DelegateType& delegate )
   {
      _delegates.pushBack( delegate );
   }

   void removeDelegate( const DelegateType& delegate )
   {
      _delegates.remove( delegate );
   }

   void clear()
   {
      _delegates.clear();
   }

   void clearEmpty( const DelegateType& v = DelegateType() )
   {
      _delegates.removeAllSwap( v );
   }

   bool empty()
   {
      return _delegates.empty();
   }

   size_t size()
   {
      return _delegates.size();
   }

   Iterator       begin()       { return _delegates.begin(); }
   Iterator       end()         { return _delegates.end();   }

   ConstIterator  begin() const { return _delegates.begin(); }
   ConstIterator  end()   const { return _delegates.end();   }

   void exec( P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8 ) const
   {
      typename DelegateContainer::ConstIterator it  = _delegates.begin();
      typename DelegateContainer::ConstIterator end = _delegates.end();

      for ( ; it != end; ++it )
      {
         (*it)( p1, p2, p3, p4, p5, p6, p7, p8 );
      }
   }

private:

   /*----- data members -----*/

   DelegateContainer  _delegates;
}; //class Delegate8List


NAMESPACE_END

#endif //BASE_DELEGATE_LIST_H
