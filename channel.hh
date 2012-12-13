#ifndef CPPCHAN_CHANNEL_H
#define CPPCHAN_CHANNEL_H

#include <queue>
#include <future>
#include <mutex>
#include <stdio.h>

template <typename T>
class channel
{
public:
  channel( int capacity = 0 );

  void operator<<( const T & val );
  void operator>>( T& retVal);

private:
  void sync_set( const T & val );
  T sync_get( );
  bool waitForReader( );
  bool waitForWritter( );
  void unblockReader( );
  void unblockWritter( );
  void unblock( std::atomic_bool & blocked, std::promise<void> & promise );
  void wait( std::atomic_bool & blocked, std::promise<void> & promise );

private:
  int m_capacity;
  std::atomic_bool m_blockedReader;
  std::atomic_bool m_blockedWritter;
  std::queue<T> m_values;
  std::promise<T>    m_uniqueValue;
  std::promise<void> m_wantsValuePromise;
  std::promise<void> m_setsValuePromise;
  std::mutex m_queueMutex;
};


template <typename T>
channel<T>::channel( int capacity ) :
  m_capacity( capacity ),
  m_blockedReader( false ),
  m_blockedWritter( false )
{ }

template <typename T>
void
channel<T>::operator<<( const T & val )
{
  if ( m_capacity == 0 )
  {
    sync_set( val );
    return;
  }

  do 
  {
    bool pushed = false;
    m_queueMutex.lock( );
    if ( m_values.size( ) < (size_t) m_capacity )
    {
      m_values.push( val );
      pushed = true;
    }
    m_queueMutex.unlock( );

    if ( pushed )
    {
      unblockReader( );
      return;
    }
  } while( waitForReader( ) );

}

template <typename T>
void channel<T>::operator>>( T & retVal )
{
  if ( m_capacity == 0 )
  {
    retVal = sync_get( );
    return;
  }

  do 
  {
    T res;
    bool pulled = false;
    m_queueMutex.lock( );
    if ( m_values.size( ) > 0 )
    {
      res = m_values.front( );
      m_values.pop( );
      pulled  = true;
    }
    m_queueMutex.unlock( );

    if ( pulled )
    {
      retVal = res;
      unblockWritter( );
      return;
    }
  } while ( waitForWritter( ) );
}

template <typename T>
void
channel<T>::sync_set( const T & val )
{
  // wait until there is a reader
  auto wantsValueFuture = m_wantsValuePromise.get_future( );
  wantsValueFuture.get( );

  m_uniqueValue.set_value( val );

  // reset the reader/writter sync
  std::promise<void> newPromise;
  std::swap( m_wantsValuePromise, newPromise );
}

template <typename T>
T
channel<T>::sync_get( )
{
  // inform we are reading
  m_wantsValuePromise.set_value( );

  auto resFuture = m_uniqueValue.get_future( );
  T res = resFuture.get( );

  // reset the unique value store
  std::promise<T> newPromise;
  std::swap( m_uniqueValue, newPromise );

  return res;
}

template <typename T>
void
channel<T>::wait( std::atomic_bool & blocked, std::promise<void> & promise )
{
  bool falseVal = false;
  if ( blocked.compare_exchange_strong( falseVal, true  ) )
  {
    auto resFuture = promise.get_future( );
    fprintf( stderr, "%p %s\n", this, " wait: future get");
    resFuture.get( );
    fprintf( stderr, "%p %s\n", this, " wait: future retrieved");
    std::promise<void> newPromise;
    std::swap( promise, newPromise );
  }
}

template <typename T>
void
channel<T>::unblock( std::atomic_bool & blocked, std::promise<void> & promise )
{
  bool trueVal = true;
  if ( blocked.compare_exchange_strong( trueVal, false ) )
  {
    promise.set_value( ); // FIXME: nothing guarantees the other thread
                          // won't try to read before this set
    fprintf( stderr, "%p %s\n", this, " unblock: was blocked");
    return;
  }
  fprintf( stderr, "%p %s\n", this, " unblock: did not unblock");
}

template <typename T>
bool
channel<T>::waitForReader( )
{
  fprintf( stderr, "%p %s\n", this, " waiting for a reader");
  wait( m_blockedWritter, m_setsValuePromise );
  fprintf( stderr, "%p %s\n", this, " found a reader");
  return true;
}


template <typename T>
bool
channel<T>::waitForWritter( )
{
  fprintf( stderr, "%p %s\n", this, " waiting for a writter");
  wait( m_blockedReader, m_wantsValuePromise );
  fprintf( stderr, "%p %s\n", this, " found a writter");
  return true;
}


template <typename T>
void
channel<T>::unblockWritter( )
{
  fprintf( stderr, "%p %s\n", this, " unblocking a writter");
  unblock( m_blockedWritter, m_setsValuePromise );
}

template <typename T>
void
channel<T>::unblockReader( )
{
  fprintf( stderr, "%p %s\n", this, " unblocking a reader");
  unblock( m_blockedReader, m_wantsValuePromise );
}

#endif // CPPCHAN_CHANNEL_H
