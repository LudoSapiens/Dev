/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_WORLDVM_H
#define PLASMA_WORLDVM_H

#include <Plasma/StdDefs.h>

#include <Fusion/VM/VM.h>

NAMESPACE_BEGIN

class Camera;
class ContactReceptor;
class ContactGroupReceptor;
class Light;
class ParticleEntity;
class ProxyEntity;
class Receptor;
class RigidEntity;
class SkeletalEntity;
class WorldContext;

/*==============================================================================
  VM proxy functions.
==============================================================================*/

// World.
int world_create( VMState* vm );
int world_get( VMState* vm );
int world_set( VMState* vm );

// Entity.
int entity_get( VMState* vm );
int entity_get_brains( VMState* vm );
int entity_set( VMState* vm );
int entity_set_brains( VMState* vm );

void initCamera( VMState* vm, Camera* c, WorldContext* wc );
void initLight( VMState* vm, Light* l, WorldContext* wc );
void initParticleEntity( VMState* vm, ParticleEntity* e, WorldContext* wc );
void initProxyEntity( VMState* vm, ProxyEntity* e, WorldContext* wc );
void initRigidEntity( VMState* vm, RigidEntity* e, WorldContext* wc );
void initSkeletalEntity( VMState* vm, SkeletalEntity* e, WorldContext* wc );

// Geometry.
int geometry_get( VMState* vm );
int geometry_set( VMState* vm );

// Receptor.
int receptor_get( VMState* vm );
int receptor_set( VMState* vm );
void initReceptor( VMState* vm, int idx, Receptor* r );
void initContactReceptor( VMState* vm, int idx, ContactReceptor* r );
void initContactGroupReceptor( VMState* vm, int idx, ContactGroupReceptor* r );

NAMESPACE_END

#endif

