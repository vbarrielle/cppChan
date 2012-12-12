#include <iostream>
#include <string>

#include "channel.hh"

static const int kGo = 0;
static const int kQuit = 1;
static const int kDone = 2;

int
main( int argc, char ** argv )
{
#if 0
  channel<int> sayHello, sayWorld, quitter;
#else
  channel<int> sayHello(1), sayWorld(1), quitter(1);
#endif

  std::cerr << "sayHello: " << &sayHello << "\n";
  std::cerr << "sayWorld: " << &sayWorld << "\n";
  std::cerr << "quitter: " << &quitter << "\n";

  auto d = std::async( std::launch::async, [&]
      {
        for ( int i = 0; i < 1000; ++i )
        {
          std::cout << "Hello ";
          sayWorld << kGo;
          std::cerr << "Hello: said go\n";
          int a;
          std::cerr << "Hello: receive go\n";
          sayHello >> a;
          std::cerr << "Hello: received go\n";
        }
        sayWorld << kQuit;
      } );

  auto b = std::async( std::launch::async, [&]
      {
        while ( true )
        {
          int reply;
          std::cerr << "World: receive go\n";
          sayWorld >> reply;
          std::cerr << "World: received go\n";
          if ( reply == kQuit )
            break;
          std::cout << "world!\n";
          std::cerr << "World: say go\n";
          sayHello << kGo;
          std::cerr << "World: said go\n";
        }
        quitter << kDone;
      } );

  int a;
  quitter >> a;

  return 0;
}
