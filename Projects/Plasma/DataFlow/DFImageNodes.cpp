/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/DataFlow/DFImageNodes.h>
#include <Plasma/DataFlow/DFNodeAttr.h>
#include <Plasma/DataFlow/DFGraph.h>
#include <Plasma/Manipulator/Manipulator.h>

#include <Fusion/Resource/BitmapManipulator.h>
#include <Fusion/VM/VMFmt.h>

#if _MSC_VER
// 'this' used in member initializer list.
#pragma warning( disable: 4355 )
#endif

/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN

ConstString _outputName;
ConstString _transformName;
ConstString _blendName;
ConstString _invertName;
ConstString _swizzleName;

enum
{
   ID_REGION,
   ID_REGION_POS,
   ID_REGION_SIZE,
   ID_OUTPUT_SIZE,
   ID_SCALE,
   ID_OFFSET,
   ID_MODE,
   ID_R,
   ID_G,
   ID_B,
   ID_A
};

//------------------------------------------------------------------------------
//!
RCP<DFNode> outputVM( VMState* vm, int idx )
{
   RCP<DFOutputImageNode> node = new DFOutputImageNode();
   Vec4f r;
   Vec2i s;
   if( VM::get( vm, idx, "region", r ) ) node->parameters()._region.set( r(0,1), r(2,3) );
   if( VM::get( vm, idx, "size", s ) ) node->parameters()._size = s;

   return node;
}

//------------------------------------------------------------------------------
//!
RCP<DFNode> transformVM( VMState* vm, int idx )
{
   RCP<DFTransformImageNode> node = new DFTransformImageNode();
   Vec2f off;
   if( VM::get( vm, idx, "scale" , off ) ) node->scale( off );
   if( VM::get( vm, idx, "offset", off ) ) node->offset( off );

   return node;
}

//------------------------------------------------------------------------------
//!
RCP<DFNode> blendVM( VMState* vm, int idx )
{
   RCP<DFBlendImageNode> node = new DFBlendImageNode();
   int m;
   if( VM::get( vm, idx, "mode", m ) ) node->mode( DFBlendImageNode::BlendMode(m) );

   return node;
}

//------------------------------------------------------------------------------
//!
RCP<DFNode> invertVM( VMState*, int )
{
   RCP<DFInvertImageNode> node = new DFInvertImageNode();
   return node;
}

//------------------------------------------------------------------------------
//!
RCP<DFNode> swizzleVM( VMState* vm, int idx )
{
   RCP<DFSwizzleNode> node = new DFSwizzleNode();

   for( uint i = 0; i < 4; ++i )
   {
      if( VM::geti( vm, idx, i+1 ) )
      {
         const char* str = VM::toCString( vm, -1 );
         node->swizzle( i, toSwizzle(str) );
         VM::pop( vm );
      }
      else
      {
         break;
      }
   }

   return node;
}

UNNAMESPACE_END

NAMESPACE_BEGIN

/*==============================================================================
   STATIC INITIALIZATION
==============================================================================*/

//------------------------------------------------------------------------------
//!
void initializeImageNodes()
{
   _outputName     = "outputImage";
   _transformName  = "transformImage";
   _blendName      = "blendImage";
   _invertName     = "invertImage";
   _swizzleName    = "swizzleColor";

   DFNodeSpec::registerNode(
      DFSocket::IMAGE,
      _outputName, outputVM,
      "Output", "Define the output region and buffer size.",
      nullptr
   );
   DFNodeSpec::registerNode(
      DFSocket::IMAGE,
      _transformName, transformVM,
      "Transform", "Modify the image output region.",
      nullptr
   );
   DFNodeSpec::registerNode(
      DFSocket::IMAGE,
      _blendName, blendVM,
      "Blend", "Blend multiples images together.",
      nullptr
   );
   DFNodeSpec::registerNode(
      DFSocket::IMAGE,
      _invertName, invertVM,
      "Invert", "Inverts all of the channels of an image.",
      nullptr
   );
   DFNodeSpec::registerNode(
      DFSocket::IMAGE,
      _swizzleName, swizzleVM,
      "Swizzle", "Swizzles channels around.",
      nullptr
   );
  }

//------------------------------------------------------------------------------
//!
void terminateImageNodes()
{
   _outputName     = ConstString();
   _transformName  = ConstString();
   _blendName      = ConstString();
   _invertName     = ConstString();
   _swizzleName    = ConstString();
}

/*==============================================================================
   CLASS DFImageParams
==============================================================================*/

//------------------------------------------------------------------------------
//!
Mat3f
DFImageParams::regionToBitmap() const
{
   // A scale and translate to remap a coordinate inside the region
   // into a coordinate inside [0,0] to _size.
   float sx = _size.x / _region.width();
   float sy = _size.y / _region.height();
   // Does S(sx,sy) * T(-l,-b).
   return Mat3f(   sx, 0.0f, -_region.left()*sx,
                 0.0f,   sy, -_region.bottom()*sy,
                 0.0f, 0.0f,        1.0f       );
}

//------------------------------------------------------------------------------
//!
Mat3f
DFImageParams::bitmapToRegion() const
{
   // A scale and translate to remap a coordinate inside the region
   // into a coordinate inside [0,0] to _size.
   float sx = _region.width()  / _size.x;
   float sy = _region.height() / _size.y;
   // Does T(l,b) * S(sx,sy).
   return Mat3f(   sx, 0.0f, _region.left(),
                 0.0f,   sy, _region.bottom(),
                 0.0f, 0.0f,        1.0f       );
}

/*==============================================================================
   CLASS DFImageOutput
==============================================================================*/

//------------------------------------------------------------------------------
//!
RCP<Bitmap>
DFImageOutput::getImage( const DFImageParams& p )
{
   return _delegate( p );
}

//------------------------------------------------------------------------------
//!
DFSocket::Type
DFImageOutput::type() const
{
   return IMAGE;
}

/*==============================================================================
   CLASS DFImageInput
==============================================================================*/

//------------------------------------------------------------------------------
//!
RCP<Bitmap>
DFImageInput::getImage( const DFImageParams& p )
{
   if( !_output ) return nullptr;
   return _output->getImage( p );
}

//------------------------------------------------------------------------------
//!
DFSocket::Type
DFImageInput::type() const
{
   return IMAGE;
}

//------------------------------------------------------------------------------
//!
bool
DFImageInput::isConnected() const
{
   return _output != nullptr;
}

//------------------------------------------------------------------------------
//!
void
DFImageInput::connect( DFOutput* output )
{
   CHECK( type() == output->type() );
   if( _output ) disconnectFrom( _output );
   _output = (DFImageOutput*)output;
}

//------------------------------------------------------------------------------
//!
void
DFImageInput::disconnect( DFOutput* output )
{
   if( _output == output ) _output = nullptr;
}

//------------------------------------------------------------------------------
//!
void
DFImageInput::disconnect()
{
   if( !_output ) return;
   disconnectFrom( _output );
   _output = nullptr;
}

/*==============================================================================
   CLASS DFImageMultiInput
==============================================================================*/

//------------------------------------------------------------------------------
//!
RCP<Bitmap>
DFImageMultiInput::getImage( uint i, const DFImageParams& p )
{
   if( i >= _outputs.size() ) return nullptr;
   return _outputs[i]->getImage( p );
}

//------------------------------------------------------------------------------
//!
DFSocket::Type
DFImageMultiInput::type() const
{
   return IMAGE;
}

//------------------------------------------------------------------------------
//!
bool
DFImageMultiInput::isConnected() const
{
   return !_outputs.empty();
}

//------------------------------------------------------------------------------
//!
void
DFImageMultiInput::connect( DFOutput* output )
{
   CHECK( type() == output->type() );
   _outputs.pushBack( (DFImageOutput*)output );
}

//------------------------------------------------------------------------------
//!
void
DFImageMultiInput::disconnect( DFOutput* output )
{
   _outputs.removeSwap( (DFImageOutput*)output );
}

//------------------------------------------------------------------------------
//!
void
DFImageMultiInput::disconnect()
{
   for( auto it = _outputs.begin(); it != _outputs.end(); ++it )
   {
      disconnectFrom( *it );
   }
   _outputs.clear();
}

/*==============================================================================
   CLASS DFOutputImageEditor
==============================================================================*/

class DFOutputImageEditor:
   public DFNodeEditor
{
public:

   /*----- methods -----*/

   DFOutputImageEditor( DFOutputImageNode* n ): _node(n) {}

   PLASMA_DLL_API virtual RCP<Manipulator>  manipulator() { return nullptr; }
   PLASMA_DLL_API virtual RCP<DFNodeAttrList>  attributes() const;
   PLASMA_DLL_API virtual RCP<DFNodeAttrStates>  attributesStates() const;
   PLASMA_DLL_API virtual void updateAttributes( const DFNodeAttrStates& );

protected:

   /*----- members -----*/

   DFOutputImageNode* _node;
};

//------------------------------------------------------------------------------
//!
RCP<DFNodeAttrList>
DFOutputImageEditor::attributes() const
{
   RCP<DFNodeAttrList> region = new DFNodeAttrList();
   region->add( DFNodeAttr( "FLOAT2", ID_REGION_POS, "Position" ) );
   region->add( DFNodeAttr( "FLOAT2", ID_REGION_SIZE, "Size" ) );

   RCP<DFNodeAttrList> atts = new DFNodeAttrList();
   atts->add( DFNodeAttr( ID_REGION, "", region.ptr() ) );
   atts->add( DFNodeAttr( "INT2", ID_OUTPUT_SIZE, "Buffer size" ) );
   return atts;
}

//------------------------------------------------------------------------------
//!
RCP<DFNodeAttrStates>
DFOutputImageEditor::attributesStates() const
{
   RCP<DFNodeAttrStates> states = new DFNodeAttrStates();
   states->set( ID_REGION_POS, _node->parameters().region().position() );
   states->set( ID_REGION_SIZE, _node->parameters().region().size() );
   states->set( ID_OUTPUT_SIZE, _node->parameters().size() );
   return states;
}

//------------------------------------------------------------------------------
//!
void
DFOutputImageEditor::updateAttributes( const DFNodeAttrStates& attribs )
{
   for( auto cur = attribs.begin(); cur != attribs.end(); ++cur )
   {
      switch( cur->_id )
      {
         case ID_REGION_POS:
         {
            _node->parameters()._region = AARectf( (*cur)._value.getVec2(), _node->parameters().region().size() );;
            // Invalidate graph.
            _node->graph()->invalidate( _node );
         }  break;
         case ID_REGION_SIZE:
         {
            _node->parameters()._region = AARectf( _node->parameters().region().position(), (*cur)._value.getVec2() );
            // Invalidate graph.
            _node->graph()->invalidate( _node );
         }  break;
         case ID_OUTPUT_SIZE:
         {
            _node->parameters()._size = Vec2i( (*cur)._value.getVec2() );
            // Invalidate graph.
            _node->graph()->invalidate( _node );
         }  break;
         default:;
      }
   }
}

/*==============================================================================
   CLASS DFOutputImageNode
==============================================================================*/

//------------------------------------------------------------------------------
//!
DFOutputImageNode::DFOutputImageNode():
   _input(this), _params( Vec2f(0.0f), Vec2f(1.0f), Vec2i(256) )
{
   _output.delegate( makeDelegate( this, &DFOutputImageNode::process ) );
}

//------------------------------------------------------------------------------
//!
const ConstString&
DFOutputImageNode::name() const
{
   return _outputName;
}

//------------------------------------------------------------------------------
//!
uint
DFOutputImageNode::numInputs() const
{
   return 1;
}

//------------------------------------------------------------------------------
//!
DFOutput*
DFOutputImageNode::output()
{
   return &_output;
}

//------------------------------------------------------------------------------
//!
DFInput*
DFOutputImageNode::input( uint id )
{
   if( id == 0 ) return &_input;
   return nullptr;
}

//------------------------------------------------------------------------------
//!
RCP<DFNodeEditor>
DFOutputImageNode::edit()
{
   if( _editor.isNull() )  _editor = new DFOutputImageEditor( this );
   return _editor;
}

//------------------------------------------------------------------------------
//!
RCP<Bitmap>
DFOutputImageNode::process( const DFImageParams& )
{
   return _input.getImage( _params );
}

//------------------------------------------------------------------------------
//!
bool
DFOutputImageNode::dumpCustom( TextStream& os, StreamIndent& indent ) const
{
   os << indent << "region=" << VMFmt( Vec4f( _params._region.position(), _params._region.size() ) ) << "," << nl;
   os << indent << "size="   << VMFmt( _params.size() )  << "," << nl;
   return os.ok();
}

/*==============================================================================
   CLASS DFTransformImageEditor
==============================================================================*/

class DFTransformImageEditor:
   public DFNodeEditor
{
public:

   /*----- methods -----*/

   DFTransformImageEditor( DFTransformImageNode* n ): _node(n) {}

   PLASMA_DLL_API virtual RCP<Manipulator>  manipulator() { return nullptr; }
   PLASMA_DLL_API virtual RCP<DFNodeAttrList>  attributes() const;
   PLASMA_DLL_API virtual RCP<DFNodeAttrStates>  attributesStates() const;
   PLASMA_DLL_API virtual void updateAttributes( const DFNodeAttrStates& );

protected:

   /*----- members -----*/

   DFTransformImageNode* _node;
};

//------------------------------------------------------------------------------
//!
RCP<DFNodeAttrList>
DFTransformImageEditor::attributes() const
{
   RCP<DFNodeAttrList> atts = new DFNodeAttrList();
   atts->add( DFNodeAttr( "FLOAT2", ID_SCALE , "Scale"  ) );
   atts->add( DFNodeAttr( "FLOAT2", ID_OFFSET, "Offset" ) );
   return atts;
}

//------------------------------------------------------------------------------
//!
RCP<DFNodeAttrStates>
DFTransformImageEditor::attributesStates() const
{
   RCP<DFNodeAttrStates> states = new DFNodeAttrStates();
   states->set( ID_SCALE , _node->scale()  );
   states->set( ID_OFFSET, _node->offset() );
   return states;
}

//------------------------------------------------------------------------------
//!
void
DFTransformImageEditor::updateAttributes( const DFNodeAttrStates& attribs )
{
   for( auto cur = attribs.begin(); cur != attribs.end(); ++cur )
   {
      switch( cur->_id )
      {
         case ID_SCALE:
         {
            _node->scale( (*cur)._value.getVec2() );
            _node->graph()->invalidate( _node );
         }  break;
         case ID_OFFSET:
         {
            _node->offset( (*cur)._value.getVec2() );
            _node->graph()->invalidate( _node );
         }  break;
         default:;
      }
   }
}

/*==============================================================================
   CLASS DFTransformImageNode
==============================================================================*/

//------------------------------------------------------------------------------
//!
DFTransformImageNode::DFTransformImageNode():
   _input(this), _scale(1.0f), _offset(0.0f)
{
   _output.delegate( makeDelegate( this, &DFTransformImageNode::process ) );
}

//------------------------------------------------------------------------------
//!
const ConstString&
DFTransformImageNode::name() const
{
   return _transformName;
}

//------------------------------------------------------------------------------
//!
uint
DFTransformImageNode::numInputs() const
{
   return 1;
}

//------------------------------------------------------------------------------
//!
DFOutput*
DFTransformImageNode::output()
{
   return &_output;
}

//------------------------------------------------------------------------------
//!
DFInput*
DFTransformImageNode::input( uint id )
{
   if( id == 0 ) return &_input;
   return nullptr;
}

//------------------------------------------------------------------------------
//!
RCP<DFNodeEditor>
DFTransformImageNode::edit()
{
   if( _editor.isNull() )  _editor = new DFTransformImageEditor( this );
   return _editor;
}

//------------------------------------------------------------------------------
//!
RCP<Bitmap>
DFTransformImageNode::process( const DFImageParams& p )
{
   DFImageParams nparams = p;
   nparams._region.scaleAndBias( _scale, _offset );
   return _input.getImage( nparams );
}

//------------------------------------------------------------------------------
//!
bool
DFTransformImageNode::dumpCustom( TextStream& os, StreamIndent& indent ) const
{
   os << indent << "scale="  << VMFmt( _scale  )  << "," << nl;
   os << indent << "offset=" << VMFmt( _offset )  << "," << nl;
   return os.ok();
}

/*==============================================================================
   CLASS DFBlendImageEditor
==============================================================================*/

class DFBlendImageEditor:
   public DFNodeEditor
{
public:

   /*----- methods -----*/

   DFBlendImageEditor( DFBlendImageNode* n ): _node(n) {}

   PLASMA_DLL_API virtual RCP<Manipulator>  manipulator() { return nullptr; }
   PLASMA_DLL_API virtual RCP<DFNodeAttrList>  attributes() const;
   PLASMA_DLL_API virtual RCP<DFNodeAttrStates>  attributesStates() const;
   PLASMA_DLL_API virtual void updateAttributes( const DFNodeAttrStates& );

protected:

   /*----- members -----*/

   DFBlendImageNode* _node;
};

//------------------------------------------------------------------------------
//!
RCP<DFNodeAttrList>
DFBlendImageEditor::attributes() const
{
   RCP<DFNodeAttrList> atts = new DFNodeAttrList();
   RCP<Table> enums = new Table();
   enums->pushBack( "ADD" );
   enums->pushBack( "MAX" );
   enums->pushBack( "MIN" );
   enums->pushBack( "BLEND" );
   enums->pushBack( "SUBTRACT");
   atts->add( DFNodeAttr( "ENUM", ID_MODE, "Mode" ).enums( enums.ptr() ) );
   return atts;
}

//------------------------------------------------------------------------------
//!
RCP<DFNodeAttrStates>
DFBlendImageEditor::attributesStates() const
{
   RCP<DFNodeAttrStates> states = new DFNodeAttrStates();
   states->set( ID_MODE, (float)_node->mode() );
   return states;
}

//------------------------------------------------------------------------------
//!
void
DFBlendImageEditor::updateAttributes( const DFNodeAttrStates& attribs )
{
   for( auto cur = attribs.begin(); cur != attribs.end(); ++cur )
   {
      if( cur->_id == ID_MODE )
      {
         _node->mode( (DFBlendImageNode::BlendMode)int((*cur)._value.getFloat()) );
         // Invalidate graph.
         _node->graph()->invalidate( _node );
      }
   }
}

/*==============================================================================
   CLASS DFBlendImageNode
==============================================================================*/

//------------------------------------------------------------------------------
//!
DFBlendImageNode::DFBlendImageNode():
   _inputs(this),
   _mode( BLEND_NORMAL )
{
   _output.delegate( makeDelegate( this, &DFBlendImageNode::process ) );
}

//------------------------------------------------------------------------------
//!
const ConstString&
DFBlendImageNode::name() const
{
   return _blendName;
}

//------------------------------------------------------------------------------
//!
uint
DFBlendImageNode::numInputs() const
{
   return 1;
}

//------------------------------------------------------------------------------
//!
DFOutput*
DFBlendImageNode::output()
{
   return &_output;
}

//------------------------------------------------------------------------------
//!
DFInput*
DFBlendImageNode::input( uint id )
{
   if( id == 0 ) return &_inputs;
   return nullptr;
}

//------------------------------------------------------------------------------
//!
RCP<DFNodeEditor>
DFBlendImageNode::edit()
{
   if( _editor.isNull() )  _editor = new DFBlendImageEditor( this );
   return _editor;
}

//------------------------------------------------------------------------------
//!
RCP<Bitmap>
DFBlendImageNode::process( const DFImageParams& p )
{
   // Copy the first input as the background.
   RCP<Bitmap> input = _inputs.getImage( 0, p );

   // 1. Is the size correct?
   if( input.isNull() || input->dimension()(0,1) != p.size() ) return nullptr;

   // 2. Copy
   RCP<Bitmap> img = new Bitmap( p.size(), Bitmap::FLOAT, 4 );
   memcpy( img->pixels(), input->pixels(), img->size() );

   // Blend each other image.
   const uint n = uint(_inputs.size());
   for( uint i = 1; i < n; ++i )
   {
      input = _inputs.getImage( i, p );
      // 1. Is the size correct?
      if( input.isNull() || input->dimension()(0,1) != p.size() ) continue;
      // blend.
      Vec4f* bg        = (Vec4f*)img->pixels();
      Vec4f* fg        = (Vec4f*)input->pixels();
      size_t numPixels = img->numPixels();

      switch( _mode )
      {
         case BLEND_ADD:
            for( size_t p = 0; p < numPixels; ++p, ++bg, ++fg ) *bg += *fg;
            break;
         case BLEND_NORMAL:
            for( size_t p = 0; p < numPixels; ++p, ++bg, ++fg ) *bg  = (*bg) * (1.0f-(*fg).w) + *fg;
            break;
         case BLEND_MAX:
            for( size_t p = 0; p < numPixels; ++p, ++bg, ++fg ) *bg  = CGM::max( *bg, *fg );
            break;
         case BLEND_MIN:
            for( size_t p = 0; p < numPixels; ++p, ++bg, ++fg ) *bg  = CGM::min( *bg, *fg );
            break;
         case BLEND_SUBTRACT:
            for( size_t p = 0; p < numPixels; ++p, ++bg, ++fg ) *bg -= *fg;
            break;
      }
   }
   return img;
}

//------------------------------------------------------------------------------
//!
bool
DFBlendImageNode::dumpCustom( TextStream& os, StreamIndent& indent ) const
{
   os << indent << "mode=" << _mode << "," << nl;
   return os.ok();
}


/*==============================================================================
   CLASS DFInvertImageNode
==============================================================================*/

//------------------------------------------------------------------------------
//!
DFInvertImageNode::DFInvertImageNode():
   _input(this)
{
   _output.delegate( makeDelegate( this, &DFInvertImageNode::process ) );
}

//------------------------------------------------------------------------------
//!
const ConstString&
DFInvertImageNode::name() const
{
   return _invertName;
}

//------------------------------------------------------------------------------
//!
uint
DFInvertImageNode::numInputs() const
{
   return 1;
}

//------------------------------------------------------------------------------
//!
DFOutput*
DFInvertImageNode::output()
{
   return &_output;
}

//------------------------------------------------------------------------------
//!
DFInput*
DFInvertImageNode::input( uint id )
{
   if( id == 0 ) return &_input;
   return nullptr;
}

//------------------------------------------------------------------------------
//!
RCP<Bitmap>
DFInvertImageNode::process( const DFImageParams& p )
{
   // Retrieve the input.
   RCP<Bitmap> input = _input.getImage( p );

   // Check size.
   if( input.isNull() || input->dimension()(0,1) != p.size() ) return nullptr;

   // Make a copy.
   RCP<Bitmap> img = new Bitmap( p.size(), Bitmap::FLOAT, 4 );
   memcpy( img->pixels(), input->pixels(), img->size() );

   // Invert all of the channels in-place.
   Vec4f* cur = (Vec4f*)img->pixels();
   Vec4f* end = cur + img->numPixels();
   for( ; cur < end; ++cur )
   {
      *cur = Vec4f(1.0f) - *cur;  // Invert RGBA.
      //*cur = Vec4f( Vec3f(1.0f)-((*cur)(0,1,2)), (*cur)(3) );  // Keep alpha.
   }

   return img;
}


/*==============================================================================
   CLASS DFSwizzleEditor
==============================================================================*/

class DFSwizzleEditor:
   public DFNodeEditor
{
public:

   /*----- methods -----*/

   DFSwizzleEditor( DFSwizzleNode* n ): _node(n) {}

   PLASMA_DLL_API virtual RCP<Manipulator>  manipulator() { return nullptr; }
   PLASMA_DLL_API virtual RCP<DFNodeAttrList>  attributes() const;
   PLASMA_DLL_API virtual RCP<DFNodeAttrStates>  attributesStates() const;
   PLASMA_DLL_API virtual void updateAttributes( const DFNodeAttrStates& );

protected:

   /*----- members -----*/

   DFSwizzleNode* _node;
};

//------------------------------------------------------------------------------
//!
RCP<DFNodeAttrList>
DFSwizzleEditor::attributes() const
{
   RCP<DFNodeAttrList> atts = new DFNodeAttrList();
   RCP<Table> enums = new Table();
   enums->pushBack( "0" );
   enums->pushBack( "1" );
   enums->pushBack( "Red" );
   enums->pushBack( "Green" );
   enums->pushBack( "Blue" );
   enums->pushBack( "Alpha" );
   enums->pushBack( "Luma" );
   enums->pushBack( "Chroma U" );
   enums->pushBack( "Chroma V" );
   atts->add( DFNodeAttr( "ENUM", ID_R, "R" ).enums( enums.ptr() ) );
   atts->add( DFNodeAttr( "ENUM", ID_G, "G" ).enums( enums.ptr() ) );
   atts->add( DFNodeAttr( "ENUM", ID_B, "B" ).enums( enums.ptr() ) );
   atts->add( DFNodeAttr( "ENUM", ID_A, "A" ).enums( enums.ptr() ) );
   return atts;
}

//------------------------------------------------------------------------------
//!
RCP<DFNodeAttrStates>
DFSwizzleEditor::attributesStates() const
{
   RCP<DFNodeAttrStates> states = new DFNodeAttrStates();
   states->set( ID_R, (float)_node->swizzle(0) );
   states->set( ID_G, (float)_node->swizzle(1) );
   states->set( ID_B, (float)_node->swizzle(2) );
   states->set( ID_A, (float)_node->swizzle(3) );
   return states;
}

//------------------------------------------------------------------------------
//!
void
DFSwizzleEditor::updateAttributes( const DFNodeAttrStates& attribs )
{
   for( auto cur = attribs.begin(); cur != attribs.end(); ++cur )
   {
      switch( cur->_id )
      {
         case ID_R:
         case ID_G:
         case ID_B:
         case ID_A:
            _node->swizzle( cur->_id-ID_R, DFSwizzleNode::Swizzle(int((*cur)._value.getFloat())) );
            _node->graph()->invalidate( _node );
            break;
         default:
            StdErr << "WARNING: Swizzle doesn't understand '" << cur->_id << "'." << nl;
      }
   }
}

/*==============================================================================
   CLASS DFSwizzleNode
==============================================================================*/

//------------------------------------------------------------------------------
//!
DFSwizzleNode::DFSwizzleNode():
   _input( this )
{
   swizzle( RED, GREEN, BLUE, ALPHA );
   _output.delegate( makeDelegate( this, &DFSwizzleNode::process ) );
}

//------------------------------------------------------------------------------
//!
const ConstString&
DFSwizzleNode::name() const
{
   return _swizzleName;
}

//------------------------------------------------------------------------------
//!
uint
DFSwizzleNode::numInputs() const
{
   return 1;
}

//------------------------------------------------------------------------------
//!
DFOutput*
DFSwizzleNode::output()
{
   return &_output;
}

//------------------------------------------------------------------------------
//!
DFInput*
DFSwizzleNode::input( uint idx )
{
   if( idx == 0 ) return &_input;
   return nullptr;
}

//------------------------------------------------------------------------------
//!
RCP<DFNodeEditor>
DFSwizzleNode::edit()
{
   if( _editor.isNull() )  _editor = new DFSwizzleEditor( this );
   return _editor;
}

//------------------------------------------------------------------------------
//!
RCP<Bitmap>
DFSwizzleNode::process( const DFImageParams& p )
{
   // Copy the first input as the background.
   RCP<Bitmap> input = _input.getImage( p );

   // 1. Is the size correct?
   if( input.isNull() || input->dimension()(0,1) != p.size() ) return nullptr;

   // 2. Copy
   RCP<Bitmap> img = new Bitmap( p.size(), Bitmap::FLOAT, 4 );
   memcpy( img->pixels(), input->pixels(), img->size() );

   // 3. Prepare color transform matrix.
   Mat4f mat;
   Vec4f off;
   Vec4f tmpV;
   float tmpF;
   for( uint i = 0; i < 4; ++i )
   {
      getTransform( swizzle(i), tmpV, tmpF );
      mat.row( i, tmpV );
      off(i) = tmpF;
   }

   // 4. Perform the transform on the whole image.
   BitmapManipulator::transform( *img, Vec2i(0), p.size(), mat, off );

   return img;
}

//------------------------------------------------------------------------------
//!
bool
DFSwizzleNode::dumpCustom( TextStream& os, StreamIndent& indent ) const
{
   for( uint i = 0; i < 4; ++i )
   {
      os << indent << "\"" << toStr(swizzle(i)) << "\"," << nl;
   }
   return os.ok();
}

//------------------------------------------------------------------------------
//!
void
DFSwizzleNode::getTransform( Swizzle s, Vec4f& scale, float& offset )
{
   switch( s )
   {
      case ZERO:
         scale  = Vec4f( 0.0f );
         offset = 0.0f;
         break;
      case ONE:
         scale  = Vec4f( 0.0f );
         offset = 1.0f;
         break;
      case RED:
         scale  = Vec4f( 1.0f, 0.0f, 0.0f, 0.0f );
         offset = 0.0f;
         break;
      case GREEN:
         scale  = Vec4f( 0.0f, 1.0f, 0.0f, 0.0f );
         offset = 0.0f;
         break;
      case BLUE:
         scale  = Vec4f( 0.0f, 0.0f, 1.0f, 0.0f );
         offset = 0.0f;
         break;
      case ALPHA:
         scale  = Vec4f( 0.0f, 0.0f, 0.0f, 1.0f );
         offset = 0.0f;
         break;
      case LUMA:
         scale  = Vec4f( 0.299f, 0.587f, 0.114f, 0.0f );
         offset = 0.0f;
         break;
      case CHROMA_U:
         scale  = Vec4f( -0.147f, -0.289f, 0.436f, 0.0f );
         offset = 0.0f;
         break;
      case CHROMA_V:
         scale  = Vec4f( 0.615f, -0.515f, -0.100f, 0.0f );
         offset = 0.0f;
         break;
      default:
         break;
   }
}

//------------------------------------------------------------------------------
//!
const char*  toStr( DFSwizzleNode::Swizzle v )
{
   switch( v )
   {
      case DFSwizzleNode::ZERO    : return "0";
      case DFSwizzleNode::ONE     : return "1";
      case DFSwizzleNode::RED     : return "R";
      case DFSwizzleNode::GREEN   : return "G";
      case DFSwizzleNode::BLUE    : return "B";
      case DFSwizzleNode::ALPHA   : return "A";
      case DFSwizzleNode::LUMA    : return "L";
      case DFSwizzleNode::CHROMA_U: return "U";
      case DFSwizzleNode::CHROMA_V: return "V";
      default                     : return "?";
   }
}

//------------------------------------------------------------------------------
//!
DFSwizzleNode::Swizzle  toSwizzle( const char* str )
{
   if( str )
   {
      switch( str[0] )
      {
         case '0': return DFSwizzleNode::ZERO    ;
         case '1': return DFSwizzleNode::ONE     ;
         case 'R': return DFSwizzleNode::RED     ;
         case 'G': return DFSwizzleNode::GREEN   ;
         case 'B': return DFSwizzleNode::BLUE    ;
         case 'A': return DFSwizzleNode::ALPHA   ;
         case 'L': return DFSwizzleNode::LUMA    ;
         case 'U': return DFSwizzleNode::CHROMA_U;
         case 'V': return DFSwizzleNode::CHROMA_V;
      }
   }
   return DFSwizzleNode::ZERO;
}


NAMESPACE_END
