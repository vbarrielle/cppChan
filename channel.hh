#ifndef CPPCHAN_CHANNEL_H
#define CPPCHAN_CHANNEL_H

#include <queue>
#include <future>
#include <mutex>

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
  std::cerr << this << " entering setter\n";
  if ( m_capacity == 0 )
  {
    std::cerr << this << " sync set\n";
    sync_set( val );
    return;
  }
  std::cerr << this << " queue set\n";

  do 
  {
    bool pushed = false;
    std::cerr << this << " setter locking\n";
    m_queueMutex.lock( );
    std::cerr << this << " setter locked\n";
    if ( m_values.size( ) < (size_t) m_capacity )
    {
      m_values.push( val );
      pushed = true;
    }
    std::cerr << this << " setter unlocking\n";
    m_queueMutex.unlock( );
    std::cerr << this << " setter unlocked\n";

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
  std::cerr << this << " entering getter\n";
  if ( m_capacity == 0 )
  {
    std::cerr << this << " sync get\n";
    retVal = sync_get( );
    return;
  }

  std::cerr << this << " queue get\n";
  do 
  {
    bool pulled = false;
    T res;
    std::cerr << this << " getter locking\n";
    m_queueMutex.lock( );
    std::cerr << this << " getter locked\n";
    if ( m_values.size( ) > 0 )
    {
      res = m_values.front( );
      m_values.pop( );
      pulled  = true;
    }
    std::cerr << this << " getter unlocking\n";
    m_queueMutex.unlock( );
    std::cerr << this << " getter unlocked\n";

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

void
wait( std::atomic_bool & blocked, std::promise<void> & promise )
{
  std::cerr << "wait: atomic store begin\n";
  std::cerr << "wait: atomic value: " << blocked.load( ) << "\n";
  blocked.store( true );
  std::cerr << "wait: atomic value: " << blocked.load( ) << "\n";
  std::cerr << "wait: atomic store end\n";
  auto resFuture = promise.get_future( );
  std::cerr << "wait: future get\n";
  resFuture.get( );
  std::cerr << "wait: future retrieve\n";
  std::promise<void> newPromise;
  std::swap( promise, newPromise );
}

void
unblock( std::atomic_bool & blocked, std::promise<void> & promise )
{
  bool trueVal = true;
  std::cerr << "unblock: atomic value: " << blocked.load( ) << "\n";
  if ( blocked.compare_exchange_strong( trueVal, false ) )
  {
    promise.set_value( );
    std::cerr << "unblock: was blocked\n";
  }
}

template <typename T>
bool
channel<T>::waitForReader( )
{
  std::cerr << this << " waiting for a reader\n";
  wait( m_blockedWritter, m_setsValuePromise );
  std::cerr << this << " found a reader\n";
  return true;
}


template <typename T>
bool
channel<T>::waitForWritter( )
{
  std::cerr << this << " waiting for a writter\n";
  wait( m_blockedReader, m_wantsValuePromise );
  std::cerr << this << " found a writter\n";
  return true;
}


template <typename T>
void
channel<T>::unblockWritter( )
{
  std::cerr << this << " unblocking a writter\n";
  unblock( m_blockedWritter, m_setsValuePromise );
  std::cerr << this << " unblocked a writter\n";
}

template <typename T>
void
channel<T>::unblockReader( )
{
  std::cerr << this << " unblocking a reader\n";
  unblock( m_blockedReader, m_wantsValuePromise );
  std::cerr << this << " unblocked a reader\n";
}

#endif // CPPCHAN_CHANNEL_H
