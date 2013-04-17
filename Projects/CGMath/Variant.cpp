/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <CGMath/Variant.h>

#include <Base/Dbg/Defs.h>

NAMESPACE_BEGIN

/*==============================================================================
   CLASS Variant
==============================================================================*/

Variant Variant::_null;

//------------------------------------------------------------------------------
//!
void
Variant::print( TextStream& os ) const
{
   //os << "type:" << type() << " ";
   switch( type() )
   {
      case NIL    : os << "<nil>"; break;
      case BOOL   : os << (getBoolean()?"true":"false"); break;
      case FLOAT  : os << getFloat(); break;
      case VEC2   : os << getVec2(); break;
      case VEC3   : os << getVec3(); break;
      case VEC4   : os << getVec4(); break;
      case QUAT   : os << getQuat(); break;
      case STRING : os << getString().cstr(); break;
      case POINTER: os << getPointer(); break;
      case TABLE  : getTable()->print( os ); break;
      default:
         CHECK( false );
         break;
   }
}

//------------------------------------------------------------------------------
//!
void
Variant::print( StreamIndent& indent, TextStream& os ) const
{
   //os << "type:" << type() << " ";
   switch( type() )
   {
      case NIL    : os << "<nil>"; break;
      case BOOL   : os << (getBoolean()?"true":"false"); break;
      case FLOAT  : os << getFloat(); break;
      case VEC2   : os << getVec2(); break;
      case VEC3   : os << getVec3(); break;
      case VEC4   : os << getVec4(); break;
      case QUAT   : os << getQuat(); break;
      case STRING : os << '"' << getString().cstr() << '"'; break;
      case POINTER: os << getPointer(); break;
      case TABLE  : getTable()->print( indent, os ); break;
      default:
         CHECK( false );
         break;
   }
}


/*==============================================================================
   CLASS Table
==============================================================================*/

RCP<Table> Table::_null = new Table();

//-----------------------------------------------------------------------------
//!
void
Table::extend( const Table& t )
{
   for( MapContainer::ConstIterator cur = t._map.begin();
        cur != t._map.end();
        ++cur )
   {
      _map[(*cur).first] = (*cur).second;
   }

   for( ArrayContainer::ConstIterator cur = t._array.begin();
        cur != t._array.end();
        ++cur )
   {
      _array.pushBack( *cur );
   }
}

//------------------------------------------------------------------------------
//!
void
Table::print( TextStream& os ) const
{
   os << "{";
   const char* pre = "";
   for( uint i = 0; i < _array.size(); ++i )
   {
      os << pre;
      _array[i].print( os );
      pre = ",";
   }
   for( ConstIterator cur = begin(); cur != end(); ++cur )
   {
      os << pre << (*cur).first.cstr() << "=";
      (*cur).second.print( os );
      pre = ",";
   }
   os << "}";
}

//------------------------------------------------------------------------------
//!
void
Table::print( StreamIndent& indent, TextStream& os ) const
{
   os << indent << "{" << nl;
   ++indent;
   for( uint i = 0; i < _array.size(); ++i )
   {
      os << indent;
      _array[i].print( os );
      os << "," << nl;
   }
   for( ConstIterator cur = begin(); cur != end(); ++cur )
   {
      os << indent << (*cur).first.cstr() << " = ";
      (*cur).second.print( indent, os );
      os << "," << nl;
   }
   --indent;
   os << indent << "}" << nl;
}

NAMESPACE_END
