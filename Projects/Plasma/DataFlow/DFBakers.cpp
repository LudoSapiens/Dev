/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/DataFlow/DFBakers.h>

#include <Plasma/DataFlow/DFGraph.h>
#include <Plasma/DataFlow/DFNodeAttr.h>
#include <Plasma/Manipulator/Manipulator.h>
#include <Plasma/Render/PlasmaBaker.h>

#include <Fusion/Resource/Image.h>
#include <Fusion/VM/VMFmt.h>

#include <Base/IO/FileSystem.h>

#if _MSC_VER
// 'this' used in member initializer list.
#pragma warning( disable: 4355 )
#endif

USING_NAMESPACE

/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN

ConstString _name_probeBaker;

//------------------------------------------------------------------------------
//!
enum
{
   ID_BAKE,
};

//------------------------------------------------------------------------------
//!
RCP<DFNode> probeBakerVM( VMState* /*vm*/, int /*idx*/ )
{
   RCP<DFProbeBaker> node = new DFProbeBaker();
   return node;
}

UNNAMESPACE_END

NAMESPACE_BEGIN

//------------------------------------------------------------------------------
//!
void initializeBakers()
{
   _name_probeBaker = "probeBaker";

   DFNodeSpec::registerNode(
      DFSocket::WORLD,
      _name_probeBaker, probeBakerVM,
      "Probe Baker", "A probe baker meta-node.",
      nullptr
   );
}

//------------------------------------------------------------------------------
//!
void terminateBakers()
{
   _name_probeBaker = ConstString();
}


/*==============================================================================
   CLASS DFProbeBakerEditor
==============================================================================*/

class DFProbeBakerEditor:
   public DFNodeEditor
{
public:

   /*----- methods -----*/

   DFProbeBakerEditor( DFProbeBaker* n ): _node(n) {}

   PLASMA_DLL_API virtual RCP<Manipulator> manipulator();
   PLASMA_DLL_API virtual RCP<DFNodeAttrList> attributes() const;
   PLASMA_DLL_API virtual RCP<DFNodeAttrStates> attributesStates() const;
   PLASMA_DLL_API virtual void updateAttributes( const DFNodeAttrStates& );

protected:

   /*----- methods -----*/

   void updateUI();

   /*----- data members -----*/

   DFProbeBaker*  _node;
   //RCP<ProbeRenderable> _renderable;
};

//------------------------------------------------------------------------------
//!
RCP<Manipulator>
DFProbeBakerEditor::manipulator()
{
   /**
   if( _renderable.isNull() )
   {
      _renderable = new ProbeRenderable();
      _renderable->addOnModify( makeDelegate( this, &DFProbeEditor::referentialCb ) );
      _renderable->update( _node->camera()->referential() );
   }
   return RCP<Manipulator>( new RefManipulator( _renderable.ptr() ) );
   **/
   return nullptr;
}


//------------------------------------------------------------------------------
//!
RCP<DFNodeAttrList>
DFProbeBakerEditor::attributes() const
{
   RCP<DFNodeAttrList> atts = new DFNodeAttrList();

   atts->add( DFNodeAttr( "BUTTON", ID_BAKE, "Bake" ) );

   return atts;
}

//------------------------------------------------------------------------------
//!
RCP<DFNodeAttrStates>
DFProbeBakerEditor::attributesStates() const
{
   RCP<DFNodeAttrStates> states = new DFNodeAttrStates();

   return states;
}

//------------------------------------------------------------------------------
//!
void
DFProbeBakerEditor::updateAttributes( const DFNodeAttrStates& attribs )
{
   for( auto cur = attribs.begin(); cur != attribs.end(); ++cur )
   {
      switch( cur->_id )
      {
         case ID_BAKE:
         {
            _node->markForBaking();
         }  break;
         default:;
      }
   }
   _node->graph()->invalidate( _node );
}

//------------------------------------------------------------------------------
//!
void
DFProbeBakerEditor::updateUI()
{
   _node->graph()->msg().modify( _node, attributesStates().ptr() );
}


/*==============================================================================
   CLASS DFProbeBaker
==============================================================================*/

//------------------------------------------------------------------------------
//!
DFProbeBaker::DFProbeBaker():
   _input( this ),
   _mustBake( false )
{
   _world = new DFWorld();
   _output.delegate( makeDelegate( this, &DFProbeBaker::process ) );
}

//------------------------------------------------------------------------------
//!
const ConstString&
DFProbeBaker::name() const
{
   return _name_probeBaker;
}

//------------------------------------------------------------------------------
//!
uint
DFProbeBaker::numInputs() const
{
   return 1;
}

//------------------------------------------------------------------------------
//!
DFInput*
DFProbeBaker::input( uint id )
{
   return (id < 1) ? &_input : nullptr;
}

//------------------------------------------------------------------------------
//!
DFOutput*
DFProbeBaker::output()
{
   return &_output;
}

//------------------------------------------------------------------------------
//!
RCP<DFNodeEditor>
DFProbeBaker::edit()
{
   if( _editor.isNull() )  _editor = new DFProbeBakerEditor( this );
   return _editor;
}

//------------------------------------------------------------------------------
//!
RCP<DFWorld>
DFProbeBaker::process()
{
   _world = _input.getWorld()->clone();
   if( _mustBake )
   {
      if( !_world->probes().empty() )
      {
         RCP<CubemapBaker> b = new CubemapBaker();
         RCP<World>        w = _world->createWorld();
         for( auto cur = _world->probes().begin(); cur != _world->probes().end(); ++cur )
         {
            DFProbe& p = *(*cur);
            switch( p.type() )
            {
               case Probe::CUBEMAP:
               {
                  // Replace whatever image was already there (if any).
                  p.image( new Image( new Bitmap( p.size(), Bitmap::BYTE, 4 ) ) );
                  b->add( w.ptr(), p.position(), p.image().ptr() );
               }  break;
            }
         }
         // TEMP: Since we are in the Gfx thread, it's OK to generate things right away.
         // The perfect solution would mean we are in a separate thread, submit the animator,
         // then do a waitFor( Condition ) in order to let other nodes process stuff.
         // Then, where the main thread is done with rendering, resume this thread and return
         // the result.
         //Core::addAnimator( b ); // Queue jobs, one face per frame.
         while( !b->exec(0.0, 0.0) ) {} // Simply do everything right away (blocks rendering).
      }
      _mustBake = false;
   }
   return _world;
}

NAMESPACE_END
