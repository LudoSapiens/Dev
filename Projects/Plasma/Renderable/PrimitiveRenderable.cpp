/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/Renderable/PrimitiveRenderable.h>

#include <Fusion/Core/Core.h>
#include <Fusion/Resource/ResManager.h>

#include <Base/ADT/StringMap.h>
#include <Base/Dbg/DebugStream.h>

/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN

DBG_STREAM( os_pr, "PrimitiveRenderable" );

//------------------------------------------------------------------------------
//!
enum 
{
   ATTRIB_ATTRIBUTES,
   ATTRIB_FORMAT,
   ATTRIB_INDEX,
   ATTRIB_MATERIAL,
   ATTRIB_OFFSET,
   ATTRIB_ORIENTATION,
   ATTRIB_POSITION,
   ATTRIB_PRIMITIVE,
   ATTRIB_RANGE,
   ATTRIB_STATE_ALPHA,
   ATTRIB_STATE_COLOR,
   ATTRIB_STATE_DEPTH,
   ATTRIB_STATE_STENCIL,
   ATTRIB_TRANSFORM,
   ATTRIB_TYPE
};

//------------------------------------------------------------------------------
//!
StringMap _attributes(
   "attributes",  ATTRIB_ATTRIBUTES,
   "format",      ATTRIB_FORMAT,
   "index",       ATTRIB_INDEX,
   "mat",         ATTRIB_MATERIAL,
   "offset",      ATTRIB_OFFSET,
   "orientation", ATTRIB_ORIENTATION,
   "position",    ATTRIB_POSITION,
   "prim",        ATTRIB_PRIMITIVE,
   "range",       ATTRIB_RANGE,
   "alpha",       ATTRIB_STATE_ALPHA,
   "color",       ATTRIB_STATE_COLOR,
   "depth",       ATTRIB_STATE_DEPTH,
   "stencil",     ATTRIB_STATE_STENCIL,
   "xform",       ATTRIB_TRANSFORM,
   "type",        ATTRIB_TYPE,
   ""
);

UNNAMESPACE_END


USING_NAMESPACE


/*==============================================================================
  CLASS PrimitiveRenderable
==============================================================================*/
//------------------------------------------------------------------------------
//!
PrimitiveRenderable::PrimitiveRenderable
( void )
{
   _geom = Core::gfx()->createGeometry( Gfx::PRIM_INVALID );
   _xform = Reff::identity();
   _range = Vec2i::zero();
   _hasRange = false;
}

//------------------------------------------------------------------------------
//!
PrimitiveRenderable::~PrimitiveRenderable
( void )
{
   
}

//------------------------------------------------------------------------------
//!
void
PrimitiveRenderable::render
( const RCP<Gfx::Pass>& pass ) const
{
   //DBG_BLOCK( os_pr, "PrimitiveRenderable::render" );

   CHECK(_mat.isValid());
   _mat->apply(*pass);

   if( _aState.isValid() )
   {
      pass->setAlphaState(_aState);
   }

   if( _cState.isValid() )
   {
      pass->setColorState(_cState);
   }

   if( _dState.isValid() )
   {
      pass->setDepthState(_dState);
   }

   if( _sState.isValid() )
   {
      pass->setStencilState(_sState);
   }

   pass->setWorldMatrix( _xform.toMatrix().ptr() );
   if( _hasRange )
   {
      pass->execRangeGeometry( _geom, (const uint*)_range.ptr() );
   }
   else
   {
      pass->execGeometry( _geom );
   }

   //Reset?
   if( _sState.isValid() )
   {
      pass->setStencilState( RCP<Gfx::StencilState>(new Gfx::StencilState) );
   }

   if( _dState.isValid() )
   {
      pass->setDepthState( RCP<Gfx::DepthState>(new Gfx::DepthState) );
   }

   if( _cState.isValid() )
   {
      pass->setColorState( RCP<Gfx::ColorState>(new Gfx::ColorState) );
   }

   if( _aState.isValid() )
   {
      pass->setAlphaState( RCP<Gfx::AlphaState>(new Gfx::AlphaState) );
   }
}

//------------------------------------------------------------------------------
//!
void
PrimitiveRenderable::init
( VMState* vm )
{
   DBG_BLOCK( os_pr, "PrimitiveRenderable::init" );
   if( VM::isTable(vm, -1) )
   {
      // Start iterating at index 0 (nil, pushed below).
      VM::push(vm);
      while( VM::next(vm, -2) )
      {
         // Let the performSet() routine handle the various assignments.
         performSet(vm);

         // Pop the value, but keep the key.
         VM::pop(vm, 1);
      }
   }
   else
   {
      DBG_MSG( os_pr, "Top of stack isn't a table" );
   }
}

//------------------------------------------------------------------------------
//!
bool
PrimitiveRenderable::performGet
( VMState* vm )
{
   const char* str = VM::toCString( vm, -1 );
   switch( _attributes[str] )
   {
      case ATTRIB_PRIMITIVE:
      {
         VM::push(vm, (uint)_geom->primitiveType());
      } return true;
      case ATTRIB_MATERIAL:
      {
         MaterialVM::push(vm, _mat);
      } return true;
      case ATTRIB_POSITION:
      {
         VM::push(vm, _xform.position());
      } return true;
      case ATTRIB_ORIENTATION:
      {
         VM::push(vm, _xform.orientation());
      } return true;
      case ATTRIB_TRANSFORM:
      {
         VM::push(vm, _xform);
      } return true;
      default:
      {
         // Nothing/read-only, push nil.
         VM::push( vm );
      } return true;
   } //switch..case
   return false;
}

//------------------------------------------------------------------------------
//!
bool
PrimitiveRenderable::performSet
( VMState* vm )
{
   if( VM::isNumber(vm, -2) )
   {
      // Do NOT call toCString else you screw up the iterator used in init().
      return setBuffer( vm, String(VM::toUInt(vm, -2)) );
   }

   const char* str = VM::toCString( vm, -2 );
   switch( _attributes[str] )
   {
      case ATTRIB_PRIMITIVE:
      {
         //Ex: prim = const_prim.prim_type.TRIANGLES
         _geom->primitiveType((Gfx::PrimitiveType)VM::toUInt(vm, -1));
         DBG_MSG( os_pr, "PrimType: " << _geom->primitiveType() );
      } return true;
      case ATTRIB_MATERIAL:
      {
         //Ex: mat = "colorTex"
         _mat = MaterialVM::to(vm, -1);
         if( _mat.isValid() )
         {
            DBG_MSG( os_pr, "Successfully loaded material" );
         }
         else
         {
            DBG_MSG( os_pr, "Could not load material" );
         }
      } return true;
      case ATTRIB_POSITION:
      {
         //Ex: pos = { x, y, z }
         _xform.position( VM::toVec3f(vm, -1) );
         DBG_MSG( os_pr, "Pos: " << _xform.position() );
      } return true;
      case ATTRIB_ORIENTATION:
      {
         //Ex: orient = { x, y, z, w }
         _xform.orientation( VM::toQuatf(vm, -1) );
         DBG_MSG( os_pr, "Orient: " << _xform.orientation() );
      } return true;
      case ATTRIB_TRANSFORM:
      {
         //Ex: xform = { ...something... }
         _xform = VM::toReff(vm, -1);
         DBG_MSG( os_pr, "XForm: " << _xform );
      } return true;
      case ATTRIB_RANGE:
      {
         //Ex: range = { startIndex, numIndices }
         if( VM::isNil(vm, -1) )
         {
            _hasRange = false;
            DBG_MSG( os_pr, "Range: <nil>" );
         }
         else
         {
            _hasRange = true;
            _range = VM::toVec2i(vm, -1);
            DBG_MSG( os_pr, "Range: " << _range );
         }
      } return true;
      case ATTRIB_STATE_ALPHA:
      {
         //Ex: alpha = { blending=1, src=const_prim.alpha_blend.ONE, dst=const_prim.alpha_blend.ONE_MINUS_SRC_ALPHA }
         if( VM::isTable(vm, -1) )
         {
            if( _aState.isNull() )  _aState = new Gfx::AlphaState();

            int tmp;
            float ref;
            if( VM::get(vm, -1, "blending", tmp) )
            {
               _aState->alphaBlending( tmp != 0 );
               DBG_MSG( os_pr, "Alpha blending set to: " << (tmp != 0) );
            }
            if( VM::get(vm, -1, "src", tmp) )
            {
               _aState->alphaBlendSrc( (Gfx::AlphaBlend)tmp );
               DBG_MSG( os_pr, "Alpha blend source set to: " << tmp );
            }
            if( VM::get(vm, -1, "dst", tmp) )
            {
               _aState->alphaBlendDst( (Gfx::AlphaBlend)tmp );
               DBG_MSG( os_pr, "Alpha blend destination set to: " << tmp );
            }
            if( VM::get(vm, -1, "testing", tmp) )
            {
               _aState->alphaTesting( tmp != 0 );
               DBG_MSG( os_pr, "Alpha testing set to: " << (tmp != 0) );
            }
            if( VM::get(vm, -1, "func", tmp) )
            {
               _aState->alphaTestFunc( (Gfx::CompareFunc)tmp );
               DBG_MSG( os_pr, "Alpha test compare function set to: " << tmp );
            }
            if( VM::get(vm, -1, "ref", ref) )
            {
               _aState->alphaTestRef( ref );
               DBG_MSG( os_pr, "Alpha test reference value set to: " << ref );
            }
         }
         else
         {
            DBG_MSG( os_pr, "'alpha' not a table" );
         }
         DBG_MSG( os_pr, "AlphaState changed" );
      } return true;
      case ATTRIB_STATE_COLOR:
      {
         //Ex: color = { writing=0 }
         if( VM::isTable(vm, -1) )
         {
            if( _cState.isNull() )  _cState = new Gfx::ColorState();

            int tmp;
            if( VM::get(vm, -1, "writing", tmp) )
            {
               _cState->colorWriting( tmp != 0 );
               DBG_MSG( os_pr, "Color writing set to: " << (tmp != 0) );
            }
         }
         else
         {
            DBG_MSG( os_pr, "'color' not a table" );
         }
         DBG_MSG( os_pr, "ColorState changed" );
      } return true;
      case ATTRIB_STATE_DEPTH:
      {
         //Ex: depth = { testing=1, writing=1, func=const_prim.compare_func.LESS_EQUAL }
         if( VM::isTable(vm, -1) )
         {
            if( _dState.isNull() )  _dState = new Gfx::DepthState();

            int tmp;
            if( VM::get(vm, -1, "testing", tmp) )
            {
               _dState->depthTesting( tmp != 0 );
               DBG_MSG( os_pr, "Depth testing set to: " << (tmp != 0) );
            }
            if( VM::get(vm, -1, "writing", tmp) )
            {
               _dState->depthWriting( tmp != 0 );
               DBG_MSG( os_pr, "Depth writing set to: " << (tmp != 0) );
            }
            if( VM::get(vm, -1, "func", tmp) )
            {
               _dState->depthTestFunc( (Gfx::CompareFunc)tmp );
               DBG_MSG( os_pr, "Depth compare function set to: " << tmp );
            }
         }
         else
         {
            DBG_MSG( os_pr, "'depth' not a table" );
         }
         DBG_MSG( os_pr, "DepthState changed" );
      } return true;
      case ATTRIB_STATE_STENCIL:
      {
         //Ex: stencil = { testing=1, ref=tonumber("0xFF00FF00", 16), func=const_prim.compare_func.GREATER_EQUAL }
         if( VM::isTable(vm, -1) )
         {
            if( _sState.isNull() )  _sState = new Gfx::StencilState();

            int tmp;
            if( VM::get(vm, -1, "testing", tmp) )
            {
               _sState->stencilTesting( tmp != 0 );
               DBG_MSG( os_pr, "Stencil testing set to: " << (tmp != 0) );
            }
            if( VM::get(vm, -1, "ref", tmp) )
            {
               _sState->stencilTestRef( tmp );
               DBG_MSG( os_pr, "Stencil reference value set to: " << tmp );
            }
            if( VM::get(vm, -1, "refMask", tmp) )
            {
               _sState->stencilTestRefMask( tmp );
               DBG_MSG( os_pr, "Stencil reference mask value set to: " << tmp );
            }
            if( VM::get(vm, -1, "writeMask", tmp) )
            {
               _sState->stencilTestWriteMask( tmp );
               DBG_MSG( os_pr, "Stencil write value set to: " << tmp );
            }
            if( VM::get(vm, -1, "func", tmp) )
            {
               _sState->stencilTestFunc( (Gfx::CompareFunc)tmp );
               DBG_MSG( os_pr, "Stencil test function set to: " << tmp );
            }
            if( VM::get(vm, -1, "stencilFailOp", tmp) )
            {
               _sState->stencilFailOp( (Gfx::StencilOp)tmp );
               DBG_MSG( os_pr, "Stencil-fail operation set to: " << tmp );
            }
            if( VM::get(vm, -1, "stencilPassDepthFailOp", tmp) )
            {
               _sState->stencilPassDepthFailOp( (Gfx::StencilOp)tmp );
               DBG_MSG( os_pr, "Stencil-pass-but-depth-fail operation set to: " << tmp );
            }
            if( VM::get(vm, -1, "stencilPassDepthPassOp", tmp) )
            {
               _sState->stencilPassDepthPassOp( (Gfx::StencilOp)tmp );
               DBG_MSG( os_pr, "Stencil-pass-and-depth-pass operation set to: " << tmp );
            }
         }
         else
         {
            DBG_MSG( os_pr, "'stencil' not a table" );
         }
         DBG_MSG( os_pr, "StencilState changed" );
      } return true;
      case ATTRIB_INDEX:
      {
         if( VM::isTable(vm, -1) )
         {
            uint format;
            VM::get(vm, -1, "format", format);
            uint numElems = VM::getTableSize(vm, -1);
            DBG_MSG( os_pr, "Format: " << format << " (" << numElems << " elements)" );
            _range(0) = 0;
            _range(1) = numElems;
            switch( format )
            {
               case Gfx::INDEX_FMT_8:
               {
                  Vector<uchar> index;
                  index.reserve(numElems);
                  for( int i = 1; VM::geti( vm, -1, i ); ++i )
                  {
                     index.pushBack(VM::toUInt(vm, -1));
                     VM::pop(vm, 1);
                  }
                  _geom->indexBuffer(
                     Core::gfx()->createBuffer( (Gfx::IndexFormat)format, Gfx::BUFFER_FLAGS_NONE, index.dataSize(), index.data() )
                  );
               } break;
               case Gfx::INDEX_FMT_16:
               {
                  Vector<ushort> index;
                  index.reserve(numElems);
                  for( int i = 1; VM::geti( vm, -1, i ); ++i )
                  {
                     index.pushBack(VM::toUInt(vm, -1));
                     VM::pop(vm, 1);
                  }
                  _geom->indexBuffer(
                     Core::gfx()->createBuffer( (Gfx::IndexFormat)format, Gfx::BUFFER_FLAGS_NONE, index.dataSize(), index.data() )
                  );
               } break;
               case Gfx::INDEX_FMT_32:
               {
                  Vector<uint> index;
                  index.reserve(numElems);
                  for( int i = 1; VM::geti( vm, -1, i ); ++i )
                  {
                     index.pushBack(VM::toUInt(vm, -1));
                     VM::pop(vm, 1);
                  }
                  /**
                  for( uint i = 0; i < index.size(); ++i )
                  {
                     DBG_MSG( os_pr, "Index[" << i << "]: " << index[i] );
                  }
                  **/
                  _geom->indexBuffer(
                     Core::gfx()->createBuffer( (Gfx::IndexFormat)format, Gfx::BUFFER_FLAGS_NONE, index.dataSize(), index.data() )
                  );
               } break;
               default:
               {
                  DBG_MSG( os_pr, "Invalid index format: " << format );
                  CHECK(false);
               }
            }
         }
         else
         {
            DBG_MSG( os_pr, "'index' not a table" );
         }
      } return true;
      default:
      {
         return setBuffer(vm, String(str));
      }
   }  //switch..case

   return false;
}

//------------------------------------------------------------------------------
//!
bool
PrimitiveRenderable::setBuffer( VMState* vm, const String& name )
{
   if( VM::isTable(vm, -1) )
   {
      if( VM::get(vm, -1, "attributes") )
      {
         if( VM::isTable(vm, -1) )
         {
            RCP<Gfx::VertexBuffer> buffer( Core::gfx()->createBuffer(Gfx::BUFFER_FLAGS_NONE, 0, NULL) );
            DBG_MSG( os_pr, VM::getTableSize(vm, -1) << " attributes:" );
            for( int i = 1; VM::geti( vm, -1, i ); ++i )
            {
               if( VM::isTable(vm, -1) )
               {
                  uint attribType, attribFormat;
                  int attribOffset = -1;  //auto-determination
                  bool okT = VM::get(vm, -1, "type", attribType);
                  bool okF = VM::get(vm, -1, "format", attribFormat);
                  VM::get(vm, -1, "offset", attribOffset);
                  if( okT && okF )
                  {
                     uint oldAttribFormat = attribFormat;
                     switch( Gfx::toNumChannels((Gfx::AttributeFormat)attribFormat) )
                     {
                        case 0: break;
                        case 1: attribFormat = Gfx::ATTRIB_FMT_32F;             break;
                        case 2: attribFormat = Gfx::ATTRIB_FMT_32F_32F;         break;
                        case 3: attribFormat = Gfx::ATTRIB_FMT_32F_32F_32F;     break;
                        case 4: attribFormat = Gfx::ATTRIB_FMT_32F_32F_32F_32F; break;
                     }
                     if( attribFormat != oldAttribFormat )
                     {
                        printf("WARNING - PrimitiveRenderable: replaced attribute format from %d to %d (supports only floats)\n",
                               oldAttribFormat, attribFormat);
                     }

                     if( attribOffset < 0 )
                     {
                        attribOffset = buffer->strideInBytes();
                     }
                     DBG_MSG( os_pr, "  Attrib #" << i << ": type=" << attribType << " format=" << attribFormat << " offset=" << attribOffset << ")" );
                     buffer->addAttribute(
                        (Gfx::AttributeType)attribType,
                        (Gfx::AttributeFormat)attribFormat,
                        attribOffset
                     );
                  }
                  else
                  {
                     if( okT )
                     {
                        DBG_MSG( os_pr, "Attribute missing 'format' attribute" );
                     }
                     else
                     if( okF )
                     {
                        DBG_MSG( os_pr, "Attribute missing 'type' attribute" );
                     }
                     else
                     {
                        DBG_MSG( os_pr, "Attribute missing 'type' and 'format' attributes" );
                     }
                  }
               }
               else
               {
                  DBG_MSG( os_pr, "Attribute #" << i << " not specified in a table" );
               }
               VM::pop(vm, 1);
            }

            // Pop the attributes table.
            VM::pop(vm, 1);

            // Then, iterate over the data.
            Vector<float>  data;  //FIXME: only floats supported for now
            uint numElems = VM::getTableSize(vm, -1);
            data.reserve(numElems*4);
            for( int i = 1; VM::geti(vm, -1, i); ++i )
            {
               data.pushBack( (float)VM::toNumber(vm, -1) );
               VM::pop(vm, 1);
            }
            /**
            for( uint i = 0; i < data.size(); ++i )
            {
               DBG_MSG( os_pr, "data[" << i << "]: " << data[i] );
            }
            **/
            Core::gfx()->setData(buffer, data.dataSize(), data.data());

            CurBufMap::Iterator cur = _curBufMap.find( name );
            if( cur != _curBufMap.end() )
            {
               _geom->removeBuffer( (*cur).second );
               (*cur).second = buffer.ptr();
            }
            else
            {
               _curBufMap[name] = buffer.ptr();
            }
            _geom->addBuffer(buffer);
         }
         else
         {
            DBG_MSG( os_pr, "'attributes' not a table" );
            VM::pop(vm, 1);
         }
      }
      else
      {
         DBG_MSG( os_pr, "Missing 'attributes' table" );
      }
   }
   else
   {
      DBG_MSG( os_pr, "Vertex buffer is not a table" );
   }
   return true;
}
