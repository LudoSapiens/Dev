/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_DFNODEATTR_H
#define PLASMA_DFNODEATTR_H

#include <Plasma/StdDefs.h>

#include <CGMath/Variant.h>

#include <Base/ADT/Vector.h>
#include <Base/Util/RCObject.h>
#include <Base/Util/RCP.h>

NAMESPACE_BEGIN

class DFNodeAttrList;

/*==============================================================================
  CLASS DFNodeAttr
==============================================================================*/
class DFNodeAttr
{
public:

   /*----- static methods -----*/

   static void  initialize();
   static void  terminate();

   /*----- static variable -----*/

   static const DFNodeAttr&  null() { return _null; }

   enum {
      INVALID_ID = uint(-1),
   };


   /*----- methods -----*/

   inline DFNodeAttr() {}

   inline DFNodeAttr( uint id ):
      _id( id ), _pid( INVALID_ID ), _sid( INVALID_ID ) {}

   inline DFNodeAttr(
      const ConstString& type,
      uint               id,
      const ConstString& label
   ):
      _id( id ), _pid( INVALID_ID ), _sid( INVALID_ID ),
      _type( type ), _label( label ) {}

   inline DFNodeAttr(
      uint               id,
      const ConstString& label,
      DFNodeAttrList*    attrs
   ):
      _id( id ), _pid(INVALID_ID), _sid(INVALID_ID),
      _type( "GROUP" ), _label( label ),
      _attributes( attrs ) {}

   DFNodeAttr&  id ( uint v )      { _id  = v; return *this; }
   DFNodeAttr&  pid( uint v )      { _pid = v; return *this; }
   DFNodeAttr&  sid( uint v )      { _sid = v; return *this; }
   DFNodeAttr&  extras( Table* v ) { _extras = v; return *this; }

   inline bool  isNull() const { return this == &_null; }

   uint id() const                           { return _id; }
   uint pid() const                          { return _pid; }
   uint sid() const                          { return _sid; }
   const ConstString&  label() const         { return _label; }
   const ConstString&  type() const          { return _type; }
   const DFNodeAttrList*  attributes() const { return _attributes.ptr(); }
         Table*  extras()                    { return _extras.ptr(); }
   const Table*  extras() const              { return _extras.ptr(); }

   inline         DFNodeAttr&  enums( Table* v ) { CHECK(_extras.isNull()); _extras = v; return *this; }

   PLASMA_DLL_API DFNodeAttr&  compact( bool v = true );
   PLASMA_DLL_API DFNodeAttr&  range( float min, float max );
   PLASMA_DLL_API DFNodeAttr&  step( float v );
   PLASMA_DLL_API DFNodeAttr&  length( float v );

protected:

   /*----- mthods -----*/

   inline Table&  e() { if( _extras.isNull() ) _extras = new Table(); return *_extras; }

   /*----- static members -----*/

   static PLASMA_DLL_API const DFNodeAttr  _null;

   /*----- data members -----*/

   uint                _id;
   uint                _pid;
   uint                _sid;
   ConstString         _type;
   ConstString         _label;
   RCP<Table>          _extras;
   RCP<DFNodeAttrList> _attributes;
};


/*==============================================================================
  CLASS DFNodeAttrList
==============================================================================*/
class DFNodeAttrList:
   public RCObject
{
public:

   /*----- types -----*/

   typedef Vector<DFNodeAttr>              AttribContainer;
   typedef AttribContainer::ConstIterator  ConstIterator;

   /*----- methods -----*/

   inline uint  numAttributes() const { return uint(_attr.size()); }

   inline         const DFNodeAttr&  attribute( uint idx ) const { return (idx < numAttributes()) ? _attr[idx] : DFNodeAttr::null(); }

   inline ConstIterator  begin() const        { return _attr.begin(); }
   inline ConstIterator  end()   const        { return _attr.end();   }

   inline void  add( const DFNodeAttr& attr ) { _attr.pushBack(attr); }
   inline DFNodeAttr&  last() { return _attr.back(); }

protected:

   /*----- data members -----*/

   AttribContainer  _attr;
};

/*==============================================================================
   STRUCT DFNodeAttrState
==============================================================================*/
struct DFNodeAttrState
{
   DFNodeAttrState( uint id, bool v ):               _id( id ), _value( v ) {}
   DFNodeAttrState( uint id, float v ):              _id( id ), _value( v ) {}
   DFNodeAttrState( uint id, const char* v ):        _id( id ), _value( v ) {}
   DFNodeAttrState( uint id, const Vec2f& v ):       _id( id ), _value( v ) {}
   DFNodeAttrState( uint id, const Vec3f& v ):       _id( id ), _value( v ) {}
   DFNodeAttrState( uint id, const Vec4f& v ):       _id( id ), _value( v ) {}
   DFNodeAttrState( uint id, const Quatf& v ):       _id( id ), _value( v ) {}
   DFNodeAttrState( uint id, const Variant& v ):     _id( id ), _value( v ) {}
   DFNodeAttrState( uint id, const ConstString& v ): _id( id ), _value( v ) {}

   uint    _id;
   Variant _value;
};

/*==============================================================================
   CLASS DFNodeAttrStates
==============================================================================*/
class DFNodeAttrStates:
   public RCObject
{
public:

   /*----- types -----*/

   typedef Vector<DFNodeAttrState> StateContainer;

   inline StateContainer::ConstIterator  begin() const { return _states.begin(); }
   inline StateContainer::ConstIterator  end()   const { return _states.end();   }

   void set( uint id, bool v )               { _states.pushBack( DFNodeAttrState( id, v ) ); }
   void set( uint id, float v )              { _states.pushBack( DFNodeAttrState( id, v ) ); }
   void set( uint id, const char* v )        { _states.pushBack( DFNodeAttrState( id, v ) ); }
   void set( uint id, const Vec2f& v )       { _states.pushBack( DFNodeAttrState( id, v ) ); }
   void set( uint id, const Vec3f& v )       { _states.pushBack( DFNodeAttrState( id, v ) ); }
   void set( uint id, const Vec4f& v )       { _states.pushBack( DFNodeAttrState( id, v ) ); }
   void set( uint id, const Quatf& v )       { _states.pushBack( DFNodeAttrState( id, v ) ); }
   void set( uint id, const Variant& v )     { _states.pushBack( DFNodeAttrState( id, v ) ); }
   void set( uint id, const ConstString& v ) { _states.pushBack( DFNodeAttrState( id, v ) ); }

protected:

   /*----- data members -----*/

   StateContainer _states;
};

NAMESPACE_END

#endif //PLASMA_DFNODEATTR_H
