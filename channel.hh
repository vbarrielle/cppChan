#ifndef CPPCHAN_ATOMIC_CHANNEL_H
#define CPPCHAN_ATOMIC_CHANNEL_H

#include <atomic>
#include <chrono>

template <typename T>
class channel
{
public:
  channel( int64_t capacity = 0 );
  ~channel( );
  void operator<<( const T & val ); //< send data to the channel
  void operator>>( T& retVal); //< receive data from the channel

private:
  T * m_data;
  const int64_t m_capacity;
  const int64_t m_dataSize;
  std::atomic<int64_t> m_counterLeft;
  std::atomic<int64_t> m_counterRight;
};

template <typename T>
channel<T>::channel( int64_t capacity ) :
  m_capacity( capacity ),
  m_dataSize( capacity + 1 ),
  m_counterLeft( 0 ),
  m_counterRight( 0 )
{
  if ( m_dataSize )
    m_data = new T[m_dataSize];
}

template <typename T>
channel<T>::~channel( )
{
  delete[] m_data;
}

template <typename T>
void
channel<T>::operator<<( const T & val )
{
  int64_t counterRight = m_counterRight.fetch_add( 1 );
  while ( counterRight - m_counterLeft.load( ) >= m_capacity )
  {
    std::this_thread::yield( );
  }

  m_data[counterRight%(m_dataSize)] = val;
}

template <typename T>
void channel<T>::operator>>( T & retVal )
{
  int64_t counterLeft = m_counterLeft.fetch_add( 1 );
  while ( m_counterRight.load( ) - counterLeft < 1 )
  {
    std::this_thread::yield( );
  }

  retVal = m_data[counterLeft%(m_dataSize)];
}


#endif // CPPCHAN_ATOMIC_CHANNEL_H

