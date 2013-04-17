/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Base/Net/_SocketImpl.h>

#include <Base/Dbg/Defs.h>

/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN



UNNAMESPACE_END


NAMESPACE_BEGIN


// ========================
// === BERKELEY SOCKETS ===
// ============================================================================
// ============================================================================
#if BASE_SOCKET_USE_BSD

/*==============================================================================
  Special routines used by Modules.cpp.
==============================================================================*/

//------------------------------------------------------------------------------
//!
void  initNetLayer()
{
}

//------------------------------------------------------------------------------
//!
void  termNetLayer()
{
}

/*==============================================================================
  Utility routines.
==============================================================================*/

//------------------------------------------------------------------------------
//!
void  printAPIError( const char* msg )
{
   perror( msg );
}

#endif // BASE_SOCKET_USE_BSD


// ===============
// === WINSOCK ===
// ============================================================================
// ============================================================================
#if BASE_SOCKET_USE_WINSOCK


/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN

WSADATA  _wsaData;

UNNAMESPACE_END

/*==============================================================================
  Special routines used by Modules.cpp.
==============================================================================*/

//------------------------------------------------------------------------------
//!
void  initNetLayer()
{
   int err = WSAStartup( MAKEWORD(2, 2), &_wsaData );
   if( err != 0 )
   {
      const char* errMsg;
      switch( err )
      {
         case WSASYSNOTREADY: errMsg = "System not ready"         ;  break;
         case WSAEBADF      : errMsg = "Version not supported"    ;  break;
         case WSAEINPROGRESS: errMsg = "In progress"              ;  break;
         case WSAEPROCLIM   : errMsg = "Tasks limit reached"      ;  break;
         case WSAEFAULT     : errMsg = "Invalid lpWSAData pointer";  break;
         default            : errMsg = "Unknown error"            ;  break;
      }
      printf( "WSAStartup failed: (%d) %s.", err, errMsg );
   }
}

//------------------------------------------------------------------------------
//!
void  termNetLayer()
{
   WSACleanup();
}

/*==============================================================================
  Utility routines.
==============================================================================*/

//------------------------------------------------------------------------------
//!
void  printAPIError( const char* msg )
{
   const char* errMsg;
   int err = WSAGetLastError();
   switch( err )
   {
      case 0                 : errMsg = "No error"                           ;  break;
      case WSAEINTR          : errMsg = "Interrupted system call"            ;  break;
      case WSAEBADF          : errMsg = "Bad file number"                    ;  break;
      case WSAEACCES         : errMsg = "Permission denied"                  ;  break;
      case WSAEFAULT         : errMsg = "Bad address"                        ;  break;
      case WSAEINVAL         : errMsg = "Invalid argument"                   ;  break;
      case WSAEMFILE         : errMsg = "Too many open sockets"              ;  break;
      case WSAEWOULDBLOCK    : errMsg = "Operation would block"              ;  break;
      case WSAEINPROGRESS    : errMsg = "Operation now in progress"          ;  break;
      case WSAEALREADY       : errMsg = "Operation already in progress"      ;  break;
      case WSAENOTSOCK       : errMsg = "Socket operation on non-socket"     ;  break;
      case WSAEDESTADDRREQ   : errMsg = "Destination address required"       ;  break;
      case WSAEMSGSIZE       : errMsg = "Message too long"                   ;  break;
      case WSAEPROTOTYPE     : errMsg = "Protocol wrong type for socket"     ;  break;
      case WSAENOPROTOOPT    : errMsg = "Bad protocol option"                ;  break;
      case WSAEPROTONOSUPPORT: errMsg = "Protocol not supported"             ;  break;
      case WSAESOCKTNOSUPPORT: errMsg = "Socket type not supported"          ;  break;
      case WSAEOPNOTSUPP     : errMsg = "Operation not supported on socket"  ;  break;
      case WSAEPFNOSUPPORT   : errMsg = "Protocol family not supported"      ;  break;
      case WSAEAFNOSUPPORT   : errMsg = "Address family not supported"       ;  break;
      case WSAEADDRINUSE     : errMsg = "Address already in use"             ;  break;
      case WSAEADDRNOTAVAIL  : errMsg = "Can't assign requested address"     ;  break;
      case WSAENETDOWN       : errMsg = "Network is down"                    ;  break;
      case WSAENETUNREACH    : errMsg = "Network is unreachable"             ;  break;
      case WSAENETRESET      : errMsg = "Net connection reset"               ;  break;
      case WSAECONNABORTED   : errMsg = "Software caused connection abort"   ;  break;
      case WSAECONNRESET     : errMsg = "Connection reset by peer"           ;  break;
      case WSAENOBUFS        : errMsg = "No buffer space available"          ;  break;
      case WSAEISCONN        : errMsg = "Socket is already connected"        ;  break;
      case WSAENOTCONN       : errMsg = "Socket is not connected"            ;  break;
      case WSAESHUTDOWN      : errMsg = "Can't send after socket shutdown"   ;  break;
      case WSAETOOMANYREFS   : errMsg = "Too many references can't splice"   ;  break;
      case WSAETIMEDOUT      : errMsg = "Connection timed out"               ;  break;
      case WSAECONNREFUSED   : errMsg = "Connection refused"                 ;  break;
      case WSAELOOP          : errMsg = "Too many levels of symbolic links"  ;  break;
      case WSAENAMETOOLONG   : errMsg = "File name too long"                 ;  break;
      case WSAEHOSTDOWN      : errMsg = "Host is down"                       ;  break;
      case WSAEHOSTUNREACH   : errMsg = "No route to host"                   ;  break;
      case WSAENOTEMPTY      : errMsg = "Directory not empty"                ;  break;
      case WSAEPROCLIM       : errMsg = "Too many processes"                 ;  break;
      case WSAEUSERS         : errMsg = "Too many users"                     ;  break;
      case WSAEDQUOT         : errMsg = "Disc quota exceeded"                ;  break;
      case WSAESTALE         : errMsg = "Stale NFS file handle"              ;  break;
      case WSAEREMOTE        : errMsg = "Too many levels of remote in path"  ;  break;
      case WSASYSNOTREADY    : errMsg = "Network system is unavailable"      ;  break;
      case WSAVERNOTSUPPORTED: errMsg = "Winsock version out of range"       ;  break;
      case WSANOTINITIALISED : errMsg = "WSAStartup not yet called"          ;  break;
      case WSAEDISCON        : errMsg = "Graceful shutdown in progress"      ;  break;
      case WSAHOST_NOT_FOUND : errMsg = "Host not found"                     ;  break;
      case WSANO_DATA        : errMsg = "No host data of that type was found";  break;
      default                : errMsg = "Unknown error"                      ;  break;
   }
   printf( msg );
   printf( ": " );
   printf( "(%d) ", err );
   printf( errMsg );
   printf( "\n" );
}

#endif //BASE_SOCKET_USE_WINSOCK

// ==============================
// === SHARED BSD AND WINSOCK ===
// ============================================================================
// ============================================================================


#if BASE_SOCKET_USE_BSD || BASE_SOCKET_USE_WINSOCK


/*==============================================================================
  CLASS Socket::Endpoint
==============================================================================*/

//------------------------------------------------------------------------------
//!
Socket::Endpoint::Endpoint()
{
   _impl = new Impl();
}

//------------------------------------------------------------------------------
//!
Socket::Endpoint::Endpoint( uint16_t port )
{
   _impl = new Impl();
   set( port );
}

//------------------------------------------------------------------------------
//!
Socket::Endpoint::Endpoint( const uint32_t ipv4addr, uint16_t port )
{
   _impl = new Impl();
   set( ipv4addr, port );
}

//------------------------------------------------------------------------------
//!
Socket::Endpoint::Endpoint( uint8_t a, uint8_t b, uint8_t c, uint8_t d, uint16_t port )
{
   _impl = new Impl();
   set( a, b, c, d, port );
}

//------------------------------------------------------------------------------
//!
Socket::Endpoint::Endpoint( const uint8_t* ipv6addr, uint16_t port )
{
   _impl = new Impl();
   set( ipv6addr, port );
}

//------------------------------------------------------------------------------
//!
Socket::Endpoint::Endpoint( const String& host, uint16_t port, Family family )
{
   _impl = new Impl();
   set( host, port, family );
}

//------------------------------------------------------------------------------
//!
Socket::Endpoint::Endpoint( const Endpoint& src )
{
   _impl = new Impl();
   *this = src;
}

//------------------------------------------------------------------------------
//!
Socket::Endpoint::~Endpoint()
{
   delete _impl;
}

//------------------------------------------------------------------------------
//!
Socket::Endpoint&
Socket::Endpoint::operator=( const Endpoint& src )
{
   *_impl = *(src._impl);
   return *this;
}

//------------------------------------------------------------------------------
//!
Socket::Endpoint&
Socket::Endpoint::reset()
{
   _impl->reset();
   return *this;
}

//------------------------------------------------------------------------------
//!
Socket::Endpoint&
Socket::Endpoint::set( uint16_t port )
{
   sockaddr_in& addr = reinterpret_cast<sockaddr_in&>(_impl->_stor);
   addr.sin_family      = AF_INET;
   addr.sin_port        = htons( port );
   addr.sin_addr.s_addr = htonl( 0x7F000001 ); // 127.0.0.1
   return *this;
}

//------------------------------------------------------------------------------
//!
Socket::Endpoint&
Socket::Endpoint::set( const uint32_t ipv4addr, uint16_t port )
{
   sockaddr_in& addr = reinterpret_cast<sockaddr_in&>(_impl->_stor);
   addr.sin_family      = AF_INET;
   addr.sin_port        = htons( port );
   addr.sin_addr.s_addr = htonl( ipv4addr );
   return *this;
}

//------------------------------------------------------------------------------
//!
Socket::Endpoint&
Socket::Endpoint::set( uint8_t a, uint8_t b, uint8_t c, uint8_t d, uint16_t port )
{
   sockaddr_in& addr = reinterpret_cast<sockaddr_in&>(_impl->_stor);
   addr.sin_family      = AF_INET;
   addr.sin_port        = htons( port );
   addr.sin_addr.s_addr = htonl( (a << 24) | (b << 16) | (c << 8) | d );
   return *this;
}

//------------------------------------------------------------------------------
//!
Socket::Endpoint&
Socket::Endpoint::set( const uint8_t* ipv6addr, uint16_t port )
{
   sockaddr_in6& addr = reinterpret_cast<sockaddr_in6&>(_impl->_stor);
   addr.sin6_family = AF_INET6;
   addr.sin6_port   = htons( port );
   memcpy( &addr.sin6_addr, ipv6addr, sizeof(in6_addr) );
   return *this;
}

//------------------------------------------------------------------------------
//!
Socket::Endpoint&
Socket::Endpoint::set( const String& host, uint16_t port, Family family )
{
   _impl->_stor.ss_family = toAPI( family );
   switch( family )
   {
      case FAMILY_IPv4:
      {
         sockaddr_in& addr = reinterpret_cast<sockaddr_in&>( _impl->_stor );
         if( !resolveHost(host.cstr(), *this) )
         {
            fprintf( stderr, "Endpoint::set(const String&, uint16_t, Family) - Failed to resolve host.\n" );
         }
         addr.sin_port = htons( port );
      }  break;
      case FAMILY_IPv6:
      {
         sockaddr_in6& addr = reinterpret_cast<sockaddr_in6&>( _impl->_stor );
         if( !resolveHost(host.cstr(), *this) )
         {
            fprintf( stderr, "Endpoint::set(const String&, uint16_t, Family) - Failed to resolve host.\n" );
         }
         addr.sin6_port = htons( port );
      }  break;
      default:
      {
         fprintf( stderr, "Endpoint::set(const String&, uint16_t, Family) - Unsupported family: %d.\n", family );
      }  break;
   }
   return *this;
}

//------------------------------------------------------------------------------
//!
Socket::Endpoint&
Socket::Endpoint::get( uint16_t& port )
{
   switch( _impl->_stor.ss_family )
   {
      case AF_INET:
      {
         sockaddr_in& addr = reinterpret_cast<sockaddr_in&>(_impl->_stor);
         port = ntohs( addr.sin_port );
      }  break;
      case AF_INET6:
      {
         sockaddr_in6& addr = reinterpret_cast<sockaddr_in6&>(_impl->_stor);
         port = ntohs( addr.sin6_port );
      }  break;
      default:
      {
         fprintf( stderr, "Endpoint::get(uint16_t&) - Invalid family.\n" );
      }  break;
   }
   return *this;
}

//------------------------------------------------------------------------------
//!
Socket::Endpoint&
Socket::Endpoint::get( uint32_t& ipv4addr, uint16_t& port )
{
   switch( _impl->_stor.ss_family )
   {
      case AF_INET:
      {
         sockaddr_in& addr = reinterpret_cast<sockaddr_in&>(_impl->_stor);
         ipv4addr = ntohl( addr.sin_addr.s_addr );
         port     = ntohs( addr.sin_port );
      }  break;
      default:
      {
         fprintf( stderr, "Endpoint::get( uint16_t& ) - Invalid family.\n" );
      }  break;
   }
   return *this;
}

//------------------------------------------------------------------------------
//!
Socket::Endpoint&
Socket::Endpoint::get( uint8_t& a, uint8_t& b, uint8_t& c, uint8_t& d, uint16_t& port )
{
   switch( _impl->_stor.ss_family )
   {
      case AF_INET:
      {
         sockaddr_in& addr = reinterpret_cast<sockaddr_in&>(_impl->_stor);
         uint32_t tmp = ntohl( addr.sin_addr.s_addr );
         d = (tmp & 0xFF);
         tmp >>= 8;
         c = (tmp & 0xFF);
         tmp >>= 8;
         b = (tmp & 0xFF);
         tmp >>= 8;
         a = (tmp & 0xFF);
         port = ntohs( addr.sin_port );
      }  break;
      default:
      {
         fprintf( stderr, "Endpoint::get( uint16_t& ) - Invalid family.\n" );
      }  break;
   }
   return *this;
}
//------------------------------------------------------------------------------
//!
Socket::Endpoint&
Socket::Endpoint::get( uint8_t* ipv6addr, uint16_t& port )
{
   switch( _impl->_stor.ss_family )
   {
      case AF_INET6:
      {
         sockaddr_in6& addr = reinterpret_cast<sockaddr_in6&>(_impl->_stor);
         memcpy( ipv6addr, &addr.sin6_addr, sizeof(in6_addr) );
         port = ntohs( addr.sin6_port );
      }  break;
      default:
      {
         fprintf( stderr, "Endpoint::get( uint16_t& ) - Invalid family.\n" );
      }  break;
   }
   return *this;
}
//------------------------------------------------------------------------------
//!
Socket::Endpoint&
Socket::Endpoint::get( String& host, uint16_t& port, Family& family )
{
   char tmpHost[NI_MAXHOST+1];
   int flags = 0; //NI_NUMERICHOST
   int err = getnameinfo( (const sockaddr*)&(_impl->_stor), sizeof(_impl->_stor), tmpHost, NI_MAXHOST, NULL, 0, flags );
   if( err != 0 )
   {
      fprintf( stderr, "Endpoint::get(String&, uint16_t&, Family&) - getnameinfo() failed: %s\n", gai_strerror(err) );
      return *this;
   }
   switch( _impl->_stor.ss_family )
   {
      case AF_INET:
      {
         sockaddr_in& addr = reinterpret_cast<sockaddr_in&>(_impl->_stor);
         host   = tmpHost;
         port   = ntohs( addr.sin_port );
         family = FAMILY_IPv4;
      }  break;
      case AF_INET6:
      {
         sockaddr_in6& addr = reinterpret_cast<sockaddr_in6&>(_impl->_stor);
         host   = tmpHost;
         port   = ntohs( addr.sin6_port );
         family = FAMILY_IPv6;
      }  break;
   }
   return *this;
}


//------------------------------------------------------------------------------
//!
Socket::Family
Socket::Endpoint::family() const
{
   return fromAPI_family( _impl->_stor.ss_family );
}

//------------------------------------------------------------------------------
//!
String
Socket::Endpoint::hostname() const
{
   char host[NI_MAXHOST+1];
   //char serv[NI_MAXSERV+1];
   int flags = NI_NUMERICHOST;
   const sockaddr& addr = reinterpret_cast<const sockaddr&>( _impl->_stor );
   int err = getnameinfo( &addr, getSize(_impl->_stor), host, NI_MAXHOST, NULL, 0, flags );
   if( err != 0 )
   {
      printAPIError( "ERROR - getnameinfo() failed" );
      return String();
   }
   return String( host );
}

//------------------------------------------------------------------------------
//!
uint16_t
Socket::Endpoint::port() const
{
   switch( _impl->_stor.ss_family )
   {
      case AF_INET:
      {
         const sockaddr_in& addr = reinterpret_cast<const sockaddr_in&>( _impl->_stor );
         return ntohs( addr.sin_port );
      }  break;
      case AF_INET6:
      {
         const sockaddr_in6& addr = reinterpret_cast<const sockaddr_in6&>( _impl->_stor );
         return ntohs( addr.sin6_port );
      }  break;
      default:
         fprintf( stderr, "Endpoint::port() - Unknown family: %d.\n", _impl->_stor.ss_family );
   }
   return 0;
}

//------------------------------------------------------------------------------
//!
String
Socket::Endpoint::toString() const
{
   String tmp = toStr( family() );
   tmp += '[';
   tmp += hostname();
   tmp += ',';
   tmp += ' ';
   tmp += String(port());
   tmp += ']';
   return tmp;
}

//------------------------------------------------------------------------------
//!
addrinfo*  getAddrInfo( Socket& socket, const char* hostname, uint16_t port )
{
   // Prepare request.
   addrinfo  request;
   memset( &request, 0, sizeof(request) );
   request.ai_flags    = AI_PASSIVE; // Use my IP when I send NULL hostname in getaddrinfo.
   request.ai_family   = toAPI( socket.family()   );
   request.ai_socktype = toAPI( socket.type()     );
   request.ai_protocol = toAPI( socket.protocol() );

   // getaddrinfo() requires the port as a string.
   String portStr( port );

   // Retrieve candidates.
   addrinfo* candidates;
   int err = getaddrinfo( hostname, portStr.cstr(), &request, &candidates );
   if( err != 0 )
   {
      printAPIError( "ERROR - getaddrinfo() failed" );
      return NULL;
   }

   return candidates;
}

//------------------------------------------------------------------------------
//! Tries to bind the socket to a specified local port.
bool  bind( Socket& socket, uint16_t port, Socket::Endpoint& ep )
{
   addrinfo* candidates = getAddrInfo( socket, NULL, port );
   if( candidates == NULL )
   {
      return false;
   }

   // Select first valid candidate.
   //for( addrinfo* cur = candidates; cur != NULL; cur = cur->ai_next )
   //{
   //   // FIXME: Update family/type/protocol if it changed? (requires making new socket)
   //}
   int err = ::bind( socket.impl()->_sock, candidates->ai_addr, SocketSSizeT(candidates->ai_addrlen) );
   if( err != 0 )
   {
      printAPIError( "ERROR - bind() failed" );
      DBG_HALT();
      freeaddrinfo( candidates );
      return false;
   }

   // Save the sockaddr structure.
   sockaddr_storage& stor = ep.impl()->_stor;
   memcpy( &stor, candidates->ai_addr, candidates->ai_addrlen );

   freeaddrinfo( candidates );

   return true;
}

#endif //BASE_SOCKET_USE_BSD || BASE_SOCKET_USE_WINSOCK

NAMESPACE_END
