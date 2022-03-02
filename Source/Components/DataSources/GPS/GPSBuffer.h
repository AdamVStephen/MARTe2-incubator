#ifndef MFI_GPS_BUFFER_H_
#define MFI_GPS_BUFFER_H_

#include "CompilerTypes.h"

namespace MFI {

/**
 * @brief Buffer for storing raw GPS message bytes
 * 
 * @details Provides a fixed-size, FIFO, circular byte buffer, with some useful utility functions for 
 * managing the space.
 */
class GPSBuffer {
 public:
    
    GPSBuffer(MARTe::uint32 size);
    ~GPSBuffer();

    /**
     * @brief Get the total size of the buffer
     */
    MARTe::uint32 size() const;

    /**
     * @brief Get the count of existing bytes in the buffer
     */
    MARTe::uint32 count() const;
    
    /**
     * @brief Get the space remaining in the buffer
     */
    MARTe::uint32 available() const;

    /**
     * @brief Queue new characters to the tail of the buffer. Returns the actual number of bytes 
     * queued, which may be less than the number requested, depending on how much space is available
     * in the buffer
     */
    MARTe::uint32 queue(const MARTe::uint8* buffer, MARTe::uint32 size);

    /**
     * @brief Remove existing characters from the head of the buffer. Returns the actual number of bytes 
     * removed, which may be less than the number requested, depending on how many bytes are available
     * in the buffer 
     */
    MARTe::uint32 dequeue(MARTe::uint8* buffer, MARTe::uint32 size);

    /**
     * @brief Find the first occurrence of a value within the buffer, searhcing from a given location,
     * and get its index.
     * 
     * Returns true if the value was found, otherwise false
     */
    bool find(MARTe::uint8 value, MARTe::uint32& index, MARTe::uint32 start=0);

    /**
     * @brief Get the value of the byte at a given index
     * 
     * The value is obtained without removing the byte from the buffer. Return strue if the index
     * was valid, otherwise false
     */
    bool at(MARTe::uint32 index, MARTe::uint8& value);
    
    /**
     * @brief Empty the entire contents of the buffer
     * 
     * All of the buffer's contents are discarded
     */
    void empty();

    /**
     * @brief Empty a number of characters from the buffer
     * 
     * Discards the first n characters from the buffer
     */
    void empty(MARTe::uint32 n);

 private:
     /**
     * @brief Physical start of the buffer (fixed)
     */
    MARTe::uint8* start_;

    /**
     * @brief Physical end of the buffer (fixed)
     */
    MARTe::uint8* end_;

    /**
     * @brief Head of the queue (oldest byte)
     */
    MARTe::uint8* head_;

    /**
     * @brief Tail of the queue (where the first new byte will be appended)
     */
    MARTe::uint8* tail_;

    /**
     * @brief Physical size of the queue
     */
    MARTe::uint32 size_;

    /**
     * @brief Number of byets currently queued
     */
    MARTe::uint32 count_;
};

/**
 * @brief Print the contents of a buffer to a stream
 */
void print_buffer(GPSBuffer& buffer);

}

#endif // MFI_GPS_BUFFER_H_
