/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/Action/Command.h>

#include <Plasma/World/Entity.h>
#include <Plasma/World/RigidEntity.h>

#include <Base/ADT/StringMap.h>


USING_NAMESPACE

/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN

//------------------------------------------------------------------------------
//!
enum
{
   CMD_ENTITY,
   CMD_MAKE_DYNAMIC,
   CMD_MAKE_KINEMATIC,
   CMD_MAKE_STATIC,
   CMD_MATERIAL_COLOR,
   NUM_CMDS
};

StringMap _commands(
   "entity"       ,  CMD_ENTITY,
   "makeDynamic"  ,  CMD_MAKE_DYNAMIC,
   "makeKinematic",  CMD_MAKE_KINEMATIC,
   "makeStatic"   ,  CMD_MAKE_STATIC,
   "materialColor",  CMD_MATERIAL_COLOR,
   ""
);

UNNAMESPACE_END


/*==============================================================================
  CLASS Command
==============================================================================*/

//------------------------------------------------------------------------------
//!
Command::Command( uint type, uint rcpCode ):
   _typeRCP( rcpCode | type )
{
   memset( _data, 0, sizeof(_data) );
   CHECK( (type >> 28) == 0x0 ); // Check rcpCode fits in 28b.
}

//-----------------------------------------------------------------------------
//!
Command::Command( const Command& cmd )
{
   // Handle RCP reference counts.
   for( uint i = 0; i < 4; ++i )
   {
      RCObject* o;
      if( cmd.isRCP(i) ) { o = cmd.rcptr(i); if( o ) o->addReference(); }
      // This is why *this = we cannot work.
   }

   // Assign _typeRCP and _data.
   // Note: _typeRCP must be assigned after the code above.
   _typeRCP = cmd._typeRCP;
   _data[0] = cmd._data[0];
   _data[1] = cmd._data[1];
   _data[2] = cmd._data[2];
   _data[3] = cmd._data[3];
}

//------------------------------------------------------------------------------
//!
Command::~Command()
{
   if( hasRCP() )
   {
      RCObject* o;
      for( uint i = 0; i < 4; ++i )
      {
         if( isRCP(i) )
         {
            o = rcptr( i );
            if( o )  o->removeReference();
         }
      }
   }
}

//-----------------------------------------------------------------------------
//!
Command&
Command::operator=( const Command& cmd )
{
   // Handle RCP reference counts.
   for( uint i = 0; i < 4; ++i )
   {
      RCObject* o;
      if( cmd.isRCP(i) ) { o = cmd.rcptr(i); if( o ) o->addReference();    }
      if(     isRCP(i) ) { o =     rcptr(i); if( o ) o->removeReference(); }
   }

   // Assign _typeRCP and _data.
   // Note: _typeRCP must be assigned after the code above.
   _typeRCP = cmd._typeRCP;
   _data[0] = cmd._data[0];
   _data[1] = cmd._data[1];
   _data[2] = cmd._data[2];
   _data[3] = cmd._data[3];

   return *this;
}

//-----------------------------------------------------------------------------
//!
void
Command::print( TextStream& os ) const
{
   os << "Cmd: type=" << type() << " rcp=" << toHex(getRCP()) << nl;
   for( uint i = 0; i < 4; ++i )
   {
      const Chunk& c = _data[i];
      os << " data[" << i << "]:"
         << " u64: " << toHex(c.u64) // As uints.
         << " f32: " << Vec2f::as(c.f32)
         << " ptr: " << c.p << nl;
   }
}

//-----------------------------------------------------------------------------
//!
bool
Command::to( VMState* vm, int idx, Command& cmd )
{
   if( VM::isTable( vm, idx ) )
   {
      if( !VM::geti( vm, idx, 1 ) )  return false;
      const char* type = VM::toCString( vm, -1 );
      VM::pop( vm );

      switch( _commands[type] )
      {
         case CMD_ENTITY:
            StdErr << "TODO: Entity" << nl;
            break;

         case CMD_MAKE_DYNAMIC:
         {
            if( !VM::geti( vm, idx, 2 ) )  return false;
            RigidEntity* e = (RigidEntity*)VM::toProxy( vm, -1 );
            VM::pop( vm );

            cmd = CmdRigidBodyType( e, RigidBody::DYNAMIC );
            return true;
         }  break;

         case CMD_MAKE_KINEMATIC:
         {
            if( !VM::geti( vm, idx, 2 ) )  return false;
            RigidEntity* e = (RigidEntity*)VM::toProxy( vm, -1 );
            VM::pop( vm );

            cmd = CmdRigidBodyType( e, RigidBody::KINEMATIC );
            return true;
         }  break;

         case CMD_MAKE_STATIC:
         {
            if( !VM::geti( vm, idx, 2 ) )  return false;
            RigidEntity* e = (RigidEntity*)VM::toProxy( vm, -1 );
            VM::pop( vm );

            cmd = CmdRigidBodyType( e, RigidBody::STATIC );
            return true;
         }  break;

         case CMD_MATERIAL_COLOR:
         {
            if( !VM::geti( vm, idx, 2 ) )  return false;
            Entity* e = (Entity*)VM::toProxy( vm, -1 );
            VM::pop( vm );

            if( !VM::geti( vm, idx, 3 ) )  return false;
            Vec4f c = VM::toVec4f( vm, -1 );
            VM::pop( vm );

            cmd = CmdMaterialColor( e, c );
            return true;
         }  break;

         default:
            StdErr << "Unknown command type: " << String(type) << nl;
            return false;
      }

      return false;
   }
   else
   {
      StdErr << "ERROR - Command should be defined in a table." << nl;
      return false;
   }
}


/*==============================================================================
  CLASS CmdDelegate0
==============================================================================*/
//-----------------------------------------------------------------------------
//!
//! Layout:
//!   _data[0+] = delegate.
CmdDelegate0::CmdDelegate0( const Func& cb ):
   Command( TYPE_DELEGATE0 )
{
   COMMAND_SANITY_CHECK();
   CHECK( sizeof(Func) < sizeof(*this) );
   delegate( cb );
}


/*==============================================================================
  CLASS CmdDelegate1
==============================================================================*/
//-----------------------------------------------------------------------------
//!
//! Layout:
//!   _data[0+] = delegate.
//!   _data[3]  = ptr0.
CmdDelegate1::CmdDelegate1( const Func& cb, void* p0 ):
   Command( TYPE_DELEGATE1 )
{
   COMMAND_SANITY_CHECK();
   CHECK( sizeof(Func) < sizeof(Chunk)*3 );
   delegate( cb );
   ptr0( p0 );
}

//------------------------------------------------------------------------------
//!
CmdDelegate1::CmdDelegate1( const Func& cb, RCObject* p0 ):
   Command( TYPE_DELEGATE1, RCP_D )
{
   COMMAND_SANITY_CHECK();
   CHECK( sizeof(Func) < sizeof(Chunk)*3 );
   delegate( cb );
   rcptr0( p0 );
}


/*==============================================================================
  CLASS CmdMaterialColor
==============================================================================*/
//-----------------------------------------------------------------------------
//!
//! Layout:
//!   _data[0]._p      = entity (RCP)
//!   _data[1]._f32[0] = color.x
//!   _data[1]._f32[1] = color.y
//!   _data[2]._f32[0] = color.z
//!   _data[2]._f32[1] = color.w
CmdMaterialColor::CmdMaterialColor( Entity* e, const Vec4f& c ):
   Command( TYPE_MATERIAL_COLOR, RCP_A )
{
   COMMAND_SANITY_CHECK();
   entity( e );
   color( c );
}

//-----------------------------------------------------------------------------
//!
void
CmdMaterialColor::entity( Entity* e )
{
   rcptr( 0, e );
}


/*==============================================================================
  CLASS CmdRigidBodyType
==============================================================================*/
//-----------------------------------------------------------------------------
//!
//! Layout:
//!   _data[0]._p      = rigid entity (RCP)
//!   _data[1]._u32[0] = rigid body type
CmdRigidBodyType::CmdRigidBodyType( RigidEntity* e, uint32_t t ):
   Command( TYPE_RIGID_BODY_TYPE, RCP_A )
{
   COMMAND_SANITY_CHECK();
   entity( e );
   type( t );
}

//-----------------------------------------------------------------------------
//!
void
CmdRigidBodyType::entity( RigidEntity* e )
{
   rcptr( 0, e );
}


/*==============================================================================
  CLASS CmdStimulate
==============================================================================*/
//-----------------------------------------------------------------------------
//!
//! Layout:
//!   _data[0]._p = entity (RCP)
//!   _data[1]._p = stimulus (RCP)
CmdStimulate::CmdStimulate( Entity* e, Stimulus* s ):
   Command( TYPE_STIMULATE, RCP_A|RCP_B )
{
   COMMAND_SANITY_CHECK();
   entity( e );
   stimulus( s );
}

//-----------------------------------------------------------------------------
//!
void
CmdStimulate::entity( Entity* e )
{
   rcptr( 0, e );
}

//-----------------------------------------------------------------------------
//!
void
CmdStimulate::stimulus( Stimulus* s )
{
   rcptr( 1, s );
}
