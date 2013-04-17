/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Base/Dbg/DebugStream.h>

#include <Base/ADT/Map.h>
#include <Base/Dbg/Defs.h>
#include <Base/IO/FileDevice.h>
#include <Base/IO/Path.h>
#include <Base/IO/TextStream.h>
#include <Base/Util/RCObject.h>
#include <Base/Util/RCP.h>

USING_NAMESPACE

/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN

inline const char*  skipWS( const char* str )
{
   while( str[0] == ' ' )
   {
      ++str;
   }
   return str;
}

inline const char*  skipTo( const char* str, char c )
{
   while( str[0] != c )
   {
      if( str[0] == '\0' )  return NULL;
      ++str;
   }
   return str;
}

/*
inline const char*  skipNWS( const char* str )
{
   while( str[0] != '\0' && str[0] != ' ' )
   {
      ++str;
   }
   return str;
}
 */

/*==============================================================================
  CLASS DebugStreamManager
==============================================================================*/
class DebugStreamManager:
   public RCObject
{
public:


   class DeviceData
   {
   public:
      DeviceData( IODevice* device = NULL, uint depth = 0 ): _device( device ), _depth( depth ) { }

      inline IODevice*  device() { return _device.ptr(); }

      inline uint*  depthPtr() { return &_depth; }

      inline void  set( IODevice* device )
      {
         _device = device;
         _depth  = 0;
      }
   protected:
      RCP<IODevice>  _device;
      uint           _depth;
   }; // class DeviceData

   /*----- types -----*/
   typedef Map< String, DebugStream::State >  GroupStateContainer;
   typedef Map< String, Path >                GroupPathContainer;
   typedef Map< Path, DeviceData >            PathDeviceContainer;

   /*----- methods -----*/

   DebugStreamManager();
   ~DebugStreamManager();

   void  readConfigFile();
   void  readConfigFile( const char* filename );
   void  readEnvVars();

   DebugStream::State*  get( const String& group );
   void  set( const String& group, IODevice* device );

   const Path&  groupToPath( const String& group );

   DeviceData&  pathToDeviceData( const Path& path );

   inline DeviceData&  groupToDeviceData( const String& group ) { return pathToDeviceData(groupToPath(group)); }

   void  rePath( const Path& name, const Path& path );

protected:

   /*----- data members -----*/

   GroupStateContainer   _states;   //!< The states for every group.
   GroupPathContainer    _paths;    //!< The paths to use for every uncached group.
   PathDeviceContainer   _devices;  //!< The devices for every file path.

   /*static*/ const Path  _kDefault;
   /*static*/ const Path  _kStdErr;
   /*static*/ const Path  _kStdOut;

   bool  _all;  //!< Set to true if the trace group is set to "all".

private:
}; //class DebugStreamManager

//const Path  DebugStreamManager::_kDefault = Path( "@default" );
//const Path  DebugStreamManager::_kStdErr  = Path( "@stderr"  );
//const Path  DebugStreamManager::_kStdOut  = Path( "@stdout"  );

//------------------------------------------------------------------------------
//!
DebugStreamManager::DebugStreamManager():
   _kDefault( Path("@default") ),
   _kStdErr ( Path("@stderr" ) ),
   _kStdOut ( Path("@stdout" ) ),
   _all( false )
{
   _devices[_kDefault] = new FileDevice( stderr, IODevice::MODE_WRITE | IODevice::MODE_STRICT );
   _devices[_kStdErr ] = new FileDevice( stderr, IODevice::MODE_WRITE | IODevice::MODE_STRICT );
   _devices[_kStdOut ] = new FileDevice( stdout, IODevice::MODE_WRITE | IODevice::MODE_STRICT );
   readConfigFile();
   readEnvVars();
}

//------------------------------------------------------------------------------
//!
DebugStreamManager::~DebugStreamManager()
{
}

//------------------------------------------------------------------------------
//!
void
DebugStreamManager::readConfigFile()
{
   const char* env;
   env = getenv( "DBG_MSG_CONFIG" );
   if( env == NULL )
   {
      env = "debug.conf";
   }
   readConfigFile( env );
}

//------------------------------------------------------------------------------
//!
void
DebugStreamManager::readConfigFile( const char* filename )
{
   FILE* fd = fopen( filename, "r" );
   if( fd )
   {
      char tmp[1024];
      String group;
      String path;

      const char* s;
      const char* e;
      while( fgets(tmp, 1024, fd) != NULL )
      {
         s = skipWS( tmp );
         if( s == NULL )  break;
         if( s[0] == '#' || (s[0] == '/' && s[1] == '/') || s[0] == '\0' || s[0] == '\n' )
         {
            // Skip comment or empty lines.
            continue;
         }

         e = skipTo( s, ':' );
         if( e == NULL || e == s )
         {
            fprintf( stderr, "INVALID CONFIG LINE: %s", tmp );
            continue;
         }

         group = String( s, e-s ).eatTrailingWhites();
         s = skipWS( e + 1 );  // Skip the ':'.
         path = String( s ).eatTrailingWhites();
         if( group == "all" )
         {
            _all = true;
            rePath( _kDefault, path );
         }
         else
         if( group == "@stderr" )
         {
            if( freopen( path.cstr(), "w", stderr ) == NULL )
            {
               fprintf( stderr, "ERROR - Could not repoen stderr to '%s'\n", path.cstr() );
            }
         }
         else
         if( group == "@stdout" )
         {
            if( freopen( path.cstr(), "w", stdout ) == NULL )
            {
               fprintf( stderr, "ERROR - Could not repoen stdout to '%s'\n", path.cstr() );
            }
         }
         else
         {
            _paths[group] = Path( path );
            DebugStream::State* state = get( group );
            state->activate();
         }
      }

      fclose( fd );
   }
}

//------------------------------------------------------------------------------
//!
void
DebugStreamManager::readEnvVars()
{
   const char* env;

   env = getenv( "DBG_MSG_FILE" );
   if( env )
   {
      RCP<FileDevice> fd = new FileDevice( Path(env), IODevice::MODE_WRITE | IODevice::MODE_STRICT );
      _devices[_kDefault].set( fd.ptr() );
   }

   env = getenv( "DBG_MSG_GROUPS" );
   if( env )
   {
      String trace_group = env;
      if( !trace_group.empty() )
      {
         const String seps(",;: \n\r\t\0");
         String  group;
         String::SizeType s = trace_group.findFirstNotOf(seps);
         String::SizeType e = trace_group.findFirstOf(seps, s+1);
         while( s != String::npos )
         {
            group = String( trace_group, s, e-s ).eatWhites();

            if( group == "all" )
            {
               _all = true;
            }
            else
            {
               DebugStream::State* state = get( group );
               state->activate();
            }

            s = trace_group.findFirstNotOf(seps, e);
            e = trace_group.findFirstOf(seps, s+1);
         }
      }
   }
}

//------------------------------------------------------------------------------
//!
DebugStream::State*
DebugStreamManager::get( const String& group )
{
   GroupStateContainer::Iterator it = _states.find( group );
   if( it != _states.end() )
   {
      // Return cached state.
      DebugStream::State& state = (*it).second;
      return &state;
   }

   // Need to create a new state.
   DeviceData& deviceData = groupToDeviceData( group );
   DebugStream::State& state = _states[group];
   state.set( group, deviceData.device(), deviceData.depthPtr(), _all );
   return &state;
}

//------------------------------------------------------------------------------
//!
void
DebugStreamManager::set( const String& group, IODevice* device )
{
   DeviceData& deviceData = groupToDeviceData( group );
   deviceData.set( device );
}

//------------------------------------------------------------------------------
//!
const Path&
DebugStreamManager::groupToPath( const String& group )
{
   GroupPathContainer::Iterator it = _paths.find( group );
   if( it != _paths.end() )
   {
      const Path& path = (*it).second;
      return path;
   }

   return _kDefault;
}

//------------------------------------------------------------------------------
//!
DebugStreamManager::DeviceData&
DebugStreamManager::pathToDeviceData( const Path& path )
{
   PathDeviceContainer::Iterator it = _devices.find( path );
   if( it != _devices.end() )
   {
      return (*it).second;
   }

   // Need to create a new device data.
   DeviceData& deviceData = _devices[path];
   RCP<IODevice> device = new FileDevice( path, IODevice::MODE_WRITE | IODevice::MODE_STRICT );
   deviceData.set( device.ptr() );
   return deviceData;
}

//------------------------------------------------------------------------------
//!
void
DebugStreamManager::rePath( const Path& name, const Path& path )
{
   if( path.empty() )  return;

   if( path == _kDefault )
   {
      _devices[name] = _devices[_kDefault];
   }
   else
   if( path == _kStdErr )
   {
      _devices[name] = _devices[_kStdErr];
   }
   else
   if( path == _kStdOut )
   {
      _devices[name] = _devices[_kStdOut];
   }
   else
   {
      _devices[name].set( new FileDevice( path, IODevice::MODE_WRITE | IODevice::MODE_STRICT ) );
   }
}

//------------------------------------------------------------------------------
//!
RCP<DebugStreamManager>  _manager;
DebugStreamManager*      _managerPtr = NULL;

//------------------------------------------------------------------------------
//!
inline DebugStreamManager&  manager()
{
   if( _manager.isNull() )
   {
      // Sometimes, static ctors will override a former assignment of _manager,
      // but the basic C pointer doesn't seem to be affected by this.
      // So we basically allocate the manager only once, and store it in _managerPtr,
      // but we still hold on to an RCP in order to automatically deallocate memory.
      //fprintf( stderr, "manager() %p %p\n", _manager.ptr(), _managerPtr );
      if( _managerPtr == NULL )  _managerPtr = new DebugStreamManager();
      _manager = _managerPtr;
      //fprintf( stderr, " --> %p %p\n", _manager.ptr(), _managerPtr );
   }

   return *_manager;
}


//--------------------
// DebugStream::State
//--------------------

// Prepare 1024-deep strings (start, continue, and end).
const char* _null = "";
String  _1023 = String("| ") * 1023;
String  _s = _1023 + String( "+" );
String  _c = _1023 + String( "|  " );
String  _e = _1023 + String( " --" );

//------------------------------------------------------------------------------
//!
const char*  startString( uint depth )
{
   return (depth == 0) ? _null : _s.cstr() + 2*(1024-(depth&0x03FF));
}

//------------------------------------------------------------------------------
//!
const char*  contString( uint depth )
{
   return (depth == 0) ? _null : _c.cstr() + 2*(1024-(depth&0x03FF));
}

//------------------------------------------------------------------------------
//!
const char*  endString( uint depth )
{
   return (depth == 0) ? _null : _e.cstr() + 2*(1024-(depth&0x03FF));
}


UNNAMESPACE_END

//------------------------------------------------------------------------------
//!
void
DebugStream::readConfigFile( const char* filename )
{
   manager().readConfigFile( filename );
}

//------------------------------------------------------------------------------
//!
DebugStream::DebugStream( const String& group ):
   _state( manager().get(group) )
{
   CHECK( _state != NULL );
   manager().addReference();
}

//------------------------------------------------------------------------------
//!
DebugStream::~DebugStream()
{
   manager().removeReference();
}

//------------------------------------------------------------------------------
//! Redefines the IODevice to use with this stream.
void
DebugStream::device( IODevice* device )
{
   // Redirect our own device directly.
   _state->stream().device( device );
   // Update manager's cache.
   manager().set( group(), device );
}

//------------------------------------------------------------------------------
//!
const char*
DebugStream::State::pre()
{
   switch( mode() )
   {
      case 0:
         mode( 1 );
         return startString( depth() );
      case 1:
         return contString( depth() );
      case 2:
      default:
         //mode( 0 );
         return endString( depth() );
   }
}

//------------------------------------------------------------------------------
//!
DebugStream::State&
DebugStream::State::operator++()
{
   mode( 0 );
   ++(*_depth);
   return *this;
}

//------------------------------------------------------------------------------
//!
DebugStream::State&
DebugStream::State::operator--()
{
   mode( 2 );
   if( active() )
   {
      // Automatically terminate block (but only if active).
      _stream << pre() << nl;
   }
   mode( 1 ); // Continue previous depth.
   if( depth() != 0 )  --(*_depth);
   return *this;
}
