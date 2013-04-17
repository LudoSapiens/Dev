/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/Widget/DFScreen.h>
#include <Plasma/World/SkeletalEntity.h>
#include <Plasma/DataFlow/DFAnimNodes.h>
#include <Plasma/DataFlow/DFGeomNodes.h>
#include <Plasma/DataFlow/DFImageNodes.h>
#include <Plasma/DataFlow/DFPolygonNodes.h>
#include <Plasma/DataFlow/DFStrokes.h>
#include <Plasma/DataFlow/DFStrokesNodes.h>
#include <Plasma/DataFlow/DFWorldNodes.h>
#include <Plasma/DataFlow/DFNode.h>
#include <Plasma/Geometry/MeshGeometry.h>
#include <Plasma/Geometry/SurfaceGeometry.h>
#include <Plasma/Renderable/Renderable.h>
#include <Plasma/Manipulator/ManipulatorGroup.h>
#include <Plasma/Manipulator/TouchCameraManipulator.h>

#include <Fusion/VM/VMObjectPool.h>
#include <Fusion/Core/Core.h>

#include <Base/ADT/StringMap.h>


/*==============================================================================
  UNNAME NAMESPACE
==============================================================================*/

UNNAMESPACE_BEGIN

Vec4f _bgColor(0.3f,0.3f,0.3f,1.0f);

//------------------------------------------------------------------------------
//!
enum
{
   ATTRIB_ADD_ON_CHANGE_OUTPUT,
   ATTRIB_CAMERA,
   ATTRIB_CAMERA_ORIENTATION,
   ATTRIB_CAMERA_POSITION,
   ATTRIB_GRAPH,
   ATTRIB_MANIPULATOR,
   ATTRIB_OUTPUT_TYPE,
   ATTRIB_REMOVE_ON_CHANGE_OUTPUT,
   ATTRIB_RENDER_SIZE,
   ATTRIB_TIME,
   ATTRIB_TIME_RANGE
};

StringMap _attributes(
   "addOnChangeOutput",     ATTRIB_ADD_ON_CHANGE_OUTPUT,
   "camera",                ATTRIB_CAMERA,
   "cameraOrientation",     ATTRIB_CAMERA_ORIENTATION,
   "cameraPosition",        ATTRIB_CAMERA_POSITION,
   "graph",                 ATTRIB_GRAPH,
   "manipulator",           ATTRIB_MANIPULATOR,
   "outputType",            ATTRIB_OUTPUT_TYPE,
   "removeOnChangeOutput",  ATTRIB_REMOVE_ON_CHANGE_OUTPUT,
   "renderSize",            ATTRIB_RENDER_SIZE,
   "time",                  ATTRIB_TIME,
   "timeRange",             ATTRIB_TIME_RANGE,
   ""
);

//------------------------------------------------------------------------------
//!
const char* _dfScreen_str_  = "dfScreen";

//------------------------------------------------------------------------------
//!
void execute( const VMRef& ref, DFScreen* screen )
{
   if( ref.isValid() )
   {
      VMState* vm = ref.vm();
      VM::push( vm, ref );
      VM::pushProxy( vm, screen );
      VM::ecall( vm, 1, 0 );
   }
}

//------------------------------------------------------------------------------
//!
int dfscreen_addOnChangeOutputVM( VMState* vm )
{
   DFScreen* screen = (DFScreen*)VM::thisPtr( vm );
   VMRef ref;
   VM::toRef( vm, 1, ref );
   screen->addOnChangeOutput( ref );
   return 0;
}

//------------------------------------------------------------------------------
//!
int dfscreen_removeOnChangeOutputVM( VMState* vm )
{
   DFScreen* screen = (DFScreen*)VM::thisPtr( vm );
   VMRef ref;
   VM::toRef( vm, 1, ref );
   screen->removeOnChangeOutput( ref );
   return 0;
}

UNNAMESPACE_END

NAMESPACE_BEGIN

/*==============================================================================
   CLASS DFScreen
==============================================================================*/

//------------------------------------------------------------------------------
//!
void
DFScreen::initialize()
{
   VMObjectPool::registerObject(
      "UI",
      _dfScreen_str_,
      stdCreateVM<DFScreen>,
      stdGetVM<DFScreen>,
      stdSetVM<DFScreen>
   );
}

//------------------------------------------------------------------------------
//!
DFScreen::DFScreen():
   _time(0.0f),
   _outputNode(nullptr)
{
   // World.
   _world = new World();
   _world->backgroundColor( _bgColor );
   world( _world.ptr() );

   // Lights.
   _light = new Light( RigidBody::STATIC );
   _light->shape( Light::DIRECTIONAL );
   _light->position( Vec3f(-5.0f, 10.0f, 2.0f ) );
   _light->lookAt( Vec3f(0.0f), Vec3f(0.0f,1.0f,0.0f) );

   // Entity use for showing the graph output result.
   // 1. geometry.
   _entity = new RigidEntity( RigidBody::STATIC );
   RCP<BaseMaterial> bmat = new BaseMaterial();
   bmat->addLayer( BaseMaterial::Layer( data( ResManager::getImage( "image/checker" ) ) ) );
   RCP<MaterialSet> bset = new MaterialSet();
   bset->add( bmat.ptr() );
   _entity->materialSet( bset.ptr() );

   // 2. animation.
   _puppet = new SkeletalEntity();
   _puppet->makeStatic();
   // 3. image.
   _image                = new Image();
   _imageEntity          = new RigidEntity( RigidBody::STATIC );

   RCP<CustomMaterial> mat = new CustomMaterial();
   mat->programName( "shader/program/colorTex" );
   mat->addImage( _image.ptr() );
   RCP<Table> tex( new Table() );
   tex->set( "data", 0.0f );
   tex->set( "clamp", float(Gfx::TEX_CLAMP_LAST) );
   tex->set( "minFilter", float(Gfx::TEX_FILTER_POINT) );
   tex->set( "magFilter", float(Gfx::TEX_FILTER_POINT) );
   tex->set( "mipFilter", float(Gfx::TEX_FILTER_NONE) );
   mat->variants().set( "colorTex", tex.ptr() );
   mat->variants().set( "color", Vec4f(1.0f) );

   _imageEntity->materialSet( new MaterialSet() );
   _imageEntity->materialSet()->add( mat.ptr() );

   int attribs[] = {
      MeshGeometry::POSITION,
      MeshGeometry::MAPPING,
      MeshGeometry::NORMAL,
      0
   };
   _imageMesh     = new MeshGeometry( MeshGeometry::TRIANGLES, attribs, 6, 4 );
   uint32_t* idx  = _imageMesh->indices();
   idx[0] = 0;
   idx[1] = 1;
   idx[2] = 2;
   idx[3] = 0;
   idx[4] = 2;
   idx[5] = 3;

   _imageMesh->addPatch( 0, 6 );
   _imageEntity->geometry( _imageMesh.ptr() );

   updateImageGeom( Vec2f(2.0f), Vec2f(1.0f) );

   manipulators()->addBack( new TouchCameraManipulator() );

   // Set up cameras.
   initCameras();
   RCP<Camera> c( new Camera( RigidBody::KINEMATIC ) );
   c->projection( Camera::ORTHO );
   c->position( Vec3f( 0.0f, 0.0f, 100.0f ) );
   c->lookAt( Vec3f( 0.0f, 0.0f, 0.0f ), Vec3f( 0.0f, 1.0f, 0.0f ) );
   c->orthoScale( 4.0f );
   _cameras.pushBack( c );
}

//------------------------------------------------------------------------------
//!
DFScreen::~DFScreen()
{
   if( _graph.isValid() ) _graph->msg().removeOnUpdate( makeDelegate( this, &DFScreen::graphUpdated ) );
   if( _manip.isValid() ) _world->removeRenderable( _manip->renderable() );
}

//------------------------------------------------------------------------------
//!
void DFScreen::time( float t )
{
   _time = t;
   // FIXME: DO better...
   if( _anim.isValid() )
      _puppet->setPose( _anim.ptr(), _time );
}

//------------------------------------------------------------------------------
//!
Vec2f DFScreen::timeRange() const
{
   if( !_outputNode ) return Vec2f(0.0f);

   if( _anim.isValid() ) return Vec2f( 0.0f, _anim->duration() );

   return Vec2f(0.0f);
}

//------------------------------------------------------------------------------
//!
void DFScreen::graph( DFGraph* g )
{
   // Nothing new?
   if( g == _graph ) return;

   // Remove update callback.
   if( _graph.isValid() ) _graph->msg().removeOnUpdate( makeDelegate( this, &DFScreen::graphUpdated ) );
   _graph = g;
   // Add update callback.
   if( _graph.isValid() ) _graph->msg().addOnUpdate( makeDelegate( this, &DFScreen::graphUpdated ) );

   graphUpdated( g );
}

//------------------------------------------------------------------------------
//!
void DFScreen::graphUpdated( DFGraph* )
{
   // Clearing old computed resource(s).
   _anim = nullptr;

   // Clearing the world.
   if( _graph.isNull() || !_graph->output() )
   {
      emptyOutput();
      return;
   }

   // Evaluate the output node of the graph.
   DFNode* out = _graph->output();

   switch( out->type() )
   {
      case DFSocket::IMAGE:
      {
         DFImageParams params( Vec2f(0.0f), Vec2f(1.0f), Vec2i(256) );
         DFImageOutput* imageOutput = (DFImageOutput*)out->output();
         RCP<Bitmap> img            = imageOutput->getImage( params );

         if( img.isNull() )
         {
            emptyOutput();
            return;
         }

         // Update texture and geometry.
         _image->bitmap( img.ptr() );
         _imageEntity->materialSet()->material(0)->updateSamplers();
         Vec2f defSize( float(_image->texture()->definedWidth()), float(_image->texture()->definedHeight()) );
         Vec2f size( float(_image->texture()->width()), float(_image->texture()->height()) );

         // FIXME: This code is not very safe.
         // Update the region if the first node is an output image node.
         if( out->name() == "outputImage" )
         {
            DFOutputImageNode* outImg = (DFOutputImageNode*)out;
            params = outImg->parameters();
         }

         updateImageGeom( params.region().size(), defSize/size );

         // Ortho camera.
         camera( -int(_cameras.size()) );

         // Add the entity if not already done.
         if( !_imageEntity->world() )
         {
            _world->removeAllEntities();
            _world->addEntity( _imageEntity.ptr() );
         }

         changeOutput( out, _world.ptr() );
      }  break;
      case DFSocket::GEOMETRY:
      {
         DFGeomOutput* geomOutput = (DFGeomOutput*)out->output();
         RCP<DFGeometry> geom     = geomOutput->getGeometry();

         if( geom.isNull() )
         {
            emptyOutput();
            return;
         }

         geom->updateMesh();
         _entity->geometry( geom.ptr() );

         // Add the entity if not already done.
         if( !_entity->world() )
         {
            _world->removeAllEntities();
            _world->addEntity( _entity.ptr() );
            _world->addEntity( _light.ptr() );
         }

         // Default camera.
         camera( -1 );

         changeOutput( out, _world.ptr() );
      }  break;
      case DFSocket::ANIMATION:
      {
         DFAnimOutput*    animOutput = (DFAnimOutput*)out->output();
         RCP<SkeletalAnimation> anim = animOutput->getAnimation();
         _anim                       = anim;

         if( anim.isNull() )
         {
            emptyOutput();
            return;
         }

         RCP<MeshGeometry> geom = anim->skeleton()->createMesh();
         _puppet->geometry( geom.ptr() );
         _puppet->setPose( anim.ptr(), _time );

         // Add the entity if not already done.
         if( !_puppet->world() )
         {
            _world->removeAllEntities();
            _world->addEntity( _puppet.ptr() );
            _world->addEntity( _light.ptr() );
         }

         // Default camera.
         camera( -1 );

         changeOutput( out, _world.ptr() );
      }  break;
      case DFSocket::WORLD:
      {
         DFWorldOutput* worldOutout = (DFWorldOutput*)out->output();
         RCP<DFWorld> dfw           = worldOutout->getWorld();

         if( dfw.isNull() )
         {
            emptyOutput();
            return;
         }

         // Create new world and changing to it.
         RCP<World> w = dfw->createWorld();
         w->backgroundColor( _bgColor );

         // Default camera.
         camera( -1 );

         changeOutput( out, w.ptr() );
      }  break;
      case DFSocket::STROKES:
      {
         DFStrokesOutput* strokesOutput = (DFStrokesOutput*)out->output();
         RCP<DFStrokes>   strokes       = strokesOutput->getStrokes();

         if( strokes.isNull() )
         {
            emptyOutput();
            return;
         }

         RCP<DFGeometry>  geom = strokes->geometry();
         if( geom.isNull() )
         {
            emptyOutput();
            return;
         }

         geom->updateMesh();
         _entity->geometry( geom.ptr() );

         // Add the entity if not already done.
         if( !_entity->world() )
         {
            _world->removeAllEntities();
            _world->addEntity( _entity.ptr() );
            _world->addEntity( _light.ptr() );
         }

         // Default camera.
         camera( -1 );

         changeOutput( out, _world.ptr() );
      }  break;
      case DFSocket::POLYGON:
      {
         DFPolygonOutput* polyOutput = (DFPolygonOutput*)out->output();
         RCP<DFPolygon>   poly       = polyOutput->getPolygon();

         if( poly.isNull() )
         {
            emptyOutput();
            return;
         }

         // RCP<DFGeometry>  geom = strokes->geometry();
         // if( geom.isNull() )
         // {
         //    emptyOutput();
         //    return;
         // }

         // geom->updateMesh();
         // _entity->geometry( geom.ptr() );

         // Add the entity if not already done.
         //if( !_entity->world() )
         {
            _world->removeAllEntities();
            //_world->addEntity( _entity.ptr() );
         }

         // Ortho camera.
         //camera( -int(_cameras.size()) );
         camera( -1 );

         changeOutput( out, _world.ptr() );
      }  break;
      default:
         StdErr << "Node type (" << out->type() << ") not supported!\n";
         emptyOutput();
         break;
   }
}

//------------------------------------------------------------------------------
//!
void DFScreen::emptyOutput()
{
   _world->removeAllEntities();
   changeOutput( nullptr, _world.ptr() );
}

//------------------------------------------------------------------------------
//!
void DFScreen::changeOutput( DFNode* output, World* w )
{
   // Same output?
   if( output == _outputNode )
   {
      if( w != world() )
      {
         // Move renderable to the new world.
         RCP<Renderable> renderable = _manip.isValid() ? _manip->renderable() : nullptr;
         if( renderable.isValid() ) world()->removeRenderable( renderable.ptr() );
         world( w );
         if( renderable.isValid() ) world()->addRenderable( renderable.ptr() );
      }
      return;
   }

   // Remove manipulator if we had one
   if( _manip.isValid() )
   {
      manipulators()->remove( _manip.ptr() );
      // Remove renderable is one exist.
      Renderable* renderable = _manip->renderable();
      if( renderable ) world()->removeRenderable( renderable );
      _manip = nullptr;
   }

   // Change world and output.
   world( w );
   _outputNode = output;

   // Update manipulator and renderable.
   if( output )
   {
      RCP<DFNodeEditor> editor = output->edit();
      RCP<Manipulator>   manip = editor.isValid() ? editor->manipulator() : nullptr;
      // Add new manipulator.
      if( manip.isValid() )
      {
         _manip = manip;
         manipulators()->addFront( manip.ptr() );
         if( _manip->renderable() ) world()->addRenderable( _manip->renderable() );
      }
   }

   _onChangeOutput.exec( this, execute );
}

//------------------------------------------------------------------------------
//!
void
DFScreen::updateImageGeom( const Vec2f& size, const Vec2f& uv )
{
   float hx = size.x * 0.5f;
   float hy = size.y * 0.5f;

   float* vertices = _imageMesh->vertices();

   Vec3f::as( vertices     ) = Vec3f( -hx, -hy, 0.0f );
   Vec2f::as( vertices + 3 ) = Vec2f( 0.0f, 0.0f );
   Vec3f::as( vertices + 5 ) = Vec3f( 0.0f, 0.0f, 1.0f );
   vertices += 8;

   Vec3f::as( vertices     ) = Vec3f( hx, -hy, 0.0f );
   Vec2f::as( vertices + 3 ) = Vec2f( uv.x, 0.0f );
   Vec3f::as( vertices + 5 ) = Vec3f( 0.0f, 0.0f, 1.0f );
   vertices += 8;

   Vec3f::as( vertices     ) = Vec3f( hx, hy, 0.0f );
   Vec2f::as( vertices + 3 ) = Vec2f( uv.x, uv.y );
   Vec3f::as( vertices + 5 ) = Vec3f( 0.0f, 0.0f, 1.0f );
   vertices += 8;

   Vec3f::as( vertices     ) = Vec3f( -hx, hy, 0.0f );
   Vec2f::as( vertices + 3 ) = Vec2f( 0.0f, uv.y );
   Vec3f::as( vertices + 5 ) = Vec3f( 0.0f, 0.0f, 1.0f );

   _imageMesh->updateProperties();
   _imageMesh->invalidateRenderableGeometry();
}

//------------------------------------------------------------------------------
//!
const char*
DFScreen::meta() const
{
   return _dfScreen_str_;
}

//------------------------------------------------------------------------------
//!
void
DFScreen::init( VMState* vm )
{
   PlasmaScreen::init( vm );
   if( VM::get( vm, 1, "graph" ) )
   {
      graph( (DFGraph*)VM::toProxy( vm, -1 ) );
      VM::pop( vm, 1 );
   }

   if( VM::get( vm, 1, "camera" ) )
   {
      camera( VM::toInt( vm, -1 ) );
      VM::pop( vm, 1 );
   }
}

//------------------------------------------------------------------------------
//!
bool
DFScreen::performGet( VMState* vm )
{
   switch( _attributes[ VM::toCString( vm, 2 ) ] )
   {
      case ATTRIB_ADD_ON_CHANGE_OUTPUT:
         VM::push( vm, this, dfscreen_addOnChangeOutputVM );
         return true;
      case ATTRIB_CAMERA:
         VM::push( vm, _cameraId );
         return true;
      case ATTRIB_CAMERA_ORIENTATION:
         VM::push( vm, viewport().camera()->orientation() );
         return true;
      case ATTRIB_CAMERA_POSITION:
         VM::push( vm, viewport().camera()->position() );
         return true;
      case ATTRIB_GRAPH:
         VM::pushProxy( vm, graph() );
         return true;
      case ATTRIB_OUTPUT_TYPE:
      {
         const char* type = nullptr;
         if( graph() )
         {
            DFNode* outNode = graph()->output();
            if( outNode ) type = outNode->output()->typeAsStr();
         }
         VM::push( vm, type );
         return true;
      }
      case ATTRIB_REMOVE_ON_CHANGE_OUTPUT:
         VM::push( vm, this, dfscreen_removeOnChangeOutputVM );
         return true;
      case ATTRIB_TIME:
         VM::push( vm, time() );
         return true;
      case ATTRIB_TIME_RANGE:
         VM::push( vm, timeRange() );
         return true;
   }

   return PlasmaScreen::performGet( vm );
}

//------------------------------------------------------------------------------
//!
bool
DFScreen::performSet( VMState* vm )
{
   switch( _attributes[ VM::toCString( vm, 2 ) ] )
   {
      case ATTRIB_ADD_ON_CHANGE_OUTPUT:
         // Read-only.
         return false;
      case ATTRIB_CAMERA:
         camera( VM::toInt( vm, 3 ) );
         return true;
      case ATTRIB_CAMERA_ORIENTATION:
      case ATTRIB_CAMERA_POSITION:
         // Read-only.
         return false;
      case ATTRIB_GRAPH:
         graph( (DFGraph*)VM::toProxy( vm, 3 ) );
         return true;
      case ATTRIB_REMOVE_ON_CHANGE_OUTPUT:
         // Read-only.
         return false;
      case ATTRIB_TIME:
         time( VM::toFloat( vm, 3 ) );
         return true;
      case ATTRIB_TIME_RANGE:
         return false;
   }

   return PlasmaScreen::performSet( vm );
}

//------------------------------------------------------------------------------
//!
bool
DFScreen::isAttribute( const char* name ) const
{
   if( _attributes[ name ] != StringMap::INVALID ) return true;
   return PlasmaScreen::isAttribute( name );
}


NAMESPACE_END
