/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef FUSION_VM_FMT_H
#define FUSION_VM_FMT_H

#include <Fusion/StdDefs.h>

#include <CGMath/Ref.h>

#include <Base/IO/TextStream.h>

NAMESPACE_BEGIN


/*==============================================================================
   CLASS VMOut
==============================================================================*/
//!< A class used to format some data types in a format accepted by the VM.
template< typename T >
class VMOut
{
   public:
      VMOut( const T& t, StreamIndent* i ) : _t( t ), _indent( i ) {}

      const T&      _t;
      StreamIndent* _indent;
};

//------------------------------------------------------------------------------
//! A generic TextStream wrapper to allow calls of the form:
//!   os << VMFmt( myVec3 );
//! to defer to one of the overloaded functions defined below (or elsewhere).
template< typename T >
VMOut<T> VMFmt( const T& t )
{
   return VMOut<T>(t, nullptr);
}

//------------------------------------------------------------------------------
//!
template< typename T >
VMOut<T> VMFmt( const T& t, StreamIndent& i )
{
   return VMOut<T>(t,&i);
}

/*==============================================================================
   STREAM OPERATORS
==============================================================================*/

//------------------------------------------------------------------------------
//!
template< typename T > inline TextStream&
operator<<( TextStream& os, const VMOut<T>& v )
{
   return os << v._t;
}

//------------------------------------------------------------------------------
//!
inline TextStream&
operator<<( TextStream& os, const VMOut<bool>& v )
{
   return os << ( v._t ? "true" : "false" );
}

//------------------------------------------------------------------------------
//!
inline TextStream&
operator<<( TextStream& os, const VMOut<const char*>& v )
{
   return os << "\"" << v._t << "\"";
}

//------------------------------------------------------------------------------
//!
inline TextStream&
operator<<( TextStream& os, const VMOut<String>& v )
{
   return os << "\"" << v._t << "\"";
}

//------------------------------------------------------------------------------
//!
inline TextStream&
operator<<( TextStream& os, const VMOut<ConstString>& v )
{
   return os << "\"" << v._t << "\"";
}

//------------------------------------------------------------------------------
//!
inline TextStream&
operator<<( TextStream& os, const VMOut<Vec2i>& v )
{
   return os << "vec2(" << v._t(0) << "," << v._t(1) << ")";
}

//------------------------------------------------------------------------------
//!
inline TextStream&
operator<<( TextStream& os, const VMOut<Vec2f>& v )
{
   return os << "vec2(" << v._t(0) << "," << v._t(1) << ")";
}

//------------------------------------------------------------------------------
//!
inline TextStream&
operator<<( TextStream& os, const VMOut<Vec3f>& v )
{
   return os << "vec3(" << v._t(0) << "," << v._t(1) << "," << v._t(2) << ")";
}

//------------------------------------------------------------------------------
//!
inline TextStream&
operator<<( TextStream& os, const VMOut<Vec4f>& v )
{
   return os << "vec4(" << v._t(0) << "," << v._t(1) << "," << v._t(2) << "," << v._t(3) << ")";
}

//------------------------------------------------------------------------------
//!
inline TextStream&
operator<<( TextStream& os, const VMOut<Quatf>& v )
{
   return os << "quat(" << v._t.x() << "," << v._t.y() << "," << v._t.z() << "," << v._t.w() << ")";
}

//------------------------------------------------------------------------------
//!
inline TextStream&
operator<<( TextStream& os, const VMOut<Reff>& v )
{
   return os << "{" << VMFmt(v._t.orientation()) << "," << VMFmt(v._t.position()) << "}";
}

//------------------------------------------------------------------------------
//! Forward declare so that the correct specialized version will be used next.
inline TextStream&
operator<<( TextStream& os, const VMOut<Variant>& v );

//------------------------------------------------------------------------------
//!
inline TextStream&
operator<<( TextStream& os, const VMOut<Table>& v )
{
   os << "{" << nl;
   ++(*v._indent);
   // Integer key.
   for( size_t i = 0; i < v._t.arraySize(); ++i )
   {
      os << *v._indent << VMFmt( v._t[i], *v._indent ) << "," << nl;
   }
   // ConstString key.
   for( auto cur = v._t.begin(); cur != v._t.end(); ++cur )
   {
      os << *v._indent << (*cur).first << " = " << VMFmt( (*cur).second, *v._indent ) << "," << nl;
   }
   --(*v._indent);
   os << *v._indent << "}";
   return os;
}

//------------------------------------------------------------------------------
//!
inline TextStream&
operator<<( TextStream& os, const VMOut<Variant>& v )
{
   switch( v._t.type() )
   {
      case Variant::NIL:                                                  break;
      case Variant::BOOL:    os << VMFmt( v._t.getBoolean() );            break;
      case Variant::FLOAT:   os << v._t.getFloat();                       break;
      case Variant::VEC2:    os << VMFmt( v._t.getVec2() );               break;
      case Variant::VEC3:    os << VMFmt( v._t.getVec3() );               break;
      case Variant::VEC4:    os << VMFmt( v._t.getVec4() );               break;
      case Variant::QUAT:    os << VMFmt( v._t.getQuat() );               break;
      case Variant::STRING:  os << VMFmt( v._t.getString() );             break;
      case Variant::POINTER:                                              break;
      case Variant::TABLE:   os << VMFmt( *v._t.getTable(), *v._indent );  break;
      default:
         break;
   }
   return os;
}


NAMESPACE_END

#endif //FUSION_VM_FMT_H
