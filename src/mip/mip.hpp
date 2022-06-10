#pragma once

#include <cstring>

#include <stdint.h>
#include <stddef.h>

#include "mip_packet.h"
#include "mip_field.h"
#include "mip_parser.h"
#include "mip_offsets.h"

///@defgroup CppApi  C++ API for MSCL-Embedded
///
///@see mscl namespace
///

////////////////////////////////////////////////////////////////////////////////
///@brief A collection of C++ classes and functions covering the full
/// mscl-embedded api.
///
namespace mscl
{

using C::PacketLength;


////////////////////////////////////////////////////////////////////////////////
///@brief C++ class representing a MIP field.
///
class MipField : public C::MipField
{
public:
    /// Construct an empty MIP field.
    MipField() { C::MipField::payload=nullptr; C::MipField::payloadLength=0; C::MipField::fieldDescriptor=0x00; C::MipField::descriptorSet=0x00; C::MipField::remainingLength=0; }
    ///@copydoc MipField_init()
    MipField(uint8_t descriptorSet, uint8_t fieldDescriptor, const uint8_t* payload, uint8_t payloadLength) { C::MipField_init(this, descriptorSet, fieldDescriptor, payload, payloadLength); }
    ///@copydoc MipField_fromHeaderPtr()
    MipField(const uint8_t* header, uint8_t totalLength, uint8_t descriptorSet) { *this = C::MipField_fromHeaderPtr(header, totalLength, descriptorSet); }
    /// Creates a %MipField class from the MipField C struct.
    MipField(const C::MipField& other) { std::memcpy(static_cast<C::MipField*>(this), &other, sizeof(C::MipField)); }

    ///@copydoc MipField_descriptorSet
    uint8_t descriptorSet() const { return C::MipField_descriptorSet(this); }
    ///@copydoc MipField_fieldDescriptor
    uint8_t fieldDescriptor() const { return C::MipField_fieldDescriptor(this); }
    ///@copydoc MipField_payloadLength
    uint8_t payloadLength() const { return C::MipField_payloadLength(this); }
    ///@copydoc MipField_payload
    const uint8_t* payload() const { return C::MipField_payload(this); }

    ///@copydoc MipField_isValid
    bool isValid() const { return C::MipField_isValid(this); }

    ///@copydoc MipField_nextAfter
    MipField nextAfter() const { return C::MipField_nextAfter(this); }
    ///@copydoc MipField_next
    bool next() { return MipField_next(this); }
};


////////////////////////////////////////////////////////////////////////////////
///@brief C++ class representing a MIP Packet.
///
/// Fields may be iterated over using the C-style method or with a range-based
/// for loop:
///@code{.cpp}
/// for(MipField field : packet) { ... }
///@endcode
///
class MipPacket : public C::MipPacket
{
public:
    ///@copydoc MipPacket_create
    MipPacket(uint8_t* buffer, size_t bufferSize, uint8_t descriptorSet) { C::MipPacket_create(this, buffer, bufferSize, descriptorSet); }
    ///@copydoc MipPacket_fromBuffer
    MipPacket(uint8_t* buffer, size_t length) { C::MipPacket_fromBuffer(this, buffer, length); }
    /// Constructs a C++ %MipPacket class from the base C object.
    MipPacket(const C::MipPacket& other) { std::memcpy(static_cast<C::MipPacket*>(this), &other, sizeof(*this)); }

    uint8_t      descriptorSet() const { return C::MipPacket_descriptorSet(this); }  ///<@copydoc MipPacket_descriptorSet
    PacketLength totalLength()   const { return C::MipPacket_totalLength(this);   }  ///<@copydoc MipPacket_totalLength
    uint8_t      payloadLength() const { return C::MipPacket_payloadLength(this); }  ///<@copydoc MipPacket_payloadLength

    const uint8_t* pointer() const { return C::MipPacket_pointer(this); }  ///<@copydoc MipPacket_pointer
    const uint8_t* payload() const { return C::MipPacket_payload(this); }  ///<@copydoc MipPacket_payload

    uint16_t checksumValue() const { return C::MipPacket_checksumValue(this); }     ///<@copydoc MipPacket_checksumValue
    uint16_t computeChecksum() const { return C::MipPacket_computeChecksum(this); } ///<@copydoc MipPacket_computeChecksum

    bool isSane() const { return C::MipPacket_isSane(this); }    ///<@copydoc MipPacket_isSane
    bool isValid() const { return C::MipPacket_isValid(this); }  ///<@copydoc MipPacket_isValid

    PacketLength bufferSize() const { return C::MipPacket_bufferSize(this); }            ///<@copydoc MipPacket_bufferSize
    RemainingCount remainingSpace() const { return C::MipPacket_remainingSpace(this); }  ///<@copydoc MipPacket_remainingSpace

    bool addField(uint8_t fieldDescriptor, const uint8_t* payload, size_t payloadLength) { return C::MipPacket_addField(this, fieldDescriptor, payload, payloadLength); }  ///<@copydoc MipPacket_addField
    RemainingCount allocField(uint8_t fieldDescriptor, uint8_t payloadLength, uint8_t** payloadPtr_out) { return C::MipPacket_allocField(this, fieldDescriptor, payloadLength, payloadPtr_out); }  ///<@copydoc MipPacket_allocField
    // RemainingCount reallocLastField(uint8_t* payloadPtr, uint8_t newPayloadLength) { return C::MipPacket_reallocField(this, payload, newPayloadLength); }  ///<@copydoc MipPacket_reallocField

    void finalize() { C::MipPacket_finalize(this); }  ///<@copydoc MipPacket_finalize

    /// Returns the first field in the packet.
    MipField firstField() const { return MipField(C::MipField_fromPacket(this)); }

    /// Iterator class for use with the range-based for loop.
    ///@internal
    class FieldIterator
    {
    public:
        FieldIterator(const MipField& first) : mField(first) {}
        FieldIterator() {}

        bool operator==(const FieldIterator& other) const {
            // Required to make invalid fields equivalent for range-based for loop
            if( !mField.isValid() && !other.mField.isValid() )
                return true;
            return mField.descriptorSet() == other.mField.descriptorSet() && mField.fieldDescriptor() == other.mField.fieldDescriptor() && mField.payload() == other.mField.payload();
        }
        bool operator!=(const FieldIterator& other) const { return !(*this == other); }

        bool operator==(std::nullptr_t) const { return !mField.isValid(); }
        bool operator!=(std::nullptr_t) const { return mField.isValid(); }

        const MipField& operator*() const { return mField; }

        FieldIterator& operator++() { mField.next(); return *this; }
    private:
        MipField mField;
    };

    /// Returns a forward iterator to the first field in the packet.
    ///@internal
    FieldIterator begin() const { return firstField(); }

    /// Returns a sentry object representing the end of fields in the packet.
    ///@internal
#if __cpp_range_based_for >= 201603
    nullptr_t     end() const { return nullptr; }
#else
    FieldIterator end() const { return MipField(); }
#endif
};


////////////////////////////////////////////////////////////////////////////////
///@brief C++ class representing a MIP parser.
///
class MipParser : public C::MipParsingState
{
public:
    ///@copydoc MipParser_init
    MipParser(uint8_t* buffer, size_t bufferSize, C::PacketCallback, void* callbackObject, Timestamp timeout) { C::MipParser_init(this, buffer, bufferSize, callback, callbackObject, timeout); }
    ///@copydoc MipParser_init
    MipParser(uint8_t* buffer, size_t bufferSize, bool (*callback)(void*,const MipPacket*,Timestamp), void* callbackObject, Timestamp timeout) { C::MipParser_init(this, buffer, bufferSize, (C::PacketCallback)callback, callbackObject, timeout); }

    ///@copydoc MipParser_reset
    void reset() { C::MipParser_reset(this); }

    ///@copydoc MipParser_parse
    RemainingCount parse(const uint8_t* inputBuffer, size_t inputCount, Timestamp timestamp, unsigned int maxPackets) { return C::MipParser_parse(this, inputBuffer, inputCount, timestamp, maxPackets); }

    ///@copydoc MipParser_timeout
    Timestamp timeout() const { return C::MipParser_timeout(this); }
    ///@copydoc MipParser_setTimeout
    void setTimeout(Timestamp timeout) { return C::MipParser_setTimeout(this, timeout); }
};


} // namespace mscl
////////////////////////////////////////////////////////////////////////////////
