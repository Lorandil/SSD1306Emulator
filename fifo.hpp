#include <Arduino.h>

// FIFO size is required to be a power of 2.
// Usable size is FIFO size - 1.
// FIFO read is blocking if the FIFO is empty.
// FIFO write is blocking if the FIFO is full.
constexpr uint16_t FIFO_SIZE = 256;

constexpr bool BLOCKING_FIFO_WRITE = false;
constexpr bool BLOCKING_FIFO_READ = true;

class SimpleFIFO
{
public:
  SimpleFIFO() {}

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
    bytesWritten++;
  }

  uint8_t readByte()
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
    // fetch next byte
    uint8_t value = buffer[tail];
    tail = ++tail & ( FIFO_SIZE - 1 );
    bytesRead++;
    return( value );
  }

public:
  void     clear() { head = 0; tail = 0; bytesWritten = 0; bytesRead = 0; overflowCount = 0; underflowCount = 0; }
  bool     isEmpty() { return( head == tail ); }
  bool     isFull() { return( ( ( head + 1 ) & ( FIFO_SIZE - 1 ) ) == tail ); }
  bool     isOverflow() { return( overflowCount != 0 ); }
  bool     isUnderflow() { return( underflowCount != 0 ); }
  uint16_t size() { return( FIFO_SIZE - 1 ); }
  uint16_t freeCount() { return( size() - fillCount() ); }
  uint16_t fillCount() { return( ( head - tail )  & ( FIFO_SIZE - 1 ) ); }
  uint32_t getBytesWritten() { return( bytesWritten ); }
  uint32_t getBytesRead() { return( bytesRead ); }
  uint32_t getOverflowCount() { return( overflowCount ); }
  uint32_t getUnderflowCount() { return( underflowCount ); }

protected:
  uint16_t head{};
  uint16_t tail{};
  uint32_t bytesWritten{};
  uint32_t bytesRead{};
  uint32_t overflowCount{};
  uint32_t underflowCount{};
  uint8_t buffer[FIFO_SIZE];
};