/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef BASE__SOCKET_IMPL_H
#define BASE__SOCKET_IMPL_H

// NOTES
// For some annoying reason, Cygwin compiles yields the following warning:
//   warning "fd_set and associated macros have been defined in sys/types.
//   This may cause runtime problems with W32 sockets"
// if winsock2.h isn't the [mostly] first header included.
//
// To circumvent that, we have deferred the includes until after the socket
// type is determined.
// This also requires any file including _SocketImpl.h to include it prior
// to any other file.
// ... annoying, but it does keep the implementation private...


#if !defined(BASE_SOCKET_USE_BSD)
#  if defined(_POSIX_VERSION) || defined(__unix__) || defined(__APPLE__)
#    define BASE_SOCKET_USE_BSD 1
#  else
#    define BASE_SOCKET_USE_BSD 0
#  endif
#endif

#if !defined(BASE_SOCKET_USE_WINSOCK)
#  if defined(_WIN32) || defined(__CYGWIN__)
#    define BASE_SOCKET_USE_WINSOCK 1
#  else
#    define BASE_SOCKET_USE_WINSOCK 0
#  endif
#endif


// Guarantee that only one API is used.
#if (BASE_SOCKET_USE_BSD+BASE_SOCKET_USE_WINSOCK) > 1
// Prefer Winsock over BSD
#undef BASE_SOCKET_USE_BSD
#define BASE_SOCKET_USE_BSD 0
// Prefer BSD over Winsock
//#undef BASE_SOCKET_USE_WINSOCK
//#define BASE_SOCKET_USE_WINSOCK 0
#endif


// ========================
// === BERKELEY SOCKETS ===
// ============================================================================
// ============================================================================
#if BASE_SOCKET_USE_BSD

#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <Base/StdDefs.h>

#include <Base/Dbg/Defs.h>
#include <Base/Net/Socket.h>
#include <Base/Net/SocketDevice.h>

typedef int      SocketDescriptor;
typedef ssize_t  SocketSSizeT;
const SocketDescriptor  sInvalidSocket = -1;
const SocketSSizeT      sInvalidSSizeT = -1;

NAMESPACE_BEGIN

/*==============================================================================
  Special routines used by Modules.cpp.
==============================================================================*/

//------------------------------------------------------------------------------
//!
BASE_DLL_API void  initNetLayer();

//------------------------------------------------------------------------------
//!
BASE_DLL_API void  termNetLayer();

NAMESPACE_END

#endif //BASE_SOCKET_USE_BSD


// ===============
// === WINSOCK ===
// ============================================================================
// ============================================================================
#if BASE_SOCKET_USE_WINSOCK

// Ref: http://msdn.microsoft.com/en-us/library/ms740673.aspx
#include <winsock2.h>
#include <ws2tcpip.h>

#include <Base/StdDefs.h>

#include <Base/Dbg/Defs.h>
#include <Base/Net/Socket.h>
#include <Base/Net/SocketDevice.h>

typedef SOCKET  SocketDescriptor;
typedef int     SocketSSizeT;
const SocketDescriptor  sInvalidSocket = INVALID_SOCKET;
const SocketSSizeT      sInvalidSSizeT = SOCKET_ERROR;

NAMESPACE_BEGIN

/*==============================================================================
  Special routines used by Modules.cpp.
==============================================================================*/

//------------------------------------------------------------------------------
//!
BASE_DLL_API void  initNetLayer();

//------------------------------------------------------------------------------
//!
BASE_DLL_API void  termNetLayer();

NAMESPACE_END

#endif // BASE_SOCKET_USE_WINSOCK


// ==============================
// === SHARED BSD AND WINSOCK ===
// ============================================================================
// ============================================================================

#if BASE_SOCKET_USE_BSD || BASE_SOCKET_USE_WINSOCK

NAMESPACE_BEGIN


/*==============================================================================
  Utility routines.
==============================================================================*/

//------------------------------------------------------------------------------
//!
BASE_DLL_API void  printAPIError( const char* msg );

//------------------------------------------------------------------------------
//!
inline int  toAPI( Socket::Family family )
{
   switch( family )
   {
      case Socket::FAMILY_UNKNOWN:  return AF_UNSPEC;
#if BASE_SOCKET_USE_BSD
      case Socket::FAMILY_IPC    :  return AF_UNIX;
#endif
      case Socket::FAMILY_IPv4   :  return AF_INET;
      case Socket::FAMILY_IPv6   :  return AF_INET6;
      default                    :  return -1;
   }
}

//------------------------------------------------------------------------------
//!
inline Socket::Family  fromAPI_family( int family )
{
   switch( family )
   {
      case AF_UNSPEC:  return Socket::FAMILY_UNKNOWN;
#if BASE_SOCKET_USE_BSD
      case AF_UNIX  :  return Socket::FAMILY_IPC;
#endif
      case AF_INET  :  return Socket::FAMILY_IPv4;
      case AF_INET6 :  return Socket::FAMILY_IPv6;
      default       :  return Socket::FAMILY_UNKNOWN;
   }
}

//------------------------------------------------------------------------------
//!
inline int  toAPI( Socket::Type type )
{
   switch( type )
   {
      case Socket::TYPE_UNKNOWN  :  return -1;
      case Socket::TYPE_RAW      :  return SOCK_RAW;
      case Socket::TYPE_DATAGRAM :  return SOCK_DGRAM;
      case Socket::TYPE_STREAM   :  return SOCK_STREAM;
      case Socket::TYPE_SEQPACKET:  return SOCK_SEQPACKET;
      default                    :  return -1;
   }
}

//------------------------------------------------------------------------------
//!
inline Socket::Type  fromAPI_type( int type )
{
   switch( type )
   {
      case SOCK_RAW      : return Socket::TYPE_RAW;
      case SOCK_DGRAM    : return Socket::TYPE_DATAGRAM;
      case SOCK_STREAM   : return Socket::TYPE_STREAM;
      case SOCK_SEQPACKET: return Socket::TYPE_SEQPACKET;
      default            : return Socket::TYPE_UNKNOWN;
   }
}

//------------------------------------------------------------------------------
//!
inline int  toAPI( Socket::Protocol protocol )
{
   switch( protocol )
   {
      case Socket::PROTOCOL_DEFAULT:  return 0;
      case Socket::PROTOCOL_UDP    :  return IPPROTO_UDP;
      case Socket::PROTOCOL_TCP    :  return IPPROTO_TCP;
      //case Socket::PROTOCOL_SCTP :  return IPPROTO_SCTP;
      //case Socket::PROTOCOL_DCCP :  return IPPROTO_DCCP;
      default                      :  return -1;
   }
}

//------------------------------------------------------------------------------
//!
inline Socket::Protocol  fromAPI_protocol( int protocol )
{
   switch( protocol )
   {
      case IPPROTO_UDP   : return Socket::PROTOCOL_UDP;
      case IPPROTO_TCP   : return Socket::PROTOCOL_TCP;
      //case IPPROTO_SCTP: return Socket::PROTOCOL_SCTP;
      //case IPPROTO_DCCP: return Socket::PROTOCOL_DCCP;
      default            : return Socket::PROTOCOL_DEFAULT;
   }
}

//------------------------------------------------------------------------------
//!
inline socklen_t  getSize( const sockaddr_storage& stor )
{
   switch( stor.ss_family )
   {
      case AF_UNSPEC:  return sizeof(sockaddr); // Or 0?
#if BASE_SOCKET_USE_BSD
      case AF_UNIX  :  return sizeof(sockaddr);
#endif
      case AF_INET  :  return sizeof(sockaddr_in);
      case AF_INET6 :  return sizeof(sockaddr_in6);
      default       :  CHECK( false );
                       return (socklen_t)0;
   }
}

//------------------------------------------------------------------------------
//!
addrinfo*  getAddrInfo( Socket& socket, const char* hostname, uint16_t port );

//------------------------------------------------------------------------------
//!
bool  bind( Socket& socket, uint16_t port, Socket::Endpoint& ep );


/*==============================================================================
  CLASS Socket::Endpoint::Impl
==============================================================================*/

class Socket::Endpoint::Impl
{
public:
   /*----- methods -----*/
   Impl() { reset(); }
   inline void  reset() { memset( &_stor, 0, sizeof(_stor) ); }

   /*----- members -----*/
   sockaddr_storage  _stor;
}; //class Socket::Endpoint::Impl


/*==============================================================================
  CLASS Socket::Impl
==============================================================================*/

class Socket::Impl
{
public:
   /*----- members -----*/
   SocketDescriptor  _sock;  //!< The socket descriptor.
}; //class Socket::Impl

NAMESPACE_END

#endif //BASE_SOCKET_USE_BSD || BASE_SOCKET_USE_WINSOCK


#endif //BASE__SOCKET_IMPL_H
