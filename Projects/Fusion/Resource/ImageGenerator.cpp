/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Fusion/Resource/ImageGenerator.h>

#include <Fusion/Resource/BitmapManipulator.h>
#include <Fusion/VM/VM.h>
#include <Fusion/VM/VMRegistry.h>

#include <Base/ADT/StringMap.h>
#include <Base/Dbg/Defs.h>
#include <Base/Util/Timer.h>

USING_NAMESPACE

/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN

const char* _context_str = "ImageGeneratorContext";

//------------------------------------------------------------------------------
//!
struct Context
{
   Vec2f        _xy;        //!< The current pixel index being generated.
   Vec2f        _size;      //!< The total size of the image being generated.
   Vec2f        _size_inv;  //!< 1/_size for faster UV calculations.
   RCP<Bitmap>  _bitmap;    //!< The Bitmap storing the generated pixels.

   Context( const Vec2i& size, const Bitmap::PixelType type, const int numChannels ):
      _xy( 0.0f ),
      _size( size )
   {
      _size_inv = Vec2f( 1.0f ) / _size;
      _bitmap   = new Bitmap( size, type, numChannels );
   }
};

//------------------------------------------------------------------------------
//!
int rectVM( VMState* vm )
{
   Context* context = (Context*)VM::userData( vm );

   Vec2f pos( 0.0f );
   Vec2f size( context->_size );
   switch( VM::getTop( vm ) )
   {
      case 0:
         StdErr << "ImageGenerator::rectVM() - Missing arguments." << nl;
         return 0;
      case 1:
         // [ func ]
         break;
      case 2:
         // [ size, func ]
         size = VM::toVec2f( vm, 1 );
         break;
      case 3:
         // [ pos, size, func ]
         pos  = VM::toVec2f( vm, 1 );
         size = VM::toVec2f( vm, 2 );
         break;
   }

   // [ ..., func ]

   if( VM::isFunction( vm, -1 ) )
   {
      // Execute function for every pixel.
      Context** inObj = (Context**)VM::newObject( vm, sizeof(Context*) );  // [..., func, cntx]
      *inObj = context;
      VM::getMetaTable( vm, _context_str );                                // [..., func, cntx, mt]
      VM::setMetaTable( vm, -2 );                                          // [..., func, cntxProxy]

#define DEF_CASE( Type, VMFunc ) \
      do                                                                               \
      {                                                                                \
         Type* cur = (Type*)context->_bitmap->pixels();                                \
         for( context->_xy.y = sPos.y; context->_xy.y <= ePos.y; ++context->_xy.y )    \
         {                                                                             \
            for( context->_xy.x = sPos.x; context->_xy.x <= ePos.x; ++context->_xy.x ) \
            {                                                                          \
               VM::pushValue( vm, -2 );  /* [..., func, cntxProxy, func]            */ \
               VM::pushValue( vm, -2 );  /* [..., func, cntxProxy, func, cntxProxy] */ \
               VM::ecall( vm, 1, 1 );    /* [..., func, cntxProxy, result]          */ \
               *cur = VM::VMFunc( vm, -1 );                                            \
               cur += 1;                                                               \
               VM::pop( vm );            /* [..., func, cntxProxy]                  */ \
            }                                                                          \
         }                                                                             \
      }                                                                                \
      while( 0 )

      Vec2f sPos = pos;
      Vec2f ePos = sPos + size;
      ePos -= 1.0f; // Last pixel is inclusive.
      switch( context->_bitmap->numChannels() )
      {
         case 1: DEF_CASE( float, toFloat ); break;
         case 2: DEF_CASE( Vec2f, toVec2f ); break;
         case 3: DEF_CASE( Vec3f, toVec3f ); break;
         case 4: DEF_CASE( Vec4f, toVec4f ); break;
         default:
            StdErr << "ImageGenerator::rectVM() - Invalid number of channels in bitmap: " << context->_bitmap->numChannels() << "." << nl;
            break;
      }

#undef DEF_CASE

      // Clean up the stack.
      VM::pop( vm ); // [..., func]
   }
   else
   {
      // Assign a constant color to every pixel.
      switch( context->_bitmap->numChannels() )
      {
         case 1:
         {
            float v = VM::toFloat( vm, -1 );
            BitmapManipulator::fillRect( *(context->_bitmap), pos, size, &v );
         }  break;
         case 2:
         {
            Vec2f v = VM::toVec2f( vm, -1 );
            BitmapManipulator::fillRect( *(context->_bitmap), pos, size, &v );
         }  break;
         case 3:
         {
            Vec3f v = VM::toVec3f( vm, -1 );
            BitmapManipulator::fillRect( *(context->_bitmap), pos, size, &v );
         }  break;
         case 4:
         {
            Vec4f v = VM::toVec4f( vm, -1 );
            BitmapManipulator::fillRect( *(context->_bitmap), pos, size, &v );
         }  break;
         default:
            StdErr << "ImageGenerator::rectVM() - Invalid number of channels in bitmap: " << context->_bitmap->numChannels() << "." << nl;
            break;
      }
   }

   return 0;
}

//------------------------------------------------------------------------------
//!
const VM::Reg funcs[] = {
   { "rect"   , rectVM    },
   //{ "circle" , circleVM  },
   { 0,0 }
};

//------------------------------------------------------------------------------
//!
enum {
   ATTRIB_U,
   ATTRIB_UV,
   ATTRIB_V,
   ATTRIB_X,
   ATTRIB_XY,
   ATTRIB_Y
};

StringMap _attributes(
   "u"     ,  ATTRIB_U     ,
   "uv"    ,  ATTRIB_UV    ,
   "v"     ,  ATTRIB_V     ,
   "x"     ,  ATTRIB_X     ,
   "xy"    ,  ATTRIB_XY    ,
   "y"     ,  ATTRIB_Y     ,
   ""
);

//------------------------------------------------------------------------------
//!
int contextGetVM( VMState* vm )
{
   Context* context = *(Context**)VM::toPtr( vm, 1 );
   CHECK( context );
   switch( _attributes[ VM::toCString( vm, 2 ) ] )
   {
      case ATTRIB_U:
         VM::push( vm, context->_xy.x * context->_size_inv.x );
         return 1;
      case ATTRIB_UV:
         VM::push( vm, context->_xy * context->_size_inv );
         return 1;
      case ATTRIB_V:
         VM::push( vm, context->_xy.y * context->_size_inv.y );
         return 1;
      case ATTRIB_X:
         VM::push( vm, context->_xy.x );
         return 1;
      case ATTRIB_XY:
         VM::push( vm, context->_xy );
         return 1;
      case ATTRIB_Y:
         VM::push( vm, context->_xy.y );
         return 1;
      default:
         // TODO: Retrieve from arguments Table.
         break;
   }
   return 0;
}

//------------------------------------------------------------------------------
//!
int contextSetVM( VMState* vm )
{
   StdErr << "WARNING - ImageGenerator - Parameter '" << VM::toCString( vm, 2 ) << "' is read-only." << nl;
   return 0;
}

//------------------------------------------------------------------------------
//!
void initVM( VMState* vm, uint /*mask*/ )
{
   VM::registerFunctions( vm, "_G", funcs );
}

UNNAMESPACE_END

/*==============================================================================
  CLASS ImageGenerator
==============================================================================*/

//------------------------------------------------------------------------------
//!
void
ImageGenerator::initialize()
{
   VMRegistry::add( initVM, VM_CAT_IMAGE );
   VMRegistry::add( _context_str, NULL, contextGetVM, contextSetVM, VM_CAT_IMAGE );
}

//------------------------------------------------------------------------------
//!
ImageGenerator::ImageGenerator(
   const           String&  path,
   const            Table*  args,
   const            Vec2i&  size,
   const Bitmap::PixelType  type,
   const               int  numChannels
):
   _path( path ),
   _args( args ),
   _size( size ),
   _type( type ),
   _chan( numChannels )
{
   _result = new Image( path );
}

//------------------------------------------------------------------------------
//!
//ImageGenerator( const String& path, const Vec2i& size, const Table& args );

//------------------------------------------------------------------------------
//!
ImageGenerator::~ImageGenerator()
{

}

//------------------------------------------------------------------------------
//!
void
ImageGenerator::execute()
{
   // Prepare to build image.
   VMState* vm = VM::open( VM_CAT_IMAGE | VM_CAT_MATH, true );

   Context context( _size, Bitmap::FLOAT, _chan );
   VM::userData( vm, &context );

   // Set a SIZE global variable.
   VM::push( vm, _size );
   VM::setGlobal( vm, "SIZE" );

   Timer timer;
   if( _args.isValid() )
   {
      // Push arguments Table.
      VM::push( vm, *_args );
      // Execute script with arguments.
      VM::doFile( vm, _path, 1, 0 );
   }
   else
   {
      // Execute script.
      VM::doFile( vm, _path, 0 );
   }
   double t = timer.elapsed();
   double r = _size.x * _size.y;
   r /= t;
   StdErr << "Took " << t << "s (throughput: " << r << " px/s)" << nl;

   VM::close( vm );

   if( context._bitmap->pixelType() != _type )
   {
      // Convert to the user-specified type.
      context._bitmap = BitmapManipulator::convert( *(context._bitmap), _type );
      CHECK( context._bitmap->pixelType() == _type );
   }

   _result->bitmap( context._bitmap.ptr() );
}
