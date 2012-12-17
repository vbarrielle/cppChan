#include <iostream>
#include <string>
#include <future>

#if 1
#define ATOMIC_CHANNEL
#endif

#ifdef ATOMIC_CHANNEL
#include "atomic_channel.hh"
#else
#include "channel.hh"
#endif

static const int kGo = 0;
static const int kQuit = 1;
static const int kDone = 2;

int
main( int argc, char ** argv )
{
  channel<int> sayHello(0), sayWorld(0), quitter(0);

  std::cerr << "sayHello: " << &sayHello << "\n";
  std::cerr << "sayWorld: " << &sayWorld << "\n";
  std::cerr << "quitter: " << &quitter << "\n";

  auto d = std::async( std::launch::async, [&]
      {
        for ( int i = 0; i < 1000; ++i )
        {
          std::cout << "Hello ";
          sayWorld << kGo;
          int a;
          sayHello >> a;
        }
        sayWorld << kQuit;
      } );

  auto b = std::async( std::launch::async, [&]
      {
        while ( true )
        {
          int reply;
          sayWorld >> reply;
          if ( reply == kQuit )
            break;
          std::cout << "world!\n";
          sayHello << kGo;
        }
        quitter << kDone;
      } );

  int a;
  quitter >> a;

  return 0;
}
