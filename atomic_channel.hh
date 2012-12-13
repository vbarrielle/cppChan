#ifndef CPPCHAN_ATOMIC_CHANNEL_H
#define CPPCHAN_ATOMIC_CHANNEL_H

#include <atomic>
#include <stdio.h>

template <typename T>
class channel
{
public:
  channel( int64_t capacity = 0, bool log = false );
  ~channel( );

  void operator<<( const T & val );
  void operator>>( T& retVal);

private:
  T * m_data;
  const int64_t m_capacity;
  const int64_t m_dataSize;
  bool   m_log;
  std::atomic<int64_t> m_counter;
};

static const int64_t kOneZero = 0x0000000100000000;
static const int64_t kZeroOne = 1;


template <typename T>
channel<T>::channel( int64_t capacity, bool log ) :
  m_capacity( capacity ),
  m_dataSize( capacity + 1 ),
  m_log( log ),
  m_counter( 0 )
{
  if ( m_capacity )
    m_data = new T[m_dataSize];
}

template <typename T>
channel<T>::~channel( )
{
  delete[] m_data;
}

int64_t queueSize( int64_t doubleInt, int64_t & doubleIntOut )
{
  doubleIntOut = doubleInt;
  int64_t leftInt = doubleInt >> 32;
  int64_t rightInt = doubleInt & 0x00000000ffffffff;
  return rightInt - leftInt;
}

template <typename T>
void
channel<T>::operator<<( const T & val )
{
  int64_t counter;
  while ( queueSize( m_counter.fetch_add( kZeroOne ), counter ) >= m_capacity )
    m_counter.fetch_sub( kZeroOne );

  int64_t right = counter & 0x00000000ffffffff;

  m_data[right%(m_dataSize)] = val;
}

template <typename T>
void channel<T>::operator>>( T & retVal )
{
  int64_t counter;
  while ( queueSize( m_counter.fetch_add( kOneZero ), counter ) < 1 )
    m_counter.fetch_sub( kOneZero );

  int64_t left = counter >> 32;

  retVal = m_data[left%(m_dataSize)];
}


#endif // CPPCHAN_ATOMIC_CHANNEL_H

