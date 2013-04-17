/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_TKACTION_H
#define PLASMA_TKACTION_H

#include <Plasma/StdDefs.h>
#include <Plasma/Action/Action.h>
#include <Plasma/World/RigidEntity.h>

#include <MotionBullet/Attractor/Attractor.h>

NAMESPACE_BEGIN

class RigidEntity;
class TKAttractor;
class TKShield;
class TKShockwave;

/*==============================================================================
   CLASS TKSource
==============================================================================*/
class TKSource
{
public:

   /*----- methods -----*/

   TKSource();
   inline const Entity* entity() const            { return _entity;       }
   inline const  Vec3f& localPosition() const     { return _localPos;     }
   inline const  Vec3f& targetPosition() const    { return _targetPos;    }
   inline const  Quatf& targetOrientation() const { return _targetOrient; }
   inline         float force() const             { return _cachedForce;  }
                  float force( const Vec3f& pos );

   void  targetPosition( const Vec3f& desPos );

   /*----- data members -----*/

   Entity* _entity;
   bool    _followOrientation;
   Vec3f   _localPos;
   Vec3f   _targetPos;
   Quatf   _targetOrient;
   float   _maxSqrDist;
   float   _maxForce;
   float   _cachedForce;
};

/*==============================================================================
  CLASS TKAction
==============================================================================*/
class TKAction:
   public Action
{
public:

   /*----- enumerations -----*/

   enum {
      MODE_NONE,
      MODE_FLOW,
      MODE_GRAB,
      MODE_SHIELD,
      MODE_SHIELD_LOWERING,
      MODE_SHIELD_RAISING,
      MODE_SHOCKWAVE,
      MODE_THROW
   };

   /*----- static methods -----*/

   static void initialize();
   static void terminate();

   static Action* create();

   /*----- methods -----*/

   TKAction();

   PLASMA_DLL_API virtual const ConstString&  type() const;

   PLASMA_DLL_API virtual bool  execute( Entity& entity, double time, double delta );

   PLASMA_DLL_API virtual bool  performGet( VMState* vm );
   PLASMA_DLL_API virtual bool  performSet( VMState* vm );

protected:

   /*----- methods -----*/

   virtual ~TKAction();

   void postExecute();

   void grabEntity( RigidEntity* target );
   void targetPosition( const Vec3f& );
   void throwEntity( const Vec3f& );
   void shockwave( float, float );
   void shield( bool );
   void shieldToggle();
   void flow( const Vec3f& dir );
   void flowStop();

   /*----- data members -----*/

   int                        _mode;
   Vec3f                      _throwVelocity;
   Vec3f                      _offset;
   Vec3f                      _flowDir;
   float                      _shockwaveSpeed;
   float                      _shockwaveForce;
   RigidEntity*               _target;
   TKSource                   _source;
   RCP<RigidEntity>           _beam;
   RCP<TKShield>              _shield;
   Vector< RCP<TKShockwave> > _shockwaves;
   TKAttractor*               _attractor;
};

/*==============================================================================
   CLASS TKAttractor
==============================================================================*/
class TKAttractor:
   public Attractor
{
public:

   /*----- static methods -----*/

   static PLASMA_DLL_API TKAttractor*  getAttractor( RigidEntity* e );
   static PLASMA_DLL_API TKAttractor*  getCurrentAttractor( RigidEntity* e );
   static PLASMA_DLL_API         void  cleanAttractors();
   static PLASMA_DLL_API const Map< RigidEntity*, RCP<TKAttractor> >& getAttractors();

   /*----- methods -----*/

   inline TKAttractor() {}

   PLASMA_DLL_API virtual void addForce( const Vector< RCP<RigidBody> >& );

   inline    RigidEntity* target() const              { return _target.ptr();   }
   inline            void target( RigidEntity* e )    { _target = e; _expLinVel = e->linearVelocity(); _correction = 0.0f; }
   inline            void addSource( TKSource* s )    { _sources.pushBack( s ); }
   inline            void removeSource( TKSource* s ) { _sources.remove( s );   }
   inline          size_t numSources() const          { return _sources.size(); }
   inline       TKSource* source( size_t i )          { return _sources[i];     }
   inline const TKSource* source( size_t i ) const    { return _sources[i];     }
   inline    const Vec3f& correction() const          { return _correction;     }
   inline            void resetCorrection()           { _correction = 0.0f;     }

   PLASMA_DLL_API void  targetPosition( const Vec3f& v );

protected:

   /*----- static methods -----*/


   /*----- data members -----*/

   RCP<RigidEntity>  _target;
   Vector<TKSource*> _sources;
   Vec3f             _expLinVel;
   Vec3f             _correction;
};
NAMESPACE_END

#endif
