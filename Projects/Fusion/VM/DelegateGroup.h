/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef FUSION_DELEGATE_GROUP_H
#define FUSION_DELEGATE_GROUP_H

#include <Fusion/StdDefs.h>

#include <Fusion/VM/VM.h>

#include <Base/ADT/Set.h>
#include <Base/Msg/DelegateList.h>

NAMESPACE_BEGIN


/*==============================================================================
  CLASS Delegate0Group
==============================================================================*/

template< typename RT=void >
class Delegate0Group
{
public:

   /*----- types -----*/

   typedef Delegate0List<RT>            List;
   typedef typename List::DelegateType  Func;
   typedef void (*RefExec)( const VMRef& );

   /*----- methods -----*/

   inline void  add   ( const Func& f ) { _dels.addDelegate(f);    }
   inline void  remove( const Func& f ) { _dels.removeDelegate(f); }

   inline void  add   ( const VMRef& r ) { _refs.add(r);    }
   inline void  remove( const VMRef& r ) { _refs.remove(r); }

   inline void  clear() { _dels.clear(); _refs.clear(); }

   inline void  exec( RefExec refExec )
   {
      _dels.exec();
      Set<VMRef> refsCopy( _refs ); // Make a copy in case the callback removes something.
      for( auto cur = refsCopy.begin(); cur != refsCopy.end(); ++cur )
      {
         refExec( *cur );
      }
   }

   inline size_t  numDels() const { return _dels.size(); }
   inline size_t  numRefs() const { return _refs.size(); }

protected:

   /*----- data members -----*/

   List        _dels;
   Set<VMRef>  _refs;

private:
}; //class Delegate0Group


/*==============================================================================
  CLASS Delegate1Group
==============================================================================*/
template< typename P1, typename RT=void >
class Delegate1Group
{
public:

   /*----- types -----*/

   typedef Delegate1List<P1, RT>        List;
   typedef typename List::DelegateType  Func;
   typedef void (*RefExec)( const VMRef&, P1 p1 );

   /*----- methods -----*/

   inline void  add   ( const Func& f ) { _dels.addDelegate(f);    }
   inline void  remove( const Func& f ) { _dels.removeDelegate(f); }

   inline void  add   ( const VMRef& r ) { _refs.add(r);    }
   inline void  remove( const VMRef& r ) { _refs.remove(r); }

   inline void  clear() { _dels.clear(); _refs.clear(); }

   inline void  exec( P1 p1, RefExec refExec )
   {
      _dels.exec( p1 );
      Set<VMRef> refsCopy( _refs ); // Make a copy in case the callback removes something.
      for( auto cur = refsCopy.begin(); cur != refsCopy.end(); ++cur )
      {
         refExec( *cur, p1 );
      }
   }

   inline size_t  numDels() const { return _dels.size(); }
   inline size_t  numRefs() const { return _refs.size(); }

protected:

   /*----- data members -----*/

   List        _dels;
   Set<VMRef>  _refs;

private:
}; //class Delegate1Group


/*==============================================================================
  CLASS Delegate2Group
==============================================================================*/
template< typename P1, typename P2, typename RT=void >
class Delegate2Group
{
public:

   /*----- types -----*/
   typedef Delegate2List<P1, P2, RT>    List;
   typedef typename List::DelegateType  Func;
   typedef void (*RefExec)( const VMRef&, P1 p1, P2 p2 );

   /*----- methods -----*/

   inline void  add   ( const Func& f ) { _dels.addDelegate(f);    }
   inline void  remove( const Func& f ) { _dels.removeDelegate(f); }

   inline void  add   ( const VMRef& r ) { _refs.add(r);    }
   inline void  remove( const VMRef& r ) { _refs.remove(r); }

   inline void  clear() { _dels.clear(); _refs.clear(); }

   inline void  exec( P1 p1, P2 p2, RefExec refExec )
   {
      _dels.exec( p1, p2 );
      Set<VMRef> refsCopy( _refs ); // Make a copy in case the callback removes something.
      for( auto cur = refsCopy.begin(); cur != refsCopy.end(); ++cur )
      {
         refExec( *cur, p1, p2 );
      }
   }

   inline size_t  numDels() const { return _dels.size(); }
   inline size_t  numRefs() const { return _refs.size(); }

protected:

   /*----- data members -----*/

   List        _dels;
   Set<VMRef>  _refs;

private:
}; //class Delegate2Group


/*==============================================================================
  CLASS Delegate3Group
==============================================================================*/
template< typename P1, typename P2, typename P3, typename RT=void >
class Delegate3Group
{
public:

   /*----- types -----*/
   typedef Delegate3List<P1, P2, P3, RT>  List;
   typedef typename List::DelegateType    Func;
   typedef void (*RefExec)( const VMRef&, P1 p1, P2 p2, P3 p3 );

   /*----- methods -----*/

   inline void  add   ( const Func& f ) { _dels.addDelegate(f);    }
   inline void  remove( const Func& f ) { _dels.removeDelegate(f); }

   inline void  add   ( const VMRef& r ) { _refs.add(r);    }
   inline void  remove( const VMRef& r ) { _refs.remove(r); }

   inline void  clear() { _dels.clear(); _refs.clear(); }

   inline void  exec( P1 p1, P2 p2, P3 p3, RefExec refExec )
   {
      _dels.exec( p1, p2, p3 );
      Set<VMRef> refsCopy( _refs ); // Make a copy in case the callback removes something.
      for( auto cur = refsCopy.begin(); cur != refsCopy.end(); ++cur )
      {
         refExec( *cur, p1, p2, p3 );
      }
   }

   inline size_t  numDels() const { return _dels.size(); }
   inline size_t  numRefs() const { return _refs.size(); }

protected:

   /*----- data members -----*/

   List        _dels;
   Set<VMRef>  _refs;

private:
}; //class Delegate3Group


/*==============================================================================
  CLASS Delegate4Group
==============================================================================*/
template< typename P1, typename P2, typename P3, typename P4, typename RT=void >
class Delegate4Group
{
public:

   /*----- types -----*/
   typedef Delegate4List<P1, P2, P3, P4, RT>  List;
   typedef typename List::DelegateType        Func;
   typedef void (*RefExec)( const VMRef&, P1 p1, P2 p2, P3 p3, P4 p4 );

   /*----- methods -----*/

   inline void  add   ( const Func& f ) { _dels.addDelegate(f);    }
   inline void  remove( const Func& f ) { _dels.removeDelegate(f); }

   inline void  add   ( const VMRef& r ) { _refs.add(r);    }
   inline void  remove( const VMRef& r ) { _refs.remove(r); }

   inline void  clear() { _dels.clear(); _refs.clear(); }

   inline void  exec( P1 p1, P2 p2, P3 p3, P4 p4, RefExec refExec )
   {
      _dels.exec( p1, p2, p3, p4 );
      Set<VMRef> refsCopy( _refs ); // Make a copy in case the callback removes something.
      for( auto cur = refsCopy.begin(); cur != refsCopy.end(); ++cur )
      {
         refExec( *cur, p1, p2, p3, p4 );
      }
   }

   inline size_t  numDels() const { return _dels.size(); }
   inline size_t  numRefs() const { return _refs.size(); }

protected:

   /*----- data members -----*/

   List        _dels;
   Set<VMRef>  _refs;

private:
}; //class Delegate4Group


NAMESPACE_END

#endif //FUSION_DELEGATE_GROUP_H
