/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_DFANIMNODE_H
#define PLASMA_DFANIMNODE_H

#include <Plasma/StdDefs.h>
#include <Plasma/DataFlow/DFNode.h>
#include <Plasma/Animation/SkeletalAnimation.h>

#include <Base/Msg/Delegate.h>

NAMESPACE_BEGIN

/*==============================================================================
   STATIC INITIALIZATION
==============================================================================*/

PLASMA_DLL_API void initializeAnimNodes();
PLASMA_DLL_API void terminateAnimNodes();

/*==============================================================================
   CLASS DFAnimOutput
==============================================================================*/

class DFAnimOutput:
   public DFOutput
{
public:

   /*----- types -----*/

   typedef Delegate0< RCP<SkeletalAnimation> > ProcessDelegate;

   /*----- methods -----*/

   PLASMA_DLL_API RCP<SkeletalAnimation> getAnimation();
   PLASMA_DLL_API virtual Type type() const;

   void delegate( const ProcessDelegate& d ) { _delegate = d; }

protected:

   /*----- data members -----*/

   ProcessDelegate _delegate;
};

/*==============================================================================
   CLASS DFAnimInput
==============================================================================*/

class DFAnimInput:
   public DFInput
{
public:

   /*----- methods -----*/

   DFAnimInput( DFNode* n ): DFInput(n), _output(nullptr) {}

   PLASMA_DLL_API RCP<SkeletalAnimation> getAnimation();
   PLASMA_DLL_API virtual Type type() const;
   PLASMA_DLL_API virtual bool isConnected() const;

protected:

   /*----- methods -----*/

   virtual void connect( DFOutput* );
   virtual void disconnect( DFOutput* );
   virtual void disconnect();

   /*----- data members -----*/

   DFAnimOutput* _output;
};

/*==============================================================================
   CLASS DFAnimMultiInput
==============================================================================*/

class DFAnimMultiInput:
   public DFInput
{
public:

   /*----- methods -----*/

   DFAnimMultiInput( DFNode* n ): DFInput(n) {}
   PLASMA_DLL_API RCP<SkeletalAnimation> getAnimation( uint );

   inline size_t size() const { return _outputs.size(); }

   PLASMA_DLL_API virtual Type type() const;
   PLASMA_DLL_API virtual bool isConnected() const;
   PLASMA_DLL_API virtual bool isMulti() const { return true; }

protected:

   /*----- methods -----*/

   virtual void connect( DFOutput* );
   virtual void disconnect( DFOutput* );
   virtual void disconnect();

   /*----- data members -----*/

   Vector<DFAnimOutput*> _outputs;
};

/*==============================================================================
   CLASS DFImportAnimNode
==============================================================================*/

class DFImportAnimNode:
   public DFNode
{
public:

   /*----- methods -----*/

   PLASMA_DLL_API DFImportAnimNode();

   // Attributes.
   PLASMA_DLL_API virtual const ConstString& name() const;

   // Input/output.
   PLASMA_DLL_API virtual uint numInputs() const;

   PLASMA_DLL_API virtual DFOutput* output();
   PLASMA_DLL_API virtual DFInput* input( uint );

   inline const String& animationID() const { return _animID; }
   PLASMA_DLL_API void animationID( const String& );

   // Editing.
   PLASMA_DLL_API virtual RCP<DFNodeEditor> edit();

   // Dump.
   PLASMA_DLL_API virtual bool  dumpCustom( TextStream& os, StreamIndent& indent ) const;

protected:

   /*----- methods -----*/

   RCP<SkeletalAnimation> process();

   /*----- data members -----*/

   DFAnimOutput           _output;
   RCP<DFNodeEditor>      _editor;
   RCP<SkeletalAnimation> _anim;
   String                 _animID;
};

/*==============================================================================
   CLASS DFCycleAnimNode
==============================================================================*/

class DFCycleAnimNode:
   public DFNode
{
public:

   /*----- methods -----*/

   PLASMA_DLL_API DFCycleAnimNode();

   // Attributes.
   PLASMA_DLL_API virtual const ConstString& name() const;

   // Input/output.
   PLASMA_DLL_API virtual uint numInputs() const;

   PLASMA_DLL_API virtual DFOutput* output();
   PLASMA_DLL_API virtual DFInput* input( uint );

   inline float fraction() const   { return _fraction; }
   inline void fraction( float v ) { _fraction = v; }

   // Editing.
   PLASMA_DLL_API virtual RCP<DFNodeEditor> edit();

   // Dump.
   PLASMA_DLL_API virtual bool  dumpCustom( TextStream& os, StreamIndent& indent ) const;

protected:

   /*----- methods -----*/

   RCP<SkeletalAnimation> process();

   /*----- data members -----*/

   DFAnimOutput      _output;
   DFAnimInput       _input;
   RCP<DFNodeEditor> _editor;
   float             _fraction;
};

/*==============================================================================
   CLASS DFResampleAnimNode
==============================================================================*/

class DFResampleAnimNode:
   public DFNode
{
public:

   /*----- methods -----*/

   PLASMA_DLL_API DFResampleAnimNode();

   // Attributes.
   PLASMA_DLL_API virtual const ConstString& name() const;

   // Input/output.
   PLASMA_DLL_API virtual uint numInputs() const;

   PLASMA_DLL_API virtual DFOutput* output();
   PLASMA_DLL_API virtual DFInput* input( uint );

   inline float rate() const  { return _rate; }
   inline void rate( float r) { _rate = r; }

   // Editing.
   PLASMA_DLL_API virtual RCP<DFNodeEditor> edit();

   // Dump.
   PLASMA_DLL_API virtual bool  dumpCustom( TextStream& os, StreamIndent& indent ) const;

protected:

   /*----- methods -----*/

   RCP<SkeletalAnimation> process();

   /*----- data members -----*/

   DFAnimOutput      _output;
   DFAnimInput       _input;
   RCP<DFNodeEditor> _editor;
   float             _rate;
};

/*==============================================================================
   CLASS DFTrimAnimNode
==============================================================================*/

class DFTrimAnimNode:
   public DFNode
{
public:

   /*----- methods -----*/

   PLASMA_DLL_API DFTrimAnimNode();

   // Attributes.
   PLASMA_DLL_API virtual const ConstString& name() const;

   // Input/output.
   PLASMA_DLL_API virtual uint numInputs() const;

   PLASMA_DLL_API virtual DFOutput* output();
   PLASMA_DLL_API virtual DFInput* input( uint );

   inline const Vec2i& range() const   { return _range; }
   inline void range( const Vec2i& r ) { _range = CGM::max( r, Vec2i(0) ); }

   // Editing.
   PLASMA_DLL_API virtual RCP<DFNodeEditor> edit();

   // Dump.
   PLASMA_DLL_API virtual bool  dumpCustom( TextStream& os, StreamIndent& indent ) const;

protected:

   /*----- methods -----*/

   RCP<SkeletalAnimation> process();

   /*----- data members -----*/

   DFAnimOutput      _output;
   DFAnimInput       _input;
   RCP<DFNodeEditor> _editor;
   Vec2i             _range;
};

NAMESPACE_END

#endif
