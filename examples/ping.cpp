
#include "serial_mip_device.hpp"

#include <mip/definitions/commands_base.h>
#include <mip/mip.hpp>
#include <mip/mip_device.hpp>
#include <serial/serial.h>

#include <cstring>
#include <stdio.h>




int usage(const char* argv[])
{
    fprintf(stderr, "Usage: %s <port> <baudrate>\n", argv[0]);
    return 1;
}


int main(int argc, const char* argv[])
{
    uint32_t baud = 0;

    if( argc == 1 )
    {
        printf("Available serial ports:\n");
        std::vector<serial::PortInfo> ports = serial::list_ports();

        for(const serial::PortInfo& port : ports)
        {
            printf("  %s %s %s\n", port.port.c_str(), port.description.c_str(), port.hardware_id.c_str());
        }
        return 0;
    }
    else if( argc == 3 )
    {
        baud = std::strtoul(argv[2], nullptr, 10);
        if( baud == 0 )
        {
            fprintf(stderr, "Error: invalid baud rate '%s'\n", argv[2]);
            return 1;
        }
    }
    else
    {
        return usage(argv);
    }

    try
    {
#define METHOD 4

#if METHOD == 1 || METHOD == 2 || METHOD == 3
        serial::Serial port(argv[1], baud, serial::Timeout::simpleTimeout(10));

        uint8_t buffer[MIP_PACKET_LENGTH_MAX];

        mscl::MipPacket packet(buffer, sizeof(buffer), mscl::MIP_BASE_COMMAND_DESC_SET);

        uint8_t* payload;
        mscl::RemainingCount available = packet.allocField(mscl::MIP_CMD_DESC_BASE_PING, 0, &payload);

    #if METHOD == 1
        mscl::MipCmd_Base_GetDeviceInfo info;
        size_t used = mscl::insert_MipCmd_Base_GetDeviceInfo(payload, available, 0, &info);
    #elif METHOD == 2
        mscl::MipCmd_Base_GetDeviceInfo info;
        size_t used = mscl::insert(payload, available, 0, info);
    #elif METHOD == 3
        size_t used = mscl::insert_MipCmd_Base_GetDeviceInfo_args(payload, available, 0);
    #endif

        // Skip error checking as this field will always fit in this buffer.
        packet.reallocLastField(payload, used);

        packet.finalize();

        printf("Send bytes [");
        for(size_t i=0; i<packet.totalLength(); i++)
            printf("%02X", packet.pointer()[i]);
        printf("]\n");

        port.write(packet.pointer(), packet.totalLength());

        size_t read = port.read(buffer, sizeof(buffer));

        printf("Received bytes [");
        for(size_t i=0; i<read; i++)
            printf("%02X", buffer[i]);
        printf("]\n");

#elif METHOD == 4

        MipDevice device(argv[1], baud);

        uint8_t responseBuffer[MIP_FIELD_PAYLOAD_LENGTH_MAX];
        mscl::C::MipPendingCmd cmd;
        mscl::C::MipPendingCmd_initWithResponse(&cmd, mscl::MIP_BASE_COMMAND_DESC_SET, mscl::MIP_CMD_DESC_BASE_GET_DEVICE_INFO, mscl::MIP_REPLY_DESC_BASE_DEVICE_INFO, responseBuffer, sizeof(responseBuffer));
        mscl::C::MipCmdQueue_enqueue(device.cmdQueue(), &cmd);

        uint8_t packetBuffer[MIP_PACKET_LENGTH_MAX];
        mscl::MipPacket packet(packetBuffer, sizeof(packetBuffer), mscl::MIP_BASE_COMMAND_DESC_SET);

        packet.addField(mscl::MIP_CMD_DESC_BASE_GET_DEVICE_INFO, NULL, 0);
        packet.finalize();

        device.sendToDevice(packet);

        mscl::C::MipCmdResult result = device.waitForReply(cmd);

        if( result == mscl::C::MIP_ACK_OK )
        {
            const size_t responseSize = mscl::C::MipPendingCmd_responseLength(&cmd);
            printf("Success: command completed with ACK: responseLength=%ld\n", responseSize);

            mscl::MipCmd_Base_GetDeviceInfo_Response response;
            size_t used = mscl::extract_MipCmd_Base_GetDeviceInfo_Response(mscl::C::MipPendingCmd_response(&cmd), responseSize, 0, &response);
            if( used == responseSize )
            {
                auto print_info = [](const char* name, const char info[16])
                {
                    char msg[17] = {0};
                    std::strncpy(msg, info, 16);
                    printf("  %s%s\n", name, msg);
                };

                print_info("Model name:     ", response.device_info.model_name);
                print_info("Model number:   ", response.device_info.model_number);
                print_info("Serial Number:  ", response.device_info.serial_number);
                print_info("Device Options: ", response.device_info.device_options);
                print_info("Lot Number:     ", response.device_info.lot_number);

                printf(  "  Firmware version %d.%d.%d\n\n",
                    (response.device_info.firmware_version / 1000),
                    (response.device_info.firmware_version / 100) % 10,
                    (response.device_info.firmware_version / 1)   % 100
                );
            }
            else
            {
                printf("Error: response size does not match expected (at least %ld bytes)\n", used);
            }
        }
        else
        {
            printf("Error: command completed with NACK: %s (%d)\n", mscl::C::MipCmdResult_toString(result), result);
        }

#endif
    }
    catch(const std::exception& ex)
    {
        fprintf(stderr, "Could not open serial port: %s", ex.what());
        return 1;
    }

    return 0;
}