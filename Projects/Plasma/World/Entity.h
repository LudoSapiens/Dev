/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_ENTITY_H
#define PLASMA_ENTITY_H

#include <Plasma/StdDefs.h>
#include <Plasma/Geometry/Geometry.h>
#include <Plasma/World/Brain.h>

#include <CGMath/Ref.h>
#include <CGMath/Variant.h>

#include <Base/Util/RCObject.h>
#include <Base/Util/RCP.h>
#include <Base/Msg/Subject.h>


NAMESPACE_BEGIN

class Brain;
class World;

/*==============================================================================
  CLASS Entity
==============================================================================*/

//! Base Class for all entity type.

class Entity:
   public RCObject,
   public VMProxy
{

public:

   /*----- types and enumerations ----*/

   enum Type
   {
      RIGID,
      CAMERA,
      LIGHT,
      SKELETAL,
      PROXY,
      PARTICLE,
      FLUID,
      SOFT
   };

   /*----- static methods -----*/

   static void initialize();

   /*----- methods -----*/

   inline Type type() const;
   inline World* world() const;
   inline Subject& positionSubject();

   // Position, orientation and scaling.
   PLASMA_DLL_API virtual void referential( const Reff& ref );
   PLASMA_DLL_API virtual void position( const Vec3f& pos );
   PLASMA_DLL_API virtual void orientation( const Quatf& orient );
   PLASMA_DLL_API virtual void scale( float );
   PLASMA_DLL_API virtual void lookAt( const Vec3f& at, const Vec3f& up );
   PLASMA_DLL_API virtual void rotate( const Vec3f& pos, const Vec3f axis, float angle );

   inline const  Reff& referential() const;
   inline const Vec3f& position() const;
   inline const Quatf& orientation() const;
   inline const Mat4f& transform() const;
   inline float scale() const;

   // Visibility states.
   inline bool visible() const;
   inline void visible( bool );
   inline bool castsShadows() const;
   inline void castsShadows( bool );
   inline bool ghost() const;
   inline void ghost( bool );
   inline bool runningActionsBefore() const;
   inline void runningActionsBefore( bool );
   inline bool runningActionsAfter() const;
   inline void runningActionsAfter( bool );

   // Identification.
   PLASMA_DLL_API void id( const ConstString& str );
   inline const ConstString& id() const    { return _id; }

   // Geometry.
   PLASMA_DLL_API virtual void geometry( Geometry* geom );
   inline Geometry* geometry() const       { return _geometry.ptr(); }

   // Material.
   PLASMA_DLL_API void materialSet( MaterialSet* );
   inline MaterialSet* materialSet() const { return _materials.ptr(); }

   // Brain.
   inline void brain( Brain* b )           { _brain = b; }
   inline Brain* brain() const             { return _brain.ptr(); }
   PLASMA_DLL_API void stimulate( Stimulus* s );

   inline void attributes( Table* attr )   { _attributes = attr; }
   inline Table* attributes() const        { return _attributes.ptr(); }

   // VM.
   virtual const char* meta() const;

protected:

   /*----- friends -----*/

   friend class World;

   /*----- methods -----*/

   Entity( Type type );
   PLASMA_DLL_API virtual ~Entity();

   PLASMA_DLL_API virtual void connect( World* w );
   PLASMA_DLL_API virtual void disconnect();

   inline void  update( const Reff& ref, const Mat4f& mat );

   /*----- types -----*/
   enum
   {
      VISIBILE               = 0x01,
      CASTS_SHADOWS          = 0x02,
      GHOST                  = 0x04,
      RUNNING_ACTIONS_BEFORE = 0x08,
      RUNNING_ACTIONS_AFTER  = 0x10,
   };

   inline uint32_t state() const                     { return _state; }
   inline     void state( uint32_t v )               { _state = v; }
   inline     bool getState( uint32_t mask ) const   { return (_state & mask) != 0; }
   inline     void setState( uint32_t mask )         { _state |= mask; }
   inline     void unsetState( uint32_t mask )       { _state &= ~mask; }
   inline     void setState( uint32_t mask, bool v ) { if(v) setState(mask); else unsetState(mask); }
   inline uint32_t getStateBits( uint32_t start, uint32_t size ) const
   {
      return getbits(_state, start, size);
   }
   inline void setStateBits( uint32_t start, uint32_t size, uint32_t newBits )
   {
      _state = setbits( _state, start, size, newBits );
   }

   /*----- data members -----*/

   World*           _world;
   Type             _type;
   uint32_t         _state;
   ConstString      _id;
   RCP<Geometry>    _geometry;
   RCP<MaterialSet> _materials;
   RCP<Brain>       _brain;
   RCP<Table>       _attributes;
   Reff             _referential;
   Mat4f            _transform;
   Subject          _positionSubject;
};

//------------------------------------------------------------------------------
//!
inline Entity::Type
Entity::type() const
{
   return _type;
}

//------------------------------------------------------------------------------
//!
inline World*
Entity::world() const
{
   return _world;
}

//------------------------------------------------------------------------------
//!
inline Subject&
Entity::positionSubject()
{
   return _positionSubject;
}

//------------------------------------------------------------------------------
//!
inline const Reff&
Entity::referential() const
{
   return _referential;
}

//------------------------------------------------------------------------------
//!
inline const Vec3f&
Entity::position() const
{
   return _referential.position();
}

//------------------------------------------------------------------------------
//!
inline const Quatf&
Entity::orientation() const
{
   return _referential.orientation();
}

//------------------------------------------------------------------------------
//!
inline const Mat4f&
Entity::transform() const
{
   return _transform;
}

//------------------------------------------------------------------------------
//!
inline float
Entity::scale() const
{
   return _referential.scale();
}

//------------------------------------------------------------------------------
//!
inline bool
Entity::visible() const
{
   return getState( VISIBILE );
}

//------------------------------------------------------------------------------
//!
inline void
Entity::visible( bool v )
{
   setState( VISIBILE, v );
}

//------------------------------------------------------------------------------
//!
inline bool
Entity::castsShadows() const
{
   return getState( CASTS_SHADOWS );
}

//------------------------------------------------------------------------------
//!
inline void
Entity::castsShadows( bool s )
{
   setState( CASTS_SHADOWS, s );
}

//------------------------------------------------------------------------------
//!
inline bool
Entity::ghost() const
{
   return getState( GHOST );
}

//------------------------------------------------------------------------------
//!
inline void
Entity::ghost( bool g )
{
   setState( GHOST, g );
}

//------------------------------------------------------------------------------
//!
inline bool
Entity::runningActionsBefore() const
{
   return getState( RUNNING_ACTIONS_BEFORE );
}

//------------------------------------------------------------------------------
//!
inline void
Entity::runningActionsBefore( bool g )
{
   setState( RUNNING_ACTIONS_BEFORE, g );
}

//------------------------------------------------------------------------------
//!
inline bool
Entity::runningActionsAfter() const
{
   return getState( RUNNING_ACTIONS_AFTER );
}

//------------------------------------------------------------------------------
//!
inline void
Entity::runningActionsAfter( bool g )
{
   setState( RUNNING_ACTIONS_AFTER, g );
}

//------------------------------------------------------------------------------
//!
inline void
Entity::update( const Reff& ref, const Mat4f& mat )
{
#if 1
   _referential = ref;
   _transform   = mat;
#else
   // Enforce scale.
   _referential.position( ref.position() );
   _referential.orientation( ref.orientation() );
   _transform = Mat4f::scaling( _referential.scale() ) * mat;
#endif
   _positionSubject.notify();
}

NAMESPACE_END

#endif

