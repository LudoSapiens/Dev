/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/Widget/PlasmaBrowser.h>
#include <Plasma/Geometry/Geometry.h>
#include <Plasma/Resource/ResManager.h>
#include <Plasma/Intersector.h>

#include <Fusion/VM/VMObjectPool.h>
#include <Fusion/Core/Core.h>

#include <Base/ADT/StringMap.h>
#include <Base/Util/Platform.h>

#if PLAT_MOBILE
#include <Plasma/Render/ForwardRenderer.h>
#include <Plasma/Render/ForwardRendererFixed.h>
#else
#include <Plasma/Render/ForwardRendererHDR.h>
#endif

/*==============================================================================
  UNNAME NAMESPACE
==============================================================================*/

UNNAMESPACE_BEGIN

const char* _plasmaBrowser_str_ = "plasmaBrowser";
const char* _browserItem_str_   = "browserItem";

//------------------------------------------------------------------------------
//!
inline void
execute( const VMRef& ref, Widget* widget, int i )
{
   if( ref.isValid() )
   {
      VMState* vm = ref.vm();
      VM::push( vm, ref );
      VM::pushProxy( vm, widget );
      VM::push( vm, i );
      VM::ecall( vm, 2, 0 );
   }
}

//------------------------------------------------------------------------------
//!
int numItemsVM( VMState* vm )
{
   PlasmaBrowser* br = (PlasmaBrowser*)VM::thisPtr( vm );
   VM::push( vm, br->numItems() );
   return 1;
}

//------------------------------------------------------------------------------
//!
int numStacksVM( VMState* vm )
{
   PlasmaBrowser* br = (PlasmaBrowser*)VM::thisPtr( vm );
   VM::push( vm, br->numStacks() );
   return 1;
}

//------------------------------------------------------------------------------
//!
int addItemVM( VMState* vm )
{
   PlasmaBrowser* br     = (PlasmaBrowser*)VM::thisPtr( vm );
   RCP<BrowserItem> item = (BrowserItem*)VM::toProxy( vm, 1 );
   br->addItem( item );
   return 0;
}

//------------------------------------------------------------------------------
//!
int removeItemVM( VMState* vm )
{
   PlasmaBrowser* br = (PlasmaBrowser*)VM::thisPtr( vm );

   if( VM::isNumber( vm, 1 ) )
   {
      br->removeItem( VM::toUInt( vm, 1 ) );
   }
   else
   {
      RCP<BrowserItem> item = (BrowserItem*)VM::toProxy( vm, 1 );
      br->removeItem( item );
   }
   return 0;
}

//------------------------------------------------------------------------------
//!
int moveItemVM( VMState* vm )
{
   PlasmaBrowser* br = (PlasmaBrowser*)VM::thisPtr( vm );
   br->moveItem( VM::toUInt( vm, 1 ) - 1, VM::toUInt( vm, 2 ) - 1 );
   return 0;
}

//------------------------------------------------------------------------------
//!
int clearCurrentItemsVM( VMState* vm )
{
   PlasmaBrowser* br = (PlasmaBrowser*)VM::thisPtr( vm );
   br->clearCurrentItems();
   return 0;
}

//------------------------------------------------------------------------------
//!
int clearItemsVM( VMState* vm )
{
   PlasmaBrowser* br = (PlasmaBrowser*)VM::thisPtr( vm );
   br->clearItems();
   return 0;
}

//------------------------------------------------------------------------------
//!
int setItemsVM( VMState* vm )
{
   PlasmaBrowser* br = (PlasmaBrowser*)VM::thisPtr( vm );
   Vector< RCP<BrowserItem> > items;
   for( int i = 1; VM::geti( vm, 1, i ); ++i )
   {
      items.pushBack( RCP<BrowserItem>( (BrowserItem*)VM::toProxy( vm, -1 ) ) );
      VM::pop( vm );
   }
   br->setItems( items );
   return 0;
}

//------------------------------------------------------------------------------
//!
int pushItemsVM( VMState* vm )
{
   PlasmaBrowser* br = (PlasmaBrowser*)VM::thisPtr( vm );
   Vector< RCP<BrowserItem> > items;
   for( int i = 1; VM::geti( vm, 1, i ); ++i )
   {
      items.pushBack( RCP<BrowserItem>( (BrowserItem*)VM::toProxy( vm, -1 ) ) );
      VM::pop( vm );
   }
   br->pushItems( items );
   return 0;
}

//------------------------------------------------------------------------------
//!
int popItemsVM( VMState* vm )
{
   PlasmaBrowser* br = (PlasmaBrowser*)VM::thisPtr( vm );
   br->popItems();
   return 0;
}

//------------------------------------------------------------------------------
//!
enum {
   ATTRIB_NUM_ITEMS,
   ATTRIB_NUM_STACKS,
   ATTRIB_ADD_ITEM,
   ATTRIB_REMOVE_ITEM,
   ATTRIB_MOVE_ITEM,
   ATTRIB_CLEAR_CURRENT_ITEMS,
   ATTRIB_CLEAR_ITEMS,
   ATTRIB_CURRENT_ITEM,
   ATTRIB_ON_ITEM_SELECT,
   ATTRIB_OFFSET,
   ATTRIB_POP_ITEMS,
   ATTRIB_PUSH_ITEMS,
   ATTRIB_ROTATION_SPEED,
   ATTRIB_SET_ITEMS
};

StringMap _attributes(
   "numItems",          ATTRIB_NUM_ITEMS,
   "numStacks",         ATTRIB_NUM_STACKS,
   "addItem",           ATTRIB_ADD_ITEM,
   "removeItem",        ATTRIB_REMOVE_ITEM,
   "moveItem",          ATTRIB_MOVE_ITEM,
   "clearCurrentItems", ATTRIB_CLEAR_CURRENT_ITEMS,
   "clearItems",        ATTRIB_CLEAR_ITEMS,
   "currentItem",       ATTRIB_CURRENT_ITEM,
   "onItemSelect",      ATTRIB_ON_ITEM_SELECT,
   "offset",            ATTRIB_OFFSET,
   "popItems",          ATTRIB_POP_ITEMS,
   "pushItems",         ATTRIB_PUSH_ITEMS,
   "rotationSpeed",     ATTRIB_ROTATION_SPEED,
   "setItems",          ATTRIB_SET_ITEMS,
   ""
);

enum {
   ATTRIB_GEOMETRY,
   ATTRIB_MATERIAL,
   ATTRIB_SCALE,
   ATTRIB_WIDGET,
};

StringMap _itemAttributes(
   "geometry", ATTRIB_GEOMETRY,
   "material", ATTRIB_MATERIAL,
   "scale",    ATTRIB_SCALE,
   "widget",   ATTRIB_WIDGET,
   ""
);

UNNAMESPACE_END


NAMESPACE_BEGIN


/*==============================================================================
   CLASS BrowserAnimator
==============================================================================*/

class BrowserAnimator:
   public Animator
{
public:

   /*----- methods -----*/

   BrowserAnimator( PlasmaBrowser* br ): _browser( br ) {}
   virtual ~BrowserAnimator() {}

   bool exec( double /*time*/, double delta )
   {
      float dindex = _browser->_targetIndex - _browser->_index;
      if( dindex != 0.0f )
      {
         float maxDelta = float(delta*_browser->_currentSpeed);
         if( dindex > maxDelta )
         {
            _browser->_index += maxDelta;
         }
         else if( dindex < -maxDelta )
         {
            _browser->_index -= maxDelta;
         }
         else
         {
            _browser->_index = _browser->_targetIndex;
         }
         _browser->updateWorld();
      }
      // Item rotation.
      uint index   = _browser->currentItem();
      float rspeed = _browser->_rotationSpeed;
      if( index < _browser->numItems() && rspeed != 0.0f )
      {
         Entity* e = _browser->_entities[index];
         Quatf q = Quatf::axisCir( Vec3f( 0.0f, 1.0f, 0.0f ), float(rspeed*delta) );
         e->orientation( e->orientation() * q );
      }
      return false;
   }

protected:

   /*----- data members -----*/

   PlasmaBrowser*  _browser;
};

/*==============================================================================
   CLASS BrowserItem
==============================================================================*/

//------------------------------------------------------------------------------
//!
void
BrowserItem::initialize()
{
   VMObjectPool::registerObject(
      "UI",
      _browserItem_str_,
      stdCreateVM<BrowserItem>,
      stdGetVM<BrowserItem>,
      stdSetVM<BrowserItem>
   );
}

//------------------------------------------------------------------------------
//!
BrowserItem::BrowserItem():
   _browser(nullptr),
   _scale(0.0f)
{
}

//------------------------------------------------------------------------------
//!
BrowserItem::~BrowserItem()
{
}

//------------------------------------------------------------------------------
//!
void
BrowserItem::scale( float s )
{
   if( s == _scale ) return;
   _scale = s;

   if( _browser ) _browser->updateEntity( _entity.ptr(), _scale );
}

//------------------------------------------------------------------------------
//!
void
BrowserItem::geometry( const String& name )
{
   if( name == _geomName ) return;
   _geomName = name;
   _geom     = 0;

   if( _entity.isValid() )
   {
      if( !geometry().empty() )
      {
         _geom = ResManager::getGeometry( geometry(), nullptr );
         _geom->callOnLoad( makeDelegate( this, &BrowserItem::geometryCb ) );
      }
      else
      {
         _entity->geometry(0);
      }
   }
}

//------------------------------------------------------------------------------
//!
void
BrowserItem::material( const String& name )
{
   if( name == _materialsName ) return;
   _materialsName = name;
   _materials     = 0;

   if( _entity.isValid() )
   {
      if( !material().empty() )
      {
         _materials = ResManager::newMaterialSet( material(), nullptr );
         _materials->callOnLoad( makeDelegate( this, &BrowserItem::materialCb ) );
      }
      else
      {
         _entity->materialSet(0);
      }
   }
}

//------------------------------------------------------------------------------
//!
void
BrowserItem::widget( Widget* w )
{
   if( _widget == w ) return;
   _widget = w;

   if( _browser ) _browser->markForUpdate(true);
}

//------------------------------------------------------------------------------
//!
const char*
BrowserItem::meta() const
{
   return _browserItem_str_;
}

//------------------------------------------------------------------------------
//!
void
BrowserItem::prepareEntity( Entity* e, PlasmaBrowser* br )
{
   // Assignment.
   _entity  = e;
   _browser = br;

   // Material creation.
   if( !material().empty() )
   {
      if( _mparams.isValid() )
         _materials = ResManager::newMaterialSet( material(), *_mparams, nullptr );
      else
         _materials = ResManager::newMaterialSet( material(), nullptr );
      _materials->callOnLoad( makeDelegate( this, &BrowserItem::materialCb ) );
   }

   // Geometry.
   if( !geometry().empty() )
   {
      if( _gparams.isValid() )
         _geom = ResManager::getGeometry( geometry(), *_gparams, nullptr );
      else
         _geom = ResManager::getGeometry( geometry(), nullptr );
      _geom->callOnLoad( makeDelegate( this, &BrowserItem::geometryCb ) );
   }
}

//------------------------------------------------------------------------------
//!
void
BrowserItem::geometryCb( Resource<Geometry>* res )
{
   if( res != _geom.ptr() ) return;

   if( _materials.isNull() || _materials->state() == Resource<MaterialSet>::LOADED )
   {
      _entity->geometry( data( _geom ) );
      _entity->materialSet( data( _materials ) );
      _browser->updateEntity( _entity.ptr(), _scale );
   }
}

//------------------------------------------------------------------------------
//!
void
BrowserItem::materialCb( Resource<MaterialSet>* res )
{
   if( res != _materials.ptr() ) return;

   if( _geom.isValid() && _geom->state() == Resource<Geometry>::LOADED )
   {
      _entity->geometry( data( _geom ) );
      _entity->materialSet( data( _materials ) );
      _browser->updateEntity( _entity.ptr(), _scale );
   }
}

//------------------------------------------------------------------------------
//!
void
BrowserItem::releaseEntity()
{
   _browser   = 0;
   _entity    = 0;
   _materials = 0;
   _geom      = 0;
}

//------------------------------------------------------------------------------
//!
void
BrowserItem::init( VMState* vm )
{
   // Read geometry.
   if( VM::get( vm, 1, "geometry" ) )
   {
      _geomName = VM::toString( vm, -1 );
      VM::pop( vm );
   }

   // Read geometry parameters.
   if( VM::get( vm, 1, "gparams" ) )
   {
      _gparams = new Table();
      VM::toTable( vm, -1, *_gparams );
      VM::pop( vm );
   }

   // Read material.
   if( VM::get( vm, 1, "material" ) )
   {
      _materialsName = VM::toString( vm, -1 );
      VM::pop( vm );
   }

   // Read material parameters.
   if( VM::get( vm, 1, "mparams" ) )
   {
      _mparams = new Table();
      VM::toTable( vm, -1, *_mparams );
      VM::pop( vm );
   }

   // Scaling factor for geometry rendering. <= 0 means auto mode.
   VM::get( vm, 1, "scale", _scale );

   // Read widget.
   if( VM::geti( vm, 1, 1 ) )
   {
      _widget = (Widget*)VM::toProxy( vm, -1 );
      VM::pop( vm );
   }

   // Create attributes table.
   VM::newTable( vm );
   VM::push( vm );

   while( VM::next( vm, 1 ) )
   {
      if( VM::isNumber( vm, -2 ) ||
          isAttribute( VM::toCString( vm, -2 ) ) )
      {
         VM::pop( vm, 1 );
      }
      else
      {
         // Duplicate the key.
         VM::pushValue( vm, -2 );
         VM::insert( vm, -3 );

         // Insert attribute into table.
         VM::set( vm, -4 );
      }
   }
   // Keep a reference on user attributes table.
   VM::toRef( vm, -1, _userAttributes );
}

//------------------------------------------------------------------------------
//!
bool
BrowserItem::performGet( VMState* vm )
{
   switch( _itemAttributes[ VM::toCString( vm, 2 ) ] )
   {
      case ATTRIB_GEOMETRY:
         VM::push( vm, _geomName );
         return true;
      case ATTRIB_MATERIAL:
         VM::push( vm, _materialsName );
         return true;
      case ATTRIB_SCALE:
         VM::push( vm, _scale );
         return true;
      case ATTRIB_WIDGET:
         VM::pushProxy( vm, _widget.ptr() );
         return true;
      default: break;
   }

   // Attribute not found, so its considered a user attribute.
   VM::push( vm, _userAttributes );
   VM::pushValue( vm, 2 );
   VM::get( vm, -2 );

   return true;
}

//------------------------------------------------------------------------------
//!
bool
BrowserItem::performSet( VMState* vm )
{
   switch( _itemAttributes[ VM::toCString( vm, 2 ) ] )
   {
      case ATTRIB_GEOMETRY:
         geometry( VM::toString( vm, 3 ) );
         return true;
      case ATTRIB_MATERIAL:
         material( VM::toString( vm, 3 ) );
         return true;
      case ATTRIB_SCALE:
         scale( VM::toFloat( vm, 3 ) );
         return true;
      case ATTRIB_WIDGET:
         widget( (Widget*)VM::toProxy( vm, 3 ) );
         return true;
      default: break;
   }

   // Attribute not found, so its considered a user attribute.
   VM::push( vm, _userAttributes );
   VM::insert( vm, 2 );
   VM::set( vm, 2 );

   return true;
}
//------------------------------------------------------------------------------
//!
bool
BrowserItem::isAttribute( const char* name ) const
{
   return _itemAttributes[ name ] != StringMap::INVALID;
}

/*==============================================================================
   CLASS PlasmaBrowser
==============================================================================*/

//------------------------------------------------------------------------------
//!
void
PlasmaBrowser::initialize()
{
   VMObjectPool::registerObject(
      "UI",
      _plasmaBrowser_str_,
      stdCreateVM<PlasmaBrowser>,
      stdGetVM<PlasmaBrowser>,
      stdSetVM<PlasmaBrowser>
   );

   BrowserItem::initialize();
}

//------------------------------------------------------------------------------
//!
PlasmaBrowser::PlasmaBrowser():
   _mode( FLOW ),
   _index( 0.0f ), _targetIndex( 0.0f ),
   _speed( 5.0f ), _currentSpeed(0.0f), _rotationSpeed( 0.0f ),
   _offset( 0.0f )
{
   _world      = new World();
   _background = new RigidEntity( RigidBody::STATIC );
   _lists.pushBack( new BrowserList() );

#if PLAT_MOBILE
   if( Core::gfxVersion() == 1 )
   {
      _renderer = new ForwardRendererFixed();
   }
   else
   {
      _renderer = new ForwardRenderer();
   }
#else
   _renderer = new ForwardRendererHDR();
#endif

   _animator = new BrowserAnimator( this );
   Core::addAnimator( _animator.ptr() );
}

//------------------------------------------------------------------------------
//!
PlasmaBrowser::~PlasmaBrowser()
{
   releaseItems();
   Core::removeAnimator( _animator.ptr() );
}

//------------------------------------------------------------------------------
//!
void
PlasmaBrowser::mode( int /*m*/ )
{
   // TODO
}

//------------------------------------------------------------------------------
//!
void
PlasmaBrowser::addItem( const RCP<BrowserItem>& item )
{
   items().pushBack( item );
   markForUpdate( true );

   // Create entity.
   RigidEntity* e = new RigidEntity( RigidBody::STATIC );
   _world->addEntity( e );
   _entities.pushBack( e );
   // Load material and geometry.
   item->prepareEntity( e, this );
   // Position entities.
   updateWorld();
}

//------------------------------------------------------------------------------
//!
void
PlasmaBrowser::removeItem( const RCP<BrowserItem>& /*item*/ )
{
   // TODO
   //items().remove( item );
   //markForUpdate();
}

//------------------------------------------------------------------------------
//!
void
PlasmaBrowser::removeItem( uint i )
{
   removeItem( item(i) );
}

//------------------------------------------------------------------------------
//!
void
PlasmaBrowser::moveItem( uint fromIdx, uint toIdx )
{
   _entities.move( fromIdx, toIdx );
   _lists.back()->_items.move( fromIdx, toIdx );
   updateWorld();
}

//------------------------------------------------------------------------------
//!
void
PlasmaBrowser::clearCurrentItems()
{
   releaseItems();
   items().clear();
   _index        = 0.0f;
   _targetIndex  = 0.0f;
   createWorld();
   markForUpdate( true );
}

//------------------------------------------------------------------------------
//!
void
PlasmaBrowser::clearItems()
{
   releaseItems();
   _lists.clear();
   _lists.pushBack( new BrowserList() );

   _index        = 0.0f;
   _targetIndex  = 0.0f;
   createWorld();
   markForUpdate( true );
}

//------------------------------------------------------------------------------
//!
void PlasmaBrowser::setItems( Vector< RCP<BrowserItem> >& items )
{
   releaseItems();
   _lists.clear();
   _lists.pushBack( new BrowserList() );

   _index        = 0.0f;
   _targetIndex  = 0.0f;
   _lists.back()->_items.swap( items );
   createWorld();
   markForUpdate( true );
}

//------------------------------------------------------------------------------
//!
void
PlasmaBrowser::pushItems( Vector< RCP<BrowserItem> >& items )
{
   releaseItems();

   _lists.back()->_index = CGM::floor(_index+0.5f);
   _lists.pushBack( new BrowserList() );

   _index        = 0.0f;
   _targetIndex  = 0.0f;
   _lists.back()->_items.swap( items );
   createWorld();
   markForUpdate( true );
}

//------------------------------------------------------------------------------
//!
void
PlasmaBrowser::popItems()
{
   // Always keep at least one list.
   if( _lists.size() <= 1 ) return;

   releaseItems();
   _lists.popBack();
   _index        = _lists.back()->_index;
   _targetIndex  = _index;
   createWorld();
   markForUpdate( true );
}

//------------------------------------------------------------------------------
//!
void
PlasmaBrowser::gotoItem( int i )
{
   _currentSpeed = _speed;
   _targetIndex  = float(i);
   _targetIndex  = CGM::clamp( _targetIndex, 0.0f, float(items().size()-1) );
}

//------------------------------------------------------------------------------
//!
void
PlasmaBrowser::addOnItemSelect( const ItemDelegate& delegate )
{
   _onItemSelect.addDelegate( delegate );
}

//------------------------------------------------------------------------------
//!
void
PlasmaBrowser::removeOnItemSelect( const ItemDelegate& delegate )
{
   _onItemSelect.removeDelegate( delegate );
}

//------------------------------------------------------------------------------
//!
void
PlasmaBrowser::onPointerPress( const Event& ev )
{
   Widget::onPointerPress( ev );
}

//------------------------------------------------------------------------------
//!
void
PlasmaBrowser::onPointerRelease( const Event& ev )
{
   Pointer& pointer = ev.pointer();
   if( !_entities.empty() && pointer.withinPress( 10 ) )
   {
      // Test if clicking on item.
      int index = currentItem();
      Entity* e = _entities[index];

      if( e->geometry() )
      {
         // 1. Construct a canonical bb.
         AABBoxf box( e->transform()*e->geometry()->boundingBox().center() );
         box.grow( 1.0f );

         // 2. Test bb intersection with camera ray.
         Rayf ray( _viewport.camera()->position(), _viewport.direction( ev.position() ) );
         float mint = 0.0f;
         float maxt = CGConstf::infinity();
         if( Intersector::trace( box, ray, mint, maxt ) )
         {
            onItemSelect( index );
         }
         else
         {
            Mat4f m  = _viewport.cameraMatrix();
            float y0 = (m | box.corner(4)).y;
            float y1 = (m | box.corner(6)).y;
            Vec2i p  = ev.position();
            if( p.y > y0 && p.y < y1 )
            {
               gotoItem( p.x < getCenterGlobal().x ? index-1 : index+1 );
            }
         }
      }
   }
   else
   {
      Vec2f speed   = ev.pointer().getSpeed();
      gotoItem( (int)CGM::round( _index - speed.x*0.001f ) );
      _currentSpeed = CGM::max( 0.5f, CGM::abs( speed.x*0.005f ) );
   }
   Widget::onPointerRelease( ev );
}

//------------------------------------------------------------------------------
//!
void
PlasmaBrowser::onPointerMove( const Event& ev )
{
   Pointer& pointer = ev.pointer();
   if( !pointer.pressed() ) return;

   const Vec2i& curPos = ev.position();
   const Vec2i& oldPos = pointer.lastPosition();

   if( !pointer.withinPress( 10 ) )
   {
      _index      -= float(curPos.x-oldPos.x)*0.01f;
      _index       = CGM::clamp( _index, 0.0f, float(items().size()-1) );
      _targetIndex = _index;
      updateWorld();
   }
   Widget::onPointerMove( ev );
}

//------------------------------------------------------------------------------
//!
void PlasmaBrowser::onPointerScroll( const Event& ev )
{
   float dx = ev.scrollValue().x;
   if( CGM::abs(dx) < 0.1f ) return;

   _index       += dx * 0.1f;
   _index        = CGM::clamp( _index, 0.0f, float(items().size()-1) );
   _targetIndex  = dx > 0.0f ? CGM::ceil(_index) : CGM::floor(_index);
   _currentSpeed = CGM::abs(dx);
   updateWorld();
}

//------------------------------------------------------------------------------
//!
void
PlasmaBrowser::onItemSelect( int i )
{
   _onItemSelect.exec( i );
   execute( _onItemSelectRef, this, i+1 );
}

//------------------------------------------------------------------------------
//!
const char*
PlasmaBrowser::meta() const
{
   return _plasmaBrowser_str_;
}

//------------------------------------------------------------------------------
//!
void
PlasmaBrowser::render( const RCP<Gfx::RenderNode>& rn )
{
   _renderer->beginFrame();
   _renderer->render( rn, _world.ptr(), &_viewport );
   _renderer->endFrame();
   Widget::render( rn );

   // Render items widgets.
   int idx = currentItem();
   if( idx < (int)items().size() && item(idx)->widget() )
   {
      item(idx)->widget()->render( rn );
   }
}

//------------------------------------------------------------------------------
//!
void
PlasmaBrowser::performSetGeometry()
{
   _renderer->size( actualSize() );
   _viewport.region( globalPosition(), actualSize() );
   Widget::performSetGeometry();

   Vec2i newPos(0);
   Vec2i newSize = actualSize();
   Vector< RCP<BrowserItem> >& items = this->items();
   for( uint i = 0; i < items.size(); ++i )
   {
      if( items[i]->widget() )
         items[i]->widget()->geometry( globalPosition(), newPos, newSize );
   }
}

//------------------------------------------------------------------------------
//!
void
PlasmaBrowser::performSetPosition()
{
   _viewport.region( globalPosition(), actualSize() );
   Widget::performSetPosition();

   Vector< RCP<BrowserItem> >& items = this->items();
   for( uint i = 0; i < items.size(); ++i )
   {
      if( items[i]->widget() )
         items[i]->widget()->position( globalPosition(), items[i]->widget()->localPosition() );
   }
}

//------------------------------------------------------------------------------
//!
void
PlasmaBrowser::init( VMState* vm )
{
   VM::get( vm, 1, "onItemSelect", _onItemSelectRef );
   VM::get( vm, 1, "rotationSpeed", _rotationSpeed );
   VM::get( vm, 1, "offset", _offset );

   // Read geometry.
   if( VM::get( vm, 1, "geometry" ) )
   {
      _geom = ResManager::getGeometry( VM::toString( vm, -1 ), nullptr );
      VM::pop( vm );
   }

   // Read material.
   if( VM::get( vm, 1, "material" ) )
   {
      _materials = ResManager::newMaterialSet( VM::toString( vm, -1 ), nullptr );
      VM::pop( vm );
   }

   Vector< RCP<BrowserItem> >& items = this->items();
   for( int i = 1; VM::geti( vm, 1, i ); ++i )
   {
      items.pushBack( RCP<BrowserItem>( (BrowserItem*)VM::toProxy( vm, -1 ) ) );
      VM::pop( vm );
   }

   createWorld();

   Widget::init( vm );
}

//------------------------------------------------------------------------------
//!
void
PlasmaBrowser::createWorld()
{
   // Reset world.
   _world->removeAllEntities();
   _entities.clear();

   // Camera.
   Camera* cam = new Camera( RigidBody::STATIC );
   _world->addEntity( cam );
   _world->backgroundColor( Vec4f( 0.0f, 0.0f, 0.0f, 1.0f ) );
   _viewport.camera( cam );
   _viewport.camera()->position( Vec3f(0.0f, 1.0f, 0.0f ) );
   _viewport.camera()->lookAt( Vec3f( 0.0f, 1.0f, -8.0f ), Vec3f( 0.0f, 1.0f, 0.0f ) );

   // Background geometry.
   _world->addEntity( _background.ptr() );
   if( _geom.isValid() ) _geom->callOnLoad( makeDelegate( this, &PlasmaBrowser::geometryReadyCb ) );
   if( _materials.isValid() ) _materials->callOnLoad( makeDelegate( this, &PlasmaBrowser::materialSetReadyCb ) );

   // Item entities.
   Vector< RCP<BrowserItem> >& items = this->items();
   for( uint i = 0; i < items.size(); ++i )
   {
      // Create entity.
      RigidEntity* e = new RigidEntity( RigidBody::STATIC );
      _world->addEntity( e );
      _entities.pushBack( e );

      // Load material and geometry.
      items[i]->prepareEntity( e, this );
   }
   // Position items.
   updateWorld();
}

//------------------------------------------------------------------------------
//!
void
PlasmaBrowser::updateWorld()
{
   Vector< RCP<BrowserItem> >& items = this->items();
   for( uint i = 0; i < items.size(); ++i )
   {
      Vec3f pos( _entities[i]->position() );
      pos.x  = (float(i)-_index);
      pos.z  = -16.0f + 8.0f*CGM::smoothStep( -1.0f, 1.0f, 1.0f-CGM::abs(pos.x) );
      pos.x *= 2.0f;
      _entities[i]->position( pos );
   }
}

//------------------------------------------------------------------------------
//!
void
PlasmaBrowser::updateEntity( Entity* e, float scale )
{
   Geometry* geom = e->geometry();
   if( geom )
   {
      float s   = scale <= 0.0f ? 2.0f / geom->boundingBox().maxSize() : scale;
      Vec3f pos = e->position();
      pos.y     = -s * geom->boundingBox().min(1) + _offset;
      e->scale( s );
      e->position( pos );
   }
}

//------------------------------------------------------------------------------
//!
void
PlasmaBrowser::updateEntities()
{
   for( uint i = 0; i < _entities.size(); ++i )
   {
      Entity* e = _entities[i];
      if( e->geometry() )
         updateEntity( e, e->scale() );
   }
}

//------------------------------------------------------------------------------
//!
void
PlasmaBrowser::releaseItems()
{
   Vector< RCP<BrowserItem> >& items = this->items();
   for( uint i = 0; i < items.size(); ++i )
   {
      items[i]->releaseEntity();
   }
}

//------------------------------------------------------------------------------
//!
void
PlasmaBrowser::geometryReadyCb( Resource<Geometry>* res )
{
   if( _geom != res ) return;
   if( _materials.isNull() || _materials->isReady() )
   {
      _background->geometry( data( _geom ) );
      _background->materialSet( data( _materials ) );
   }
}

//------------------------------------------------------------------------------
//!
void
PlasmaBrowser::materialSetReadyCb( Resource<MaterialSet>* res )
{
   if( _materials != res ) return;
   if( _geom.isValid() && _geom->isReady() )
   {
      _background->geometry( data( _geom ) );
      _background->materialSet( data( _materials ) );
   }
}

//------------------------------------------------------------------------------
//!
bool
PlasmaBrowser::performGet( VMState* vm )
{
   if( VM::isNumber( vm, 2 ) )
   {
      int i = VM::toInt( vm, 2 )-1;
      if( i < 0 || i >= (int)items().size() ) return false;
      VM::pushProxy( vm, item(i) );
      return true;
   }

   switch( _attributes[ VM::toCString( vm, 2 ) ] )
   {
      case ATTRIB_NUM_ITEMS:
         VM::push( vm, this, numItemsVM );
         return true;
      case ATTRIB_NUM_STACKS:
         VM::push( vm, this, numStacksVM );
         return true;
      case ATTRIB_ADD_ITEM:
         VM::push( vm, this, addItemVM );
         return true;
      case ATTRIB_REMOVE_ITEM:
         VM::push( vm, this, removeItemVM );
         return true;
      case ATTRIB_MOVE_ITEM:
         VM::push( vm, this, moveItemVM );
         return true;
      case ATTRIB_CLEAR_CURRENT_ITEMS:
         VM::push( vm, this, clearCurrentItemsVM );
         return true;
      case ATTRIB_CLEAR_ITEMS:
         VM::push( vm, this, clearItemsVM );
         return true;
      case ATTRIB_CURRENT_ITEM:
         VM::push( vm, currentItem()+1 );
         return true;
      case ATTRIB_ON_ITEM_SELECT:
         VM::push( vm, _onItemSelectRef );
         return true;
      case ATTRIB_OFFSET:
         VM::push( vm, _offset );
      case ATTRIB_POP_ITEMS:
         VM::push( vm, this, popItemsVM );
         return true;
      case ATTRIB_PUSH_ITEMS:
         VM::push( vm, this, pushItemsVM );
         return true;
      case ATTRIB_ROTATION_SPEED:
         VM::push( vm, _rotationSpeed );
         return true;
      case ATTRIB_SET_ITEMS:
         VM::push( vm, this, setItemsVM );
         return true;
      default: break;
   }

   return Widget::performGet( vm );
}

//------------------------------------------------------------------------------
//!
bool
PlasmaBrowser::performSet( VMState* vm )
{
   switch( _attributes[ VM::toCString( vm, 2 ) ] )
   {
      case ATTRIB_NUM_ITEMS:
      case ATTRIB_NUM_STACKS:
      case ATTRIB_ADD_ITEM:
      case ATTRIB_REMOVE_ITEM:
      case ATTRIB_MOVE_ITEM:
      case ATTRIB_CLEAR_CURRENT_ITEMS:
      case ATTRIB_CLEAR_ITEMS:
         return true;
      case ATTRIB_CURRENT_ITEM:
         gotoItem( VM::toInt( vm, 3 )-1 );
         return true;
      case ATTRIB_ON_ITEM_SELECT:
         VM::toRef( vm, 3, _onItemSelectRef );
         return true;
      case ATTRIB_OFFSET:
         _offset = VM::toFloat( vm, 3 );
         updateEntities();
      case ATTRIB_POP_ITEMS:
      case ATTRIB_PUSH_ITEMS:
         return true;
      case ATTRIB_ROTATION_SPEED:
         _rotationSpeed = VM::toFloat( vm, 3 );
         return true;
      case ATTRIB_SET_ITEMS:
         return true;
      default: break;
   }

   return Widget::performSet( vm );
}

//------------------------------------------------------------------------------
//!
bool
PlasmaBrowser::isAttribute( const char* name ) const
{
   if( _attributes[ name ] != StringMap::INVALID ) return true;
   return Widget::isAttribute( name );
}

NAMESPACE_END
