/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef BASE_SOCKET_H
#define BASE_SOCKET_H

#include <Base/StdDefs.h>

#include <Base/ADT/String.h>
#include <Base/Util/RCObject.h>
#include <Base/Util/RCP.h>

NAMESPACE_BEGIN

// Forward declarations.
class ConnectedSocketDevice;
class ConnectionlessSocketDevice;

// Winsock requires initialization (doesn't need to be called).
BASE_DLL_API void  initNetLayer();
BASE_DLL_API void  termNetLayer();

/*==============================================================================
  CLASS Socket
==============================================================================*/
class Socket:
   public RCObject
{
public:

   /*----- types -----*/

   enum Family
   {
      FAMILY_UNKNOWN,
      FAMILY_IPC,
      FAMILY_IPv4,
      FAMILY_IPv6
   };

   enum Protocol
   {
      PROTOCOL_DEFAULT,
      PROTOCOL_UDP,
      PROTOCOL_TCP
      //PROTOCOL_SCTP,
      //PROTOCOL_DCCP
   };

   enum State
   {
      STATE_UNCONNECTED,
      STATE_RESOLVING_HOST,
      STATE_CONNECTING,
      STATE_CONNECTED,
      STATE_BOUND,
      STATE_CLOSING,
      STATE_LISTENING
   };

   enum Type
   {
      TYPE_UNKNOWN,
      TYPE_RAW,
      TYPE_DATAGRAM,
      TYPE_STREAM,
      TYPE_SEQPACKET
   };

   /*==============================================================================
     CLASS Endpoint
   ==============================================================================*/
   class Endpoint
   {
   public:

      /*----- types -----*/
      class Impl;

      /*----- methods -----*/

      BASE_DLL_API Endpoint();
      BASE_DLL_API Endpoint( uint16_t port );
      BASE_DLL_API Endpoint( const uint32_t ipv4addr, uint16_t port );
      BASE_DLL_API Endpoint( uint8_t a, uint8_t b, uint8_t c, uint8_t d, uint16_t port );
      BASE_DLL_API Endpoint( const uint8_t* ipv6addr, uint16_t port );
      BASE_DLL_API Endpoint( const String& host, uint16_t port, Family family = FAMILY_IPv4 );
      BASE_DLL_API Endpoint( const Endpoint& ep );

      BASE_DLL_API ~Endpoint();

      BASE_DLL_API Endpoint&  operator=( const Endpoint& ep );

      BASE_DLL_API Endpoint&  reset();

      BASE_DLL_API Endpoint&  set( uint16_t port );
      BASE_DLL_API Endpoint&  set( const uint32_t ipv4addr, uint16_t port );
      BASE_DLL_API Endpoint&  set( uint8_t a, uint8_t b, uint8_t c, uint8_t d, uint16_t port );
      BASE_DLL_API Endpoint&  set( const uint8_t* ipv6addr, uint16_t port );
      BASE_DLL_API Endpoint&  set( const String& host, uint16_t port, Family family = FAMILY_IPv4 );

      BASE_DLL_API Endpoint&  get( uint16_t& port );
      BASE_DLL_API Endpoint&  get( uint32_t& ipv4addr, uint16_t& port );
      BASE_DLL_API Endpoint&  get( uint8_t& a, uint8_t& b, uint8_t& c, uint8_t& d, uint16_t& port );
      BASE_DLL_API Endpoint&  get( uint8_t* ipv6addr, uint16_t& port );
      BASE_DLL_API Endpoint&  get( String& host, uint16_t& port, Family& family );

      BASE_DLL_API Family    family() const;
      BASE_DLL_API String    hostname() const;
      BASE_DLL_API uint16_t  port() const;
      BASE_DLL_API String    toString() const;

      inline       Impl*  impl()       { return _impl; }
      inline const Impl*  impl() const { return _impl; }

   protected:

      /*----- data members -----*/
      Impl*  _impl;

   private:
   }; //class Endpoint

   // Forward declaration.
   class Impl;  // Needs to be declared after the Endpoint's, otherwise the compiler assumes they are the same.


   /*----- static methods -----*/

   static BASE_DLL_API Protocol  getDefaultProtocol( Family family, Type type );

   /*----- methods -----*/

   BASE_DLL_API Socket( Type type, Protocol protocol = PROTOCOL_DEFAULT );
   BASE_DLL_API Socket( Family family, Type type, Protocol protocol );
   BASE_DLL_API ~Socket();

   inline Family    family()   const { return _family;   }
   inline Type      type()     const { return _type;     }
   inline Protocol  protocol() const { return _protocol; }
   inline State     state()    const { return _state;    }

   inline const Endpoint&  localEndpoint() const { return _localEndpoint; }

   inline       Impl*  impl()       { return _impl; }
   inline const Impl*  impl() const { return _impl; }

   BASE_DLL_API bool  listen( uint16_t port );

   BASE_DLL_API RCP<ConnectedSocketDevice>  connect( const Endpoint& ep );

   BASE_DLL_API RCP<ConnectedSocketDevice>  accept();

   BASE_DLL_API RCP<ConnectionlessSocketDevice>  device();

   BASE_DLL_API RCP<ConnectionlessSocketDevice>  device( uint16_t port );

   BASE_DLL_API void  blocking( bool v );
   BASE_DLL_API bool  blocking();

protected:

   /*----- data members -----*/
   Family    _family;
   Type      _type;
   Protocol  _protocol;
   State     _state;
   Endpoint  _localEndpoint;
   Impl*     _impl;

   /*----- methods -----*/
   Socket( const Socket* socket );
   inline void  state( State s ) { _state = s; }

private:
}; //class Socket

//------------------------------------------------------------------------------
//!
inline const char*  toStr( Socket::Family family )
{
   switch( family )
   {
      case Socket::FAMILY_UNKNOWN:  return "<unknown>";
      case Socket::FAMILY_IPC    :  return "IPC";
      case Socket::FAMILY_IPv4   :  return "IPv4";
      case Socket::FAMILY_IPv6   :  return "IPv6";
      default                    :  return "<invalid>";
   }
}

//------------------------------------------------------------------------------
//!
inline const char*  toStr( Socket::Type type )
{
   switch( type )
   {
      case Socket::TYPE_UNKNOWN  :  return "<unknown>";
      case Socket::TYPE_RAW      :  return "Raw";
      case Socket::TYPE_DATAGRAM :  return "Datagram";
      case Socket::TYPE_STREAM   :  return "Stream";
      case Socket::TYPE_SEQPACKET:  return "SeqPacket";
      default                    :  return "<invalid>";
   }
}

//------------------------------------------------------------------------------
//!
inline const char*  toStr( Socket::Protocol protocol )
{
   switch( protocol )
   {
      case Socket::PROTOCOL_DEFAULT:  return "<default>";
      case Socket::PROTOCOL_UDP    :  return "UDP";
      case Socket::PROTOCOL_TCP    :  return "TCP";
      //case Socket::PROTOCOL_SCTP :  return "SCTP";
      //case Socket::PROTOCOL_DCCP :  return "DCCP";
      default                      :  return "<invalid>";
   }
}

//------------------------------------------------------------------------------
//!
inline  String  ipToStr( uint32_t ip )
{
   return String().format( "%d.%d.%d.%d", (ip >> 24) & 0xFF, (ip >> 16) & 0xFF, (ip >> 8) & 0xFF, (ip >> 0) & 0xFF );
}

//------------------------------------------------------------------------------
//!
BASE_DLL_API bool  resolveHost( const char* hostname, Socket::Endpoint& dst );

//------------------------------------------------------------------------------
//!
BASE_DLL_API bool  resolveHost( const char* hostname, Vector<Socket::Endpoint>& dst );

NAMESPACE_END

#endif //BASE_SOCKET_H
