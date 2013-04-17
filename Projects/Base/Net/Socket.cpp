/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Base/Net/_SocketImpl.h>

#include <Base/Net/Socket.h>


/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN

const int DefaultBacklogSize = 10;

UNNAMESPACE_END


NAMESPACE_BEGIN

//------------------------------------------------------------------------------
//!
Socket::Protocol
Socket::getDefaultProtocol( Family /*family*/, Type type )
{
   switch( type )
   {
      case TYPE_UNKNOWN  :  return PROTOCOL_UDP;
      case TYPE_RAW      :  return PROTOCOL_UDP;
      case TYPE_DATAGRAM :  return PROTOCOL_UDP;
      case TYPE_STREAM   :  return PROTOCOL_TCP;
      case TYPE_SEQPACKET:  return PROTOCOL_UDP;
      default            :  return PROTOCOL_UDP;
   }
}

//------------------------------------------------------------------------------
//!
Socket::Socket( const Socket* socket ):
   _family( socket->family() ),
   _type( socket->type() ),
   _protocol( socket->protocol() ),
   _state( STATE_UNCONNECTED )
{
   _impl = new Socket::Impl();
   // ... but do not create the socket.
}

//------------------------------------------------------------------------------
//!
Socket::Socket( Type t, Protocol p ):
   _family( FAMILY_UNKNOWN ),
   _type( t ),
   _protocol( p ),
   _state( STATE_UNCONNECTED )
{
   _impl = new Socket::Impl();

#if BASE_SOCKET_USE_BSD || BASE_SOCKET_USE_WINSOCK
   _impl->_sock = socket( toAPI(family()), toAPI(type()), toAPI(protocol()) );
   if( _impl->_sock == sInvalidSocket )
   {
      printAPIError( "ERROR - Socket::Socket(Type, Protocol) - socket() failed" );
      CHECK( false );
   }
#else
#endif
}

//------------------------------------------------------------------------------
//!
Socket::Socket( Family f, Type t, Protocol p ):
   _family( f ),
   _type( t ),
   _protocol( p ),
   _state( STATE_UNCONNECTED )
{
   _impl = new Socket::Impl();

#if BASE_SOCKET_USE_BSD || BASE_SOCKET_USE_WINSOCK
   _impl->_sock = socket( toAPI(family()), toAPI(type()), toAPI(protocol()) );
   if( _impl->_sock == sInvalidSocket )
   {
      printAPIError( "ERROR - Socket::Socket(Family, Type, Protocol) - socket() failed" );
      CHECK( false );
   }
#else
#endif
}

//------------------------------------------------------------------------------
//!
Socket::~Socket()
{
#if BASE_SOCKET_USE_BSD
   close( _impl->_sock );
#elif BASE_SOCKET_USE_WINSOCK
   closesocket( _impl->_sock );
#endif
   delete _impl;
}

//------------------------------------------------------------------------------
//!
bool
Socket::listen( uint16_t port )
{
#if BASE_SOCKET_USE_BSD || BASE_SOCKET_USE_WINSOCK
   if( !bind( *this, port, this->_localEndpoint ) )
   {
      return false;
   }

   // Update state.
   state( STATE_BOUND );

   if( type() == TYPE_DATAGRAM || protocol() == PROTOCOL_UDP )
   {
      // Datagram sockets don't need to listen; a simple bind is enough.
      return true;
   }

   // Start listening.
   int err = ::listen( _impl->_sock, DefaultBacklogSize );
   if( err != 0 )
   {
      printAPIError( "ERROR - Socket::listen() - listen failed" );
      return false;
   }

   state( STATE_LISTENING );

   return true;
#else
   return false;
#endif
}

//------------------------------------------------------------------------------
//!
RCP<ConnectedSocketDevice>
Socket::connect( const Endpoint& ep )
{
#if BASE_SOCKET_USE_BSD || BASE_SOCKET_USE_WINSOCK
   state( STATE_CONNECTING );

   addrinfo* candidates = getAddrInfo( *this, ep.hostname().cstr(), ep.port() );
   if( candidates == NULL )
   {
      return NULL;
   }

   Family   family   = fromAPI_family  ( candidates->ai_family   );
   Type     type     = fromAPI_type    ( candidates->ai_socktype );
   Protocol protocol = fromAPI_protocol( candidates->ai_protocol );

   RCP<Socket>  socket = new Socket( family, type, protocol );

   int err = ::connect( socket->_impl->_sock, candidates->ai_addr, SocketSSizeT(candidates->ai_addrlen) );
   if( err != 0 )
   {
      printAPIError( "ERROR - Socket::connect() - connect failed" );
      freeaddrinfo( candidates );
      return NULL;
   }

   RCP<ConnectedSocketDevice>  device = new ConnectedSocketDevice( socket.ptr() );
   device->_remoteEndpoint = ep;

   freeaddrinfo( candidates );

   state( STATE_CONNECTED );

   return device;
#else
   return NULL;
#endif
}

//------------------------------------------------------------------------------
//!
RCP<ConnectedSocketDevice>
Socket::accept()
{
#if BASE_SOCKET_USE_BSD || BASE_SOCKET_USE_WINSOCK
   sockaddr_storage  stor;
   socklen_t  slen = getSize(_localEndpoint.impl()->_stor);
   SocketDescriptor sock = ::accept( _impl->_sock, (sockaddr*)&stor, &slen );
   if( sock == sInvalidSocket )
   {
      printAPIError( "ERROR - Socket::accept() - accept failed" );
      return NULL;
   }

   //fprintf(stderr,"... accept() suceeded\n");

   RCP<Socket>  socket = new Socket( this );
   //fprintf(stderr,"... Socket created\n");

   socket->_impl->_sock = sock;
   //fprintf(stderr,"... _sock assigned %p\n", socket->_impl);
   //socket->_localEndpoint = ; // Not setting the source endpoint for now.

   RCP<ConnectedSocketDevice>  device = new ConnectedSocketDevice( socket.ptr() );
   //fprintf(stderr,"... device created\n");
   device->_remoteEndpoint.impl()->_stor = stor;
   //fprintf(stderr,"... _stor assigned\n");

   socket->state( STATE_CONNECTED );

   return device;
#else
   return NULL;
#endif
}

//------------------------------------------------------------------------------
//!
RCP<ConnectionlessSocketDevice>
Socket::device()
{
#if BASE_SOCKET_USE_BSD || BASE_SOCKET_USE_WINSOCK
   return new ConnectionlessSocketDevice( this );
#else
   return NULL;
#endif
}

//------------------------------------------------------------------------------
//!
RCP<ConnectionlessSocketDevice>
Socket::device( uint16_t port )
{
#if BASE_SOCKET_USE_BSD || BASE_SOCKET_USE_WINSOCK
   if( !bind( *this, port, this->_localEndpoint ) )
   {
      return NULL;
   }
   state( STATE_BOUND );
   return new ConnectionlessSocketDevice( this );
#else
   return NULL;
#endif
}

//------------------------------------------------------------------------------
//!
void
Socket::blocking( bool v )
{
#if BASE_SOCKET_USE_BSD
   int f = fcntl( _impl->_sock, F_GETFL, 0 );
   if( v )
   {
      fcntl( _impl->_sock, F_SETFL, f | O_NONBLOCK );
   }
   else
   {
      fcntl( _impl->_sock, F_SETFL, f & ~O_NONBLOCK );
   }
#elif BASE_SOCKET_USE_WINSOCK
   unused(v);
#else
#endif
}

//------------------------------------------------------------------------------
//!
bool
Socket::blocking()
{
#if BASE_SOCKET_USE_BSD
   return (fcntl( _impl->_sock, F_GETFL, 0 ) & O_NONBLOCK) == 0;
#elif BASE_SOCKET_USE_WINSOCK
   return true;
#else
   return true;
#endif
}

//------------------------------------------------------------------------------
//!
bool  resolveHost( const char* hostname, Socket::Endpoint& ep )
{
#if BASE_SOCKET_USE_BSD || BASE_SOCKET_USE_WINSOCK
   // Prepare request.
   addrinfo  request;
   memset( &request, 0, sizeof(request) );
   request.ai_family = AF_UNSPEC;

   // Retrieve candidates.
   addrinfo* candidates;
   int err = getaddrinfo( hostname, NULL, &request, &candidates );
   if( err != 0 )
   {
      printAPIError( "ERROR - resolveHost() - getaddrinfo() failed" );
      return false;
   }

   addrinfo* cur = candidates;
   // Search for some criterion here (for now, take the last one).
   while( cur->ai_next != NULL )
   {
      cur = cur->ai_next;
   }
   memcpy( &(ep.impl()->_stor), cur->ai_addr, cur->ai_addrlen );

   freeaddrinfo( candidates );

   return true;
#else
   return true;
#endif
}

//------------------------------------------------------------------------------
//!
bool  resolveHost( const char* hostname, Vector<Socket::Endpoint>& dst )
{
#if BASE_SOCKET_USE_BSD || BASE_SOCKET_USE_WINSOCK
   // Prepare request.
   addrinfo  request;
   memset( &request, 0, sizeof(request) );
   request.ai_family = AF_UNSPEC;

   // Retrieve candidates.
   addrinfo* candidates;
   int err = getaddrinfo( hostname, NULL, &request, &candidates );
   if( err != 0 )
   {
      printAPIError( "ERROR - resolveHost() - getaddrinfo() failed" );
      return false;
   }

   for( addrinfo* cur = candidates; cur != NULL; cur = cur->ai_next )
   {
      dst.pushBack( Socket::Endpoint() );
      memcpy( &(dst.back().impl()->_stor), cur->ai_addr, cur->ai_addrlen );
   }

   freeaddrinfo( candidates );
   return true;
#else
   return false;
#endif
}

NAMESPACE_END
