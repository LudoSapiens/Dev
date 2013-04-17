/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Fusion/Core/EventProfiler.h>

#include <Base/Dbg/Defs.h>
#include <Base/IO/FileDevice.h>

/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN

inline String  fmtReadable( size_t s, bool si = false )
{
   const char* unitsSI[] = { "B", "KB" , "MB" , "GB" , "TB" , "PB" , "EB"  };
   const char* unitsBI[] = { "B", "KiB", "MiB", "GiB", "TiB", "PiB", "EiB" };

   size_t scale;
   const char** units;
   if( si )
   {
      scale = 1000;
      units = unitsSI;
   }
   else
   {
      scale = 1024;
      units = unitsBI;
   }

   // Doubles cannot represent 64b values, so we compute the fraction with
   // at least a full spread of "scale".
   size_t denom1 = 1;
   size_t denom2 = 1;
   for( size_t range = scale; range < s; range *= scale )
   {
      denom1 = denom2;
      denom2 = scale;
      ++units;
   }

   //StdErr << "fmt s=" << s << " denom1=" << denom1 << " denom2=" << denom2 << " scale=" << scale << " un=" << (units - unitsBI) << nl;
   double rs = double(s/denom1) / denom2;
   return String().format("%.2f %s", rs, *units);
}

inline String  durToStr( double d )
{
   const char* unit = nullptr;
   const char* unitSmall[] = { "ms", "us", "ns" };
   if( d < 1.0 )
   {
      size_t i = 0;
      size_t n = sizeof(unitSmall)/sizeof(unitSmall[0]) - 1;
      d *= 1000.0;
      for( i = 0; i < n; ++i, d *= 1000.0 )
      {
         if( d >= 1.0 )  break;
      }
      unit = unitSmall[i];
   }
   else
   {
      //CHECK( false );
      // TODO: Support minutes, hours, days, etc.
      unit = "s";
   }
   return String().format("%.2f ", d) + unit;
}

UNNAMESPACE_END

NAMESPACE_BEGIN

//------------------------------------------------------------------------------
//!
const char*  toStr( EventProfiler::EventType e )
{
   switch( e )
   {
      case EventProfiler::CORE_BEGIN:                         return "CORE_BEGIN";
      case EventProfiler::CORE_END:                           return "CORE_END";
      case EventProfiler::DISPLAY_BEGIN:                      return "DISPLAY_BEGIN";
      case EventProfiler::DISPLAY_END:                        return "DISPLAY_END";
      case EventProfiler::EXEC_BEGIN:                         return "EXEC_BEGIN";
      case EventProfiler::EXEC_END:                           return "EXEC_END";
      case EventProfiler::GUI_BEGIN:                          return "GUI_BEGIN";
      case EventProfiler::GUI_END:                            return "GUI_END";
      case EventProfiler::LOOP_BEGIN:                         return "LOOP_BEGIN";
      case EventProfiler::LOOP_END:                           return "LOOP_END";
      case EventProfiler::LOOPS_BEGIN:                        return "LOOPS_BEGIN";
      case EventProfiler::LOOPS_END:                          return "LOOPS_END";
      case EventProfiler::PROCESS_EVENTS_BEGIN:               return "PROCESS_EVENTS_BEGIN";
      case EventProfiler::PROCESS_EVENTS_END:                 return "PROCESS_EVENTS_END";
      case EventProfiler::WORLD_BEGIN:                        return "WORLD_BEGIN";
      case EventProfiler::WORLD_BEGIN_ANIMATORS:              return "WORLD_BEGIN_ANIMATORS";
      case EventProfiler::WORLD_BEGIN_BRAINS:                 return "WORLD_BEGIN_BRAINS";
      case EventProfiler::WORLD_BEGIN_COMMANDS_AFTER_BRAINS:  return "WORLD_BEGIN_COMMANDS_AFTER_BRAINS";
      case EventProfiler::WORLD_BEGIN_ACTIONS:                return "WORLD_BEGIN_ACTIONS";
      case EventProfiler::WORLD_BEGIN_COMMANDS_AFTER_ACTIONS: return "WORLD_BEGIN_COMMANDS_AFTER_ACTIONS";
      case EventProfiler::WORLD_BEGIN_PHYSICS:                return "WORLD_BEGIN_PHYSICS";
      case EventProfiler::WORLD_BEGIN_ACTIONS_AFTER_PHYSICS:  return "WORLD_BEGIN_ACTIONS_AFTER_PHYSICS";
      case EventProfiler::WORLD_BEGIN_COMMANDS_AFTER_PHYSICS: return "WORLD_BEGIN_COMMANDS_AFTER_PHYSICS";
      case EventProfiler::WORLD_END:                          return "WORLD_END";
      default:                                                return "<UNKNOWN>";
   }
}

//------------------------------------------------------------------------------
//!
String  toStr( const EventProfiler::Event& e )
{
   String tmp;
   tmp.format( "%g ", e._timestamp );
   tmp += toStr( e.event() );
   if( e._count != uint32_t(-1) )
   {
      tmp += ' ';
      tmp += '(';
      tmp += e._count;
      tmp += ')';
   }
   return tmp;
}

//------------------------------------------------------------------------------
//!
void
EventProfiler::print( TextStream& os ) const
{
   size_t s = _events.size() * sizeof(_events[0]);
   os << "EventProfile: " << fmtReadable(s) << nl;
   size_t first = _events.size();
   if( first > 1000 )  first -= 1000;
   else                first  = 0;
   ConstIterator  last[NUM_EVENT_TYPES];
   for( uint i = 0; i < NUM_EVENT_TYPES; ++i )
   {
      last[i] = _events.end();
   }
   for( auto cur = _events.begin() + first; cur != _events.end(); ++cur )
   {
      const Event& ev = *cur;
      switch( ev._event )
      {
         case CORE_BEGIN:
            break;
         case CORE_END:
            break;
         case DISPLAY_BEGIN:
            break;
         case DISPLAY_END:
            break;
         case EXEC_BEGIN:
            break;
         case EXEC_END:
            break;
         case GUI_BEGIN:
            break;
         case GUI_END:
            break;
         case LOOP_BEGIN:
            break;
         case LOOP_END:
            break;
         case LOOPS_BEGIN:
            break;
         case LOOPS_END:
            break;
         case PROCESS_EVENTS_BEGIN:
            break;
         case PROCESS_EVENTS_END:
            break;
         case RENDER_BEGIN:
            break;
         case RENDER_END:
            StdErr << "Render took " << dur( last[RENDER_BEGIN], cur ) << nl;
            break;
         case WORLD_BEGIN:
            break;
         case WORLD_BEGIN_ANIMATORS:
            break;
         case WORLD_BEGIN_BRAINS:
            StdErr << "  + Anims   : " << dur( last[WORLD_BEGIN_ANIMATORS], cur ) << nl;
            break;
         case WORLD_BEGIN_COMMANDS_AFTER_BRAINS:
            StdErr << "  + Brains  : " << dur( last[WORLD_BEGIN_BRAINS], cur ) << nl;
            break;
         case WORLD_BEGIN_ACTIONS:
            StdErr << "  + Cmds(Br): " << dur( last[WORLD_BEGIN_COMMANDS_AFTER_BRAINS], cur ) << nl;
            break;
         case WORLD_BEGIN_COMMANDS_AFTER_ACTIONS:
            StdErr << "  + Actions : " << dur( last[WORLD_BEGIN_ACTIONS], cur ) << nl;
            break;
         case WORLD_BEGIN_PHYSICS:
            StdErr << "  + Cmds(Ac): " << dur( last[WORLD_BEGIN_COMMANDS_AFTER_ACTIONS], cur ) << nl;
            break;
         case WORLD_BEGIN_ACTIONS_AFTER_PHYSICS:
            StdErr << "  + Physics : " << dur( last[WORLD_BEGIN_PHYSICS], cur ) << nl;
            break;
         case WORLD_BEGIN_COMMANDS_AFTER_PHYSICS:
            StdErr << "  + Actions+: " << dur( last[WORLD_BEGIN_ACTIONS_AFTER_PHYSICS], cur ) << nl;
            break;
         case WORLD_END:
            StdErr << "  + Cmds(Ph): " << dur( last[WORLD_BEGIN_COMMANDS_AFTER_PHYSICS], cur ) << nl;
            StdErr << "  World: " << dur( last[WORLD_BEGIN], cur ) << nl;
            break;
      }
      last[ev._event] = cur;
   }

   auto iter = findLast( LOOP_BEGIN );
   for( uint i = 0; i < 10 && iter != begin(); ++i )
   {
      iter = findLast( LOOP_BEGIN, iter );
   }

   os << "Last 10 frames take " << end()-iter << " events." << nl;
}

//------------------------------------------------------------------------------
//!
BinaryStream&
EventProfiler::operator<<( BinaryStream& os ) const
{
   os << uint8_t(1); // Version.
   os << uint32_t(_events.size());
   for( auto cur = _events.begin(); cur != _events.end(); ++cur )
   {
      os << (*cur);
   }
   return os;
}

//------------------------------------------------------------------------------
//!
BinaryStream&
EventProfiler::operator>>( BinaryStream& is )
{
   uint8_t  version;
   is >> version;
   if( !is.ok() )  return is;
   CHECK( version == 1 );
   uint32_t nEntries;
   is >> nEntries;
   if( !is.ok() )  return is;
   _events.resize( nEntries );
   for( uint32_t i = 0; i < nEntries; ++i )
   {
      is >> _events[i];
   }
   return is;
}

//------------------------------------------------------------------------------
//!
bool
EventProfiler::save( const char* filename ) const
{
   {
      BinaryStream os( new FileDevice( filename, IODevice::MODE_WRITE|IODevice::MODE_STRICT ) );
      if( !os.ok() )  return false;

      operator<<( os );

      if( !os.ok() )  return false;
   }

   return true;
}

//------------------------------------------------------------------------------
//!
bool
EventProfiler::load( const char* filename )
{
   {
      BinaryStream is( new FileDevice( filename, IODevice::MODE_READ|IODevice::MODE_STRICT ) );
      if( !is.ok() )  return false;

      operator>>( is );

      if( !is.ok() )  return false;
   }

   return true;
}

//------------------------------------------------------------------------------
//!
String
EventProfiler::dur( ConstIterator from, ConstIterator to ) const
{
   if( from != end() )
   {
      return durToStr( (*to)._timestamp - (*from)._timestamp );
   }
   else
   {
      return durToStr( (*to)._timestamp );
   }
}

NAMESPACE_END
