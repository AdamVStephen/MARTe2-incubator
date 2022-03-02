#include "UBX.h"
#include "GPSBuffer.h"
#include "CompilerTypes.h"

#include "gtest/gtest.h"
#include <string.h>

using namespace MFI;
using namespace MARTe;

namespace ubx_test {

bool buffers_equal(uint8* buffer1, uint8* buffer2, uint32 size) {
    for (uint32 i = 0; i < size; i++) {
        if (buffer1[i] != buffer2[i]) {
            return false;
        }
    }

    return true;
}

bool buffer_contains(GPSBuffer& buffer, uint8* expected, uint32 expected_len) {
    if (buffer.count() != expected_len) {
        return false;
    }
    
    uint8* temp = new uint8[expected_len];
    buffer.dequeue(temp, expected_len);
    bool ret = buffers_equal(temp, expected, expected_len);
    delete [] temp;

    return ret;
}

}

class ABufferWithNoSyncPair : public ::testing::Test {
 public:
    ABufferWithNoSyncPair() : ::testing::Test(), buffer(10) {
        for (uint32 i = 0; i < 10; i++) {
            bytes_in[i] = i;
        }
        buffer.queue(bytes_in, 10);
    }

    GPSBuffer buffer;
    uint8 bytes_in[10];
};

TEST_F(ABufferWithNoSyncPair, DiscardsAllCharactersOnSanitise) {
    UBX::SanitiseBuffer(buffer);

    ASSERT_EQ(0u, buffer.count());
}

TEST_F(ABufferWithNoSyncPair, ReturnsAllCharactersDiscardedOnSanitise) {
    uint32 discarded = UBX::SanitiseBuffer(buffer);

    ASSERT_EQ(10u, discarded);
}


class ABufferWithSyncPairInMiddle : public ::testing::Test {
 public:
    ABufferWithSyncPairInMiddle() : ::testing::Test(), buffer(10) {
        for (uint32 i = 0; i < 10; i++) {
            bytes_in[i] = i;
        }
        bytes_in[4] = UBX::SYNC_CHAR_1;
        bytes_in[5] = UBX::SYNC_CHAR_2;
        buffer.queue(bytes_in, 10);
    }

    GPSBuffer buffer;
    uint8 bytes_in[10];
};

TEST_F(ABufferWithSyncPairInMiddle, DiscardsExpectedNumberOfCharactersOnSanitise) {
    UBX::SanitiseBuffer(buffer);
    
    ASSERT_TRUE(ubx_test::buffer_contains(buffer, &bytes_in[4], 6u));
}

TEST_F(ABufferWithSyncPairInMiddle, ReturnsAllCharactersDiscardedOnSanitise) {
    uint32 discarded = UBX::SanitiseBuffer(buffer);
    
    ASSERT_EQ(discarded, 4u);
}

class ABufferWithSyncPairAtEnd : public ::testing::Test {
 public:
    ABufferWithSyncPairAtEnd() : ::testing::Test(), buffer(10) {
        for (uint32 i = 0; i < 10; i++) {
            bytes_in[i] = i;
        }
        bytes_in[8] = UBX::SYNC_CHAR_1;
        bytes_in[9] = UBX::SYNC_CHAR_2;
        buffer.queue(bytes_in, 10);
    }

    GPSBuffer buffer;
    uint8 bytes_in[10];
};

TEST_F(ABufferWithSyncPairAtEnd, DiscardsExpectedNumberOfCharactersOnSanitise) {
    UBX::SanitiseBuffer(buffer);
    
    ASSERT_TRUE(ubx_test::buffer_contains(buffer, &bytes_in[8], 2u));
}

TEST_F(ABufferWithSyncPairAtEnd, ReturnsAllCharactersDiscardedOnSanitise) {
    uint32 discarded = UBX::SanitiseBuffer(buffer);
    
    ASSERT_EQ(discarded, 8u);
}

class ABufferWithSyncChar1AtStart : public ::testing::Test {
 public:
    ABufferWithSyncChar1AtStart() : ::testing::Test(), buffer(10) {
        for (uint32 i = 0; i < 10; i++) {
            bytes_in[i] = i;
        }
        bytes_in[0] = UBX::SYNC_CHAR_1;
        bytes_in[1] = 0x00;
        buffer.queue(bytes_in, 10);
    }

    GPSBuffer buffer;
    uint8 bytes_in[10];
};

TEST_F(ABufferWithSyncChar1AtStart, DiscardsAllCharactersOnSanitise) {
    UBX::SanitiseBuffer(buffer);
    
    ASSERT_EQ(0u, buffer.count());
}

TEST_F(ABufferWithSyncChar1AtStart, ReturnsAllCharactersDiscardedOnSanitise) {
    uint32 discarded = UBX::SanitiseBuffer(buffer);
    
    ASSERT_EQ(discarded, 10u);
}

class ABufferWithSyncChar1InMiddle : public ::testing::Test {
 public:
    ABufferWithSyncChar1InMiddle() : ::testing::Test(), buffer(10) {
        for (uint32 i = 0; i < 10; i++) {
            bytes_in[i] = i;
        }
        bytes_in[4] = UBX::SYNC_CHAR_1;
        bytes_in[5] = 0x00;
        buffer.queue(bytes_in, 10);
    }

    GPSBuffer buffer;
    uint8 bytes_in[10];
};

TEST_F(ABufferWithSyncChar1InMiddle, DiscardsAllCharactersOnSanitise) {
    UBX::SanitiseBuffer(buffer);
    
    ASSERT_EQ(0u, buffer.count());
}

TEST_F(ABufferWithSyncChar1InMiddle, ReturnsAllCharactersDiscardedOnSanitise) {
    uint32 discarded = UBX::SanitiseBuffer(buffer);
    
    ASSERT_EQ(discarded, 10u);
}

class ABufferWithSyncChar1AtEnd : public ::testing::Test {
 public:
    ABufferWithSyncChar1AtEnd() : ::testing::Test(), buffer(10) {
        for (uint32 i = 0; i < 10; i++) {
            bytes_in[i] = i;
        }
        bytes_in[9] = UBX::SYNC_CHAR_1;
        buffer.queue(bytes_in, 10);
    }

    GPSBuffer buffer;
    uint8 bytes_in[10];
};

TEST_F(ABufferWithSyncChar1AtEnd, DiscardsUpToSyncChar) {
    UBX::SanitiseBuffer(buffer);
    
    ASSERT_TRUE(ubx_test::buffer_contains(buffer, &bytes_in[9], 1u));
}

TEST_F(ABufferWithSyncChar1AtEnd, ReturnsCharactersDiscardedOnSanitise) {
    uint32 discarded = UBX::SanitiseBuffer(buffer);
    
    ASSERT_EQ(discarded, 9u);
}

class ABufferWithMultipleSyncCharPairs : public ::testing::Test {
 public:
    ABufferWithMultipleSyncCharPairs() : ::testing::Test(), buffer(10) {
        for (uint32 i = 0; i < 10; i++) {
            bytes_in[i] = i;
        }
        bytes_in[3] = UBX::SYNC_CHAR_1;
        bytes_in[4] = UBX::SYNC_CHAR_2;
        bytes_in[7] = UBX::SYNC_CHAR_1;
        bytes_in[8] = UBX::SYNC_CHAR_2;
        buffer.queue(bytes_in, 10);
    }

    GPSBuffer buffer;
    uint8 bytes_in[10];
};

TEST_F(ABufferWithMultipleSyncCharPairs, DiscardsUpToFirstSyncChar) {
    UBX::SanitiseBuffer(buffer);
    
    ASSERT_TRUE(ubx_test::buffer_contains(buffer, &bytes_in[3], 7u));
}

TEST_F(ABufferWithMultipleSyncCharPairs, ReturnsCharactersDiscardedOnSanitise) {
    uint32 discarded = UBX::SanitiseBuffer(buffer);
    
    ASSERT_EQ(discarded, 3u);
}

class ABufferWithLeadingSyncChar1 : public ::testing::Test {
 public:
    ABufferWithLeadingSyncChar1() : ::testing::Test(), buffer(10) {
        for (uint32 i = 0; i < 10; i++) {
            bytes_in[i] = i;
        }
        bytes_in[3] = UBX::SYNC_CHAR_1;
        bytes_in[4] = 0x00;
        bytes_in[7] = UBX::SYNC_CHAR_1;
        bytes_in[8] = UBX::SYNC_CHAR_2;
        buffer.queue(bytes_in, 10);
    }

    GPSBuffer buffer;
    uint8 bytes_in[10];
};

TEST_F(ABufferWithLeadingSyncChar1, DiscardsUpToSyncCharPair) {
    UBX::SanitiseBuffer(buffer);
    
    ASSERT_TRUE(ubx_test::buffer_contains(buffer, &bytes_in[7], 3u));
}

TEST_F(ABufferWithLeadingSyncChar1, ReturnsCharactersDiscardedOnSanitise) {
    uint32 discarded = UBX::SanitiseBuffer(buffer);
    
    ASSERT_EQ(discarded, 7u);
}

class ABufferWithMultipleLeadingSyncChars : public ::testing::Test {
 public:
    ABufferWithMultipleLeadingSyncChars() : ::testing::Test(), buffer(10) {
        for (uint32 i = 0; i < 10; i++) {
            bytes_in[i] = i;
        }
        bytes_in[1] = UBX::SYNC_CHAR_1;
        bytes_in[2] = 0x00;
        bytes_in[4] = UBX::SYNC_CHAR_1;
        bytes_in[5] = 0x00;
        bytes_in[7] = UBX::SYNC_CHAR_1;
        bytes_in[8] = UBX::SYNC_CHAR_2;
        buffer.queue(bytes_in, 10);
    }

    GPSBuffer buffer;
    uint8 bytes_in[10];
};

TEST_F(ABufferWithMultipleLeadingSyncChars, DiscardsUpToSyncCharPair) {
    UBX::SanitiseBuffer(buffer);
    
    ASSERT_TRUE(ubx_test::buffer_contains(buffer, &bytes_in[7], 3u));
}

TEST_F(ABufferWithMultipleLeadingSyncChars, ReturnsCharactersDiscardedOnSanitise) {
    uint32 discarded = UBX::SanitiseBuffer(buffer);
    
    ASSERT_EQ(discarded, 7u);
}

namespace ubx_test {

class UBXMessageHeader {
 public:
    UBXMessageHeader() : sync_char1(0xB5), sync_char2(0x62) {
        raw_ = new uint8[2];
    }

    ~UBXMessageHeader() {
        delete [] raw_;
    }
    
    uint32 len() {
        return 2;
    }

    const uint8* raw() {
        raw_[0] = sync_char1;
        raw_[1] = sync_char2;

        return raw_;
    }

    MARTe::uint8 sync_char1;
    MARTe::uint8 sync_char2;
 
 private:
    MARTe::uint8* raw_;
};


class UBXMessage {
 public:
     UBXMessage(uint8 msg_class, uint8 msg_id, const uint8* payload_, uint32 payload_len) : 
            header(), msg_class(msg_class), msg_id(msg_id) {
        payload = new uint8[payload_len];
        memcpy(payload, payload_, payload_len);
        length = payload_len;
        
        raw_ = new uint8[payload_len + 8];
    }

    ~UBXMessage() {
        delete [] raw_;
        delete [] payload;
    }
    
    uint8* raw() {
        uint32 i = 0;
        
        memcpy(&raw_[0], header.raw(), header.len()); 
        i += header.len();

        raw_[i] = msg_class;
        i += 1;
        raw_[i] = msg_id;
        i += 1;

        raw_[i] = static_cast<uint8>(length & 0x00FF);
        i += 1;
        raw_[i] = static_cast<uint8>((length & 0xFF00) >> 8);
        i += 1;

        memcpy(&raw_[i], payload, length);
        i += length;

        uint16 checksum = UBX::ubx_checksum(raw_ + 2, length + 4);
        raw_[i] = static_cast<uint8>(checksum & 0x00FF);
        i += 1;
        raw_[i] = static_cast<uint8>((checksum & 0xFF00) >> 8);
        i += 1;

        return raw_;
    }

    uint32 len() {
        return length + 8;
    }

    UBXMessageHeader header;
    uint8 msg_class;
    uint8 msg_id;
    uint16 length;
    uint8* payload;

 private:
    uint8* raw_;
};

} // namespace ubx_test

class GetNextMessage : public ::testing::Test {
 public:
    GetNextMessage() : buffer(10),
                       test_payload(),
                       test_message(UBX::TIMTPMessageClass, UBX::TIMTPMessageId, test_payload, 2u),
                       message(),
                       discarded(0) {}

    GPSBuffer buffer;
    uint8 test_payload[2];
    ubx_test::UBXMessage test_message;
    UBX::Message message;
    uint32 discarded;
};

TEST_F(GetNextMessage, ReturnsFalseIfBufferEmpty) {
    bool ret = UBX::GetNextMessage(buffer, message, discarded);

    ASSERT_FALSE(ret);
}


TEST_F(GetNextMessage, DoesNotDiscardIfBufferEmpty) {
    bool ret = UBX::GetNextMessage(buffer, message, discarded);

    ASSERT_EQ(0u, discarded);
}

TEST_F(GetNextMessage, ReturnsFalseIfBufferOnlyHasSyncChar1) {
    buffer.queue(test_message.raw(), 1u);

    bool ret = UBX::GetNextMessage(buffer, message, discarded);

    ASSERT_FALSE(ret);
}

TEST_F(GetNextMessage, DoesNotDiscardIfBufferOnlyHasSyncChar1) {
    buffer.queue(test_message.raw(), 1u);

    bool ret = UBX::GetNextMessage(buffer, message, discarded);

    ASSERT_EQ(0u, discarded);
}

TEST_F(GetNextMessage, ReturnsFalseIfBufferOnlyHasSyncChars) {
    buffer.queue(test_message.raw(), 2u);

    bool ret = UBX::GetNextMessage(buffer, message, discarded);

    ASSERT_FALSE(ret);
}

TEST_F(GetNextMessage, DoesNotDiscardIfBufferOnlyHasSyncChars) {
    buffer.queue(test_message.raw(), 2u);

    bool ret = UBX::GetNextMessage(buffer, message, discarded);

    ASSERT_EQ(0u, discarded);
}

TEST_F(GetNextMessage, ReturnsFalseIfLengthCannotBeDetermined) {
    buffer.queue(test_message.raw(), 2u);

    bool ret = UBX::GetNextMessage(buffer, message, discarded);

    ASSERT_FALSE(ret);
}

TEST_F(GetNextMessage, DoesNotDiscardIfMessageNotComplete) {
    buffer.queue(test_message.raw(), test_message.len() - 1);

    bool ret = UBX::GetNextMessage(buffer, message, discarded);

    ASSERT_EQ(0u, discarded);
}

TEST_F(GetNextMessage, ReturnsTrueIfMessageComplete) {
    buffer.queue(test_message.raw(), test_message.len());

    bool ret = UBX::GetNextMessage(buffer, message, discarded);

    ASSERT_TRUE(ret);
}

TEST_F(GetNextMessage, DoesNotDiscardIfMessageComplete) {
    buffer.queue(test_message.raw(), test_message.len());

    bool ret = UBX::GetNextMessage(buffer, message, discarded);

    ASSERT_EQ(0u, discarded);
}

TEST_F(GetNextMessage, ExtractsMessageClass) {
    buffer.queue(test_message.raw(), test_message.len());

    bool ret = UBX::GetNextMessage(buffer, message, discarded);

    ASSERT_EQ(test_message.msg_class, message.msg_class);
}

TEST_F(GetNextMessage, ExtractsMessageId) {
    buffer.queue(test_message.raw(), test_message.len());

    bool ret = UBX::GetNextMessage(buffer, message, discarded);

    ASSERT_EQ(test_message.msg_id, message.msg_id);
}

TEST_F(GetNextMessage, ReturnsFalseIfChecksumFails) {
    uint8* temp = new uint8[test_message.len()];
    memcpy(temp, test_message.raw(), test_message.len());
    // Manually modify the message to invalidate the checksum
    temp[2] = temp[2] + 1;
    buffer.queue(temp, test_message.len());

    bool ret = UBX::GetNextMessage(buffer, message, discarded);
    delete [] temp;

    ASSERT_FALSE(ret);
}

TEST_F(GetNextMessage, DiscardsMessageIfChecksumFails) {
    uint8* temp = new uint8[test_message.len()];
    memcpy(temp, test_message.raw(), test_message.len());
    // Manually modify the message to invalidate the checksum
    temp[2] = temp[2] + 1;
    buffer.queue(temp, test_message.len());

    bool ret = UBX::GetNextMessage(buffer, message, discarded);
    delete [] temp;

    ASSERT_EQ(test_message.len(), discarded);
}

TEST_F(GetNextMessage, SetsThePayload) {
    test_message.payload[0] = 0xab;
    test_message.payload[1] = 0xcd;
    buffer.queue(test_message.raw(), test_message.len());

    bool ret = UBX::GetNextMessage(buffer, message, discarded);

    ASSERT_TRUE(ubx_test::buffers_equal(message.payload, test_message.raw() + 6, test_message.len() - 8));
}

TEST_F(GetNextMessage, SetsThePayloadLength) {
    buffer.queue(test_message.raw(), test_message.len());

    bool ret = UBX::GetNextMessage(buffer, message, discarded);

    ASSERT_EQ(test_message.len() - 8, message.payload_len);
}

TEST_F(GetNextMessage, ReturnsFalseIfPayloadLengthGreaterThanMaximum) {
    uint8* temp = new uint8[test_message.len()];
    memcpy(temp, test_message.raw(), test_message.len());
    // Manually modify the payload length
    temp[4] = 0xFF;
    temp[5] = 0xFF;
    buffer.queue(temp, test_message.len());

    bool ret = UBX::GetNextMessage(buffer, message, discarded);
    delete [] temp;

    ASSERT_FALSE(ret);
}

TEST_F(GetNextMessage, DiscardsBufferIfPayloadLengthGreaterThanMaximum) {
    uint8* temp = new uint8[test_message.len()];
    memcpy(temp, test_message.raw(), test_message.len());
    // Manually modify the payload length
    temp[4] = 0xFF;
    temp[5] = 0xFF;
    buffer.queue(temp, test_message.len());
    uint32 count = buffer.count();

    bool ret = UBX::GetNextMessage(buffer, message, discarded);
    delete [] temp;

    ASSERT_EQ(count, discarded);
}

TEST_F(GetNextMessage, EmptiesBufferIfPayloadLengthGreaterThanMaximum) {
    uint8* temp = new uint8[test_message.len()];
    memcpy(temp, test_message.raw(), test_message.len());
    // Manually modify the payload length
    temp[4] = 0xFF;
    temp[5] = 0xFF;
    buffer.queue(temp, test_message.len());

    UBX::GetNextMessage(buffer, message, discarded);
    
    ASSERT_EQ(0u, buffer.count());
}
