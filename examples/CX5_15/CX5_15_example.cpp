
/////////////////////////////////////////////////////////////////////////////
//
// CX5_15_Example.cpp
//
// C++ Example set-up program for the CX5-15
//
// This example shows a typical setup for the CX5-15 sensor in a wheeled-vehicle application using using C++.
// It is not an exhaustive example of all CX5-15 settings.
// If your specific setup needs are not met by this example, please consult
// the MSCL-embedded API documentation for the proper commands.
//
//
//!@section LICENSE
//!
//! THE PRESENT SOFTWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING
//! CUSTOMERS WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER
//! FOR THEM TO SAVE TIME. AS A RESULT, PARKER MICROSTRAIN SHALL NOT BE HELD
//! LIABLE FOR ANY DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY
//! CLAIMS ARISING FROM THE CONTENT OF SUCH SOFTWARE AND/OR THE USE MADE BY CUSTOMERS
//! OF THE CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
//
/////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// Include Files
////////////////////////////////////////////////////////////////////////////////

#include <mip/mip_all.hpp>
#include <array>
#include "../example_utils.hpp"

using namespace mip;


////////////////////////////////////////////////////////////////////////////////
// Global Variables
////////////////////////////////////////////////////////////////////////////////


//Sensor-to-vehicle frame rotation (Euler Angles)
float sensor_to_vehicle_rotation_euler[3] = {0.0, 0.0, 0.0};

//GNSS antenna offset
float gnss_antenna_offset_meters[3] = {-0.25, 0.0, 0.0};

//Device data stores
data_sensor::GpsTimestamp sensor_gps_time;
data_sensor::ScaledAccel  sensor_accel;
data_sensor::ScaledGyro   sensor_gyro;
data_sensor::ScaledMag    sensor_mag;

data_gnss::FixInfo        gnss_fix_info;

bool gnss_fix_info_valid = false;

data_filter::Timestamp    filter_gps_time;
data_filter::Status       filter_status;
data_filter::PositionLlh  filter_position_llh;
data_filter::VelocityNed  filter_velocity_ned;
data_filter::EulerAngles  filter_euler_angles;
data_filter::CompAngularRate  filter_comp_angular_rate;
data_filter::CompAccel    filter_comp_accel;

bool filter_state_running = false;


////////////////////////////////////////////////////////////////////////////////
// Function Prototypes
////////////////////////////////////////////////////////////////////////////////

int usage(const char* argv0);

void exit_gracefully(const char *message);
bool should_exit();


////////////////////////////////////////////////////////////////////////////////
// Main Function
////////////////////////////////////////////////////////////////////////////////


int main(int argc, const char* argv[])
{

    std::unique_ptr<ExampleUtils> utils = handleCommonArgs(argc, argv);
    std::unique_ptr<mip::DeviceInterface>& device = utils->device;

    printf("Connecting to and configuring sensor.\n");

    //
    //Ping the device (note: this is good to do to make sure the device is present)
    //

    if(commands_base::ping(*device) != CmdResult::ACK_OK)
        exit_gracefully("ERROR: Could not ping the device!");


    //
    //Idle the device (note: this is good to do during setup)
    //

    if(commands_base::setIdle(*device) != CmdResult::ACK_OK)
        exit_gracefully("ERROR: Could not set the device to idle!");


    //
    //Load the device default settings (so the device is in a known state)
    //

    if(commands_3dm::defaultDeviceSettings(*device) != CmdResult::ACK_OK)
        exit_gracefully("ERROR: Could not load default device settings!");


    float gyro_bias[3] = {0, 0, 0};

    const uint32_t sampling_time = 15000;
    const int32_t old_mip_sdk_timeout = device->baseReplyTimeout();
    printf("Capturing gyro bias. This will take %d seconds \n", sampling_time/1000);
    device->setBaseReplyTimeout(sampling_time * 2);

    if(commands_3dm::captureGyroBias(*device, sampling_time, gyro_bias) != CmdResult::ACK_OK)
        exit_gracefully("ERROR: Could not capture gyro bias!");

    if(commands_3dm::saveGyroBias(*device) != CmdResult::ACK_OK)
        exit_gracefully("ERROR: Could not save gyro bias!");

    const uint8_t fn_selector = 1;
    const uint8_t device_selector = 3;
    const uint8_t enable_flag = 1;
    if(commands_3dm::writeDatastreamControl(*device, device_selector, enable_flag) != CmdResult::ACK_OK)
        exit_gracefully("ERROR: Could not enable device data stream!");

    // Reset the timeout
    device->setBaseReplyTimeout(old_mip_sdk_timeout);

    printf("Gyro bias captured with sampling time: %d, and gyro bias captured as: %f %f %f.\n", sampling_time, gyro_bias[0], gyro_bias[1], gyro_bias[3]);


    //
    //Setup Sensor data format to 100 Hz
    //

    uint16_t sensor_base_rate;

    //Note: Querying the device base rate is only one way to calculate the descriptor decimation.
    //We could have also set it directly with information from the datasheet (shown in GNSS setup).

    if(commands_3dm::imuGetBaseRate(*device, &sensor_base_rate) != CmdResult::ACK_OK)
         exit_gracefully("ERROR: Could not get sensor base rate format!");

    const uint16_t sensor_sample_rate = 100; // Hz
    const uint16_t sensor_decimation = sensor_base_rate / sensor_sample_rate;

    std::array<DescriptorRate, 3> sensor_descriptors = {{
        { data_sensor::DATA_TIME_STAMP_GPS, sensor_decimation },
        { data_sensor::DATA_ACCEL_SCALED,   sensor_decimation },
        { data_sensor::DATA_GYRO_SCALED,    sensor_decimation },
    }};

    if(commands_3dm::writeImuMessageFormat(*device, sensor_descriptors.size(), sensor_descriptors.data()) != CmdResult::ACK_OK)
        exit_gracefully("ERROR: Could not set sensor message format!");


    //
    //Setup GNSS data format to 4 Hz (decimation of 1)
    //

    std::array<DescriptorRate, 1> gnss_descriptors = {{
        { data_gnss::DATA_FIX_INFO, 1 }
    }};


    //
    //Setup FILTER data format
    //

    uint16_t filter_base_rate;

    if(commands_3dm::filterGetBaseRate(*device, &filter_base_rate) != CmdResult::ACK_OK)
         exit_gracefully("ERROR: Could not get filter base rate format!");

    const uint16_t filter_sample_rate = 100; // Hz
    const uint16_t filter_decimation = filter_base_rate / filter_sample_rate;

    std::array<DescriptorRate, 5> filter_descriptors = {{
        { data_filter::DATA_FILTER_TIMESTAMP, filter_decimation },
        { data_filter::DATA_FILTER_STATUS,    filter_decimation },
        { data_filter::DATA_ATT_EULER_ANGLES, filter_decimation },
        { data_filter::DATA_COMPENSATED_ANGULAR_RATE, filter_decimation },
        { data_filter::DATA_COMPENSATED_ACCELERATION, filter_decimation },
    }};

    if(commands_3dm::writeFilterMessageFormat(*device, filter_descriptors.size(), filter_descriptors.data()) != CmdResult::ACK_OK)
        exit_gracefully("ERROR: Could not set filter message format!");


    //
    //Setup the sensor to vehicle rotation
    //

    if(commands_filter::writeSensorToVehicleRotationEuler(*device, sensor_to_vehicle_rotation_euler[0], sensor_to_vehicle_rotation_euler[1], sensor_to_vehicle_rotation_euler[2]) != CmdResult::ACK_OK)
        exit_gracefully("ERROR: Could not set sensor-2-vehicle rotation!");

    //
    //Enable filter auto-initialization
    //

    if(commands_filter::writeAutoInitControl(*device, 1) != CmdResult::ACK_OK)
        exit_gracefully("ERROR: Could not set filter autoinit control!");



    //
    //Reset the filter (note: this is good to do after filter setup is complete)
    //

    if(commands_filter::reset(*device) != CmdResult::ACK_OK)
        exit_gracefully("ERROR: Could not reset the filter!");


    //
    // Register data callbacks
    //

    //Sensor Data
    DispatchHandler sensor_data_handlers[4];

    device->registerExtractor(sensor_data_handlers[0], &sensor_gps_time);
    device->registerExtractor(sensor_data_handlers[1], &sensor_accel);
    device->registerExtractor(sensor_data_handlers[2], &sensor_gyro);
    device->registerExtractor(sensor_data_handlers[3], &sensor_mag);

    //GNSS Data
    DispatchHandler gnss_data_handlers[1];

    device->registerExtractor(gnss_data_handlers[0], &gnss_fix_info);
 
    //Filter Data
    DispatchHandler filter_data_handlers[7];

    device->registerExtractor(filter_data_handlers[0], &filter_gps_time);
    device->registerExtractor(filter_data_handlers[1], &filter_status);
    device->registerExtractor(filter_data_handlers[2], &filter_position_llh);
    device->registerExtractor(filter_data_handlers[3], &filter_velocity_ned);
    device->registerExtractor(filter_data_handlers[4], &filter_euler_angles);
    device->registerExtractor(filter_data_handlers[5], &filter_comp_angular_rate);
    device->registerExtractor(filter_data_handlers[6], &filter_comp_accel);
    

    //
    //Resume the device
    //

    if(commands_base::resume(*device) != CmdResult::ACK_OK)
        exit_gracefully("ERROR: Could not resume the device!");


    //
    //Main Loop: Update the interface and process data
    //

    bool running = true;
    mip::Timestamp prev_print_timestamp = getCurrentTimestamp();

    printf("Sensor is configured... waiting for filter to enter running mode.\n");

    while(running)
    {
        device->update();
 
        //Check Filter State
        if((!filter_state_running) && ((filter_status.filter_state == data_filter::FilterMode::GX5_RUN_SOLUTION_ERROR) || (filter_status.filter_state == data_filter::FilterMode::GX5_RUN_SOLUTION_VALID)))
        {
            printf("NOTE: Filter has entered running mode.\n");
            filter_state_running = true;
        }

        //Once in running mode, print out data at 1 Hz
        if(filter_state_running)
        {
           mip::Timestamp curr_timestamp = getCurrentTimestamp();

           if(curr_timestamp - prev_print_timestamp >= 1000)
           {
                printf("TOW = %f: ATT_EULER = [%f %f %f]: COMP_ANG_RATE = [%f %f %f]: COMP_ACCEL = [%f %f %f]\n",
                       filter_gps_time.tow, filter_euler_angles.roll, filter_euler_angles.pitch, filter_euler_angles.yaw, 
                       filter_comp_angular_rate.gyro[0], filter_comp_angular_rate.gyro[1], filter_comp_angular_rate.gyro[2],
                       filter_comp_accel.accel[0], filter_comp_accel.accel[1], filter_comp_accel.accel[2]);

                prev_print_timestamp = curr_timestamp;
           }
        }

        running = !should_exit();
    }

    exit_gracefully("Example Completed Successfully.");
}


////////////////////////////////////////////////////////////////////////////////
// Print Usage Function
////////////////////////////////////////////////////////////////////////////////

int usage(const char* argv0)
{
    printf("Usage: %s <port> <baudrate>\n", argv0);
    return 1;
}


////////////////////////////////////////////////////////////////////////////////
// Exit Function
////////////////////////////////////////////////////////////////////////////////

void exit_gracefully(const char *message)
{
    if(message)
        printf("%s\n", message);

#ifdef _WIN32
    int dummy = getchar();
#endif

    exit(0);
}


////////////////////////////////////////////////////////////////////////////////
// Check for Exit Condition
////////////////////////////////////////////////////////////////////////////////

bool should_exit()
{
  return false;

}

