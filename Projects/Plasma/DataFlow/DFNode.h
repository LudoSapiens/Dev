/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_DFNODE_H
#define PLASMA_DFNODE_H

#include <Plasma/StdDefs.h>

#include <Fusion/VM/VM.h>

#include <CGMath/Vec2.h>

#include <Base/ADT/ConstString.h>
#include <Base/ADT/String.h>
#include <Base/ADT/Vector.h>
#include <Base/ADT/Set.h>
#include <Base/IO/StreamIndent.h>
#include <Base/Util/RCObject.h>

NAMESPACE_BEGIN

class DFGraph;
class DFInput;
class DFNode;
class DFNodeAttrList;
class DFNodeAttrStates;
class DFNodeSpec;
class Manipulator;

/*==============================================================================
   CLASS DFSocket
==============================================================================*/

class DFSocket
{
public:

   /*----- types and enumerations -----*/

   enum Type {
      FLOAT,
      VEC2,
      VEC3,
      VEC4,
      SOUND,
      IMAGE,
      GEOMETRY,
      ANIMATION,
      WORLD,
      STROKES,
      POLYGON
   };

   /*----- static methods -----*/

   static PLASMA_DLL_API const char* toStr( Type type );

   /*----- methods -----*/

   PLASMA_DLL_API virtual Type type() const = 0;
   inline const char* typeAsStr() const { return toStr(type()); }
};

/*==============================================================================
   CLASS DFOutput
==============================================================================*/

class DFOutput:
   public DFSocket
{
public:

   inline bool isConnected() const               { return !_inputs.empty(); }
   inline uint numInputs() const                 { return uint(_inputs.size()); }
   inline const Vector<DFInput*>& inputs() const { return _inputs; }

protected:

   /*----- friends -----*/

   friend class DFGraph;
   friend class DFInput;

   /*----- methods -----*/

   void connect( DFInput* input )    { _inputs.pushBack( input ); }
   void disconnect( DFInput* input ) { _inputs.removeSwap( input ); }

   /*----- data members -----*/

   Vector<DFInput*> _inputs;
};

/*==============================================================================
   CLASS DFInput
==============================================================================*/

class DFInput:
   public DFSocket
{
public:

   DFInput( DFNode* n ): _node( n ) {}

   DFNode* node() const { return _node; }

   virtual bool isConnected() const = 0;
   virtual bool isMulti() const { return false; }

protected:

   /*----- friends -----*/

   friend class DFGraph;

   /*----- methods -----*/

   virtual void connect( DFOutput* ) = 0;
   virtual void disconnect( DFOutput* ) = 0;
   virtual void disconnect() = 0;

   void disconnectFrom( DFOutput* output ) { output->disconnect( this ); }

   /*----- data members -----*/

   DFNode* _node;
};


/*==============================================================================
  CLASS DFNodeEditor
==============================================================================*/
//! An editing context for a particular edit node.
//! This is meant to be subclassed in order to offer a more complete
//! interface.
class DFNodeEditor:
   public RCObject
{
public:

   /*----- methods -----*/

   PLASMA_DLL_API DFNodeEditor();
   PLASMA_DLL_API virtual ~DFNodeEditor();

   // UI.
   PLASMA_DLL_API virtual RCP<Manipulator> manipulator();
   PLASMA_DLL_API virtual RCP<DFNodeAttrList> attributes() const;
   PLASMA_DLL_API virtual RCP<DFNodeAttrStates> attributesStates() const;
   PLASMA_DLL_API virtual void updateAttributes( const DFNodeAttrStates& );

}; //class DFNodeEditor


/*==============================================================================
   CLASS DFNode
==============================================================================*/

class DFNode:
   public RCObject
{
public:

   /*----- methods -----*/

   PLASMA_DLL_API DFNode();

   DFGraph* graph() const      { return _graph; }
   DFSocket::Type type() const { return output()->type(); }

   // Attributes.
   PLASMA_DLL_API virtual const ConstString& name() const = 0;
   PLASMA_DLL_API const String& label() const;
   PLASMA_DLL_API const DFNodeSpec& spec() const;

   PLASMA_DLL_API virtual bool isGraph() const;

   // Input/output.
   PLASMA_DLL_API virtual uint numInputs() const;

   PLASMA_DLL_API virtual DFOutput* output();
   PLASMA_DLL_API virtual DFInput* input( uint );

   const DFOutput* output() const       { return const_cast<DFNode*>(this)->output(); }
   const DFInput* input( uint i ) const { return const_cast<DFNode*>(this)->input(i); }

   PLASMA_DLL_API bool hasConnectedInput();
   PLASMA_DLL_API bool hasConnectedOutput();

   // Editor.
   PLASMA_DLL_API virtual RCP<DFNodeEditor>  edit();

   // Appearence.
   const Vec2i& position() const { return _position; }
   int width() const             { return _width; }

   // Dumping.
   PLASMA_DLL_API         bool  dump      ( TextStream& os, StreamIndent& indent ) const;
   PLASMA_DLL_API         bool  dumpBegin ( TextStream& os, StreamIndent& indent ) const;
   PLASMA_DLL_API virtual bool  dumpCustom( TextStream& os, StreamIndent& indent ) const;
   PLASMA_DLL_API         bool  dumpEnd   ( TextStream& os, StreamIndent& indent ) const;

protected:

   /*----- friends -----*/

   friend class DFGraph;

   /*----- data members -----*/

   DFGraph* _graph;
   Vec2i    _position;
   int      _width;
};


/*==============================================================================
  CLASS DFNodeSpec
==============================================================================*/
class DFNodeSpec:
   public RCObject
{
public:

   /*----- types -----*/

   typedef RCP<DFNode>  (*CreateVMFunc)( VMState* vm, int idx );

   /*----- static methods -----*/

   PLASMA_DLL_API static const DFNodeSpec*  get( const ConstString& name );
   PLASMA_DLL_API static void  getAll( Vector<const DFNodeSpec*>& dst );

   PLASMA_DLL_API static bool  registerNode(
      DFSocket::Type     type,
      const ConstString& name,
            CreateVMFunc createFunc,
      const      String& label,
      const      String& info,
      const      String& icon
   );
   PLASMA_DLL_API static bool  unregisterNode( const ConstString& name );
   PLASMA_DLL_API static void  unregisterAll();

   /*----- methods -----*/

   inline const char*         typeName() const { return DFSocket::toStr( _type ); }
   inline DFSocket::Type      type()     const { return _type;  }
   inline const ConstString&  name()     const { return _name;  }
   inline      const String&  label()    const { return _label; }
   inline      const String&  info()     const { return _info;  }
   inline      const String&  icon()     const { return _icon;  }

   inline        RCP<DFNode>  create( VMState* vm, int idx ) const { return _createFunc( vm, idx ); }

protected:

   /*----- data members -----*/

   DFSocket::Type _type;       //!< Output type.
   ConstString    _name;       //!< A unique type name.
   CreateVMFunc   _createFunc; //!< A creation callback routine.
   String         _label;      //!< The node's label.
   String         _info;       //!< An information string (optional).
   String         _icon;       //!< The resource ID of an icon (optional).

}; //class DFNodeSpec

/*==============================================================================
   CLASS DFSelection
==============================================================================*/
template< typename T >
class DFSelection
{
public:

   /*----- types -----*/

   typedef Set<T>                                  EntryContainer;
   typedef typename EntryContainer::ConstIterator  ConstIterator;

   /*----- methods -----*/

   inline DFSelection()                   {}
   inline void clear()                    { _entries.clear(); _last = T(); }

   inline bool empty()      const         { return _entries.empty(); }
   inline uint numEntries() const         { return uint(_entries.size());  }

   inline const T& last() const           { return _last; }

   inline ConstIterator begin()  const    { return _entries.begin(); }
   inline ConstIterator end()    const    { return _entries.end();   }

   inline bool has( const T& p ) const    { return _entries.has(p); }
   inline void add( const T& p )          { _entries.add(p);  _last = p; }
   inline void set( const T& p )          { clear(); add(p);  _last = p; }
   inline void remove( const T& p )
   {
      _entries.remove(p);
      if( p == _last )
      {
         // Choose one arbitrarily.
         if( _entries.empty() ) _last = T();
         else
            _last = *begin();
      }
   }

   inline bool toggle( const T& p )
   {
      if( _entries.has(p) )
      {
         remove(p);
         return false;
      }
      else
      {
         add(p);
         return true;
      }
   }

protected:

   /*----- data members -----*/

   T               _last;
   EntryContainer  _entries;
};

NAMESPACE_END

#endif
