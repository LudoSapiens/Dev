/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_COMMAND_H
#define PLASMA_COMMAND_H

#include <Plasma/StdDefs.h>

#include <Fusion/VM/VM.h>

#include <CGMath/Vec3.h>

#include <Base/Dbg/Defs.h>
#include <Base/Msg/Delegate.h>
#include <Base/Util/RCObject.h>

NAMESPACE_BEGIN

class Entity;
class RigidEntity;
class Stimulus;

/*==============================================================================
  CLASS Command
==============================================================================*/
// A common class for various world commands (i.e. instantaneous actions).
// Important note: children CANNOT add member, as all Commands must have the same size.
// We store everything in a single 256b chunk which can store various elements.
// In order to easily support 32b and 64b architecture, pointers take up 64b.
// The last 32b of data is reserved for internal bookkeeping (both type and RCP flags).
// This means we can have up to:
//     28 bytes (8b) values
//     16 half (16b) values
//     7 dword (32b) values
//  or 3 qword (64b) values (including pointers).
// Every Command needs to carefully mix and match the layout of all of the required fields.
// The pointers (up to 3) can be marked as RCObjects.  When they are, the Command class
// will automatically handle incrementing and decrementing the reference counter.
class Command
{
public:

   enum
   {
      TYPE_DELEGATE0,
      TYPE_DELEGATE1,
      TYPE_MATERIAL_COLOR,
      TYPE_RIGID_BODY_TYPE,
      TYPE_STIMULATE,
      NUM_TYPES
   };

   class Chunk
   {
   public:
      union
      {
         char      c[8];
         int8_t    i8[8];
         uint8_t   u8[8];

         short     s[4];
         int16_t   i16[4];
         uint16_t  u16[4];

         int       i[2];
         float     f[2];
         int32_t   i32[2];
         uint32_t  u32[2];
         float     f32[2];

         double    d;
         double    f64;
         int64_t   i64;
         uint64_t  u64;

         void*     p;
      };
   };

   /*----- methods -----*/
   Command(): _typeRCP( NUM_TYPES ) {}
   PLASMA_DLL_API Command( const Command& cmd );
   PLASMA_DLL_API ~Command();

   inline uint  type() const { return _typeRCP & ~RCP_ALL; }

   inline void  ptr( uint chunkID, void* ptr ) { _data[chunkID].p = ptr; }
   inline void*  ptr( uint chunkID ) { return _data[chunkID].p; }

   inline void  rcptr( uint chunkID, RCObject* ptr );
   inline RCObject*  rcptr( uint chunkID ) const { return (RCObject*)_data[chunkID].p; }

   PLASMA_DLL_API Command& operator=( const Command& cmd );

   PLASMA_DLL_API void  print( TextStream& os = StdErr ) const;

   // VM.
   static PLASMA_DLL_API bool  to( VMState* vm, int idx, Command& cmd );

protected:

   /*----- data members -----*/

   uint32_t  _typeRCP;  //!< The type (bits[27:0]) and RCP code (bits[31:28]).
   Chunk     _data[4];  //!< 4 x 64b chunks of data.

   /*----- methods -----*/

   Command( uint type, uint rcpCode = 0x0 );

   enum
   {
      RCP_A   = 0x01 << 28,
      RCP_B   = 0x02 << 28,
      RCP_C   = 0x04 << 28,
      RCP_D   = 0x08 << 28,
      RCP_ALL = RCP_D | RCP_C | RCP_B | RCP_A
   };

   inline uint  getRCP() const { return (_typeRCP >> 28); }
   inline bool  hasRCP() const { return (_typeRCP >> 28) != 0x0; }
   inline bool  isRCP( uint chunkID ) const { return ((_typeRCP >> (28+chunkID))&0x01) != 0x0; }

#define COMMAND_SANITY_CHECK() \
   CHECK( sizeof(*this) == sizeof(Command) )

#define COMMAND_STANDARD_DECLARE(T) \
   static inline const T&  as( const Command& cmd ) \
   {                                                \
      return reinterpret_cast<const T&>( cmd );     \
   }

private:
}; //class Command


//-----------------------------------------------------------------------------
//!
inline void
Command::rcptr( uint chunkID, RCObject* ptr )
{
   CHECK( isRCP(chunkID) );
   RCObject* old = (RCObject*)_data[chunkID].p;
   if( old ) old->removeReference();
   if( ptr ) ptr->addReference();
   _data[chunkID].p = ptr;
}


/*==============================================================================
  CLASS CmdDelegate0
==============================================================================*/
class CmdDelegate0:
   public Command
{
public:

   /*----- typedef -----*/

   typedef Delegate0<>  Func;

   /*----- methods -----*/

   CmdDelegate0( const Func& cb );

   inline       Func& delegate()         { return reinterpret_cast<      Func&>(_data[0]); }
   inline const Func& delegate() const   { return reinterpret_cast<const Func&>(_data[0]); }
   inline void delegate( const Func& v ) { reinterpret_cast<Func&>(_data[0]) = v;          }

   COMMAND_STANDARD_DECLARE(CmdDelegate0)

protected:
}; //class CmdDelegate0


/*==============================================================================
  CLASS CmdDelegate1
==============================================================================*/
class CmdDelegate1:
   public Command
{
public:

   /*----- typedef -----*/

   typedef Delegate1<void*>  Func;

   /*----- methods -----*/

   CmdDelegate1( const Func& cb, void*     ptr0 );
   CmdDelegate1( const Func& cb, RCObject* ptr0 );

   inline       Func& delegate()         { return reinterpret_cast<      Func&>(_data[0]); }
   inline const Func& delegate() const   { return reinterpret_cast<const Func&>(_data[0]); }
   inline void delegate( const Func& v ) { reinterpret_cast<Func&>(_data[0]) = v;          }

   inline void* ptr0() const    { return _data[3].p; }
   inline void  ptr0( void* v ) { _data[3].p = v;    }

   inline RCObject* rcptr0() const        { return rcptr(3); }
   inline void      rcptr0( RCObject* v ) { rcptr(3, v);     }

   COMMAND_STANDARD_DECLARE(CmdDelegate1)

protected:
}; //class CmdDelegate0


/*==============================================================================
  CLASS CmdMaterialColor
==============================================================================*/
class CmdMaterialColor:
   public Command
{
public:

   /*----- methods -----*/

   CmdMaterialColor( Entity* e, const Vec4f& c );

   inline Entity* entity() const { return (Entity*)rcptr(0); }
   PLASMA_DLL_API void entity( Entity* ent );

   inline void color( const Vec4f& v ) { Vec4f::as(_data[1].f32) = v; }
   inline const Vec4f& color() const { return Vec4f::as(_data[1].f32); }

   COMMAND_STANDARD_DECLARE(CmdMaterialColor)

protected:
}; //class CmdMaterialColor


/*==============================================================================
  CLASS CmdRigidBodyType
==============================================================================*/
class CmdRigidBodyType:
   public Command
{
public:

   /*----- methods -----*/

   CmdRigidBodyType( RigidEntity* e, uint32_t t );

   inline RigidEntity* entity() const { return (RigidEntity*)rcptr(0); }
   PLASMA_DLL_API void entity( RigidEntity* e );

   inline void type( uint32_t type ) { _data[1].u32[0] = type; }
   inline uint32_t type() const { return _data[1].u32[0]; }

   COMMAND_STANDARD_DECLARE(CmdRigidBodyType)

protected:
}; //class CmdRigidBodyType


/*==============================================================================
  CLASS CmdStimulate
==============================================================================*/
class CmdStimulate:
   public Command
{
public:

   /*----- methods -----*/

   CmdStimulate( Entity* e, Stimulus* s );

   inline Entity* entity() const { return (Entity*)rcptr(0); }
   PLASMA_DLL_API void entity( Entity* e );

   inline Stimulus* stimulus() const { return (Stimulus*)rcptr(1); }
   PLASMA_DLL_API void stimulus( Stimulus* s );

   COMMAND_STANDARD_DECLARE(CmdStimulate)

protected:
}; //class CmdStimulate


NAMESPACE_END

#endif //PLASMA_COMMAND_H
