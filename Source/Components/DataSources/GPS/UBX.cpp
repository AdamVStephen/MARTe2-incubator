#include "UBX.h"
#include <iostream>
#include <cstring>

using namespace MARTe;

namespace MFI {
namespace UBX {

const uint8 SYNC_CHAR_1 = 0xB5;
const uint8 SYNC_CHAR_2 = 0x62;
const MARTe::uint8 TIMTPMessageClass = 0x0D;
const MARTe::uint8 TIMTPMessageId = 0x01;
const MARTe::uint32 TIMTPPayloadLength = 16;
const MARTe::uint32 TIM_TP_MSG_SIZE = 24;

TIMTPPayload::TIMTPPayload() :
    tow_ms(0u),
    tow_sub_ms(0u),
    q_err(0),
    week(0u),
    flags(0u),
    ref_info(0u) {
}

bool TIMTPPayload::operator==(const TIMTPPayload& other) {
    return tow_ms == other.tow_ms &&
           tow_sub_ms == other.tow_sub_ms &&
           q_err == other.q_err &&
           week == other.week &&
           flags == other.flags &&
           ref_info == other.ref_info;
}

uint32 SanitiseBuffer(GPSBuffer& buffer) {
    uint32 index = 0;

    while (1) {
        // If SYNC_CHAR_1 doesn't exist, discard the whole buffer
        bool found = buffer.find(SYNC_CHAR_1, index, index);
        if (!found) {
            index = buffer.count();
            break;
        }

        // SYNC_CHAR_1 is located at index
        uint8 value = 0;
        // If the next value exists
        if (buffer.at(index + 1, value)) {
            // ... and the value at the next index  is SYNC_CHAR_2...
            if (value == SYNC_CHAR_2) {
                // .. then empty the buffer up to index
                break;
            }
            else {
                // ... continue searching from the next character
                index += 1;
            }
        } 
        // ... and if the next index doesn't exist...
        else {
            // ... just empty up to SYNC_CHAR_1          
            break;
        }
    }

    buffer.empty(index);
    
    return index;
}

bool GetNextMessage(GPSBuffer& buffer, Message& message, uint32& discarded) {
    discarded = 0u;
    
    uint32 count = buffer.count();
    
    // Remove lower bound as it is irrelevant for unsigned integers
    if (count >=0 && count < 6) {
        // Not enough information to know the message length
        return false;
    }

    buffer.at(2u, message.msg_class);
    buffer.at(3u, message.msg_id);
    
    uint8 payload_len_lsb, payload_len_msb;
    buffer.at(4u, payload_len_lsb);
    buffer.at(5u, payload_len_msb);
    uint16 payload_len = static_cast<uint16>(payload_len_lsb) + 
                        (static_cast<uint16>(payload_len_msb) << 8);
    // If the indicated payload length is too large, scrub the entire buffer
    if (payload_len > UBX::MAX_PAYLOAD_LEN) {
        discarded = buffer.count();
        buffer.empty();
        
        return false;
    }
    // If we don't have enough bytes for the complete message, give up
    if (count < (payload_len + 8u)) {
        return false;
    }

    // Allocate enough space for the payload plus class and id
    uint8 temp[MAX_PAYLOAD_LEN + 2];

    // Get rid of the synchronisation characters
    buffer.empty(2u);
    // Dequeue the remainder of the message
    buffer.dequeue(temp, payload_len + 6);
    
    // Verify the checksum
    uint16 expected_checksum = ubx_checksum(temp, payload_len + 4);
    uint16 actual_checksum = static_cast<uint16>(temp[payload_len + 4]) + 
                             (static_cast<uint16>(temp[payload_len + 5]) << 8);
    if (expected_checksum != actual_checksum) {
        discarded = payload_len + 8;
        return false;
    }

    // Retrieve the payload
    memcpy(message.payload, &temp[4], payload_len);
    message.payload_len = payload_len;
    
    return true;
}

uint16 ubx_checksum(const uint8* buffer, uint32 len) {
    uint8 chk_a = 0, chk_b = 0;

    for (uint32 i = 0; i < len; i++) {
        chk_a = chk_a + buffer[i];
        chk_b = chk_b + chk_a;
    }

    return (static_cast<uint16>(chk_b) << 8) + chk_a;
}

void ParseTIMTPPayload(uint8* raw, TIMTPPayload& payload) {
    payload.tow_ms = static_cast<uint32>(raw[0]) +
                     (static_cast<uint32>(raw[1]) << 8) +
                     (static_cast<uint32>(raw[2]) << 16) +
                     (static_cast<uint32>(raw[3]) << 24);

    payload.tow_sub_ms = static_cast<uint32>(raw[4]) +
                        (static_cast<uint32>(raw[5]) << 8) +
                        (static_cast<uint32>(raw[6]) << 16) +
                        (static_cast<uint32>(raw[7]) << 24);

    payload.q_err = static_cast<int32>(raw[8]) +
                   (static_cast<int32>(raw[9]) << 8) +
                   (static_cast<int32>(raw[10]) << 16) +
                   (static_cast<int32>(raw[11]) << 24);

    payload.week = static_cast<uint16>(raw[12]) +
                   (static_cast<uint16>(raw[13]) << 8);

    payload.flags = raw[14];
    payload.ref_info = raw[15];
}

} // namespace UBX
} // namespace MFI
