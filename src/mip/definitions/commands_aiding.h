#pragma once

#include "descriptors.h"
#include "../mip_result.h"

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
namespace mip {
namespace C {
extern "C" {

#endif // __cplusplus
struct mip_interface;
struct mip_serializer;
struct mip_field;

////////////////////////////////////////////////////////////////////////////////
///@addtogroup MipCommands_c  MIP Commands [C]
///@{
///@defgroup aiding_commands_c  Aiding Commands [C]
///
///@{

////////////////////////////////////////////////////////////////////////////////
// Descriptors
////////////////////////////////////////////////////////////////////////////////

enum 
{
    MIP_AIDING_CMD_DESC_SET                = 0x13,
    
    MIP_CMD_DESC_AIDING_FRAME_CONFIG       = 0x01,
    MIP_CMD_DESC_AIDING_SENSOR_FRAME_MAP   = 0x02,
    MIP_CMD_DESC_AIDING_LOCAL_FRAME        = 0x03,
    MIP_CMD_DESC_AIDING_ECHO_CONTROL       = 0x1F,
    MIP_CMD_DESC_AIDING_POS_LOCAL          = 0x20,
    MIP_CMD_DESC_AIDING_POS_ECEF           = 0x21,
    MIP_CMD_DESC_AIDING_POS_LLH            = 0x22,
    MIP_CMD_DESC_AIDING_HEIGHT_ABS         = 0x23,
    MIP_CMD_DESC_AIDING_HEIGHT_REL         = 0x24,
    MIP_CMD_DESC_AIDING_VEL_ECEF           = 0x28,
    MIP_CMD_DESC_AIDING_VEL_NED            = 0x29,
    MIP_CMD_DESC_AIDING_VEL_ODOM           = 0x2A,
    MIP_CMD_DESC_AIDING_WHEELSPEED         = 0x2B,
    MIP_CMD_DESC_AIDING_HEADING_TRUE       = 0x31,
    MIP_CMD_DESC_AIDING_DELTA_POSITION     = 0x38,
    MIP_CMD_DESC_AIDING_DELTA_ATTITUDE     = 0x39,
    MIP_CMD_DESC_AIDING_LOCAL_ANGULAR_RATE = 0x3A,
    
    MIP_REPLY_DESC_AIDING_SENSOR_FRAME_MAP = 0x82,
    MIP_REPLY_DESC_AIDING_FRAME_CONFIG     = 0x81,
    MIP_REPLY_DESC_AIDING_ECHO_CONTROL     = 0x9F,
};

////////////////////////////////////////////////////////////////////////////////
// Shared Type Definitions
////////////////////////////////////////////////////////////////////////////////

typedef uint8_t mip_time_timebase;
static const mip_time_timebase MIP_TIME_TIMEBASE_INTERNAL_REFERENCE = 1; ///<  Internal reference time from the device.
static const mip_time_timebase MIP_TIME_TIMEBASE_EXTERNAL_TIME      = 2; ///<  External reference time from PPS.

struct mip_time
{
    mip_time_timebase timebase; ///< Timebase reference, e.g. internal, external, GPS, UTC, etc.
    uint8_t reserved; ///< Reserved, set to 0x01.
    uint64_t nanoseconds; ///< Nanoseconds since the timebase epoch.
    
};
typedef struct mip_time mip_time;
void insert_mip_time(struct mip_serializer* serializer, const mip_time* self);
void extract_mip_time(struct mip_serializer* serializer, mip_time* self);

void insert_mip_time_timebase(struct mip_serializer* serializer, const mip_time_timebase self);
void extract_mip_time_timebase(struct mip_serializer* serializer, mip_time_timebase* self);


////////////////////////////////////////////////////////////////////////////////
// Mip Fields
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
///@defgroup c_aiding_sensor_frame_mapping  (0x13,0x02) Sensor Frame Mapping [C]
///
///@{

struct mip_aiding_sensor_frame_mapping_command
{
    mip_function_selector function;
    uint8_t sensor_id; ///< Sensor ID to configure. Cannot be 0.
    uint8_t frame_id; ///< Frame ID to assign to the sensor. Defaults to 1.
    
};
typedef struct mip_aiding_sensor_frame_mapping_command mip_aiding_sensor_frame_mapping_command;
void insert_mip_aiding_sensor_frame_mapping_command(struct mip_serializer* serializer, const mip_aiding_sensor_frame_mapping_command* self);
void extract_mip_aiding_sensor_frame_mapping_command(struct mip_serializer* serializer, mip_aiding_sensor_frame_mapping_command* self);

struct mip_aiding_sensor_frame_mapping_response
{
    uint8_t sensor_id; ///< Sensor ID to configure. Cannot be 0.
    uint8_t frame_id; ///< Frame ID to assign to the sensor. Defaults to 1.
    
};
typedef struct mip_aiding_sensor_frame_mapping_response mip_aiding_sensor_frame_mapping_response;
void insert_mip_aiding_sensor_frame_mapping_response(struct mip_serializer* serializer, const mip_aiding_sensor_frame_mapping_response* self);
void extract_mip_aiding_sensor_frame_mapping_response(struct mip_serializer* serializer, mip_aiding_sensor_frame_mapping_response* self);

mip_cmd_result mip_aiding_write_sensor_frame_mapping(struct mip_interface* device, uint8_t sensor_id, uint8_t frame_id);
mip_cmd_result mip_aiding_read_sensor_frame_mapping(struct mip_interface* device, uint8_t* sensor_id_out, uint8_t* frame_id_out);
mip_cmd_result mip_aiding_save_sensor_frame_mapping(struct mip_interface* device);
mip_cmd_result mip_aiding_load_sensor_frame_mapping(struct mip_interface* device);
mip_cmd_result mip_aiding_default_sensor_frame_mapping(struct mip_interface* device);
///@}
///
////////////////////////////////////////////////////////////////////////////////
///@defgroup c_aiding_reference_frame  (0x13,0x01) Reference Frame [C]
///
///@{

typedef uint8_t mip_aiding_reference_frame_command_format;
static const mip_aiding_reference_frame_command_format MIP_AIDING_REFERENCE_FRAME_COMMAND_FORMAT_EULER      = 1; ///<  Translation vector followed by euler angles (roll, pitch, yaw).
static const mip_aiding_reference_frame_command_format MIP_AIDING_REFERENCE_FRAME_COMMAND_FORMAT_QUATERNION = 2; ///<  Translation vector followed by quaternion (w, x, y, z).

struct mip_aiding_reference_frame_command
{
    mip_function_selector function;
    uint8_t frame_id; ///< Reference frame number. Cannot be 0.
    mip_aiding_reference_frame_command_format format; ///< Format of the transformation.
    float translation[3]; ///< Translation X, Y, and Z.
    float rotation[4]; ///< Depends on the format parameter. Unused values are ignored.
    
};
typedef struct mip_aiding_reference_frame_command mip_aiding_reference_frame_command;
void insert_mip_aiding_reference_frame_command(struct mip_serializer* serializer, const mip_aiding_reference_frame_command* self);
void extract_mip_aiding_reference_frame_command(struct mip_serializer* serializer, mip_aiding_reference_frame_command* self);

void insert_mip_aiding_reference_frame_command_format(struct mip_serializer* serializer, const mip_aiding_reference_frame_command_format self);
void extract_mip_aiding_reference_frame_command_format(struct mip_serializer* serializer, mip_aiding_reference_frame_command_format* self);

struct mip_aiding_reference_frame_response
{
    uint8_t frame_id; ///< Reference frame number. Cannot be 0.
    mip_aiding_reference_frame_command_format format; ///< Format of the transformation.
    float translation[3]; ///< Translation X, Y, and Z.
    float rotation[4]; ///< Depends on the format parameter. Unused values are ignored.
    
};
typedef struct mip_aiding_reference_frame_response mip_aiding_reference_frame_response;
void insert_mip_aiding_reference_frame_response(struct mip_serializer* serializer, const mip_aiding_reference_frame_response* self);
void extract_mip_aiding_reference_frame_response(struct mip_serializer* serializer, mip_aiding_reference_frame_response* self);

mip_cmd_result mip_aiding_write_reference_frame(struct mip_interface* device, uint8_t frame_id, mip_aiding_reference_frame_command_format format, const float* translation, const float* rotation);
mip_cmd_result mip_aiding_read_reference_frame(struct mip_interface* device, uint8_t* frame_id_out, mip_aiding_reference_frame_command_format* format_out, float* translation_out, float* rotation_out);
mip_cmd_result mip_aiding_save_reference_frame(struct mip_interface* device);
mip_cmd_result mip_aiding_load_reference_frame(struct mip_interface* device);
mip_cmd_result mip_aiding_default_reference_frame(struct mip_interface* device);
///@}
///
////////////////////////////////////////////////////////////////////////////////
///@defgroup c_aiding_aiding_echo_control  (0x13,0x1F) Aiding Echo Control [C]
///
///@{

typedef uint8_t mip_aiding_aiding_echo_control_command_mode;
static const mip_aiding_aiding_echo_control_command_mode MIP_AIDING_AIDING_ECHO_CONTROL_COMMAND_MODE_SUPPRESS_ACK = 0; ///<  Suppresses the usual command ack field for aiding messages.
static const mip_aiding_aiding_echo_control_command_mode MIP_AIDING_AIDING_ECHO_CONTROL_COMMAND_MODE_STANDARD     = 1; ///<  Normal ack/nack behavior.
static const mip_aiding_aiding_echo_control_command_mode MIP_AIDING_AIDING_ECHO_CONTROL_COMMAND_MODE_RESPONSE     = 2; ///<  Echo the data back as a response.

struct mip_aiding_aiding_echo_control_command
{
    mip_function_selector function;
    mip_aiding_aiding_echo_control_command_mode mode; ///< Controls data echoing.
    
};
typedef struct mip_aiding_aiding_echo_control_command mip_aiding_aiding_echo_control_command;
void insert_mip_aiding_aiding_echo_control_command(struct mip_serializer* serializer, const mip_aiding_aiding_echo_control_command* self);
void extract_mip_aiding_aiding_echo_control_command(struct mip_serializer* serializer, mip_aiding_aiding_echo_control_command* self);

void insert_mip_aiding_aiding_echo_control_command_mode(struct mip_serializer* serializer, const mip_aiding_aiding_echo_control_command_mode self);
void extract_mip_aiding_aiding_echo_control_command_mode(struct mip_serializer* serializer, mip_aiding_aiding_echo_control_command_mode* self);

struct mip_aiding_aiding_echo_control_response
{
    mip_aiding_aiding_echo_control_command_mode mode; ///< Controls data echoing.
    
};
typedef struct mip_aiding_aiding_echo_control_response mip_aiding_aiding_echo_control_response;
void insert_mip_aiding_aiding_echo_control_response(struct mip_serializer* serializer, const mip_aiding_aiding_echo_control_response* self);
void extract_mip_aiding_aiding_echo_control_response(struct mip_serializer* serializer, mip_aiding_aiding_echo_control_response* self);

mip_cmd_result mip_aiding_write_aiding_echo_control(struct mip_interface* device, mip_aiding_aiding_echo_control_command_mode mode);
mip_cmd_result mip_aiding_read_aiding_echo_control(struct mip_interface* device, mip_aiding_aiding_echo_control_command_mode* mode_out);
mip_cmd_result mip_aiding_save_aiding_echo_control(struct mip_interface* device);
mip_cmd_result mip_aiding_load_aiding_echo_control(struct mip_interface* device);
mip_cmd_result mip_aiding_default_aiding_echo_control(struct mip_interface* device);
///@}
///
////////////////////////////////////////////////////////////////////////////////
///@defgroup c_aiding_ecef_pos  (0x13,0x21) Ecef Pos [C]
/// Cartesian vector position aiding command. Coordinates are given in the WGS84 ECEF system.
///
///@{

typedef uint16_t mip_aiding_ecef_pos_command_valid_flags;
static const mip_aiding_ecef_pos_command_valid_flags MIP_AIDING_ECEF_POS_COMMAND_VALID_FLAGS_NONE = 0x0000;
static const mip_aiding_ecef_pos_command_valid_flags MIP_AIDING_ECEF_POS_COMMAND_VALID_FLAGS_X    = 0x0002; ///<  
static const mip_aiding_ecef_pos_command_valid_flags MIP_AIDING_ECEF_POS_COMMAND_VALID_FLAGS_Y    = 0x0004; ///<  
static const mip_aiding_ecef_pos_command_valid_flags MIP_AIDING_ECEF_POS_COMMAND_VALID_FLAGS_Z    = 0x0008; ///<  
static const mip_aiding_ecef_pos_command_valid_flags MIP_AIDING_ECEF_POS_COMMAND_VALID_FLAGS_ALL  = 0x000E;

struct mip_aiding_ecef_pos_command
{
    mip_time time; ///< Timestamp of the measurement.
    uint8_t sensor_id; ///< Sensor ID.
    double position[3]; ///< ECEF position [m].
    float uncertainty[3]; ///< ECEF position uncertainty [m].
    mip_aiding_ecef_pos_command_valid_flags valid_flags; ///< Valid flags.
    
};
typedef struct mip_aiding_ecef_pos_command mip_aiding_ecef_pos_command;
void insert_mip_aiding_ecef_pos_command(struct mip_serializer* serializer, const mip_aiding_ecef_pos_command* self);
void extract_mip_aiding_ecef_pos_command(struct mip_serializer* serializer, mip_aiding_ecef_pos_command* self);

void insert_mip_aiding_ecef_pos_command_valid_flags(struct mip_serializer* serializer, const mip_aiding_ecef_pos_command_valid_flags self);
void extract_mip_aiding_ecef_pos_command_valid_flags(struct mip_serializer* serializer, mip_aiding_ecef_pos_command_valid_flags* self);

mip_cmd_result mip_aiding_ecef_pos(struct mip_interface* device, const mip_time* time, uint8_t sensor_id, const double* position, const float* uncertainty, mip_aiding_ecef_pos_command_valid_flags valid_flags);
///@}
///
////////////////////////////////////////////////////////////////////////////////
///@defgroup c_aiding_llh_pos  (0x13,0x22) Llh Pos [C]
/// Geodetic position aiding command. Coordinates are given in WGS84 geodetic latitude, longitude, and height above the ellipsoid.
/// Uncertainty is given in NED coordinates, which are parallel to incremental changes in latitude, longitude, and height.
///
///@{

typedef uint16_t mip_aiding_llh_pos_command_valid_flags;
static const mip_aiding_llh_pos_command_valid_flags MIP_AIDING_LLH_POS_COMMAND_VALID_FLAGS_NONE      = 0x0000;
static const mip_aiding_llh_pos_command_valid_flags MIP_AIDING_LLH_POS_COMMAND_VALID_FLAGS_LATITUDE  = 0x0002; ///<  
static const mip_aiding_llh_pos_command_valid_flags MIP_AIDING_LLH_POS_COMMAND_VALID_FLAGS_LONGITUDE = 0x0004; ///<  
static const mip_aiding_llh_pos_command_valid_flags MIP_AIDING_LLH_POS_COMMAND_VALID_FLAGS_HEIGHT    = 0x0008; ///<  
static const mip_aiding_llh_pos_command_valid_flags MIP_AIDING_LLH_POS_COMMAND_VALID_FLAGS_ALL       = 0x000E;

struct mip_aiding_llh_pos_command
{
    mip_time time; ///< Timestamp of the measurement.
    uint8_t sensor_id; ///< Sensor ID.
    double latitude; ///< [deg]
    double longitude; ///< [deg]
    double height; ///< [m]
    float uncertainty[3]; ///< NED position uncertainty.
    mip_aiding_llh_pos_command_valid_flags valid_flags; ///< Valid flags.
    
};
typedef struct mip_aiding_llh_pos_command mip_aiding_llh_pos_command;
void insert_mip_aiding_llh_pos_command(struct mip_serializer* serializer, const mip_aiding_llh_pos_command* self);
void extract_mip_aiding_llh_pos_command(struct mip_serializer* serializer, mip_aiding_llh_pos_command* self);

void insert_mip_aiding_llh_pos_command_valid_flags(struct mip_serializer* serializer, const mip_aiding_llh_pos_command_valid_flags self);
void extract_mip_aiding_llh_pos_command_valid_flags(struct mip_serializer* serializer, mip_aiding_llh_pos_command_valid_flags* self);

mip_cmd_result mip_aiding_llh_pos(struct mip_interface* device, const mip_time* time, uint8_t sensor_id, double latitude, double longitude, double height, const float* uncertainty, mip_aiding_llh_pos_command_valid_flags valid_flags);
///@}
///
////////////////////////////////////////////////////////////////////////////////
///@defgroup c_aiding_ecef_vel  (0x13,0x28) Ecef Vel [C]
/// ECEF velocity aiding command. Coordinates are given in the WGS84 ECEF frame.
///
///@{

typedef uint16_t mip_aiding_ecef_vel_command_valid_flags;
static const mip_aiding_ecef_vel_command_valid_flags MIP_AIDING_ECEF_VEL_COMMAND_VALID_FLAGS_NONE = 0x0000;
static const mip_aiding_ecef_vel_command_valid_flags MIP_AIDING_ECEF_VEL_COMMAND_VALID_FLAGS_X    = 0x0002; ///<  
static const mip_aiding_ecef_vel_command_valid_flags MIP_AIDING_ECEF_VEL_COMMAND_VALID_FLAGS_Y    = 0x0004; ///<  
static const mip_aiding_ecef_vel_command_valid_flags MIP_AIDING_ECEF_VEL_COMMAND_VALID_FLAGS_Z    = 0x0008; ///<  
static const mip_aiding_ecef_vel_command_valid_flags MIP_AIDING_ECEF_VEL_COMMAND_VALID_FLAGS_ALL  = 0x000E;

struct mip_aiding_ecef_vel_command
{
    mip_time time; ///< Timestamp of the measurement.
    uint8_t sensor_id; ///< Sensor ID.
    float velocity[3]; ///< ECEF velocity [m/s].
    float uncertainty[3]; ///< ECEF velocity uncertainty [m/s].
    mip_aiding_ecef_vel_command_valid_flags valid_flags; ///< Valid flags.
    
};
typedef struct mip_aiding_ecef_vel_command mip_aiding_ecef_vel_command;
void insert_mip_aiding_ecef_vel_command(struct mip_serializer* serializer, const mip_aiding_ecef_vel_command* self);
void extract_mip_aiding_ecef_vel_command(struct mip_serializer* serializer, mip_aiding_ecef_vel_command* self);

void insert_mip_aiding_ecef_vel_command_valid_flags(struct mip_serializer* serializer, const mip_aiding_ecef_vel_command_valid_flags self);
void extract_mip_aiding_ecef_vel_command_valid_flags(struct mip_serializer* serializer, mip_aiding_ecef_vel_command_valid_flags* self);

mip_cmd_result mip_aiding_ecef_vel(struct mip_interface* device, const mip_time* time, uint8_t sensor_id, const float* velocity, const float* uncertainty, mip_aiding_ecef_vel_command_valid_flags valid_flags);
///@}
///
////////////////////////////////////////////////////////////////////////////////
///@defgroup c_aiding_ned_vel  (0x13,0x29) Ned Vel [C]
/// NED velocity aiding command. Coordinates are given in the local North-East-Down frame.
///
///@{

typedef uint16_t mip_aiding_ned_vel_command_valid_flags;
static const mip_aiding_ned_vel_command_valid_flags MIP_AIDING_NED_VEL_COMMAND_VALID_FLAGS_NONE = 0x0000;
static const mip_aiding_ned_vel_command_valid_flags MIP_AIDING_NED_VEL_COMMAND_VALID_FLAGS_X    = 0x0002; ///<  
static const mip_aiding_ned_vel_command_valid_flags MIP_AIDING_NED_VEL_COMMAND_VALID_FLAGS_Y    = 0x0004; ///<  
static const mip_aiding_ned_vel_command_valid_flags MIP_AIDING_NED_VEL_COMMAND_VALID_FLAGS_Z    = 0x0008; ///<  
static const mip_aiding_ned_vel_command_valid_flags MIP_AIDING_NED_VEL_COMMAND_VALID_FLAGS_ALL  = 0x000E;

struct mip_aiding_ned_vel_command
{
    mip_time time; ///< Timestamp of the measurement.
    uint8_t sensor_id; ///< Sensor ID.
    float velocity[3]; ///< NED velocity [m/s].
    float uncertainty[3]; ///< NED velocity uncertainty [m/s].
    mip_aiding_ned_vel_command_valid_flags valid_flags; ///< Valid flags.
    
};
typedef struct mip_aiding_ned_vel_command mip_aiding_ned_vel_command;
void insert_mip_aiding_ned_vel_command(struct mip_serializer* serializer, const mip_aiding_ned_vel_command* self);
void extract_mip_aiding_ned_vel_command(struct mip_serializer* serializer, mip_aiding_ned_vel_command* self);

void insert_mip_aiding_ned_vel_command_valid_flags(struct mip_serializer* serializer, const mip_aiding_ned_vel_command_valid_flags self);
void extract_mip_aiding_ned_vel_command_valid_flags(struct mip_serializer* serializer, mip_aiding_ned_vel_command_valid_flags* self);

mip_cmd_result mip_aiding_ned_vel(struct mip_interface* device, const mip_time* time, uint8_t sensor_id, const float* velocity, const float* uncertainty, mip_aiding_ned_vel_command_valid_flags valid_flags);
///@}
///
////////////////////////////////////////////////////////////////////////////////
///@defgroup c_aiding_vehicle_fixed_frame_velocity  (0x13,0x2A) Vehicle Fixed Frame Velocity [C]
/// Estimate of velocity of the vehicle in the frame associated
/// with the given sensor ID.
///
///@{

typedef uint16_t mip_aiding_vehicle_fixed_frame_velocity_command_valid_flags;
static const mip_aiding_vehicle_fixed_frame_velocity_command_valid_flags MIP_AIDING_VEHICLE_FIXED_FRAME_VELOCITY_COMMAND_VALID_FLAGS_NONE = 0x0000;
static const mip_aiding_vehicle_fixed_frame_velocity_command_valid_flags MIP_AIDING_VEHICLE_FIXED_FRAME_VELOCITY_COMMAND_VALID_FLAGS_X    = 0x0002; ///<  
static const mip_aiding_vehicle_fixed_frame_velocity_command_valid_flags MIP_AIDING_VEHICLE_FIXED_FRAME_VELOCITY_COMMAND_VALID_FLAGS_Y    = 0x0004; ///<  
static const mip_aiding_vehicle_fixed_frame_velocity_command_valid_flags MIP_AIDING_VEHICLE_FIXED_FRAME_VELOCITY_COMMAND_VALID_FLAGS_Z    = 0x0008; ///<  
static const mip_aiding_vehicle_fixed_frame_velocity_command_valid_flags MIP_AIDING_VEHICLE_FIXED_FRAME_VELOCITY_COMMAND_VALID_FLAGS_ALL  = 0x000E;

struct mip_aiding_vehicle_fixed_frame_velocity_command
{
    mip_time time; ///< Timestamp of the measurement.
    uint8_t sensor_id; ///< Source ID for this estimate ( source_id == 0 indicates this sensor, source_id > 0 indicates an external estimate )
    float velocity[3]; ///< [m/s]
    float uncertainty[3]; ///< [m/s] 1-sigma uncertainty (if velocity_uncertainty[i] <= 0, then velocity[i] should be treated as invalid and ingnored)
    mip_aiding_vehicle_fixed_frame_velocity_command_valid_flags valid_flags;
    
};
typedef struct mip_aiding_vehicle_fixed_frame_velocity_command mip_aiding_vehicle_fixed_frame_velocity_command;
void insert_mip_aiding_vehicle_fixed_frame_velocity_command(struct mip_serializer* serializer, const mip_aiding_vehicle_fixed_frame_velocity_command* self);
void extract_mip_aiding_vehicle_fixed_frame_velocity_command(struct mip_serializer* serializer, mip_aiding_vehicle_fixed_frame_velocity_command* self);

void insert_mip_aiding_vehicle_fixed_frame_velocity_command_valid_flags(struct mip_serializer* serializer, const mip_aiding_vehicle_fixed_frame_velocity_command_valid_flags self);
void extract_mip_aiding_vehicle_fixed_frame_velocity_command_valid_flags(struct mip_serializer* serializer, mip_aiding_vehicle_fixed_frame_velocity_command_valid_flags* self);

mip_cmd_result mip_aiding_vehicle_fixed_frame_velocity(struct mip_interface* device, const mip_time* time, uint8_t sensor_id, const float* velocity, const float* uncertainty, mip_aiding_vehicle_fixed_frame_velocity_command_valid_flags valid_flags);
///@}
///
////////////////////////////////////////////////////////////////////////////////
///@defgroup c_aiding_true_heading  (0x13,0x31) True Heading [C]
///
///@{

struct mip_aiding_true_heading_command
{
    mip_time time;
    uint8_t sensor_id;
    float heading; ///< Heading in [radians]
    float uncertainty;
    uint16_t valid_flags;
    
};
typedef struct mip_aiding_true_heading_command mip_aiding_true_heading_command;
void insert_mip_aiding_true_heading_command(struct mip_serializer* serializer, const mip_aiding_true_heading_command* self);
void extract_mip_aiding_true_heading_command(struct mip_serializer* serializer, mip_aiding_true_heading_command* self);

mip_cmd_result mip_aiding_true_heading(struct mip_interface* device, const mip_time* time, uint8_t sensor_id, float heading, float uncertainty, uint16_t valid_flags);
///@}
///

///@}
///@}
///
////////////////////////////////////////////////////////////////////////////////
#ifdef __cplusplus
} // namespace C
} // namespace mip
} // extern "C"
#endif // __cplusplus

