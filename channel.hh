/*
* Copyright (c) 2012 Vincent Barrielle
* 
* Permission is hereby granted, free of charge, to any person obtaining a copy of
* this software and associated documentation files (the "Software"), to deal in
* the Software without restriction, including without limitation the rights to
* use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
* of the Software, and to permit persons to whom the Software is furnished to do
* so, subject to the following conditions:
* 
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
* 
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/

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

