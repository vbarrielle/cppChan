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

private:
  int m_capacity;
  std::queue<T> m_values;
  std::promise<T>    m_uniqueValue;
  std::promise<void> m_wantsValuePromise;
  std::mutex m_queueMutex;
};


template <typename T>
channel<T>::channel( int capacity ) :
  m_capacity( capacity )
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

  bool pushed = false;
  m_queueMutex.lock( );
  if ( m_values.size( ) <= (size_t) m_capacity )
  {
    m_values.push( val );
    pushed  = true;
  }
  m_queueMutex.unlock( );

  if ( pushed )
    return;

  sync_set( val );
}

template <typename T>
void channel<T>::operator>>( T & retVal )
{
  if ( m_capacity == 0 )
  {
    retVal = sync_get( );
    return;
  }

  bool pulled = false;
  T res;
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
    return;
  }

  retVal = sync_get( );
  return;
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

#endif // CPPCHAN_CHANNEL_H
