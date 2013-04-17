/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_PLASMABROWSER_H
#define PLASMA_PLASMABROWSER_H

#include <Plasma/StdDefs.h>

#include <Plasma/World/World.h>
#include <Plasma/World/Viewport.h>

#include <Fusion/Widget/Widget.h>

NAMESPACE_BEGIN

class Geometry;
class Renderer;
class BrowserAnimator;
class PlasmaBrowser;

/*==============================================================================
   CLASS BrowserItem
==============================================================================*/

class BrowserItem:
   public RCObject,
   public VMProxy
{
public:

   /*----- methods -----*/

   static void initialize();

   /*----- methods -----*/

   BrowserItem();

   float scale() const             { return _scale; }
   const String& geometry() const  { return _geomName; }
   const String& material() const  { return _materialsName; }
   Widget* widget() const          { return _widget.ptr(); }

   void scale( float );
   void geometry( const String& );
   void material( const String& );
   void widget( Widget* );

   // VM.
   PLASMA_DLL_API virtual const char*  meta() const;
   PLASMA_DLL_API void init( VMState* );
   PLASMA_DLL_API bool performGet( VMState* );
   PLASMA_DLL_API bool performSet( VMState* );

protected:

   /*----- friends -----*/

   friend class PlasmaBrowser;

   /*----- methods -----*/

   virtual ~BrowserItem();
   bool isAttribute( const char* ) const;

   void prepareEntity( Entity*, PlasmaBrowser* );
   void releaseEntity();
   void geometryCb( Resource<Geometry>* );
   void materialCb( Resource<MaterialSet>* );

   /*----- data members -----*/

   PlasmaBrowser*               _browser;
   float                        _scale;
   RCP<Widget>                  _widget;
   RCP<Entity>                  _entity;
   RCP< Resource<MaterialSet> > _materials;
   RCP< Resource<Geometry> >    _geom;
   RCP<Table>                   _mparams;
   RCP<Table>                   _gparams;
   String                       _geomName;
   String                       _materialsName;
   VMRef                        _userAttributes;
};

/*==============================================================================
   STRUCT BrowserList
==============================================================================*/

struct BrowserList:
   public RCObject
{
   /*----- data members -----*/

   float                      _index;
   Vector< RCP<BrowserItem> > _items;
};

/*==============================================================================
   CLASS PlasmaBrowser
==============================================================================*/

class PlasmaBrowser:
   public Widget
{
public:

   /*----- types and enumerations -----*/

   enum {
      FLOW,
      LIST,
      GRID
   };
   typedef Delegate1<int> ItemDelegate;

   /*----- static methods -----*/

   static void initialize();

   /*----- methods -----*/

   PLASMA_DLL_API PlasmaBrowser();

   int mode() const { return _mode; }
   PLASMA_DLL_API void mode( int );

   Vector< RCP<BrowserItem> >& items() { return _lists.back()->_items; }
   uint numItems()                     { return int(items().size()); }
   uint numStacks()                    { return uint(_lists.size()); }
   BrowserItem* item( uint i )         { return items()[i].ptr(); }

   // Items.
   PLASMA_DLL_API void addItem( const RCP<BrowserItem>& );
   PLASMA_DLL_API void removeItem( const RCP<BrowserItem>& );
   PLASMA_DLL_API void removeItem( uint i );
   PLASMA_DLL_API void moveItem( uint fromIdx, uint toIdx );
   PLASMA_DLL_API void clearCurrentItems();
   PLASMA_DLL_API void clearItems();
   PLASMA_DLL_API void setItems( Vector< RCP<BrowserItem> >& );
   PLASMA_DLL_API void pushItems( Vector< RCP<BrowserItem> >& );
   PLASMA_DLL_API void popItems();

   inline uint currentItem() const { return uint(_index+0.5f); }
   PLASMA_DLL_API void gotoItem( int );

   // Callbacks.
   PLASMA_DLL_API void addOnItemSelect( const ItemDelegate& );
   PLASMA_DLL_API void removeOnItemSelect( const ItemDelegate& );

   // VM.
   const char* meta() const;
   PLASMA_DLL_API void init( VMState* );
   PLASMA_DLL_API bool performGet( VMState* );
   PLASMA_DLL_API bool performSet( VMState* );

protected:

   /*----- friends -----*/

   friend class BrowserAnimator;
   friend class BrowserItem;

   /*----- methods -----*/

   virtual ~PlasmaBrowser();

   virtual void onPointerPress( const Event& );
   virtual void onPointerRelease( const Event& );
   virtual void onPointerMove( const Event& );
   virtual void onPointerScroll( const Event& );

   virtual void onItemSelect( int i );
   virtual void render( const RCP<Gfx::RenderNode>& );
   virtual void performSetGeometry();
   virtual void performSetPosition();
   virtual bool isAttribute( const char* ) const;

   void createWorld();
   void updateWorld();
   void updateEntity( Entity*, float scale );
   void updateEntities();
   void releaseItems();

   void geometryReadyCb( Resource<Geometry>* );
   void materialSetReadyCb( Resource<MaterialSet>* );

   /*----- data members -----*/

   int                          _mode;

   float                        _index;
   float                        _targetIndex;
   float                        _speed;
   float                        _currentSpeed;
   float                        _rotationSpeed;
   float                        _offset;

   Viewport                     _viewport;
   RCP< Resource<Geometry> >    _geom;
   RCP< Resource<MaterialSet> > _materials;
   RCP<World>                   _world;
   RCP<Entity>                  _background;
   RCP<Renderer>                _renderer;
   RCP<BrowserAnimator>         _animator;
   Vector< RCP<BrowserList> >   _lists;
   Vector< Entity* >            _entities;

   Delegate1List< int >         _onItemSelect;
   VMRef                        _onItemSelectRef;
};

NAMESPACE_END

#endif
