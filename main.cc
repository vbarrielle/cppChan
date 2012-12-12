#include <iostream>
#include <string>

#include "channel.hh"

static const int kGo = 0;
static const int kQuit = 1;
static const int kDone = 2;

int
main( int argc, char ** argv )
{
  channel<int> sayHello, sayWorld, quitter;

  std::async( std::launch::async, [&]
      {
        for ( int i = 0; i < 1000; ++i )
        {
          std::cerr << "Hello ";
          sayWorld << kGo;
          int a;
          sayHello >> a;
        }
        sayWorld << kQuit;
      } );

  std::async( std::launch::async, [&]
      {
        while ( true )
        {
          int reply;
          sayWorld >> reply;
          if ( reply == kQuit )
            break;
            std::cerr << "world!";
          sayHello << kGo;
        }
        quitter << kDone;
      } );

  int a;
  quitter >> a;

  return 0;
}
