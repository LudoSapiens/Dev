/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Base/Net/_SocketImpl.h>

#include <Base/Net/SocketDevice.h>

#include <Base/Dbg/Defs.h>

USING_NAMESPACE

/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN


UNNAMESPACE_END

/*==============================================================================
  CLASS SocketDevice
==============================================================================*/

//------------------------------------------------------------------------------
//!
SocketDevice::SocketDevice( Socket* socket ):
   IODevice( MODE_READ_WRITE ),
   _socket( socket )
{
}

//------------------------------------------------------------------------------
//!
SocketDevice::~SocketDevice()
{
}

/*==============================================================================
  CLASS ConnectedSocketDevice
==============================================================================*/

//------------------------------------------------------------------------------
//!
ConnectedSocketDevice::ConnectedSocketDevice( Socket* socket ):
   SocketDevice( socket )
{
}

//------------------------------------------------------------------------------
//!
ConnectedSocketDevice::~ConnectedSocketDevice()
{
}

//------------------------------------------------------------------------------
//!
size_t
ConnectedSocketDevice::doRead( char* data, size_t n )
{
#if BASE_SOCKET_USE_BSD || BASE_SOCKET_USE_WINSOCK
   int flags = 0;
   SocketSSizeT s = recv( _socket->impl()->_sock, data, SocketSSizeT(n), flags );
   if( s == sInvalidSSizeT )
   {
      printAPIError( "ERROR - ConnectedSocketDevice::doRead() - recv failed" );
      return INVALID_SIZE;
   }
   return s;
#else
   return 0;
#endif
}

//------------------------------------------------------------------------------
//!
size_t
ConnectedSocketDevice::doPeek( char* data, size_t n )
{
#if BASE_SOCKET_USE_BSD || BASE_SOCKET_USE_WINSOCK
   int flags = MSG_PEEK;
   SocketSSizeT s = recv( _socket->impl()->_sock, data, SocketSSizeT(n), flags );
   if( s == sInvalidSSizeT )
   {
      printAPIError( "ERROR - ConnectedSocketDevice::doPeek() - recv failed" );
      return INVALID_SIZE;
   }
   return s;
#else
   return 0;
#endif
}

//------------------------------------------------------------------------------
//!
size_t
ConnectedSocketDevice::doWrite( const char* data, size_t n )
{
#if BASE_SOCKET_USE_BSD || BASE_SOCKET_USE_WINSOCK
   Socket::Impl&  impl = *_socket->impl();
   int flags = 0;
   SocketSSizeT s = send( impl._sock, data, SocketSSizeT(n), flags );
   if( s == sInvalidSSizeT )
   {
      printAPIError( "ERROR - ConnectedSocketDevice::doWrite() - send failed" );
      return INVALID_SIZE;
   }
   return s;
#else
   return 0;
#endif
}


/*==============================================================================
  CLASS ConnectionlessSocketDevice
==============================================================================*/

//------------------------------------------------------------------------------
//!
ConnectionlessSocketDevice::ConnectionlessSocketDevice( Socket* socket ):
   SocketDevice( socket )
{
}

//------------------------------------------------------------------------------
//!
ConnectionlessSocketDevice::~ConnectionlessSocketDevice()
{
}

//------------------------------------------------------------------------------
//!
size_t
ConnectionlessSocketDevice::doRead( char* data, size_t n )
{
#if BASE_SOCKET_USE_BSD || BASE_SOCKET_USE_WINSOCK
   sockaddr_storage& from = _lastOrigin.impl()->_stor;
   socklen_t         flen = sizeof(from);
   int flags = 0;
   SocketSSizeT s = recvfrom( _socket->impl()->_sock, data, SocketSSizeT(n), flags, (sockaddr*)&from, &flen );
   if( s == sInvalidSSizeT )
   {
      printAPIError( "ERROR - ConnectionlessSocketDevice::doRead() - recvfrom failed" );
      return INVALID_SIZE;
   }
   return s;
#else
   return 0;
#endif
}

//------------------------------------------------------------------------------
//!
size_t
ConnectionlessSocketDevice::doPeek( char* data, size_t n )
{
#if BASE_SOCKET_USE_BSD || BASE_SOCKET_USE_WINSOCK
   sockaddr_storage& from = _lastOrigin.impl()->_stor;
   socklen_t         flen = sizeof(from);
   int flags = MSG_PEEK;
   SocketSSizeT s = recvfrom( _socket->impl()->_sock, data, SocketSSizeT(n), flags, (sockaddr*)&from, &flen );
   if( s == sInvalidSSizeT )
   {
      printAPIError( "ERROR - ConnectionlessSocketDevice::doPeek() - recvfrom failed" );
      return INVALID_SIZE;
   }
   return s;
#else
   return 0;
#endif
}

//------------------------------------------------------------------------------
//!
size_t
ConnectionlessSocketDevice::doWrite( const char* data, size_t n )
{
#if BASE_SOCKET_USE_BSD || BASE_SOCKET_USE_WINSOCK
   const socklen_t sizes[] = {
      (size_t)0           , // FAMILY_UNKNOWN
      sizeof(sockaddr    ), // FAMILY_IPC
      sizeof(sockaddr_in ), // FAMILY_IPv4
      sizeof(sockaddr_in6), // FAMILY_IPv6
   };
   Socket::Impl&            impl = *_socket->impl();
   const sockaddr_storage&  to   = destination().impl()->_stor;
   int flags = 0;
   SocketSSizeT s = sendto( impl._sock, data, SocketSSizeT(n), flags, (sockaddr*)&to, sizes[_socket->family()%4] );
   if( s == sInvalidSSizeT )
   {
      printAPIError( "ERROR - ConnectionlessSocketDevice::doWrite() - sendto failed" );
      fprintf( stderr, "sendto --> %d\n", (int)s );
      fprintf( stderr, "dst: %s\n", destination().toString().cstr() );
      return INVALID_SIZE;
   }
   return s;
#else
   return 0;
#endif
}
