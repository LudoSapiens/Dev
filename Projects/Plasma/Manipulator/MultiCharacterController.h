/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_MULTI_CHARACTER_CONTROLLER_H
#define PLASMA_MULTI_CHARACTER_CONTROLLER_H

#include <Plasma/StdDefs.h>

#include <Plasma/Manipulator/Controller.h>


NAMESPACE_BEGIN

class ArrowRenderable;
class DotRenderable;

/*==============================================================================
  CLASS MultiCharacterController
==============================================================================*/
class MultiCharacterController:
   public Controller
{
public:

   /*----- types -----*/

   enum
   {
      CMODE_FLOW,
      CMODE_GRAB,
      CMODE_SHIELD,
      CMODE_SHOCKWAVE,
   };

   /*----- static methods -----*/

   static void initialize();

   /*----- methods -----*/

   PLASMA_DLL_API MultiCharacterController();
   PLASMA_DLL_API virtual ~MultiCharacterController();

   //void  charEvent();
   void pointerPress();
   void pointerMove();
   void pointerRelease();
   void pointerScroll();
   virtual void onViewportChange();
   virtual void onCameraChange();

   // VM.
   PLASMA_DLL_API virtual bool  performGet( VMState* vm );
   PLASMA_DLL_API virtual bool  performSet( VMState* vm );

   void  setMode( const ConstString& charID, int mode );

protected:

   /*----- types and enumerations -----*/

   enum
   {
      MODE_NONE,
      MODE_CHARACTER,
      MODE_FLOW,
      MODE_FLOW_INDICATOR,
      MODE_SCENE,
      MODE_TK,
      MODE_TK_INDICATOR,
      MODE_ZOOM,
   };

   struct PointerData
   {
      int             _mode;
      Vec3f           _pos;
      RigidEntity*    _pickedEntity;
      Vec3f           _pickedOffset;
      SkeletalEntity* _curCharacter;
      bool            _needReset;
   };

   struct CharacterData
   {
      int  _mode;
      CharacterData(): _mode(CMODE_GRAB) {}
   };

   typedef Map< ConstString, CharacterData >  CharacterDataMap;

   /*----- methods -----*/

   RigidEntity* pickTargetIndicator( const Vec2f& pos );
   Entity* pickEntity( const Vec2f& pos, Vec3f& hitPos );
   Vec3f computeDirection( const Vec2f& screenPos, const Vec3f& origin, float& len );
   Vec3f computePathPosition( const Vec2f& screenPos, const Vec3f& lastPos );
   void selectEntity( const ConstString& id );
   void selectEntity( SkeletalEntity* se );

   void handleCorrectionReset( PointerData& pd );

   void arrowReadyCb( Resource<Geometry>* );
   bool renderBeginCb();

   /*----- data members -----*/

   SkeletalEntity*         _selectedCharacter;
   RCP< ArrowRenderable >  _arrow;
   RCP< DotRenderable >    _dot;
   DynArray< PointerData > _pointerData;
   VMRef                   _onCharSelectRef;
   CharacterDataMap        _charData;
};


NAMESPACE_END

#endif
