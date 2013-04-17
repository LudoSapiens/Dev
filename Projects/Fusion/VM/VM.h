/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef FUSION_VM_H
#define FUSION_VM_H

#include <Fusion/StdDefs.h>

#include <CGMath/Vec2.h>
#include <CGMath/Vec3.h>
#include <CGMath/Vec4.h>
#include <CGMath/Mat2.h>
#include <CGMath/Mat3.h>
#include <CGMath/Mat4.h>
#include <CGMath/Quat.h>
#include <CGMath/Ref.h>
#include <CGMath/Ray.h>
#include <CGMath/AABBox.h>

#include <Base/ADT/HashTable.h>
#include <Base/IO/Path.h>
#include <Base/Util/RCP.h>

struct lua_State;

NAMESPACE_BEGIN

class Variant;
class Table;
class RCObject;
class Widget;
class Event;
typedef struct lua_State VMState;
typedef Vector<uchar>    VMByteCode;

/*==============================================================================
   CLASS VMAddress
==============================================================================*/

class VMAddress:
   public RCObject
{
public:
   typedef HashTable< void*, int >  CountTable;

   VMState*    _vm;
   CountTable  _counter;

   void  increment( void* ptr, int idx );
   void  increment( void* ptr );
   void  decrement( void* ptr );
};

/*==============================================================================
  CLASS VMRef
==============================================================================*/

class VMRef
{
public:

   /*----- methods -----*/

   inline VMRef(): _ptr( nullptr ) {}
   inline VMRef( const VMRef& ref ): _ptr( nullptr ) { *this = ref; }
   FUSION_DLL_API ~VMRef();
   FUSION_DLL_API VMRef& operator=( const VMRef& ref );

   FUSION_DLL_API void set( VMState* vm, int idx );
   FUSION_DLL_API VMState* vm() const;
   inline void*  ptr()     const { return _ptr; }
   inline bool   isValid() const { return _ptr != nullptr && vm() != nullptr; }

   inline bool  operator==( const VMRef& ref ) const { return _ptr == ref._ptr && _vma == ref._vma; }
   inline bool  operator< ( const VMRef& ref ) const
   {
      return _ptr < ref._ptr ||
             (_ptr == ref._ptr && _vma < ref._vma);
   }

private:

   //! Disallow copying for this class and its children.

   /*----- data members -----*/

   RCP<VMAddress> _vma;
   void*          _ptr;
};


/*==============================================================================
  CLASS VMProxy
==============================================================================*/
// Defines the interface which is required to push a proxy into a VM.
class VMProxy
{
public:

   virtual FUSION_DLL_API const char*  meta() const = 0;

}; //class VMProxy


/*==============================================================================
  CLASS VM
==============================================================================*/

//! Virtual Machine utility.


class VM
{
public:

   /*----- types and enumerations ----*/

   typedef int (*Function)( VMState* );

   enum {
      MULTRET = -1
   };

   enum {
      RC       = 1,
      CONSTANT = 2,
   };

   enum {
      NIL      = 0,
      BOOL     = 1,
      PTR      = 2,
      NUMBER   = 3,
      STRING   = 4,
      TABLE    = 5,
      FUNCTION = 6,
      OBJECT   = 7,
      THREAD   = 8
   };

   /*----- classes -----*/

   struct Reg {
      const char* name;
      Function    func;
   };

   struct EnumReg {
      const char* name;
      uint        id;
   };

   /*----- static members -----*/

   static FUSION_DLL_API Function  readsDisabledVM;
   static FUSION_DLL_API Function  writesDisabledVM;

   /*----- static methods -----*/

   static FUSION_DLL_API void printInfo( TextStream& os );

   inline static int upvalue( int idx ) { return _upidx - idx; }
   inline static int upvalue( void*, int idx ) { return upvalue(idx+1); }

   static FUSION_DLL_API VMState* open( uint mask = ~0, bool localsOnly = false );
   static FUSION_DLL_API VMState* open( uint mask, Function get, Function set );
   static FUSION_DLL_API void  close( VMState* );
   static FUSION_DLL_API void* userData( VMState* );
   static FUSION_DLL_API void  userData( VMState*, void* data );
   static FUSION_DLL_API int   loadFile( VMState*, const Path& fileName );
   static FUSION_DLL_API void  doFile( VMState*, const String& fileName, int nresults = 0 );
   static FUSION_DLL_API void  doFile( VMState*, const String& fileName, int nargs, int nresults );
   static FUSION_DLL_API int   doString( VMState*, const String& string );
   static FUSION_DLL_API void  setArguments( VMState*, const Vector<String>& arguments );

   static FUSION_DLL_API int   getByteCode( VMState*, VMByteCode& dst );
   static FUSION_DLL_API int   getByteCode( VMState*, const Path& fileName, VMByteCode& dst );
   static FUSION_DLL_API int   loadByteCode( VMState*, const VMByteCode& src, const char* chunkName = "ByteCode" );
   static FUSION_DLL_API int   doByteCode( VMState*, const VMByteCode& src, int nresults = 0, const char* chunkName = "ByteCode" );
   static FUSION_DLL_API int   doByteCode( VMState*, const VMByteCode& src, int nargs, int nresults, const char* chunkName = "ByteCode" );

   static FUSION_DLL_API void ecall( VMState*, int nargs, int nresults );

   static FUSION_DLL_API int getTop( VMState* );
   static FUSION_DLL_API void setTop( VMState*, int n );
   static FUSION_DLL_API int  absIndex( VMState*, int idx );
   static FUSION_DLL_API void pop( VMState*, int n = 1 );
   static FUSION_DLL_API void insert( VMState*, int idx );
   static FUSION_DLL_API void remove( VMState*, int idx );
   static FUSION_DLL_API void replace( VMState*, int idx );

   static FUSION_DLL_API void getGlobal( VMState*, const char* var );
   static FUSION_DLL_API void setGlobal( VMState*, const char* var );

   static FUSION_DLL_API void* thisPtr( VMState* );

   static FUSION_DLL_API int type( VMState*, int idx );
   static FUSION_DLL_API bool isBoolean( VMState*, int idx );
   static FUSION_DLL_API bool isFunction( VMState*, int idx );
   static FUSION_DLL_API bool isNil( VMState*, int idx );
   static FUSION_DLL_API bool isNumber( VMState*, int idx );
   static FUSION_DLL_API bool isObject( VMState*, int idx );
   static FUSION_DLL_API bool isString( VMState*, int idx );
   static FUSION_DLL_API bool isTable( VMState*, int idx );

   static FUSION_DLL_API void* unboxPointer( VMState*, int idx, int& flags );
   static FUSION_DLL_API void* unboxPointer( VMState*, int idx );

   static FUSION_DLL_API void* newObject( VMState*, uint size );

   static FUSION_DLL_API void pushProxy( VMState*, VMProxy* proxy );
   static FUSION_DLL_API VMProxy* toProxy( VMState*, int idx );
   static FUSION_DLL_API VMProxy* toProxy( VMState*, int idx, const char* meta );

   static FUSION_DLL_API void pushValue( VMState*, int idx );
   static FUSION_DLL_API void push( VMState* );
   static FUSION_DLL_API void push( VMState*, Function );
   static FUSION_DLL_API void push( VMState*, Function, int nbUpvalue );
   static FUSION_DLL_API void push( VMState*, void*, Function );
   static FUSION_DLL_API void push( VMState*, void*, Function, int nbUpvalue );
   static FUSION_DLL_API void push( VMState*, double );
   static FUSION_DLL_API void push( VMState*, int );
   static FUSION_DLL_API void push( VMState*, uint );
   static FUSION_DLL_API void push( VMState*, bool );
   static FUSION_DLL_API void push( VMState*, void* );
   static FUSION_DLL_API void push( VMState*, const char* );
   static FUSION_DLL_API void push( VMState*, const String& );
   static FUSION_DLL_API void push( VMState*, const ConstString& );
   static FUSION_DLL_API void push( VMState*, const Vec2i& );
   static FUSION_DLL_API void push( VMState*, const Vec2f& );
   static FUSION_DLL_API void push( VMState*, const Vec2d& );
   static FUSION_DLL_API void push( VMState*, const Vec3i& );
   static FUSION_DLL_API void push( VMState*, const Vec3f& );
   static FUSION_DLL_API void push( VMState*, const Vec3d& );
   static FUSION_DLL_API void push( VMState*, const Vec4i& );
   static FUSION_DLL_API void push( VMState*, const Vec4f& );
   static FUSION_DLL_API void push( VMState*, const Vec4d& );
   static FUSION_DLL_API void push( VMState*, const Quatf& );
   static FUSION_DLL_API void push( VMState*, const Quatd& );
   static FUSION_DLL_API void push( VMState*, const Mat2i& );
   static FUSION_DLL_API void push( VMState*, const Mat2f& );
   static FUSION_DLL_API void push( VMState*, const Mat2d& );
   static FUSION_DLL_API void push( VMState*, const Mat3i& );
   static FUSION_DLL_API void push( VMState*, const Mat3f& );
   static FUSION_DLL_API void push( VMState*, const Mat3d& );
   static FUSION_DLL_API void push( VMState*, const Mat4i& );
   static FUSION_DLL_API void push( VMState*, const Mat4f& );
   static FUSION_DLL_API void push( VMState*, const Mat4d& );
   static FUSION_DLL_API void push( VMState*, const Reff& );
   static FUSION_DLL_API void push( VMState*, const Refd& );
   static FUSION_DLL_API void push( VMState*, const Rayf& );
   static FUSION_DLL_API void push( VMState*, const Rayd& );
   static FUSION_DLL_API void push( VMState*, const AABBoxi& );
   static FUSION_DLL_API void push( VMState*, const AABBoxf& );
   static FUSION_DLL_API void push( VMState*, const AABBoxd& );
   static FUSION_DLL_API void push( VMState*, const VMRef& );
   static FUSION_DLL_API void push( VMState*, const Event& );
   static FUSION_DLL_API void push( VMState*, const Variant& );
   static FUSION_DLL_API void push( VMState*, const Table& );
   static FUSION_DLL_API void push( VMState*, int idx, const Table& );
   static FUSION_DLL_API void push( VMState*, const RCP<RCObject>&, const char* mtName );
   static FUSION_DLL_API void pushConst( VMState*, const RCP<const RCObject>&, const char* mtName );
   static FUSION_DLL_API void pushTable( VMState*, const char* name, const bool create = true );

   static FUSION_DLL_API void* toPtr( VMState*, int idx );
   static FUSION_DLL_API double toNumber( VMState*, int idx );
   static FUSION_DLL_API int toInt( VMState*, int idx );
   static FUSION_DLL_API uint toUInt( VMState*, int idx );
   static FUSION_DLL_API short toShort( VMState*, int idx );
   static FUSION_DLL_API ushort toUShort( VMState*, int idx );
   static inline float toFloat( VMState* vm, int idx ) { return (float)toNumber(vm, idx); }
   static inline double toDouble( VMState* vm, int idx ) { return toNumber(vm, idx); }
   static FUSION_DLL_API bool toBoolean( VMState*, int idx );
   static FUSION_DLL_API const char* toCString( VMState*, int idx );
   static FUSION_DLL_API ConstString toConstString( VMState*, int idx );
   static FUSION_DLL_API String toString( VMState*, int idx );
   static FUSION_DLL_API const char* toTypename( VMState*, int idx );
   static FUSION_DLL_API Path toPath( VMState*, int idx );
   static FUSION_DLL_API Vec2i toVec2i( VMState*, int idx );
   static FUSION_DLL_API Vec2f toVec2f( VMState*, int idx );
   static FUSION_DLL_API Vec2d toVec2d( VMState*, int idx );
   static FUSION_DLL_API Vec3i toVec3i( VMState*, int idx );
   static FUSION_DLL_API Vec3f toVec3f( VMState*, int idx );
   static FUSION_DLL_API Vec3d toVec3d( VMState*, int idx );
   static FUSION_DLL_API Vec4i toVec4i( VMState*, int idx );
   static FUSION_DLL_API Vec4f toVec4f( VMState*, int idx );
   static FUSION_DLL_API Vec4d toVec4d( VMState*, int idx );
   static FUSION_DLL_API Quatf toQuatf( VMState*, int idx );
   static FUSION_DLL_API Quatd toQuatd( VMState*, int idx );
   static FUSION_DLL_API Mat2i toMat2i( VMState*, int idx );
   static FUSION_DLL_API Mat2f toMat2f( VMState*, int idx );
   static FUSION_DLL_API Mat2d toMat2d( VMState*, int idx );
   static FUSION_DLL_API Mat3i toMat3i( VMState*, int idx );
   static FUSION_DLL_API Mat3f toMat3f( VMState*, int idx );
   static FUSION_DLL_API Mat3d toMat3d( VMState*, int idx );
   static FUSION_DLL_API Mat4i toMat4i( VMState*, int idx );
   static FUSION_DLL_API Mat4f toMat4f( VMState*, int idx );
   static FUSION_DLL_API Mat4d toMat4d( VMState*, int idx );
   static FUSION_DLL_API Reff  toReff( VMState*, int idx );
   static FUSION_DLL_API Refd  toRefd( VMState*, int idx );
   static FUSION_DLL_API Rayf  toRayf( VMState*, int idx );
   static FUSION_DLL_API Rayd  toRayd( VMState*, int idx );
   static FUSION_DLL_API AABBoxi  toAABBoxi( VMState*, int idx );
   static FUSION_DLL_API AABBoxf  toAABBoxf( VMState*, int idx );
   static FUSION_DLL_API AABBoxd  toAABBoxd( VMState*, int idx );
   static FUSION_DLL_API void toRef( VMState*, int idx, VMRef& ref );
   static FUSION_DLL_API void toTable( VMState*, int idx, Table& );
   static FUSION_DLL_API Variant toVariant( VMState*, int idx );
   static FUSION_DLL_API void toVariant( VMState*, Table& );

   static FUSION_DLL_API void newMetaTable( VMState*, const char* );
   static FUSION_DLL_API void getMetaTable( VMState*, const char* );
   static FUSION_DLL_API void setMetaTable( VMState*, int idx );
   static FUSION_DLL_API bool checkMetaTable( VMState*, int idx, const char* );

   static FUSION_DLL_API int getTableSize( VMState*, int idx );
   static FUSION_DLL_API void newTable( VMState* );
   static FUSION_DLL_API void getTable( VMState*, int idx );
   static FUSION_DLL_API void setTable( VMState*, int idx );
   static FUSION_DLL_API bool next( VMState*, int idx );
   static FUSION_DLL_API void set( VMState*, int idx );
   static FUSION_DLL_API void seti( VMState*, int idx, int n );
   static FUSION_DLL_API void get( VMState*, int idx );
   static FUSION_DLL_API bool get( VMState*, int idx, const char* key );
   static FUSION_DLL_API bool geti( VMState*, int idx, int n );

   //! Set an element with key in a table located at idx in the stack.
   static FUSION_DLL_API void set( VMState*, int idx, const char* key, int value );
   static FUSION_DLL_API void set( VMState*, int idx, const char* key, Function func );

   //! Get an element with key in a table located at idx in the stack.
   //@{
   static FUSION_DLL_API bool get( VMState*, int idx, const char* key, const char*& result );
   static FUSION_DLL_API bool get( VMState*, int idx, const char* key, Path& result );
   static FUSION_DLL_API bool get( VMState*, int idx, const char* key, String& result );
   static FUSION_DLL_API bool get( VMState*, int idx, const char* key, ConstString& result );
   static FUSION_DLL_API bool get( VMState*, int idx, const char* key, uint8_t& result );
   static FUSION_DLL_API bool get( VMState*, int idx, const char* key, int& result );
   static FUSION_DLL_API bool get( VMState*, int idx, const char* key, uint& result );
   static FUSION_DLL_API bool get( VMState*, int idx, const char* key, short& result );
   static FUSION_DLL_API bool get( VMState*, int idx, const char* key, ushort& result );
   static FUSION_DLL_API bool get( VMState*, int idx, const char* key, double& result );
   static FUSION_DLL_API bool get( VMState*, int idx, const char* key, float& result );
   static FUSION_DLL_API bool get( VMState*, int idx, const char* key, bool& result );
   static FUSION_DLL_API bool get( VMState*, int idx, const char* key, Vec2i& result );
   static FUSION_DLL_API bool get( VMState*, int idx, const char* key, Vec2f& result );
   static FUSION_DLL_API bool get( VMState*, int idx, const char* key, Vec2d& result );
   static FUSION_DLL_API bool get( VMState*, int idx, const char* key, Vec3i& result );
   static FUSION_DLL_API bool get( VMState*, int idx, const char* key, Vec3f& result );
   static FUSION_DLL_API bool get( VMState*, int idx, const char* key, Vec3d& result );
   static FUSION_DLL_API bool get( VMState*, int idx, const char* key, Vec4i& result );
   static FUSION_DLL_API bool get( VMState*, int idx, const char* key, Vec4f& result );
   static FUSION_DLL_API bool get( VMState*, int idx, const char* key, Vec4d& result );
   static FUSION_DLL_API bool get( VMState*, int idx, const char* key, Quatf& result );
   static FUSION_DLL_API bool get( VMState*, int idx, const char* key, Quatd& result );
   static FUSION_DLL_API bool get( VMState*, int idx, const char* key, Mat2i& result );
   static FUSION_DLL_API bool get( VMState*, int idx, const char* key, Mat2f& result );
   static FUSION_DLL_API bool get( VMState*, int idx, const char* key, Mat2d& result );
   static FUSION_DLL_API bool get( VMState*, int idx, const char* key, Mat3i& result );
   static FUSION_DLL_API bool get( VMState*, int idx, const char* key, Mat3f& result );
   static FUSION_DLL_API bool get( VMState*, int idx, const char* key, Mat3d& result );
   static FUSION_DLL_API bool get( VMState*, int idx, const char* key, Mat4i& result );
   static FUSION_DLL_API bool get( VMState*, int idx, const char* key, Mat4f& result );
   static FUSION_DLL_API bool get( VMState*, int idx, const char* key, Mat4d& result );
   static FUSION_DLL_API bool get( VMState*, int idx, const char* key, Reff& result );
   static FUSION_DLL_API bool get( VMState*, int idx, const char* key, Refd& result );
   static FUSION_DLL_API bool get( VMState*, int idx, const char* key, Rayf& result );
   static FUSION_DLL_API bool get( VMState*, int idx, const char* key, Rayd& result );
   static FUSION_DLL_API bool get( VMState*, int idx, const char* key, AABBoxi& result );
   static FUSION_DLL_API bool get( VMState*, int idx, const char* key, AABBoxf& result );
   static FUSION_DLL_API bool get( VMState*, int idx, const char* key, AABBoxd& result );
   static FUSION_DLL_API bool get( VMState*, int idx, const char* key, VMRef& result );
   static FUSION_DLL_API bool geti( VMState*, int idx, int i, RCP<RCObject>& result );
   static FUSION_DLL_API bool getiWidget( VMState*, int idx, int n, RCP<Widget>& result );
   static FUSION_DLL_API bool getWidget(
      VMState*,
      int          idx,
      const char*  key,
      RCP<Widget>& result
   );
   //@}

   static FUSION_DLL_API void registerFunctions( VMState*, const Reg* );
   static FUSION_DLL_API void registerFunctions( VMState*, const char* name, const Reg* );
   static FUSION_DLL_API void registerFunction( VMState*, const char* name, Function func );
   static FUSION_DLL_API void registerFunction( VMState*, const char* nameSpace, const char* name, Function func );
   static FUSION_DLL_API void registerEnum( VMState*, const char* name, const EnumReg* );
   static FUSION_DLL_API void registerEnumReversed( VMState*, const char* name, const EnumReg* );

   static FUSION_DLL_API int gcRCObject( VMState* );

   static FUSION_DLL_API void printStack( VMState*, int numEntries = -1 );
   static FUSION_DLL_API void printCallStack( VMState*, int numEntries = -1 );

   /*----- static data members -----*/

   FUSION_DLL_API static int   _upidx;
};

NAMESPACE_END

#endif

