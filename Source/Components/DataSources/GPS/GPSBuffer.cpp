#include "GPSBuffer.h"

#include <iostream>

using namespace MARTe;

namespace MFI {

GPSBuffer::GPSBuffer(uint32 size) {
    start_ = new uint8[size];
    end_ = start_ + size;
    head_ = tail_ = start_;
    size_ = size;
    count_ = 0;
}

GPSBuffer::~GPSBuffer() {
    delete [] start_;
}

uint32 GPSBuffer::size() const {
    return size_;
}

uint32 GPSBuffer::count() const {
    return count_;
}

uint32 GPSBuffer::available() const {
    return size_ - count_;
}

uint32 GPSBuffer::queue(const uint8* buffer, uint32 size) {
    if ((count_ + size) > size_) {
        size = size_ - count_;
    }

    for (uint32 i = 0; i < size; i++) {
        if (tail_ > end_) {
            tail_ = start_;
        }
        *tail_ = buffer[i];
        tail_++;
    }
    count_ += size;

    return size;
}

uint32 GPSBuffer::dequeue(uint8* buffer, uint32 size) {
    if (size > count_) {
        size = count_;
    }

    for (uint32 i = 0; i < size; i++) {
        if (head_ > end_) {
            head_ = start_;
        }
        buffer[i] = *head_;
        head_++;
    }
    count_ -= size;

    return size;
}

void GPSBuffer::empty() {
    count_ = 0u;
    head_ = tail_ = start_;
}

void GPSBuffer::empty(uint32 n) {
    if (n > count_) {
        n = count_;
    }
    
    for (uint32 i = 0; i < n; i++) {
        if (head_ > end_) {
            head_ = start_;
        }

        head_++;
        count_--;
    }
}

bool GPSBuffer::find(uint8 value, uint32& index, uint32 start) {
    if (start >= count_) {
        return false;
    }
    
    uint8* ptr = head_ + start;
    for (uint32 i = start; i < count_; i++) {
        if (ptr > end_) {
            ptr = start_ + (ptr - end_);
        }
        
        if (*ptr == value) {
            index = i;
            
            return true;
        }

        ptr++;
    }

    return false;
}

bool GPSBuffer::at(uint32 index, uint8& value) {   
    if (index < count_) {
        uint8* ptr = head_;

        for (uint32 i = 0; i < index; i++) {
            if (ptr > end_) {
                ptr = start_;
            }

            ptr++;
        }

        value = *ptr;
            
        return true;
    }

    return false;
}

void print_buffer(GPSBuffer& buffer) {
    uint8 value;
    for (uint32 i = 0; i < buffer.count(); i++) {
        buffer.at(i, value);
        std::cout << std::hex << static_cast<unsigned short int>(value);
        std::cout << " ";
    }
    std::cout << std::endl;
}

} // namespace MFI