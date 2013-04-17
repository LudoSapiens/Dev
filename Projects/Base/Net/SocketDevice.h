/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef BASE_SOCKET_DEVICE_H
#define BASE_SOCKET_DEVICE_H

#include <Base/StdDefs.h>

#include <Base/IO/IODevice.h>
#include <Base/Net/Socket.h>
#include <Base/Util/RCP.h>

NAMESPACE_BEGIN


/*==============================================================================
  CLASS SocketDevice
==============================================================================*/
class SocketDevice:
   public IODevice
{
public:

   /*----- methods -----*/
   inline const Socket::Endpoint&  localEndpoint() const { return _socket->localEndpoint(); }
   inline const Socket::Endpoint&  remoteEndpoint() const { return _remoteEndpoint; }

   //inline       Socket&  socket()       { return *_socket; }
   inline const Socket&  socket() const { return *_socket; }

protected:

   friend class Socket;

   /*----- methods -----*/
   SocketDevice( Socket* socket );
   virtual ~SocketDevice();

   /*----- members -----*/
   RCP<Socket>       _socket;
   Socket::Endpoint  _remoteEndpoint;

private:
}; //class SocketDevice


/*==============================================================================
  CLASS ConnectedSocketDevice
==============================================================================*/
class ConnectedSocketDevice:
   public SocketDevice
{
public:

   /*----- methods -----*/

protected:

   friend class Socket;

   ConnectedSocketDevice( Socket* socket );
   virtual ~ConnectedSocketDevice();

   BASE_DLL_API virtual size_t  doRead( char* data, size_t n );
   BASE_DLL_API virtual size_t  doPeek( char* data, size_t n );

   BASE_DLL_API virtual size_t  doWrite( const char* data, size_t n );

private:
}; //class ConnectedSocketDevice


/*==============================================================================
  CLASS ConnectionlessSocketDevice
==============================================================================*/
class ConnectionlessSocketDevice:
   public SocketDevice
{
public:

   /*----- methods -----*/

   inline const Socket::Endpoint&  lastOrigin() const { return _lastOrigin; }

   inline Socket::Endpoint&  destination() { return _remoteEndpoint; }

protected:

   friend class Socket;

   ConnectionlessSocketDevice( Socket* socket );
   virtual ~ConnectionlessSocketDevice();

   BASE_DLL_API virtual size_t  doRead( char* data, size_t n );
   BASE_DLL_API virtual size_t  doPeek( char* data, size_t n );

   BASE_DLL_API virtual size_t  doWrite( const char* data, size_t n );

   /*----- members -----*/
   Socket::Endpoint  _lastOrigin;

private:
}; //class ConnectionlessSocketDevice


NAMESPACE_END

#endif //BASE_SOCKET_DEVICE_H
