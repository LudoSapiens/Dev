/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/DataFlow/DFImageGenerator.h>
#include <Plasma/DataFlow/DFNodeAttr.h>
#include <Plasma/DataFlow/DFGraph.h>
#include <Plasma/Manipulator/Manipulator.h>

#include <Fusion/Resource/BitmapManipulator.h>
#include <Fusion/VM/VMFmt.h>

#include <CGMath/Noise.h>

#include <Base/MT/Thread.h>

#if _MSC_VER
// 'this' used in member initializer list.
#pragma warning( disable: 4355 )
#endif

/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN

ConstString _importName;
ConstString _drawCircleName;
ConstString _drawRectName;
ConstString _fillName;
ConstString _noiseName;

enum
{
   ID_COLOR,
   ID_CENTER,
   ID_FBM,
   ID_FBM_WIDTH,
   ID_GAIN,
   ID_JITTER,
   ID_LACUNARITY,
   ID_NAME,
   ID_OCTAVE,
   ID_PERFIL,
   ID_PERFIL_WIDTH,
   ID_RADIUS,
   ID_SIZE,
   ID_TYPE,
   ID_VORONOI
};

//------------------------------------------------------------------------------
//!
RCP<DFNode> importVM( VMState* vm, int idx )
{
   RCP<DFImportImageNode> node = new DFImportImageNode();
   if( VM::geti( vm, idx, 1 ) )
   {
      node->imageID( VM::toString( vm, -1 ) );
      VM::pop( vm );
   }
   else
   {
      StdErr << "ERROR: Missing image ID in importImage()." << nl;
   }
   return node;
}

//------------------------------------------------------------------------------
//!
RCP<DFNode> fillVM( VMState* vm, int idx )
{
   RCP<DFFillColorNode> node = new DFFillColorNode();
   Vec4f c;
   if( VM::get( vm, idx, "color", c ) ) node->color( c );

   return node;
}

//------------------------------------------------------------------------------
//!
RCP<DFNode> drawCircleVM( VMState* vm, int idx )
{
   RCP<DFDrawCircleNode> node = new DFDrawCircleNode();
   Vec4f v4;
   if( VM::get( vm, idx, "color", v4 ) ) node->color( v4 );
   Vec2f v2;
   if( VM::get( vm, idx, "center", v2 ) ) node->center( v2 );
   float v1;
   if( VM::get( vm, idx, "radius", v1 ) ) node->radius( v1 );

   return node;
}

//------------------------------------------------------------------------------
//!
RCP<DFNode> drawRectVM( VMState* vm, int idx )
{
   RCP<DFDrawRectNode> node = new DFDrawRectNode();
   Vec4f v4;
   if( VM::get( vm, idx, "color", v4 ) ) node->color( v4 );
   Vec2f v2;
   if( VM::get( vm, idx, "center", v2 ) ) node->center( v2 );
   if( VM::get( vm, idx, "size", v2 ) ) node->size( v2 );

   return node;
}

//------------------------------------------------------------------------------
//!
RCP<DFNode> noiseVM( VMState* vm, int idx )
{
   RCP<DFNoiseImageNode> node = new DFNoiseImageNode();
   int i;
   if( VM::get( vm, idx, "type", i ) ) node->type( DFNoiseImageNode::Type(i) );

   return node;
}

UNNAMESPACE_END

NAMESPACE_BEGIN

/*==============================================================================
   STATIC INITIALIZATION
==============================================================================*/

//------------------------------------------------------------------------------
//!
void initializeImageGenerators()
{
   _importName     = "importImage";
   _drawCircleName = "drawCircle";
   _drawRectName   = "drawRect";
   _fillName       = "fillColor";
   _noiseName      = "noiseImage";

   DFNodeSpec::registerNode(
      DFSocket::IMAGE,
      _importName, importVM,
      "Import", "Import an image.",
      nullptr
   );
   DFNodeSpec::registerNode(
      DFSocket::IMAGE,
      _fillName, fillVM,
      "Fill color", "Create an image filled with an uniform color.",
      nullptr
   );
   DFNodeSpec::registerNode(
      DFSocket::IMAGE,
      _drawCircleName, drawCircleVM,
      "Circle", "Draws a circle.",
      nullptr
   );
   DFNodeSpec::registerNode(
      DFSocket::IMAGE,
      _drawRectName, drawRectVM,
      "Rectangle", "Draws a rectangle.",
      nullptr
   );
   DFNodeSpec::registerNode(
      DFSocket::IMAGE,
      _noiseName, noiseVM,
      "Noise", "Generate grayscale noise image.",
      nullptr
   );
}

//------------------------------------------------------------------------------
//!
void terminateImageGenerators()
{
   _importName     = ConstString();
   _drawCircleName = ConstString();
   _drawRectName   = ConstString();
   _fillName       = ConstString();
   _noiseName      = ConstString();
}

/*==============================================================================
   CLASS DFImportImageEditor
==============================================================================*/

class DFImportImageEditor:
   public DFNodeEditor
{
public:

   /*----- methods -----*/

   DFImportImageEditor( DFImportImageNode* n ): _node(n) {}

   PLASMA_DLL_API virtual RCP<Manipulator>  manipulator() { return nullptr; }
   PLASMA_DLL_API virtual RCP<DFNodeAttrList>  attributes() const;
   PLASMA_DLL_API virtual RCP<DFNodeAttrStates>  attributesStates() const;
   PLASMA_DLL_API virtual void updateAttributes( const DFNodeAttrStates& );

protected:

   /*----- members -----*/

   DFImportImageNode* _node;
};

//------------------------------------------------------------------------------
//!
RCP<DFNodeAttrList>
DFImportImageEditor::attributes() const
{
   RCP<DFNodeAttrList> atts = new DFNodeAttrList();
   atts->add( DFNodeAttr( "STRING", ID_NAME, "Name" ) );
   return atts;
}

//------------------------------------------------------------------------------
//!
RCP<DFNodeAttrStates>
DFImportImageEditor::attributesStates() const
{
   RCP<DFNodeAttrStates> states = new DFNodeAttrStates();
   states->set( ID_NAME, _node->imageID().cstr() );
   return states;
}

//------------------------------------------------------------------------------
//!
void
DFImportImageEditor::updateAttributes( const DFNodeAttrStates& attribs )
{
   for( auto cur = attribs.begin(); cur != attribs.end(); ++cur )
   {
      if( cur->_id == ID_NAME )
      {
         _node->imageID( (*cur)._value.getString().cstr() );
         _node->graph()->invalidate( _node );
      }
   }
}

/*==============================================================================
   CLASS DFImportImageNode
==============================================================================*/

//------------------------------------------------------------------------------
//!
DFImportImageNode::DFImportImageNode()
{
   _output.delegate( makeDelegate( this, &DFImportImageNode::process ) );
}

//------------------------------------------------------------------------------
//!
const ConstString&
DFImportImageNode::name() const
{
   return _importName;
}

//------------------------------------------------------------------------------
//!
uint
DFImportImageNode::numInputs() const
{
   return 0;
}

//------------------------------------------------------------------------------
//!
DFOutput*
DFImportImageNode::output()
{
   return &_output;
}

//------------------------------------------------------------------------------
//!
DFInput*
DFImportImageNode::input( uint )
{
   return nullptr;
}

//------------------------------------------------------------------------------
//!
RCP<DFNodeEditor>
DFImportImageNode::edit()
{
   if( _editor.isNull() )  _editor = new DFImportImageEditor( this );
   return _editor;
}

//------------------------------------------------------------------------------
//!
RCP<Bitmap>
DFImportImageNode::process( const DFImageParams& p )
{
   // Check if cached version is OK.
   if( _cachedBitmap.isValid() && _cachedParams == p )  return _cachedBitmap;

   // Read image file.
   if( _image.isNull() )
   {
      RCP< Resource<Image> > imageRes = ResManager::getImage( _imageID );
      // Is it a valid existing resource?
      if( imageRes.isValid() )
      {
         StdErr << "Inefficient resource wait (" << __FILE__ << "@" << __LINE__ << ")" << nl;
         while( !imageRes->isReady() )  Thread::sleep( 0.01f ); // FIXME: more efficient waiting.
         _image = data( imageRes );
      }
      else
      {
         return nullptr;
      }
   }

   // Guarantee a bitmap of the proper size.
   if( _cachedBitmap.isNull() || _cachedBitmap->dimension()(0,1) != p.size() )
   {
      _cachedBitmap = new Bitmap( p.size(), Bitmap::FLOAT, 4 );
   }

   // Retrieve the image's bitmap.
   const Bitmap& bmp = *(_image->bitmap());

   // Retrieve transform matrix.
   Mat3f mat = Mat3f::scaling( 1.0f/bmp.dimension().x, 1.0f/bmp.dimension().y, 1.0f ) * p.bitmapToRegion();

   // Read using nearest for now.
   Vec4f* dst = (Vec4f*)_cachedBitmap->pixels();
   Vec2f curIdx;
   Vec2f fSize = p.size();
   for( curIdx.y = 0; curIdx.y < fSize.y; ++curIdx.y )
   {
      for( curIdx.x = 0; curIdx.x < fSize.x; ++curIdx.x, ++dst )
      {
         *dst = BitmapManipulator::nearest( bmp, mat*curIdx );
      }
   }

   _cachedParams = p;

   return _cachedBitmap;
}

//------------------------------------------------------------------------------
//!
void
DFImportImageNode::imageID( const String& id )
{
   _imageID      = id;
   _image        = nullptr;
}

//------------------------------------------------------------------------------
//!
bool
DFImportImageNode::dumpCustom( TextStream& os, StreamIndent& indent ) const
{
   os << indent << "\"" << _imageID << "\"," << nl;
   return os.ok();
}

/*==============================================================================
   CLASS DFFillColorEditor
==============================================================================*/

class DFFillColorEditor:
   public DFNodeEditor
{
public:

   /*----- methods -----*/

   DFFillColorEditor( DFFillColorNode* n ): _node(n) {}

   PLASMA_DLL_API virtual RCP<Manipulator>  manipulator() { return nullptr; }
   PLASMA_DLL_API virtual RCP<DFNodeAttrList>  attributes() const;
   PLASMA_DLL_API virtual RCP<DFNodeAttrStates>  attributesStates() const;
   PLASMA_DLL_API virtual void updateAttributes( const DFNodeAttrStates& );

protected:

   /*----- members -----*/

   DFFillColorNode* _node;
};

//------------------------------------------------------------------------------
//!
RCP<DFNodeAttrList>
DFFillColorEditor::attributes() const
{
   RCP<DFNodeAttrList> atts = new DFNodeAttrList();
   atts->add( DFNodeAttr( "RGBA", ID_COLOR, "Color" ) );
   return atts;
}

//------------------------------------------------------------------------------
//!
RCP<DFNodeAttrStates>
DFFillColorEditor::attributesStates() const
{
   RCP<DFNodeAttrStates> states = new DFNodeAttrStates();
   states->set( ID_COLOR, _node->color() );
   return states;
}

//------------------------------------------------------------------------------
//!
void
DFFillColorEditor::updateAttributes( const DFNodeAttrStates& attribs )
{
   for( auto cur = attribs.begin(); cur != attribs.end(); ++cur )
   {
      if( cur->_id == ID_COLOR )
      {
         _node->color( (*cur)._value.getVec4() );
         // Invalidate graph.
         _node->graph()->invalidate( _node );
      }
   }
}

/*==============================================================================
   CLASS DFFillColorNode
==============================================================================*/

//------------------------------------------------------------------------------
//!
DFFillColorNode::DFFillColorNode():
   _color( 0.0f )
{
   _output.delegate( makeDelegate( this, &DFFillColorNode::process ) );
}

//------------------------------------------------------------------------------
//!
const ConstString&
DFFillColorNode::name() const
{
   return _fillName;
}

//------------------------------------------------------------------------------
//!
uint
DFFillColorNode::numInputs() const
{
   return 0;
}

//------------------------------------------------------------------------------
//!
DFOutput*
DFFillColorNode::output()
{
   return &_output;
}

//------------------------------------------------------------------------------
//!
DFInput*
DFFillColorNode::input( uint )
{
   return nullptr;
}

//------------------------------------------------------------------------------
//!
RCP<DFNodeEditor>
DFFillColorNode::edit()
{
   if( _editor.isNull() )  _editor = new DFFillColorEditor( this );
   return _editor;
}

//------------------------------------------------------------------------------
//!
RCP<Bitmap>
DFFillColorNode::process( const DFImageParams& p )
{
   RCP<Bitmap> img = new Bitmap( p.size(), Bitmap::FLOAT, 4 );
   BitmapManipulator::fillRect( *img, Vec2i(0), p.size(), _color.getPremultiplied().ptr() );
   return img;
}

//------------------------------------------------------------------------------
//!
bool
DFFillColorNode::dumpCustom( TextStream& os, StreamIndent& indent ) const
{
   os << indent << "color=" << VMFmt(_color) << "," << nl;
   return os.ok();
}

/*==============================================================================
   CLASS DFDrawRectEditor
==============================================================================*/

class DFDrawRectEditor:
   public DFNodeEditor
{
public:

   /*----- methods -----*/

   DFDrawRectEditor( DFDrawRectNode* n ): _node(n) {}

   PLASMA_DLL_API virtual RCP<Manipulator>  manipulator() { return nullptr; }
   PLASMA_DLL_API virtual RCP<DFNodeAttrList>  attributes() const;
   PLASMA_DLL_API virtual RCP<DFNodeAttrStates>  attributesStates() const;
   PLASMA_DLL_API virtual void updateAttributes( const DFNodeAttrStates& );

protected:

   /*----- members -----*/

   DFDrawRectNode* _node;
};

//------------------------------------------------------------------------------
//!
RCP<DFNodeAttrList>
DFDrawRectEditor::attributes() const
{
   RCP<DFNodeAttrList> atts = new DFNodeAttrList();
   atts->add( DFNodeAttr( "RGBA",   ID_COLOR,  "Color" ) );
   atts->add( DFNodeAttr( "FLOAT2", ID_CENTER, "Center" ) );
   atts->add( DFNodeAttr( "FLOAT2", ID_SIZE,   "Size" ) );
   return atts;
}

//------------------------------------------------------------------------------
//!
RCP<DFNodeAttrStates>
DFDrawRectEditor::attributesStates() const
{
   RCP<DFNodeAttrStates> states = new DFNodeAttrStates();
   states->set( ID_COLOR, _node->color() );
   states->set( ID_CENTER, _node->center() );
   states->set( ID_SIZE, _node->size() );
   return states;
}

//------------------------------------------------------------------------------
//!
void
DFDrawRectEditor::updateAttributes( const DFNodeAttrStates& attribs )
{
   for( auto cur = attribs.begin(); cur != attribs.end(); ++cur )
   {
      switch( cur->_id )
      {
         case ID_COLOR:  _node->color( (*cur)._value.getVec4() );  break;
         case ID_CENTER: _node->center( (*cur)._value.getVec2() ); break;
         case ID_SIZE:   _node->size( (*cur)._value.getVec2() );   break;
         default:
            StdErr << "WARNING: DrawRect doesn't understand '" << cur->_id << "'." << nl;
      }
      _node->graph()->invalidate( _node );
   }
}

/*==============================================================================
   CLASS DFDrawRectNode
==============================================================================*/

//------------------------------------------------------------------------------
//!
DFDrawRectNode::DFDrawRectNode():
   _color( 1.0f ),
   _center( 0.0f ),
   _size( 16.0f )
{
   _output.delegate( makeDelegate( this, &DFDrawRectNode::process ) );
}

//------------------------------------------------------------------------------
//!
const ConstString&
DFDrawRectNode::name() const
{
   return _drawRectName;
}

//------------------------------------------------------------------------------
//!
uint
DFDrawRectNode::numInputs() const
{
   return 0;
}

//------------------------------------------------------------------------------
//!
DFOutput*
DFDrawRectNode::output()
{
   return &_output;
}

//------------------------------------------------------------------------------
//!
DFInput*
DFDrawRectNode::input( uint )
{
   return nullptr;
}

//------------------------------------------------------------------------------
//!
RCP<DFNodeEditor>
DFDrawRectNode::edit()
{
   if( _editor.isNull() )  _editor = new DFDrawRectEditor( this );
   return _editor;
}

//------------------------------------------------------------------------------
//!
RCP<Bitmap>
DFDrawRectNode::process( const DFImageParams& p )
{
   RCP<Bitmap> img = new Bitmap( p.size(), Bitmap::FLOAT, 4 );
   img->clearBuffer();
   Mat3f m = p.regionToBitmap();
   Vec2f lCenter = m * center();
   Vec2f lSize   = m ^ size();
   Vec2f lCorner = lCenter - lSize*0.5f;
   // Aliased for now.
   BitmapManipulator::fillRect( *img, Vec2i(lCorner+0.5f), Vec2i(lSize+0.5f), _color.getPremultiplied().ptr() );
   return img;
}

//------------------------------------------------------------------------------
//!
bool
DFDrawRectNode::dumpCustom( TextStream& os, StreamIndent& indent ) const
{
   os << indent << "color="  << VMFmt(_color)  << "," << nl;
   os << indent << "center=" << VMFmt(_center) << "," << nl;
   os << indent << "size="   << VMFmt(_size)   << "," << nl;
   return os.ok();
}

/*==============================================================================
   CLASS DFDrawCircleEditor
==============================================================================*/

class DFDrawCircleEditor:
   public DFNodeEditor
{
public:

   /*----- methods -----*/

   DFDrawCircleEditor( DFDrawCircleNode* n ): _node(n) {}

   PLASMA_DLL_API virtual RCP<Manipulator>  manipulator() { return nullptr; }
   PLASMA_DLL_API virtual RCP<DFNodeAttrList>  attributes() const;
   PLASMA_DLL_API virtual RCP<DFNodeAttrStates>  attributesStates() const;
   PLASMA_DLL_API virtual void updateAttributes( const DFNodeAttrStates& );

protected:

   /*----- members -----*/

   DFDrawCircleNode* _node;
};

//------------------------------------------------------------------------------
//!
RCP<DFNodeAttrList>
DFDrawCircleEditor::attributes() const
{
   RCP<DFNodeAttrList> atts = new DFNodeAttrList();
   atts->add( DFNodeAttr( "RGBA",   ID_COLOR,  "Color" ) );
   atts->add( DFNodeAttr( "FLOAT2", ID_CENTER, "Center" ) );
   atts->add( DFNodeAttr( "FLOAT",  ID_RADIUS, "Radius" ) );
   return atts;
}

//------------------------------------------------------------------------------
//!
RCP<DFNodeAttrStates>
DFDrawCircleEditor::attributesStates() const
{
   RCP<DFNodeAttrStates> states = new DFNodeAttrStates();
   states->set( ID_COLOR, _node->color() );
   states->set( ID_CENTER, _node->center() );
   states->set( ID_RADIUS, _node->radius() );
   return states;
}

//------------------------------------------------------------------------------
//!
void
DFDrawCircleEditor::updateAttributes( const DFNodeAttrStates& attribs )
{
   for( auto cur = attribs.begin(); cur != attribs.end(); ++cur )
   {
      switch( cur->_id )
      {
         case ID_COLOR:  _node->color( (*cur)._value.getVec4() );   break;
         case ID_CENTER: _node->center( (*cur)._value.getVec2() );  break;
         case ID_RADIUS: _node->radius( (*cur)._value.getFloat() ); break;
         default:
            StdErr << "WARNING: DrawCircle doesn't understand '" << cur->_id << "'." << nl;
      }
      _node->graph()->invalidate( _node );
   }
}

/*==============================================================================
   CLASS DFDrawCircleNode
==============================================================================*/

//------------------------------------------------------------------------------
//!
DFDrawCircleNode::DFDrawCircleNode():
   _color( 1.0f ),
   _center( 0.0f ),
   _radius( 16.0f )
{
   _output.delegate( makeDelegate( this, &DFDrawCircleNode::process ) );
}

//------------------------------------------------------------------------------
//!
const ConstString&
DFDrawCircleNode::name() const
{
   return _drawCircleName;
}

//------------------------------------------------------------------------------
//!
uint
DFDrawCircleNode::numInputs() const
{
   return 0;
}

//------------------------------------------------------------------------------
//!
DFOutput*
DFDrawCircleNode::output()
{
   return &_output;
}

//------------------------------------------------------------------------------
//!
DFInput*
DFDrawCircleNode::input( uint )
{
   return nullptr;
}

//------------------------------------------------------------------------------
//!
RCP<DFNodeEditor>
DFDrawCircleNode::edit()
{
   if( _editor.isNull() )  _editor = new DFDrawCircleEditor( this );
   return _editor;
}

//------------------------------------------------------------------------------
//!
RCP<Bitmap>
DFDrawCircleNode::process( const DFImageParams& p )
{
   RCP<Bitmap> img = new Bitmap( p.size(), Bitmap::FLOAT, 4 );
   img->clearBuffer();

   // Compute range.
   Mat3f r2b = p.regionToBitmap();
   Vec2i bl = CGM::floor( r2b*(_center - _radius) );
   Vec2i tr = CGM::ceil ( r2b*(_center + _radius) );
   // Clamp region to the Bitmap's size.
   Vec2i last = p.size() - 1;
   bl = CGM::clamp( bl, Vec2i(0), last );
   tr = CGM::clamp( tr, Vec2i(0), last );

   // Compute anti-aliased distance for every pixel in the rectangular region.
   Mat3f b2r = p.bitmapToRegion();
   float  w = p.region().width()/float(p.size().x); // Size of a pixel in region units.
   float wd = 1.0f / (2.0f * w);
   Vec2i cur;
   for( cur.y = bl.y; cur.y <= tr.y; ++cur.y )
   {
      Vec4f* dst = (Vec4f*)img->pixel( Vec2i( bl.x, cur.y ) );
      for( cur.x = bl.x; cur.x <= tr.x; ++cur.x, ++dst )
      {
         Vec2f  p = b2r * Vec2f( cur );
         float dc = length( p - _center );
         float de = _radius - dc;
         float  f = CGM::clamp( (de+w)*wd, 0.0f, 1.0f );
         *dst = _color * f;
      }
   }

   return img;
}

//------------------------------------------------------------------------------
//!
bool
DFDrawCircleNode::dumpCustom( TextStream& os, StreamIndent& indent ) const
{
   os << indent << "color="  << VMFmt(_color)  << "," << nl;
   os << indent << "center=" << VMFmt(_center) << "," << nl;
   os << indent << "radius=" << VMFmt(_radius) << "," << nl;
   return os.ok();
}

/*==============================================================================
   CLASS DFNoiseImageEditor
==============================================================================*/

class DFNoiseImageEditor:
   public DFNodeEditor
{
public:

   /*----- methods -----*/

   DFNoiseImageEditor( DFNoiseImageNode* n ): _node(n) {}

   PLASMA_DLL_API virtual RCP<Manipulator>  manipulator() { return nullptr; }
   PLASMA_DLL_API virtual RCP<DFNodeAttrList>  attributes() const;
   PLASMA_DLL_API virtual RCP<DFNodeAttrStates>  attributesStates() const;
   PLASMA_DLL_API virtual void updateAttributes( const DFNodeAttrStates& );

   inline void  updateUI() { _node->graph()->msg().modify( _node, attributesStates().ptr() ); }

protected:

   /*----- members -----*/

   DFNoiseImageNode* _node;
};

//------------------------------------------------------------------------------
//!
RCP<DFNodeAttrList>
DFNoiseImageEditor::attributes() const
{
   RCP<DFNodeAttrList> atts = new DFNodeAttrList();

   RCP<Table> enums = new Table();
   enums->pushBack( "Cell" );
   enums->pushBack( "Perlin" );
   enums->pushBack( "Perlin filtered" );
   enums->pushBack( "Voronoi" );
   enums->pushBack( "Fractional Brownian motion");
   atts->add( DFNodeAttr( "ENUM", ID_TYPE, "Type" ).enums( enums.ptr() ) );

   RCP<DFNodeAttrList> list;
   RCP<Table>  xtra;

   ConstString range_str( "range" );

   // Perlin Filtered.
   list = new DFNodeAttrList();
   xtra = new Table();
   xtra->set( range_str, Vec2f(0.2f, 0.75f) );
   list->add( DFNodeAttr( "FLOAT", ID_PERFIL_WIDTH, "Width" ).extras( xtra.ptr() ) );
   atts->add( DFNodeAttr( ID_PERFIL, "", list.ptr() ) );

   // Voronoi.
   list = new DFNodeAttrList();
   xtra = new Table();
   xtra->set( range_str, Vec2f(-2.0f, 2.0f) );
   list->add( DFNodeAttr( "FLOAT", ID_JITTER, "Jitter" ).extras( xtra.ptr() ) );
   atts->add( DFNodeAttr( ID_VORONOI, "", list.ptr() ) );

   // FBM.
   list = new DFNodeAttrList();
   xtra = new Table();
   xtra->set( range_str, Vec2f(0.0f, 1.0f) );
   list->add( DFNodeAttr( "FLOAT", ID_FBM_WIDTH, "Width" ).extras( xtra.ptr() ) );
   xtra = new Table();
   xtra->set( range_str, Vec2f(1.0f, 8.0) );
   list->add( DFNodeAttr( "INT"  , ID_OCTAVE, "Octaves" ).extras( xtra.ptr() ) );
   list->add( DFNodeAttr( "FLOAT", ID_LACUNARITY, "Lacunarity" ) );
   list->add( DFNodeAttr( "FLOAT", ID_GAIN, "Gain" ) );
   atts->add( DFNodeAttr( ID_FBM, "", list.ptr() ) );

   return atts;
}

//------------------------------------------------------------------------------
//!
RCP<DFNodeAttrStates>
DFNoiseImageEditor::attributesStates() const
{
   RCP<DFNodeAttrStates> states = new DFNodeAttrStates();

   states->set( ID_TYPE, (float)_node->type() );

   switch( _node->type() )
   {
      case DFNoiseImageNode::TYPE_CELL:
      case DFNoiseImageNode::TYPE_PERLIN:
         states->set( ID_PERFIL, false );
         states->set( ID_VORONOI, false );
         states->set( ID_FBM, false );
         break;
      case DFNoiseImageNode::TYPE_PERLIN_FILTERED:
         states->set( ID_PERFIL, true );
         states->set( ID_VORONOI, false );
         states->set( ID_FBM, false );
         states->set( ID_PERFIL_WIDTH, _node->widthPerlinFiltered() );
         break;
      case DFNoiseImageNode::TYPE_VORONOI:
         states->set( ID_PERFIL, false );
         states->set( ID_VORONOI, true );
         states->set( ID_FBM, false );
         states->set( ID_JITTER, _node->jitter() );
         break;
      case DFNoiseImageNode::TYPE_FBM:
         states->set( ID_PERFIL, false );
         states->set( ID_VORONOI, false );
         states->set( ID_FBM, true );
         states->set( ID_FBM_WIDTH , _node->widthFBM()       );
         states->set( ID_OCTAVE    , (float)_node->octaves() );
         states->set( ID_LACUNARITY, _node->lacunarity()     );
         states->set( ID_GAIN      , _node->gain()           );
         break;
   }

   return states;
}

//------------------------------------------------------------------------------
//!
void
DFNoiseImageEditor::updateAttributes( const DFNodeAttrStates& attribs )
{
   for( auto cur = attribs.begin(); cur != attribs.end(); ++cur )
   {
      switch( cur->_id )
      {
         case ID_TYPE:
         {
            _node->type( (DFNoiseImageNode::Type)int((*cur)._value.getFloat()) );
            _node->graph()->invalidate( _node );
            updateUI();
         }  break;
         case ID_PERFIL_WIDTH:
         {
            CHECK( _node->type() == DFNoiseImageNode::TYPE_PERLIN_FILTERED );
            _node->widthPerlinFiltered( (*cur)._value.getFloat() );
            _node->graph()->invalidate( _node );
         }  break;
         case ID_JITTER:
         {
            CHECK( _node->type() == DFNoiseImageNode::TYPE_VORONOI );
            _node->jitter( (*cur)._value.getFloat() );
            _node->graph()->invalidate( _node );
         }  break;
         case ID_FBM_WIDTH:
         {
            CHECK( _node->type() == DFNoiseImageNode::TYPE_FBM );
            _node->widthFBM( (*cur)._value.getFloat() );
            _node->graph()->invalidate( _node );
         }  break;
         case ID_OCTAVE:
         {
            CHECK( _node->type() == DFNoiseImageNode::TYPE_FBM );
            _node->octaves( uint((*cur)._value.getFloat()) );
            _node->graph()->invalidate( _node );
         }  break;
         case ID_LACUNARITY:
         {
            CHECK( _node->type() == DFNoiseImageNode::TYPE_FBM );
            _node->lacunarity( (*cur)._value.getFloat() );
            _node->graph()->invalidate( _node );
         }  break;
         case ID_GAIN:
         {
            CHECK( _node->type() == DFNoiseImageNode::TYPE_FBM );
            _node->gain( (*cur)._value.getFloat() );
            _node->graph()->invalidate( _node );
         }  break;
         default:;
      }
   }
}

/*==============================================================================
   CLASS DFNoiseImageNode
==============================================================================*/

//------------------------------------------------------------------------------
//!
DFNoiseImageNode::DFNoiseImageNode():
   _type( TYPE_CELL )
{
   setDefaults();
   _output.delegate( makeDelegate( this, &DFNoiseImageNode::process ) );
}

//------------------------------------------------------------------------------
//!
const ConstString&
DFNoiseImageNode::name() const
{
   return _noiseName;
}

//------------------------------------------------------------------------------
//!
DFOutput*
DFNoiseImageNode::output()
{
   return &_output;
}

//------------------------------------------------------------------------------
//!
RCP<DFNodeEditor>
DFNoiseImageNode::edit()
{
   if( _editor.isNull() )  _editor = new DFNoiseImageEditor( this );
   return _editor;
}

//------------------------------------------------------------------------------
//!
RCP<Bitmap>
DFNoiseImageNode::process( const DFImageParams& params )
{
   const Vec2i& size = params.size();
   RCP<Bitmap> img = new Bitmap( size, Bitmap::FLOAT, 4 );

   Vec4f* dst    = (Vec4f*)img->pixels();
   Vec2f  scale  = params.region().size() / Vec2f(size);
   Vec2f  corner = params.region().position();
   Vec3f  p;
   switch( _type )
   {
      case TYPE_CELL:
      {
         p.z = 0.0f;
         for( int y = 0; y < size.y; ++y )
         {
            p.y = (y+0.5f)*scale.y + corner.y;
            for( int x = 0; x < size.x; ++x, ++dst )
            {
               p.x = (x+0.5f)*scale.x + corner.x;
               float v = CGM::cellNoise1( p );
               *dst = Vec4f( v, v, v, 1.0f );
            }
         }
      }  break;
      case TYPE_PERLIN:
      {
         p.z = 0.0f;
         for( int y = 0; y < size.y; ++y )
         {
            p.y = (y+0.5f)*scale.y + corner.y;
            for( int x = 0; x < size.x; ++x, ++dst )
            {
               p.x = (x+0.5f)*scale.x + corner.x;
               float v = CGM::perlinNoise1( p );
               *dst = Vec4f( v, v, v, 1.0f );
            }
         }
      }  break;
      case TYPE_PERLIN_FILTERED:
      {
         p.z = 0.0f;
         for( int y = 0; y < size.y; ++y )
         {
            p.y = (y+0.5f)*scale.y + corner.y;
            for( int x = 0; x < size.x; ++x, ++dst )
            {
               p.x = (x+0.5f)*scale.x + corner.x;
               float v = CGM::filteredPerlinNoise1( p, _perFil._width );
               *dst = Vec4f( v, v, v, 1.0f );
            }
         }
      }  break;
      case TYPE_VORONOI:
      {
         Vec3f _;
         p.z = 0.0f;
         for( int y = 0; y < size.y; ++y )
         {
            p.y = (y+0.5f)*scale.y + corner.y;
            for( int x = 0; x < size.x; ++x, ++dst )
            {
               p.x = (x+0.5f)*scale.x + corner.x;
               float v = CGM::voronoiNoise1( p, _vor._jitter, _ );
               *dst = Vec4f( v, v, v, 1.0f );
            }
         }
      }  break;
      case TYPE_FBM:
      {
         p.z = 0.0f;
         for( int y = 0; y < size.y; ++y )
         {
            p.y = (y+0.5f)*scale.y + corner.y;
            for( int x = 0; x < size.x; ++x, ++dst )
            {
               p.x = (x+0.5f)*scale.x + corner.x;
               float v = CGM::fBmNoise1( p, _fbm._width, _fbm._octaves, _fbm._lacunarity, _fbm._gain );
               *dst = Vec4f( v, v, v, 1.0f );
            }
         }
      }  break;
      default:
         memset( img->pixels(), 0, img->size() );
         break;
   }
   return img;
}

//------------------------------------------------------------------------------
//!
bool
DFNoiseImageNode::dumpCustom( TextStream& os, StreamIndent& indent ) const
{
   os << indent << "type = " << _type << "," << nl;
   switch( _type )
   {
      case TYPE_CELL:
      case TYPE_PERLIN:
         break;
      case TYPE_PERLIN_FILTERED:
         os << indent << "perfilwidth = " << _perFil._width << "," << nl;
         break;
      case TYPE_VORONOI:
         os << indent << "jitter = " << _vor._jitter << "," << nl;
         break;
      case TYPE_FBM:
         os << indent << "fbmwidth = " << _fbm._width << "," << nl;
         os << indent << "octaves = " << _fbm._octaves << "," << nl;
         os << indent << "lacunarity = " << _fbm._lacunarity << "," << nl;
         os << indent << "gain = " << _fbm._gain << "," << nl;
         break;
      default:
         break;
   }
   return os.ok();
}

//------------------------------------------------------------------------------
//!
void
DFNoiseImageNode::setDefaults()
{
#if 0
   switch( _type )
   {
      case TYPE_CELL:
      case TYPE_PERLIN:
         break;
      case TYPE_PERLIN_FILTERED:
         _perFil._width = 0.2f;
         break;
      case TYPE_VORONOI:
         _vor._jitter = 1.0f;
         break;
      case TYPE_FBM:
         _fbm._width      = 0.0f;
         _fbm._octaves    = 4;
         _fbm._lacunarity = 2.0f;
         _fbm._gain       = 0.5f;
         break;
      default:
         break;
   }
#else
   _perFil._width   = 0.2f;
   _vor._jitter     = 1.0f;
   _fbm._width      = 0.0f;
   _fbm._octaves    = 4;
   _fbm._lacunarity = 2.0f;
   _fbm._gain       = 0.5f;
#endif
}


NAMESPACE_END
