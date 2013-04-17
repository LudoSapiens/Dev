/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef CGMATH_VARIANT_H
#define CGMATH_VARIANT_H

#include <CGMath/StdDefs.h>

#include <CGMath/Vec2.h>
#include <CGMath/Vec3.h>
#include <CGMath/Vec4.h>
#include <CGMath/Quat.h>

#include <Base/ADT/Map.h>
#include <Base/ADT/ConstString.h>
#include <Base/Dbg/Defs.h>
#include <Base/IO/StreamIndent.h>
#include <Base/IO/TextStream.h>

NAMESPACE_BEGIN

class Table;

/*==============================================================================
   CLASS Variant
==============================================================================*/

class Variant
{
public:

   /*----- types and enumerations -----*/

   enum Type {
      NIL,
      BOOL,
      FLOAT,
      VEC2,
      VEC3,
      VEC4,
      QUAT,
      STRING,
      POINTER,
      TABLE
   };

   /*----- static methods -----*/

    static const Variant& null()            { return _null; }

   /*----- methods -----*/

   Variant() : _type(NIL) {}
   explicit Variant( bool v )               { initialize(v); }
   explicit Variant( float v )              { initialize(v); }
   explicit Variant( const Vec2f& v )       { initialize(v); }
   explicit Variant( const Vec3f& v )       { initialize(v); }
   explicit Variant( const Vec4f& v )       { initialize(v); }
   explicit Variant( const Quatf& v )       { initialize(v); }
   explicit Variant( const ConstString& v ) { initialize(v); }
   explicit Variant( const char* v )        { initialize(ConstString(v)); }
   explicit Variant( void* v )              { initialize(v); }
   explicit Variant( Table* v )             { initialize(v); }
   Variant( const Variant& v )              { initialize(v); }

   ~Variant()                               { finalize(); }

   // Accessor.
   bool getBoolean() const                 { return value._b; }
   float getFloat() const                  { return value._f; }
   const Vec2f& getVec2() const            { return *((Vec2f*)(&value._v[0])); }
   const Vec3f& getVec3() const            { return *((Vec3f*)(&value._v[0])); }
   const Vec4f& getVec4() const            { return *value._v4; }
   const Quatf& getQuat() const            { return *value._q;  }
   const ConstString getString() const     { return ConstString( value._s ); }
   void* getPointer() const                { return value._p; }
   const Table* getTable() const           { return value._t; }
   Table* getTable()                       { return value._t; }

   // Types.
   int type() const                        { return _type; }
   bool isNil() const                      { return _type == NIL; }
   bool isBoolean() const                  { return _type == BOOL; }
   bool isFloat() const                    { return _type == FLOAT; }
   bool isVec2() const                     { return _type == VEC2; }
   bool isVec3() const                     { return _type == VEC3; }
   bool isVec4() const                     { return _type == VEC4; }
   bool isQuat() const                     { return _type == QUAT; }
   bool isString() const                   { return _type == STRING; }
   bool isPointer() const                  { return _type == POINTER; }
   bool isTable() const                    { return _type == TABLE; }

   // Assignment operators.
   Variant& operator=( bool v )            { finalize(); initialize(v); return *this; }
   Variant& operator=( float v )           { finalize(); initialize(v); return *this; }
   Variant& operator=( const Vec2f& v )    { finalize(); initialize(v); return *this; }
   Variant& operator=( const Vec3f& v )    { finalize(); initialize(v); return *this; }
   Variant& operator=( const Vec4f& v )
   {
      if( _type == VEC4 ) *value._v4 = v;
      else
      {
         finalize();
         initialize(v);
      }
      return *this;
   }
   Variant& operator=( const Quatf& v )
   {
      if( _type == QUAT ) *value._q = v;
      else
      {
         finalize();
         initialize(v);
      }
      return *this;
   }
   Variant& operator=( const ConstString& v ) { finalize(); initialize(v); return *this; }
   Variant& operator=( const char* v )        { finalize(); initialize(ConstString(v)); return *this; }
   Variant& operator=( void* v )              { finalize(); initialize(v); return *this; }
   Variant& operator=( Table* v )             { finalize(); initialize(v); return *this; }
   Variant& operator=( const Variant& v )     { finalize(); initialize(v); return *this; }

   CGMATH_DLL_API void print( TextStream& os = StdErr ) const;
   CGMATH_DLL_API void print( StreamIndent& indent, TextStream& os = StdErr ) const;

protected:

   /*----- methods -----*/

   void initialize( bool v )               { _type = BOOL;    value._b = v; }
   void initialize( float v )              { _type = FLOAT;   value._f = v; }
   void initialize( const Vec2f& v )       { _type = VEC2;    value._v[0] = v.x; value._v[1] = v.y; }
   void initialize( const Vec3f& v )       { _type = VEC3;    value._v[0] = v.x; value._v[1] = v.y; value._v[2] = v.z; }
   void initialize( const Vec4f& v )       { _type = VEC4;    value._v4 = new Vec4f(v); }
   void initialize( const Quatf& v )       { _type = QUAT;    value._q  = new Quatf(v); }
   void initialize( void* v )              { _type = POINTER; value._p = v; }
   void initialize( const Table* v );
   void initialize( const ConstString& v ) { initialize( v.rcstring() ); }
   void initialize( RCString* v )
   {
      _type = STRING;
      value._s = v;
      value._s->addReference();
   }
   void initialize( const Variant& v )
   {
      switch( v._type )
      {
         case NIL:     _type = NIL;                   break;
         case BOOL:    initialize( v.getBoolean() );  break;
         case FLOAT:   initialize( v.getFloat()   );  break;
         case VEC2:    initialize( v.getVec2()    );  break;
         case VEC3:    initialize( v.getVec3()    );  break;
         case VEC4:    initialize( v.getVec4()    );  break;
         case QUAT:    initialize( v.getQuat()    );  break;
         case STRING:  initialize( v.value._s     );  break;
         case POINTER: initialize( v.getPointer() );  break;
         case TABLE:   initialize( v.getTable()   );  break;
      }
      CHECK( _type == v._type );
   }

   inline void finalize();

   /*----- static members -----*/

   CGMATH_DLL_API static Variant _null;

   /*----- members -----*/

   int _type;

   union {
      bool      _b;
      float     _f;
      float     _v[3];
      Vec4f*    _v4;
      Quatf*    _q;
      RCString* _s;
      Table*    _t;
      void*     _p;
   } value;
};

//------------------------------------------------------------------------------
//!
inline TextStream&
operator<<( TextStream& os, const Variant& v )
{
   v.print( os );
   return os;
}

/*==============================================================================
   CLASS Table
==============================================================================*/

class Table:
   public RCObject
{
public:

   /*----- types and enumerations ----*/

   typedef Map<ConstString,Variant>    MapContainer;
   typedef MapContainer::ConstIterator ConstIterator;
   typedef MapContainer::Iterator      Iterator;
   typedef Vector<Variant>             ArrayContainer;

   /*----- static methods -----*/

   static const Table& null()  { return *_null; }

   /*----- methods -----*/

   Table() {}

   // Operations.
   void clear();
   void remove( const ConstString& key );
   void remove( size_t key );

   // String keys.
   void set( const ConstString& key, bool );
   void set( const ConstString& key, float );
   void set( const ConstString& key, const Vec2f& );
   void set( const ConstString& key, const Vec3f& );
   void set( const ConstString& key, const Vec4f& );
   void set( const ConstString& key, const Quatf& );
   void set( const ConstString& key, const ConstString& );
   void set( const ConstString& key, const char* );
   void set( const ConstString& key, void* );
   void set( const ConstString& key, const Variant& );
   void set( const ConstString& key, Table* );
         Variant& get( const ConstString& key );
   const Variant& get( const ConstString& key ) const;
   bool has( const ConstString& key ) const;

   // Integer keys.
   void set( size_t key, bool );
   void set( size_t key, float );
   void set( size_t key, const Vec2f& );
   void set( size_t key, const Vec3f& );
   void set( size_t key, const Vec4f& );
   void set( size_t key, const Quatf& );
   void set( size_t key, const ConstString& );
   void set( size_t key, const char* );
   void set( size_t key, void* );
   void set( size_t key, const Variant& );
   void set( size_t key, Table* );
         Variant& get( size_t key );
   const Variant& get( size_t key ) const;
   bool has( size_t key ) const;

   void pushBack( bool );
   void pushBack( float );
   void pushBack( const Vec2f& );
   void pushBack( const Vec3f& );
   void pushBack( const Vec4f& );
   void pushBack( const Quatf& );
   void pushBack( const ConstString& );
   void pushBack( const char* );
   void pushBack( void* );
   void pushBack( const Variant& );
   void pushBack( Table* );

   CGMATH_DLL_API void extend( const Table& );

   // Accessor.
   bool empty() const          { return _map.empty() && _array.empty(); }
   size_t size() const         { return _map.size()+_array.size(); }
   size_t mapSize() const      { return _map.size(); }
   size_t arraySize() const    { return _array.size(); }
   ConstIterator begin() const { return _map.begin(); }
   ConstIterator end() const   { return _map.end(); }
   Iterator begin()            { return _map.begin(); }
   Iterator end()              { return _map.end(); }


   // Operator.
         Variant& operator[]( const char* key )               { return get( ConstString(key) ); }
   const Variant& operator[]( const char* key ) const         { return get( ConstString(key) ); }
         Variant& operator[]( const ConstString& key )        { return get( key ); }
   const Variant& operator[]( const ConstString& key ) const  { return get( key ); }
         Variant& operator[]( size_t key )                    { return get( key ); }
   const Variant& operator[]( size_t key ) const              { return get( key ); }

   CGMATH_DLL_API void print( TextStream& os = StdErr ) const;
   CGMATH_DLL_API void print( StreamIndent& indent, TextStream& os = StdErr ) const;

protected:

   /*----- methods -----*/

   virtual ~Table(){}

   /*----- static members -----*/

   CGMATH_DLL_API static RCP<Table>  _null;

   /*----- members -----*/

   MapContainer   _map;
   ArrayContainer _array;
};

//------------------------------------------------------------------------------
//!
inline void
Variant::initialize( const Table* v )
{
   _type = TABLE;
   value._t = const_cast<Table*>( v );
   value._t->addReference();
}

//------------------------------------------------------------------------------
//!
inline void
Variant::finalize()
{
   switch( _type )
   {
      case NIL:
      case BOOL:
      case FLOAT:
      case VEC2:
      case VEC3:
         break;
      case VEC4:
         delete value._v4;
         break;
      case QUAT:
         delete value._q;
         break;
      case STRING:
         value._s->removeReference();
         break;
      case POINTER:
         break;
      case TABLE:
         value._t->removeReference();
         break;
   }
}

//------------------------------------------------------------------------------
//!
inline void
Table::clear()
{
   _map.clear();
   _array.clear();
}

//------------------------------------------------------------------------------
//!
inline void
Table::remove( const ConstString& key )
{
   _map.erase( key );
}

//------------------------------------------------------------------------------
//!
inline void
Table::set( const ConstString& key, bool v )
{
   _map[key] = v;
}

//------------------------------------------------------------------------------
//!
inline void
Table::set( const ConstString& key, float v )
{
   _map[key] = v;
}

//------------------------------------------------------------------------------
//!
inline void
Table::set( const ConstString& key, const Vec2f& v )
{
   _map[key] = v;
}

//------------------------------------------------------------------------------
//!
inline void
Table::set( const ConstString& key, const Vec3f& v )
{
   _map[key] = v;
}

//------------------------------------------------------------------------------
//!
inline void
Table::set( const ConstString& key, const Vec4f& v )
{
   _map[key] = v;
}

//------------------------------------------------------------------------------
//!
inline void
Table::set( const ConstString& key, const Quatf& v )
{
   _map[key] = v;
}

//------------------------------------------------------------------------------
//!
inline void
Table::set( const ConstString& key, const ConstString& v )
{
   _map[key] = v;
}


//------------------------------------------------------------------------------
//!
inline void
Table::set( const ConstString& key, const char* v )
{
   _map[key] = v;
}

//------------------------------------------------------------------------------
//!
inline void
Table::set( const ConstString& key, void* v )
{
   _map[key] = v;
}

//------------------------------------------------------------------------------
//!
inline void
Table::set( const ConstString& key, const Variant& v )
{
   _map[key] = v;
}

//------------------------------------------------------------------------------
//!
inline void
Table::set( const ConstString& key, Table* v )
{
   _map[key] = v;
}

//------------------------------------------------------------------------------
//!
inline Variant&
Table::get( const ConstString& key )
{
   return _map[key];
}

//------------------------------------------------------------------------------
//!
inline const Variant&
Table::get( const ConstString& key ) const
{
   ConstIterator iter = _map.find( key );
   if( iter != _map.end() ) return (*iter).second;
   return Variant::null();
}

//------------------------------------------------------------------------------
//!
inline bool
Table::has( const ConstString& key ) const
{
   ConstIterator iter = _map.find( key );
   if( iter != _map.end() ) return true;
   return false;
}

//------------------------------------------------------------------------------
//!
inline void
Table::remove( size_t key )
{
   _array.erase( _array.begin() + key );
}

//------------------------------------------------------------------------------
//!
inline void
Table::set( size_t key, bool v )
{
   _array.grow(key+1);
   _array[key] = v;
}

//------------------------------------------------------------------------------
//!
inline void
Table::set( size_t key, float v )
{
   _array.grow(key+1);
   _array[key] = v;
}

//------------------------------------------------------------------------------
//!
inline void
Table::set( size_t key, const Vec2f& v )
{
   _array.grow(key+1);
   _array[key] = v;
}

//------------------------------------------------------------------------------
//!
inline void
Table::set( size_t key, const Vec3f& v )
{
   _array.grow(key+1);
   _array[key] = v;
}

//------------------------------------------------------------------------------
//!
inline void
Table::set( size_t key, const Vec4f& v )
{
   _array.grow(key+1);
   _array[key] = v;
}

//------------------------------------------------------------------------------
//!
inline void
Table::set( size_t key, const Quatf& v )
{
   _array.grow(key+1);
   _array[key] = v;
}

//------------------------------------------------------------------------------
//!
inline void
Table::set( size_t key, const ConstString& v )
{
   _array.grow(key+1);
   _array[key] = v;
}

//------------------------------------------------------------------------------
//!
inline void
Table::set( size_t key, const char* v )
{
   _array.grow(key+1);
   _array[key] = v;
}

//------------------------------------------------------------------------------
//!
inline void
Table::set( size_t key, void* v )
{
   _array.grow(key+1);
   _array[key] = v;
}

//------------------------------------------------------------------------------
//!
inline void
Table::set( size_t key, const Variant& v )
{
   _array.grow(key+1);
   _array[key] = v;
}

//------------------------------------------------------------------------------
//!
inline void
Table::set( size_t key, Table* v )
{
   _array.grow(key+1);
   _array[key] = v;
}

//------------------------------------------------------------------------------
//!
inline Variant&
Table::get( size_t key )
{
   _array.grow(key+1);
   return _array[key];
}

//------------------------------------------------------------------------------
//!
inline const Variant&
Table::get( size_t key ) const
{
   if( key >= _array.size() ) return Variant::null();
   return _array[key];
}

//------------------------------------------------------------------------------
//!
inline bool
Table::has( size_t key ) const
{
   if( key >= _array.size() ) return false;
   return true;
}

//------------------------------------------------------------------------------
//!
inline void
Table::pushBack( bool v )
{
   _array.pushBack( Variant(v) );
}

//------------------------------------------------------------------------------
//!
inline void
Table::pushBack( float v )
{
   _array.pushBack( Variant(v) );
}

//------------------------------------------------------------------------------
//!
inline void
Table::pushBack( const Vec2f& v )
{
   _array.pushBack( Variant(v) );
}

//------------------------------------------------------------------------------
//!
inline void
Table::pushBack( const Vec3f& v )
{
   _array.pushBack( Variant(v) );
}

//------------------------------------------------------------------------------
//!
inline void
Table::pushBack( const Vec4f& v )
{
   _array.pushBack( Variant(v) );
}

//------------------------------------------------------------------------------
//!
inline void
Table::pushBack( const Quatf& v )
{
   _array.pushBack( Variant(v) );
}

//------------------------------------------------------------------------------
//!
inline void
Table::pushBack( const ConstString& v )
{
   _array.pushBack( Variant(v) );
}

//------------------------------------------------------------------------------
//!
inline void
Table::pushBack( const char* v )
{
   _array.pushBack( Variant(v) );
}

//------------------------------------------------------------------------------
//!
inline void
Table::pushBack( void* v )
{
   _array.pushBack( Variant(v) );
}

//------------------------------------------------------------------------------
//!
inline void
Table::pushBack( const Variant& v )
{
   _array.pushBack( Variant(v) );
}

//------------------------------------------------------------------------------
//!
inline void
Table::pushBack( Table* v )
{
   _array.pushBack( Variant(v) );
}

//------------------------------------------------------------------------------
//!
inline TextStream&
operator<<( TextStream& os, const Table& t )
{
   t.print( os );
   return os;
}

NAMESPACE_END

#endif
