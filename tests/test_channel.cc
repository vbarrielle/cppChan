#include <gtest/gtest.h>
#include <future>
#include "../channel.hh"


TEST( test_channel, preserve_order )
{
  for ( int i = 1; i < 100; ++i )
  {
    channel<int> chan( i );
    for ( int j = 0; j < i; ++j )
      chan << j;
    for ( int j = 0; j < i; ++j )
    {
      int res;
      chan >> res;
      ASSERT_EQ( res, j ) << "channel of capacity "
         << i << " does not preserve order";
    }
  }
}

TEST( test_channel, blocks_when_full )
{
  int lastLoop = 0;
  static const int loopCount = 100;
  for ( int i = 0; i < loopCount; ++i )
  {
    channel<int> chan( i );
    auto a = std::async( std::launch::async, [&]
        {
          for ( int j = 0; j < i+1; ++j )
          {
            chan << j;
          }
          return 0;
        } );
    auto status = a.wait_for( std::chrono::milliseconds( 10 ) );
    bool ok = status != std::future_status::ready;

    // now unlock the blocked thread (done before checking the result,
    // to enable correct destruction of a if the test fails
    int res;
    for ( int j = 0; j < i+1; ++j )
      chan >> res;
    auto get = a.get( );

    if ( !ok )
      break;
    lastLoop = i;
  }
  ASSERT_TRUE( lastLoop == loopCount-1 ) << "The channel of size " << lastLoop
    << " is supposed to be full and to block";
}

TEST( test_channel, blocks_when_empty )
{
  static const int loopCount = 100;
  int lastLoop = 0;
  for ( int i = 0; i < 100; ++i )
  {
    channel<int> chan( i );
    for ( int j = 0; j < i; ++j )
      chan << j;
    auto b = std::async( std::launch::async, [&]
        {
          for ( int j = 0; j < i+1; ++j )
          {
            int res;
            chan >> res;
          }
          return 0;
        } );
    auto status = b.wait_for( std::chrono::milliseconds( 10 ) );
    bool ok = status == std::future_status::timeout;

    // unblock thread
    chan << 0;
    auto get = b.get( );

    if ( !ok )
      break;
    lastLoop = i;
  }
  ASSERT_TRUE( lastLoop == loopCount-1 ) << "The channel of size " << lastLoop
    << " is supposed to be empty and to block";
}

int
main( int argc, char ** argv )
{
  ::testing::InitGoogleTest( &argc, argv );
  return RUN_ALL_TESTS( );
}
