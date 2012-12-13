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
          fprintf( stderr, "%s to %p\n", "Hello: say go", &sayWorld );
          sayWorld << kGo;
          fprintf( stderr, "%s to %p\n", "Hello: said go", &sayWorld );
          int a;
          fprintf( stderr, "%s from %p\n", "Hello: receive go", &sayHello );
          sayHello >> a;
          fprintf( stderr, "%s from %p\n", "Hello: received go", &sayHello );
        }
        sayWorld << kQuit;
      } );

  auto b = std::async( std::launch::async, [&]
      {
        while ( true )
        {
          int reply;
          fprintf( stderr, "%s from %p\n", "World: receive go", &sayWorld );
          sayWorld >> reply;
          fprintf( stderr, "%s from %p\n", "World: received go", &sayWorld );
          if ( reply == kQuit )
            break;
          std::cout << "world!\n";
          fprintf( stderr, "%s to %p\n", "World: say go", &sayHello );
          sayHello << kGo;
          fprintf( stderr, "%s to %p\n", "World: said go", &sayHello );
        }
        quitter << kDone;
      } );

  int a;
  quitter >> a;

  return 0;
}
