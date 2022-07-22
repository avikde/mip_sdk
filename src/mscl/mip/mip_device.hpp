#pragma once

#include "mip.hpp"
#include "mip_interface.h"

#include "definitions/descriptors.h"


namespace mscl
{

using MipDispatchHandler = C::mip_dispatch_handler;


class MipDeviceInterface : public C::mip_interface
{
public:
    //
    // Constructors
    //

    ///@copydoc mip_interface_init
    MipDeviceInterface(uint8_t* parseBuffer, size_t parseBufferSize, Timeout parseTimeout, Timeout baseReplyTimeout) { C::mip_interface_init(this, parseBuffer, parseBufferSize, parseTimeout, baseReplyTimeout); }

    MipDeviceInterface(const MipDeviceInterface&) = delete;
    MipDeviceInterface& operator=(const MipDeviceInterface&) = delete;

    ~MipDeviceInterface() = default;

    //
    // Accessors
    //

    void setMaxPacketsPerPoll(unsigned int maxPackets) { C::mip_interface_set_max_packets_per_update(this, maxPackets); }
    unsigned int maxPacketsPerPoll() const             { return C::mip_interface_max_packets_per_update(this); }

    Timeout baseReplyTimeout() const          { return C::mip_cmd_queue_base_reply_timeout(&cmdQueue()); }
    void setBaseReplyTimeout(Timeout timeout) { C::mip_cmd_queue_set_base_reply_timeout(&cmdQueue(), timeout); }


    MipParser&              parser()   { return *static_cast<MipParser*>(C::mip_interface_parser(this)); }
    C::mip_cmd_queue&       cmdQueue() { return *C::mip_interface_cmd_queue(this); }

    const MipParser&        parser() const   { return const_cast<MipDeviceInterface*>(this)->parser(); }
    const C::mip_cmd_queue& cmdQueue() const { return const_cast<MipDeviceInterface*>(this)->cmdQueue(); }

    //
    // Communications
    //

    RemainingCount receiveBytes(const uint8_t* data, size_t length, Timestamp timestamp) { return C::mip_interface_receive_bytes(this, data, length, timestamp); }

    void           receivePacket(const C::mip_packet& packet, Timestamp timestamp) { C::mip_interface_receive_packet(this, &packet, timestamp); }

    virtual bool   sendToDevice(const uint8_t* data, size_t length) = 0;  // Must be implemented by a derived class.
    bool           sendToDevice(const mscl::C::mip_packet& packet) { return sendToDevice(mscl::C::mip_packet_pointer(&packet), mscl::C::mip_packet_total_length(&packet)); }

    virtual bool   update() = 0; // Must be implemented by a derived class. Called by mip_interface_user_update().

    void           processUnparsedPackets() { C::mip_interface_process_unparsed_packets(this); }

    MipCmdResult   waitForReply(const C::mip_pending_cmd& cmd) { return C::mip_interface_wait_for_reply(this, &cmd); }


    template<class Function>
    bool parseFromSource(Function function, unsigned int maxPackets=MIPPARSER_UNLIMITED_PACKETS) { return parseMipDataFromSource(parser(), function, maxPackets); }

    //
    // These must be implemented by a derived class
    //

    // virtual bool update() = 0;
    // virtual bool sendToDevice(const uint8_t* data, size_t length) = 0;


    //
    // Data Callbacks
    //

    void registerPacketCallback(C::mip_dispatch_handler& handler, uint8_t descriptorSet, C::mip_dispatch_packet_callback callback, void* userData) { C::mip_interface_register_packet_callback(this, &handler, descriptorSet, callback, userData); }
    void registerFieldCallback(C::mip_dispatch_handler& handler, uint8_t descriptorSet, uint8_t fieldDescriptor, C::mip_dispatch_field_callback callback, void* userData) { C::mip_interface_register_field_callback(this, &handler, descriptorSet, fieldDescriptor, callback, userData); }


    template<void (*Callback)(void*, const MipPacket&, Timestamp)>
    void registerPacketCallback(C::mip_dispatch_handler& handler, uint8_t descriptorSet, void* userData=nullptr);

    template<class Object, void (Object::*Callback)(const MipPacket&, Timestamp)>
    void registerPacketCallback(C::mip_dispatch_handler& handler, uint8_t descriptorSet, Object* object);


    template<void (*Callback)(void*, const MipField&, Timestamp)>
    void registerFieldCallback(C::mip_dispatch_handler& handler, uint8_t descriptorSet, uint8_t fieldDescriptor, void* userData=nullptr);

    template<class Object, void (Object::*Callback)(const MipField& field, Timestamp)>
    void registerFieldCallback(C::mip_dispatch_handler& handler, uint8_t descriptorSet, uint8_t fieldDescriptor, Object* object);


    template<class Field, void (*Callback)(void*, const Field&, Timestamp)>
    void registerDataCallback(C::mip_dispatch_handler& handler, void* userData=nullptr, uint8_t descriptorSet=Field::descriptorSet);

    template<class Field, class Object, void (Object::*Callback)(const Field&, Timestamp)>
    void registerDataCallback(C::mip_dispatch_handler& handler, Object* object, uint8_t descriptorSet=Field::descriptorSet);
};


////////////////////////////////////////////////////////////////////////////////
///@brief Registers a packet callback (free function version).
///
///@tparam Callback A pointer to the function to call. This must be a constant
///        function pointer.
///
///@param handler
///@param descriptorSet
///@param userData
///
/// Example usage:
///@code{.cpp}
/// void handle_packet(void* context, const MipPacket& packet, Timestamp timestamp)
/// {
///   // Use the packet data
/// }
///
/// MipDeviceInterface device;
/// MipDispatchHandler handler;
///
/// void setup()
/// {
///   // Set up device...
///
///   device.registerPacketCallback<&handle_packet>(handler, descriptorSet, nullptr);
/// }
///
///@endcode
///
template<void (*Callback)(void*, const MipPacket&, Timestamp)>
void MipDeviceInterface::registerPacketCallback(C::mip_dispatch_handler& handler, uint8_t descriptorSet, void* userData)
{
    auto callback = [](void* context, const C::mip_packet* packet, Timestamp timestamp)
    {
        Callback(context, MipPacket(*packet), timestamp);
    };

    registerPacketCallback(handler, descriptorSet, callback, userData);
}


////////////////////////////////////////////////////////////////////////////////
///@brief Registers a packet callback (member function version).
///
///@tparam Callback A pointer to the function to call. This must be a constant
///        member function pointer.
///
///@param handler
///@param descriptorSet
///@param object
///
/// Example usage:
///@code{.cpp}
/// class MySystem
/// {
///   void handlePacket(const MipPacket& packet, Timestamp timestamp)
///   {
///   }
///
///   void setup()
///   {
///     // setup device...
///     device.registerPacketHandler<MySystem, &MySystem::handlePacket>(packetHandler, descriptorSet, this);
///   }
///
///   MipDeviceInterface device;
///   MipDispatchHandler packetHandler;
/// };
///@endcode
///
template<class Object, void (Object::*Callback)(const MipPacket&, Timestamp)>
void MipDeviceInterface::registerPacketCallback(C::mip_dispatch_handler& handler, uint8_t descriptorSet, Object* object)
{
    auto callback = [](void* pointer, const MipPacket& packet, Timestamp timestamp)
    {
        Object* obj = static_cast<Object*>(pointer);
        (obj->*Callback)(MipPacket(packet), timestamp);
    };

    registerPacketCallback(handler, descriptorSet, callback, object);
}


////////////////////////////////////////////////////////////////////////////////
///@brief Registers a field callback (free function version).
///
///@tparam Callback A pointer to the function to call. This must be a constant
///        function pointer.
///
///@param handler
///@param descriptorSet
///@param fieldDescriptor
///@param userData
///
/// Example usage:
///@code{.cpp}
/// void handle_field(void* context, const MipField& packet, Timestamp timestamp)
/// {
///   // Use the field data
/// }
///
/// MipDeviceInterface device;
/// MipDispatchHandler handler;
///
/// void setup()
/// {
///   // Set up device...
///
///   device.registerFieldCallback<&handle_field>(handler, descriptorSet, nullptr);
/// }
///
///@endcode
///
template<void (*Callback)(void*, const MipField&, Timestamp)>
void MipDeviceInterface::registerFieldCallback(C::mip_dispatch_handler& handler, uint8_t descriptorSet, uint8_t fieldDescriptor, void* userData)
{
    auto callback = [](void* context, const C::mip_field* field, Timestamp timestamp)
    {
        Callback(context, MipField(*field), timestamp);
    };

    registerFieldCallback(handler, descriptorSet, fieldDescriptor, callback, userData);
}


////////////////////////////////////////////////////////////////////////////////
///@brief Registers a field callback (member function version).
///
///@tparam Callback A pointer to the function to call. This must be a constant
///        member function pointer.
///
///@param handler
///@param descriptorSet
///@param fieldDescriptor
///@param object
///
/// Example usage:
///@code{.cpp}
/// class MySystem
/// {
///   void handleField(const MipField& field, Timestamp timestamp)
///   {
///   }
///
///   void setup()
///   {
///     // setup device...
///     device.registerFieldHandler<MySystem, &MySystem::handleField>(fieldHandler, descriptorSet, fieldDescriptor, this);
///   }
///
///   MipDeviceInterface device;
///   MipDispatchHandler fieldHandler;
/// };
///@endcode
///
template<class Object, void (Object::*Callback)(const MipField&, Timestamp)>
void MipDeviceInterface::registerFieldCallback(C::mip_dispatch_handler& handler, uint8_t descriptorSet, uint8_t fieldDescriptor, Object* object)
{
    auto callback = [](void* pointer, const C::mip_field* field, Timestamp timestamp)
    {
        Object* obj = static_cast<Object*>(pointer);
        (obj->*Callback)(MipField(*field), timestamp);
    };

    registerFieldCallback(handler, descriptorSet, fieldDescriptor, &callback, object);
}


////////////////////////////////////////////////////////////////////////////////
///@brief Registers a data callback (free function version).
///
///@tparam Callback A pointer to the function to call. This must be a constant
///        function pointer.
///
///@param handler
///       This must exist as long as the hander remains registered.
///
///@param userData
///       Optional data to pass to the callback function.
///
///@param descriptorSet
///       If specified, overrides the descriptor set. Intended to be used with
///       with shared data quantities.
///
/// Example usage:
///@code{.cpp}
/// void handle_packet(void* context, const MipPacket& packet, Timestamp timestamp)
/// {
///   // Use the packet data
/// }
///
/// MipDeviceInterface device;
/// MipDispatchHandler handler;
///
/// void setup()
/// {
///   // Set up device...
///
///   device.registerDataCallback<&handle_packet>(handler, descriptorSet, nullptr);
/// }
///
///@endcode
///
template<class Field, void (*Callback)(void*, const Field&, Timestamp)>
void MipDeviceInterface::registerDataCallback(C::mip_dispatch_handler& handler, void* userData, uint8_t descriptorSet)
{
    assert(descriptorSet != 0x00);
    if(descriptorSet == 0x00)
        return;

    assert(descriptorSet != 0xFF);  // Descriptor set must be specified for shared data.
    if(descriptorSet == 0xFF)
        return;

    auto callback = [](void* context, const C::mip_field* field, Timestamp timestamp)
    {
        Field data;

        data.extract(C::mip_field_payload(field), C::mip_field_payload_length(field));

        Callback(context, data, timestamp);
    };

    registerFieldCallback(handler, descriptorSet, Field::fieldDescriptor, userData);
}

////////////////////////////////////////////////////////////////////////////////
///@brief Registers a data callback (member function version).
///
///@tparam Callback A pointer to the function to call. This must be a constant
///        member function pointer.
///
///@param handler
///       This must exist as long as the hander remains registered.
///
///@param object
///       A pointer to the object. The object must exist while the handler
///       remains registered.
///
///@param descriptorSet
///       If specified, overrides the descriptor set. Intended to be used with
///       with shared data quantities.
///
/// Example usage:
///@code{.cpp}
/// class MySystem
/// {
///   void handleAccel(const SensorScaledAccel& accel, Timestamp timestamp)
///   {
///   }
///
///   void setup()
///   {
///     // setup device...
///     device.registerDataHandler<SensorScaledAccel, MySystem, &MySystem::handleAccel>(accelHandler, this);
///   }
///
///   MipDeviceInterface device;
///   MipDispatchHandler accelHandler;
/// };
///@endcode
///
template<class Field, class Object, void (Object::*Callback)(const Field&, Timestamp)>
void MipDeviceInterface::registerDataCallback(C::mip_dispatch_handler& handler, Object* object, uint8_t descriptorSet)
{
    assert(descriptorSet != 0x00);
    if(descriptorSet == 0x00)
        return;

    assert(descriptorSet != 0xFF);  // Descriptor set must be specified for shared data.
    if(descriptorSet == 0xFF)
        return;

    auto callback = [](void* pointer, const C::mip_field* field, Timestamp timestamp)
    {
        Field data;

        data.extract(C::mip_field_payload(field), C::mip_field_payload_length(field));

        Object* obj = static_cast<Object*>(pointer);

        (obj->*Callback)(data, timestamp);
    };

    registerFieldCallback(handler, descriptorSet, Field::fieldDescriptor, callback, object);
}



template<class Cmd>
MipCmdResult runCommand(C::mip_interface& device, const Cmd& cmd)
{
    uint8_t buffer[MIP_PACKET_LENGTH_MAX];
    MipPacket packet = MipPacket::createFromField(buffer, sizeof(buffer), cmd);

    C::mip_pending_cmd pending;
    C::mip_pending_cmd_init(&pending, MipFieldInfo<Cmd>::descriptorSet, MipFieldInfo<Cmd>::fieldDescriptor);

    return C::mip_interface_run_command_packet(&device, &packet, &pending);
}

template<class Cmd>
MipCmdResult runCommand(C::mip_interface& device, const Cmd& cmd, typename MipFieldInfo<Cmd>::Response& response)
{
    uint8_t buffer[MIP_PACKET_LENGTH_MAX];
    MipPacket packet = MipPacket::createFromField(buffer, sizeof(buffer), cmd);

    C::mip_pending_cmd pending;
    C::mip_pending_cmd_init_with_response(&pending, MipFieldInfo<Cmd>::descriptorSet, MipFieldInfo<Cmd>::fieldDescriptor, MipFieldInfo<Cmd>::responseDescriptor, buffer, MIP_FIELD_PAYLOAD_LENGTH_MAX);

    MipCmdResult result = C::mip_interface_run_command_packet(&device, &packet, &pending);
    if( result != C::MIP_ACK_OK )
        return result;

    size_t responseLength = C::mip_pending_cmd_response_length(&pending);
    size_t offset = MipFieldInfo<Cmd>::extract_response(buffer, responseLength, 0, response);
    if( offset != responseLength )
        return C::MIP_STATUS_ERROR;

    return C::MIP_ACK_OK;
}


} // namespace mscl