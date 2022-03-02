#ifndef MFI_GPS_UBX_H_
#define MFI_GPS_UBX_H_

#include "CompilerTypes.h"
#include "GPSBuffer.h"

namespace MFI {
namespace UBX {

extern const MARTe::uint8 SYNC_CHAR_1;
extern const MARTe::uint8 SYNC_CHAR_2;
const MARTe::uint32 MAX_PAYLOAD_LEN = 128;
extern const MARTe::uint32 TIM_TP_MSG_SIZE;

/**
 * @brief Remove any leading non-message bytes from the buffer
 * 
 * Removes any data from the buffer which precedes the two synchronisation bytes
 */
MARTe::uint32 SanitiseBuffer(GPSBuffer& buffer);

/**
 * @brief A UBX-TIM-TOS message
 * 
 * Note that only a subset of the message fields are represented here
 */

/**
 * @brief A general UBX message
 */
class Message {
 public:
    MARTe::uint8 msg_class;
    MARTe::uint8 msg_id;
    MARTe::uint8 payload[MAX_PAYLOAD_LEN];
    MARTe::uint32 payload_len;
};

extern const MARTe::uint8 TIMTPMessageClass;
extern const MARTe::uint8 TIMTPMessageId;
extern const MARTe::uint32 TIMTPPayloadLength;

/**
 * @brief Get the next available UBX message from the buffer. The contents of the message are removed
 * from the buffer as part of the extraction.
 * 
 * Assumes that SanitiseBuffer has been called on the buffer immediately prior, so that there are no
 * leading non-message bytes in the buffer
 * 
 * If the payload length included in the next message is greater than the maximum, the buffer is
 * emptied (on the basis that some corruption must have occurred in the message after the sync 
 * characters).
 * 
 * Returns true if a message was extracted successfully, otherwise false
 * 
 * If any bytes were discarded (e.g. because the payload length was invalid), the number of discarded
 * bytes is reported in the @p discarded parameter.
 */
bool GetNextMessage(GPSBuffer& buffer, Message& message, MARTe::uint32& discarded);

/**
 * @brief The payload of a UBX-TIM-TP message
 */
class TIMTPPayload {
 public:    
    TIMTPPayload();
    bool operator==(const TIMTPPayload& other);
    MARTe::uint32 tow_ms;
    MARTe::uint32 tow_sub_ms;
    MARTe::int32 q_err;
    MARTe::uint16 week;
    MARTe::uint8 flags;
    MARTe::uint8 ref_info;
};

/**
 * @brief Parse a raw UBX-TIM-TP payload
 * 
 * Assumes that the buffer is the correct length for a UBX-TIM-TP message
 */
void ParseTIMTPPayload(MARTe::uint8* raw, TIMTPPayload& payload);

/**
 * @brief Compute a UBX message checksum
 *
 * See the UBX protocol specification (Section 32.4) for details 
 */
MARTe::uint16 ubx_checksum(const MARTe::uint8* buffer, MARTe::uint32 len);

} // namespace UBX
} // namespace MFI

#endif // MFI_GPS_UBX_H_
