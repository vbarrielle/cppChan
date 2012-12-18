#include <iostream>
#include <string>
#include <future>

#include "channel.hh"

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

  // test channel of capacity > 1

  channel<int> chan( 6 );
  for ( int i = 0; i < 6; ++i )
    chan << i;

  for ( int i = 0; i < 6; ++i )
  {
    int k;
    chan >> k;
    std::cerr << k << " ";
  }
  std::cerr << "\n";
    
  return 0;
}
