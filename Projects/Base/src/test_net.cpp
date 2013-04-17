/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Base/Dbg/UnitTest.h>

#include <Base/Net/SocketDevice.h>
//#include <Base/Net/_SocketImpl.h>

#include <Base/Dbg/Defs.h>
#include <Base/Dbg/DebugStream.h>
#include <Base/MT/Semaphore.h>
#include <Base/MT/Thread.h>
#include <Base/Util/Platform.h>

USING_NAMESPACE

// Some global variables used in multiple places.
const uint16_t  sPort = 3456;
Test::Result    gServerRes;
Test::Result    gClientRes;
Semaphore*      gSemaphore;


class TCP_Connected_Client:
   public Task
{
   virtual void execute()
   {
      // Create socket.
      RCP<Socket> socket = new Socket( Socket::FAMILY_IPv4, Socket::TYPE_STREAM, Socket::PROTOCOL_TCP );
      TEST_ADD( gClientRes, socket->state() == Socket::STATE_UNCONNECTED );

      // Wait for the server to be ready.
      gSemaphore->wait();

      // Connect to the server.
      RCP<ConnectedSocketDevice> device = socket->connect( Socket::Endpoint(sPort) );
      CHECK( device.isValid() );
      TEST_ADD( gClientRes, device.isValid() );
      TEST_ADD( gClientRes, socket->state() == Socket::STATE_CONNECTED );

      //fprintf( stderr, "CLIENT socket: %s\n", socket->localEndpoint().toString().cstr() );
      //fprintf( stderr, "CLIENT  local: %s\n", device->localEndpoint().toString().cstr() );
      //fprintf( stderr, "CLIENT remote: %s\n", device->remoteEndpoint().toString().cstr() );

      // Send a message.
      size_t s = device->write( "1234", 4 );
      TEST_ADD( gClientRes, s == 4 );


      //// Receive server's response.
      char data[100];
      s = device->read( data, 100 );
      TEST_ADD( gClientRes, s == 5 );
      TEST_ADD( gClientRes, strncmp(data, "43210", 5) == 0 );
   }
};

class TCP_Connected_Server:
   public Task
{
   virtual void execute()
   {
      // Create socket.
      RCP<Socket> socket = new Socket( Socket::FAMILY_IPv4, Socket::TYPE_STREAM, Socket::PROTOCOL_TCP );
      TEST_ADD( gServerRes, socket->state() == Socket::STATE_UNCONNECTED );

      // Listen over a port.
      bool ok = socket->listen( sPort );
      CHECK( ok );
      TEST_ADD( gServerRes, ok );
      TEST_ADD( gServerRes, socket->state() == Socket::STATE_LISTENING );

      gSemaphore->post();
      // Handle connections.
      RCP<ConnectedSocketDevice> device = socket->accept();
      // ... would start a new thread if we wanted to support multiple connections.
      TEST_ADD( gServerRes, socket->state() == Socket::STATE_LISTENING );
      TEST_ADD( gServerRes, device->socket().state() == Socket::STATE_CONNECTED );

      //fprintf( stderr, "SERVER socket: %s\n", socket->localEndpoint().toString().cstr() );
      //fprintf( stderr, "SERVER  local: %s\n", device->localEndpoint().toString().cstr() );
      //fprintf( stderr, "SERVER remote: %s\n", device->remoteEndpoint().toString().cstr() );

      CHECK( device.isValid() );
      TEST_ADD( gServerRes, device.isValid() );

      // ... the client will write us something.
      char data[100];
      size_t s = device->read( data, 100 );
      TEST_ADD( gServerRes, s == 4 );
      TEST_ADD( gServerRes, strncmp(data, "1234", 4) == 0 );

      // Write something back to the client.
      s = device->write( "43210", 5 );
      TEST_ADD( gServerRes, s == 5 );
   }
};


class UDP_Connected_Client:
   public Task
{
   virtual void execute()
   {
      // Create socket.
      RCP<Socket> socket = new Socket( Socket::FAMILY_IPv4, Socket::TYPE_DATAGRAM, Socket::PROTOCOL_UDP );
      TEST_ADD( gClientRes, socket->state() == Socket::STATE_UNCONNECTED );

      // Connect to the server.
      RCP<ConnectedSocketDevice> device = socket->connect( Socket::Endpoint(sPort) );
      CHECK( device.isValid() );
      TEST_ADD( gClientRes, device.isValid() );
      TEST_ADD( gClientRes, socket->state() == Socket::STATE_CONNECTED );

      // Send a message.
      size_t s = device->write( "1234", 4 );
      TEST_ADD( gClientRes, s == 4 );
   }
};

class UDP_Connected_Server:
   public Task
{
   virtual void execute()
   {
      // Create socket.
      RCP<Socket> socket = new Socket( Socket::FAMILY_IPv4, Socket::TYPE_DATAGRAM, Socket::PROTOCOL_UDP );
      TEST_ADD( gServerRes, socket->state() == Socket::STATE_UNCONNECTED );

      // Get a connectionless device.
      RCP<ConnectionlessSocketDevice> device = socket->device( sPort );
      TEST_ADD( gServerRes, socket->state() == Socket::STATE_BOUND );

      CHECK( device.isValid() );
      TEST_ADD( gServerRes, device.isValid() );

      // ... the client will write us something.
      char data[100];
      size_t s = device->read( data, 100 );
      TEST_ADD( gServerRes, s == 4 );
      TEST_ADD( gServerRes, strncmp(data, "1234", 4) == 0 );
   }
};

void net_tcp_connected( Test::Result& res )
{
   gServerRes.reset();
   gClientRes.reset();

   Semaphore semaphore( 0 );
   gSemaphore = &semaphore;
   Thread serverTask( new TCP_Connected_Server() );
   Thread clientTask( new TCP_Connected_Client() );
   serverTask.wait();
   clientTask.wait();

   res.add( gServerRes );
   res.add( gClientRes );
}

void net_udp_connected( Test::Result& res )
{
#if PLAT_ANDROID
   // The code below is rather slow.
   printf(" *** Skipping under Android *** ");
   return;
#endif

   gServerRes.reset();
   gClientRes.reset();

   Thread serverTask( new UDP_Connected_Server() );
   Thread clientTask( new UDP_Connected_Client() );
   serverTask.wait();
   clientTask.wait();

   res.add( gServerRes );
   res.add( gClientRes );
}


void net_udp_connectionless( Test::Result& res )
{
   char data[100];
   size_t s;

   // Set up server.
   RCP<Socket>  ss = new Socket( Socket::FAMILY_IPv4, Socket::TYPE_DATAGRAM, Socket::PROTOCOL_UDP );
   TEST_ADD( res, ss->state() == Socket::STATE_UNCONNECTED );

   RCP<ConnectionlessSocketDevice>  sd = ss->device( sPort );
   CHECK( sd.isValid() );
   TEST_ADD( res, ss->state() == Socket::STATE_BOUND );

   // Check that nothing can be read yet (FIXME: socket is blocking).
   //s = sd->read( data, 100 );
   //TEST_ADD( res, s == 0 );

   // Set up client.
   RCP<Socket>  cs = new Socket( Socket::FAMILY_IPv4, Socket::TYPE_DATAGRAM, Socket::PROTOCOL_UDP );
   TEST_ADD( res, cs->state() == Socket::STATE_UNCONNECTED );

   RCP<ConnectionlessSocketDevice>  cd = cs->device();
   CHECK( cd.isValid() );
   TEST_ADD( res, cs->state() == Socket::STATE_UNCONNECTED );

   // Make it point to the server.
   cd->destination().set( sPort );
   TEST_ADD( res, cd->destination().port() == sPort );

   // Send something from client to server.
   s = cd->write( "allo", 4 );
   TEST_ADD( res, s == 4 );

   s = sd->read( data, 100 );
   TEST_ADD( res, s == 4 );
   TEST_ADD( res, strncmp(data, "allo", 4) == 0 );

   //StdErr << sd->lastOrigin().toString() << nl;

   //StdErr << nl
   //       << "Check if port " << cd->destination().port() << " is open" << nl
   //       << "> sudo nmap -sU -P0 -p " << cd->destination().port() << " localhost" << nl;
   //getchar();
}

void net_resolve( Test::Result& res )
{
   StdOut << nl;

   String  host;
   String  hostname;
   Socket::Endpoint  ep;
   Vector< Socket::Endpoint >  eps;
   bool ok;

   host = "ludosapiens.com";
   ok = resolveHost( host.cstr(), eps );
   TEST_ADD( res, ok );

   // Not sure why we receive 2 copies, but hey.
   StdOut << host << " --> ";
   for( uint i = 0; i < eps.size(); ++i )
   {
      if( i != 0 )  StdOut << ", ";
      hostname = eps[i].hostname();
      StdOut << hostname;
      TEST_ADD( res, hostname == "64.14.68.73" );
   }

   ok = resolveHost( host.cstr(), ep );
   TEST_ADD( res, ok );
   hostname = ep.hostname();
   StdOut << " (" << hostname << ")" << nl;
   TEST_ADD( res, hostname == "64.14.68.73" );

   host = "localhost";
   ok = resolveHost( host.cstr(), ep );
   hostname = ep.hostname();
   TEST_ADD( res, ok );
   StdOut << String(' ', 6) << host << " --> " << hostname << nl;
   TEST_ADD( res, hostname == "127.0.0.1" );

   // IPv6 format for localhost.
   host = "::1";
   ok = resolveHost( host.cstr(), ep );
   hostname = ep.hostname();
   TEST_ADD( res, ok );
   StdOut << String(' ', 12) << host << " --> " << hostname << nl;
   TEST_ADD( res, hostname == "::1" );

   // Listing all of the IPs for localhost.
   host = "localhost";
   eps.clear();
   ok = resolveHost( host.cstr(), eps );
   StdOut << String(' ', 6) << host << " --> ";
   for( uint i = 0; i < eps.size(); ++i )
   {
      if( i != 0 )  StdOut << ", ";
      hostname = eps[i].hostname();
      StdOut << eps[i].toString();
   }

   StdOut << nl;
}

void init_net()
{
   initNetLayer();
   RCP<Test::Collection> col = new Test::Collection( "net", "Collection for Base/Net" );
   col->add( new Test::Function("tcp_connected"     , "Tests connected TCP client/server"     , net_tcp_connected      ) );
   col->add( new Test::Function("udp_connected"     , "Tests connected UDP client/server"     , net_udp_connected      ) );
   col->add( new Test::Function("udp_connectionless", "Tests connectionless UDP client/server", net_udp_connectionless ) );
   Test::standard().add( col.ptr() );

   col = new Test::Collection( "net_special", "Collection of special test for Base/Net" );
   col->add( new Test::Function("resolve"   , "Prints the IP of the current machine", net_resolve    ) );
   Test::special().add( col.ptr() );

}
