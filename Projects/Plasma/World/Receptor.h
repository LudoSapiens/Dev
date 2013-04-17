/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_RECEPTOR_H
#define PLASMA_RECEPTOR_H

#include <Plasma/StdDefs.h>

#include <Fusion/VM/VM.h>

#include <Base/ADT/Set.h>
#include <Base/Util/RCObject.h>
#include <Base/Util/RCP.h>

NAMESPACE_BEGIN

class RigidEntity;

/*==============================================================================
  CLASS Receptor
==============================================================================*/
class Receptor:
   public RCObject,
   public VMProxy
{
public:
   /*----- static methods -----*/
   static PLASMA_DLL_API void  initialize();

   /*----- types -----*/
   enum
   {
      TYPE_CONTACT,
      TYPE_CONTACT_GROUP
   };

   /*----- methods -----*/
   PLASMA_DLL_API virtual ~Receptor();

   int  type() const { return _type; }

   // VM.
   PLASMA_DLL_API virtual const char* meta() const;

protected:

   /*----- data members -----*/

   int  _type;    //!< The type of the receptor.

   /*----- methods -----*/

   PLASMA_DLL_API Receptor( int type );

}; //class Receptor


/*==============================================================================
  CLASS ContactReceptor
==============================================================================*/
//! A receptor which detects contact with a specific receptor entity.
class ContactReceptor:
   public Receptor
{
public:

   /*----- methods -----*/

   PLASMA_DLL_API ContactReceptor();
   PLASMA_DLL_API virtual ~ContactReceptor();

         RigidEntity*  receivingEntity()       { return _receivingEntity; }
   const RigidEntity*  receivingEntity() const { return _receivingEntity; }
   PLASMA_DLL_API void receivingEntity( RigidEntity* e );

   PLASMA_DLL_API bool interestedInA( const RigidEntity* a, const RigidEntity* b ) const;
   PLASMA_DLL_API bool interestedInB( const RigidEntity* a, const RigidEntity* b ) const;
   inline bool interestedIn( const RigidEntity* a, const RigidEntity* b ) const
   {
      return interestedInA( a, b ) || interestedInB( a, b );
   }

protected:

   /*----- data members -----*/

   RigidEntity*  _receivingEntity;  //!< The entity representing the contact receptor.
}; //class ContactReceptor


/*==============================================================================
  CLASS ContactGroupReceptor
==============================================================================*/
//! A receptor which detects contact with certain collision groups.
class ContactGroupReceptor:
   public Receptor
{
public:

   /*----- methods -----*/

   PLASMA_DLL_API ContactGroupReceptor();
   PLASMA_DLL_API virtual ~ContactGroupReceptor();

   uint  receivingGroups() const { return _receivingGroups; }
   void  receivingGroups( const uint g ) { _receivingGroups = g; }

   uint  collidingGroups() const { return _collidingGroups; }
   void  collidingGroups( const uint c ) { _collidingGroups = c; }

   PLASMA_DLL_API bool  interestedInA( const RigidEntity* a, const RigidEntity* b ) const;
   PLASMA_DLL_API bool  interestedInB( const RigidEntity* a, const RigidEntity* b ) const;
   inline bool  interestedIn( const RigidEntity* a, const RigidEntity* b ) const
   {
      return interestedInA( a, b ) || interestedInB( a, b );
   }

protected:

   /*----- data members -----*/

   uint  _receivingGroups;  //!< The groups of entities serving as contact receptors.
   uint  _collidingGroups;  //!< The groups which can activate the receptor.

}; //class ContactGroupReceptor


NAMESPACE_END

#endif //PLASMA_RECEPTOR_H
