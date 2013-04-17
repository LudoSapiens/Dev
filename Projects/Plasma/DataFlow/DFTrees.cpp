/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/DataFlow/DFTrees.h>

#include <Plasma/DataFlow/DFGraph.h>
#include <Plasma/DataFlow/DFNodeAttr.h>
#include <Plasma/Manipulator/Manipulator.h>
//#include <Plasma/Resource/ResManager.h>

#include <Fusion/VM/VMFmt.h>

//#include <CGMath/Distributions.h>


#if _MSC_VER
// 'this' used in member initializer list.
#pragma warning( disable: 4355 )
#endif

/*==============================================================================
  UNNAME NAMESPACE
==============================================================================*/

UNNAMESPACE_BEGIN

ConstString _treeWPName;
ConstString _treeJPHName;

enum
{
   ID_BASE_SPLITS,
   ID_RATIO,
   ID_SCALE,
   ID_SCALE_V,
   ID_SEED,
   ID_SHAPE,
   ID_GENERAL
};

// DFTreeNodeWP LevelParams.
enum
{
   ID_LP_GROUP,
   ID_LP_CURVE_RES,
   ID_LP_CURVE,
   ID_LP_CURVE_V,
   ID_LP_CURVE_BACK,
   ID_LP_DOWN_ANGLE,
   ID_LP_DOWN_ANGLE_V,
   ID_LP_LENGHT,
   ID_LP_LENGHT_V,
   ID_LP_SEG_SPLITS,
   ID_LP_TAPER,
};

enum
{
   ID_JPH_DETAIL,
   ID_JPH_HEIGHT,
   ID_JPH_HEIGHT_D,
   ID_JPH_LEVELS,
   ID_JPH_PHOTO_S,
   ID_JPH_SEED,
   ID_JPH_SUN,
   ID_JPH_TREE,
   // These must be contiguous.
   ID_JPH_TRUNK,
   ID_JPH_BRANCH,
   ID_JPH_SUBBRANCH,
   ID_JPH_TWIG,
};

// DFTreeNodeJPH LevelParams.
enum
{
   ID_JPH_LP_CROOK,
   ID_JPH_LP_CROOK_D,
   ID_JPH_LP_PHOTO,
   ID_JPH_LP_PHOTO_D,
   ID_JPH_LP_RADIUS,
};

//------------------------------------------------------------------------------
//!
RCP<DFNode> treeWPVM( VMState* vm, int idx )
{
   RCP<DFTreeNodeWP> node = new DFTreeNodeWP();
   node->init( vm, idx );
   return node;
}

//------------------------------------------------------------------------------
//!
RCP<DFNode> treeJPHVM( VMState* vm, int idx )
{
   RCP<DFTreeNodeJPH> node = new DFTreeNodeJPH();
   node->init( vm, idx );
   return node;
}

UNNAMESPACE_END

NAMESPACE_BEGIN

/*==============================================================================
   STATIC INITIALIZATION
==============================================================================*/

//-----------------------------------------------------------------------------
//!
void initializeTreeNodes()
{
   _treeWPName  = "treeWP";
   _treeJPHName = "treeJPH";

   DFNodeSpec::registerNode(
      DFSocket::STROKES,
      _treeWPName, treeWPVM,
      "Tree", "Tree generator (Weber+Penn).",
      nullptr
   );
   DFNodeSpec::registerNode(
      DFSocket::STROKES,
      _treeJPHName, treeJPHVM,
      "TreeJPH", "Tree generator (JPH).",
      nullptr
   );
}

//-----------------------------------------------------------------------------
//!
void terminateTreeNodes()
{
   _treeWPName  = ConstString();
   _treeJPHName = ConstString();
}


/*==============================================================================
   CLASS DFTreeNodeWPEditor
==============================================================================*/

class DFTreeNodeWPEditor:
   public DFNodeEditor
{
public:

   /*----- methods -----*/

   DFTreeNodeWPEditor( DFTreeNodeWP* n ): _node(n) {}

   PLASMA_DLL_API virtual RCP<Manipulator>  manipulator() { return nullptr; }
   PLASMA_DLL_API virtual RCP<DFNodeAttrList>  attributes() const;
   PLASMA_DLL_API virtual RCP<DFNodeAttrStates>  attributesStates() const;
   PLASMA_DLL_API virtual void updateAttributes( const DFNodeAttrStates& );

protected:

   /*----- members -----*/

   DFTreeNodeWP* _node;
};

//------------------------------------------------------------------------------
//!
RCP<DFNodeAttrList>
DFTreeNodeWPEditor::attributes() const
{
   RCP<DFNodeAttrList> atts = new DFNodeAttrList();

   RCP<Table> xtra;

   RCP<DFNodeAttrList> grp = new DFNodeAttrList();

   grp->add( DFNodeAttr( "INT", ID_SEED, "Seed" ) );

   xtra = new Table();
   xtra->pushBack( DFTreeNodeWP::toStr(DFTreeNodeWP::SHAPE_CONICAL            ) );
   xtra->pushBack( DFTreeNodeWP::toStr(DFTreeNodeWP::SHAPE_SPHERICAL          ) );
   xtra->pushBack( DFTreeNodeWP::toStr(DFTreeNodeWP::SHAPE_HEMISPHERICAL      ) );
   xtra->pushBack( DFTreeNodeWP::toStr(DFTreeNodeWP::SHAPE_CYLINDRICAL        ) );
   xtra->pushBack( DFTreeNodeWP::toStr(DFTreeNodeWP::SHAPE_TAPERED_CYLINDRICAL) );
   xtra->pushBack( DFTreeNodeWP::toStr(DFTreeNodeWP::SHAPE_FLAME              ) );
   xtra->pushBack( DFTreeNodeWP::toStr(DFTreeNodeWP::SHAPE_INVERSE_CONICAL    ) );
   xtra->pushBack( DFTreeNodeWP::toStr(DFTreeNodeWP::SHAPE_TEND_FLAME         ) );
   xtra->pushBack( DFTreeNodeWP::toStr(DFTreeNodeWP::SHAPE_ENVELOPE           ) );
   grp->add( DFNodeAttr( "ENUM" , ID_SHAPE, "Shape" ).enums( xtra.ptr() ) );

   grp->add( DFNodeAttr( "FLOAT", ID_SCALE      , "Scale"      ) );
   grp->add( DFNodeAttr( "FLOAT", ID_SCALE_V    , "ScaleV"     ) );
   grp->add( DFNodeAttr( "FLOAT", ID_RATIO      , "Ratio"      ) );
   grp->add( DFNodeAttr( "FLOAT", ID_BASE_SPLITS, "BaseSplits" ) );

   atts->add( DFNodeAttr( ID_GENERAL, "General", grp.ptr() ) );

   xtra = new Table();
   xtra->set( "range", Vec2f(0.0f, 65535.0f) );
   for( uint i = 0; i < 4; ++i )
   {
      uint gID = (i+1)*0x100; // Offset ID.
      grp = new DFNodeAttrList();
      grp->add( DFNodeAttr( "INT"  , gID+ID_LP_CURVE_RES   , "CurveRes" ).extras( xtra.ptr() ) );
      grp->add( DFNodeAttr( "FLOAT", gID+ID_LP_CURVE       , "Curve"      ) );
      grp->add( DFNodeAttr( "FLOAT", gID+ID_LP_CURVE_V     , "CurveV"     ) );
      grp->add( DFNodeAttr( "FLOAT", gID+ID_LP_CURVE_BACK  , "CurveBack"  ) );
      grp->add( DFNodeAttr( "FLOAT", gID+ID_LP_DOWN_ANGLE  , "DownAngle"  ) );
      grp->add( DFNodeAttr( "FLOAT", gID+ID_LP_DOWN_ANGLE_V, "DownAngleV" ) );
      grp->add( DFNodeAttr( "FLOAT", gID+ID_LP_LENGHT      , "Length"     ) );
      grp->add( DFNodeAttr( "FLOAT", gID+ID_LP_LENGHT_V    , "LengthV"    ) );
      grp->add( DFNodeAttr( "FLOAT", gID+ID_LP_SEG_SPLITS  , "SegSplits"  ) );
      grp->add( DFNodeAttr( "FLOAT", gID+ID_LP_TAPER       , "Taper"      ) );
      String label = String("Level ") + String( i );
      atts->add( DFNodeAttr( gID+ID_LP_GROUP, label.cstr(), grp.ptr() ) );
   }

   return atts;
}

//------------------------------------------------------------------------------
//!
RCP<DFNodeAttrStates>
DFTreeNodeWPEditor::attributesStates() const
{
   RCP<DFNodeAttrStates> states = new DFNodeAttrStates();
   states->set( ID_BASE_SPLITS, _node->_baseSplits   );
   states->set( ID_RATIO      , _node->_ratio        );
   states->set( ID_SCALE      , _node->_scale        );
   states->set( ID_SCALE_V    , _node->_scaleV       );
   states->set( ID_SEED       , float(_node->_seed)  );
   states->set( ID_SHAPE      , float(_node->_shape) );
   for( uint i = 0; i < 4; ++i )
   {
      uint gID = (i+1)*0x100; // Offset ID.
      const DFTreeNodeWP::LevelParams& p = _node->_[i];
      states->set( gID+ID_LP_CURVE_RES   , float(p._curveRes) );
      states->set( gID+ID_LP_CURVE       , p._curve           );
      states->set( gID+ID_LP_CURVE_V     , p._curveV          );
      states->set( gID+ID_LP_CURVE_BACK  , p._curveBack       );
      states->set( gID+ID_LP_DOWN_ANGLE  , p._downAngle       );
      states->set( gID+ID_LP_DOWN_ANGLE_V, p._downAngleV      );
      states->set( gID+ID_LP_LENGHT      , p._length          );
      states->set( gID+ID_LP_LENGHT_V    , p._lengthV         );
      states->set( gID+ID_LP_SEG_SPLITS  , p._segSplits       );
      states->set( gID+ID_LP_TAPER       , p._taper           );
   }
   return states;
}

//------------------------------------------------------------------------------
//!
void
DFTreeNodeWPEditor::updateAttributes( const DFNodeAttrStates& attribs )
{
   for( auto cur = attribs.begin(); cur != attribs.end(); ++cur )
   {
      switch( cur->_id )
      {
         case ID_BASE_SPLITS:
            _node->_baseSplits = (*cur)._value.getFloat();
            break;
         case ID_RATIO:
            _node->_ratio = (*cur)._value.getFloat();
            break;
         case ID_SCALE:
            _node->_scale = (*cur)._value.getFloat();
            break;
         case ID_SCALE_V:
            _node->_scaleV = (*cur)._value.getFloat();
            break;
         case ID_SEED:
            _node->_seed = uint32_t( (*cur)._value.getFloat() );
            break;
         case ID_SHAPE:
            _node->_shape = (DFTreeNodeWP::Shape)int((*cur)._value.getFloat());
            break;
         default:
            if( cur->_id > 0x100 )
            {
               uint i  = (cur->_id >> 8)-1;
               uint id = cur->_id & 0xff;
               DFTreeNodeWP::LevelParams& params = _node->_[i];
               switch( id )
               {
                  case ID_LP_CURVE_RES   : params._curveRes   = uint16_t( (*cur)._value.getFloat() ); break;
                  case ID_LP_CURVE       : params._curve      = (*cur)._value.getFloat();             break;
                  case ID_LP_CURVE_V     : params._curveV     = (*cur)._value.getFloat();             break;
                  case ID_LP_CURVE_BACK  : params._curveBack  = (*cur)._value.getFloat();             break;
                  case ID_LP_DOWN_ANGLE  : params._downAngle  = (*cur)._value.getFloat();             break;
                  case ID_LP_DOWN_ANGLE_V: params._downAngleV = (*cur)._value.getFloat();             break;
                  case ID_LP_LENGHT      : params._length     = (*cur)._value.getFloat();             break;
                  case ID_LP_LENGHT_V    : params._lengthV    = (*cur)._value.getFloat();             break;
                  case ID_LP_SEG_SPLITS  : params._segSplits  = (*cur)._value.getFloat();             break;
                  case ID_LP_TAPER       : params._taper      = (*cur)._value.getFloat();             break;
               }
            }
      }
   }
   _node->graph()->invalidate( _node );  // Even called on invalid attributes.
}

/*==============================================================================
   CLASS DFTreeNodeWP
==============================================================================*/

//------------------------------------------------------------------------------
//!
DFTreeNodeWP::DFTreeNodeWP():
   _seed( 0 ),
   _shape( SHAPE_CONICAL ),
   _scale(10.0f), _scaleV(0.0f),
   _ratio( 1.0f/20.0f ),
   _baseSplits( 0.0f )
{
   _output.delegate( makeDelegate( this, &DFTreeNodeWP::process ) );
}

//------------------------------------------------------------------------------
//!
const ConstString&
DFTreeNodeWP::name() const
{
   return _treeWPName;
}

//------------------------------------------------------------------------------
//!
DFOutput*
DFTreeNodeWP::output()
{
   return &_output;
}

//------------------------------------------------------------------------------
//!
RCP<DFNodeEditor>
DFTreeNodeWP::edit()
{
   return new DFTreeNodeWPEditor( this );
}

//------------------------------------------------------------------------------
//!
bool
DFTreeNodeWP::dumpCustom( TextStream& os, StreamIndent& indent ) const
{
   os << indent << "seed="       << _seed         << ","   << nl;
   os << indent << "shape=\""    << toStr(_shape) << "\"," << nl;
   os << indent << "scale="      << _scale        << ","   << nl;
   os << indent << "scaleV="     << _scaleV       << ","   << nl;
   os << indent << "ratio="      << _ratio        << ","   << nl;
   os << indent << "baseSplits=" << _baseSplits   << ","   << nl;

   for( uint i = 0; i < 4; ++i )
   {
      const LevelParams& p = _[i];
      os << indent << "{";
      os <<  " curveRes="   << p._curveRes  ;
      os << ", curve="      << p._curve     ;
      os << ", curveV="     << p._curveV    ;
      os << ", curveBack="  << p._curveBack ;
      os << ", downAngle="  << p._downAngle ;
      os << ", downAngleV=" << p._downAngleV;
      os << ", length="     << p._length    ;
      os << ", lengthV="    << p._lengthV   ;
      os << ", segSplits="  << p._segSplits ;
      os << ", taper="      << p._taper     ;
      os << " }," << nl;
   }
   return os.ok();
}

//------------------------------------------------------------------------------
//!
void
DFTreeNodeWP::init( VMState* vm, int idx )
{
   VM::get( vm, idx, "seed", _seed );
   if( VM::get( vm, idx, "shape" ) )
   {
      _shape = toShape( VM::toCString( vm, -1 ) );
      VM::pop( vm );
   }
   VM::get( vm, idx, "scale"     , _scale      );
   VM::get( vm, idx, "scaleV"    , _scaleV     );
   VM::get( vm, idx, "ratio"     , _ratio      );
   VM::get( vm, idx, "baseSplits", _baseSplits );
   for( uint i = 0; i < 4; ++i )
   {
      if( VM::geti( vm, idx, i+1 ) )
      {
         if( VM::type( vm, -1 ) == VM::TABLE )
         {
            LevelParams& p = _[i];
            uint32_t u32;
            u32 = p._curveRes;
            VM::get( vm, -1, "curveRes"  , u32           );
            p._curveRes = uint16_t(u32);
            VM::get( vm, -1, "curve"     , p._curve      );
            VM::get( vm, -1, "curveV"    , p._curveV     );
            VM::get( vm, -1, "curveBack" , p._curveBack  );
            VM::get( vm, -1, "downAngle" , p._downAngle  );
            VM::get( vm, -1, "downAngleV", p._downAngleV );
            VM::get( vm, -1, "length"    , p._length     );
            VM::get( vm, -1, "lengthV"   , p._lengthV    );
            VM::get( vm, -1, "segSplits" , p._segSplits  );
            VM::get( vm, -1, "taper"     , p._taper      );
         }
         else
         {
            StdErr << "ERROR - DFTreeNodeWP::init() expected table, got: " << VM::type( vm, -1 ) << nl;
         }
         VM::pop( vm );
      }
   }
}

//------------------------------------------------------------------------------
//!
RCP<DFStrokes>
DFTreeNodeWP::process()
{
   RCP<DFStrokes> strokes = new DFStrokes();

   _rng.seed( _seed );

   // Make trunk.
   Reff ref = Reff::identity();
   float length0 = _scale + _scaleV*rs();
   float radius0 = length0 * _ratio;
   makeStem( ref, length0, radius0, _[0], *strokes );

   //strokes->print();

   return strokes;
}

//------------------------------------------------------------------------------
//!
void
DFTreeNodeWP::makeStem( const Reff& start, float length, float radius, const LevelParams& p, DFStrokes& strokes )
{
   DFStrokes::Stroke& st = strokes.stroke( strokes.addStroke() );
   DFStrokes::Vertex& bv = strokes.vertex( st, strokes.addVertex(st) );

   //float len  = length * p._length + p._lengthV*rs();
   float radS = radius; //p._scale;  // +/-_scaleV
   float radE = CGM::max( radS * (1.0f - p._taper), 1.0f/1024.0f ); // Avoid corner case in blocks when radius is 0.
   float radD = radE - radS;

   Reff ref = start;

   bv._ref     = ref;
   bv._flags   = 0x1;
   bv._creases = 0xF;
   bv.setRadius( radS );

   float f_da =  CGM::degToRad( p._curveV / p._curveRes );

   float cs  = p._segSplits;
   float ss  = p._segSplits;
   float sa  = 45.0f;

   if( p._curveBack == 0.0f )
   {
      float f_a = -CGM::degToRad( p._curve / p._curveRes );
      float f_l = length / p._curveRes;
      makeSegments( Vec3i(1,p._curveRes,p._curveRes), f_l, f_a, f_da, radS, radD, cs, ss, sa, ref, strokes, st );
   }
   else
   {
      uint16_t curveRes_2 = p._curveRes >> 1;
      float f_a = -CGM::degToRad( p._curve / curveRes_2 );
      float f_l = length / p._curveRes;
      makeSegments( Vec3i(1,curveRes_2,p._curveRes), f_l, f_a, f_da, radS, radD, cs, ss, sa, ref, strokes, st );

      uint16_t curveRes_2b = p._curveRes - curveRes_2;
      f_a = CGM::degToRad( p._curveBack / curveRes_2b );
      makeSegments( Vec3i(curveRes_2+1,p._curveRes,p._curveRes), f_l, f_a, f_da, radS, radD, cs, ss, sa, ref, strokes, st );
   }

   strokes.invalidate();
}

//------------------------------------------------------------------------------
//!
void
DFTreeNodeWP::makeSegments(
   const Vec3i& loop,
   float l, float a, float da, float r, float dr,
   float& cs, float ss, float sa,
   Reff& ref, DFStrokes& strokes, DFStrokes::Stroke& st
)
{
   for( int i = loop.x; i <= loop.y; ++i, cs += ss )
   {
      float f = float(i) / loop.z;

      if( cs > 1.0f )
      {
         --cs;
         cs -= 10000.0f;
         // Make a split.
         Reff nref = ref;

         // 1. Continue current segment, slighty angled.
         DFStrokes::Vertex&  v = strokes.vertex( st, strokes.addVertex(st) );
         //DFStrokes::Segment& s = strokes.segment( st, strokes.addSegment(st) );
         strokes.addSegment(st);
         ref.rotateLocal( Vec3f(0.0f, 0.0f, 1.0f), da*rs() + a + sa );
         ref.translateLocal( Vec3f(0.0f, l, 0.0f) );
         v._ref = ref;
         v._flags = 0x1;
         v._creases = 0xF;
         v.setRadius( dr*f + r );

#if 1
         // 2. Make a new segment.
         DFStrokes::Stroke& nst = strokes.stroke( strokes.addStroke() );
         DFStrokes::Vertex& nbv = strokes.vertex( nst, strokes.addVertex(nst) );

         nbv = strokes.vertex( st, st.numVertices()-2 ); // Copy previous vertex.
         //nbv._creases = 0xF;
         nbv._flags   = 0x1; // Yields artifact?
         nbv._ref.rotateLocal( Vec3f(0.0f, 1.0f, 0.0f), CGConstf::pi() );
         //nbv._ref.translateLocal( Vec3f(10.0f, 0.0f, 10.0f) );
         nref = nbv._ref;
         nref.rotateLocal( Vec3f(0.0f, 0.0f, 1.0f), da*rs() + a + sa );

         float ncs = 0.0f; // Or fork cs?
         Vec3i nloop = Vec3i( i, loop.y, loop.z );
         //nloop = Vec3i( 1, 1, 1 );
         makeSegments( nloop, l, a, da, r, dr, ncs, 0.0f, sa, nref, strokes, nst );
#endif
      }
      else
      {
         DFStrokes::Vertex&  v = strokes.vertex( st, strokes.addVertex(st) );
         //DFStrokes::Segment& s = strokes.segment( st, strokes.addSegment(st) );
         strokes.addSegment(st);
         ref.rotateLocal( Vec3f(0.0f, 0.0f, 1.0f), da*rs() + a );
         ref.translateLocal( Vec3f(0.0f, l, 0.0f) );
         v._ref = ref;
         v._flags = 0x1;
         v._creases = 0xF;
         v.setRadius( dr*f + r );
      }
   }
}

//------------------------------------------------------------------------------
//!
const char*
DFTreeNodeWP::toStr( Shape v )
{
   switch( v )
   {
      case SHAPE_CONICAL:
         return "conical";
      case SHAPE_SPHERICAL:
         return "spherical";
      case SHAPE_HEMISPHERICAL:
         return "hemispherical";
      case SHAPE_CYLINDRICAL:
         return "cylindrical";
      case SHAPE_TAPERED_CYLINDRICAL:
         return "tapered cylindrical";
      case SHAPE_FLAME:
         return "flame";
      case SHAPE_INVERSE_CONICAL:
         return "inverse conical";
      case SHAPE_TEND_FLAME:
         return "tend flame";
      case SHAPE_ENVELOPE:
         return "envelope";
      default:
         return "<unknown>";
   }
}

//------------------------------------------------------------------------------
//!
DFTreeNodeWP::Shape
DFTreeNodeWP::toShape( const char* s )
{
   switch( tolower(s[0]) )
   {
      case 'c': return (tolower(s[1]) == 'o') ? SHAPE_CONICAL : SHAPE_CYLINDRICAL;
      case 'e': return SHAPE_ENVELOPE;
      case 'f': return SHAPE_FLAME;
      case 'h': return SHAPE_HEMISPHERICAL;
      case 'i': return SHAPE_INVERSE_CONICAL;
      case 's': return SHAPE_SPHERICAL;
      case 't': return (tolower(s[1]) == 'a') ? SHAPE_TAPERED_CYLINDRICAL : SHAPE_TEND_FLAME;
   }
   return SHAPE_CONICAL;
}


/*==============================================================================
   CLASS DFTreeNodeJPHEditor
==============================================================================*/

class DFTreeNodeJPHEditor:
   public DFNodeEditor
{
public:

   /*----- methods -----*/

   DFTreeNodeJPHEditor( DFTreeNodeJPH* n ): _node(n) {}

   PLASMA_DLL_API virtual RCP<Manipulator>  manipulator() { return nullptr; }
   PLASMA_DLL_API virtual RCP<DFNodeAttrList>  attributes() const;
   PLASMA_DLL_API virtual RCP<DFNodeAttrStates>  attributesStates() const;
   PLASMA_DLL_API virtual void updateAttributes( const DFNodeAttrStates& );

protected:

   /*----- methods -----*/

   void updateUI();

   /*----- members -----*/

   DFTreeNodeJPH* _node;
};

//------------------------------------------------------------------------------
//!
RCP<DFNodeAttrList>
DFTreeNodeJPHEditor::attributes() const
{
   RCP<DFNodeAttrList> atts = new DFNodeAttrList();

   RCP<Table> r11 = new Table();
   r11->set( "range", Vec2f(-1.0f, 1.0f) );

   RCP<Table> r02 = new Table();
   r02->set( "range", Vec2f(0.0f, 2.0f) );

   RCP<DFNodeAttrList> grp;
   uint gID;

   // Tree.
   grp = new DFNodeAttrList();
   grp->add( DFNodeAttr( "INT"   , ID_JPH_SEED    , "Seed"    ).range( 0.0f, float(CGConsti::max()) ) );
   grp->add( DFNodeAttr( "INT"   , ID_JPH_DETAIL  , "Detail"  ).range( 1.0f, 32.0f ) );
   grp->add( DFNodeAttr( "INT"   , ID_JPH_LEVELS  , "Levels"  ).range( 1.0f, 4.0f ) );
   grp->add( DFNodeAttr( "FLOAT" , ID_JPH_HEIGHT  , "Height"  ) );
   grp->add( DFNodeAttr( "FLOAT" , ID_JPH_HEIGHT_D, "HeightD" ) );
   grp->add( DFNodeAttr( "FLOAT" , ID_JPH_PHOTO_S , "PhotoS"  ).extras( r02.ptr() ) );
   grp->add( DFNodeAttr( "FLOAT3", ID_JPH_SUN     , "Sun"     ) );

   atts->add( DFNodeAttr( ID_JPH_TREE, "Tree", grp.ptr() ) );

   // Trunk.
   gID = 1*0x100; // Offset ID.
   grp = new DFNodeAttrList();
   grp->add( DFNodeAttr( "FLOAT" , gID+ID_JPH_LP_CROOK  , "Crook"  ) );
   grp->add( DFNodeAttr( "FLOAT" , gID+ID_JPH_LP_CROOK_D, "CrookD" ).extras( r11.ptr() ) );
   grp->add( DFNodeAttr( "FLOAT" , gID+ID_JPH_LP_PHOTO  , "Photo"  ).extras( r11.ptr() ) );
   grp->add( DFNodeAttr( "FLOAT" , gID+ID_JPH_LP_PHOTO_D, "PhotoD" ).extras( r11.ptr() ) );
   grp->add( DFNodeAttr( "FLOAT" , gID+ID_JPH_LP_RADIUS , "Radius" ).extras( r02.ptr() ) );

   atts->add( DFNodeAttr( ID_JPH_TRUNK, "Trunk", grp.ptr() ) );

   // Branch.
   gID = 2*0x100; // Offset ID.
   grp = new DFNodeAttrList();
   grp->add( DFNodeAttr( "FLOAT" , gID+ID_JPH_LP_CROOK  , "Crook" ) );
   grp->add( DFNodeAttr( "FLOAT" , gID+ID_JPH_LP_CROOK_D, "CrookD" ).extras( r11.ptr() ) );
   grp->add( DFNodeAttr( "FLOAT" , gID+ID_JPH_LP_PHOTO  , "Photo"  ).extras( r11.ptr() ) );
   grp->add( DFNodeAttr( "FLOAT" , gID+ID_JPH_LP_PHOTO_D, "PhotoD" ).extras( r11.ptr() ) );
   grp->add( DFNodeAttr( "FLOAT" , gID+ID_JPH_LP_RADIUS , "Radius" ).extras( r02.ptr() ) );

   atts->add( DFNodeAttr( ID_JPH_BRANCH, "Branch", grp.ptr() ) );


   // Subbranch.
   gID = 3*0x100; // Offset ID.
   grp = new DFNodeAttrList();
   grp->add( DFNodeAttr( "FLOAT" , gID+ID_JPH_LP_CROOK  , "Crook" ) );
   grp->add( DFNodeAttr( "FLOAT" , gID+ID_JPH_LP_CROOK_D, "CrookD" ).extras( r11.ptr() ) );
   grp->add( DFNodeAttr( "FLOAT" , gID+ID_JPH_LP_PHOTO  , "Photo"  ).extras( r11.ptr() ) );
   grp->add( DFNodeAttr( "FLOAT" , gID+ID_JPH_LP_PHOTO_D, "PhotoD" ).extras( r11.ptr() ) );
   grp->add( DFNodeAttr( "FLOAT" , gID+ID_JPH_LP_RADIUS , "Radius" ).extras( r02.ptr() ) );

   atts->add( DFNodeAttr( ID_JPH_SUBBRANCH, "Subbranch", grp.ptr() ) );

   // Twig.
   gID = 4*0x100; // Offset ID.
   grp = new DFNodeAttrList();
   grp->add( DFNodeAttr( "FLOAT" , gID+ID_JPH_LP_CROOK  , "Crook"  ) );
   grp->add( DFNodeAttr( "FLOAT" , gID+ID_JPH_LP_CROOK_D, "CrookD" ).extras( r11.ptr() ) );
   grp->add( DFNodeAttr( "FLOAT" , gID+ID_JPH_LP_PHOTO  , "Photo"  ).extras( r11.ptr() ) );
   grp->add( DFNodeAttr( "FLOAT" , gID+ID_JPH_LP_PHOTO_D, "PhotoD" ).extras( r11.ptr() ) );
   grp->add( DFNodeAttr( "FLOAT" , gID+ID_JPH_LP_RADIUS , "Radius" ).extras( r02.ptr() ) );

   atts->add( DFNodeAttr( ID_JPH_TWIG, "Twig", grp.ptr() ) );

   return atts;
}

//------------------------------------------------------------------------------
//!
RCP<DFNodeAttrStates>
DFTreeNodeJPHEditor::attributesStates() const
{
   RCP<DFNodeAttrStates> states = new DFNodeAttrStates();
   states->set( ID_JPH_DETAIL  , float(_node->_detail) );
   states->set( ID_JPH_LEVELS  , float(_node->_levels) );
   states->set( ID_JPH_HEIGHT  , _node->_height        );
   states->set( ID_JPH_HEIGHT_D, _node->_heightD       );
   states->set( ID_JPH_PHOTO_S , _node->_photoS        );
   states->set( ID_JPH_SEED    , float(_node->_seed)   );
   states->set( ID_JPH_SUN     , _node->_sun           );
   for( uint16_t i = 0; i < _node->_levels; ++i )
   {
      uint gID = (i+1)*0x100;
      const auto& p = _node->_params[i];
      states->set( gID+ID_JPH_LP_CROOK  , p._crook  );
      states->set( gID+ID_JPH_LP_CROOK_D, p._crookD );
      states->set( gID+ID_JPH_LP_PHOTO  , p._photo  );
      states->set( gID+ID_JPH_LP_PHOTO_D, p._photoD );
      states->set( gID+ID_JPH_LP_RADIUS , p._radius );
      states->set( ID_JPH_TRUNK+i, true );
   }
   for( uint16_t i = _node->_levels; i < 4; ++i )
   {
      states->set( ID_JPH_TRUNK+i, false );
   }
   return states;
}

//------------------------------------------------------------------------------
//!
void
DFTreeNodeJPHEditor::updateAttributes( const DFNodeAttrStates& attribs )
{
   for( auto cur = attribs.begin(); cur != attribs.end(); ++cur )
   {
      switch( cur->_id )
      {
         case ID_JPH_DETAIL:
            _node->_detail = uint16_t((*cur)._value.getFloat());
            break;
         case ID_JPH_HEIGHT:
            _node->_height = (*cur)._value.getFloat();
            break;
         case ID_JPH_HEIGHT_D:
            _node->_heightD = (*cur)._value.getFloat();
            break;
         case ID_JPH_LEVELS:
            _node->_levels = uint16_t((*cur)._value.getFloat());
            updateUI();
            break;
         case ID_JPH_PHOTO_S:
            _node->_photoS = (*cur)._value.getFloat();
            break;
         case ID_JPH_SEED:
            _node->_seed = uint32_t((*cur)._value.getFloat());
            break;
         case ID_JPH_SUN:
            _node->_sun = (*cur)._value.getVec3();
            break;
         case ID_JPH_TREE:
         case ID_JPH_TRUNK:
         case ID_JPH_BRANCH:
         case ID_JPH_SUBBRANCH:
         case ID_JPH_TWIG:
            CHECK( false );
            break;
         default:
            if( cur->_id >= 0x100 )
            {
               uint i  = (cur->_id >> 8)-1;
               uint id = cur->_id & 0xff;
               DFTreeNodeJPH::LevelParams& p = _node->_params[i];
               switch( id )
               {
                  case ID_JPH_LP_CROOK  : p._crook  = (*cur)._value.getFloat(); break;
                  case ID_JPH_LP_CROOK_D: p._crookD = (*cur)._value.getFloat(); break;
                  case ID_JPH_LP_PHOTO  : p._photo  = (*cur)._value.getFloat(); break;
                  case ID_JPH_LP_PHOTO_D: p._photoD = (*cur)._value.getFloat(); break;
                  case ID_JPH_LP_RADIUS : p._radius = (*cur)._value.getFloat(); break;
               }
            }
            break;
      }
   }
   _node->graph()->invalidate( _node );
}

//------------------------------------------------------------------------------
//!
void
DFTreeNodeJPHEditor::updateUI()
{
   _node->graph()->msg().modify( _node, attributesStates().ptr() );
}


/*==============================================================================
   CLASS DFTreeNodeJPH
==============================================================================*/

//------------------------------------------------------------------------------
//!
DFTreeNodeJPH::DFTreeNodeJPH():
   _seed( 0 ),
   _detail( 6 ),
   _levels( 2 ),
   _height(10.0f), _heightD(0.0f),
   _photoS( 1.0f ),
   _sun( 0.0f, 1.0f, 0.0f )
{
   _params[0]._radius = 1.0f / 20.0f;

   _output.delegate( makeDelegate( this, &DFTreeNodeJPH::process ) );
}

//------------------------------------------------------------------------------
//!
const ConstString&
DFTreeNodeJPH::name() const
{
   return _treeJPHName;
}

//------------------------------------------------------------------------------
//!
DFOutput*
DFTreeNodeJPH::output()
{
   return &_output;
}

//------------------------------------------------------------------------------
//!
RCP<DFNodeEditor>
DFTreeNodeJPH::edit()
{
   return new DFTreeNodeJPHEditor( this );
}

//------------------------------------------------------------------------------
//!
bool
DFTreeNodeJPH::dumpCustom( TextStream& os, StreamIndent& indent ) const
{
   os << indent << "detail="  << _height     << "," << nl;
   os << indent << "levels="  << _levels     << "," << nl;
   os << indent << "height="  << _height     << "," << nl;
   os << indent << "heightD=" << _heightD    << "," << nl;
   os << indent << "photoS="  << _photoS     << "," << nl;
   os << indent << "seed="    << _seed       << "," << nl;
   os << indent << "sun="     << VMFmt(_sun) << "," << nl;

   for( uint i = 0; i < _levels; ++i )
   {
      const LevelParams& p = _params[i];
      os << indent << "{";
      os <<  " crook="  << p._crook ;
      os << ", crookD=" << p._crookD;
      os << ", photo="  << p._photo ;
      os << ", photoD=" << p._photoD;
      os << ", radius=" << p._radius;
      os << " }," << nl;
   }

   return os.ok();
}

//------------------------------------------------------------------------------
//!
void
DFTreeNodeJPH::init( VMState* vm, int idx )
{
   VM::get( vm, idx, "detail" , _detail  );
   VM::get( vm, idx, "levels" , _levels  );
   VM::get( vm, idx, "height" , _height  );
   VM::get( vm, idx, "heightD", _heightD );
   VM::get( vm, idx, "photoS" , _photoS  );
   VM::get( vm, idx, "seed"   , _seed    );
   VM::get( vm, idx, "sun"    , _sun     );

   for( uint i = 0; i < _levels; ++i )
   {
      if( VM::geti( vm, idx, i+1 ) )
      {
         if( VM::type( vm, -1 ) == VM::TABLE )
         {
            LevelParams& p = _params[i];
            VM::get( vm, -1, "crook" , p._crook  );
            VM::get( vm, -1, "crookD", p._crookD );
            VM::get( vm, -1, "photo" , p._photo  );
            VM::get( vm, -1, "photoD", p._photoD );
            VM::get( vm, -1, "radius", p._radius );
         }
         else
         {
            StdErr << "ERROR - DFTreeNodeJPH::init() expected table, got: " << VM::type( vm, -1 ) << nl;
         }
         VM::pop( vm );
      }
   }
}

//------------------------------------------------------------------------------
//!
RCP<DFStrokes>
DFTreeNodeJPH::process()
{
   _rings.clear();
   _stems.clear();
   _sunN = _sun.getNormalized();
   _rng.seed( _seed );

   makeTree();

   RCP<DFStrokes> strokes = stemsToStrokes();
   //strokes->print();
   return strokes;
}

//------------------------------------------------------------------------------
//!
void
DFTreeNodeJPH::makeTree()
{
   // Make trunk.
   Reff ref = Reff::identity();
   float  h = rs( _height, _heightD );
   makeStem( 0, ref, h, h*_params[0]._radius, _detail );

   for( size_t i = 0; i < _stems.size(); ++i )
   {
      makeBranches( _stems[i] );
   }
}

//------------------------------------------------------------------------------
//!
void
DFTreeNodeJPH::makeBranches( Stem& stem )
{
   uint nextLevel = stem._level + 1;

   if( nextLevel >= _levels ) return; // Maximum level of recursion.

   int numRings = int(stem._rings.size());

   // Compute number of branches.
   // Could depend on density?
   int numBranches = 4;
   for( int i = 0; i < numBranches; ++i )
   {
      // Compute referential of the branch.

      // 1. Compute its parametric position.
      float t  = float(i+1)/float(numBranches+1);
      float r  = t*(numRings-1);
      int idx0 = int(r);
      int idx1 = idx0 < numRings-1 ? idx0+1 : idx0;
      float b  = r-idx0;
      Ring& r0 = _rings[stem._rings[idx0]];
      Ring& r1 = _rings[stem._rings[idx1]];

      // 2. Compute the referential of the current stem.
      Reff curRef = r0._ref.slerp( r1._ref, b );

      // 3. Modify the current referential to generate then branch ref.
      Reff ref( curRef );

      // 3.1 Rotate around z.
      ref.rotateLocal( Vec3f( 0.0, 0.0f, 1.0f ), CGM::cirToRad(0.15f) );
      // 3.1 Rotate around y.
      ref.rotate( curRef.orientation().getAxisY(), i*CGM::cirToRad(0.25f) );

      // Compute length and radius.
      float len    = stem._length*0.8f;
      float curRad = CGM::linear2( r0._rad, r1._rad, b );
      float rad    = curRad*(len/stem._length) * _params[nextLevel]._radius;

      // Create branch.
      makeStem( stem._level+1, ref, len, rad, _detail );
   }
}

//------------------------------------------------------------------------------
//! Ref depends of the branching of the parent.
//! Length and radius depend on the parent.
//! Radius depends on the position of the branching.
uint32_t
DFTreeNodeJPH::makeStem(
   int   level,
   Reff  ref,
   float length,
   float radius,
   uint  detail
)
{
   // radius
   // splitting
   _stems.pushBack( Stem() );
   if( detail > 0 )
   {
      const LevelParams& p = _params[level];
      // Create 'detail' segments, i.e. 'detail+1' rings.
      Stem& stem   = _stems.back();
      stem._level  = level;
      stem._length = length;
      float f_l    = length/detail;

      for( uint i = 0; ; ++i )
      {
         float t   = float(i)/float(detail);
         float rad = CGM::linear2( radius, radius*0.5f, t );
         stem._rings.pushBack( newRing(ref, rad) );

         if( i >= detail ) break;

         // Phototropism.
         Vec3f y      = ref.orientation().getAxisY();
         Vec3f axis   = cross( y, _sunN );
         float yds    = dot( y, _sunN );
         float a_l    = (CGM::acos(yds) - CGM::acos(p._photo)) / (detail-i);
         ref.rotate( axis, a_l*_photoS );

         // Crookedness.
         Quatf q = Quatf::axisCir( normalize( Vec3f(0.0f, rs(), rs() ) ), rs( p._crook, p._crookD ) );
         // Growing.
         ref.translateLocal( q*Vec3f(0.0f, f_l, 0.0f) );
      }
      // TODO: splitting?
   }
   return uint32_t(_stems.size()) - 1;
}

//------------------------------------------------------------------------------
//!
RCP<DFStrokes>
DFTreeNodeJPH::stemsToStrokes() const
{
   RCP<DFStrokes> strokes = new DFStrokes();
   for( auto stemIter = _stems.begin(); stemIter != _stems.end(); ++stemIter )
   {
      const Stem& stem = (*stemIter);

      DFStrokes::Stroke& st = strokes->stroke( strokes->addStroke() );

      // Strokes have V-S-V-S-V... = V+(S+V)^n.
      auto ringIter = stem._rings.begin();

      const Ring& baseRing = _rings[(*ringIter)];
      DFStrokes::Vertex& v = strokes->vertex( st, strokes->addVertex(st) );
      set( baseRing, v );
      v._creases = 0x0F;

      for( ++ringIter; ringIter != stem._rings.end(); ++ringIter )
      {
         const Ring& ring = _rings[(*ringIter)];

         //DFStrokes::Segment& s = strokes->segment( st, strokes->addSegment(st) );
         strokes->addSegment(st);
         // Segment defaults fine for now.
         DFStrokes::Vertex&  v = strokes->vertex( st, strokes->addVertex(st) );
         set( ring, v );
      }
   }
   return strokes;
}

NAMESPACE_END
