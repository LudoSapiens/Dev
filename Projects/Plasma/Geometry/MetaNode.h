/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_META_NODE_H
#define PLASMA_META_NODE_H

#include <Plasma/StdDefs.h>

#include <CGMath/AABBox.h>

#include <Base/ADT/Vector.h>
#include <Base/ADT/Set.h>
#include <Base/ADT/Pair.h>
#include <Base/Util/RCObject.h>

NAMESPACE_BEGIN

// Forward declaration.
class MetaBlock;
class MetaFunction;

enum { IN, OUT, BIN, BOUT };

/*==============================================================================
  CLASS MetaNode
==============================================================================*/
class MetaNode:
   public RCObject
{
public:

   /*----- Static methods -----*/

   enum Type
   {
      META_BLOCKS,
      META_COMPOSITE_BLOCKS,
      META_INPUT,
      META_TRANSFORM,
      META_UNION,
      META_INTERSECTION,
      META_DIFFERENCE,
      META_COMPOSITE,
   };

   /*----- methods -----*/

   // Getters.
   inline Type  type() const                 { return _type; }
   inline bool  isLeaf() const               { return (_type <= META_COMPOSITE_BLOCKS); }
   inline bool  isOperation() const          { return (_type >= META_UNION); }
   inline MetaNode* parent() const           { return _parent; }
   inline const AABBoxf& boundingBox() const { return _box; }
   inline int layer() const                  { return _layer; }

   // Do not use.
   inline void parent( MetaNode* node )          { _parent = node; }
   inline void boundingBox( const AABBoxf& box ) { _box = box; }

   // Debug.
   void print( TextStream& os = StdErr ) const;
   virtual void print( TextStream&, const String& tab ) const;

protected:

   /*----- friends -----*/

   friend class MetaBuilder;

   /*----- methods -----*/

   inline MetaNode( Type type ): _type(type), _parent(0), _layer(0) {}
   virtual ~MetaNode() {}

   inline void layer( int l ) { _layer = l; }

   /*----- data members -----*/

   Type      _type;  //!< The type of the node.
   MetaNode* _parent;
   int       _layer;
   AABBoxf   _box;
};

/*==============================================================================
  CLASS MetaBlocks
==============================================================================*/
class MetaBlocks:
   public MetaNode
{
public:

   /*----- types and enumerations ----*/

   typedef Vector<MetaBlock*>     BlockContainer;
   typedef Pair<ushort,ushort>    GroupPair;
   typedef Set<GroupPair>         AttractionContainer;

   /*----- methods -----*/

   // Getters.
   inline const BlockContainer&  blocks() const           { return _blocks; }
   inline uint  numBlocks() const                         { return (uint)_blocks.size(); }
   inline const MetaBlock*  block( uint id ) const        { return _blocks[id]; }
   inline const AttractionContainer&  attractions() const { return _attractions; }
   inline uint  numAttractions() const                    { return (uint)_attractions.size(); }

   inline       Mat4f&  transform()                       { return _transform; }
   inline const Mat4f&  transform() const                 { return _transform; }

   inline const MetaFunction* mapping() const             { return _mapping; }
   inline const MetaFunction* displacement() const        { return _displacement; }

   // Non const getters.
   inline MetaBlock*  block( uint id )                    { return _blocks[id]; }

   // Queries.
   bool areAttracted( uint grp1, uint grp2 ) {
      if( grp1 < grp2 ) return _attractions.has( GroupPair( grp1, grp2 ) );
      return _attractions.has( GroupPair( grp2, grp1 ) );
   }

   // Counter.
   void resetCount( uint countID );
   inline int count() const    { return _count; }
   inline int& count()         { return _count; }
   inline int countID() const  { return _countID; }

   inline uint currentState( uint countID ) const
   {
      if( countID != _countID ) return OUT;
      if( _count == 0 )
      {
         if( _frontB ) return BOUT;
         return OUT;
      }
      if( _count == -1 && _backB && !_frontB ) return BIN;
      return IN;
   }

   void addIntersection( float, bool );

   // Debug.
   void dump() const;

protected:

   /*----- friends -----*/

   friend class MetaBuilder;

   /*----- methods -----*/

   MetaBlocks() : MetaNode( META_BLOCKS ), _transform( Mat4f::identity() ), _mapping(0), _displacement(0) {}

   // Builder API.
   void add( MetaBlock* block ) { _blocks.pushBack( block ); }
   void add( uint grp1, uint grp2 ) {
      if( grp1 < grp2 )
      {
         _attractions.add( GroupPair( grp1, grp2 ) );
      }
      else
      {
         _attractions.add( GroupPair( grp2, grp1 ) );
      }
   }
   inline void transform( const Mat4f& tr )       { _transform = tr; }
   inline void mapping( MetaFunction* func )      { _mapping = func; }
   inline void displacement( MetaFunction* func ) { _displacement = func; }

   inline void clear()            { _blocks.clear(); _attractions.clear(); }
   inline void clearBlocks()      { _blocks.clear();                       }
   inline void clearAttractions() { _attractions.clear(); }

   virtual void print( TextStream&, const String& tab ) const;

   /*----- data members -----*/

   BlockContainer       _blocks;
   AttractionContainer  _attractions;
   Mat4f                _transform;
   MetaFunction*        _mapping;
   MetaFunction*        _displacement;

   // Intersection computation.
   int      _count;
   uint     _countID;
   bool     _frontB;
   bool     _backB;
};


/*==============================================================================
   CLASS MetaCompositeBlocks
==============================================================================*/

class MetaCompositeBlocks:
   public MetaBlocks
{
public:

   /*----- types -----*/

   typedef Vector< MetaBlocks* >  NodeContainer;

   /*----- methods -----*/

   inline const NodeContainer&  children() const    { return _children; }
   inline uint  numChildren() const                 { return (uint)_children.size(); }
   inline       MetaBlocks*  child( uint id )       { return _children[id]; }
   inline const MetaBlocks*  child( uint id ) const { return _children[id]; }
   inline const MetaBlocks*  operator[]( uint id )  { return _children[id]; }

   void update();

protected:

   /*----- friends -----*/

   friend class MetaBuilder;

   /*----- methods -----*/

   MetaCompositeBlocks() : MetaBlocks()
   {
      _type = META_COMPOSITE_BLOCKS;
   }
   inline void  add( MetaBlocks* node )    { _children.pushBack(node); node->parent( this ); }
   inline void  remove( MetaBlocks* node ) { _children.remove(node); node->parent(0); }

   virtual void print( TextStream&, const String& tab ) const;

   /*----- data members -----*/

   NodeContainer _children;
};

/*==============================================================================
   CLASS MetaInput
==============================================================================*/
class MetaInput:
   public MetaNode
{
public:

   /*----- methods -----*/

   inline       MetaNode* child()         { return _child; }
   inline const MetaNode* child() const   { return _child; }

protected:

   /*----- friends -----*/

   friend class MetaBuilder;
   friend class MetaComposite;

   /*----- methods -----*/

   MetaInput() : MetaNode( META_INPUT ), _child(0) {}

   inline void child( MetaNode* node ) { _child = node; node->parent( this ); }

   virtual void print( TextStream&, const String& tab ) const;

   /*----- data members -----*/

   MetaNode* _child;
};

/*==============================================================================
  CLASS MetaTransform
==============================================================================*/
class MetaTransform:
   public MetaNode
{
public:

   /*----- methods -----*/

   // Getters.
   inline       Mat4f&  transform()       { return _transform; }
   inline const Mat4f&  transform() const { return _transform; }
   inline       MetaNode* child()         { return _child; }
   inline const MetaNode* child() const   { return _child; }

protected:

   /*----- friends -----*/

   friend class MetaBuilder;

   /*----- methods -----*/

   MetaTransform() : MetaNode( META_TRANSFORM ), _transform( Mat4f::identity() ), _child(0) {}

   inline void  child( MetaNode* node )      { _child = node; node->parent( this ); }
   inline void  transform( const Mat4f& tr ) { _transform = tr; }

   virtual void print( TextStream&, const String& tab ) const;

   /*----- data members -----*/

   Mat4f      _transform;
   MetaNode*  _child;      //!< The node onto which the transform is applied.
};


/*==============================================================================
  CLASS MetaOperation
==============================================================================*/
class MetaOperation:
   public MetaNode
{
public:

   /*----- types -----*/

   typedef Vector< MetaNode* >  NodeContainer;

   /*----- methods -----*/

   // Getters.
   inline const NodeContainer&  children() const  { return _children; }
   inline uint  numChildren() const               { return (uint)_children.size(); }
   inline       MetaNode*  child( uint id )       { return _children[id]; }
   inline const MetaNode*  child( uint id ) const { return _children[id]; }
   inline const MetaNode*  operator[]( uint id )  { return _children[id]; }

protected:

   /*----- friends -----*/

   friend class MetaBuilder;

   /*----- methods -----*/

   inline MetaOperation( Type type ): MetaNode( type ) {}

   inline void  add( MetaNode* node )    { _children.pushBack(node); node->parent( this ); }
   inline void  remove( MetaNode* node ) { _children.remove(node); node->parent(0); }

   virtual void print( TextStream&, const String& tab ) const;

   /*----- data members -----*/

   NodeContainer  _children;
};


/*==============================================================================
  CLASS MetaUnion
==============================================================================*/
class MetaUnion:
   public MetaOperation
{
protected:
   
   /*----- friends -----*/

   friend class MetaBuilder;
   friend class MetaComposite;

   /*----- methods -----*/

   MetaUnion(): MetaOperation( META_UNION ) { }
};

/*==============================================================================
  CLASS MetaIntersection
==============================================================================*/
class MetaIntersection:
   public MetaOperation
{
protected:

   /*----- friends -----*/

   friend class MetaBuilder;

   /*----- methods -----*/

   MetaIntersection(): MetaOperation( META_INTERSECTION ) { }
};

/*==============================================================================
  CLASS MetaDifference
==============================================================================*/
class MetaDifference:
   public MetaOperation
{
protected:

   /*----- friends -----*/

   friend class MetaBuilder;

   /*----- methods -----*/

   MetaDifference(): MetaOperation( META_DIFFERENCE ) { }
};

/*==============================================================================
   CLASS MetaComposite
==============================================================================*/
class MetaComposite:
   public MetaOperation
{
public:

   /*----- methods -----*/

   inline       MetaNode* mainChild()        { return _mainChild; }
   inline const MetaNode* mainChild() const  { return _mainChild; }
   void connectComposites();

   inline       MetaInput* input()           { return _input; }
   inline const MetaInput* input() const     { return _input; }

   inline       Mat4f&  transform()          { return _transform; }
   inline const Mat4f&  transform() const    { return _transform; }

protected:

   /*----- friends -----*/

   friend class MetaBuilder;

   /*----- methods -----*/

   MetaComposite() : MetaOperation( META_COMPOSITE ), _transform( Mat4f::identity() ), _mainChild(0), _input(0)
   {
   }

   inline void transform( const Mat4f& tr ) { _transform = tr; }
   inline void input( MetaInput* input )    { _input = input; }

   virtual void print( TextStream&, const String& tab ) const;

   /*----- data members -----*/

   Mat4f                   _transform;
   MetaNode*               _mainChild;
   MetaInput*              _input;
   Vector< RCP<MetaNode> > _interiorNodes;
};

NAMESPACE_END

#endif //PLASMA_META_NODE_H
