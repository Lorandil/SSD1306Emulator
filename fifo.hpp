#include <Arduino.h>

// FIFO size is required to be a power of 2, the capacity is FIFO size - 1.
constexpr uint16_t FIFO_SIZE = 2048;
// FIFO read is blocking if the FIFO is empty.
constexpr bool BLOCKING_FIFO_READ = true;
// FIFO write is *non* blocking if the FIFO is full.
constexpr bool BLOCKING_FIFO_WRITE = false;

template<class T>
class SimpleFIFO
{
public:
  SimpleFIFO() { head = 0; tail = 0; }

  void writeValue( T value )
  {
    if ( BLOCKING_FIFO_WRITE )
    {
      // block until at least one byte is free
      while( isFull() );
    }
    else
    {
      if ( isFull() )
      { 
        // just detect the overflow, but don't write into the buffer
        // otherwise all data in the buffer is lost
        overflowCount++; 
        return;
      }
    }
    // store in the free value
    buffer[head] = value;
    // increment head pointer
    head = ++head & ( FIFO_SIZE - 1 );
    valuesWritten++;
  }

  T readValue()
  {
    if ( BLOCKING_FIFO_READ )
    {
      // block until data is available
      while( isEmpty() );
    }
    else
    {
      if ( isEmpty() )
      { 
        // just detect the underflow, but don't read from the buffer
        // otherwise garbage data is created. Just return '0' and assume
        // that the caller checks for underflow
        underflowCount++;
        return( 0 );
      }
    }
    // fetch next value
    auto value = buffer[tail];
    tail = ++tail & ( FIFO_SIZE - 1 );
    valuesRead++;
    
    return( value );
  }

public:
  void     clear() { head = 0; tail = 0; valuesWritten = 0; valuesRead = 0; overflowCount = 0; underflowCount = 0; }
  bool     isEmpty() { return( head == tail ); }
  bool     isFull() { return( ( ( head + 1 ) & ( FIFO_SIZE - 1 ) ) == tail ); }
  bool     isOverflow() { return( overflowCount != 0 ); }
  bool     isUnderflow() { return( underflowCount != 0 ); }
  uint16_t size() { return( FIFO_SIZE - 1 ); }
  uint16_t freeCount() { return( size() - fillCount() ); }
  uint16_t fillCount() { return( ( head - tail )  & ( FIFO_SIZE - 1 ) ); }
  uint32_t getvaluesWritten() { return( valuesWritten ); }
  uint32_t getvaluesRead() { return( valuesRead ); }
  uint32_t getOverflowCount() { return( overflowCount ); }
  uint32_t getUnderflowCount() { return( underflowCount ); }

protected:
  volatile uint16_t head{};
  volatile uint16_t tail{};
  volatile uint32_t valuesWritten{};
  volatile uint32_t valuesRead{};
  volatile uint32_t overflowCount{};
  volatile uint32_t underflowCount{};
  volatile T buffer[FIFO_SIZE];
};

#if 0
class SimpleFIFO
{
public:
  SimpleFIFO() { head = 0; tail = 0; }

  void writeByte( uint8_t value )
  {
    if ( BLOCKING_FIFO_WRITE )
    {
      // block until at least one byte is free
      while( isFull() );
    }
    else
    {
      if ( isFull() )
      { 
        // just detect the overflow, but don't write into the buffer
        // otherwise all data in the buffer is lost
        overflowCount++; 
        return;
      }
    }
    // store in the free byte
    buffer[head] = value;
    // increment head pointer
    head = ++head & ( FIFO_SIZE - 1 );
    valuesWritten++;
  }

  uint8_t readValue()
  {
    if ( BLOCKING_FIFO_READ )
    {
      // block until data is available
      while( isEmpty() );
    }
    else
    {
      if ( isEmpty() )
      { 
        // just detect the underflow, but don't read from the buffer
        // otherwise garbage data is created. Just return '0' and assume
        // that the caller checks for underflow
        underflowCount++;
        return( 0 );
      }
    }
    // fetch next value
    uint8_t value = buffer[tail];
    tail = ++tail & ( FIFO_SIZE - 1 );
    valuesRead++;
    
    return( value );
  }

public:
  void     clear() { head = 0; tail = 0; valuesWritten = 0; valuesRead = 0; overflowCount = 0; underflowCount = 0; }
  bool     isEmpty() { return( head == tail ); }
  bool     isFull() { return( ( ( head + 1 ) & ( FIFO_SIZE - 1 ) ) == tail ); }
  bool     isOverflow() { return( overflowCount != 0 ); }
  bool     isUnderflow() { return( underflowCount != 0 ); }
  uint16_t size() { return( FIFO_SIZE - 1 ); }
  uint16_t freeCount() { return( size() - fillCount() ); }
  uint16_t fillCount() { return( ( head - tail )  & ( FIFO_SIZE - 1 ) ); }
  uint32_t getvaluesWritten() { return( valuesWritten ); }
  uint32_t getvaluesRead() { return( valuesRead ); }
  uint32_t getOverflowCount() { return( overflowCount ); }
  uint32_t getUnderflowCount() { return( underflowCount ); }

protected:
  volatile uint16_t head{};
  volatile uint16_t tail{};
  volatile uint32_t valuesWritten{};
  volatile uint32_t valuesRead{};
  volatile uint32_t overflowCount{};
  volatile uint32_t underflowCount{};
  volatile uint8_t buffer[FIFO_SIZE];
};
#endif