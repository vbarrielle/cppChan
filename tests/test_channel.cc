#include <gtest/gtest.h>
#include <future>


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
  for ( int i = 0; i < 100; ++i )
  {
    channel<int> chan( i );
    auto a = std::async( std::launch::async, [&]
        {
          for ( int j = 0; j < i; ++j )
          {
            chan << j;
          }
          return 0;
        } );
    auto b = std::async( std::launch::async, [&]
        {
          for ( int j = 0; j < i-1; ++j )
          {
            int res;
            chan >> res;
          }
        } );
    auto status = a.wait_for( std::chrono::milliseconds( 10 ) );
    ASSERT_EQ( status, std::future_status::timeout )
      << "The channel is supposed to be full and to block";
  }
}

TEST( test_channel, blocks_when_empty )
{
  for ( int i = 0; i < 100; ++i )
  {
    channel<int> chan( i );
    auto a = std::async( std::launch::async, [&]
        {
          for ( int j = 0; j < i-1; ++j )
          {
            chan << j;
          }
        } );
    auto b = std::async( std::launch::async, [&]
        {
          for ( int j = 0; j < i; ++j )
          {
            int res;
            chan >> res;
          }
          return 0;
        } );
    auto status = b.wait_for( std::chrono::milliseconds( 10 ) );
    ASSERT_EQ( status, std::future_status::timeout )
      << "The channel is supposed to be empty and to block";
  }
}
