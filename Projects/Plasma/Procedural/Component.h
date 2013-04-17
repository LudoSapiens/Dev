/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_COMPONENT
#define PLASMA_COMPONENT

#include <Plasma/StdDefs.h>
#include <Plasma/Procedural/Boundary.h>

#include <Fusion/VM/VM.h>

#include <CGMath/Variant.h>
#include <CGMath/Random.h>

#include <Base/Util/RCObject.h>
#include <Base/ADT/ConstString.h>
#include <Base/ADT/Pair.h>

NAMESPACE_BEGIN

class Component;
class MetaNode;
class MetaBuilder;

/*==============================================================================
   CLASS ComponentConstraint
==============================================================================*/

class ComponentConstraint
{
public:
   /*----- enumeration  -----*/

   enum
   {
      PLANE   = 0,
      VOLUME  = 1,
      POLYGON = 2,
      REPULSE = 16
   };

   /*----- methods -----*/

   ComponentConstraint( int type, Component* c, int face = -1 ) : 
      _comp( c ), _face( face ), _type( type ), _offset(0.0f) {}

   inline void offset( const Vec3f& off ) { _offset = off; }
   inline const Vec3f& offset() const     { return _offset; }
   inline int type() const                { return _type; }
   inline Component* component() const    { return _comp; }
   inline int face() const                { return _face; }

private:

   /*----- members -----*/

   Component* _comp;
   int        _face;
   int        _type;
   Vec3f      _offset;
};

/*==============================================================================
   CLASS Region
==============================================================================*/

class Region:
   public RCObject,
   public VMProxy
{
public:

   /*----- methods -----*/

   Region() :
      _eulerQuat( Quatf::identity() ),
      _box( Vec3f::zero() ),
      _ref( Reff::identity() )
   {
      _oriRange[0] = Vec2f(0.0f);
      _oriRange[1] = Vec2f(0.0f);
      _oriRange[2] = Vec2f(0.0f);
   }

   Component* component() const                                                { return _comp; }
   void component( Component* comp )                                           { _comp = comp; }

   // Attributes.
   inline const ConstString& id() const                                        { return _id; }
   inline void id( const ConstString& name )                                   { _id = name; }
   inline void id( const char* name )                                          { _id = name; }

   inline const Reff& referential() const                                      { return _ref; }
   inline void referential( const Reff& ref )                                  { _ref = ref; }

   // Position.
   inline const AABBoxf& positionRange() const                                 { return _box; }
   inline const Vec2f& positionRange( uint axis ) const                        { return _box.slab( axis ); }
   inline void positionRange( const AABBoxf& box )                             { _box = box; }
   inline void positionRange( uint axis, const Vec2f& range )                  { _box.slab(axis) = range; }
   inline void positionRange( const Vec2f& x, const Vec2f& y, const Vec2f& z ) { _box.set( x, y, z ); }

   // Orientation.
   inline const Vec2f& orientationRange( uint axis ) const                     { return _oriRange[axis]; }
   inline void orientationRange( uint axis, const Vec2f& r )                   { _oriRange[axis] = r; }
   inline void orientationRange( const Vec2f& x, const Vec2f& y, const Vec2f& z )
   {
      _oriRange[0] = x;
      _oriRange[1] = y;
      _oriRange[2] = z;
   }

   inline const Quatf& orientationTransform() const                            { return _eulerQuat; }
   inline void orientationTransform( const Quatf& orient )                     { _eulerQuat = orient; }

   Reff connection( RNG_WELL& ) const;
   Reff connection( RNG_WELL&, const Vec3f& pos ) const;
   Reff connection( const Vec3f& pos ) const;
   Reff connection( const Vec3f& pos, const Quatf& orient ) const;

   // VM.
   virtual const char* meta() const;

protected:

   /*----- members  -----*/

   ConstString _id;

   // Restriction on orientation.
   Vec2f       _oriRange[3]; // Radians [min, max]
   Quatf       _eulerQuat;

   // Restriction on position.
   AABBoxf     _box;
   Reff        _ref;

   Component*  _comp;
};

/*==============================================================================
   CLASS Connector
==============================================================================*/

class Connector
{
public:

   /*----- methods -----*/

   Connector() : _ref( Reff::identity() ) {}

   void referential( const Reff& ref ) { _ref = ref; }
   const Reff& referential() const     { return _ref; }

protected:

   /*----- members -----*/

   Reff    _ref;
   //Region* _region;
};

/*==============================================================================
   CLASS Component
==============================================================================*/
//! The boundary definition of a component is one of the following:
// 2D:
// -without boundary
// -with boundary, "dirty" and a face number (!=-1)
// -with boundary, "dirty" and no face number (==-1)
// -with boundary, not "dirty" and a face number
// -with boundary, not "dirty" and no face number
// 3D:
// -without boundary
// -with boundary and "dirty"
// -with boundary and not "dirty"
class Component:
   public RCObject,
   public VMProxy
{
public:

   /*----- static methods -----*/

   static void initialize();

   /*----- methods -----*/

   // Ids.
   void addId( const char* );
   void addId( const Vector<ConstString>& );
   void removeId( const char* );
   const ConstString& id( uint i ) const  { return _ids[i]; }
   const Vector<ConstString>& ids() const { return _ids; }

   void referential( const Reff& ref )    { _ref = ref; }
   const Reff& referential() const        { return _ref; }
   void position( const Vec3f& pos )      { _ref.position( pos ); }
   const Vec3f& position() const          { return _ref.position(); }

   const Connector& connector() const     { return _out; }
   Connector& connector()                 { return _out; }

   // Parenting.
   Component* parent() const              { return _parent; }
   Component* children() const            { return _children; }
   Component* sibling() const             { return _sibling; }
   void addComponent( Component* );
   void removeComponent( Component* );
   void inherit( Component* parent, const Reff& ref, const Vec3f& size );
   bool isParent( Component* );

   // Boundary.
   void resolveBoundary();
   void boundary( Boundary* );
   void boundary( int face );
   Boundary* boundary() const             { return _boundary.ptr(); }
   int dimension() const                  { return _dimension; }
   int face() const                       { return _face; }
   const Vec3f& size() const              { return _size; }
   void size( const Vec3f& val )          { _size = val; _dimension = 3; }
   void size( const Vec2f& val )          { _size = Vec3f( val, 0.0f ); _dimension = 2; }

   // Region.
   Region* createRegion();
   void removeRegion( Region* );
   void removeAllRegions();
   uint numRegions() const                { return uint(_regions.size()); }
   Region* region( uint i ) const         { return _regions[i].ptr(); }

   // Attributes.
   const Variant& getAttribute( const ConstString& key ) const;
   const Variant& getLocalAttribute( const ConstString& key ) const;
   void removeAttribute( const ConstString& key )                    { if( _attributes.isValid() ) _attributes->remove( key ); }
   void clearAttributes()                                            { _attributes = 0; }
   void setAttribute( const ConstString& key, bool v )               { initTable(); _attributes->set( key, v ); }
   void setAttribute( const ConstString& key, float v )              { initTable(); _attributes->set( key, v ); }
   void setAttribute( const ConstString& key, const char* v )        { initTable(); _attributes->set( key, ConstString(v) ); }
   void setAttribute( const ConstString& key, const ConstString& v ) { initTable(); _attributes->set( key, v ); }
   void setAttribute( const ConstString& key, const Vec2f& v )       { initTable(); _attributes->set( key, v ); }
   void setAttribute( const ConstString& key, const Vec3f& v )       { initTable(); _attributes->set( key, v ); }
   void setAttribute( const ConstString& key, const Vec4f& v )       { initTable(); _attributes->set( key, v ); }
   void setAttribute( const ConstString& key, Table* v )             { initTable(); _attributes->set( key, v ); }

   // VM.
   virtual const char* meta() const;

   // Debugging.
   void dump() const;

protected:

   /*----- friends -----*/

   friend class Compositor;

   /*----- methods -----*/

   Component();
   virtual ~Component();

   void initTable() { if( _attributes.isNull() ) _attributes = new Table(); }

   void transform( const Reff& );

   /*----- members -----*/

   Component*            _parent;
   Component*            _children;
   Component*            _sibling;

   Reff                  _ref;
   Connector             _out;

   Vector<ConstString>   _ids;
   Vector< RCP<Region> > _regions;
   RCP<Table>            _attributes;

   // Boundary.
   RCP<Boundary>         _boundary;
   Vec3f                 _size;
   char                  _dimension;
   int16_t               _face;
   bool                  _bdirty;
};


/*==============================================================================
   CLASS Compositor
==============================================================================*/

class Compositor:
   public RCObject
{
public:

   /*----- classes -----*/

   class Iterator
   {
   public:

      /*----- method -----*/

      Iterator( Compositor* );

      void operator++();
      bool valid() const;
      Component* operator*();

   private:

      Compositor*        _compositor;
      uint               _ipos;
      Component*         _comp;
      Vector<Component*> _stack;
   };


   /*----- methods -----*/

   Compositor();

   // Creation.
   Component* createComponent();
   void removeComponent( Component* );
   
   // Operations.
   void connect( Component* from, Component* to );
   void connect( Component* from, Region* to );
   void connect( Component* from, Region* to, const Reff& );
   void disconnect( Component* );
   void move( Component*, const Vec3f& );
   void move( Component*, const Reff& );

   void bind( MetaNode* from, Component* to );

   void updateTransform( MetaBuilder& );

   // Components.
   Iterator iterator()                  { return Iterator(this); }
   uint numComponents() const           { return uint(_components.size()); }
   Component* component( uint i ) const { return _components[i].ptr(); }

   // Query/iterator scope.
   void scope( Component* comp )        { _scopes.pushBack( comp ); }
   Component* scope() const             { return _scopes.empty() ? 0 : _scopes.back(); }
   void unscope()                       { _scopes.popBack(); }

   // Random.
   inline RNG_WELL&  rng() { return _rng; }

protected:

   /*----- methods -----*/

   virtual ~Compositor();

   /*----- members -----*/

   Vector< RCP<Component> >              _components;
   Vector< Component* >                  _scopes;
   Vector< Pair<Component*, MetaNode*> > _geomConnections;
   RNG_WELL                              _rng;
};

NAMESPACE_END

#endif

