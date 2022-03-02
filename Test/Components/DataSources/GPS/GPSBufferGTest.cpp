#include "GPSBuffer.h"
#include "CompilerTypes.h"

#include "gtest/gtest.h"

using namespace MFI;
using namespace MARTe;

namespace gps_test {

bool buffers_equal(uint8* buffer1, uint8* buffer2, uint32 size) {
    for (uint32 i = 0; i < size; i++) {
        if (buffer1[i] != buffer2[i]) {
            return false;
        }
    }

    return true;
}

}

TEST(GPSBufferSize, IsZero) {
    GPSBuffer buffer(0u);

    ASSERT_EQ(buffer.size(), 0u);
}

TEST(GPSBufferSize, IsNonZero) {
    GPSBuffer buffer(1u);

    ASSERT_EQ(buffer.size(), 1u);
}

TEST(GPSBufferCount, IsInitiallyZero) {
    GPSBuffer buffer(1u);

    ASSERT_EQ(buffer.count(), 0u);
}

TEST(GPSBufferCount, IncreasesWhenCharsAppendedFromEmpty) {
    GPSBuffer buffer(10u);
    uint8 message[5] = {0x00, 0x01, 0x02, 0x03, 0x04};

    buffer.queue(&message[0], 5u);

    ASSERT_EQ(buffer.count(), 5u);
}

TEST(GPSBufferCount, IncreasesWhenCharsAppendedFromNonEmpty) {
    GPSBuffer buffer(10u);
    uint8 message[5] = {0x00, 0x01, 0x02, 0x03, 0x04};
    buffer.queue(&message[0], 5u);

    buffer.queue(&message[0], 3u);

    ASSERT_EQ(buffer.count(), 8u);
}

TEST(GPSBufferCount, DoesNotExceedSize) {
    GPSBuffer buffer(4);
    uint8 message[5] = {0x00, 0x01, 0x02, 0x03, 0x04};
    
    buffer.queue(&message[0], 5u);

    ASSERT_EQ(buffer.count(), 4u);
}

TEST(GPSBufferCount, ResetsToZeroOnEmpty) {
    GPSBuffer buffer(5);
    uint8 message[5] = {0x00, 0x01, 0x02, 0x03, 0x04};
    buffer.queue(&message[0], 5u);

    buffer.empty();

    ASSERT_EQ(buffer.count(), 0u);
}

TEST(GPSBufferAvailable, IsInitiallyEqualToSize) {
    GPSBuffer buffer(5u);

    ASSERT_EQ(buffer.size(), buffer.available());
}

TEST(GPSBufferAvailable, DecreasesByOne) {
    GPSBuffer buffer(5u);
    uint8 message = 0x00;

    buffer.queue(&message, 1u);

    ASSERT_EQ(buffer.size() - 1, buffer.available());
}

TEST(GPSBufferAvailable, DecreasesToZero) {
    GPSBuffer buffer(5u);
    uint8 message[5] = {0x00, 0x00, 0x00, 0x00, 0x00};

    buffer.queue(&message[0], 5u);

    ASSERT_EQ(0u, buffer.available());
}

TEST(GPSBufferQueue, AppendsBytesWhenEmpty) {
    GPSBuffer buffer(5);
    uint8 bytes_in[5] = {0x00, 0x01, 0x02, 0x03, 0x04};
    uint8 bytes_out[5] = {0x00, 0x00, 0x00, 0x00, 0x00};

    buffer.queue(&bytes_in[0], 5u);
    buffer.dequeue(&bytes_out[0], 5u);

    ASSERT_TRUE(gps_test::buffers_equal(bytes_in, bytes_out, 5u));
}

TEST(GPSBufferQueue, AppendsBytesWhenNonEmpty) {
    GPSBuffer buffer(10);
    uint8 bytes_in[5] = {0x00, 0x01, 0x02, 0x03, 0x04};
    buffer.queue(&bytes_in[0], 5u);

    buffer.queue(&bytes_in[0], 5u);

    uint8 bytes_out[10];
    buffer.dequeue(&bytes_out[0], 10u);
    ASSERT_TRUE(gps_test::buffers_equal(&bytes_out[0], bytes_in, 5u) && gps_test::buffers_equal(&bytes_out[5], bytes_in, 5u));
}

TEST(GPSBufferQueue, AppendsSomeOfMessageOnAvailability) {
    GPSBuffer buffer(7);
    uint8 bytes_in[5] = {0x00, 0x01, 0x02, 0x03, 0x04};
    buffer.queue(&bytes_in[0], 5u);

    uint32 n = buffer.queue(&bytes_in[0], 5u);

    ASSERT_EQ(n, 2u);
}

TEST(GPSBufferQueue, CanDequeueAPartialAppend) {
    GPSBuffer buffer(7);
    uint8 bytes_in[5] = {0x00, 0x01, 0x02, 0x03, 0x04};
    buffer.queue(&bytes_in[0], 5u);

    buffer.queue(&bytes_in[0], 5u);
    uint8 bytes_out[7u];
    buffer.dequeue(&bytes_out[0], 7u);

    ASSERT_TRUE(gps_test::buffers_equal(&bytes_out[0], bytes_in, 5u) && gps_test::buffers_equal(&bytes_out[5], bytes_in, 2u));
}

TEST(GPSBufferQueue, ReturnsNumberBytesAppended) {
    GPSBuffer buffer(5);
    uint8 message[5] = {0x00, 0x01, 0x02, 0x03, 0x04};
    
    uint32 n = buffer.queue(&message[0], 5u);

    ASSERT_EQ(n, 5u);
}

TEST(GPSBufferQueue, AppendedBytesMatchesAvailableSpace) {
    GPSBuffer buffer(4);
    uint8 message[5] = {0x00, 0x01, 0x02, 0x03, 0x04};
    
    uint32 n = buffer.queue(&message[0], 5u);

    ASSERT_EQ(n, 4u);
}

TEST(GPSBufferFind, ReturnsFalseIfValueNotFound) {
    GPSBuffer buffer(5);
    uint8 message[5] = {0x00, 0x01, 0x02, 0x03, 0x04};
    buffer.queue(&message[0], 5u);

    uint32 index;
    bool found = buffer.find(0x05, index);

    ASSERT_FALSE(found);
}

TEST(GPSBufferFind, ReturnsTrueIfValueFound) {
    GPSBuffer buffer(5);
    uint8 message[5] = {0x00, 0x01, 0x02, 0x03, 0x04};
    buffer.queue(&message[0], 5u);

    uint32 index;
    bool found = buffer.find(0x02, index);

    ASSERT_TRUE(found);
}

TEST(GPSBufferFind, ReturnsIndexIfValueFound) {
    GPSBuffer buffer(5);
    uint8 message[5] = {0x00, 0x01, 0x02, 0x03, 0x04};
    buffer.queue(&message[0], 5u);

    uint32 index;
    bool found = buffer.find(0x02, index);

    ASSERT_EQ(index, 2u);
}

TEST(GPSBufferFind, ReturnsFalseIfStartInvalid) {
    GPSBuffer buffer(5);
    uint8 message[5] = {0x00, 0x01, 0x02, 0x03, 0x04};
    buffer.queue(&message[0], 5u);

    uint32 index;
    bool found = buffer.find(0x02, index, 5);

    ASSERT_FALSE(found);
}

TEST(GPSBufferFind, SearchesOnlyAfterStart) {
    GPSBuffer buffer(5);
    uint8 message[5] = {0x00, 0x01, 0x02, 0x03, 0x04};
    buffer.queue(&message[0], 5u);

    uint32 index;
    bool found = buffer.find(0x01, index, 2);

    ASSERT_FALSE(found);
}

TEST(GPSBufferEmpty, LeavesAllCharactersInBuffer) {
    GPSBuffer buffer(5);
    uint8 message[5] = {0x00, 0x01, 0x02, 0x03, 0x04};
    buffer.queue(&message[0], 5u);

    buffer.empty(0);
    uint8 message_out[5];
    buffer.dequeue(message_out, 5);

    ASSERT_TRUE(gps_test::buffers_equal(&message_out[0], &message[0], 5u));
}

TEST(GPSBufferEmpty, LeavesSomeCharactersInBuffer) {
    GPSBuffer buffer(5);
    uint8 message[5] = {0x00, 0x01, 0x02, 0x03, 0x04};
    buffer.queue(&message[0], 5u);

    buffer.empty(2);
    uint8 message_out[5];
    buffer.dequeue(message_out, 5);

    ASSERT_TRUE(gps_test::buffers_equal(&message_out[0], &message[2], 3u));
}

TEST(GPSBufferEmpty, LeavesNoCharactersInBuffer) {
    GPSBuffer buffer(5);
    uint8 message[5] = {0x00, 0x01, 0x02, 0x03, 0x04};
    buffer.queue(&message[0], 5u);

    buffer.empty(5);
    uint8 message_out[5];
    ASSERT_EQ(buffer.dequeue(message_out, 5), 0u);
}

TEST(GPSBufferAt, ReturnsTrueIfIndexValid) {
    GPSBuffer buffer(5);
    uint8 message[5] = {0x00, 0x01, 0x02, 0x03, 0x04};
    buffer.queue(&message[0], 5u);

    uint8 value;
    bool ret = buffer.at(0, value);

    ASSERT_TRUE(ret);
}

TEST(GPSBufferAt, ReturnsValueIfIndexValidStart) {
    GPSBuffer buffer(5);
    uint8 message[5] = {0x00, 0x01, 0x02, 0x03, 0x04};
    buffer.queue(&message[0], 5u);

    uint8 value = 0x01;
    bool ret = buffer.at(0, value);

    ASSERT_EQ(0x00, value);
}

TEST(GPSBufferAt, ReturnsValueIfIndexValidEnd) {
    GPSBuffer buffer(5);
    uint8 message[5] = {0x00, 0x01, 0x02, 0x03, 0x04};
    buffer.queue(&message[0], 5u);

    uint8 value = 0x01;
    bool ret = buffer.at(4, value);

    ASSERT_EQ(0x04, value);
}

TEST(GPSBufferAt, ReturnsFalseIfIndexInvalid) {
    GPSBuffer buffer(5);
    uint8 message[5] = {0x00, 0x01, 0x02, 0x03, 0x04};
    buffer.queue(&message[0], 5u);

    uint8 value;
    bool ret = buffer.at(5, value);

    ASSERT_FALSE(ret);
}

TEST(GPSBufferDequeue, ReducesBufferCount) {
    GPSBuffer buffer(5);
    uint8 message[5] = {0x00, 0x01, 0x02, 0x03, 0x04};
    buffer.queue(&message[0], 5u);

    uint8 bytes_out[3u];
    buffer.dequeue(&bytes_out[0], 3u);

    ASSERT_EQ(2u, buffer.count());
}

TEST(GPSBufferDequeue, ReducesBufferCountToZero) {
    GPSBuffer buffer(5);
    uint8 message[5] = {0x00, 0x01, 0x02, 0x03, 0x04};
    buffer.queue(&message[0], 5u);

    uint8 bytes_out[5u];
    buffer.dequeue(&bytes_out[0], 5u);

    ASSERT_EQ(0u, buffer.count());
}

TEST(GPSBufferDequeue, DoesNotReduceBufferCountIfZero) {
    GPSBuffer buffer(5);
    uint8 message[5] = {0x00, 0x01, 0x02, 0x03, 0x04};
    buffer.queue(&message[0], 5u);

    uint8 bytes_out[5u];
    buffer.dequeue(&bytes_out[0], 0u);

    ASSERT_EQ(5u, buffer.count());
}
