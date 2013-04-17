/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/DataFlow/DFWorldNodes.h>
#include <Plasma/DataFlow/DFNodeAttr.h>
#include <Plasma/DataFlow/DFGraph.h>
#include <Plasma/Manipulator/RefManipulator.h>


#if _MSC_VER
// 'this' used in member initializer list.
#pragma warning( disable: 4355 )
#endif

/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN


//------------------------------------------------------------------------------
//!
ConstString _mergeName;
ConstString _entityName;
ConstString _cameraName;
ConstString _lightName;
ConstString _probeName;

enum
{
   ID_BTYPE,
   ID_TYPE,
   ID_PHYS,
   ID_MASS,
   ID_FRICTION,
   ID_RESTITUTION,
   ID_EXISTS,
   ID_SENSES,
   ID_COMMON,
   ID_REF,
   ID_POS,
   ID_ORI,
   ID_FLAGS,
   ID_VISIBLE,
   ID_CASTS_SHADOWS,
   ID_GHOST,
   ID_PLANE,
   ID_FRONT,
   ID_BACK,
   ID_BASE,
   ID_INTENSITY,
   ID_SHAPE,
   ID_MODE,
   ID_FOCAL,
   ID_SPOT,
   ID_FOV,
   ID_MATRIX,
   ID_SCALE,
   ID_SHEAR,
   ID_PERSPECTIVE,
   ID_ORTHO,
   ID_BRAIN,
   ID_ADD_ATTR,
   ID_USER_ATTRIBUTES,
   ID_ID,
   ID_MATERIAL
};

enum
{
   ID_ATTR_GROUP,
   ID_ATTR_KEY,
   ID_ATTR_TYPE,
   ID_ATTR_VALUE,
   ID_ATTR_DEL,
   NUM_ID_ATTR,
};

//------------------------------------------------------------------------------
//!
enum
{
   PROBE_GROUP,
   PROBE_ID,
   PROBE_POS,
   PROBE_SIZE,
   PROBE_TYPE,
};

//------------------------------------------------------------------------------
//!
RCP<DFNode> mergeVM( VMState*, int )
{
   RCP<DFMergeWorldNode> node = new DFMergeWorldNode();
   return node;
}

//------------------------------------------------------------------------------
//!
void initEntity( VMState* vm, int idx, DFEntity* e )
{
   VM::get( vm, idx, "ref", e->_referential );
   VM::get( vm, idx, "visible", e->_visible );
   VM::get( vm, idx, "castsShadows", e->_castsShadows );
   VM::get( vm, idx, "ghost", e->_ghost );
   VM::get( vm, idx, "bodyType", e->_bodyType );
   VM::get( vm, idx, "exists", e->_exists );
   VM::get( vm, idx, "senses", e->_senses );
   VM::get( vm, idx, "mass", e->_mass );
   VM::get( vm, idx, "friction", e->_friction );
   VM::get( vm, idx, "restitution", e->_restitution );
   VM::get( vm, idx, "brain", e->_brainProg );
   VM::get( vm, idx, "id", e->_id );
   VM::get( vm, idx, "material", e->_material );

   if( VM::get( vm, idx, "attributes" ) )
   {
      Table* attr = new Table();
      VM::toTable( vm, -1, *attr );
      VM::pop( vm );
      e->_attributes = attr;
   }
}

//------------------------------------------------------------------------------
//!
RCP<DFNode> entityVM( VMState* vm, int idx )
{
   RCP<DFEntityNode> node = new DFEntityNode();
   DFEntity* e            = node->entity();

   initEntity( vm, idx, e );

   return node;
}

//------------------------------------------------------------------------------
//!
RCP<DFNode> cameraVM( VMState* vm, int idx )
{
   RCP<DFCameraNode> node = new DFCameraNode();
   DFCamera* c            = node->camera();

   // Entity parameters.
   initEntity( vm, idx, c );

   // Camera parameters.
   VM::get( vm, idx, "focal", c->_focalLength );
   VM::get( vm, idx, "fov", c->_fov );
   VM::get( vm, idx, "front", c->_front );
   VM::get( vm, idx, "back", c->_back );
   VM::get( vm, idx, "orthoScale", c->_orthoScale );
   VM::get( vm, idx, "shear", c->_shear );

   int val;
   if( VM::get( vm, idx, "fovMode", val ) ) c->_fovMode = (Camera::FOVMode)val;
   if( VM::get( vm, idx, "projection", val ) ) c->_projType = (Camera::ProjectionType)val;

   return node;
}

//------------------------------------------------------------------------------
//!
RCP<DFNode> lightVM( VMState* vm, int idx )
{
   RCP<DFLightNode> node = new DFLightNode();
   DFLight* l            = node->light();

   // Entity parameters.
   initEntity( vm, idx, l );

   // Light parameters.
   VM::get( vm, idx, "intensity", l->_intensity );
   VM::get( vm, idx, "front", l->_front );
   VM::get( vm, idx, "back", l->_back );
   VM::get( vm, idx, "fov", l->_fov );

   int shp;
   if( VM::get( vm, idx, "shape", shp ) ) l->_shape = (Light::Shape)shp;

   return node;
}

//------------------------------------------------------------------------------
//!
RCP<DFNode> probeVM( VMState* vm, int idx )
{
   RCP<DFProbeNode> node = new DFProbeNode();
   node->init( vm, idx );
   return node;
}

//------------------------------------------------------------------------------
//!
void
dumpCustom( TextStream& os, StreamIndent& indent, const DFEntity* e )
{
   os << indent << "ref = "         << VMFmt( e->referential() )  << "," << nl;
   os << indent << "visible = "     << VMFmt( e->visible() )      << "," << nl;
   os << indent << "castsShadows = "<< VMFmt( e->castsShadows() ) << "," << nl;
   os << indent << "ghost = "       << VMFmt( e->ghost() )        << "," << nl;
   os << indent << "bodyType = "    << e->_bodyType               << "," << nl;
   os << indent << "mass = "        << e->_mass                   << "," << nl;
   os << indent << "friction = "    << e->_friction               << "," << nl;
   os << indent << "restitution = " << e->_restitution            << "," << nl;
   os << indent << "exists = "      << e->_exists                 << "," << nl;
   os << indent << "senses = "      << e->_senses                 << "," << nl;

   if( !e->_id.isNull() )
      os << indent << "id = "       << VMFmt( e->_id )            << "," << nl;

   if( !e->_brainProg.isNull() )
      os << indent << "brain = "    << VMFmt( e->_brainProg )     << "," << nl;

   if( !e->_material.isNull() )
      os << indent << "material = " << VMFmt( e->_material )      << "," << nl;


   if( e->_attributes.isValid() && !e->_attributes->empty() )
      os << indent << "attributes = " << VMFmt( *e->_attributes, indent ) << "," << nl;
}

//------------------------------------------------------------------------------
//!
void  stringToVariant( const char* s, uint type, Variant& dst )
{
   switch( type )
   {
      case Variant::NIL:
         break;
      case Variant::BOOL:
      {
         // Not the most efficient.
         String str = String(s).lower();
         if( str == "false" || str == "0"  || str == "no" )
         {
            dst = false;
         }
         else
         {
            dst = true;
         }
      }  break;
      case Variant::FLOAT:
      {
         float v = dst.getFloat();
         if( s[0] == '(' || s[0] == '{' )  ++s;
         sscanf( s, "%f", &v );
         dst = v;
      }  break;
      case Variant::VEC2:
      {
         Vec2f v = dst.getVec2();
         if( s[0] == '(' || s[0] == '{' )  ++s;
         if( strchr( s, ',' ) )  sscanf( s, "%f,%f", &v.x, &v.y );
         else                    sscanf( s, "%f %f", &v.x, &v.y );
         dst = v;
      }  break;
      case Variant::VEC3:
      {
         Vec3f v = dst.getVec3();
         if( s[0] == '(' || s[0] == '{' )  ++s;
         if( strchr( s, ',' ) )  sscanf( s, "%f,%f,%f", &v.x, &v.y, &v.z );
         else                    sscanf( s, "%f %f %f", &v.x, &v.y, &v.z );
         dst = v;
      }  break;
      case Variant::VEC4:
      {
         Vec4f v = dst.getVec4();
         if( s[0] == '(' || s[0] == '{' )  ++s;
         if( strchr( s, ',' ) )  sscanf( s, "%f,%f,%f,%f", &v.x, &v.y, &v.z, &v.w );
         else                    sscanf( s, "%f %f %f %f", &v.x, &v.y, &v.z, &v.w );
         dst = v;
      }  break;
      case Variant::QUAT:
         // TODO.
         StdErr << "Quat conversion is not supported." << nl;
         break;
      case Variant::STRING:
         dst = s;
         break;
      case Variant::POINTER:
         // Not supported.
      default:
         CHECK( false );
         break;
   }
}

//------------------------------------------------------------------------------
//!
void  stringToVariant( const char* s, Variant& dst )
{
   stringToVariant( s, dst.type(), dst );
}

//------------------------------------------------------------------------------
//!
String  toString( const Variant& v )
{
   switch( v.type() )
   {
      case Variant::NIL:
         return String();
      case Variant::BOOL:
         return v.getBoolean() ? String("true") : String("false");
      case Variant::FLOAT:
      {
         float v1 = v.getFloat();
         return String().format( "%g", v1 );
      }  break;
      case Variant::VEC2:
      {
         const Vec2f& v2 = v.getVec2();
         return String().format( "%g %g", v2.x, v2.y );
      }  break;
      case Variant::VEC3:
      {
         const Vec3f& v3 = v.getVec3();
         return String().format( "%g %g %g", v3.x, v3.y, v3.z );
      }  break;
      case Variant::VEC4:
      {
         const Vec4f& v4 = v.getVec4();
         return String().format( "%g %g %g %g", v4.x, v4.y, v4.z, v4.w );
      }  break;
      case Variant::QUAT:
      {
         const Quatf& q = v.getQuat();
         return String().format( "%g %g %g %g", q.x(), q.y(), q.z(), q.w() );
      }  break;
      case Variant::STRING:
         return String(v.getString().cstr());
      case Variant::POINTER:
         // Not supported.
      default:
         CHECK( false );
         return String();
   }
}

UNNAMESPACE_END

NAMESPACE_BEGIN

/*==============================================================================
   STATIC INITIALIZATION
==============================================================================*/

//------------------------------------------------------------------------------
//!
void initializeWorldNodes()
{
   _mergeName  = "mergeWorld";
   _entityName = "object";
   _cameraName = "camera";
   _lightName  = "light";
   _probeName  = "probe";

   DFNodeSpec::registerNode(
      DFSocket::WORLD,
      _mergeName, mergeVM,
      "Merge", "Merge multiples worlds together.",
      nullptr
   );
   DFNodeSpec::registerNode(
      DFSocket::WORLD,
      _entityName, entityVM,
      "Object", "Create a simple entity (static, dynamic or skeletal).",
      nullptr
   );
   DFNodeSpec::registerNode(
      DFSocket::WORLD,
      _cameraName, cameraVM,
      "Camera", "Create a simple camera.",
      nullptr
   );
   DFNodeSpec::registerNode(
      DFSocket::WORLD,
      _lightName, lightVM,
      "Light", "Create a simple light.",
      nullptr
   );
   DFNodeSpec::registerNode(
      DFSocket::WORLD,
      _probeName, probeVM,
      "Probe", "Create a probe, for baking.",
      nullptr
   );
}

//------------------------------------------------------------------------------
//!
void terminateWorldNodes()
{
   _mergeName  = ConstString();
   _entityName = ConstString();
   _cameraName = ConstString();
   _lightName  = ConstString();
   _probeName  = ConstString();
}

/*==============================================================================
   DFWorldOutput
==============================================================================*/

//------------------------------------------------------------------------------
//!
RCP<DFWorld>
DFWorldOutput::getWorld()
{
   return _delegate();
}

//------------------------------------------------------------------------------
//!
DFSocket::Type
DFWorldOutput::type() const
{
   return WORLD;
}

/*==============================================================================
   CLASS DFWorldInput
==============================================================================*/

//------------------------------------------------------------------------------
//!
RCP<DFWorld>
DFWorldInput::getWorld()
{
   if( !_output ) return nullptr;
   return _output->getWorld();
}

//------------------------------------------------------------------------------
//!
DFSocket::Type
DFWorldInput::type() const
{
   return WORLD;
}

//------------------------------------------------------------------------------
//!
bool
DFWorldInput::isConnected() const
{
   return _output != nullptr;
}

//------------------------------------------------------------------------------
//!
void
DFWorldInput::connect( DFOutput* output )
{
   CHECK( type() == output->type() );
   if( _output ) disconnectFrom( _output );
   _output = (DFWorldOutput*)output;
}

//------------------------------------------------------------------------------
//!
void
DFWorldInput::disconnect( DFOutput* output )
{
   if( _output == output ) _output = nullptr;
}

//------------------------------------------------------------------------------
//!
void
DFWorldInput::disconnect()
{
   if( !_output ) return;
   disconnectFrom( _output );
   _output = nullptr;
}

/*==============================================================================
   CLASS DFWorldMultiInput
==============================================================================*/

//------------------------------------------------------------------------------
//!
RCP<DFWorld>
DFWorldMultiInput::getWorld( uint i )
{
   if( i >= _outputs.size() ) return nullptr;
   return _outputs[i]->getWorld();
}

//------------------------------------------------------------------------------
//!
DFSocket::Type
DFWorldMultiInput::type() const
{
   return WORLD;
}

//------------------------------------------------------------------------------
//!
bool
DFWorldMultiInput::isConnected() const
{
   return !_outputs.empty();
}

//------------------------------------------------------------------------------
//!
void
DFWorldMultiInput::connect( DFOutput* output )
{
   CHECK( type() == output->type() );
   _outputs.pushBack( (DFWorldOutput*)output );
}

//------------------------------------------------------------------------------
//!
void
DFWorldMultiInput::disconnect( DFOutput* output )
{
   _outputs.removeSwap( (DFWorldOutput*)output );
}

//------------------------------------------------------------------------------
//!
void
DFWorldMultiInput::disconnect()
{
   for( auto it = _outputs.begin(); it != _outputs.end(); ++it )
   {
      disconnectFrom( *it );
   }
   _outputs.clear();
}

/*==============================================================================
   CLASS DFMergeWorldNode
==============================================================================*/

//------------------------------------------------------------------------------
//!
DFMergeWorldNode::DFMergeWorldNode():
   _worlds(this)
{
   _output.delegate( makeDelegate( this, &DFMergeWorldNode::process ) );
}

//------------------------------------------------------------------------------
//!
const ConstString&
DFMergeWorldNode::name() const
{
   return _mergeName;
}

//------------------------------------------------------------------------------
//!
uint
DFMergeWorldNode::numInputs() const
{
   return 1;
}

//------------------------------------------------------------------------------
//!
DFOutput*
DFMergeWorldNode::output()
{
   return &_output;
}

//------------------------------------------------------------------------------
//!
DFInput*
DFMergeWorldNode::input( uint id )
{
   if( id == 0 ) return &_worlds;
   return nullptr;
}

//------------------------------------------------------------------------------
//!
RCP<DFWorld>
DFMergeWorldNode::process()
{
   // Do we have a valid input?
   if( _worlds.size() == 0 ) return nullptr;

   RCP<DFWorld> world( new DFWorld() );

   for( size_t i = 0; i < _worlds.size(); ++i )
   {
      auto w = _worlds.getWorld( uint(i) );
      world->entities().append( w->entities() );
      world->probes().append( w->probes() );
   }

   return world;
}

/*==============================================================================
   CLASS DFEntityEditor
==============================================================================*/

class DFEntityEditor:
   public DFNodeEditor
{
public:

   /*----- methods -----*/

   DFEntityEditor( DFEntityNode* n );

   PLASMA_DLL_API virtual RCP<Manipulator> manipulator();
   PLASMA_DLL_API virtual RCP<DFNodeAttrList> attributes() const;
   PLASMA_DLL_API virtual RCP<DFNodeAttrStates>  attributesStates() const;
   PLASMA_DLL_API virtual void updateAttributes( const DFNodeAttrStates& );

   void move( const Vec3f& p );
   void rotate( const Quatf& ori );

protected:

   /*----- structures -----*/

   struct Attribute
   {
      Attribute( uint id, const ConstString& k, const Variant& v ):
         _gid( id ), _key( k ), _value( v ) {}

      uint        _gid;
      ConstString _key;
      Variant     _value;
   };

   /*----- methods -----*/

   DFNodeAttr attributeUI( uint ) const;
   void updateAttributes();
   void changeAttributeKey( uint, const ConstString& );
   void changeAttributeType( uint, uint );
   void changeAttributeValue( uint, const char* );
   void removeAttribute( uint );
   void updateUI();
   void referentialCb();

   /*----- data members -----*/

   DFEntityNode*      _node;
   RCP<RefRenderable> _renderable;
   uint               _nextUserAttr;
   DFNodeAttr         _userAttrTypes;
   Vector<Attribute>  _attributes;
};

//------------------------------------------------------------------------------
//!
DFEntityEditor::DFEntityEditor( DFEntityNode* n ):
   _node( n ),
   _nextUserAttr( 0 )
{
   RCP<Table> types = new Table();
   types->pushBack( "Bool" );
   types->pushBack( "Float"  );
   types->pushBack( "Float2" );
   types->pushBack( "Float3" );
   types->pushBack( "Float4" );
   types->pushBack( "Quat" );
   types->pushBack( "String" );
   _userAttrTypes = DFNodeAttr( "ENUM", ID_ATTR_TYPE, "" ).enums( types.ptr() );

   // Build attributes vector.
   Table* attribs = _node->entity()->_attributes.ptr();
   if( attribs )
   {
      for( auto cur = attribs->begin(); cur != attribs->end(); ++cur )
      {
         _attributes.pushBack( Attribute( 1000 + NUM_ID_ATTR*(_nextUserAttr++), cur->first, cur->second ) );
      }
   }
}

//------------------------------------------------------------------------------
//!
RCP<Manipulator>
DFEntityEditor::manipulator()
{
   if( _renderable.isNull() )
   {
      _renderable = new RefRenderable();
      _renderable->addOnModify( makeDelegate( this, &DFEntityEditor::referentialCb ) );
      _renderable->update( _node->entity()->referential() );
   }
   return RCP<Manipulator>( new RefManipulator( _renderable.ptr() ) );
}

//------------------------------------------------------------------------------
//!
RCP<DFNodeAttrList>
DFEntityEditor::attributes() const
{
   RCP<DFNodeAttrList> atts = new DFNodeAttrList();

   // Common attributes.
   RCP<DFNodeAttrList> cmn = new DFNodeAttrList();

   RCP<DFNodeAttrList> ref = new DFNodeAttrList();
   ref->add( DFNodeAttr( "XYZ", ID_POS, "Position" ) );
   ref->add( DFNodeAttr( "ORI", ID_ORI, "Orientation" ) );
   cmn->add( DFNodeAttr( ID_REF, "", ref.ptr() ) );

   cmn->add( DFNodeAttr( "STRING", ID_ID, "ID" ) );
   cmn->add( DFNodeAttr( "STRING", ID_MATERIAL, "Material" ) );

   RCP<DFNodeAttrList> flg = new DFNodeAttrList();
   flg->add( DFNodeAttr( "BOOL", ID_VISIBLE, "Visible" ) );
   flg->add( DFNodeAttr( "BOOL", ID_CASTS_SHADOWS, "Casts shadows" ) );
   flg->add( DFNodeAttr( "BOOL", ID_GHOST, "Ghost" ) );
   cmn->add( DFNodeAttr( ID_FLAGS, "", flg.ptr() ).compact() );

   atts->add( DFNodeAttr( ID_COMMON, "", cmn.ptr() ) );

   // Physical attributes.
   RCP<DFNodeAttrList> phAtts = new DFNodeAttrList();

   RCP<Table> btype = new Table();
   btype->pushBack( "Dynamic" );
   btype->pushBack( "Static" );
   btype->pushBack( "Kinematic" );
   phAtts->add( DFNodeAttr( "ENUM", ID_BTYPE, "Type" ).enums( btype.ptr() ) );

   phAtts->add( DFNodeAttr( "FLOAT", ID_MASS, "Mass" ) );
   phAtts->add( DFNodeAttr( "FLOAT", ID_FRICTION, "Friction" ) );
   phAtts->add( DFNodeAttr( "FLOAT", ID_RESTITUTION, "Restitution" ) );
   phAtts->add( DFNodeAttr( "STRING", ID_EXISTS, "Exists" ).length(8) );
   phAtts->add( DFNodeAttr( "STRING", ID_SENSES, "Senses" ).length(8) );
   atts->add( DFNodeAttr( ID_PHYS, "", phAtts.ptr() ) );

   // Brain attributes.
   RCP<DFNodeAttrList> brAttr = new DFNodeAttrList();
   brAttr->add( DFNodeAttr( "STRING", ID_BRAIN, "Brain" ) );

   RCP<DFNodeAttrList> uAttr = new DFNodeAttrList();
   for( size_t i = 0; i < _attributes.size(); ++i )
   {
      uAttr->add( attributeUI( _attributes[i]._gid ) );
   }
   uAttr->add( DFNodeAttr( "BUTTON", ID_ADD_ATTR, "+" ).length(2.5f) );
   brAttr->add( DFNodeAttr( ID_USER_ATTRIBUTES, "Attributes", uAttr.ptr() ) );
   atts->add( DFNodeAttr( 333, "", brAttr.ptr() ) );

   return atts;
}

//------------------------------------------------------------------------------
//!
RCP<DFNodeAttrStates>
DFEntityEditor::attributesStates() const
{
   RCP<DFNodeAttrStates> states = new DFNodeAttrStates();
   const DFEntity* e = _node->entity();

   states->set( ID_POS, e->referential().position() );
   states->set( ID_ORI, e->referential().orientation() );
   states->set( ID_VISIBLE, e->visible() );
   states->set( ID_CASTS_SHADOWS, e->castsShadows() );
   states->set( ID_GHOST, e->ghost() );
   states->set( ID_BTYPE, (float)e->_bodyType );
   states->set( ID_MASS, e->_mass );
   states->set( ID_FRICTION, e->_friction );
   states->set( ID_RESTITUTION, e->_restitution );
   states->set( ID_EXISTS, String().format( "0x%x", e->_exists ).cstr() );
   states->set( ID_SENSES, String().format( "0x%x", e->_senses ).cstr() );
   states->set( ID_BRAIN, e->_brainProg );
   states->set( ID_ID, e->_id );
   states->set( ID_MATERIAL, e->_material );

   for( size_t i = 0; i < _attributes.size(); ++i )
   {
      uint gid = _attributes[i]._gid;
      states->set( gid + ID_ATTR_KEY, _attributes[i]._key );
      states->set( gid + ID_ATTR_TYPE, float(_attributes[i]._value.type()-1) );
      states->set( gid + ID_ATTR_VALUE, ConstString(toString(_attributes[i]._value).cstr()) );
   }

   return states;
}

//------------------------------------------------------------------------------
//!
void
DFEntityEditor::updateAttributes( const DFNodeAttrStates& attribs )
{
   DFEntity* e = _node->entity();
   for( auto cur = attribs.begin(); cur != attribs.end(); ++cur )
   {
      switch( cur->_id )
      {
         case ID_POS:
         {
            move( (*cur)._value.getVec3() );
         } break;
         case ID_ORI:
         {
            rotate( (*cur)._value.getQuat() );
         }  break;
         case ID_VISIBLE:
         {
            e->visible( (*cur)._value.getBoolean() );
            _node->graph()->invalidate( _node );
         }  break;
         case ID_CASTS_SHADOWS:
         {
            e->castsShadows( (*cur)._value.getBoolean() );
            _node->graph()->invalidate( _node );
         }  break;
         case ID_GHOST:
         {
            e->ghost( (*cur)._value.getBoolean() );
            _node->graph()->invalidate( _node );
         }  break;
         case ID_BTYPE:
         {
            e->_bodyType = int((*cur)._value.getFloat());
            _node->graph()->invalidate( _node );
         }  break;
         case ID_MASS:
         {
            e->_mass = (*cur)._value.getFloat();
            _node->graph()->invalidate( _node );
         }  break;
         case ID_FRICTION:
         {
            e->_friction = (*cur)._value.getFloat();
            _node->graph()->invalidate( _node );
         }  break;
         case ID_RESTITUTION:
         {
            e->_restitution = (*cur)._value.getFloat();
            _node->graph()->invalidate( _node );
         }  break;
         case ID_EXISTS:
         {
            e->_exists = uint(strtol( (*cur)._value.getString().cstr(), nullptr, 0 ));
            _node->graph()->invalidate( _node );
         }  break;
         case ID_SENSES:
         {
            e->_senses = uint(strtol( (*cur)._value.getString().cstr(), nullptr, 0 ));
            _node->graph()->invalidate( _node );
         }  break;
         case ID_BRAIN:
         {
            e->_brainProg = (*cur)._value.getString();
            _node->graph()->invalidate( _node );
         }  break;
         case ID_ADD_ATTR:
         {
            // New attributes.
            uint gID = 1000 + NUM_ID_ATTR*(_nextUserAttr++);
            _attributes.pushBack( Attribute( gID, "", Variant("") ) );
            // Attribute UI.
            RCP<DFNodeAttrList> grp = new DFNodeAttrList();
            grp->add( attributeUI( gID ) );
            // UI states.
            RCP<DFNodeAttrStates> states = new DFNodeAttrStates();
            states->set( gID + ID_ATTR_TYPE, float(Variant::STRING-1) );
            _node->graph()->msg().modify( _node, grp.ptr(), states.ptr() );
         }  break;
         case ID_ID:
         {
            e->_id = (*cur)._value.getString();
            _node->graph()->invalidate( _node );
         }  break;
         case ID_MATERIAL:
         {
            e->_material = (*cur)._value.getString();
            _node->graph()->invalidate( _node );
         }  break;
         default:
         {
            if( cur->_id >= 1000 )
            {
               uint bID = cur->_id - 1000;
               uint idx = bID / NUM_ID_ATTR;
               uint att = bID % NUM_ID_ATTR;
               uint gID = 1000 + NUM_ID_ATTR*idx;

               switch( att )
               {
                  case ID_ATTR_KEY:   changeAttributeKey( gID, cur->_value.getString() ); break;
                  case ID_ATTR_TYPE:  changeAttributeType( gID, uint(cur->_value.getFloat())+1 ); break;
                  case ID_ATTR_VALUE: changeAttributeValue( gID, cur->_value.getString().cstr() ); break;
                  case ID_ATTR_DEL:
                  {
                     removeAttribute( gID );
                     // Remove UI.
                     RCP<DFNodeAttrList> atts = new DFNodeAttrList();
                     atts->add( DFNodeAttr( gID+ID_ATTR_KEY ) );
                     atts->add( DFNodeAttr( gID+ID_ATTR_TYPE ) );
                     atts->add( DFNodeAttr( gID+ID_ATTR_VALUE ) );
                     atts->add( DFNodeAttr( gID+ID_ATTR_DEL ) );
                     atts->add( DFNodeAttr( gID+ID_ATTR_GROUP ) );
                     _node->graph()->msg().modify( _node, atts.ptr() );
                  }  break;
                  default:;
               }
            }
         }
      }
   }
}

//------------------------------------------------------------------------------
//!
DFNodeAttr
DFEntityEditor::attributeUI( uint gID ) const
{
   RCP<DFNodeAttrList> atts = new DFNodeAttrList();
   atts->add( DFNodeAttr( "STRING", gID+ID_ATTR_KEY, "" ).length(10) );
   atts->add( DFNodeAttr( _userAttrTypes ).id( gID+ID_ATTR_TYPE ).length(6) );
   atts->add( DFNodeAttr( "STRING", gID+ID_ATTR_VALUE, "" ).length(10) );
   atts->add( DFNodeAttr( "BUTTON", gID+ID_ATTR_DEL, "-" ).length(2.5f) );

   return DFNodeAttr( gID+ID_ATTR_GROUP, "", atts.ptr() ).sid( ID_ADD_ATTR ).compact();
}

//------------------------------------------------------------------------------
//!
void
DFEntityEditor::updateAttributes()
{
   DFEntity* e = _node->entity();
   if( _attributes.empty() )
   {
      e->_attributes = nullptr;
      return;
   }
   if( e->_attributes.isNull() ) e->_attributes = new Table();

   for( auto cur = _attributes.begin(); cur != _attributes.end(); ++cur )
   {
      e->_attributes->set( cur->_key, cur->_value );
   }
}

//------------------------------------------------------------------------------
//!
void
DFEntityEditor::changeAttributeKey( uint gid, const ConstString& key )
{
   DFEntity* e = _node->entity();
   for( auto cur = _attributes.begin(); cur != _attributes.end(); ++cur )
   {
      if( cur->_gid == gid )
      {
         if( e->_attributes.isValid() ) e->_attributes->remove( cur->_key );
         cur->_key = key;
         updateAttributes();
         return;
      }
   }
}

//------------------------------------------------------------------------------
//!
void
DFEntityEditor::changeAttributeType( uint gid, uint type )
{
   for( auto cur = _attributes.begin(); cur != _attributes.end(); ++cur )
   {
      if( cur->_gid == gid )
      {
         String str = toString( cur->_value );
         stringToVariant( str.cstr(), type, cur->_value );
         updateAttributes();
         return;
      }
   }
}

//------------------------------------------------------------------------------
//!
void
DFEntityEditor::changeAttributeValue( uint gid, const char* valueStr )
{
   for( auto cur = _attributes.begin(); cur != _attributes.end(); ++cur )
   {
      if( cur->_gid == gid )
      {
         stringToVariant( valueStr, cur->_value );
         updateAttributes();
         return;
      }
   }
}

//------------------------------------------------------------------------------
//!
void
DFEntityEditor::removeAttribute( uint gid )
{
   DFEntity* e = _node->entity();
   for( auto cur = _attributes.begin(); cur != _attributes.end(); ++cur )
   {
      if( cur->_gid == gid )
      {
         if( e->_attributes.isValid() ) e->_attributes->remove( cur->_key );
         _attributes.erase( cur );
         updateAttributes();
         return;
      }
   }
}

//------------------------------------------------------------------------------
//!
void
DFEntityEditor::move( const Vec3f& p )
{
   _node->entity()->position( p );
   _node->graph()->invalidate( _node );
   if( _renderable.isValid() )
      _renderable->update( _node->entity()->referential() );
}

//------------------------------------------------------------------------------
//!
void
DFEntityEditor::rotate( const Quatf& ori )
{
   _node->entity()->orientation( ori );
   _node->graph()->invalidate( _node );
   if( _renderable.isValid() )
      _renderable->update( _node->entity()->referential() );
}

//------------------------------------------------------------------------------
//!
void
DFEntityEditor::updateUI()
{
   _node->graph()->msg().modify( _node, attributesStates().ptr() );
}

//------------------------------------------------------------------------------
//!
void
DFEntityEditor::referentialCb()
{
   _node->entity()->referential( _renderable->referential() );
   _node->graph()->invalidate( _node );
   updateUI();
}

/*==============================================================================
   CLASS DFEntityNode
==============================================================================*/

//------------------------------------------------------------------------------
//!
DFEntityNode::DFEntityNode():
   _geometry( this )
{
   _output.delegate( makeDelegate( this, &DFEntityNode::process ) );
   _world = new DFWorld();
   _world->entities().pushBack( new DFEntity() );
}

//------------------------------------------------------------------------------
//!
const ConstString&
DFEntityNode::name() const
{
   return _entityName;
}

//------------------------------------------------------------------------------
//!
uint
DFEntityNode::numInputs() const
{
   return 1;
}

//------------------------------------------------------------------------------
//!
DFOutput*
DFEntityNode::output()
{
   return &_output;
}

//------------------------------------------------------------------------------
//!
DFInput*
DFEntityNode::input( uint id )
{
   if( id == 0 ) return &_geometry;
   return nullptr;
}

//------------------------------------------------------------------------------
//!
RCP<DFNodeEditor>
DFEntityNode::edit()
{
   if( _editor.isNull() )  _editor = new DFEntityEditor( this );
   return _editor;
}

//------------------------------------------------------------------------------
//!
RCP<DFWorld>
DFEntityNode::process()
{
   entity()->_geom = _geometry.getGeometry();
   return _world;
}

//------------------------------------------------------------------------------
//!
bool
DFEntityNode::dumpCustom( TextStream& os, StreamIndent& indent ) const
{
   ::dumpCustom( os, indent, entity() );
   return os.ok();
}

/*==============================================================================
   CLASS DFCameraEditor
==============================================================================*/

class DFCameraEditor:
   public DFNodeEditor
{
public:

   /*----- methods -----*/

   DFCameraEditor( DFCameraNode* n ): _node(n) {}

   PLASMA_DLL_API virtual RCP<Manipulator> manipulator();
   PLASMA_DLL_API virtual RCP<DFNodeAttrList> attributes() const;
   PLASMA_DLL_API virtual RCP<DFNodeAttrStates> attributesStates() const;
   PLASMA_DLL_API virtual void updateAttributes( const DFNodeAttrStates& );

   void move( const Vec3f& p );
   void rotate( const Quatf& ori );

protected:

   /*----- methods -----*/

   void updateUI();
   void referentialCb();

   /*----- data members -----*/

   DFCameraNode*      _node;
   RCP<RefRenderable> _renderable;
};

//------------------------------------------------------------------------------
//!
RCP<Manipulator>
DFCameraEditor::manipulator()
{
   if( _renderable.isNull() )
   {
      _renderable = new RefRenderable();
      _renderable->addOnModify( makeDelegate( this, &DFCameraEditor::referentialCb ) );
      _renderable->update( _node->camera()->referential() );
   }
   return RCP<Manipulator>( new RefManipulator( _renderable.ptr() ) );
}


//------------------------------------------------------------------------------
//!
RCP<DFNodeAttrList>
DFCameraEditor::attributes() const
{
   RCP<DFNodeAttrList> atts = new DFNodeAttrList();
   RCP<DFNodeAttrList> ref  = new DFNodeAttrList();
   ref->add( DFNodeAttr( "XYZ", ID_POS, "Position" ) );
   ref->add( DFNodeAttr( "ORI", ID_ORI, "Orientation" ) );
   atts->add( DFNodeAttr( ID_REF, "", ref.ptr() ) );

   // Organized by type of camera.
   RCP<DFNodeAttrList> mat = new DFNodeAttrList();
   RCP<Table> typeEnums    = new Table();
   typeEnums->pushBack( "PERSPECTIVE" );
   typeEnums->pushBack( "ORTHO" );
   mat->add( DFNodeAttr( "ENUM", ID_TYPE, "Type" ).enums( typeEnums.ptr() ) );
   mat->add( DFNodeAttr( "FLOAT2", ID_SHEAR, "Shear" ) );
   atts->add( DFNodeAttr( ID_MATRIX, "", mat.ptr() ) );

   // Projective camera.
   RCP<DFNodeAttrList> pers = new DFNodeAttrList();
   pers->add( DFNodeAttr( "FLOAT", ID_FOCAL, "FDistance") );
   pers->add( DFNodeAttr( "FLOAT", ID_FOV, "FOV" ) );
   RCP<Table> modeEnums = new Table();
   modeEnums->pushBack( "X" );
   modeEnums->pushBack( "Y" );
   modeEnums->pushBack( "SMALLEST" );
   modeEnums->pushBack( "LARGEST" );
   pers->add( DFNodeAttr( "ENUM", ID_MODE, "Mode" ).enums( modeEnums.ptr() ) );
   atts->add( DFNodeAttr( ID_PERSPECTIVE, "", pers.ptr() ) );

   // Ortho.
   RCP<DFNodeAttrList> ortho = new DFNodeAttrList();
   ortho->add( DFNodeAttr( "FLOAT", ID_SCALE, "Scale") );
   atts->add( DFNodeAttr( ID_ORTHO, "", ortho.ptr() ) );

   // Plane limits.
   RCP<DFNodeAttrList> plane = new DFNodeAttrList();
   plane->add( DFNodeAttr( "FLOAT", ID_FRONT, "Front" ) );
   plane->add( DFNodeAttr( "FLOAT", ID_BACK, "Back" ) );
   atts->add( DFNodeAttr( ID_PLANE, "", plane.ptr() ) );

   return atts;
}

//------------------------------------------------------------------------------
//!
RCP<DFNodeAttrStates>
DFCameraEditor::attributesStates() const
{
   RCP<DFNodeAttrStates> states = new DFNodeAttrStates();

   DFCamera* c = _node->camera();
   states->set( ID_POS, c->referential().position() );
   states->set( ID_ORI, c->referential().orientation() );
   states->set( ID_TYPE, (float)c->_projType );

   if( c->_projType == Camera::PERSPECTIVE )
   {
      states->set( ID_PERSPECTIVE, true );
      states->set( ID_ORTHO, false );
      states->set( ID_FOCAL, c->_focalLength );
      states->set( ID_FOV, c->_fov );
      states->set( ID_MODE, (float)c->_fovMode );
   }
   else
   {
      states->set( ID_PERSPECTIVE, false );
      states->set( ID_ORTHO, true );
      states->set( ID_SCALE, c->_orthoScale );
   }

   states->set( ID_FRONT, c->_front );
   states->set( ID_BACK, c->_back );
   states->set( ID_SHEAR, c->_shear );

   return states;
}

//------------------------------------------------------------------------------
//!
void
DFCameraEditor::updateAttributes( const DFNodeAttrStates& attribs )
{
   DFCamera* c = _node->camera();
   for( auto cur = attribs.begin(); cur != attribs.end(); ++cur )
   {
      switch( cur->_id )
      {
         case ID_POS:
         {
            move( (*cur)._value.getVec3() );
         }  break;
         case ID_ORI:
         {
            rotate( (*cur)._value.getQuat() );
         }  break;
         case ID_FOCAL:
         {
            c->_focalLength = (*cur)._value.getFloat();
            _node->graph()->invalidate( _node );
         }  break;
         case ID_FOV:
         {
            c->_fov = (*cur)._value.getFloat();
            _node->graph()->invalidate( _node );
         }  break;
         case ID_FRONT:
         {
            c->_front = (*cur)._value.getFloat();
            _node->graph()->invalidate( _node );
         }  break;
         case ID_BACK:
         {
            c->_back = (*cur)._value.getFloat();
            _node->graph()->invalidate( _node );
         }  break;
         case ID_SHEAR:
         {
            c->_shear = (*cur)._value.getVec2();
            _node->graph()->invalidate( _node );
         }  break;
         case ID_SCALE:
         {
            c->_orthoScale = (*cur)._value.getFloat();
            _node->graph()->invalidate( _node );
         }  break;
         case ID_TYPE:
         {
            c->_projType = (Camera::ProjectionType)int((*cur)._value.getFloat());
            _node->graph()->invalidate( _node );
            updateUI();
         }  break;
         case ID_MODE:
         {
            c->_fovMode = (Camera::FOVMode)int((*cur)._value.getFloat());
            _node->graph()->invalidate( _node );
         }  break;
         default:;
      }
   }
}

//------------------------------------------------------------------------------
//!
void
DFCameraEditor::move( const Vec3f& p )
{
   _node->camera()->position( p );
   _node->graph()->invalidate( _node );
   if( _renderable.isValid() )
      _renderable->update( _node->camera()->referential() );
}

//------------------------------------------------------------------------------
//!
void
DFCameraEditor::rotate( const Quatf& ori )
{
   _node->camera()->orientation( ori );
   _node->graph()->invalidate( _node );
   if( _renderable.isValid() )
      _renderable->update( _node->camera()->referential() );
}

//------------------------------------------------------------------------------
//!
void
DFCameraEditor::updateUI()
{
   _node->graph()->msg().modify( _node, attributesStates().ptr() );
}

//------------------------------------------------------------------------------
//!
void
DFCameraEditor::referentialCb()
{
   _node->camera()->referential( _renderable->referential() );
   _node->graph()->invalidate( _node );
   updateUI();
}


/*==============================================================================
   CLASS DFCameraNode
==============================================================================*/

//------------------------------------------------------------------------------
//!
DFCameraNode::DFCameraNode():
   _geometry( this )
{
   _output.delegate( makeDelegate( this, &DFCameraNode::process ) );
   _world = new DFWorld();
   _world->entities().pushBack( new DFCamera() );
}

//------------------------------------------------------------------------------
//!
const ConstString&
DFCameraNode::name() const
{
   return _cameraName;
}

//------------------------------------------------------------------------------
//!
uint
DFCameraNode::numInputs() const
{
   return 1;
}

//------------------------------------------------------------------------------
//!
DFOutput*
DFCameraNode::output()
{
   return &_output;
}

//------------------------------------------------------------------------------
//!
DFInput*
DFCameraNode::input( uint id )
{
   if( id == 0 ) return &_geometry;
   return nullptr;
}

//------------------------------------------------------------------------------
//!
RCP<DFNodeEditor>
DFCameraNode::edit()
{
   if( _editor.isNull() )  _editor = new DFCameraEditor( this );
   return _editor;
}

//------------------------------------------------------------------------------
//!
RCP<DFWorld>
DFCameraNode::process()
{
   camera()->_geom = _geometry.getGeometry();
   return _world;
}

//------------------------------------------------------------------------------
//!
bool
DFCameraNode::dumpCustom( TextStream& os, StreamIndent& indent ) const
{
   const DFCamera* c = camera();
   ::dumpCustom( os, indent, c );

   os << indent << "focal = "      << c->_focalLength    << "," << nl;
   os << indent << "fov = "        << c->_fov            << "," << nl;
   os << indent << "front = "      << c->_front          << "," << nl;
   os << indent << "back = "       << c->_back           << "," << nl;
   os << indent << "orthoScale = " << c->_orthoScale     << "," << nl;
   os << indent << "shear = "      << VMFmt( c->_shear ) << "," << nl;
   os << indent << "fovMode = "    << c->_fovMode        << "," << nl;
   os << indent << "projection = " << c->_projType       << "," << nl;
   return os.ok();
}

/*==============================================================================
   CLASS DFLightEditor
==============================================================================*/

class DFLightEditor:
   public DFNodeEditor
{
public:

   /*----- methods -----*/

   DFLightEditor( DFLightNode* n ): _node(n) {}

   PLASMA_DLL_API virtual RCP<Manipulator> manipulator();
   PLASMA_DLL_API virtual RCP<DFNodeAttrList> attributes() const;
   PLASMA_DLL_API virtual RCP<DFNodeAttrStates> attributesStates() const;
   PLASMA_DLL_API virtual void updateAttributes( const DFNodeAttrStates& );

   void move( const Vec3f& p );
   void rotate( const Quatf& ori );

protected:

   /*----- methods -----*/

   void updateUI();
   void referentialCb();

   /*----- data members -----*/

   DFLightNode*       _node;
   RCP<RefRenderable> _renderable;
};

//------------------------------------------------------------------------------
//!
RCP<Manipulator>
DFLightEditor::manipulator()
{
   if( _renderable.isNull() )
   {
      _renderable = new RefRenderable();
      _renderable->addOnModify( makeDelegate( this, &DFLightEditor::referentialCb ) );
      _renderable->update( _node->light()->referential() );
   }
   return RCP<Manipulator>( new RefManipulator( _renderable.ptr() ) );
}

//------------------------------------------------------------------------------
//!
RCP<DFNodeAttrList>
DFLightEditor::attributes() const
{
   RCP<DFNodeAttrList> atts = new DFNodeAttrList();
   RCP<DFNodeAttrList> ref  = new DFNodeAttrList();
   ref->add( DFNodeAttr( "XYZ", ID_POS, "Position" ) );
   ref->add( DFNodeAttr( "ORI", ID_ORI, "Orientation" ) );
   atts->add( DFNodeAttr( ID_REF, "", ref.ptr() ) );

   // Type.
   RCP<DFNodeAttrList> base = new DFNodeAttrList();
   RCP<Table> enums         = new Table();
   enums->pushBack( "DIRECTIONAL" );
   enums->pushBack( "GEOMETRY" );
   enums->pushBack( "POINT" );
   enums->pushBack( "SPOT" );
   base->add( DFNodeAttr( "ENUM", ID_SHAPE, "Shape" ).enums( enums.ptr() ) );
   base->add( DFNodeAttr( "RGB", ID_INTENSITY, "intensity" ) );
   atts->add( DFNodeAttr( ID_BASE, "", base.ptr() ) );

   RCP<DFNodeAttrList> spot = new DFNodeAttrList();
   spot->add( DFNodeAttr( "FLOAT", ID_FOV, "FOV" ) );
   atts->add( DFNodeAttr( ID_SPOT, "", spot.ptr() ) );

   // Plane limits.
   RCP<DFNodeAttrList> plane = new DFNodeAttrList();
   plane->add( DFNodeAttr( "FLOAT", ID_FRONT, "Front" ) );
   plane->add( DFNodeAttr( "FLOAT", ID_BACK, "Back" ) );
   atts->add( DFNodeAttr( ID_PLANE, "", plane.ptr() ) );

   return atts;
}

//------------------------------------------------------------------------------
//!
RCP<DFNodeAttrStates>
DFLightEditor::attributesStates() const
{
   RCP<DFNodeAttrStates> states = new DFNodeAttrStates();

   DFLight* l = _node->light();
   states->set( ID_POS, l->referential().position() );
   states->set( ID_ORI, l->referential().orientation() );
   states->set( ID_SHAPE, (float)l->_shape );
   states->set( ID_INTENSITY, l->_intensity );
   states->set( ID_FRONT, l->_front );
   states->set( ID_BACK, l->_back );

   if( l->_shape == Light::SPOT )
   {
      states->set( ID_SPOT, true );
      states->set( ID_FOV, l->_fov );
   }
   else
   {
      states->set( ID_SPOT, false );
   }

   return states;
}

//------------------------------------------------------------------------------
//!
void
DFLightEditor::updateAttributes( const DFNodeAttrStates& attribs )
{
   DFLight* l = _node->light();
   for( auto cur = attribs.begin(); cur != attribs.end(); ++cur )
   {
      switch( cur->_id )
      {
         case ID_POS:
         {
            move( (*cur)._value.getVec3() );
         }  break;
         case ID_ORI:
         {
            rotate( (*cur)._value.getQuat() );
         }  break;
         case ID_FRONT:
         {
            l->_front = (*cur)._value.getFloat();
            _node->graph()->invalidate( _node );
         }  break;
         case ID_BACK:
         {
            l->_back = (*cur)._value.getFloat();
            _node->graph()->invalidate( _node );
         }  break;
         case ID_FOV:
         {
            l->_fov = (*cur)._value.getFloat();
            _node->graph()->invalidate( _node );
         }  break;
         case ID_INTENSITY:
         {
            l->_intensity = (*cur)._value.getVec3();
            _node->graph()->invalidate( _node );
         }  break;
         case ID_SHAPE:
         {
            l->_shape = (Light::Shape)int((*cur)._value.getFloat());
            _node->graph()->invalidate( _node );
            updateUI();
         }  break;
         default:;
      }
   }
}

//------------------------------------------------------------------------------
//!
void
DFLightEditor::move( const Vec3f& p )
{
   _node->light()->position( p );
   _node->graph()->invalidate( _node );
   if( _renderable.isValid() )
      _renderable->update( _node->light()->referential() );
}

//------------------------------------------------------------------------------
//!
void
DFLightEditor::rotate( const Quatf& ori )
{
   _node->light()->orientation( ori );
   _node->graph()->invalidate( _node );
   if( _renderable.isValid() )
      _renderable->update( _node->light()->referential() );
}

//------------------------------------------------------------------------------
//!
void
DFLightEditor::updateUI()
{
   _node->graph()->msg().modify( _node, attributesStates().ptr() );
}

//------------------------------------------------------------------------------
//!
void
DFLightEditor::referentialCb()
{
   _node->light()->referential( _renderable->referential() );
   _node->graph()->invalidate( _node );
   updateUI();
}

/*==============================================================================
   CLASS DFLightNode
==============================================================================*/

//------------------------------------------------------------------------------
//!
DFLightNode::DFLightNode():
   _geometry( this )
{
   _output.delegate( makeDelegate( this, &DFLightNode::process ) );
   _world = new DFWorld();
   _world->entities().pushBack( new DFLight() );
}

//------------------------------------------------------------------------------
//!
const ConstString&
DFLightNode::name() const
{
   return _lightName;
}

//------------------------------------------------------------------------------
//!
uint
DFLightNode::numInputs() const
{
   return 1;
}

//------------------------------------------------------------------------------
//!
DFOutput*
DFLightNode::output()
{
   return &_output;
}

//------------------------------------------------------------------------------
//!
DFInput*
DFLightNode::input( uint id )
{
   if( id == 0 ) return &_geometry;
   return nullptr;
}

//------------------------------------------------------------------------------
//!
RCP<DFNodeEditor>
DFLightNode::edit()
{
   if( _editor.isNull() )  _editor = new DFLightEditor( this );
   return _editor;
}

//------------------------------------------------------------------------------
//!
RCP<DFWorld>
DFLightNode::process()
{
   light()->_geom = _geometry.getGeometry();
   return _world;
}

//------------------------------------------------------------------------------
//!
bool
DFLightNode::dumpCustom( TextStream& os, StreamIndent& indent ) const
{
   const DFLight* l = light();
   ::dumpCustom( os, indent, l );

   os << indent << "intensity = " << VMFmt( l->_intensity ) << "," << nl;
   os << indent << "front = "     << l->_front              << "," << nl;
   os << indent << "back = "      << l->_back               << "," << nl;
   os << indent << "fov = "       << l->_fov                << "," << nl;
   os << indent << "shape = "     << l->_shape              << "," << nl;
   return os.ok();
}


/*==============================================================================
   CLASS DFProbeEditor
==============================================================================*/

class DFProbeEditor:
   public DFNodeEditor
{
public:

   /*----- methods -----*/

   DFProbeEditor( DFProbeNode* n ): _node(n) {}

   PLASMA_DLL_API virtual RCP<Manipulator> manipulator();
   PLASMA_DLL_API virtual RCP<DFNodeAttrList> attributes() const;
   PLASMA_DLL_API virtual RCP<DFNodeAttrStates> attributesStates() const;
   PLASMA_DLL_API virtual void updateAttributes( const DFNodeAttrStates& );

protected:

   /*----- methods -----*/

   void updateUI();

   /*----- data members -----*/

   DFProbeNode*  _node;
   //RCP<ProbeRenderable> _renderable;
};

//------------------------------------------------------------------------------
//!
RCP<Manipulator>
DFProbeEditor::manipulator()
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
DFProbeEditor::attributes() const
{
   RCP<DFNodeAttrList> atts = new DFNodeAttrList();

   RCP<DFNodeAttrList> grp  = new DFNodeAttrList();

   RCP<Table> typeEnums = new Table();
   typeEnums->pushBack( "Cubemap" );
   grp->add( DFNodeAttr( "ENUM", PROBE_TYPE, "Type" ).enums( typeEnums.ptr() ) );

   grp->add( DFNodeAttr( "XYZ", PROBE_POS, "Position" ) );

   grp->add( DFNodeAttr( "STRING", PROBE_ID, "ID" ) );

   grp->add( DFNodeAttr( "INT", PROBE_SIZE, "Size").range(1,1024) );

   atts->add( DFNodeAttr( PROBE_GROUP, "", grp.ptr() ) );

   return atts;
}

//------------------------------------------------------------------------------
//!
RCP<DFNodeAttrStates>
DFProbeEditor::attributesStates() const
{
   const DFProbe& p = _node->probe();
   RCP<DFNodeAttrStates> states = new DFNodeAttrStates();

   states->set( PROBE_TYPE, float(p.type()) );
   states->set( PROBE_POS , p.position()    );
   states->set( PROBE_ID  , p.id()          );
   states->set( PROBE_SIZE, float(p.size()) );

   return states;
}

//------------------------------------------------------------------------------
//!
void
DFProbeEditor::updateAttributes( const DFNodeAttrStates& attribs )
{
   DFProbe& p = _node->probe();
   for( auto cur = attribs.begin(); cur != attribs.end(); ++cur )
   {
      switch( cur->_id )
      {
         case PROBE_ID:
         {
            p.id( (*cur)._value.getString() );
         }  break;
         case PROBE_POS:
         {
            p.position( (*cur)._value.getVec3() );
         }  break;
         case PROBE_SIZE:
         {
            p.size( uint((*cur)._value.getFloat()) );
         }  break;
         case PROBE_TYPE:
         {
            p.type( (Probe::Type)int((*cur)._value.getFloat()) );
         }  break;
         default:;
      }
   }
   _node->graph()->invalidate( _node );
}

//------------------------------------------------------------------------------
//!
void
DFProbeEditor::updateUI()
{
   _node->graph()->msg().modify( _node, attributesStates().ptr() );
}


/*==============================================================================
   CLASS DFProbeNode
==============================================================================*/

//------------------------------------------------------------------------------
//!
DFProbeNode::DFProbeNode()
{
   _world = new DFWorld();
   _world->probes().pushBack( new DFProbe() );
   DFProbe& p = probe();
   p.type( Probe::CUBEMAP );
   p.position( Vec3f(0.0f) );
   p.size( 64 );
   _output.delegate( makeDelegate( this, &DFProbeNode::process ) );
}

//------------------------------------------------------------------------------
//!
const ConstString&
DFProbeNode::name() const
{
   return _probeName;
}

//------------------------------------------------------------------------------
//!
DFOutput*
DFProbeNode::output()
{
   return &_output;
}

//------------------------------------------------------------------------------
//!
RCP<DFNodeEditor>
DFProbeNode::edit()
{
   if( _editor.isNull() )  _editor = new DFProbeEditor( this );
   return _editor;
}

//------------------------------------------------------------------------------
//!
RCP<DFWorld>
DFProbeNode::process()
{
   return _world;
}

//------------------------------------------------------------------------------
//!
bool
DFProbeNode::dumpCustom( TextStream& os, StreamIndent& indent ) const
{
   const DFProbe& p = probe();
   os << indent << "type = " << p._type << "," << nl;
   os << indent << "id = " << VMFmt(p._id) << "," << nl;
   os << indent << "pos = " << VMFmt(p._pos) << "," << nl;
   os << indent << "size = " << p._size << "," << nl;
   return os.ok();
}

//------------------------------------------------------------------------------
//!
void
DFProbeNode::init( VMState* vm, int idx )
{
   DFProbe& p = probe();
   uint u;
   if( VM::get( vm, idx, "type", u ) )
   {
      p._type = (Probe::Type)u;
   }
   VM::get( vm, idx, "id"  , p._id   );
   VM::get( vm, idx, "pos" , p._pos  );
   VM::get( vm, idx, "size", p._size );
}


NAMESPACE_END
